param(
    [string]$Action,
    [switch]$GPU
)

Import-Module ".\scripts\windows\compile_program.psm1" -Force
Import-Module ".\scripts\windows\compile_and_run_tests.psm1" -Force
Import-Module ".\scripts\windows\global_vars.psm1" -Force 
Import-Module ".\scripts\windows\setup_googletest.psm1" -Force

[Environment]::CurrentDirectory = (Get-Location -PSProvider FileSystem).ProviderPath

# Function to add a path to an environment variable if it's not already present
function Add-PathToEnvVar {
    param (
        [string]$envVarName,
        [string]$pathToAdd
    )
    $currentValue = [System.Environment]::GetEnvironmentVariable($envVarName, [System.EnvironmentVariableTarget]::User)

    if (-not $currentValue) {
        $currentValue = ""
    }
    if ($currentValue -notmatch [regex]::Escape($pathToAdd)) {
        $newValue = if ($currentValue) { "$currentValue;$pathToAdd" } else { $pathToAdd }
        [System.Environment]::SetEnvironmentVariable($envVarName, $newValue, [System.EnvironmentVariableTarget]::User)
        Write-Host "Added $pathToAdd to $envVarName."
    }
}

function Setup-VSEnvironment {
    param (
        [string]$ucrtPath = "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\ucrt",
        [string]$umPath = "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\um",
        [string]$sharedPath = "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared",
        [string]$stlPath = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.43.34808\include",
        [string]$libPath = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.43.34808\lib\x64"
    )

    # Add paths to INCLUDE
    Add-PathToEnvVar -envVarName "INCLUDE" -pathToAdd $ucrtPath
    Add-PathToEnvVar -envVarName "INCLUDE" -pathToAdd $umPath
    Add-PathToEnvVar -envVarName "INCLUDE" -pathToAdd $sharedPath
    Add-PathToEnvVar -envVarName "INCLUDE" -pathToAdd $stlPath

    # Add paths to LIB
    Add-PathToEnvVar -envVarName "LIB" -pathToAdd $libPath
}

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
    # Save the original directory
    $originalDir = Get-Location

    Set-Location -Path "C:\"

    if ($Action -eq "reset") {
        Write-Host "Resetting MSYS2 installation..."

        # Remove MSYS2 directory
        $msys2Path = "C:\msys64"
        if (Test-Path $msys2Path) {
            Write-Host "Removing MSYS2 from $msys2Path..."
            Remove-Item -Recurse -Force $msys2Path
        } else {
            Write-Host "MSYS2 not found at $msys2Path."
        }

        # Remove the MSYS2 installer if it exists
        $msys2InstallerPath = "C:\msys2-base-x86_64-20250221.sfx.exe"
        if (Test-Path $msys2InstallerPath) {
            Write-Host "Deleting MSYS2 installer..."
            Remove-Item $msys2InstallerPath
        } else {
            Write-Host "MSYS2 installer not found."
        }

        # Remove MSYS2-related paths from the PATH environment variable
        Write-Host "Removing MSYS2-related paths from environment variables..."
        $mingwPath = "C:\msys64\mingw64\bin"
        $env:Path = $env:Path -replace [regex]::Escape($mingwPath), ""

        Write-Host "MSYS2 has been reset successfully."
    }

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

        # Initialize pacman keyring and populate with the default MSYS2 keys
        Write-Host "Initializing pacman keyring..."
        & "C:\msys64\usr\bin\bash.exe" -c "pacman-key --init"
        Write-Host "Populating pacman keyring..."
        & "C:\msys64\usr\bin\bash.exe" -c "pacman-key --populate msys2"

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
        # Prevent duplicate entries in PATH
        if (-not ($env:Path -split ";" -contains $mingwBinDir)) {
            $env:Path += ";$mingwBinDir"
            Write-Host "Added MSYS2's MinGW64 bin directory to the PATH."
        } else {
            Write-Host "MSYS2's MinGW64 bin directory is already in the PATH."
        }
    }

    # Verify if the g++ in the MinGW64 bin is working properly
    $gppVersion = & "C:\msys64\mingw64\bin\g++" --version
    if ($gppVersion) {
        Write-Host "g++ version: $gppVersion"
    } else {
        Write-Host "Failed to retrieve g++ version. Something went wrong with the installation."
        exit 1
    }

    # Restore the original directory
    Set-Location -Path $originalDir
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

    $msysUsr = "C:\msys64\usr\bin"
    if (Test-Path $msysUsr) {
        Write-Host "MSYS2's MinGW64 bin directory found at: $msysUsr"
        # Prevent duplicate entries in PATH
        if (-not ($env:Path -split ";" -contains $msysUsr)) {
            $env:Path += ";$msysUsr"
            Write-Host "Added MSYS2's MinGW64 bin directory to the PATH."
        } else {
            Write-Host "MSYS2's MinGW64 bin directory is already in the PATH."
        }
    }

    $msysBin = "C:\msys64\bin"
    if (Test-Path $msysBin) {
        Write-Host "MSYS2's MinGW64 bin directory found at: $msysBin"
        # Prevent duplicate entries in PATH
        if (-not ($env:Path -split ";" -contains $msysBin)) {
            $env:Path += ";$msysBin"
            Write-Host "Added MSYS2's MinGW64 bin directory to the PATH."
        } else {
            Write-Host "MSYS2's MinGW64 bin directory is already in the PATH."
        }
    }

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
}

function Install-GLFW {

    # Create the 'glfw' subdirectory
    $glfwDir = Join-Path $includeDir "GLFW"
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

    foreach ($file in $files) {
        $fileName = [System.IO.Path]::GetFileName($file)
        $filePath = Join-Path $includeDir $fileName

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

    $mdSpanDestinationPath = Join-Path $includeDir "mdspan-stable"

    if (Test-Path $mdSpanDestinationPath) {
        Write-Host "'mdspan-stable' directory already exists in $includeDir. Skipping mdspan download and installation."
        return
    }

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

function Install-Handle {
    Write-Host "Downloading Handle tool..."
    
    # Correct URL for Handle tool
    $handleUrl = "https://download.sysinternals.com/files/Handle.zip"
    $handleZip = "C:\Handle.zip"
    $handleExtractPath = "C:\Sysinternals"

    try {
        # Download the Handle tool
        Invoke-WebRequest -Uri $handleUrl -OutFile $handleZip -ErrorAction Stop

        # Extract the Handle tool
        Write-Host "Extracting Handle.zip..."
        Expand-Archive -Path $handleZip -DestinationPath $handleExtractPath -Force

        Write-Host "Handle tool installed to $handleExtractPath"
    } catch {
        Write-Host "Error occurred while downloading or extracting the Handle tool: $_"
    }
}

function Install-VisualStudio {
    $originalDir = Get-Location
    Set-Location -Path "C:\"

    # Define paths
    $vsInstallerPath = "C:\vs_installer.exe"
    $vsUninstallerPath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vs_installer.exe"
    $vcVarsAllPath = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
    $buildToolsPath = "C:\BuildTools"
    
    # Ensure log file directory exists
    if (-not (Test-Path "C:\Logs")) {
        New-Item -Path "C:\" -Name "Logs" -ItemType Directory
    }

    # Function to set environment variables
    function Set-EnvironmentVariables {
        # Update paths based on your installation directory
        $env:PATH += ";C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.43.34808\bin\Hostx64\x64"
        $env:INCLUDE += ";C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.43.34808\include"
        $env:LIB += ";C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.43.34808\lib\x64"
    }

    # Check for full Visual Studio installation in default path
    if (Test-Path "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv.exe") {
        $installedVersion = "C:\Program Files\Microsoft Visual Studio\2022\Community"
        Write-Host "Full Visual Studio installation found at $installedVersion. Checking for MSVC..."
        
        # Check if MSVC is installed in full Visual Studio installation
        $msvcPath = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC"
        if (-not (Test-Path $msvcPath)) {
            Write-Host "MSVC is missing. Installing MSVC tools..."
            Start-Process -FilePath $vsUninstallerPath -ArgumentList "modify --quiet --norestart --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64" -NoNewWindow -Wait
            Write-Host "MSVC tools installed."
        }
        else {
            Write-Host "MSVC is already installed. Repairing MSVC tools..."
            Start-Process -FilePath $vsUninstallerPath -ArgumentList "modify --installPath `"$installedVersion`" --quiet --norestart --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64" -NoNewWindow -Wait
            Write-Host "MSVC tools repaired."
        }

        # Set environment variables
        Set-EnvironmentVariables

        # Proceed with the compilation test
    }
    # Check for Visual Studio Build Tools
    elseif (Test-Path $buildToolsPath) {
        Write-Host "Visual Studio Build Tools installation found at $buildToolsPath. Checking for MSVC..."
        
        # Check for MSVC in Build Tools
        $msvcPath = Join-Path $buildToolsPath "Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC"
        if (-not (Test-Path $msvcPath)) {
            Write-Host "MSVC is missing in Build Tools. Installing MSVC tools..."
            Start-Process -FilePath $vsUninstallerPath -ArgumentList "modify --quiet --norestart --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64" -NoNewWindow -Wait
            Write-Host "MSVC tools installed in Build Tools."
        }
        else {
            Write-Host "MSVC is already installed in Build Tools. Repairing MSVC tools..."
            Start-Process -FilePath $vsUninstallerPath -ArgumentList "modify --installPath `"$installedVersion`" --quiet --norestart --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64" -NoNewWindow -Wait
            Write-Host "MSVC tools repaired in Build Tools."
        }

        # Set environment variables
        Set-EnvironmentVariables
    }
    else {
        Write-Host "No Visual Studio or Build Tools installation found. Proceeding with Build Tools installation."

        # Download the installer if not already present
        if (-not (Test-Path $vsInstallerPath)) {
            Write-Host "Downloading Visual Studio Build Tools installer..."
            Invoke-WebRequest -Uri "https://aka.ms/vs/17/release/vs_BuildTools.exe" -OutFile $vsInstallerPath
        }

        # Install Visual Studio Build Tools (including MSVC)
        Write-Host "Installing Visual Studio Build Tools..."
        $arguments = '--passive --norestart --force --installPath "C:\BuildTools" --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64'

        # Start the installer and wait for completion
        $process = Start-Process -FilePath $vsInstallerPath -ArgumentList $arguments -NoNewWindow -PassThru -Wait
        Write-Host "Waiting for installation to complete..."
        $process.WaitForExit()

        # Check the exit code and log any relevant errors
        if ($process.ExitCode -eq 0) {
            Write-Host "Installation completed successfully."
        } else {
            Write-Host "Installation failed with exit code $($process.ExitCode)."
        }

        # Set environment variables after installation
        Set-EnvironmentVariables
    }

    # Optional: Run vcvarsall.bat to ensure the environment is fully configured
    if (Test-Path $vcVarsAllPath) {
        Write-Host "Running vcvarsall.bat to finalize the environment setup..."
        & $vcVarsAllPath x64
    } else {
        Write-Host "Error: vcvarsall.bat not found. Visual Studio or Build Tools may not be fully installed."
    }

    # Create a simple Hello World program to test compilation
    $cppSource = @"
#include <iostream>

int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
"@
    $cppFilePath = "C:\temp\test.cpp"
    $exeFilePath = "C:\temp\test.exe"

    # Save the C++ source to a file
    $cppSource | Out-File -FilePath $cppFilePath

    Write-Host "Compiling test C++ program..."

    # Determine correct compiler path based on Visual Studio or Build Tools
    $clExePath = ""
    if (Test-Path "C:\Program Files\Microsoft Visual Studio\2022\Community") {
        # Visual Studio full installation
        $clExePath = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.43.34808\bin\Hostx64\x64\cl.exe"
    }
    elseif (Test-Path $buildToolsPath) {
        # Visual Studio Build Tools
        $clExePath = "C:\BuildTools\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.43.34808\bin\Hostx64\x64\cl.exe"
    }

    # Compile the C++ program using cl.exe and capture both standard output and error output
    $compileOutput = & $clExePath $cppFilePath 2>&1

    # Check if compilation was successful by looking for the expected output (e.g., .exe file creation)
    if (Test-Path $exeFilePath) {
        Write-Host "Compilation successful. Running the program..."
        & $exeFilePath
    } else {
        Write-Host "Compilation failed. Output:"
        Write-Host $compileOutput
    }

    # Restore the original directory
    Set-Location -Path $originalDir
}

function Install-DPCPP {
    $icpxCommand = 'icpx'
    $url = 'https://fluidsim.s3.us-east-1.amazonaws.com/intel-dpcpp-cpp-compiler-2025.0.4.21_offline.exe'
    $localPath = 'C:\intel-dpcpp\intel-dpcpp-cpp-compiler-2025.0.4.21_offline.exe'
    $oneAPIPath = 'C:\Program Files (x86)\Intel\oneAPI\compiler\2025.0\bin'

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

function Install-TBBINTEL {
    $tbbCommand = 'tbbmalloc.dll'
    $url = 'https://fluidsim.s3.us-east-1.amazonaws.com/intel-onetbb-2022.0.0.397_offline.exe'
    $localPath = 'C:\intel-tbb\intel-onetbb-2022.0.0.397_offline.exe'
    $tbbPath = 'C:\Program Files (x86)\Intel\oneAPI\tbb\latest\bin'

    # Check if TBB is already installed
    if (Test-Path "$tbbPath\$tbbCommand") {
        Write-Host "Intel TBB is already installed. Skipping installation."
    } else {
        # Check if the installer file exists
        if (Test-Path -Path $localPath) {
            Write-Host "The installer file already exists. Skipping download."
        } else {
            # Ensure the directory exists
            $directory = [System.IO.Path]::GetDirectoryName($localPath)

            if (-not (Test-Path -Path $directory)) {
                Write-Host "Creating directory: $directory"
                New-Item -Path $directory -ItemType Directory -Force
            }

            Write-Host "Downloading Intel TBB installer..."
            Invoke-WebRequest -Uri $url -OutFile $localPath
            Write-Host "Successfully downloaded Intel TBB installer."
        }

        # Silently install Intel TBB
        Write-Host "Starting silent installation of Intel TBB..."
        Start-Process -FilePath $localPath -ArgumentList "--silent --eula accept" -Wait -NoNewWindow
        Write-Host "Intel TBB installation completed."
    }

    # Update system PATH if TBB is not already included
    $currentPath = [System.Environment]::GetEnvironmentVariable("Path", [System.EnvironmentVariableTarget]::Machine)

    if ($currentPath -notlike "*$tbbPath*") {
        $newPath = "$currentPath;$tbbPath"
        [System.Environment]::SetEnvironmentVariable("Path", $newPath, [System.EnvironmentVariableTarget]::Machine)
        Write-Host "Successfully added $tbbPath to the system PATH."
    } else {
        Write-Host "Intel TBB path is already in the system PATH."
    }

    # Add to current session PATH (immediate effect)
    $env:Path += ";$tbbPath"
    Write-Host "Intel TBB is now available in the current session."

    # Final check to see if TBB is available
    if (Test-Path "$tbbPath\$tbbCommand") {
        Write-Host "Intel TBB is successfully installed and available."
    } else {
        Write-Host "Intel TBB is still not available. Please check the installation."
    }
}

function Install-ImGui {
    # Get the parent directory path
    $parentDirectory = (Get-Item ..).FullName

    # Define the path for the zip file to download
    $imguiZipFile = "$parentDirectory\imgui-master.zip"
    $imguiRepoZipUrl = "https://github.com/ocornut/imgui/archive/refs/heads/master.zip" 
    
    # Check if imgui-master exists in the parent directory
    if (Test-Path "$parentDirectory\imgui-master") {
        Write-Host "imgui-master already exists in the parent directory. Skipping installation."
    } else {
        Write-Host "imgui-master not found. Installing ImGui..."

        # Check if the zip file already exists in the parent directory
        if (-not (Test-Path $imguiZipFile)) {
            Write-Host "Downloading imgui-master.zip from GitHub..."
            Invoke-WebRequest -Uri $imguiRepoZipUrl -OutFile $imguiZipFile
            Write-Host "Download completed."
        }

        # Unzip the contents into the parent directory
        Expand-Archive -Path $imguiZipFile -DestinationPath $parentDirectory -Force
        Write-Host "Unzipped imgui-master.zip to the parent directory."

        # Remove the zip file after extraction
        Remove-Item -Path $imguiZipFile -Force
        Write-Host "Removed the downloaded zip file."

        Write-Host "ImGui installation completed."
    }
}

function Download-Googletest {
    param (
        [string]$url = "https://github.com/google/googletest/archive/refs/heads/main.zip",
        [string]$destination = "../include/googletest-main.zip"
    )

    $destinationFullPath = Join-Path $includeDir "googletest-main.zip"
    $googletestDestination = Join-Path $includeDir "gtest"

    # Check if the gtest directory already exists
    if (Test-Path $googletestDestination) {
        Write-Host "gtest directory already exists in the include folder. Skipping download and setup."
        return
    }

    Write-Host "Downloading GoogleTest library to $destinationFullPath ..."
    try {
        Invoke-WebRequest -Uri $url -OutFile $destinationFullPath
        Write-Host "Successfully downloaded GoogleTest."
    } catch {
        Write-Host "Failed to download GoogleTest. Exiting."
        exit 1
    }

    Write-Host "Extracting GoogleTest..."
    try {
        Expand-Archive -Path $destinationFullPath -DestinationPath $includeDir -Force
        Write-Host "Successfully extracted GoogleTest."
    } catch {
        Write-Host "Failed to extract GoogleTest. Exiting."
        exit 1
    }

    # Define source and destination paths
    $googletestSource = Join-Path $includeDir "googletest-main/googletest/include/gtest"
    $googletestDestination = Join-Path $includeDir "gtest"

    Write-Host "Checking if source path exists: $googletestSource"
    if (-not (Test-Path $googletestSource)) {
        Write-Host "GoogleTest source path does not exist. Exiting."
        exit 1
    }

    # Remove the destination folder if it already exists
    if (Test-Path $googletestDestination) {
        Write-Host "Destination folder $googletestDestination exists. Removing it..."
        Remove-Item $googletestDestination -Recurse -Force
    }

    # Create a fresh destination folder
    New-Item -Path $googletestDestination -ItemType Directory | Out-Null

    Write-Host "Copying contents from $googletestSource to $googletestDestination..."
    try {
        # Copy all items from the source into the destination folder
        Copy-Item -Path (Join-Path $googletestSource "*") -Destination $googletestDestination -Recurse -Force
        Write-Host "Successfully copied GoogleTest headers."
    } catch {
        Write-Host "Failed to copy GoogleTest headers. Exiting."
        exit 1
    }

    # Clean up: Remove the zip file and the extracted folder
    Write-Host "Cleaning up extracted files..."
    Remove-Item $destinationFullPath -Force
    Remove-Item (Join-Path $includeDir "googletest-main") -Recurse -Force

    Write-Host "GoogleTest setup complete."
}

function Build-DLL {
    Write-Host "Compiling DPC++ DLL..."

    # Set the compiler flags and arguments for debugging and optimizations
    $icpxArgs = "-v -fsycl -fsycl-targets=nvptx64-nvidia-cuda -std=c++23 -DDPCPP_COMPILER -DGPU"
    $cudaPath = "`"C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.0`""

    # List of source files to compile
    $files = @(
        "../src/lbm/common.cpp",
        "../dpcxx_dll/lbm/collide_and_stream.cpp",
        "../dpcxx_dll/lbm/grid_to_vertex_data.cpp"
    )

    # Include directories
    $includes = @(
        "../include"
    )

    $libs = @(
        "../lib"
    )

    $dpcxx_includes = @(
        "`"C:\Program Files (x86)\Intel\oneAPI\compiler\2025.0\include`"",
        "`"C:\Program Files (x86)\Intel\oneAPI\tbb\2022.0\include`""
    )

    $dpcxx_libs = @(
        "`"C:\Program Files (x86)\Intel\oneAPI\compiler\2025.0\lib`"",
        "`"C:\Program Files (x86)\Intel\oneAPI\tbb\2022.0\lib`""
    )

    # Build command output file (DLL) path
    $outputDLL = "fs_dpcxx.dll"

    # Set the DPC++ compiler path, assuming `icpx` or its equivalent is available in your path
    $dpcxx = "icpx"  # Use `icpx` for Intel DPC++ compiler

    # Build command
    $compileCommand = "$dpcxx -DDLL_EXPORTS $icpxArgs " +
        ($files | ForEach-Object { "$_ " }) +
        ($includes | ForEach-Object { "-I" + (Join-Path (Get-Location) $_) + " " }) +
        ($libs | ForEach-Object { "-L" + (Join-Path (Get-Location) $_) + " " }) +
        ($dpcxx_includes | ForEach-Object { "-I" + $_ + " " }) +
        ($dpcxx_libs | ForEach-Object { "-L" + $_ + " " }) +
        "-shared " +
        "-o $outputDLL " +
        "--cuda-path=" + $cudaPath + " " +
        "-lsycl -lOpenCL " +
        "-LD"

    # Print the compile command for debugging
    Write-Output "Compiling with: $compileCommand"

    # Execute the build command
    Invoke-Expression $compileCommand

    Write-Host "Compilation complete."

    # Resolve the full path for the DLL (relative to current location)
    $dllPath = (Resolve-Path $outputDLL).Path

    # Check if the DLL file exists
    if (-Not (Test-Path $dllPath)) {
        Write-Error "DLL not found at $dllPath. Exiting script."
        exit 1
    } else {
        Write-Host "DLL found at $dllPath."
    }
}

function Compile-And-Run-DPCPP-Tests {

    Write-Host "Compiling tests..."

    # Set the compiler flags and arguments for debugging and optimizations
    $icpxArgs = "-v -fsycl -fsycl-targets=nvptx64-nvidia-cuda -std=c++23 -DDPCPP_COMPILER -DGPU"

    $cudaPath = "`"C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.0`""

    # List of source files to compile
    $files = @(
        "../src/lbm/common.cpp",
        "../tests/test_collide_and_stream_state.cpp"
    )

    # Include directories
    $includes = @(
        "../include"
    )

    $libs = @(
        "../lib/dpc++_libs"
    )

    $dpcxx_includes = @(
        "`"C:\Program Files (x86)\Intel\oneAPI\compiler\2025.0\include`"",
        "`"C:\Program Files (x86)\Intel\oneAPI\tbb\2022.0\include`""
    )

    $dpcxx_libs = @(
        "`"C:\Program Files (x86)\Intel\oneAPI\compiler\2025.0\lib`"",
        "`"C:\Program Files (x86)\Intel\oneAPI\tbb\2022.0\lib`""
    )

    # Output file name for the compiled executable
    $outputFile = "fs_dpcxx_test.exe"

    # Set the DPC++ compiler path, assuming `icpx` or its equivalent is available in your path
    $dpcxx = "icpx"  # Use `icpx` for Intel DPC++ compiler

    # Build command
    $compileCommand = "$dpcxx $icpxArgs -o $outputFile " +
        ($files | ForEach-Object { $_ + " " }) +
        ($includes | ForEach-Object { "-I" + (Join-Path (Get-Location) $_) + " " }) +
        ($libs | ForEach-Object { "-L" + (Join-Path (Get-Location) $_) + " " }) +
        ($dpcxx_includes | ForEach-Object { "-I" + $_ + " " }) +
        ($dpcxx_libs | ForEach-Object { "-L" + $_ + " " }) +
        "--cuda-path=" + $cudaPath + " " +
        "-lsycl -lOpenCL -lgtest -lgtest_main"

    # Print the compile command for debugging
    Write-Output "Compiling with: $compileCommand"

    # Execute the build command
    Invoke-Expression $compileCommand

    Write-Host "Compilation complete."

    Write-Host "Running fs_dpcxx_test.exe..."

    # Run the compiled test executable
    & .\fs_dpcxx_test.exe

    # Check the exit code of the test executable.
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Tests failed with exit code $LASTEXITCODE. Terminating script."
        exit $LASTEXITCODE
    } else {
        Write-Host "All tests passed."
    }
}

function Check-NvidiaGPU {
    # Use Get-CimInstance to query the video controller (GPU) information
    $gpu = Get-CimInstance -ClassName Win32_VideoController | Where-Object { $_.Name -like "*NVIDIA*" }

    # If a GPU with "NVIDIA" in the name is found, return $true, otherwise return $false
    if ($gpu) {
        return $true
    } else {
        return $false
    }
}

function Install-TBB {

    # Install TBB (Threading Building Blocks)
    Write-Host "Installing TBB using pacman..."
    & "C:\msys64\usr\bin\bash.exe" -c "pacman -S mingw-w64-x86_64-tbb --noconfirm"
    Write-Host "TBB installation completed."
}

function Install-CUDA {

    $CUDAPath = "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.0"
    $CUDAInstallerUrl = "https://developer.download.nvidia.com/compute/cuda/12.0.0/local_installers/cuda_12.0.0_527.41_windows.exe"  
    $CUDAInstallerPath = "C:\CUDA_Installer\cuda_install.exe"

    if (Test-Path $CUDAPath) {
        Write-Host "CUDA toolkit found at $CUDAPath."
        return
    } else {
        Write-Host "CUDA toolkit not found."
    }

    # Check if CUDA installer already exists
    if (-not (Test-Path $CUDAInstallerPath)) {
        Write-Host "Downloading CUDA installer..."
        Invoke-WebRequest -Uri $CUDAInstallerUrl -OutFile $CUDAInstallerPath
        Write-Host "CUDA installer downloaded."
    }

    # Install the bare minimum components for SYCL
    Write-Host "Installing CUDA toolkit (minimal install)..."
    Start-Process -FilePath $CUDAInstallerPath -ArgumentList "/silent /install /components=compiler,driver" -Wait
    Write-Host "CUDA installation completed."
}

$vcVarsAllPath = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
if (Test-Path $vcVarsAllPath) {
    Write-Host "Running vcvarsall.bat to finalize the environment setup..."
    & $vcVarsAllPath x64
} else {
    Write-Host "Error: vcvarsall.bat not found. Visual Studio or Build Tools may not be fully installed."
}

# Check if there is an NVIDIA GPU
if (Check-NvidiaGPU) {
    Write-Host "NVIDIA GPU found..."
    Install-CUDA
} else {
    Write-Host "No NVIDIA GPU found."
}

Setup-VSEnvironment

Install-Chocolatey
Install-Curl
Install-MSYS2 -Action $Action
MSYS2-Checks
Install-OpenCV
Install-TBB
Install-TBBINTEL
Install-GLFW
Download-Files
Download-mdspan
Download-GLM
Install-ImGui

Install-VisualStudio -Action $Action
# Install-Handle
Install-DPCPP -Action $Action

Write-Host "Current script root: $PSScriptRoot"
$currentRoot = $PSScriptRoot
Download-Googletest
Setup-GoogleTest -scriptRoot $currentRoot

if ($GPU) {
    Build-DLL
    Compile-And-Run-DPCPP-Tests
    Compile-And-Run-Tests -GPU 
    Compile-Program -GPU 
} else {
    Compile-And-Run-Tests
    Compile-Program
}