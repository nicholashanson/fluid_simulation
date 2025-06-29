#!/bin/bash

check_gpp_version() {
    # Check if g++ is installed
    if ! command -v g++ &> /dev/null; then
        echo "❌ g++ is not installed."
        install_gpp
        return
    else 
        echo "✅ g++ is already installed."
        return
    fi
}

install_gpp() {
    echo "🔄 Installing g++..."
    
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

    echo "✅ g++ installation complete."
}

# download and unzip the Kokkos mdspan zip file into the include directory
download_and_unzip_mdspan() {

    # URL for the Kokkos mdspan zip file from GitHub 
    ZIP_URL="https://codeload.github.com/kokkos/mdspan/zip/refs/heads/stable"

    # Get the parent directory of the current directory
    PARENT_DIR=$(dirname "$PWD")

    # Define the include directory path
    INCLUDE_DIR="$PARENT_DIR/include"

    # Download the zip file to a temporary location
    TEMP_ZIP="/tmp/mdspan.zip"
    wget -O "$TEMP_ZIP" "$ZIP_URL"

    # Unzip the downloaded zip file into the include directory
    unzip -q "$TEMP_ZIP" -d "$INCLUDE_DIR"

    # Clean up the temporary zip file
    rm "$TEMP_ZIP"

    echo "Kokkos mdspan has been downloaded and extracted into $INCLUDE_DIR."
}

install_curl() {
    echo "Ensuring dependencies are installed..."

    # Check if 'curl' is already installed
    if command -v curl &> /dev/null; then
        echo "curl is already available."
    else
        echo "curl is not found. Installing..."
        
        # Install curl using apt (for Debian/Ubuntu systems)
        sudo apt update && sudo apt install -y curl
    fi
}

install_opencv() {
    echo "🔄 Detecting Linux distribution..."
    
    if command -v apt &> /dev/null; then
        echo "📦 Installing OpenCV on Debian/Ubuntu-based system..."
        sudo apt update && sudo apt install -y libopencv-dev
    elif command -v dnf &> /dev/null; then
        echo "📦 Installing OpenCV on Fedora..."
        sudo dnf install -y opencv opencv-devel
    elif command -v pacman &> /dev/null; then
        echo "📦 Installing OpenCV on Arch Linux..."
        sudo pacman -Sy --noconfirm opencv
    elif command -v zypper &> /dev/null; then
        echo "📦 Installing OpenCV on OpenSUSE..."
        sudo zypper install -y opencv-devel
    else
        echo "❌ Unsupported package manager. Install OpenCV manually."
        return 1
    fi

    echo "✅ OpenCV installation complete."
}

install_glfw() {
    # Set up directories
    DOWNLOAD_DIR="$(realpath ../include)"
    GLFW_DIR="$DOWNLOAD_DIR/GLFW"

    mkdir -p "$GLFW_DIR"

    # Download glfw3.h from the official repository
    GLFW_URL="https://raw.githubusercontent.com/glfw/glfw/master/include/GLFW/glfw3.h"
    GLFW_FILE_PATH="$GLFW_DIR/glfw3.h"

    echo "📥 Downloading glfw3.h from GLFW official repository..."
    if curl -fsSL "$GLFW_URL" -o "$GLFW_FILE_PATH"; then
        echo "✅ Successfully downloaded glfw3.h."
    else
        echo "❌ Failed to download glfw3.h. Exiting."
        return 1
    fi

    # Detect package manager and install GLFW
    echo "🔄 Checking if GLFW is installed..."

    if command -v pacman &> /dev/null; then
        GLFW_INSTALLED=$(pacman -Qs glfw | grep -i "glfw")
        if [[ -n "$GLFW_INSTALLED" ]]; then
            echo "✅ GLFW is already installed."
        else
            echo "📦 Installing GLFW with pacman..."
            sudo pacman -S --noconfirm glfw-wayland
        fi
    elif command -v apt &> /dev/null; then
        if dpkg -l | grep -q libglfw3-dev; then
            echo "✅ GLFW is already installed."
        else
            echo "📦 Installing GLFW with apt..."
            sudo apt update && sudo apt install -y libglfw3-dev
        fi
    elif command -v dnf &> /dev/null; then
        if dnf list --installed | grep -q glfw; then
            echo "✅ GLFW is already installed."
        else
            echo "📦 Installing GLFW with dnf..."
            sudo dnf install -y glfw glfw-devel
        fi
    elif command -v zypper &> /dev/null; then
        if zypper se --installed-only glfw | grep -q glfw; then
            echo "✅ GLFW is already installed."
        else
            echo "📦 Installing GLFW with zypper..."
            sudo zypper install -y glfw-devel
        fi
    else
        echo "❌ Unsupported package manager. Install GLFW manually."
        return 1
    fi

    echo "✅ GLFW installation complete."
}

download_files() {
    echo "📥 Downloading necessary files..."

    # List of files to download
    files=(
        "https://raw.githubusercontent.com/nicholashanson/sim/refs/heads/main/grid.hpp"
        "https://raw.githubusercontent.com/nicholashanson/performance_profiler/refs/heads/main/profiler.hpp"
        "https://raw.githubusercontent.com/nicholashanson/performance_profiler/refs/heads/main/profile_manager.hpp"
        "https://raw.githubusercontent.com/nicholashanson/performance_profiler/refs/heads/main/performance_profile.hpp"
        "https://raw.githubusercontent.com/nicholashanson/performance_profiler/refs/heads/main/generate_graph.hpp"
        "https://raw.githubusercontent.com/nicholashanson/performance_profiler/refs/heads/main/fixture.hpp"
        "https://raw.githubusercontent.com/nicholashanson/performance_profiler/refs/heads/main/average_time_profiler.hpp"
    )

    # Set up the download directory
    DOWNLOAD_DIR="$(realpath ../include)"
    mkdir -p "$DOWNLOAD_DIR"

    # Download each file
    for file in "${files[@]}"; do
        file_name=$(basename "$file")
        file_path="$DOWNLOAD_DIR/$file_name"

        echo "🔽 Downloading $file_name..."
        if curl -fsSL "$file" -o "$file_path"; then
            echo "✅ Successfully downloaded $file_name."
        else
            echo "❌ Failed to download $file_name. Exiting."
            return 1
        fi
    done

    echo "✅ All files downloaded."
}

download_glm() {
    local url="https://github.com/g-truc/glm/archive/refs/tags/0.9.9.8.zip"
    local destination="../include/glm-0.9.9.8.zip"

    # Define the include directory where 'glm' should be installed
    local include_dir="$(realpath ../include)"
    local glm_destination_path="$include_dir/glm"

    # Check if GLM directory already exists
    if [ -d "$glm_destination_path" ]; then
        echo "'glm' directory already exists in $include_dir. Skipping download and installation."
        return
    fi

    # Ensure the include directory exists
    mkdir -p "$include_dir"

    echo "📥 Downloading GLM library to $destination..."
    if curl -L "$url" -o "$destination"; then
        echo "✅ Successfully downloaded GLM."
    else
        echo "❌ Failed to download GLM. Exiting."
        return 1
    fi

    echo "📦 Extracting GLM..."
    if unzip -q "$destination" -d "$include_dir"; then
        echo "✅ Successfully extracted GLM."
    else
        echo "❌ Failed to extract GLM. Exiting."
        return 1
    fi

    # Define the extracted folder name (based on the tag version)
    local extracted_folder_name="glm-0.9.9.8"
    local extracted_folder_path="$include_dir/$extracted_folder_name"

    # Move the 'glm' subdirectory to the include directory
    if [ -d "$extracted_folder_path/glm" ]; then
        mv "$extracted_folder_path/glm" "$glm_destination_path"
        echo "✅ Successfully moved 'glm' directory."
    else
        echo "❌ Could not find the 'glm' directory in the extracted folder. Exiting."
        return 1
    fi

    # Clean up: remove the extracted folder and zip file
    rm -rf "$extracted_folder_path" "$destination"
    echo "🧹 Cleaned up extracted files."
}

install_imgui() {
    # Get the parent directory of the current script
    local parent_directory="$(realpath ..)"

    # Define the zip file path and the repository URL
    local imgui_zip_file="$parent_directory/imgui-master.zip"
    local imgui_repo_zip_url="https://github.com/ocornut/imgui/archive/refs/heads/master.zip"

    # Check if imgui-master already exists
    if [ -d "$parent_directory/imgui-master" ]; then
        echo "✅ imgui-master already exists in the parent directory. Skipping installation."
        return
    fi

    echo "📥 Downloading imgui-master.zip from GitHub..."
    if curl -L "$imgui_repo_zip_url" -o "$imgui_zip_file"; then
        echo "✅ Download completed."
    else
        echo "❌ Failed to download imgui-master.zip. Exiting."
        return 1
    fi

    echo "📦 Extracting imgui-master.zip..."
    if unzip -q "$imgui_zip_file" -d "$parent_directory"; then
        echo "✅ Successfully extracted ImGui."
    else
        echo "❌ Failed to extract ImGui. Exiting."
        return 1
    fi

    # Remove the zip file after extraction
    rm -f "$imgui_zip_file"
    echo "🧹 Removed the downloaded zip file."

    echo "🎨 ImGui installation completed."
}

install_tbb() {
    echo "🔄 Installing TBB (Threading Building Blocks)..."

    if command -v tbb >/dev/null 2>&1; then
        echo "✅ TBB is already installed."
        return
    fi

    # Detect package manager and install TBB accordingly
    if command -v apt >/dev/null 2>&1; then
        sudo apt update && sudo apt install -y libtbb-dev
    elif command -v pacman >/dev/null 2>&1; then
        sudo pacman -S --noconfirm tbb
    elif command -v dnf >/dev/null 2>&1; then
        sudo dnf install -y tbb-devel
    elif command -v zypper >/dev/null 2>&1; then
        sudo zypper install -y tbb-devel
    else
        echo "❌ Unsupported package manager. Install TBB manually."
        return 1
    fi

    echo "✅ TBB installation complete."
}

compile_code() {
    echo "🚀 Compiling program..."

    # Compiler flags
    local gpp_args="-g -O0 -v -std=c++23"

    # Source files
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

    # Include directories
    local includes=(
        "../include"
        "../inline"
        "../imgui-master"
        "../imgui-master/backends"
    )

    # Output file name
    local output_file = "fs"

    local opencv_cflags = $( pkg-config --cflags opencv4 )
    local opencv_libs = " -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs"

    # Build command
    local compile_command = "g++ $gpp_args -o $output_file ${ files[ * ] }"

    # Add include directories
    for include in "${includes[@]}"; do
        compile_command+=" -I$(realpath "$include")"
    done

    # Add required libraries
    compile_command+=" $opencv_cflags $opencv_libs -lGL -lglfw -ltbb"

    # Print command for debugging
    echo "🛠️ Compiling with: $compile_command"

    # Execute the build
    eval $compile_command

    echo "✅ Compilation complete."
}

check_gpp_version
setup_googletest
download_and_unzip_mdspan
install_curl
install_opencv
install_glfw
download_files
download_glm
install_imgui
install_tbb
compile_codeg++