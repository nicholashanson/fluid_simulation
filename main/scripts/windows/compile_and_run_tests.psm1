$compileVars = Join-Path $PSScriptRoot "compile_vars.psm1"
Import-Module $compileVars -Force

function Compile-And-Run-Tests {
    param(
        [switch]$GPU
    )

    Write-Host "Compiling tests..."

    $gppArgs = "-g -O0 -v -std=c++23"

    if ($GPU) {
        $gppArgs += " -DGPU"
    }

    $files = @(
        "../tests/test_collide_and_stream_equivalence.cpp",
        "../tests/test_vertex_data_equivalence.cpp",
        "../tests/test_stateful_collide_and_stream.cpp",
        "../tests/test_M_inverse.cpp",
        "../tests/test_get_lattice_triangles.cpp",
        "../tests/test_squared_distance_to_segment.cpp",
        "../tests/test_distance_to_polygon_single_segment.cpp",
        "../tests/test_geometry.cpp",
        "gl.cpp",
        "../src/lbm/common.cpp",
        "../src/grid_renderer.cpp",
        "../src/shader.cpp",
        "../src/glad.c",
        "../src/gui.cpp"
    )

    $files += $imGuiSrc

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

    Write-Output "OpenCV Libraries: $openCVLibs"

    # Build command
    $compileCommand = "g++ $gppArgs -o $outputFile " +
        ($files | ForEach-Object { $_ + " " }) +
        ($includes | ForEach-Object { "-I" + (Join-Path (Get-Location) $_) + " " }) +
        "$opencvIncludePath " +
        ($libs | ForEach-Object { "-L" + (Join-Path (Get-Location) $_) + " " }) +
        "-lopengl32 -lglfw3 -lgdi32 -ltbb12 " + 
        ($openCVLibs -join " ") + " " +  
        "-lgtest -lgtest_main" 

    if ($GPU) {
        $compileCommand += " -lfs_dpcxx"
    }

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