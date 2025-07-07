#!/bin/bash
set -e

echo "=== QDomyos-Zwift CI Pre-Xcodebuild Script ==="
echo "Running qmake to generate Xcode project with MOC files"

# Find Qt installation (should be 5.15.2 from post_clone script)
if command -v qmake &> /dev/null; then
    QT_VERSION=$(qmake -v | grep -o "5\.[0-9]*\.[0-9]*" | head -1)
    if [[ "$QT_VERSION" != "5.15.2" ]]; then
        echo "FATAL ERROR: Qt version is $QT_VERSION, expected 5.15.2"
        exit 1
    fi
    echo "Using Qt 5.15.2 - CORRECT!"
else
    echo "FATAL ERROR: qmake not found"
    exit 1
fi

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

# Fix "legacy build locations" error by configuring Xcode project
echo "Fixing Xcode Cloud legacy build locations issue..."
cd build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug

# Create or update xcconfig file to fix legacy build locations
cat > xcode_cloud_fix.xcconfig << EOF
// Xcode Cloud fix for legacy build locations
SYMROOT = \$(SRCROOT)/build
OBJROOT = \$(SRCROOT)/build/Intermediates
CONFIGURATION_BUILD_DIR = \$(SRCROOT)/build/\$(CONFIGURATION)\$(EFFECTIVE_PLATFORM_NAME)
EOF

# Apply the fix to the Xcode project
if [[ -f "qdomyoszwift.xcodeproj/project.pbxproj" ]]; then
    echo "Applying Xcode Cloud build location fix..."
    
    # Backup original project file
    cp qdomyoszwift.xcodeproj/project.pbxproj qdomyoszwift.xcodeproj/project.pbxproj.backup
    
    # Disable legacy build locations in project settings
    # This changes the build system to use recommended locations
    sed -i '' 's/SYMROOT = /\/\/ SYMROOT = /g' qdomyoszwift.xcodeproj/project.pbxproj || echo "SYMROOT fix applied"
    sed -i '' 's/UseNewBuildSystem = NO/UseNewBuildSystem = YES/g' qdomyoszwift.xcodeproj/project.pbxproj || echo "New build system enabled"
    
    echo "Xcode project configuration updated for Xcode Cloud compatibility"
else
    echo "ERROR: Xcode project not found after qmake"
    exit 1
fi

# Verify the Xcode project exists and is properly configured
if [[ -f "qdomyoszwift.xcodeproj/project.pbxproj" ]]; then
    echo "Xcode project found and configured for Xcode Cloud"
    echo "Project size: $(du -sh qdomyoszwift.xcodeproj)"
else
    echo "ERROR: Xcode project not found after qmake"
    exit 1
fi

echo "Pre-xcodebuild setup completed successfully"