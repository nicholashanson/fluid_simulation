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

        # Install MinGW (64-bit) and development tools
        Write-Host "Installing MinGW and development tools..."
        & "C:\msys64\usr\bin\bash.exe" -c "pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-toolchain --noconfirm"

        Write-Host "MSYS2 and MinGW toolchain installed successfully."
    }

    # Check if MinGW64's g++ is available in the correct directory
    $mingwGppAvailable = Test-Path "C:\msys64\mingw64\bin\g++.exe"
    if ($mingwGppAvailable) {
        Write-Host "MinGW64's g++ is available in C:\msys64\mingw64\bin."
    } else {
        Write-Host "MinGW64's g++ is not available in C:\msys64\mingw64\bin. Installing the toolchain..."
        
        # Install MinGW64's toolchain (if not already done)
        & "C:\msys64\usr\bin\bash.exe" -c "pacman -S mingw-w64-x86_64-gcc --noconfirm"
        Write-Host "MinGW64 g++ installed."
    }

    # Add MinGW64's bin directory to the PATH if it's not already there
    $mingwBinDir = "C:\msys64\mingw64\bin"
    if (Test-Path $mingwBinDir) {
        Write-Host "MSYS2's MinGW64 bin directory found at: $mingwBinDir"
        # Add MinGW64's 'bin' directory to the PATH if it's not already there
        if ($env:Path -notlike "*$mingwBinDir*") {
            $env:Path += ";$mingwBinDir"
            Write-Host "Added MSYS2's MinGW64 bin directory to the PATH."
        }
    } else {
        Write-Host "MSYS2's MinGW64 bin directory not found at $mingwBinDir. Something went wrong during installation."
        exit 1
    }

    # Verify if the g++ in the MinGW64 bin is working properly
    $gppVersion = & "C:\msys64\mingw64\bin\g++" --version
    if ($gppVersion) {
        Write-Host "g++ version: $gppVersion"
    } else {
        Write-Host "Failed to retrieve g++ version. Something went wrong with the installation."
        exit 1
    }
}

function MSYS2-Checks {
    # Define expected paths
    $msys2Base = "C:\msys64"
    $msys2MingwBin = "$msys2Base\mingw64\bin"
    $msys2UsrBin = "$msys2Base\usr\bin"
    $msys2Lib = "$msys2Base\mingw64\lib"

    # Check if 'g++' is in the PATH
    $gppLocation = (Get-Command g++ -ErrorAction SilentlyContinue).Source

    if (-not $gppLocation -or $gppLocation -notlike "$msys2MingwBin*") {
        Write-Host "'g++' not found or not using MSYS2. Adding MSYS2 'g++' to the PATH."
        $env:PATH = "$msys2MingwBin;$env:PATH"
    } else {
        Write-Host "MSYS2 'g++' is already in the PATH."
    }

    # Verify 'g++' update
    $finalGppLocation = (Get-Command g++ -ErrorAction SilentlyContinue).Source
    Write-Host "Final 'g++' location: $finalGppLocation"

    # Check if 'pacman' can be run inside MSYS2 bash
    Write-Host "Checking if 'pacman' can be run inside MSYS2 bash..."
    try {
        $pacmanOutput = & "$msys2UsrBin\bash.exe" -c "pacman -V" 2>&1
        if ($pacmanOutput -match "pacman") {
            Write-Host "'pacman' is available inside MSYS2 bash."
        } else {
            Write-Host "'pacman' not found in bash. Adding MSYS2 to PATH."
            $env:PATH = "$msys2UsrBin;$env:PATH"
        }
    } catch {
        Write-Host "Error running 'pacman'. Adding MSYS2 to PATH."
        $env:PATH = "$msys2UsrBin;$env:PATH"
    }

    # Verify 'pacman' update
    $finalPacmanLocation = (Get-Command pacman -ErrorAction SilentlyContinue).Source
    Write-Host "Final 'pacman' location: $finalPacmanLocation"

    # Check if LIBRARY_PATH contains MSYS2 lib directory
    Write-Host "Checking LIBRARY_PATH..."
    $libraryPath = [System.Environment]::GetEnvironmentVariable("LIBRARY_PATH", "Machine")

    if ($libraryPath -notlike "*$msys2Lib*") {
        Write-Host "Adding MSYS2 lib directory to LIBRARY_PATH."
        [System.Environment]::SetEnvironmentVariable("LIBRARY_PATH", "$msys2Lib;$libraryPath", "Machine")
    } else {
        Write-Host "MSYS2 lib directory is already in LIBRARY_PATH."
    }

    # Verify LIBRARY_PATH update
    $finalLibraryPath = [System.Environment]::GetEnvironmentVariable("LIBRARY_PATH", "Machine")
    Write-Host "Final LIBRARY_PATH: $finalLibraryPath"
}

function Install-OpenCV {

    $env:PATH = "C:\msys64\usr\bin;C:\msys64\bin;" + $env:PATH

    Write-Host "Checking OpenCV installation..."

    # Check if OpenCV is installed via MSYS2
    $opencvInstalled = & "C:\msys64\usr\bin\bash.exe" -c "pacman -Qs opencv"
    if ($opencvInstalled -match "opencv") {
        Write-Host "OpenCV is already installed via MSYS2."
    } else {
        Write-Host "OpenCV is not found. Installing OpenCV via MSYS2..."

        # Initialize pacman keyring if needed
        $keyringCheck = & "C:\msys64\usr\bin\bash.exe" -c "pacman-key --init"
        if ($keyringCheck -match "error") {
            Write-Host "Initializing pacman keyring..."
            & "C:\msys64\usr\bin\bash.exe" -c "pacman-key --init"
        }

        Write-Host "Populating pacman keyring..."
        & "C:\msys64\usr\bin\bash.exe" -c "pacman-key --populate msys2"
        
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

    # Check if GLFW 3 is installed using pacman
    Write-Host "Checking if GLFW 3 is installed..."

    # Run pacman to check if glfw is installed
    $glfwInstalled = & "C:\msys64\usr\bin\bash.exe" -c "pacman -Qs glfw"
    
    if ($glfwInstalled -match "mingw-w64-x86_64-glfw") {
        Write-Host "GLFW 3 is already installed."
    } else {
        Write-Host "GLFW 3 is not found. Installing GLFW 3..."

        # Install GLFW 3 using pacman in MSYS2
        & "C:\msys64\usr\bin\bash.exe" -c "pacman -S mingw-w64-x86_64-glfw --noconfirm"
    }

    # Ensure the correct path is set for linking with GLFW
    $mingwBinPath = "C:\msys64\mingw64\bin"
    if (-not ($env:PATH -contains $mingwBinPath)) {
        Write-Host "Adding MSYS2 MinGW bin directory to PATH."
        $env:PATH = "$mingwBinPath;$env:PATH"
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

    # Define the include directory path where 'glm' should be located
    $includeDir = Join-Path (Get-Location).Path "../include"
    $glmDestinationPath = Join-Path $includeDir "glm"

    # Check if GLM directory already exists in the include folder
    if (Test-Path $glmDestinationPath) {
        Write-Host "'glm' directory already exists in $includeDir. Skipping GLM download and installation."
        return
    }

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
    $oneAPIPath = 'C:\Program Files (x86)\Intel\oneAPI\2025.0\bin'

    # Check if the 'icpx' command exists in the PATH
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

        # Silently install DPC++ using the installer
        Write-Host "Starting silent installation of Intel DPC++..."
        Start-Process -FilePath $localPath -ArgumentList "--silent --eula accept" -Wait -NoNewWindow
        Write-Host "Intel DPC++ installation completed."
    }

    # Check if the oneAPI directory is in the system PATH
    $currentPath = [System.Environment]::GetEnvironmentVariable("Path", [System.EnvironmentVariableTarget]::Machine)

    # If the DPC++ bin directory is not in the PATH, add it
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

    # Final check to see if "icpx" is runnable after installation and PATH modification
    $icpxAvailable = Get-Command -Name $icpxCommand -ErrorAction SilentlyContinue
    if ($icpxAvailable) {
        Write-Host "'icpx' command is now available and runnable."
    } else {
        Write-Host "'icpx' command is still not available. Please check the installation."
    }
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
        "-L`"C:\Program Files (x86)\Intel\oneAPI\tbb\2022.0\lib`" " +
        "-lopengl32 -lglfw3 -lgdi32 -ltbb12 -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs"

    # Execute the build
    Invoke-Expression $compileCommand

    Write-Host "Compilation complete."
}

Install-Chocolatey
Install-Curl
Install-MSYS2
MSYS2-Checks
Install-OpenCV
Install-GLFW
Download-Files
Download-mdspan
Download-GLM

Install-VisualStudio
Install-DPCPP

Compile-Code