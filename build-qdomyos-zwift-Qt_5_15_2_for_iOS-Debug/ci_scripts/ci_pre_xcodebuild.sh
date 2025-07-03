#!/bin/bash
set -e

echo "=== QDomyos-Zwift CI Pre-Xcodebuild Script ==="
echo "Running qmake to generate Xcode project with MOC files"

# Set Qt environment
export QT_DIR="/usr/local/opt/qt@5.15"
export PATH="$QT_DIR/bin:$PATH"

# Change to project root directory
cd ../..

# Verify we're in the correct directory
if [[ ! -f "qdomyos-zwift.pro" ]]; then
    echo "ERROR: qdomyos-zwift.pro not found. Are we in the right directory?"
    pwd
    ls -la
    exit 1
fi

echo "Current directory: $(pwd)"
echo "Running qmake for iOS Debug build..."

# Run qmake to generate Xcode project with MOC files
# CONFIG+=debug for debug build
# CONFIG+=device for device target (not simulator)
qmake -spec macx-ios-clang CONFIG+=debug CONFIG+=device

echo "qmake completed successfully"

# Now restore WatchOS companion app references
echo "Restoring WatchOS companion app references..."
echo "Performing git checkout of build directory..."

# Git reset della cartella build come fai tu manualmente
git checkout HEAD -- build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/

echo "WatchOS references restored successfully"

# Verify the Xcode project exists
if [[ -f "build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/qdomyoszwift.xcodeproj/project.pbxproj" ]]; then
    echo "Xcode project found and ready for build"
else
    echo "ERROR: Xcode project not found after qmake"
    exit 1
fi

echo "Pre-xcodebuild setup completed successfully"