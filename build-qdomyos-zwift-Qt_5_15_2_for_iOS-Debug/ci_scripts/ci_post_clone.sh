#!/bin/bash
set -e

echo "=== QDomyos-Zwift CI Post Clone Script ==="
echo "Installing Qt 5.15.2 EXACTLY and preparing environment"

# Exit if not on macOS (sanity check)
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo "ERROR: This script must run on macOS"
    exit 1
fi

# Apply Xcode Cloud network workarounds
export HOMEBREW_NO_AUTO_UPDATE=1
export GIT_HTTP_MAX_REQUESTS=1

# Check if Qt 5.15.2 is already installed
if command -v qmake &> /dev/null; then
    QT_VERSION=$(qmake -v | grep -o "5\.[0-9]*\.[0-9]*" | head -1)
    if [[ "$QT_VERSION" == "5.15.2" ]]; then
        echo "Qt 5.15.2 already installed - PERFECT!"
        export QT_DIR=$(dirname $(dirname $(which qmake)))
        export PATH="$QT_DIR/bin:$PATH"
    else
        echo "WRONG Qt version found: $QT_VERSION"
        echo "MUST install Qt 5.15.2 exactly"
        # Uninstall wrong version
        brew uninstall --ignore-dependencies qt@5 qt || echo "No Qt to uninstall"
    fi
fi

# Force install Qt 5.15.2 EXACTLY
if ! command -v qmake &> /dev/null || [[ "$(qmake -v | grep -o "5\.[0-9]*\.[0-9]*" | head -1)" != "5.15.2" ]]; then
    echo "Installing Qt 5.15.2 EXACTLY - NO OTHER VERSION ACCEPTED"
    
    # Method 1: Use aqt (Another Qt Installer) to get exact version
    echo "Installing aqt (Another Qt Installer) for exact Qt version control..."
    python3 -m pip install aqt || echo "aqt installation failed, trying homebrew method"
    
    if command -v aqt &> /dev/null; then
        echo "Using aqt to install Qt 5.15.2 exactly..."
        aqt install-qt mac desktop 5.15.2 --outputdir /usr/local/Qt
        export QT_DIR="/usr/local/Qt/5.15.2/clang_64"
        export PATH="$QT_DIR/bin:$PATH"
    else
        echo "aqt failed, using precompiled Qt 5.15.2 from GitHub..."
        
        # Download precompiled Qt 5.15.2 from your GitHub release
        echo "Downloading precompiled Qt 5.15.2 from GitHub..."
        cd /tmp
        curl -L "https://github.com/cagnulein/qt5.15.2/releases/download/qt-5.15.2/qt-5.15.2.tar.xz" -o qt-5.15.2.tar.xz
        
        if [[ -f "qt-5.15.2.tar.xz" ]]; then
            echo "Extracting precompiled Qt 5.15.2..."
            tar -xf qt-5.15.2.tar.xz
            
            # Install to standard location
            echo "Installing Qt 5.15.2..."
            sudo mkdir -p /usr/local/Qt-5.15.2
            sudo cp -R qt-5.15.2/* /usr/local/Qt-5.15.2/
            
            # Set environment for iOS development
            export QT_DIR="/usr/local/Qt-5.15.2/ios"
            export PATH="$QT_DIR/bin:$PATH"
            
            echo "Qt 5.15.2 precompiled installation completed"
        else
            echo "ERROR: Failed to download precompiled Qt from GitHub"
            exit 1
        fi
        
    fi
fi

# MANDATORY verification - FAIL if not 5.15.2
echo "MANDATORY Qt 5.15.2 verification..."
if command -v qmake &> /dev/null; then
    QT_VERSION=$(qmake -v | grep -o "5\.[0-9]*\.[0-9]*" | head -1)
    if [[ "$QT_VERSION" != "5.15.2" ]]; then
        echo "FATAL ERROR: Qt version is $QT_VERSION, NOT 5.15.2"
        echo "Build CANNOT continue with wrong Qt version"
        exit 1
    fi
    
    echo "SUCCESS: Qt 5.15.2 verified!"
    qmake -v
    
    # Show Qt installation path
    QT_INSTALL_PATH=$(dirname $(dirname $(which qmake)))
    echo "Qt 5.15.2 installed at: $QT_INSTALL_PATH"
    
    echo "Qt 5.15.2 installation completed successfully (Bluetooth already patched)"
else
    echo "FATAL ERROR: No qmake found after installation"
    exit 1
fi

echo "Post-clone setup completed successfully - Qt 5.15.2 EXACTLY installed"