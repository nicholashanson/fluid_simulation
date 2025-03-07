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
    lbm/grid_to_vertex_data.cpp ^
    lbm/collision_step.cpp ^
    C:/Users/HP/Documents/fluid_simulation/src/lbm/common.cpp ^
    -I"C:/Users/HP/Documents/fluid_simulation/include" ^
    -I%ONEAPI_INCLUDE% ^
    -L%ONEAPI_LIB% ^
    -lsycl -lOpenCL ^
    -shared ^
    -o fs_dpcxx.dll ^
    --cuda-path=%CUDA_PATH% ^
    -LD

echo Build process completed.