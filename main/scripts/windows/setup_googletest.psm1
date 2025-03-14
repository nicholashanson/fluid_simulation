$globalVars = Join-Path $PSScriptRoot "global_vars.psm1"
Import-Module $globalVars

function Build-WithMinGW {
    param (
        [string]$buildDir
    )

    # Build GoogleTest with G++
    Write-Host "Building GoogleTest with MinGW..."
    try {
        # Ensure the build directory exists
        if (-not (Test-Path $buildDir)) {
            New-Item -Path $buildDir -ItemType Directory | Out-Null
        }
        Push-Location $buildDir

        # Set the CMake generator to MinGW Makefiles
        $env:CC = "gcc"
        $env:CXX = "g++"

        # Run CMake with the MinGW Makefiles generator
        cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
        mingw32-make
        Pop-Location
        Write-Host "Successfully built GoogleTest with MinGW."
    } catch {
        Write-Host "Failed to build GoogleTest with MinGW. Exiting."
        exit 1
    }
}

function Setup-Googletest {
    param (
        [string]$scriptRoot,
        [string]$url = "https://github.com/google/googletest/archive/refs/heads/main.zip",
        [string]$destination = "../include/googletest-main.zip"
    )

    Write-Host "Received script root in function: $scriptRoot"

    if (-not (Test-Path $includeDir)) {
        Write-Host "$includeDir does not exist!"
    }

    $gppLibDir = Join-Path $scriptRoot "..\lib\g++_libs"
    $dpcppLibDir = Join-Path $scriptRoot "..\lib\dpc++_libs"

    $CCompilerPath = (Get-Command icx).Source
    $CXXCompilerPath = (Get-Command icx).Source

    $gccPath = (Get-Command gcc).Source
    $gppPath = (Get-Command g++).Source

    Write-Host "gcc found at $gccPath"
    Write-Host "g++ found at $gppPath"

    Write-Host "Identified $includeDir"

    $zipFullPath = Join-Path $includeDir "googletest-main.zip"
    $googletestRootDir = Join-Path $includeDir "googletest-main"

    # build directory for DPC++ build
    $cmakeBuildDir = Join-Path $googletestRootDir "build"

    # build directory for G++ build
    $minGWBuildDir = Join-Path $googletestRootDir "build_mingw"

    # Ensure both lib directories exist
    if (-not (Test-Path $gppLibDir)) {
        New-Item -Path $gppLibDir -ItemType Directory | Out-Null
    }

    if (-not (Test-Path $dpcppLibDir)) {
        New-Item -Path $dpcppLibDir -ItemType Directory | Out-Null
    }

    # Check if the gtest libraries already exist in ../lib relative to the script
    if ((Test-Path (Join-Path $gppLibDir "libgtest.a")) -and 
        (Test-Path (Join-Path $gppLibDir "libgtest_main.a")) -and
        (Test-Path (Join-Path $dpcppLibDir "gtest.lib")) -and 
        (Test-Path (Join-Path $dpcppLibDir "gtest_main.lib"))) {
        Write-Host "gtest libraries already exist. Skipping download and build."
        return
    }

    # Download GoogleTest if not already downloaded
    if (-not (Test-Path $zipFullPath)) {
        Write-Host "Downloading GoogleTest library to $zipFullPath ..."
        try {
            Invoke-WebRequest -Uri $url -OutFile $zipFullPath
            Write-Host "Successfully downloaded GoogleTest."
        } catch {
            Write-Host "Failed to download GoogleTest. Exiting."
            exit 1
        }
    } else {
        Write-Host "GoogleTest zip already exists. Skipping download."
    }

    # Extract GoogleTest
    if (-not (Test-Path $googletestRootDir)) {
        Write-Host "Extracting GoogleTest..."
        try {
            Expand-Archive -Path $zipFullPath -DestinationPath $includeDir -Force
            Write-Host "Successfully extracted GoogleTest."
        } catch {
            Write-Host "Failed to extract GoogleTest. Exiting."
            exit 1
        }
    } else {
        Write-Host "GoogleTest source already exists. Skipping extraction."
    }

    Build-WithMinGW -buildDir $minGWBuildDir

    # Copy G++ libraries to g++_libs directory
    Write-Host "Copying G++ libraries to $gppLibDir..."
    try {
        $gppBuiltLibs = Get-ChildItem -Path (Join-Path $minGWBuildDir "lib") -Filter "*.a"
        foreach ($lib in $gppBuiltLibs) {
            Copy-Item -Path $lib.FullName -Destination $gppLibDir -Force
        }
        Write-Host "Successfully copied G++ libraries."
    } catch {
        Write-Host "Failed to copy G++ libraries. Exiting."
        exit 1
    }

    # Build GoogleTest with DPC++
    Write-Host "Building GoogleTest with DPC++..."
    try {

        $env:CC = "$CCompilerPath"
        $env:CXX = "$CXXCompilerPath"

        if (-not (Test-Path $cmakeBuildDir)) {
            New-Item -Path $cmakeBuildDir -ItemType Directory | Out-Null
        }
        Push-Location $cmakeBuildDir
        cmake .. -DCMAKE_C_COMPILER="$CCompilerPath" -DCMAKE_CXX_COMPILER="$CXXCompilerPath" -DCMAKE_BUILD_TYPE=Release
        cmake --build . --config Release
        Pop-Location
        Write-Host "Successfully built GoogleTest with DPC++."
    } catch {
        Write-Host "Failed to build GoogleTest with DPC++. Exiting."
        exit 1
    }

    # Copy DPC++ libraries to dpc++_libs directory
    Write-Host "Copying DPC++ libraries to $dpcppLibDir..."
    try {
        $dpcppBuiltLibs = Get-ChildItem -Path (Join-Path $cmakeBuildDir "lib/Release") -Filter "*.lib"
        foreach ($lib in $dpcppBuiltLibs) {
            Copy-Item -Path $lib.FullName -Destination $dpcppLibDir -Force
        }
        Write-Host "Successfully copied DPC++ libraries."
    } catch {
        Write-Host "Failed to copy DPC++ libraries. Exiting."
        exit 1
    }

    Write-Host "Cleaning up extracted files..."
    Remove-Item $zipFullPath -Force
    Remove-Item $googletestRootDir -Recurse -Force

    Write-Host "GoogleTest setup complete."
}
