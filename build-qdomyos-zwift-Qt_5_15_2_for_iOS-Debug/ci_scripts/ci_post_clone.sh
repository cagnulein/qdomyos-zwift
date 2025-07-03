#!/bin/bash
set -e

echo "=== QDomyos-Zwift CI Post Clone Script ==="
echo "Installing Qt 5.15.2 and preparing environment"

# Exit if not on macOS (sanity check)
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo "ERROR: This script must run on macOS"
    exit 1
fi

# Install Qt 5.15.2 using specific Homebrew formula
echo "Installing Qt 5.15.2 via Homebrew with specific formula..."

# Apply Xcode Cloud network workarounds
export HOMEBREW_NO_AUTO_UPDATE=1
export GIT_HTTP_MAX_REQUESTS=1

# Check if Qt is already installed
if command -v qmake &> /dev/null; then
    QT_VERSION=$(qmake -v | grep -o "5\.[0-9]*\.[0-9]*" | head -1)
    if [[ "$QT_VERSION" == "5.15.2" ]]; then
        echo "Qt 5.15.2 already installed"
        export QT_DIR=$(dirname $(dirname $(which qmake)))
        export PATH="$QT_DIR/bin:$PATH"
    else
        echo "Wrong Qt version found: $QT_VERSION, installing correct version..."
    fi
fi

# Install Qt 5.15.2 using the specific formula you found
if ! command -v qmake &> /dev/null || [[ "$(qmake -v | grep -o "5\.[0-9]*\.[0-9]*" | head -1)" != "5.15.2" ]]; then
    echo "Installing Qt 5.15.2 using specific Homebrew formula..."
    
    # Create temporary formula file with the specific Qt 5.15.2 formula
    TEMP_FORMULA="/tmp/qt_5_15_2.rb"
    curl -s "https://raw.githubusercontent.com/Homebrew/homebrew-core/359cb0857099cdfa8b9ce8f421c680c9829dfe81/Formula/qt%405.rb" -o "$TEMP_FORMULA"
    
    # Install from the specific formula
    brew install --formula "$TEMP_FORMULA" || echo "Homebrew installation failed, trying alternatives..."
    
    # Alternative: Try installing qt@5 and verify version
    if ! command -v qmake &> /dev/null; then
        echo "Trying qt@5 formula..."
        brew install qt@5 || echo "qt@5 installation failed"
    fi
    
    # Set Qt environment variables
    # Try different possible paths for Qt installation
    QT_PATHS=(
        "/usr/local/opt/qt@5"
        "/usr/local/Cellar/qt@5"
        "/usr/local/opt/qt"
        "/opt/homebrew/opt/qt@5"
        "/opt/homebrew/Cellar/qt@5"
    )
    
    for qt_path in "${QT_PATHS[@]}"; do
        if [[ -d "$qt_path" ]] && [[ -f "$qt_path/bin/qmake" ]]; then
            export QT_DIR="$qt_path"
            export PATH="$QT_DIR/bin:$PATH"
            echo "Using Qt from: $QT_DIR"
            break
        fi
    done
fi

# Final verification of Qt installation
echo "Verifying Qt installation..."
if command -v qmake &> /dev/null; then
    QT_VERSION=$(qmake -v | grep -o "5\.[0-9]*\.[0-9]*" | head -1)
    echo "Qt version found: $QT_VERSION"
    qmake -v
    
    # Show Qt installation path
    QT_INSTALL_PATH=$(dirname $(dirname $(which qmake)))
    echo "Qt installed at: $QT_INSTALL_PATH"
    
    # Copy patched Qt libraries from project to Qt installation
    echo "Copying patched Qt Bluetooth libraries..."
    QT_LIB_DIR="$QT_INSTALL_PATH/lib"
    PATCHED_LIBS_DIR="../../qt-patches/ios/5.15.2/binary"
    
    if [[ -d "$PATCHED_LIBS_DIR" ]]; then
        echo "Found patched libraries at: $PATCHED_LIBS_DIR"
        ls -la "$PATCHED_LIBS_DIR"
        
        # Create backup of original libraries
        if [[ -f "$QT_LIB_DIR/libQt5Bluetooth.a" ]]; then
            echo "Backing up original Qt Bluetooth libraries..."
            cp "$QT_LIB_DIR/libQt5Bluetooth.a" "$QT_LIB_DIR/libQt5Bluetooth.a.backup" || echo "Backup failed"
        fi
        
        # Copy patched libraries
        echo "Copying patched libraries..."
        cp -f "$PATCHED_LIBS_DIR"/libQt5Bluetooth* "$QT_LIB_DIR/" && echo "Patched libraries copied successfully" || echo "Warning: Could not copy patched libraries"
        
        # Verify copy
        echo "Verifying patched libraries:"
        ls -la "$QT_LIB_DIR"/libQt5Bluetooth*
    else
        echo "Warning: Patched libraries not found at $PATCHED_LIBS_DIR"
        echo "Current directory: $(pwd)"
        echo "Searching for patched libraries..."
        find . -name "libQt5Bluetooth*" || echo "No patched libraries found"
    fi
    
    echo "Qt 5.15.2 installation and patching completed successfully"
else
    echo "ERROR: Qt installation failed"
    echo "Available Homebrew packages:"
    brew list | grep -i qt || echo "No Qt packages found"
    echo "System paths:"
    echo $PATH
    exit 1
fi

echo "Post-clone setup completed successfully"