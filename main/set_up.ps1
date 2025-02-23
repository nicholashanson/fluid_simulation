function Install-Chocolatey {
    if (-not (Get-Command choco -ErrorAction SilentlyContinue)) {
        Write-Host "Chocolatey not found. Installing..."
        Set-ExecutionPolicy Bypass -Scope Process -Force;
        iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
    } else {
        Write-Host "Chocolatey is already installed."
    }
}


# Install necessary dependencies if not already installed
function Install-Dependencies {
    Write-Host "Ensuring dependencies are installed..."

    $dependencies = @(
        "mingw",
        "curl"
    )

    foreach ($dep in $dependencies) {
        if (-not (choco list --local-only | Select-String $dep)) {
            Write-Host "Installing $dep..."
            choco install $dep -y --force
        } else {
            Write-Host "$dep is already installed."
        }
    }

    $downloadDir = [System.IO.Path]::Combine((Get-Location).Path, "../include")
    if (-not (Test-Path $downloadDir)) {
        New-Item -ItemType Directory -Path $downloadDir
    }

    # Create the 'glfw' subdirectory
    $glfwDir = Join-Path $downloadDir "GLFW"
    if (-not (Test-Path $glfwDir)) {
        New-Item -ItemType Directory -Path $glfwDir
    }

    # Download glfw3.h from GLFW official repository
    $glfwUrl = "https://raw.githubusercontent.com/glfw/glfw/master/include/GLFW/glfw3.h"
    $glfwFileName = "glfw3.h"
    $glfwFilePath = Join-Path $glfwDir $glfwFileName

    Write-Host "Downloading $glfwFileName from GLFW official repository..."
    try {
        Invoke-WebRequest -Uri $glfwUrl -OutFile $glfwFilePath
        Write-Host "Successfully downloaded $glfwFileName."
    } catch {
        Write-Host "Failed to download $glfwFileName. Exiting."
        exit 1
    }
}

# Function to download the necessary files
function Download-Files {
    Write-Host "Downloading necessary files..."

    $files = @(
        "https://raw.githubusercontent.com/nicholashanson/sim/refs/heads/main/grid.hpp",
        "https://raw.githubusercontent.com/nicholashanson/performance_profiler/refs/heads/main/profiler.hpp",
        "https://raw.githubusercontent.com/nicholashanson/performance_profiler/refs/heads/main/profile_manager.hpp",
        "https://raw.githubusercontent.com/nicholashanson/performance_profiler/refs/heads/main/performance_profile.hpp",
        "https://raw.githubusercontent.com/nicholashanson/performance_profiler/refs/heads/main/generate_graph.hpp",
        "https://raw.githubusercontent.com/nicholashanson/performance_profiler/refs/heads/main/fixture.hpp",
        "https://raw.githubusercontent.com/nicholashanson/performance_profiler/refs/heads/main/average_time_profiler.hpp"
    )

    $downloadDir = [System.IO.Path]::Combine((Get-Location).Path, "../include")
    if (-not (Test-Path $downloadDir)) {
        New-Item -ItemType Directory -Path $downloadDir
    }

    foreach ($file in $files) {
        $fileName = [System.IO.Path]::GetFileName($file)
        $filePath = Join-Path $downloadDir $fileName

        Write-Host "Downloading $file..."
        try {
            Invoke-WebRequest -Uri $file -OutFile $filePath
            Write-Host "Successfully downloaded $fileName."
        } catch {
            Write-Host "Failed to download $fileName. Exiting."
            exit 1
        }
    }
}

function Compile-Code {
    param (
        [string]$buildType = "--db"
    )

    Write-Host "Compiling program..."

    $gppArgs = "-g -O0 -v -std=c++23"

    $files = @(
        "gl.cpp",
        "../src/lbm/common.cpp",
        "../src/grid_renderer.cpp",
        "../src/shader.cpp",
        "../src/glad.c"
    )

    $includes = @(
        "../include",
        "../inline"
    )

    # Output file name
    $outputFile = "fs.exe"

    # Build command
    $compileCommand = "g++ $gppArgs -o $outputFile " + ($files | ForEach-Object { $_ }) + " " + ($includes | ForEach-Object { "-I" + (Join-Path (Get-Location) $_) }) + " " + " -lfs_dpcxx -lopengl32 -lglfw3 -lgdi32 -ltbb12 -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs"

    # Execute the build
    Invoke-Expression $compileCommand

    Write-Host "Compilation complete."
}

Install-Chocolatey
Install-Dependencies
Download-Files

Compile-Code -buildType $buildType