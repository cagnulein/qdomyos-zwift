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
        echo "aqt failed, using local Homebrew Qt 5.15.2 formula..."
        
        # Method 2: Comprehensive Qt 5.15.2 installation with extensive debugging
        echo "=== COMPREHENSIVE Qt 5.15.2 INSTALLATION DEBUG ==="
        
        # DEBUG: Show current state
        echo "Current directory: $(pwd)"
        echo "Available Qt installations:"
        find /usr/local /opt/homebrew -name "qmake" 2>/dev/null || echo "No qmake found"
        
        # Go back to project root
        cd ../..
        echo "Project root: $(pwd)"
        
        # Try Method 2A: dmuth/core tap with fixes
        echo "--- METHOD 2A: dmuth/core tap ---"
        brew tap dmuth/core https://github.com/dmuth/homebrew-core.git || echo "Tap already exists"
        
        # DEBUG: Show tap information
        echo "Tap repository location: $(brew --repository dmuth/core)"
        cd $(brew --repository dmuth/core)
        echo "Current commit: $(git rev-parse HEAD)"
        
        # Reset to specific commit
        git reset --hard 7cc83b1e58fcd3026f0fb7d4130b1d36d9649b47 || echo "Reset failed"
        echo "After reset commit: $(git rev-parse HEAD)"
        
        # DEBUG: List all formulas
        echo "Available formulas in tap:"
        ls -la Formula/ || echo "No Formula directory"
        
        # Try multiple formula names
        FORMULA_CANDIDATES=("qt5.rb" "qt.rb" "qt@5.rb")
        FORMULA_FILE=""
        
        for candidate in "${FORMULA_CANDIDATES[@]}"; do
            if [[ -f "Formula/$candidate" ]]; then
                echo "Found formula: Formula/$candidate"
                FORMULA_FILE="Formula/$candidate"
                break
            fi
        done
        
        if [[ -n "$FORMULA_FILE" ]]; then
            echo "Using formula: $FORMULA_FILE"
            
            # DEBUG: Show formula content before fix
            echo "Formula content (first 30 lines):"
            head -30 "$FORMULA_FILE"
            
            # Backup and fix bottle syntax
            cp "$FORMULA_FILE" "$FORMULA_FILE.backup"
            
            # Multiple fix strategies
            echo "Applying bottle syntax fixes..."
            
            # Fix 1: Remove entire bottle section
            sed -i '' '/bottle do/,/^  end$/d' "$FORMULA_FILE" || echo "Fix 1 failed"
            
            # Fix 2: Remove cellar: syntax specifically
            sed -i '' 's/cellar: :any,//g' "$FORMULA_FILE" || echo "Fix 2 failed"
            sed -i '' 's/cellar: :any_skip_relocation,//g' "$FORMULA_FILE" || echo "Fix 3 failed"
            
            # DEBUG: Show formula after fixes
            echo "Formula after fixes (first 30 lines):"
            head -30 "$FORMULA_FILE"
            
            # Try installation
            echo "Installing Qt from fixed formula..."
            brew install --build-from-source dmuth/core/qt5 || brew install --build-from-source dmuth/core/qt || echo "dmuth installation failed"
        else
            echo "No Qt formula found in dmuth/core tap"
        fi
        
        # Try Method 2B: Direct formula download and install
        if ! command -v qmake &> /dev/null; then
            echo "--- METHOD 2B: Direct formula download ---"
            cd /tmp
            
            # Try downloading the original formula
            echo "Downloading original Qt@5 formula..."
            curl -f "https://raw.githubusercontent.com/Homebrew/homebrew-core/359cb0857099cdfa8b9ce8f421c680c9829dfe81/Formula/qt%405.rb" -o qt5_original.rb || echo "Original formula download failed"
            
            if [[ -f "qt5_original.rb" ]]; then
                # Fix the downloaded formula
                echo "Fixing downloaded formula..."
                sed '/bottle do/,/^  end$/d' qt5_original.rb > qt5_fixed.rb
                
                # Try to install it
                echo "Installing from fixed downloaded formula..."
                brew install --build-from-source ./qt5_fixed.rb || echo "Downloaded formula installation failed"
            fi
        fi
        
        # NO METHOD 2C - Never use official qt@5 as it won't be 5.15.2 exactly
        
        # DEBUG: Final Qt status
        echo "=== FINAL Qt STATUS ==="
        if command -v qmake &> /dev/null; then
            echo "qmake found at: $(which qmake)"
            echo "Qt version: $(qmake -v)"
            echo "QT_DIR: $QT_DIR"
        else
            echo "ERROR: No qmake found after all attempts"
            echo "Homebrew packages containing 'qt':"
            brew list | grep -i qt || echo "No Qt packages found"
            echo "System-wide qmake search:"
            find /usr /opt -name "qmake" 2>/dev/null || echo "No qmake found system-wide"
        fi
        
        # Verify it's the right version
        QT_PATHS=(
            "/usr/local/opt/qt@5"
            "/usr/local/Cellar/qt@5/5.15.2"
            "/opt/homebrew/opt/qt@5"
            "/opt/homebrew/Cellar/qt@5/5.15.2"
        )
        
        QT_FOUND=false
        for qt_path in "${QT_PATHS[@]}"; do
            if [[ -d "$qt_path" ]] && [[ -f "$qt_path/bin/qmake" ]]; then
                INSTALLED_VERSION=$("$qt_path/bin/qmake" -v | grep -o "5\.[0-9]*\.[0-9]*" | head -1)
                if [[ "$INSTALLED_VERSION" == "5.15.2" ]]; then
                    export QT_DIR="$qt_path"
                    export PATH="$QT_DIR/bin:$PATH"
                    echo "FOUND Qt 5.15.2 at: $QT_DIR"
                    QT_FOUND=true
                    break
                else
                    echo "Wrong version at $qt_path: $INSTALLED_VERSION"
                fi
            fi
        done
        
        if [[ "$QT_FOUND" == "false" ]]; then
            echo "ERROR: Could not install Qt 5.15.2 exactly"
            echo "Available Qt installations:"
            find /usr/local /opt/homebrew -name "qmake" 2>/dev/null || echo "No qmake found"
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
        echo "ERROR: Patched libraries not found at $PATCHED_LIBS_DIR"
        echo "Current directory: $(pwd)"
        exit 1
    fi
    
    echo "Qt 5.15.2 installation and patching completed successfully"
else
    echo "FATAL ERROR: No qmake found after installation"
    exit 1
fi

echo "Post-clone setup completed successfully - Qt 5.15.2 EXACTLY installed"