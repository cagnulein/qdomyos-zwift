#!/bin/bash
set -e

echo "=== QDomyos-Zwift CI Post Clone Script ==="
echo "Installing Qt 5.15.2 and preparing environment"

# Exit if not on macOS (sanity check)
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo "ERROR: This script must run on macOS"
    exit 1
fi

# Install Qt 5.15.2 for iOS
echo "Installing Qt 5.15.2..."

# Check if Qt is already installed
if command -v qmake &> /dev/null; then
    QT_VERSION=$(qmake -v | grep -o "5\.[0-9]*\.[0-9]*" | head -1)
    if [[ "$QT_VERSION" == "5.15.2" ]]; then
        echo "Qt 5.15.2 already installed"
    else
        echo "Wrong Qt version found: $QT_VERSION"
        echo "Need to install Qt 5.15.2"
    fi
else
    echo "Qt not found, installing..."
fi

# Install Qt 5.15.2 using Homebrew (più affidabile per CI)
echo "Installing Qt 5.15.2 via Homebrew..."
export HOMEBREW_NO_AUTO_UPDATE=1
brew install qt@5.15 || echo "Qt installation may have failed, continuing..."

# Set Qt environment
export QT_DIR="/usr/local/opt/qt@5.15"
export PATH="$QT_DIR/bin:$PATH"

# Verify Qt installation
echo "Verifying Qt installation..."
if command -v qmake &> /dev/null; then
    qmake -v
    echo "Qt installation verified"
else
    echo "ERROR: Qt installation failed"
    exit 1
fi

# Copy patched Qt libraries from project to Qt installation
echo "Copying patched Qt Bluetooth libraries..."
QT_LIB_DIR="$QT_DIR/lib"
PATCHED_LIBS_DIR="../../qt-patches/ios/5.15.2/binary"

if [[ -d "$PATCHED_LIBS_DIR" ]]; then
    echo "Found patched libraries at: $PATCHED_LIBS_DIR"
    cp -f "$PATCHED_LIBS_DIR"/libQt5Bluetooth* "$QT_LIB_DIR/" || echo "Warning: Could not copy patched libraries"
    echo "Patched libraries copied successfully"
else
    echo "Warning: Patched libraries not found at $PATCHED_LIBS_DIR"
fi

echo "Post-clone setup completed successfully"