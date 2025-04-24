function Build-DLL {
    param (
        [string]$scriptRoot
    )

    Write-Host "Compiling DPC++ DLL..."

    Write-Host $scriptRoot

    $icpxArgs = "-O3 -v -qopt-report=max -qopt-report-phase=vec -emit-llvm -fsycl -fsycl-targets=nvptx64-nvidia-cuda -std=c++23 -DDPCPP_COMPILER -DGPU"
    $cudaPath = "`"C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.0`""

    # List of source files to compile
    $files = @(
        "../src/lbm/common.cpp",
        "../dpcxx_dll/lbm/collide_and_stream.cpp",
        "../dpcxx_dll/lbm/grid_to_vertex_data.cpp"
    )

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

    $outputDLL = "fs_dpcxx.dll"

    $dpcxx = "icpx"  

    $dpcpp_libs = Join-Path $scriptRoot "..\lib\dpc++_libs"

    Resolve-Path (Join-Path $scriptRoot $_)

    # Build command
    $compileCommand = "$dpcxx -DDLL_EXPORTS $icpxArgs " +
        ($files | ForEach-Object { "$_ " }) +
        ($includes | ForEach-Object { "-I" + (Resolve-Path (Join-Path $scriptRoot $_)).Path + " " }) +
        ($libs | ForEach-Object { "-L" + (Resolve-Path (Join-Path $scriptRoot $_)).Path + " " }) +
        ($dpcxx_includes | ForEach-Object { "-I" + $_ + " " }) +
        ($dpcxx_libs | ForEach-Object { "-L" + $_ + " " }) +
        "-L" + $dpcpp_libs + " " +
        "-shared " +
        "-o $outputDLL " +
        "--cuda-path=" + $cudaPath + " " +
        "-lsycl -lOpenCL -lopencv_core4120 -lopencv_imgproc4120 " +
        "-LD"

    # Print the compile command for debugging
    Write-Output "Compiling with: $compileCommand"

    # Execute the build command
    Invoke-Expression $compileCommand

    Write-Host "Compilation complete."

    $dllPath = Join-Path $scriptRoot $outputDLL

    # Check if the DLL file exists
    if (-Not (Test-Path $dllPath)) {
        Write-Error "DLL not found at $dllPath. Exiting script."
        exit 1
    } else {
        Write-Host "DLL found at $dllPath."
    }
}