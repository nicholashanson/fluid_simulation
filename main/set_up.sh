#!/bin/bash

check_gpp_version() {
    # Check if g++ is installed
    if ! command -v g++ &> /dev/null; then
        echo "❌ g++ is not installed."
        install_gpp
        return
    fi

    # Get g++ version
    GPP_VERSION=$(g++ -dumpversion)
    echo "✅ Found g++ version: $GPP_VERSION"

    # Check if g++ supports C++23
    if echo '#include <version>' | g++ -std=c++23 -x c++ -E - 2>/dev/null | grep -q "cplusplus=2023"; then
        echo "✅ g++ supports C++23"
    else
        echo "⚠️ g++ does not support C++23. Updating..."
        install_gpp
    fi
}

install_gpp() {
    echo "🔄 Installing or updating g++..."
    
    # Detect package manager
    if command -v apt &> /dev/null; then
        sudo apt update && sudo apt install -y g++ 
    elif command -v dnf &> /dev/null; then
        sudo dnf install -y gcc-c++
    elif command -v pacman &> /dev/null; then
        sudo pacman -S --noconfirm gcc
    elif command -v zypper &> /dev/null; then
        sudo zypper install -y gcc-c++
    else
        echo "❌ Unsupported package manager. Install g++ manually."
        exit 1
    fi

    echo "✅ g++ installation/update complete."
}

download_kokkos_mdspan() {
    # Define the URL to the Kokkos mdspan zip file on GitHub
    local url="https://github.com/kokkos/mdspan/archive/refs/heads/main.zip"
    
    # Define the destination directory for the 'include' folder
    local target_dir="$(dirname "$(pwd)")/include"
    
    # Create the 'include' directory if it doesn't exist
    mkdir -p "$target_dir"
    
    # Download the Kokkos mdspan zip file to the current directory
    curl -L "$url" -o mdspan.zip
    
    # Unzip the downloaded zip file into the 'include' directory
    unzip -q mdspan.zip -d "$target_dir"
    
    # Optionally, clean up the zip file after extraction
    rm -f mdspan.zip
    
    # Notify user that the operation is complete
    echo "Kokkos mdspan has been downloaded and extracted to $target_dir"
}

download_glm() {
    # Define the URL to the GLM zip file on GitHub
    local url="https://github.com/g-truc/glm/archive/refs/heads/master.zip"
    
    # Define the destination directory for the 'include' folder
    local target_dir="$(dirname "$(pwd)")/include"
    
    # Create the 'include' directory if it doesn't exist
    mkdir -p "$target_dir"
    
    # Download the GLM zip file to the current directory
    curl -L "$url" -o glm.zip
    
    # Unzip the downloaded zip file into the 'include' directory
    unzip -q glm.zip -d "$target_dir"
    
    # Move extracted contents inside the include directory
    mv "$target_dir/glm-master" "$target_dir/glm"
    
    # Optionally, clean up the zip file after extraction
    rm -f glm.zip
    
    # Notify user that the operation is complete
    echo "GLM has been downloaded and extracted to $target_dir/glm"
}

install_cuda_for_dpcpp() {
    # URL for CUDA toolkit installer (to be provided by you)
    local cuda_url="$1"
    
    # Define the name of the CUDA installer file (based on the URL)
    local cuda_installer="cuda_installer.run"

    # Download the CUDA installer
    echo "Downloading CUDA toolkit installer from $cuda_url..."
    curl -L "$cuda_url" -o "$cuda_installer"

    # Make the installer executable
    chmod +x "$cuda_installer"

    # Run the installer with options to only install the components necessary for DPC++
    echo "Running CUDA toolkit installer..."
    sudo ./"$cuda_installer" --silent --toolkit --samples --driver --no-opengl-libs

    # Check if CUDA is installed properly
    if command -v nvcc &>/dev/null; then
        echo "CUDA Toolkit successfully installed."
    else
        echo "CUDA installation failed."
        exit 1
    fi
}

install_glfw() {
    # Set up directories
    downloadDir="$(dirname "$(pwd)")/include"
    if [ ! -d "$downloadDir" ]; then
        mkdir -p "$downloadDir"
    fi

    # Create the 'glfw' subdirectory
    glfwDir="$downloadDir/GLFW"
    if [ ! -d "$glfwDir" ]; then
        mkdir -p "$glfwDir"
    fi

    # Download glfw3.h from GLFW official repository
    glfwUrl="https://raw.githubusercontent.com/glfw/glfw/master/include/GLFW/glfw3.h"
    glfwFileName="glfw3.h"
    glfwFilePath="$glfwDir/$glfwFileName"

    echo "Downloading $glfwFileName from GLFW official repository..."
    if curl -L "$glfwUrl" -o "$glfwFilePath"; then
        echo "Successfully downloaded $glfwFileName."
    else
        echo "Failed to download $glfwFileName. Exiting."
        exit 1
    fi

    # Check if GLFW 3 is installed using apt
    echo "Checking if GLFW 3 is installed..."

    if dpkg -l | grep -q "libglfw3"; then
        echo "GLFW 3 is already installed."
    else
        echo "GLFW 3 is not found. Installing GLFW 3..."
        
        # Install GLFW 3 using apt
        sudo apt update
        sudo apt install -y libglfw3 libglfw3-dev
    fi
}

install_opencv() {
    # Update system package list
    sudo apt update

    # Install dependencies
    sudo apt install -y build-essential cmake git pkg-config libjpeg8-dev \
        libtiff5-dev libpng-dev libjasper-dev libavcodec-dev libavformat-dev \
        libswscale-dev libv4l-dev libxvidcore-dev libx264-dev libgtk-3-dev \
        libcanberra-gtk3-dev libatlas-base-dev gfortran python3-dev

    # Install Python 3 libraries (optional, if you want to use OpenCV with Python)
    sudo apt install -y python3-opencv

    # Clone the OpenCV repository (latest stable version)
    cd ~
    git clone https://github.com/opencv/opencv.git

    # Clone the opencv_contrib repository for additional features
    git clone https://github.com/opencv/opencv_contrib.git

    # Create a build directory and navigate into it
    cd opencv
    mkdir build
    cd build

    # Run cmake to configure the build with opencv_contrib modules
    cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr/local \
          -D OPENCV_EXTRA_MODULES_PATH=~/opencv_contrib/modules ..

    # Compile OpenCV (this can take some time depending on your system)
    make -j$(nproc)  # Use all available cores to speed up the build

    # Install OpenCV to the system
    sudo make install

    # Update the shared library cache
    sudo ss

    # Verify installation
    echo "OpenCV installation complete."
}

install_opengl() {
    # Update package list
    sudo apt update
    
    # Install Mesa and OpenGL development libraries
    echo "Installing OpenGL and Mesa packages..."
    sudo apt install -y libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev

    # Install graphics drivers (For AMD/Intel, Mesa is usually sufficient)
    # For NVIDIA, use the NVIDIA drivers if required
    echo "Installing necessary graphics drivers..."
    sudo apt install -y mesa-utils

    # Verify OpenGL installation using glxinfo
    echo "Verifying OpenGL installation..."
    if ! command -v glxinfo &> /dev/null; then
        echo "glxinfo command is not available. Installing mesa-utils..."
        sudo apt install -y mesa-utils
    fi

    # Check OpenGL info
    glxinfo | grep "OpenGL"

    # Check if the system has a GPU with OpenGL
    echo "Checking for OpenGL-capable GPU..."
    if lspci | grep -i "vga" | grep -i "nvidia" > /dev/null; then
        echo "NVIDIA GPU detected. Ensure proprietary NVIDIA drivers are installed."
        echo "You may need to install NVIDIA drivers with: sudo apt install nvidia-driver"
    elif lspci | grep -i "vga" | grep -i "amd" > /dev/null; then
        echo "AMD GPU detected. OpenGL support is provided via Mesa drivers."
    elif lspci | grep -i "vga" | grep -i "intel" > /dev/null; then
        echo "Intel GPU detected. OpenGL support is provided via Mesa drivers."
    else
        echo "No known GPU found. OpenGL might not be available."
    fi

    # Confirm installation
    echo "OpenGL installation complete."
}

Install-ImGui() {
    # Get the parent directory path
    local parentDirectory=$(dirname "$(pwd)")

    # Define the path for the zip file to download
    local imguiZipFile="$parentDirectory/imgui-master.zip"
    local imguiRepoZipUrl="https://github.com/ocornut/imgui/archive/refs/heads/master.zip"

    # Check if imgui-master exists in the parent directory
    if [ -d "$parentDirectory/imgui-master" ]; then
        echo "imgui-master already exists in the parent directory. Skipping installation."
    else
        echo "imgui-master not found. Installing ImGui..."

        # Check if the zip file already exists in the parent directory
        if [ ! -f "$imguiZipFile" ]; then
            echo "Downloading imgui-master.zip from GitHub..."
            curl -L "$imguiRepoZipUrl" -o "$imguiZipFile"
            echo "Download completed."
        fi

        # Unzip the contents into the parent directory
        unzip -q "$imguiZipFile" -d "$parentDirectory"
        echo "Unzipped imgui-master.zip to the parent directory."

        # Remove the zip file after extraction
        rm -f "$imguiZipFile"
        echo "Removed the downloaded zip file."

        echo "ImGui installation completed."
    fi
}

Compile-Code() {
    echo "Compiling program..."

    # Compiler arguments
    local gppArgs="-g -O0 -v -std=c++23"

    # List of source files
    local files=(
        "../imgui-master/imgui.cpp"
        "../imgui-master/imgui_draw.cpp"
        "../imgui-master/imgui_widgets.cpp"
        "../imgui-master/imgui_tables.cpp"
        "../imgui-master/backends/imgui_impl_opengl3.cpp"
        "../imgui-master/backends/imgui_impl_glfw.cpp"
        "main.cpp"
        "gl.cpp"
        "../src/lbm/common.cpp"
        "../src/grid_renderer.cpp"
        "../src/shader.cpp"
        "../src/glad.c"
    )

    # List of include directories
    local includes=(
        "../include"
        "../inline"
        "../imgui-master"
        "../imgui-master/backends"
    )

    # Output file name
    local outputFile="fs.out"

    # Get the OpenCV include path using pkg-config
    local opencvIncludePath=$(pkg-config --cflags-only-I opencv4)

    # Build command
    local compileCommand="g++ $gppArgs -o $outputFile"

    # Add files to compile command
    for file in "${files[@]}"; do
        compileCommand="$compileCommand $file"
    done

    # Add includes to compile command
    for include in "${includes[@]}"; do
        compileCommand="$compileCommand -I$(pwd)/$include"
    done

    # Add OpenCV include path
    compileCommand="$compileCommand $opencvIncludePath"

    # Add libraries to compile command
    compileCommand="$compileCommand -lopengl32 -lglfw3 -lgdi32 -ltbb12 -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs"

    # Print the command for debugging
    echo "Compiling with: $compileCommand"

    # Execute the build
    eval "$compileCommand"

    echo "Compilation complete."
}

Install-TBB() {
    echo "Installing Intel TBB (Threading Building Blocks)..."

    # Detect package manager and install Intel TBB based on the distribution
    if command -v apt-get &> /dev/null; then
        # For Debian/Ubuntu-based distributions
        sudo apt-get update
        sudo apt-get install -y libtbb-dev
    elif command -v dnf &> /dev/null; then
        # For Fedora/RHEL-based distributions
        sudo dnf install -y tbb-devel
    elif command -v pacman &> /dev/null; then
        # For Arch-based distributions
        sudo pacman -S --noconfirm tbb
    elif command -v zypper &> /dev/null; then
        # For openSUSE-based distributions
        sudo zypper install -y tbb-devel
    else
        echo "❌ Unsupported package manager. Please install Intel TBB manually."
        exit 1
    fi

    echo "Intel TBB installation complete."
}

# Run the check
check_gpp_version