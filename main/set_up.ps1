[Environment]::CurrentDirectory = (Get-Location -PSProvider FileSystem).ProviderPath

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

function Download-mdspan {
    param (
        [string]$url = "https://github.com/kokkos/mdspan/archive/refs/heads/stable.zip",
        [string]$destination = "../include/mdspan-stable.zip"
    )

    $destinationFullPath = [System.IO.Path]::GetFullPath($destination)
    $destinationDir = [System.IO.Path]::GetDirectoryName($destinationFullPath)

    Write-Host "Downloading mdspan library to $destinationFullPath ..."
    try {
        Invoke-WebRequest -Uri $url -OutFile $destinationFullPath
        Write-Host "Successfully downloaded mdspan."
    } catch {
        Write-Host "Failed to download mdspan. Exiting."
        exit 1
    }

    Write-Host "Extracting mdspan..."
    try {
        Expand-Archive -Path $destinationFullPath -DestinationPath $destinationDir -Force
        Write-Host "Successfully extracted mdspan."
    } catch {
        Write-Host "Failed to extract mdspan. Exiting."
        exit 1
    }

    # Clean up the zip file
    Remove-Item $destination
}

function Download-GLM {
    param (
        [string]$url = "https://github.com/g-truc/glm/archive/refs/tags/0.9.9.8.zip",
        [string]$destination = "../include/glm-0.9.9.8.zip"
    )

    # Determine the full path for the destination zip file and destination directory (i.e. the include directory)
    $destinationFullPath = [System.IO.Path]::GetFullPath($destination)
    $destinationDir = [System.IO.Path]::GetDirectoryName($destinationFullPath)

    Write-Host "Downloading GLM library to $destinationFullPath ..."
    try {
        Invoke-WebRequest -Uri $url -OutFile $destinationFullPath
        Write-Host "Successfully downloaded GLM."
    } catch {
        Write-Host "Failed to download GLM. Exiting."
        exit 1
    }

    Write-Host "Extracting GLM..."
    try {
        Expand-Archive -Path $destinationFullPath -DestinationPath $destinationDir -Force
        Write-Host "Successfully extracted GLM."
    } catch {
        Write-Host "Failed to extract GLM. Exiting."
        exit 1
    }

    # Define the extracted folder name (based on the tag version)
    $extractedFolderName = "glm-0.9.9.8"
    $extractedFolderPath = Join-Path $destinationDir $extractedFolderName

    # Path to the 'glm' subdirectory within the extracted folder
    $glmSourcePath = Join-Path $extractedFolderPath "glm"
    # Destination for the glm directory in the include folder
    $includeDir = Join-Path (Get-Location).Path "../include"
    $glmDestinationPath = Join-Path $includeDir "glm"

    if (Test-Path $glmSourcePath) {
        Write-Host "Moving 'glm' directory to $includeDir ..."
        try {
            # Move the 'glm' folder from the extracted folder to the include directory
            Move-Item -Path $glmSourcePath -Destination $glmDestinationPath -Force
            Write-Host "Successfully moved 'glm' directory."
        } catch {
            Write-Host "Failed to move 'glm' directory. Exiting."
            exit 1
        }
    } else {
        Write-Host "Could not find the 'glm' directory in the extracted folder. Exiting."
        exit 1
    }

    # Clean up: remove the extracted folder and the zip file
    try {
        Remove-Item -Path $extractedFolderPath -Recurse -Force
        Remove-Item -Path $destinationFullPath -Force
        Write-Host "Cleaned up extracted files."
    } catch {
        Write-Host "Failed to clean up extracted files. Please remove them manually."
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
Download-mdspan
Download-GLM

Compile-Code -buildType $buildType