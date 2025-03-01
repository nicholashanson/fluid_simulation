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

# Run the check
check_gpp_version