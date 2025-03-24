$compileVars = Join-Path $PSScriptRoot "compile_vars.psm1"
Import-Module $compileVars -Force

function Compile-Program {
    param(
        [switch]$GPU,
        [switch]$PAR,
        [switch]$ThreeD
    )

    Write-Host "Compiling program..."

    $gppArgs = "-g -O0 -v -std=c++23"

    if ($GPU) {
        $gppArgs += " -DGPU"
    }

    $files = @(
        "gl.cpp",
        "../src/lbm/common.cpp",
        "../src/grid_renderer.cpp",
        "../src/shader.cpp",
        "../src/glad.c",
        "../src/gui.cpp"
    )

    if ($PAR) {
        $files += "main_parallel.cpp"
    } elseif ($ThreeD) {
        $files += "main_3D.cpp"
    } else {
        $files += "main.cpp"
    }

    $files += $imGuiSrc

    $includes = @(
        "../include",
        "../inline",
        "../imgui-master",
        "../imgui-master/backends"
    )

    if ($GPU) {
        $outputFile = "fs_gpu.exe"
    } elseif ($ThreeD) {
        $outputFile = "fs_3D.exe"
    } else {
        $outputFile = "fs.exe"
    }

    # Get the OpenCV include path using pkg-config
    $opencvIncludePath = $(pkg-config --cflags-only-I opencv4)

    # Build command
    $compileCommand = "g++ $gppArgs -o $outputFile " +
        ($files -join " ") + " " +
        ($includes | ForEach-Object { "-I" + (Join-Path (Get-Location) $_) }) + " " +
        "$opencvIncludePath " +
        ($openCVLibs -join " ") + " " +  
        "-lopengl32 -lglfw3 -lgdi32 -ltbb12"

    if ($GPU) {
        $compileCommand += " -lfs_dpcxx"
    }

    # Print the command for debugging
    Write-Output "Compiling with: $compileCommand"

    # Execute the build
    Invoke-Expression $compileCommand

    Write-Host "Compilation complete."
}