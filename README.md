# Fluid Simulation
## Lattice Boltzmann Method ( LBM )

<p align="center">
  <img src="https://github.com/user-attachments/assets/95214616-9cde-4c26-ab0f-af06f671033e" height="160" width="400">
  <img src="https://github.com/user-attachments/assets/24f21671-62f4-4fb1-8f71-8f834e3fce89" height="160" width="400">
  <br>
  <img src="https://github.com/user-attachments/assets/17f60251-462b-441c-a8e6-456738ee8cfc" height="160" width="400">
  <img src="https://github.com/user-attachments/assets/31d7a299-d3e2-4f0e-9794-5e444115d28e" height="160" width="400">
  <br>
  <span>top-left: curl, top-right: speed, bottom-left: ux, bottom-right: uy</span>
</p>

### Feautures

* Visualization of multiple properties including speed, density, and curl
* Cross-platform: Linux and Windows
* Rendering using OpenGL
* CPU multi-threading using TBB
* GPU acceleration using SYCL
* OpenCV colormaps
* UI using ImGui

### Building

To build the project, clone the GitHub repo to your machine:
```cpp
git clone https://github.com/nicholashanson/fluid_simulation
```
Navigate to the project directory:
```cpp
cd fluid_simulation
```
Navigate to the "main" directory:
```cpp
cd main
```
On Windows, run "set_up.ps1" in PowerShell with Administrator access:
```cpp
./set_up.ps1
```
On Linux, run "set_up.sh":
```cpp
./set_up.sh
```
This will handle the entire build process from downloading and managing dependencies to compilation and linking.
This may take a while depending on which dependencies are already present on your machine. This build makes no assumptions about what is already installed on the system, except git. After the build is complete, you can run the program:
```cpp
./fs
```
This build won't include GPU acceleration for now.

### Profiling

<p align="center">
  <img src="main/profiling_data.png" width="800">
</p>

* In OpenGL, the CPU offloads rendering to the GPU asynchronously, so calling "render_grid" is negligible.
* Most time at the moment is spent calculating the vertex data. 

### Optimizations

LBM is highly-parallelizable, which makes it a perfect candidate for GPU acceleration.

<p align="center">
  <img src="main/performance_profiling/graphs/performance_collide_and_stream.png" width="400">
</p>


