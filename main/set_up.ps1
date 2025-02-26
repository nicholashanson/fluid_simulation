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


function Install-Dependencies {
    Write-Host "Ensuring dependencies are installed..."

    # Check if 'curl' is already installed
    $curlAvailable = Get-Command curl -ErrorAction SilentlyContinue
    if ($curlAvailable) {
        Write-Host "curl is already available."
    } else {
        Write-Host "curl is not found. Installing..."
        choco install curl -y --force
    }

    # Check if 'g++' is already in the PATH
    $gppAvailable = Get-Command g++ -ErrorAction SilentlyContinue
    if ($gppAvailable) {
        Write-Host "g++ is already in the PATH."
    } else {
        Write-Host "g++ is not found in the PATH. Checking for MinGW installation..."

        # Check if MinGW is installed via Chocolatey
        $mingwInstalled = choco list --local-only | Select-String "mingw"
        
        if ($mingwInstalled) {
            Write-Host "MinGW is installed via Chocolatey."
        } else {
            Write-Host "MinGW is not found. Installing MinGW..."
            choco install mingw -y --force
        }

        # Check if MinGW's 'bin' directory exists at the expected location
        $mingwBinDir = "C:\ProgramData\mingw64\mingw64\bin"
        if (Test-Path $mingwBinDir) {
            Write-Host "MinGW's bin directory found at: $mingwBinDir"
            # Add MinGW's 'bin' directory to the PATH if it's not already there
            $env:Path += ";$mingwBinDir"
            Write-Host "Added MinGW's bin directory to the PATH."
        } else {
            Write-Host "MinGW's bin directory not found at $mingwBinDir. Something went wrong during installation."
            exit 1
        }
    }

    # Set up directories
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

# Function to install and configure OpenCV
function Install-OpenCV {
    Write-Host "Checking OpenCV installation..."

    # Check if OpenCV is installed via Chocolatey
    $opencvInstalled = choco list --local-only | Select-String "opencv"
    if ($opencvInstalled) {
        Write-Host "OpenCV is already installed."
    } else {
        Write-Host "OpenCV is not found. Installing..."
        choco install opencv -y --force
    }

    # Set OpenCV environment variables
    $opencvDir = "C:\tools\opencv\build"
    $opencvInclude = "$opencvDir\include"
    $opencvLib = "$opencvDir\x64\vc16\lib"

    if ( (Test-Path $opencvInclude) -and (Test-Path $opencvLib) ) {
        Write-Host "OpenCV directories found. Adding to environment variables..."
        [System.Environment]::SetEnvironmentVariable("OpenCV_DIR", $opencvDir, [System.EnvironmentVariableTarget]::Machine)
        [System.Environment]::SetEnvironmentVariable("PATH", "$env:Path;$opencvLib", [System.EnvironmentVariableTarget]::Machine)
    } else {
        Write-Host "Error: OpenCV directories not found. Installation may have failed."
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

    # If the destination exists, remove it
    if (Test-Path $glmDestinationPath) {
        Write-Host "Removing existing 'glm' directory at $glmDestinationPath..."
        try {
            Remove-Item -Path $glmDestinationPath -Recurse -Force
            Write-Host "Successfully removed existing 'glm' directory."
        } catch {
            Write-Host "Failed to remove existing 'glm' directory. Exiting."
            exit 1
        }
    }

    if (Test-Path $glmSourcePath) {
        Write-Host "Moving 'glm' directory to $includeDir ..."
        try {
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

function Install-DPCPP {
    $icpxCommand = 'icpx'
    $url = 's3://fluidsim/intel-dpcpp-cpp-compiler-2025.0.4.21_offline.exe'
    $localPath = 'C:\intel-dpcpp\intel-dpcpp-cpp-compiler-2025.0.4.21_offline.exe'

    # Check if the 'icpx' command exists
    if (Get-Command -Name $icpxCommand -ErrorAction SilentlyContinue) {
        Write-Host "DPC++ compiler is already installed. Skipping installation."
    } else {

        # Check if the installer file already exists
        if (Test-Path -Path $localPath) {
            Write-Host "The installer file already exists. Skipping download."
        } else {

            # Check if the directory exists; if not, create it
            $directory = [System.IO.Path]::GetDirectoryName($localPath)

            if (-not (Test-Path -Path $directory)) {
                Write-Host "Directory does not exist. Creating directory: $directory"
                New-Item -Path $directory -ItemType Directory -Force
            }

            Write-Host "Downloading DPC++ installer..."
            Invoke-WebRequest -Uri $url -OutFile $localPath
            Write-Host "Successfully downloaded DPC++ installer."
        }
    }

    # Silently install DPC++ using the installer
    Write-Host "Starting silent installation of Intel DPC++..."
    Start-Process -FilePath $localPath -ArgumentList '/s' -Wait -NoNewWindow
    Write-Host "Intel DPC++ installation completed."
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
    $compileCommand = "g++ $gppArgs -o $outputFile " + ($files | ForEach-Object { $_ }) + " " + ($includes | ForEach-Object { "-I" + (Join-Path (Get-Location) $_) }) + " -IC:\tools\opencv\build\include" + " -lfs_dpcxx -lopengl32 -lglfw3 -lgdi32 -ltbb12 -lopencv_world4110"

    # Execute the build
    Invoke-Expression $compileCommand

    Write-Host "Compilation complete."
}

Install-Chocolatey
Install-Dependencies
Install-OpenCV
Download-Files
Download-mdspan
Download-GLM

Install-DPCPP

Compile-Code -buildType $buildType