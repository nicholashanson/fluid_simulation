#
# take main.cpp and create a new file called main_profiled.cpp
# that is copy of main.cpp with code for profiling added in.
#

# for compiling and running main_profiled.cpp
import subprocess
import os
import itertools

import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import numpy as np

def insert_profiling( object_path, target_path ):

    profile_steps = 10
    
    with open( object_path, 'r', encoding = 'utf-8' ) as file:
        original_content = file.readlines()

    sections = [ 'imgui set up', 
                 'grid buffer set up', 
                 'boundary setting',
                 'collide and stream',
                 'vertex calculation',
                 'render set up',
                 'render' ]

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

            if stripped_line == 'bool simulation_running = false;':

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
                    profiled_content.append( f"{ indent }std::ofstream file(\"profile_output.txt\");\n" )
                    ofstream_defined = True

                profiled_content.append( f"{ indent }file << \"{ section }: \" <<  std::chrono::duration_cast<std::chrono::microseconds>( end_{ cpp_var } - start_{ cpp_var } ).count() << std::endl; \n" ) 
                
                matched = True
                
                break
        
        if matched == False:
            profiled_content.append( original_line )
    
    with open( target_path, 'w', encoding = 'utf-8' ) as file:
        file.writelines( profiled_content )

def compile_and_run_linux():
    print('Compiling program...')

    gpp_args = "-g -v -std=c++23"

    files = [
        "../imgui-master/imgui.cpp",
        "../imgui-master/imgui_draw.cpp",
        "../imgui-master/imgui_widgets.cpp",
        "../imgui-master/imgui_tables.cpp",
        "../imgui-master/backends/imgui_impl_opengl3.cpp",
        "../imgui-master/backends/imgui_impl_glfw.cpp",
        "main_profiled.cpp", 
        "gl.cpp",
        "../src/lbm/common.cpp",
        "../src/grid_renderer.cpp",
        "../src/shader.cpp",
        "../src/glad.c"
    ]

    includes = [
        "../include",
        "../inline",
        "../imgui-master",
        "../imgui-master/backends"
    ]

    output_file = "main_profile"

    opencv_cflags = subprocess.check_output( 'pkg-config --cflags opencv4', shell = True ).decode( 'utf-8' ).strip()
    opencv_libs = " -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs"

    compile_command = f"g++ {gpp_args} -o {output_file} {' '.join(files)}"

    for include in includes:
        compile_command += f" -I{include}"

    compile_command += f" {opencv_cflags} {opencv_libs} -lGL -lglfw -ltbb"

    print(f"🛠️ Compiling with: {compile_command}")

    subprocess.run(compile_command, shell=True)

    print("✅Compilation complete.")

    # Run the compiled program
    run_command = f"./{output_file}"
    print(f"🚀 Running {run_command}...")
    subprocess.run(run_command, shell=True)

def compile_and_run_windows():
    print("🚀 Compiling program...")

    # Compiler flags
    gpp_args = ["-g", "-v", "-std=c++23", "-DGPU"]

    # Source files
    files = [
        "../imgui-master/imgui.cpp",
        "../imgui-master/imgui_draw.cpp",
        "../imgui-master/imgui_widgets.cpp",
        "../imgui-master/imgui_tables.cpp",
        "../imgui-master/backends/imgui_impl_opengl3.cpp",
        "../imgui-master/backends/imgui_impl_glfw.cpp",
        "main_profiled.cpp",
        "gl.cpp",
        "../src/lbm/common.cpp",
        "../src/grid_renderer.cpp",
        "../src/shader.cpp",
        "../src/glad.c"
    ]

    # Include directories
    includes = [
        "../include",
        "../inline",
        "../imgui-master",
        "../imgui-master/backends"
    ]

    # Output file name
    output_file = "main_profile.exe"

    # Get OpenCV include path using pkg-config
    opencv_include_path = subprocess.check_output(["pkg-config", "--cflags-only-I", "opencv4"]).decode().strip()

    # Build command
    compile_command = [
        "g++", *gpp_args, "-o", output_file
    ]
    
    # Add source files
    compile_command.extend(files)

    # Add include directories
    for include in includes:
        compile_command.append(f"-I{os.path.abspath(include)}")

    # Add OpenCV include path
    compile_command.append(opencv_include_path)

    dpcxx = "../dpcxx_dll"

    compile_command.append(f"-L{os.path.abspath(dpcxx)}")

    # Add required libraries
    compile_command.extend([
        "-lopengl32", "-lglfw3", "-lgdi32", "-ltbb12",
        "-lopencv_core", "-lopencv_imgproc", "-lopencv_highgui", "-lopencv_imgcodecs", "-lfs_dpcxx"
    ])

    # Print the command for debugging
    print(f"🛠️ Compiling with: {' '.join(map(str, compile_command))}")

    # Execute the build
    subprocess.run(compile_command)

    print("✅ Compilation complete.")

    run_command = f"./{output_file}"
    print(f"🚀 Running {run_command}...")
    subprocess.run(run_command, shell=True)

def plot_profiling_data(filename):
    # Read data from the file
    with open(filename, 'r') as file:
        lines = file.readlines()

    states = []
    times = []
    
    for line in lines:
        if line.strip():  # Skip empty lines
            parts = line.split(':')
            state = parts[0].strip()
            time = int(parts[1].strip())
            states.append(state)
            times.append(time)

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
    plt.savefig("profiling_data.png", bbox_inches='tight', dpi=300)  # Save with tight bounding box and high resolution
    plt.close()  # Close the plot to prevent it from displaying interactively

if __name__ == '__main__':
    insert_profiling( 'main.cpp', 'main_profiled.cpp' )

    compile_and_run_windows()

    plot_profiling_data( 'profile_output.txt' )