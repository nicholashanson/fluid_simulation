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

check_gpp_version
download_and_unzip_mdspan
install_curl