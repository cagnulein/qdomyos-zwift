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
        
        # CRITICAL: Save Qt path to persistent file for next script
        echo "Saving existing Qt installation path for ci_pre_xcodebuild.sh..."
        echo "export QT_DIR=\"$QT_DIR\"" > /tmp/qt_env.sh
        echo "export PATH=\"$QT_DIR/bin:/tmp/Qt-5.15.2/ios/bin:/private/tmp/Qt-5.15.2/ios/bin:\$PATH\"" >> /tmp/qt_env.sh
        chmod +x /tmp/qt_env.sh
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
        
        # CRITICAL: Save Qt path to persistent file for next script
        echo "Saving aqt Qt installation path for ci_pre_xcodebuild.sh..."
        echo "export QT_DIR=\"/usr/local/Qt/5.15.2/clang_64\"" > /tmp/qt_env.sh
        echo "export PATH=\"/usr/local/Qt/5.15.2/clang_64/bin:/tmp/Qt-5.15.2/ios/bin:/private/tmp/Qt-5.15.2/ios/bin:\$PATH\"" >> /tmp/qt_env.sh
        chmod +x /tmp/qt_env.sh
    else
        echo "aqt failed, using precompiled Qt 5.15.2 from GitHub..."
        
        # Download precompiled Qt 5.15.2 from your GitHub release
        echo "Downloading precompiled Qt 5.15.2 from GitHub..."
        cd /tmp
        curl -L "https://github.com/cagnulein/qt5.15.2/releases/download/qt-5.15.2/qt-5.15.2.tar.xz" -o qt-5.15.2.tar.xz
        
        if [[ -f "qt-5.15.2.tar.xz" ]]; then
            echo "Extracting precompiled Qt 5.15.2..."
            tar -mxf qt-5.15.2.tar.xz
            
            cd 5.15.2 || { echo "Extraction failed or directory not found"; exit 1; }

            # Debug: Check extraction result
            echo "Contents after extraction:"
            ls -la
            
            # Install to temp location (no sudo needed)
            echo "Setting up Qt 5.15.2..."
            mkdir -p /tmp/Qt-5.15.2
            
            # Files are extracted directly - copy Qt directories
            echo "Files extracted directly, copying Qt directories..."
            
            # Copy the Qt directories we need
            if [[ -d "ios" ]]; then
                cp -R ios /tmp/Qt-5.15.2/
                echo "Copied ios directory"
            fi
            
            if [[ -d "clang_64" ]]; then
                cp -R clang_64 /tmp/Qt-5.15.2/
                echo "Copied clang_64 directory"
            fi
            
            if [[ -d "qthttpserver" ]]; then
                cp -R qthttpserver /tmp/Qt-5.15.2/
                echo "Copied qthttpserver directory"
            fi
            
            if [[ -f "sha1s.txt" ]]; then
                cp sha1s.txt /tmp/Qt-5.15.2/
                echo "Copied sha1s.txt"
            fi
            
            # Set environment for iOS development - support both /tmp and /private/tmp
            export QT_DIR="/tmp/Qt-5.15.2/ios"
            export PATH="$QT_DIR/bin:$PATH"
            
            # CRITICAL: Save Qt path to persistent file for next script
            echo "Saving Qt installation path for ci_pre_xcodebuild.sh..."
            echo "export QT_DIR=\"/tmp/Qt-5.15.2/ios\"" > /tmp/qt_env.sh
            echo "export PATH=\"/tmp/Qt-5.15.2/ios/bin:/private/tmp/Qt-5.15.2/ios/bin:\$PATH\"" >> /tmp/qt_env.sh
            chmod +x /tmp/qt_env.sh
            
            echo "Qt 5.15.2 precompiled installation completed"

            # CRITICAL: Create symlink to match local development path
            # The Qt archive contains .pri files with absolute paths from local machine
            # Create symlink so qmake can find httpserver and other modules
            echo "Creating symlink for Qt path compatibility..."
            sudo mkdir -p /Users/cagnulein/Qt/5.15.2
            sudo ln -sfn /tmp/Qt-5.15.2/ios /Users/cagnulein/Qt/5.15.2/ios
            echo "Symlink created: /Users/cagnulein/Qt/5.15.2/ios -> /tmp/Qt-5.15.2/ios"
            ls -la /Users/cagnulein/Qt/5.15.2/ || echo "Symlink creation failed"
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