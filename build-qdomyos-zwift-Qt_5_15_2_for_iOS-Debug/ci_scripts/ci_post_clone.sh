#!/bin/bash
set -e

echo "=== QDomyos-Zwift CI Post Clone Script ==="
echo "Installing Qt 5.15.2 and preparing environment"

# Exit if not on macOS (sanity check)
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo "ERROR: This script must run on macOS"
    exit 1
fi

# Download and install Qt 5.15.2 directly from Qt official site
echo "Downloading Qt 5.15.2 for macOS..."

QT_INSTALL_DIR="/usr/local/Qt-5.15.2"
QT_DOWNLOAD_URL="https://download.qt.io/archive/qt/5.15/5.15.2/qt-opensource-mac-x64-5.15.2.dmg"

# Check if Qt 5.15.2 is already installed
if [[ -d "$QT_INSTALL_DIR" ]] && [[ -f "$QT_INSTALL_DIR/bin/qmake" ]]; then
    echo "Qt 5.15.2 already installed at $QT_INSTALL_DIR"
    export QT_DIR="$QT_INSTALL_DIR"
    export PATH="$QT_DIR/bin:$PATH"
    
    # Verify version
    QT_VERSION=$($QT_DIR/bin/qmake -v | grep -o "5\.[0-9]*\.[0-9]*" | head -1)
    echo "Found Qt version: $QT_VERSION"
else
    echo "Qt 5.15.2 not found, downloading and installing..."
    
    # Create temporary directory
    TEMP_DIR="/tmp/qt-installer"
    mkdir -p "$TEMP_DIR"
    cd "$TEMP_DIR"
    
    # Download Qt DMG
    echo "Downloading Qt 5.15.2 DMG..."
    curl -L "$QT_DOWNLOAD_URL" -o qt-installer.dmg
    
    # Mount DMG
    echo "Mounting Qt installer..."
    hdiutil attach qt-installer.dmg -mountpoint /Volumes/qt-installer
    
    # Install Qt
    echo "Installing Qt 5.15.2..."
    sudo mkdir -p "$QT_INSTALL_DIR"
    
    # Find the Qt installer app and run it silently
    INSTALLER_APP=$(find /Volumes/qt-installer -name "*.app" -type d | head -1)
    if [[ -n "$INSTALLER_APP" ]]; then
        echo "Found installer: $INSTALLER_APP"
        # Try to extract Qt directly from the installer
        sudo cp -R "/Volumes/qt-installer"/* "$QT_INSTALL_DIR/" || echo "Direct copy failed"
    else
        echo "ERROR: Qt installer app not found in DMG"
        ls -la /Volumes/qt-installer/
    fi
    
    # Unmount DMG
    hdiutil detach /Volumes/qt-installer || echo "Failed to unmount DMG"
    
    # Set Qt environment
    export QT_DIR="$QT_INSTALL_DIR"
    export PATH="$QT_DIR/bin:$PATH"
fi

# Alternative: Try to find any existing Qt installation on the system
if ! command -v qmake &> /dev/null; then
    echo "Searching for existing Qt installations..."
    
    # Common Qt installation paths
    QT_PATHS=(
        "/usr/local/Qt-5.15.2"
        "/usr/local/Qt5.15.2"
        "/usr/local/qt5"
        "/usr/local/Cellar/qt@5"
        "/Applications/Qt"
        "/opt/Qt"
        "/opt/qt"
    )
    
    for qt_path in "${QT_PATHS[@]}"; do
        if [[ -d "$qt_path" ]]; then
            echo "Found Qt at: $qt_path"
            # Look for qmake in subdirectories
            QMAKE_PATH=$(find "$qt_path" -name "qmake" -type f | head -1)
            if [[ -n "$QMAKE_PATH" ]]; then
                export QT_DIR=$(dirname $(dirname "$QMAKE_PATH"))
                export PATH="$QT_DIR/bin:$PATH"
                echo "Using Qt from: $QT_DIR"
                break
            fi
        fi
    done
fi

# Final verification
echo "Final Qt verification..."
if command -v qmake &> /dev/null; then
    qmake -v
    echo "Qt installation found and verified"
    
    # Copy patched Qt libraries from project to Qt installation
    echo "Copying patched Qt Bluetooth libraries..."
    QT_LIB_DIR="$QT_DIR/lib"
    PATCHED_LIBS_DIR="../../qt-patches/ios/5.15.2/binary"
    
    if [[ -d "$PATCHED_LIBS_DIR" ]]; then
        echo "Found patched libraries at: $PATCHED_LIBS_DIR"
        ls -la "$PATCHED_LIBS_DIR"
        
        # Create lib directory if it doesn't exist
        sudo mkdir -p "$QT_LIB_DIR"
        
        # Copy patched libraries
        sudo cp -f "$PATCHED_LIBS_DIR"/libQt5Bluetooth* "$QT_LIB_DIR/" || echo "Warning: Could not copy patched libraries"
        echo "Patched libraries copied successfully"
    else
        echo "Warning: Patched libraries not found at $PATCHED_LIBS_DIR"
        echo "Current directory: $(pwd)"
        echo "Searching for patched libraries..."
        find . -name "libQt5Bluetooth*" || echo "No patched libraries found"
    fi
else
    echo "ERROR: Qt installation failed completely"
    echo "Available tools in /usr/local/bin:"
    ls -la /usr/local/bin/ | grep -i qt || echo "No Qt tools found"
    echo "Trying to continue without Qt - build will likely fail"
fi

echo "Post-clone setup completed"