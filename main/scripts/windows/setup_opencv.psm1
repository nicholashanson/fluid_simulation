$globalVars = Join-Path $PSScriptRoot "global_vars.psm1"
Import-Module $globalVars

function Setup-OpenCV {
    param (
        [string]$scriptRoot,
        [string]$url = "https://github.com/opencv/opencv/archive/refs/heads/master.zip"
    )

    if (-not (Test-Path $includeDir)) {
        Write-Host "$includeDir does not exist!"
    }

    $dpcppLibDir = [System.IO.Path]::GetFullPath((Join-Path $scriptRoot "..\lib\dpc++_libs"))
    $resolvedIncludeDir = [System.IO.Path]::GetFullPath( $includeDir )

    $zipFullPath = Join-Path $includeDir "opencv-master.zip"

    if (-not (Test-Path $dpcppLibDir)) {
        New-Item -Path $dpcppLibDir -ItemType Directory | Out-Null
    }

    # Check if the OpenCV libraries already exist in ../lib relative to the script
    if ((Test-Path (Join-Path $dpcppLibDir "opencv_core4120.lib")) -and
        (Test-Path (Join-Path $resolvedIncludeDir "opencv.hpp")) -and
        (Test-Path (Join-Path $resolvedIncludeDir "opencv2/opencv_modules.hpp")) -and  
        (Test-Path (Join-Path $resolvedIncludeDir "opencv2/core.hpp")) -and    
        (Test-Path (Join-Path $resolvedIncludeDir "opencv2/imgproc.hpp")) -and   
        (Test-Path (Join-Path $dpcppLibDir "opencv_imgproc4120.lib")) -and
        (Test-Path (Join-Path $scriptRoot "opencv_core4120.dll")) -and
        (Test-Path (Join-Path $scriptRoot "opencv_imgproc4120.dll")) -and
        (Test-Path (Join-Path $dpcppLibDir "opencv_core4120.exp")) -and
        (Test-Path (Join-Path $dpcppLibDir "opencv_imgproc4120.exp"))) {

        Write-Host "OpenCV header-files and libraries already exist. Skipping download and build."
        return
    }

    if (-not (Test-Path $zipFullPath)) {
        Write-Host "Downloading OpenCV library to $zipFullPath ..."
        try {
            Invoke-WebRequest -Uri $url -OutFile $zipFullPath
            Write-Host "Successfully downloaded OpenCV."
        } catch {
            Write-Host "Failed to download OpenCV. Exiting."
            exit 1
        }
    } else {
        Write-Host "OpenCV zip already exists. Skipping download."
    }

    # Extract OpenCV
    $opencvRootDir = Join-Path $includeDir "opencv-master"
    if (-not (Test-Path $opencvRootDir)) {
        Write-Host "Extracting OpenCV..."
        try {
            Expand-Archive -Path $zipFullPath -DestinationPath $includeDir -Force
            Write-Host "Successfully extracted OpenCV."
        } catch {
            Write-Host "Failed to extract OpenCV. Exiting."
            exit 1
        }
    } else {
        Write-Host "OpenCV source already exists. Skipping extraction."
    }

    $CCompilerPath = (Get-Command icx).Source
    $CXXCompilerPath = (Get-Command icx).Source

    $cmakeBuildDir = Join-Path $opencvRootDir "build"

    # Build OpenCV with ICX
    try {

        $env:CC = "$CCompilerPath"
        $env:CXX = "$CXXCompilerPath"

        if (-not (Test-Path $cmakeBuildDir)) {
            New-Item -Path $cmakeBuildDir -ItemType Directory | Out-Null
        }
        Push-Location $cmakeBuildDir
        cmake .. -DCMAKE_C_COMPILER="$CCompilerPath" `
                 -DCMAKE_CXX_COMPILER="$CXXCompilerPath" `
                 -DCMAKE_BUILD_TYPE=Release `
                 -DBUILD_LIST="core,imgproc" `
                 -DBUILD_opencv_world=OFF `
                 -DBUILD_opencv_core=ON `
                 -DBUILD_opencv_imgproc=ON `
                 -DBUILD_EXAMPLES=OFF `
                 -DBUILD_TESTS=OFF `
                 -DBUILD_DOCS=OFF

        $buildProcess = Start-Process cmake -ArgumentList "--build . --config Release" -PassThru -Wait
        # cmake --build . --config Release
        Pop-Location
        Write-Host "Successfully built OpenCV with DPC++."
    } catch {
        Write-Host "Failed to build OpenCV with DPC++. Exiting."
        exit 1
    }

    # Copy ICX-built OpenCV libraries to dpc++_libs directory
    Write-Host "Copying ICX libraries to $dpcppLibDir..."
    try {
        # Define the specific libraries you want to copy
        $opencvLibs = @("opencv_core4120.lib", "opencv_imgproc4120.lib")
        $opencvDlls = @("opencv_core4120.dll", "opencv_imgproc4120.dll")
        $opencvExps = @("opencv_core4120.exp", "opencv_imgproc4120.exp")

        # Get all the libraries in the specified build directory
        $dpcppBuiltLibs = Get-ChildItem -Path (Join-Path $cmakeBuildDir "lib/Release") -Filter "*.lib" | 
            Where-Object { $_.Name -in $opencvLibs }

        $dpcppBuiltDlls = Get-ChildItem -Path (Join-Path $cmakeBuildDir "bin/Release") -Filter "*.dll" | 
            Where-Object { $_.Name -in $opencvDlls }

        $dpcppBuiltExps = Get-ChildItem -Path (Join-Path $cmakeBuildDir "lib/Release") -Filter "*.exp" | 
            Where-Object { $_.Name -in $opencvExps }

        # Copy each of the specified libraries, DLLs, and EXPs
        foreach ($lib in $dpcppBuiltLibs) {
            Copy-Item -Path $lib.FullName -Destination $dpcppLibDir -Force
        }
        
        foreach ($dll in $dpcppBuiltDlls) {
            Copy-Item -Path $dll.FullName -Destination $scriptRoot -Force
        }

        foreach ($exp in $dpcppBuiltExps) {
            Copy-Item -Path $exp.FullName -Destination $dpcppLibDir -Force
        }

        Write-Host "Successfully copied specified ICX libraries, DLLs, and EXPs."
    } catch {
        Write-Host "Failed to copy ICX libraries, DLLs, and EXPs. Exiting."
        exit 1
    }

    $opencvIncludeDir = Join-Path $includeDir "opencv2"
    if (-not (Test-Path -Path $opencvIncludeDir)) {
        Write-Host "Creating directory: $opencvIncludeDir"
        New-Item -Path $opencvIncludeDir -ItemType Directory -Force
    }

    # Correct mapping of headers to their exact locations in modules/<module>/include/
    $headersToCopy = @{
        "core.hpp"    = "modules/core/include/opencv2"
        "imgproc.hpp" = "modules/imgproc/include/opencv2"
        "gapi.hpp"    = "modules/gapi/include/opencv2"
    }

    # Copy headers from strictly <module>/include/ (not deeper)
    foreach ($header in $headersToCopy.Keys) {
        $expectedPath = Join-Path $opencvRootDir $headersToCopy[$header] $header

        if (Test-Path $expectedPath) {
            $destinationPath = Join-Path $includeDir "opencv2" $header
            Write-Host "Copying header: $($header) from $expectedPath to $destinationPath"
            Copy-Item -Path $expectedPath -Destination $destinationPath -Force -Verbose
        } else {
            Write-Host "WARNING: Expected header not found: $expectedPath"
        }
    }

    $headersToCopy = @("opencv.hpp", "opencv_modules.hpp")
    $headerPaths = Get-ChildItem -Path $opencvRootDir -Recurse -Filter "*.hpp" | 
        Where-Object { $_.Name -in $headersToCopy }

    foreach ($header in $headerPaths) {
        if ($header.Name -eq "opencv.hpp") {
            $destinationPath = Join-Path $includeDir $header.Name
        } else {
            $destinationPath = Join-Path $opencvIncludeDir $header.Name
        }
        Write-Host "Copying header: $($header.Name) to $destinationPath"
        Copy-Item -Path $header.FullName -Destination $destinationPath -Force -Verbose
    }

    $coreDir = Get-ChildItem -Path $opencvRootDir -Recurse -Directory | Where-Object { $_.FullName -like "*\include\opencv2\core" }

    if ($coreDir) {
        Write-Host "Found core directory: $($coreDir.FullName)"

        # Define destination directory for core
        $destinationCoreDir = Join-Path $opencvIncludeDir "core"

        # Copy the entire 'core' directory structure to the target location
        Write-Host "Copying 'core' directory to $destinationCoreDir"
        Copy-Item -Path $coreDir.FullName -Destination $destinationCoreDir -Recurse -Force

        Write-Host "'core' directory copied successfully."
    } else {
        Write-Host "'core' directory not found in the OpenCV source."
    }

    $imgProcDir = Get-ChildItem -Path $opencvRootDir -Recurse -Directory | Where-Object { $_.FullName -like "*\include\opencv2\imgproc" }

    if ($imgProcDir) {
        Write-Host "Found core directory: $($imgProcDir.FullName)"

        # Define destination directory for core
        $destinationImgProcDir = Join-Path $opencvIncludeDir "imgproc"

        # Copy the entire 'core' directory structure to the target location
        Write-Host "Copying 'imgproc' directory to $destinationImgProcDir"
        Copy-Item -Path $imgProcDir.FullName -Destination $destinationImgProcDir -Recurse -Force

        Write-Host "'core' directory copied successfully."
    } else {
        Write-Host "'core' directory not found in the OpenCV source."
    }

    $gapiDir = Get-ChildItem -Path $opencvRootDir -Recurse -Directory | Where-Object { $_.FullName -like "*\include\opencv2\gapi" }

    if ($gapiDir) {
        Write-Host "Found core directory: $($gapiDir.FullName)"

        # Define destination directory for core
        $destinationGapiDir = Join-Path $opencvIncludeDir "gapi"

        # Copy the entire 'core' directory structure to the target location
        Write-Host "Copying 'gapi' directory to $destinationGapiDir"
        Copy-Item -Path $gapiDir.FullName -Destination $destinationGapiDir -Recurse -Force

        Write-Host "'core' directory copied successfully."
    } else {
        Write-Host "'core' directory not found in the OpenCV source."
    }

    Write-Host "Cleaning up extracted files..."
    Remove-Item $zipFullPath -Force
    Remove-Item $opencvRootDir -Recurse -Force

    Write-Host "OpenCV setup complete."
}
