#
# take main.cpp and create a new file called main_profiled.cpp
# that is copy of main.cpp with code for profiling added in.
#

# for compiling and running main_profiled.cpp
import subprocess
import os
import itertools

def insert_profiling( object_path, target_path ):

    profile_steps = 10
    
    with open( object_path, 'r', encoding = 'utf-8' ) as file:
        original_content = file.readlines()

    sections = [ 'imgui set up', 
                 'grid buffer set up', 
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
    gpp_args = ["-g", "-v", "-std=c++23"]

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

    # Add required libraries
    compile_command.extend([
        "-lopengl32", "-lglfw3", "-lgdi32", "-ltbb12",
        "-lopencv_core", "-lopencv_imgproc", "-lopencv_highgui", "-lopencv_imgcodecs"
    ])

    # Print the command for debugging
    print(f"🛠️ Compiling with: {' '.join(map(str, compile_command))}")

    # Execute the build
    subprocess.run(compile_command)

    print("✅ Compilation complete.")

if __name__ == '__main__':
    insert_profiling( 'main.cpp', 'main_profiled.cpp' )

    compile_and_run_windows()

        