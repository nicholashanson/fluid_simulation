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

    $downloadDir = Join-Path (Get-Location) "include"
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

Install-Chocolatey
Install-Dependencies
Download-Files