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
    echo "Ensuring OpenCV is installed..."

    # Check if 'curl' is available (dependency for some package managers)
    if ! command -v curl &> /dev/null; then
        echo "curl is not installed. Installing curl first..."
        sudo apt-get update && sudo apt-get install -y curl
    fi

    # Detect distribution and install OpenCV accordingly
    if [[ -f /etc/os-release ]]; then
        # Load distribution name from /etc/os-release
        DISTRO_NAME=$(awk -F= '/^NAME/{print $2}' /etc/os-release | tr -d '"')

        case $DISTRO_NAME in
            *"Ubuntu"* | *"Debian"*)
                # For Ubuntu/Debian-based systems
                echo "Detected Ubuntu/Debian-based distribution"
                sudo apt-get update
                sudo apt-get install -y libopencv-dev
                echo "OpenCV installed on Ubuntu/Debian."
                ;;

            *"Fedora"*)
                # For Fedora
                echo "Detected Fedora"
                sudo dnf install -y opencv opencv-devel
                echo "OpenCV installed on Fedora."
                ;;

            *"CentOS"* | *"Red Hat"*)
                # For CentOS/RedHat
                echo "Detected CentOS/Red Hat"
                sudo yum install -y opencv opencv-devel
                echo "OpenCV installed on CentOS/Red Hat."
                ;;

            *"Arch"* | *"Manjaro"*)
                # For Arch/Manjaro
                echo "Detected Arch-based system"
                sudo pacman -S --noconfirm opencv
                echo "OpenCV installed on Arch-based system."
                ;;

            *)
                echo "Unsupported distribution: $DISTRO_NAME"
                exit 1
                ;;
        esac
    else
        echo "Unable to detect Linux distribution. Exiting."
        exit 1
    fi
}

check_gpp_version
download_and_unzip_mdspan
install_curl
install_opencv