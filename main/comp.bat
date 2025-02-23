set CUDA_PATH="C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.0"

set PATH=C:\Program Files (x86)\Intel\oneAPI\tbb\2022.0\bin;%PATH%

icpx -fsycl -std=c++23 -fsycl-targets=nvptx64-nvidia-cuda ^
    "C:/Users/HP/Documents/fluid_simulation/main/cpu_gpu_performance_comp.cpp" ^
    -I"C:/Users/HP/Documents/fluid_simulation/include" ^
    -I"C:/Program Files (x86)/Intel/OneAPI/tbb/2022.0/include" ^
    -L"C:/Program Files (x86)/Intel/OneAPI/tbb/2022.0/lib" ^
    --cuda-path=%CUDA_PATH% ^
    -lsycl -lOpenCL -ltbb12
