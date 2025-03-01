@echo off
set CUDA_PATH="C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.0"
set ONEAPI_INCLUDE="C:/Program Files (x86)/Intel/oneAPI/compiler/2025.0/include"
set ONEAPI_LIB="C:/Program Files (x86)/Intel/oneAPI/compiler/2025.0/lib"

rem Compiler command
icpx ^
    -DDLL_EXPORTS ^
    -fsycl ^
    -fsycl-targets=nvptx64-nvidia-cuda ^
    -std=c++23 ^
    -DDPCPP_COMPILER ^
    lbm/collide_and_stream.cpp ^
    C:/Users/HP/Documents/fluid_simulation/src/lbm/common.cpp ^
    -I"C:/Users/HP/Documents/fluid_simulation/include" ^
    -I"C:/Program Files (x86)/Intel/OneAPI/tbb/2022.0/include" ^
    -L"C:/Program Files (x86)/Intel/OneAPI/tbb/2022.0/lib" ^
    -L%ONEAPI_LIB% ^
    -lsycl -lOpenCL -ltbb12 ^
    -shared ^
    -o fs_dpcxx.dll ^
    --cuda-path=%CUDA_PATH% ^
    -LD

echo Build process completed.