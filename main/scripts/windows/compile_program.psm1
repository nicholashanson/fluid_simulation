Import-Module "./compile_vars.ps1"

function Compile-Program {

    Write-Host "Compiling program..."

    $gppArgs = "-g -O0 -v -std=c++23"

    $files = @(
        "main.cpp",
        "gl.cpp",
        "../src/lbm/common.cpp",
        "../src/grid_renderer.cpp",
        "../src/shader.cpp",
        "../src/glad.c"
    )

    $files += $imgui_src

    $includes = @(
        "../include",
        "../inline",
        "../imgui-master",
        "../imgui-master/backends"
    )

    # Output file name
    $outputFile = "fs.exe"

    # Get the OpenCV include path using pkg-config
    $opencvIncludePath = $(pkg-config --cflags-only-I opencv4)

    # Build command
    $compileCommand = "g++ $gppArgs -o $outputFile " +
        ($files | ForEach-Object { $_ }) + " " +
        ($includes | ForEach-Object { "-I" + (Join-Path (Get-Location) $_) }) + " " +
        "$opencvIncludePath " +  
        "-lopengl32 -lglfw3 -lgdi32 -ltbb12 -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs"

    # Print the command for debugging
    Write-Output "Compiling with: $compileCommand"

    # Execute the build
    Invoke-Expression $compileCommand

    Write-Host "Compilation complete."
}