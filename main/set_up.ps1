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
            choco install $dep -y
        } else {
            Write-Host "$dep is already installed."
        }
    }
}

Install-Chocolatey