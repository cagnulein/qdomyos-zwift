#!/bin/bash
set -e

echo "=== QDomyos-Zwift CI Post-Xcodebuild Script ==="
echo "Build completed, performing cleanup and verification"

# Change to project root directory
cd ../..

# Check if build was successful
if [[ -d "build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/Debug-iphoneos" ]]; then
    echo "✅ iOS Debug build appears successful"
    echo "Build artifacts found:"
    ls -la build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/Debug-iphoneos/
else
    echo "❌ Build artifacts not found in expected location"
    echo "Checking alternative locations..."
    find build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug -name "*.app" -type d || echo "No .app bundles found"
fi

# Log build information
echo "Build information:"
echo "- Qt Version: $(qmake -v | grep Qt | head -1)"
echo "- Xcode Version: $(xcodebuild -version | head -1)"
echo "- Build Date: $(date)"

# Optional: Archive build artifacts for distribution
if [[ -d "build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/Debug-iphoneos/qdomyoszwift.app" ]]; then
    echo "App bundle found, build successful"
    echo "App bundle contents:"
    ls -la build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/Debug-iphoneos/qdomyoszwift.app/
fi

echo "Post-xcodebuild script completed"