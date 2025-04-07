#
# take main.cpp and create a new file called main_profiled.cpp
# that is copy of main.cpp with code for profiling added in.
#
# this script won't work without running set_up first

# for compiling and running main_profiled.cpp
import subprocess
import os
import itertools

from scripts.python.compile_main import compile_for_windows
from scripts.python.compule_main_for_linux import compile_and_run_linux

import argparse

import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import numpy as np

sections = [ 'imgui set up', 
             'grid buffer set up', 
             'boundary setting',
             'collide and stream',
             'property calculation',
             'vertex calculation',
             'render set up',
             'render',
             'imgui render' ]

sections_par = [ 'imgui set up', 
                 'grid buffer set up', 
                 'boundary setting',
                 'grid copy',
                 'collide and stream',
                 'property calculation',
                 'vertex calculation',
                 'render set up',
                 'render',
                 'imgui render' ]

kernels = [ 'kernel execution ']

def insert_profiling( object_path, target_path, out, gpu_out, GPU, sections ):

    profile_steps = 10
    
    with open( object_path, 'r', encoding = 'utf-8' ) as file:
        original_content = file.readlines()

    profiled_content = []

    profiled_content.append( '#include <fstream>\n\n' )

    ofstream_defined = False

    for original_line in original_content:

        stripped_line = original_line.lstrip()

        indent = original_line[:len( original_line) - len( stripped_line ) ]

        matched = False

        stripped_line = stripped_line.rstrip()

        for section in sections:

            if stripped_line == 'while ( !glfwWindowShouldClose( window ) ) {':
                profiled_content.append(f'{ indent }for ( size_t i = 0; i < { profile_steps }; ++i ) {{\n' )
                matched = True
                break

            elif stripped_line == 'bool simulation_running = false;':
                profiled_content.append( f'{ indent }bool simulation_running = true;\n' )
                matched = True
                break

            elif stripped_line == f"// start { section }":
                cpp_var = section.replace( ' ', '_' )
                profiled_content.append( f"{ indent }auto start_{ cpp_var } = std::chrono::high_resolution_clock::now();\n"  )
                matched = True
                break

            elif stripped_line == f"// end { section }":
                cpp_var = section.replace( ' ', '_' )
                profiled_content.append( f"{ indent }auto end_{ cpp_var } = std::chrono::high_resolution_clock::now();\n" )
                
                if ofstream_defined == False:
                    if GPU:
                        profiled_content.append( f"{ indent }std::ofstream file(\"{ gpu_out }\");\n" )
                    else:
                        profiled_content.append( f"{ indent }std::ofstream file(\"{ out }\");\n" )
                    ofstream_defined = True

                profiled_content.append( f"{ indent }file << \"{ section }: \" <<  std::chrono::duration_cast<std::chrono::microseconds>( end_{ cpp_var } - start_{ cpp_var } ).count() << std::endl; \n" ) 
                matched = True
                break
        
        if matched == False:
            profiled_content.append( original_line )
    
    with open( target_path, 'w', encoding = 'utf-8' ) as file:
        file.writelines( profiled_content )

def insert_profiling_abs( object_path, target_path, out, gpu_out, state_out, GPU, state, sections ):

    profile_steps = 10
    
    with open( object_path, 'r', encoding = 'utf-8' ) as file:
        original_content = file.readlines()

    profiled_content = []

    profiled_content.append( '#include <fstream>\n\n' )

    for original_line in original_content:

        stripped_line = original_line.lstrip()

        indent = original_line[:len( original_line) - len( stripped_line ) ]

        matched = False

        stripped_line = stripped_line.rstrip()

        for section in sections:

            if stripped_line == 'while ( !glfwWindowShouldClose( window ) ) {':
                profiled_content.append(f'{ indent }for ( size_t i = 0; i < { profile_steps }; ++i ) {{\n' )
                matched = True
                break

            elif stripped_line == 'bool simulation_running = false;':
                profiled_content.append( f'{ indent }bool simulation_running = true;\n' )
                matched = True
                break

            elif stripped_line == f"// start loop":
                if state: 
                    profiled_content.append( f"{ indent }std::ofstream file(\"{ state_out }\");\n" )
                elif GPU:
                    profiled_content.append( f"{ indent }std::ofstream file(\"{ gpu_out }\");\n" )
                else:
                    profiled_content.append( f"{ indent }std::ofstream file(\"{ out }\");\n" )

                profiled_content.append( f"{ indent }auto start_loop = std::chrono::high_resolution_clock::now();\n" )
                matched = True
                break
                
            elif stripped_line == f"// start { section }":
                cpp_var = section.replace( ' ', '_' )
                profiled_content.append( f"{ indent }auto start_{ cpp_var } = std::chrono::high_resolution_clock::now();\n" )
                profiled_content.append( f"{ indent }file << \"{ section } start: \" << std::chrono::duration_cast<std::chrono::microseconds>( start_{ cpp_var } - start_loop ).count() << std::endl;;\n"  )
                matched = True
                break

            elif stripped_line == f"// end { section }":
                cpp_var = section.replace( ' ', '_' )
                profiled_content.append( f"{ indent }auto end_{ cpp_var } = std::chrono::high_resolution_clock::now();\n" )
                profiled_content.append( f"{ indent }file << \"{ section }: \" <<  std::chrono::duration_cast<std::chrono::microseconds>( end_{ cpp_var } - start_{ cpp_var } ).count() << std::endl; \n" ) 
                matched = True
                break
        
        if matched == False:
            profiled_content.append( original_line )
    
    with open( target_path, 'w', encoding = 'utf-8' ) as file:
        file.writelines( profiled_content )

def run_on_windows():
    run_command = ".\main_profile.exe"
    print( f"Running main_profile.exe..." )
    subprocess.run( run_command, shell = True )

def plot_profiling_data( filename ):

    with open( filename, 'r' ) as file:
        lines = file.readlines()

    states = []
    times = []
    
    for line in lines:
        if line.strip():
            parts = line.split( ':' )
            state = parts[ 0 ].strip()
            time = int( parts[ 1 ].strip() )
            states.append( state )
            times.append( time )

    # Calculate cumulative times for stacking the bars end-to-end
    cumulative_times = [sum(times[:i+1]) for i in range(len(times))]

    # Create a figure with a wide aspect ratio (short in height, long in width)
    fig, ax = plt.subplots(figsize=(14, 1))  # Adjusted height for more space horizontally

    # Create the stacked bars with a minimal height to create a line-style appearance
    bar_colors = plt.cm.get_cmap('tab10', len(states))  # Use a colormap
    
    # Create the bars sequentially using the cumulative times
    for i, (state, time, cumulative_time) in enumerate(zip(states, times, cumulative_times)):
        ax.barh(y=0, width=time, left=cumulative_time - time, color=bar_colors(i), height=0.3)  # Reduced height to 0.3 for thinner bars

    # Remove Y-axis ticks and labels as we don't need them
    ax.set_yticks([])

    # Create a legend arranged in a grid
    patches = [mpatches.Patch(color=bar_colors(i), label=f"{state}: {time} µs") for i, (state, time) in enumerate(zip(states, times))]
    ax.legend(handles=patches, loc="upper center", bbox_to_anchor=(0.5, -0.8), ncol=3, fontsize=10, frameon=False)  # Moved legend further down

    # Adjust the layout manually to ensure the chart and legend do not overlap
    plt.subplots_adjust(bottom=0.5)  # Increased bottom margin for the legend

    ax.spines['left'].set_visible(False)   # Remove the left spine
    ax.spines['right'].set_visible(False)  # Remove the right spine
    ax.spines['top'].set_visible(False)

    # Save the plot to a PNG file
    if ( filename == 'profile_output.txt' ):
        plt.savefig("profiling_data.png", bbox_inches='tight', dpi=300) 
    else:
        plt.savefig("profiling_data_gpu.png", bbox_inches='tight', dpi=300) 
    plt.close() 

def plot_profiling_data_fixed( filename, fixed_length=150000 ):
    with open(filename, 'r') as file:
        lines = file.readlines()

    states = []
    times = []
    
    for line in lines:
        if line.strip():
            parts = line.split(':')
            state = parts[0].strip()
            time = int(parts[1].strip())
            states.append(state)
            times.append(time)

    # Calculate cumulative times for stacking the bars end-to-end
    cumulative_times = [sum(times[:i+1]) for i in range(len(times))]

    # Create a figure with a fixed width and an automatically calculated height
    fig, ax = plt.subplots(figsize=(14, 1))  # Adjust the height for more horizontal space

    # Create the stacked bars with a minimal height to create a line-style appearance
    bar_colors = plt.cm.get_cmap('tab10', len(states))  # Use a colormap
    
    # Create the bars sequentially using the cumulative times
    for i, (state, time, cumulative_time) in enumerate(zip(states, times, cumulative_times)):
        ax.barh(y=0, width=time, left=cumulative_time - time, color=bar_colors(i), height=0.3)  # Reduced height for thinner bars

    # Set the x-axis to always go from 0 to a fixed value (e.g., 10)
    ax.set_xlim(0, fixed_length)

    # Remove Y-axis ticks and labels as we don't need them
    ax.set_yticks([])

    # Create a legend arranged in a grid
    patches = [mpatches.Patch(color=bar_colors(i), label=f"{state}: {time} µs") for i, (state, time) in enumerate(zip(states, times))]
    ax.legend(handles=patches, loc="upper center", bbox_to_anchor=(0.5, -0.8), ncol=3, fontsize=10, frameon=False)  # Moved legend further down

    # Adjust the layout manually to ensure the chart and legend do not overlap
    plt.subplots_adjust(bottom=0.5)  # Increased bottom margin for the legend

    ax.spines[ 'left' ].set_visible(False)   # Remove the left spine
    ax.spines[ 'right' ].set_visible(False)  # Remove the right spine
    ax.spines[ 'top' ].set_visible(False)

    # Save the plot to a PNG file
    if filename == 'profile_output.txt':
        plt.savefig("profiling_data.png", bbox_inches='tight', dpi=300) 
    else:
        plt.savefig("profiling_data_gpu.png", bbox_inches='tight', dpi=300) 
    plt.close()

def plot_profiling_data_abs( filename, output_filename, fixed_length = 170000, par = False ):
    
    with open( filename, 'r' ) as file:
        lines = file.readlines()

    events = []  # [ start_time, duration, state ]

    for line in lines:
        if line.strip():
            parts = line.split( ':' )
            state = parts[ 0 ].strip()
            time = int( parts[ 1 ].strip() )

            if "start" in state: 
                base_state = state.replace( " start", "" )
                events.append( [ time, None, base_state ] ) 
            else:  
                for event in events:
                    if event[ 2 ] == state and event[ 1 ] is None: 
                        event[ 1 ] = time 
                        break

    states, start_times, durations = [], [], []

    for start, duration, state in events:
        states.append( state )
        start_times.append( start )
        durations.append( duration )  

    finish_times = [ start + duration for start, duration in zip( start_times, durations ) ]

    sorted_indices = sorted( range( len( states ) ), key = lambda i: start_times[ i ] )

    states = [ states[ i ] for i in sorted_indices ]
    start_times = [ start_times[ i ] for i in sorted_indices ]
    durations = [ durations[ i ] for i in sorted_indices ]
    finish_times = [ finish_times[ i ] for i in sorted_indices ]


    fig, ax = plt.subplots( figsize = ( 14, 4 ) ) 
    bar_colors = plt.cm.get_cmap( 'tab10', len( states ) )  

    bar_height = 0.2

    for i, ( state, start, duration ) in enumerate( zip( states, start_times, durations ) ):
        if par and state == 'collide and stream':
            y_pos = 0.75  
        else:
            y_pos = 0.25 if par else 0.5  

        ax.barh( y = y_pos, width = duration, left = start, color = bar_colors( i ), height = bar_height )  

    ax.set_xlim( 0, fixed_length )
    ax.set_ylim( 0, 1 )
    ax.set_yticks( [] )
    ax.set_yticklabels( [] ) 

    legend_entries = []
    for i, ( state, duration ) in enumerate( zip( states, durations ) ):
        if state not in [ entry[ 0 ] for entry in legend_entries ]:
            legend_entries.append( ( state, bar_colors( i ), duration ) )

    patches = [ mpatches.Patch( color = color, label = f"{ state }: { duration } µs" ) for state, color, duration in legend_entries ]
    ax.legend( handles = patches, loc = "upper center", bbox_to_anchor=( 0.5, -0.8 ), ncol = 3, fontsize = 10, frameon = False )

    plt.subplots_adjust( bottom = 0.5 )
    ax.spines[ 'left' ].set_visible( False )  
    ax.spines[ 'right' ].set_visible( False ) 
    ax.spines[ 'top' ].set_visible( False )  

    plt.savefig( output_filename, bbox_inches = 'tight', dpi = 300 )
    plt.close()

def profile_main( gpu, comp ):
    insert_profiling( 'main.cpp', 'main_profiled.cpp', 'profile_output.txt', 'profile_output_gpu.txt', gpu, sections )

    if comp:
        compile_for_windows(  'main_profiled.cpp', gpu )

    run_on_windows()

    if ( gpu ):
        plot_profiling_data_fixed( 'profile_output_gpu.txt' )
    else:
        plot_profiling_data_fixed( 'profile_output.txt' )

def profile_main_parallel( gpu, comp ):
    insert_profiling( 'main_parallel.cpp', 'main_parallel_profiled.cpp', 'profile_output_paralell.txt', 'profile_output_paralell_gpu.txt', gpu, sections_par )

    if comp:
        compile_for_windows( 'main_parallel_profiled.cpp', gpu )

    run_on_windows()

def profile_main_parallel_abs( gpu, comp, state ):
    insert_profiling_abs( 'main_parallel.cpp', 'main_parallel_profiled.cpp', 'profile_output_paralell_abs.txt', 'profile_output_paralell_gpu_abs.txt', 'profile_output_paralell_state_abs.txt', gpu, state, sections_par )

    if comp:
        compile_for_windows( 'main_parallel_profiled.cpp', gpu, state )

    run_on_windows()

    if state:
        plot_profiling_data_abs( 'profile_output_paralell_state_abs.txt', 'profiling_data_parallel_state_abs.png', par = True )
    elif gpu:
        plot_profiling_data_abs( 'profile_output_paralell_gpu_abs.txt', 'profiling_data_parallel_gpu_abs.png', par = True )
    else:
        plot_profiling_data_abs( 'profile_output_paralell_abs.txt', 'profiling_data_parallel_abs.png', par = True )

def profile_main_abs( gpu, comp, state ):
    insert_profiling_abs( 'main.cpp', 'main_profiled.cpp', 'profile_output_abs.txt', 'profile_output_gpu_abs.txt', gpu, sections )

    if comp:
        compile_for_windows( 'main_profiled.cpp', gpu, state )

    run_on_windows()

    if state:
        plot_profiling_data_abs( 'profile_output_state_abs.txt', 'profiling_data_state_abs.png', par = False )
    elif gpu:
        plot_profiling_data_abs( 'profile_output_gpu_abs.txt', 'profiling_data_gpu_abs.png', par = False )
    else:
        plot_profiling_data_abs( 'profile_output_abs.txt', 'profiling_data_abs.png', par = False )


if __name__ == '__main__':

    parser = argparse.ArgumentParser( description = 'Compile and run the program with profiling.' )

    parser.add_argument( '--gpu', action ='store_true', help = 'Use GPU acceleration.' )

    parser.add_argument( '--comp', action ='store_true', help = 'Re-compile with GPU acceleration.' )

    parser.add_argument( '--state', action ='store_true', help = 'Maintain state on the GPU.' )

    args = parser.parse_args()

    profile_main_parallel_abs( args.gpu, args.comp, args.state )
    # profile_main_abs( args.gpu, args.comp, args.state )
