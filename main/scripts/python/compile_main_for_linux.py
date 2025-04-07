import os
import subprocess

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

    compile_command = f"g++ { gpp_args } -o { output_file } {' '.join( files ) }"

    for include in includes:
        compile_command += f" -I{ include }"

    compile_command += f" { opencv_cflags } { opencv_libs } -lGL -lglfw -ltbb"

    print( f"Compiling with: { compile_command }" )

    subprocess.run( compile_command, shell = True )

    print( "Compilation complete." )

    run_command = f"./{ output_file }"
    print( f"Running { run_command }..." )
    subprocess.run( run_command, shell = True )