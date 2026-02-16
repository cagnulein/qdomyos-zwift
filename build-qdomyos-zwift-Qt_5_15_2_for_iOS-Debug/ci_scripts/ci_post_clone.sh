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

            # CRITICAL: Fix hardcoded paths in .pri files
            # The Qt archive contains .pri files with absolute paths from local machine
            # Replace them with the Xcode Cloud installation path
            echo "Fixing hardcoded paths in Qt .pri files..."
            find /tmp/Qt-5.15.2 -name "*.pri" -type f -exec sed -i '' 's|/Users/cagnulein/Qt/5.15.2|/tmp/Qt-5.15.2|g' {} \;
            find /tmp/Qt-5.15.2 -name "*.pri" -type f -exec sed -i '' 's|/Users/cagnulein/Qt/5.15.2|/private/tmp/Qt-5.15.2|g' {} \;
            echo "Fixed paths in .pri files"

            # CRITICAL: Download missing qmldbg libraries
            echo "Downloading missing qmldbg libraries..."
            cd /tmp

            # Download libqmldbg_debugger.a
            echo "Downloading libqmldbg_debugger.a.zip..."
            curl -L -o libqmldbg_debugger.a.zip https://github.com/cagnulein/qt5.15.2/releases/download/qt-5.15.2/libqmldbg_debugger.a.zip
            unzip -o libqmldbg_debugger.a.zip

            # Download libqmldbg_nativedebugger.a (from the old zip)
            echo "Downloading libqmldbg_nativedebugger.zip..."
            curl -L -o libqmldbg_nativedebugger.zip https://github.com/cagnulein/qt5.15.2/releases/download/qt-5.15.2/libqmldbg_debugger.zip
            unzip -o libqmldbg_nativedebugger.zip

            echo "Contents after extraction:"
            ls -la libqmldbg*.a 2>/dev/null || echo "No .a files found in current directory"

            # Ensure target directory exists
            mkdir -p /tmp/Qt-5.15.2/ios/plugins/qmltooling

            # Move libqmldbg_debugger.a
            if [[ -f "libqmldbg_debugger.a" ]]; then
                mv libqmldbg_debugger.a /tmp/Qt-5.15.2/ios/plugins/qmltooling/
                echo "SUCCESS: Moved libqmldbg_debugger.a"
            else
                echo "FATAL ERROR: libqmldbg_debugger.a not found after extraction"
                exit 1
            fi

            # Move libqmldbg_nativedebugger.a (rename from _debug version if needed)
            if [[ -f "libqmldbg_nativedebugger.a" ]]; then
                mv libqmldbg_nativedebugger.a /tmp/Qt-5.15.2/ios/plugins/qmltooling/
                echo "SUCCESS: Moved libqmldbg_nativedebugger.a"
            elif [[ -f "libqmldbg_nativedebugger_debug.a" ]]; then
                # Use debug version as fallback (better than nothing)
                mv libqmldbg_nativedebugger_debug.a /tmp/Qt-5.15.2/ios/plugins/qmltooling/libqmldbg_nativedebugger.a
                echo "WARNING: Used libqmldbg_nativedebugger_debug.a as fallback"
            else
                echo "FATAL ERROR: libqmldbg_nativedebugger.a not found after extraction"
                exit 1
            fi

            echo "Installed missing qmldbg libraries"
            rm -f libqmldbg_debugger.a.zip libqmldbg_nativedebugger.zip

            # Verify httpserver module is now findable
            if [[ -f "/tmp/Qt-5.15.2/ios/mkspecs/modules-inst/qt_lib_httpserver.pri" ]]; then
                echo "SUCCESS: httpserver module .pri file found"
                grep "QT.httpserver.libs" /tmp/Qt-5.15.2/ios/mkspecs/modules-inst/qt_lib_httpserver.pri | head -1
            else
                echo "WARNING: httpserver .pri file not found at expected location"
                find /tmp/Qt-5.15.2 -name "*httpserver*.pri" 2>/dev/null || echo "No httpserver .pri files found"
            fi
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

# CRITICAL: Generate secret.h from Xcode Cloud environment variables
echo "Generating secret.h from environment variables..."
cd "$CI_PRIMARY_REPOSITORY_PATH/src"

cat > secret.h << EOF
#define STRAVA_SECRET_KEY ${STRAVA_SECRET_KEY:-""}
#define PELOTON_SECRET_KEY ${PELOTON_SECRET_KEY:-""}
#define SMTP_USERNAME ${SMTP_USERNAME:-""}
#define SMTP_PASSWORD ${SMTP_PASSWORD:-""}
#define SMTP_SERVER ${SMTP_SERVER:-""}
#define INTERVALSICU_CLIENT_ID ${INTERVALSICU_CLIENT_ID:-""}
#define INTERVALSICU_CLIENT_SECRET ${INTERVALSICU_CLIENT_SECRET:-""}
EOF

echo "secret.h generated successfully"

# Generate cesium-key.js if cesiumkey is provided
if [[ -n "${CESIUMKEY}" ]]; then
    echo "Generating cesium-key.js..."
    echo "${CESIUMKEY}" > inner_templates/googlemaps/cesium-key.js
    echo "cesium-key.js generated successfully"
else
    echo "CESIUMKEY not provided, skipping cesium-key.js generation"
fi

cd "$CI_PRIMARY_REPOSITORY_PATH"

# CRITICAL FIX: Disable legacy build locations to enable Swift Package support
# This must be done BEFORE xcodebuild -resolvePackageDependencies is called
echo "Configuring Xcode project to disable legacy build locations..."
cd "$CI_PRIMARY_REPOSITORY_PATH/build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug"

# Create xcshareddata directory if it doesn't exist
mkdir -p qdomyoszwift.xcodeproj/project.xcworkspace/xcshareddata

# Create WorkspaceSettings.xcsettings to disable legacy build locations
cat > qdomyoszwift.xcodeproj/project.xcworkspace/xcshareddata/WorkspaceSettings.xcsettings << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>BuildSystemType</key>
	<string>Latest</string>
	<key>BuildLocationStyle</key>
	<string>UseAppPreferences</string>
</dict>
</plist>
EOF

# Create IDEWorkspaceChecks.plist
cat > qdomyoszwift.xcodeproj/project.xcworkspace/xcshareddata/IDEWorkspaceChecks.plist << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>IDEDidComputeMac32BitWarning</key>
	<true/>
</dict>
</plist>
EOF

echo "Workspace settings created - modern build system enabled"

# Remove SYMROOT from project.pbxproj to disable legacy build locations
if [[ -f "qdomyoszwift.xcodeproj/project.pbxproj" ]]; then
    echo "Removing SYMROOT settings from project.pbxproj..."
    sed -i '' '/SYMROOT = /d' qdomyoszwift.xcodeproj/project.pbxproj
    sed -i '' '/OBJROOT = /d' qdomyoszwift.xcodeproj/project.pbxproj
    echo "SYMROOT removed - legacy build locations disabled"
else
    echo "WARNING: project.pbxproj not found"
fi

cd "$CI_PRIMARY_REPOSITORY_PATH"

echo "Post-clone setup completed successfully - Qt 5.15.2 EXACTLY installed"