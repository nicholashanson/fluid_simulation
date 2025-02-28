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
* Rendering using OpenGL
* CPU multi-threading using TBB
* GPU acceleration using SYCL
* OpenCV colormaps
* UI using ImGui

### Building

At the moment, this project can only be built on Windows.

To build the project, clone the GitHub repo to your machine:
```cpp
git clone https://github.com/nicholashanson/fluid_simulation
```
Navigate to the directory:
```cpp
cd fluid_simulation
```
Navigate to the "main" directory:
```cpp
cd main
```
Run "setup.ps1" in PowerShell with Administrator access:
```cpp
./set_up.ps1
```
This will handle the entire build process from downloading and managing dependencies to compilation and linking.
This may take a while depending on which dependencies are already present on your machine. This build 
process was tested on AWS using a clean EC2 instance and makes no assumptions about what is already installed on the system, except git. After the build is complete, you can run the program:
```cpp
./fs
```
This build won't include GPU acceleration for now.

### Optimizations

LBM is highly-parallelizable, which makes it a perfect candidate for GPU acceleration.

<p align="center">
  <img src="main/performance_profiling/graphs/performance_collide_and_stream.png" width="400">
</p>


