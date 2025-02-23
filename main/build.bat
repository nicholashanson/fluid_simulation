@echo off

set PATH=C:\Program Files (x86)\Intel\oneAPI\tbb\2022.0\bin;%PATH%

set FILES="https://raw.githubusercontent.com/nicholashanson/sim/refs/heads/main/grid.hpp"
set FILES=%FILES% "https://raw.githubusercontent.com/nicholashanson/performance_profiler/refs/heads/main/profiler.hpp"
set FILES=%FILES% "https://raw.githubusercontent.com/nicholashanson/performance_profiler/refs/heads/main/profile_manager.hpp"
set FILES=%FILES% "https://raw.githubusercontent.com/nicholashanson/performance_profiler/refs/heads/main/performance_profile.hpp"
set FILES=%FILES% "https://raw.githubusercontent.com/nicholashanson/performance_profiler/refs/heads/main/generate_graph.hpp"
set FILES=%FILES% "https://raw.githubusercontent.com/nicholashanson/performance_profiler/refs/heads/main/fixture.hpp"
set FILES=%FILES% "https://raw.githubusercontent.com/nicholashanson/performance_profiler/refs/heads/main/average_time_profiler.hpp"

REM Loop through and download each file
for %%f in (%FILES%) do (
    echo Downloading %%f...
    curl -o "C:/Users/HP/Documents/fluid_simulation/include/%%~nxf" %%f
    if errorlevel 1 (
        echo Failed to download %%f. Please check the URL or your internet connection.
        pause
        exit /b 1
    ) else (
        echo Successfully downloaded %%f.
    )
)

REM Check the first command-line argument
if "%1"=="--profile" (

    echo Compiling profiles...
    g++ -g -v -std=c++23 -o fs_profile.exe ^
        "C:/Users/HP/Documents/fluid_simulation/performance_profiling/profile_collide_and_stream.cpp" ^
        "C:/Users/HP/Documents/fluid_simulation/main/gl.cpp" ^
        "C:/Users/HP/Documents/fluid_simulation/src/glad.c" ^
        "C:/Users/HP/Documents/fluid_simulation/src/grid_renderer.cpp" ^
        "C:/Users/HP/Documents/fluid_simulation/src/shader.cpp" ^
        "C:/Users/HP/Documents/fluid_simulation/src/lbm/common.cpp" ^
        "C:/imgui-master/imgui.cpp" ^
        "C:/imgui-master/imgui_draw.cpp" ^
        "C:/imgui-master/imgui_widgets.cpp" ^
        "C:/imgui-master/imgui_tables.cpp" ^
        "C:/imgui-master/backends/imgui_impl_opengl3.cpp" ^
        "C:/imgui-master/backends/imgui_impl_glfw.cpp" ^
        -I"C:/imgui-master" ^
        -I"C:/imgui-master/backends" ^
        -I"C:/Users/HP/Documents/fluid_simulation/include" ^
        -I"C:/Users/HP/Documents/fluid_simulation/inline" ^
        -L"C:/glfw-3.4.bin.WIN64/glfw-3.4.bin.WIN64/lib-mingw-w64" ^
        -L"C:/Users/HP/Documents/fluid_simulation/dpcxx_dll" ^
        -lfs_dpcxx -lopengl32 -lglfw3 -lgdi32 -ltbb12 -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs

) else if "%1"=="--test" (
    echo Compiling tests...
    g++ -g -v -std=c++23 -o fs_test.exe -DFS_TEST ^
        "C:/Users/HP/Documents/fluid_simulation/tests/test_collide_and_stream_equivalence.cpp" ^
        "C:/Users/HP/Documents/fluid_simulation/main/gl.cpp" ^
        "C:/Users/HP/Documents/fluid_simulation/src/lbm/common.cpp" ^
        "C:/Users/HP/Documents/fluid_simulation/src/glad.c" ^
        "C:/Users/HP/Documents/fluid_simulation/src/grid_renderer.cpp" ^
        "C:/Users/HP/Documents/fluid_simulation/src/shader.cpp" ^
        "C:/imgui-master/imgui.cpp" ^
        "C:/imgui-master/imgui_draw.cpp" ^
        "C:/imgui-master/imgui_widgets.cpp" ^
        "C:/imgui-master/imgui_tables.cpp" ^
        "C:/imgui-master/backends/imgui_impl_opengl3.cpp" ^
        "C:/imgui-master/backends/imgui_impl_glfw.cpp" ^
        -I"C:/imgui-master" ^
        -I"C:/imgui-master/backends" ^
        -I"C:/Users/HP/Documents/fluid_simulation/include" ^
        -I"C:/Users/HP/Documents/fluid_simulation/inline" ^
        -L"C:/glfw-3.4.bin.WIN64/glfw-3.4.bin.WIN64/lib-mingw-w64" ^
        -L"C:/Users/HP/Documents/fluid_simulation/dpcxx_dll" ^
        -lfs_dpcxx -lopengl32 -lglfw3 -lgdi32 -ltbb12 -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs -lgtest -lgtest_main
) else if "%1"=="--db" (
    REM Default compile command
    echo Compiling program...
    g++ -g -O0 -v -std=c++23 -o fs_db.exe ^
        "C:/Users/HP/Documents/fluid_simulation/main/gl.cpp" ^
        "C:/Users/HP/Documents/fluid_simulation/main/main_double_buffered.cpp" ^
        "C:/Users/HP/Documents/fluid_simulation/src/glad.c" ^
        "C:/Users/HP/Documents/fluid_simulation/src/grid_renderer.cpp" ^
        "C:/Users/HP/Documents/fluid_simulation/src/shader.cpp" ^
        "C:/Users/HP/Documents/fluid_simulation/src/lbm/common.cpp" ^
        "C:/imgui-master/imgui.cpp" ^
        "C:/imgui-master/imgui_draw.cpp" ^
        "C:/imgui-master/imgui_widgets.cpp" ^
        "C:/imgui-master/imgui_tables.cpp" ^
        "C:/imgui-master/backends/imgui_impl_opengl3.cpp" ^
        "C:/imgui-master/backends/imgui_impl_glfw.cpp" ^
        -I"C:/imgui-master" ^
        -I"C:/imgui-master/backends" ^
        -I"C:/Users/HP/Documents/fluid_simulation/include" ^
        -I"C:/Users/HP/Documents/fluid_simulation/inline" ^
        -L"C:/glfw-3.4.bin.WIN64/glfw-3.4.bin.WIN64/lib-mingw-w64" ^
        -L"C:/Users/HP/Documents/fluid_simulation/dpcxx_dll" ^
        -lfs_dpcxx -lopengl32 -lglfw3 -lgdi32 -ltbb12 -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs
    echo Compilation successful
) else (
    REM Default compile command
    echo Compiling program...
    g++ -g -O0 -v -std=c++23 -o fs.exe ^
        "C:/Users/HP/Documents/fluid_simulation/main/gl.cpp" ^
        "C:/Users/HP/Documents/fluid_simulation/main/main.cpp" ^
        "C:/Users/HP/Documents/fluid_simulation/src/glad.c" ^
        "C:/Users/HP/Documents/fluid_simulation/src/grid_renderer.cpp" ^
        "C:/Users/HP/Documents/fluid_simulation/src/shader.cpp" ^
        "C:/Users/HP/Documents/fluid_simulation/src/lbm/common.cpp" ^
        "C:/imgui-master/imgui.cpp" ^
        "C:/imgui-master/imgui_draw.cpp" ^
        "C:/imgui-master/imgui_widgets.cpp" ^
        "C:/imgui-master/imgui_tables.cpp" ^
        "C:/imgui-master/backends/imgui_impl_opengl3.cpp" ^
        "C:/imgui-master/backends/imgui_impl_glfw.cpp" ^
        -I"C:/imgui-master" ^
        -I"C:/imgui-master/backends" ^
        -I"C:/Users/HP/Documents/fluid_simulation/include" ^
        -I"C:/Users/HP/Documents/fluid_simulation/inline" ^
        -L"C:/glfw-3.4.bin.WIN64/glfw-3.4.bin.WIN64/lib-mingw-w64" ^
        -L"C:/Users/HP/Documents/fluid_simulation/dpcxx_dll" ^
        -lfs_dpcxx -lopengl32 -lglfw3 -lgdi32 -ltbb12 -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs

    copy fs.exe "C:/Users/HP/Documents/fluid_simulation/bin"
    echo Compilation successful
)

pause