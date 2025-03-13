function Compile-And-Run-Tests {

    Write-Host "Compiling tests..."

    $gppArgs = "-g -O0 -v -std=c++23 -DGPU"

    $files = @(
        "../imgui-master/imgui.cpp",
        "../imgui-master/imgui_draw.cpp",
        "../imgui-master/imgui_widgets.cpp",
        "../imgui-master/imgui_tables.cpp",
        "../imgui-master/backends/imgui_impl_opengl3.cpp",
        "../imgui-master/backends/imgui_impl_glfw.cpp",
        "../tests/test_collide_and_stream_equivalence.cpp",
        "../tests/test_vertex_data_equivalence.cpp",
        "gl.cpp",
        "../src/lbm/common.cpp",
        "../src/grid_renderer.cpp",
        "../src/shader.cpp",
        "../src/glad.c"
    )

    $includes = @(
        "../include",
        "../inline",
        "../imgui-master",
        "../imgui-master/backends"
    )

    $libs = @(
        "../lib/g++_libs"
    )

    # Output file name
    $outputFile = "fs_test.exe"

    # Get the OpenCV include path using pkg-config (no need to modify)
    $opencvIncludePath = $(pkg-config --cflags-only-I opencv4)

    # Build command
    $compileCommand = "g++ $gppArgs -o $outputFile " +
        ($files | ForEach-Object { $_ + " " }) +
        ($includes | ForEach-Object { "-I" + (Join-Path (Get-Location) $_) + " " }) +
        "$opencvIncludePath " +
        ($libs | ForEach-Object { "-L" + (Join-Path (Get-Location) $_) + " " }) +
        "-lopengl32 -lglfw3 -lgdi32 -ltbb12 -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs -lfs_dpcxx -lgtest -lgtest_main" 

    # Print the command for debugging
    Write-Output "Compiling with: $compileCommand"

    # Execute the build
    Invoke-Expression $compileCommand

    Write-Host "Compilation complete."

    Write-Host "Running fs_test.exe..."
    & .\fs_test.exe

    # Check the exit code of the test executable.
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Tests failed with exit code $LASTEXITCODE. Terminating script."
        exit $LASTEXITCODE
    } else {
        Write-Host "All tests passed."
    }
}