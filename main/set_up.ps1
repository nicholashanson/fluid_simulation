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

function Install-Curl {
    Write-Host "Ensuring dependencies are installed..."

    # Check if 'curl' is already installed
    $curlAvailable = Get-Command curl -ErrorAction SilentlyContinue
    if ($curlAvailable) {
        Write-Host "curl is already available."
    } else {
        Write-Host "curl is not found. Installing..."
        choco install curl -y --force
    }
}

function Install-MSYS2 {
    # Check if MSYS2 is installed
    $msys2Path = "C:\msys64\usr\bin\bash.exe"
    if (Test-Path $msys2Path) {
        Write-Host "MSYS2 is already installed at $msys2Path."
    } else {
        Write-Host "MSYS2 is not installed. Installing MSYS2..."

        # Download and install MSYS2
        $msys2InstallerUrl = "https://repo.msys2.org/distrib/x86_64/msys2-base-x86_64-20250221.sfx.exe"
        $msys2InstallerPath = "C:\msys2-base-x86_64-20250221.sfx.exe"

        if (-not (Test-Path $msys2InstallerPath)) {
            Write-Host "Downloading MSYS2 installer..."
            Invoke-WebRequest -Uri $msys2InstallerUrl -OutFile $msys2InstallerPath
        }

        Write-Host "Running MSYS2 installer..."
        Start-Process -FilePath $msys2InstallerPath -Wait

        # Ensure MSYS2 is up to date
        Write-Host "Updating MSYS2..."
        & "C:\msys64\usr\bin\bash.exe" -c "pacman -Syu --noconfirm"

        # Install MinGW (64-bit) and base-devel package
        Write-Host "Installing MinGW and development tools..."
        & "C:\msys64\usr\bin\bash.exe" -c "pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-toolchain --noconfirm"

        Write-Host "MSYS2 and MinGW toolchain installed successfully."
    }

    # Check if g++ is available in the MSYS2 environment
    $gppAvailable = & "C:\msys64\usr\bin\bash.exe" -c "g++ --version"
    if ($gppAvailable) {
        Write-Host "g++ is available in MSYS2."
    } else {
        Write-Host "g++ is not available. Something went wrong with the MSYS2 installation."
        exit 1
    }

    # Add MSYS2's MinGW bin directory to the PATH
    $mingwBinDir = "C:\msys64\mingw64\bin"
    if (Test-Path $mingwBinDir) {
        Write-Host "MSYS2's MinGW bin directory found at: $mingwBinDir"
        # Add MinGW's 'bin' directory to the PATH if it's not already there
        if ($env:Path -notlike "*$mingwBinDir*") {
            $env:Path += ";$mingwBinDir"
            Write-Host "Added MSYS2's MinGW bin directory to the PATH."
        }
    } else {
        Write-Host "MSYS2's MinGW bin directory not found at $mingwBinDir. Something went wrong during installation."
        exit 1
    }
}

function MSYS2-Checks {
    # Define the MSYS2 g++ path (adjust this path based on your MSYS2 installation)
    $msys2GppPath = "C:\msys64\mingw64\bin\g++.exe"
    $msys2PacmanPath = "C:\msys64\usr\bin\pacman.exe"
    
    # Check if 'g++' is in the PATH
    $gppLocation = (Get-Command g++ -ErrorAction SilentlyContinue).Source

    # Check if 'g++' is not found
    if (-not $gppLocation) {
        Write-Host "'g++' not found in the PATH."
        Write-Host "Adding MSYS2 'g++' to the PATH."
        
        # Add MSYS2's 'g++' to the front of the PATH
        $env:PATH = "C:\msys64\mingw64\bin;" + $env:PATH
        Write-Host "MSYS2 'g++' added to the PATH."
    }
    elseif ($gppLocation -notlike "$msys2GppPath*") {
        Write-Host "'g++' found at: $gppLocation"
        Write-Host "This is not the MSYS2 'g++'. Adding MSYS2 'g++' to the front of the PATH."
        
        # Add MSYS2's 'g++' to the front of the PATH
        $env:PATH = "C:\msys64\mingw64\bin;" + $env:PATH
        Write-Host "MSYS2 'g++' added to the PATH."
    } else {
        Write-Host "MSYS2 'g++' is already in the PATH."
    }

    # Optionally, verify where g++ is after modification
    $finalGppLocation = (Get-Command g++).Source
    Write-Host "Final 'g++' location: $finalGppLocation"

    # Now check if pacman can be run from MSYS2 bash
    Write-Host "Checking if 'pacman' can be run inside MSYS2 bash..."
    
    try {
        # Attempt to run pacman using MSYS2's bash
        $pacmanOutput = & "C:\msys64\usr\bin\bash.exe" -c "pacman -V" 2>&1
        
        if ($pacmanOutput -match "pacman") {
            Write-Host "'pacman' is available inside MSYS2 bash."
        } else {
            Write-Host "'pacman' could not be run. Adding MSYS2 'pacman' to the PATH."
            
            # Add MSYS2's 'pacman' directory to the PATH if it's not already available
            $env:PATH = "C:\msys64\usr\bin;" + $env:PATH
            Write-Host "MSYS2 'pacman' added to the PATH."
        }
    }
    catch {
        Write-Host "'pacman' could not be run. Adding MSYS2 'pacman' to the PATH."
        # Add MSYS2's 'pacman' directory to the PATH if the command fails
        $env:PATH = "C:\msys64\usr\bin;" + $env:PATH
        Write-Host "MSYS2 'pacman' added to the PATH."
    }

    # Optionally, verify where pacman is after modification
    $finalPacmanLocation = (Get-Command pacman -ErrorAction SilentlyContinue).Source
    Write-Host "Final 'pacman' location: $finalPacmanLocation"
}

function Install-GLFW {
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

function Install-OpenCV {
    Write-Host "Checking OpenCV installation..."

    # Check if OpenCV is installed via MSYS2
    $opencvInstalled = & "C:\msys64\usr\bin\bash.exe" -c "pacman -Qs opencv"
    if ($opencvInstalled -match "opencv") {
        Write-Host "OpenCV is already installed via MSYS2."
    } else {
        Write-Host "OpenCV is not found. Installing OpenCV via MSYS2..."
        
        # Install OpenCV using pacman in MSYS2
        & "C:\msys64\usr\bin\bash.exe" -c "pacman -S mingw-w64-x86_64-opencv --noconfirm"
    }

    # Set OpenCV environment variables
    $opencvDir = "C:\msys64\mingw64"
    $opencvInclude = "$opencvDir\include"
    $opencvLib = "$opencvDir\lib"

    if ( (Test-Path $opencvInclude) -and (Test-Path $opencvLib) ) {
        Write-Host "OpenCV directories found. Adding to environment variables..."

        # Set OpenCV_DIR environment variable
        [System.Environment]::SetEnvironmentVariable("OpenCV_DIR", $opencvDir, [System.EnvironmentVariableTarget]::Machine)

        # Add OpenCV libraries to PATH
        $opencvBin = "$opencvDir\bin"
        if (-not ($env:PATH -contains $opencvBin)) {
            [System.Environment]::SetEnvironmentVariable("PATH", "$env:Path;$opencvBin", [System.EnvironmentVariableTarget]::Machine)
        }

        Write-Host "OpenCV environment variables set successfully."
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

function Install-VisualStudio {
    # Define the installer URL for VS Build Tools 2022
    $vsInstallerUrl = "https://aka.ms/vs/17/release/vs_BuildTools.exe"
    $vsInstallerPath = "C:\vs_BuildTools.exe"

    # Check if Visual Studio Build Tools 2022 is installed
    $vswherePath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

    if (Test-Path $vswherePath) {
        $installedVersion = & $vswherePath -latest -products * -requires Microsoft.VisualStudio.Workload.VCTools -property installationPath
        if ($installedVersion) {
            Write-Host "Visual Studio Build Tools 2022 is already installed at $installedVersion. Skipping installation."
            return
        }
    }

    Write-Host "VS Build Tools not found. Proceeding with installation."

    # Download the installer if not already present
    if (-not (Test-Path $vsInstallerPath)) {
        Write-Host "Downloading Visual Studio Build Tools installer..."
        Invoke-WebRequest -Uri $vsInstallerUrl -OutFile $vsInstallerPath
    }

    # Install only the minimal required components
    Write-Host "Installing Visual Studio Build Tools..."
    Start-Process -FilePath $vsInstallerPath -ArgumentList `
        "--quiet", `
        "--wait", `
        "--norestart", `
        "--nocache", `
        "--installPath C:\BuildTools", `
        "--add Microsoft.VisualStudio.Workload.VCTools", `
        "--add Microsoft.VisualStudio.Component.VC.Tools.x86.x64", `
        "--add Microsoft.VisualStudio.Component.Windows10SDK", `
        "--add Microsoft.VisualStudio.Component.CMake", `
        "--add Microsoft.VisualStudio.Component.VC.CoreIde", `
        "--add Microsoft.VisualStudio.Component.VC.Redist.14.Latest" `
        -NoNewWindow -Wait

    Write-Host "Installation completed."
}

function Install-DPCPP {
    $icpxCommand = 'icpx'
    $url = 'https://fluidsim.s3.us-east-1.amazonaws.com/intel-oneapi-base-toolkit-2025.0.1.47_offline.exe'
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
    Start-Process -FilePath $localPath -ArgumentList "--silent --eula accept" -Wait -NoNewWindow
    Write-Host "Intel DPC++ installation completed."

    # Add to the system PATH
    Write-Host "Adding DPC++ to the system PATH..."
    
    $currentPath = [System.Environment]::GetEnvironmentVariable("Path", [System.EnvironmentVariableTarget]::Machine)
    
    if ($currentPath -notlike "*$oneAPIPath*") {
        $newPath = "$currentPath;$oneAPIPath"
        [System.Environment]::SetEnvironmentVariable("Path", $newPath, [System.EnvironmentVariableTarget]::Machine)
        Write-Host "Successfully added $oneAPIPath to the system PATH."
    } else {
        Write-Host "DPC++ path is already in the system PATH."
    }

    # Add to the current session PATH (immediate effect)
    $env:Path += ";$oneAPIPath"
    Write-Host "DPC++ is now available in the current session."
}

function Compile-Code {

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
    $compileCommand = "g++ $gppArgs -o $outputFile " +
        ($files | ForEach-Object { $_ }) + " " +
        ($includes | ForEach-Object { "-I" + (Join-Path (Get-Location) $_) }) + " " +
        "-IC:\tools\opencv\build\include " +
        "-I`"C:\Program Files (x86)\Intel\oneAPI\tbb\2022.0\include`" " +
        "-lfs_dpcxx -lopengl32 -lglfw3 -lgdi32 -ltbb12 -lopencv_world4110"

    # Execute the build
    Invoke-Expression $compileCommand

    Write-Host "Compilation complete."
}

Install-Chocolatey
Install-Curl
Install-MSYS2
MSYS2-Checks
Install-OpenCV
Download-Files
Download-mdspan
Download-GLM

Install-VisualStudio
Install-DPCPP

Compile-Code