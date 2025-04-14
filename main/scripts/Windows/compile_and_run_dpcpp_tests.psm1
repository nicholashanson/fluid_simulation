function Compile-And-Run-DPCPP-Tests {
    param (
        [string]$scriptRoot
    )

    Write-Host "Compiling tests..."

    # Compiler flags
    $icpxArgs = "-g -v -fsycl -fsycl-targets=nvptx64-nvidia-cuda -std=c++23 -DDPCPP_COMPILER -DGPU"

    # Path to CudaToolkit
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

    # DPC++ compiler path
    $dpcxx = "icpx"  

    # Build command
    $compileCommand = "$dpcxx $icpxArgs -o $outputFile " +
        ($files | ForEach-Object { $_ + " " }) +
        ($includes | ForEach-Object { "-I" + (Resolve-Path (Join-Path $scriptRoot $_)).Path + " " }) +
        ($libs | ForEach-Object { "-L" + (Resolve-Path (Join-Path $scriptRoot $_)).Path + " " }) +
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