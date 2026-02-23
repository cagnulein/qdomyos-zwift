#!/bin/bash
set -e

echo "=== QDomyos-Zwift CI Pre-Xcodebuild Script ==="
echo "Running qmake to generate Xcode project with MOC files"

# CRITICAL: Load Qt environment from persistent file
echo "Loading Qt environment from ci_post_clone.sh..."
if [[ -f "/tmp/qt_env.sh" ]]; then
    echo "Found Qt environment file, loading..."
    source /tmp/qt_env.sh
    echo "Qt environment loaded from persistent file"
    echo "QT_DIR: $QT_DIR"
    echo "PATH: $PATH"
else
    echo "WARNING: No Qt environment file found, trying to find Qt anyway..."
fi

# Find Qt installation (should be 5.15.2 from post_clone script)
if command -v qmake &> /dev/null; then
    QT_VERSION=$(qmake -v | grep -o "5\.[0-9]*\.[0-9]*" | head -1)
    if [[ "$QT_VERSION" != "5.15.2" ]]; then
        echo "FATAL ERROR: Qt version is $QT_VERSION, expected 5.15.2"
        exit 1
    fi
    echo "Using Qt 5.15.2 - CORRECT!"
    echo "qmake location: $(which qmake)"
else
    echo "FATAL ERROR: qmake not found"
    echo "Current PATH: $PATH"
    echo "Listing /tmp for debugging:"
    ls -la /tmp/ | grep -i qt || echo "No Qt directories in /tmp"
    exit 1
fi

# Change to project root directory
cd ../..

# CRITICAL: Save absolute path to project root for later use
PROJECT_ROOT="$(pwd)"
export PROJECT_ROOT
echo "Project root saved: $PROJECT_ROOT"

# Verify we're in the correct directory
if [[ ! -f "qdomyos-zwift.pro" ]]; then
    echo "ERROR: qdomyos-zwift.pro not found. Are we in the right directory?"
    pwd
    ls -la
    exit 1
fi

echo "Current directory: $(pwd)"
echo "Running qmake for iOS Debug build..."

# Run qmake to generate Xcode project and Makefiles
# Use release config since precompiled Qt doesn't have debug libs
# Force iphoneos SDK for device builds (not simulator)
export QMAKE_XCODE_DEVELOPER_PATH="/Applications/Xcode.app/Contents/Developer"
export QMAKE_IOS_DEPLOYMENT_TARGET=12.0
qmake -spec macx-ios-clang CONFIG+=release CONFIG+=device CONFIG-=simulator CONFIG+=iphoneos "QMAKE_APPLE_DEVICE_ARCHS=arm64"

echo "qmake completed successfully"

# CRITICAL: Debug Qt installation before make
echo "Debugging Qt installation before make..."
echo "Checking Qt include directories:"
ls -la /tmp/Qt-5.15.2/ios/include/ 2>/dev/null || echo "No /tmp/Qt-5.15.2/ios/include/"
ls -la /private/tmp/Qt-5.15.2/ios/include/ 2>/dev/null || echo "No /private/tmp/Qt-5.15.2/ios/include/"

echo "Checking for QDebug specifically:"
find /tmp/Qt-5.15.2/ios/include/ -name "*QDebug*" 2>/dev/null || echo "QDebug not found in /tmp/"
find /private/tmp/Qt-5.15.2/ios/include/ -name "*QDebug*" 2>/dev/null || echo "QDebug not found in /private/tmp/"

echo "Checking QtCore include directory:"
ls -la /tmp/Qt-5.15.2/ios/include/QtCore/ 2>/dev/null || echo "No QtCore in /tmp/"
ls -la /private/tmp/Qt-5.15.2/ios/include/QtCore/ 2>/dev/null || echo "No QtCore in /private/tmp/"

# Setup generated-file cache directory for faster prebuilds
BUILD_CACHE_DIR="$HOME/Library/Caches/XcodeCloud/QDomyos-Zwift-Build"
mkdir -p "$BUILD_CACHE_DIR"

# CRITICAL: Create fake xcodebuild BEFORE make to prevent build failures
# During make, qmake will try to call xcodebuild which will fail due to code signing
# We create a fake xcodebuild that just returns success
echo "Creating fake xcodebuild to skip Xcode build during make..."
mkdir -p /tmp/fake_xcode
cat > /tmp/fake_xcode/xcodebuild << 'XCODE_EOF'
#!/bin/bash
echo "Skipping xcodebuild during make - will use correct project later"
exit 0
XCODE_EOF
chmod +x /tmp/fake_xcode/xcodebuild

# Prepend fake xcodebuild to PATH so it's found first
export PATH="/tmp/fake_xcode:$PATH"
echo "Fake xcodebuild created and added to PATH"
which xcodebuild

# CRITICAL: Generate only Qt auto-generated sources needed by Xcode.
# Do NOT build full objects/archives here, to avoid duplicate work in xcodebuild archive.
echo "Generating Qt auto-generated sources (moc/rcc/qmltyperegistrar only)..."

GENERATOR_SCRIPT="/tmp/qz_generated_commands.sh"
rm -f "${GENERATOR_SCRIPT}"

extract_generator_commands() {
    local dryrun_cmd="$1"
    eval "${dryrun_cmd}" 2>/dev/null \
        | sed -n -E '/(^|[[:space:]])(moc|rcc|qmltyperegistrar)([[:space:]]|$)/p' \
        | sed -n -E '/[[:space:]]-o[[:space:]][^[:space:]]+\.(cpp|json)([[:space:]]|$)/p' \
        | sed '/^[[:space:]]*$/d'
}

# Prefer the qmake-generated library Makefile where moc/rcc rules actually live.
if [[ -f "$PROJECT_ROOT/src/Makefile.qdomyos-zwift-lib" ]]; then
    extract_generator_commands "make -C \"$PROJECT_ROOT/src\" -f Makefile.qdomyos-zwift-lib -n" > "${GENERATOR_SCRIPT}"
fi

# Fallback: try top-level dry-run only if the specific Makefile did not yield commands.
if [[ ! -s "${GENERATOR_SCRIPT}" ]]; then
    extract_generator_commands "make -n" > "${GENERATOR_SCRIPT}"
fi

if [[ ! -s "${GENERATOR_SCRIPT}" ]]; then
    echo "ERROR: could not discover generator commands from Makefile dry-run"
    exit 1
fi

echo "Discovered $(wc -l < "${GENERATOR_SCRIPT}") generator commands, executing from src/..."
chmod +x "${GENERATOR_SCRIPT}"
(
    cd "$PROJECT_ROOT/src"
    bash "${GENERATOR_SCRIPT}"
)

echo "Generated-source command execution completed successfully"

# Remove fake xcodebuild from PATH
export PATH="${PATH#/tmp/fake_xcode:}"
echo "Fake xcodebuild removed from PATH"

# Cache only generated source files for next time
echo "Caching generated sources..."
mkdir -p "$BUILD_CACHE_DIR/generated"
find . \( -name "moc_*.cpp" -o -name "qrc_*.cpp" -o -name "*_qmltyperegistrations.cpp" -o -name "*_metatypes.json" \) \
    | while IFS= read -r file; do
        cp "$file" "$BUILD_CACHE_DIR/generated/" 2>/dev/null || echo "Could not cache $file"
      done
echo "Generated-source cache updated"

# NOW restore Xcode project and fix qmake corruption AFTER make
echo "Restoring Xcode project from git AFTER make..."
echo "qmake regenerates src/qdomyoszwift.xcodeproj without proper code signing"

# Return to project root for git operations (use absolute path)
cd "$PROJECT_ROOT"
echo "Back to project root: $(pwd)"

# Restore the build directory project (has WatchOS and proper code signing)
git checkout -- build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/

echo "Build directory Xcode project restored from git"

# CRITICAL: Verify Qt labs calendar library exists
echo "Verifying Qt labs calendar library..."
if [[ -f "/tmp/Qt-5.15.2/ios/qml/Qt/labs/calendar/libqtlabscalendarplugin.a" ]]; then
    echo "SUCCESS: libqtlabscalendarplugin.a found"
    ls -lh /tmp/Qt-5.15.2/ios/qml/Qt/labs/calendar/libqtlabscalendarplugin.a
else
    echo "ERROR: libqtlabscalendarplugin.a NOT FOUND"
    echo "Searching for calendar files..."
    find /tmp/Qt-5.15.2 -name "*calendar*" 2>/dev/null || echo "No calendar files found"
fi

# CRITICAL: Fix ALL paths in Xcode project for Xcode Cloud compatibility
# The project has absolute paths from local development that need to be converted
echo "Fixing all paths in Xcode project for Xcode Cloud..."
cd "$PROJECT_ROOT/build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug"
if [[ -f "qdomyoszwift.xcodeproj/project.pbxproj" ]]; then
    echo "Converting local development paths to Xcode Cloud paths..."

    # Fix all paths in correct order (specific to general)

    # 1. Fix Qt library paths (most specific)
    sed -i '' 's|/Users/cagnulein/Qt/5\.15\.2/ios/|/tmp/Qt-5.15.2/ios/|g' qdomyoszwift.xcodeproj/project.pbxproj
    sed -i '' 's|../../Qt/5\.15\.2/ios/|/tmp/Qt-5.15.2/ios/|g' qdomyoszwift.xcodeproj/project.pbxproj
    sed -i '' 's|../Qt/5\.15\.2/ios/|/tmp/Qt-5.15.2/ios/|g' qdomyoszwift.xcodeproj/project.pbxproj

    # 2. Fix source file paths to relative (must be before general fix)
    sed -i '' 's|/Users/cagnulein/qdomyos-zwift/src/|../src/|g' qdomyoszwift.xcodeproj/project.pbxproj

    # 3. Fix all other absolute paths with general replacement
    sed -i '' 's|/Users/cagnulein/qdomyos-zwift/|/Volumes/workspace/repository/|g' qdomyoszwift.xcodeproj/project.pbxproj

    # 4. Fix sourceTree for relative src paths (must be <group> not <absolute>)
    sed -i '' 's|path = "\.\./src/\([^"]*\)"; sourceTree = "<absolute>";|path = "../src/\1"; sourceTree = "<group>";|g' qdomyoszwift.xcodeproj/project.pbxproj

    echo "Fixed all paths in project file"

    # CRITICAL: Change scheme to Release configuration
    # The scheme is committed with Debug configuration but we need Release for Xcode Cloud
    echo "Changing scheme to Release configuration..."
    if [[ -f "qdomyoszwift.xcodeproj/xcshareddata/xcschemes/qdomyoszwift.xcscheme" ]]; then
        # Change TestAction from Debug to Release
        sed -i '' 's|<TestAction[^>]*buildConfiguration = "Debug"|<TestAction buildConfiguration = "Release"|g' qdomyoszwift.xcodeproj/xcshareddata/xcschemes/qdomyoszwift.xcscheme

        # Change LaunchAction from Debug to Release
        sed -i '' 's|<LaunchAction[^>]*buildConfiguration = "Debug"|<LaunchAction buildConfiguration = "Release"|g' qdomyoszwift.xcodeproj/xcshareddata/xcschemes/qdomyoszwift.xcscheme

        # Change AnalyzeAction from Debug to Release
        sed -i '' 's|<AnalyzeAction[^>]*buildConfiguration = "Debug"|<AnalyzeAction buildConfiguration = "Release"|g' qdomyoszwift.xcodeproj/xcshareddata/xcschemes/qdomyoszwift.xcscheme

        echo "Scheme changed to Release configuration"
    else
        echo "WARNING: Scheme file not found"
    fi

    # CRITICAL: Remove _debug suffix from Qt libraries
    # The Qt package only contains release libraries, not debug versions
    # Replace all lib*_debug.a references with lib*.a (release versions)
    echo "Replacing debug Qt libraries with release versions..."
    sed -i '' 's|lib\([a-zA-Z0-9_]*\)_debug\.a|lib\1.a|g' qdomyoszwift.xcodeproj/project.pbxproj
    sed -i '' 's|-l\([a-zA-Z0-9_]*\)_debug|-l\1|g' qdomyoszwift.xcodeproj/project.pbxproj
    echo "Replaced all _debug library references with release versions"

    # Add ALL necessary Qt library search paths
    # qmake generates these but they might be missing from the committed project
    echo "Adding all Qt library search paths..."
    sed -i '' 's|\(LIBRARY_SEARCH_PATHS = (\)|\1\n\t\t\t\t/tmp/Qt-5.15.2/ios/qml/Qt/labs/calendar,\n\t\t\t\t/tmp/Qt-5.15.2/ios/qml/Qt/labs/platform,\n\t\t\t\t/tmp/Qt-5.15.2/ios/qml/QtCharts,\n\t\t\t\t/tmp/Qt-5.15.2/ios/qml/QtWebView,\n\t\t\t\t/tmp/Qt-5.15.2/ios/qml/QtPositioning,\n\t\t\t\t/tmp/Qt-5.15.2/ios/qml/QtLocation,\n\t\t\t\t/tmp/Qt-5.15.2/ios/qml/QtMultimedia,\n\t\t\t\t/tmp/Qt-5.15.2/ios/plugins/platforms,\n\t\t\t\t/tmp/Qt-5.15.2/ios/plugins/webview,\n\t\t\t\t/tmp/Qt-5.15.2/ios/plugins/texttospeech,\n\t\t\t\t/tmp/Qt-5.15.2/ios/plugins/geoservices,\n\t\t\t\t/tmp/Qt-5.15.2/ios/plugins/sqldrivers,\n\t\t\t\t/tmp/Qt-5.15.2/ios/plugins/mediaservice,\n\t\t\t\t/tmp/Qt-5.15.2/ios/plugins/playlistformats,\n\t\t\t\t/tmp/Qt-5.15.2/ios/plugins/audio,|g' qdomyoszwift.xcodeproj/project.pbxproj
    echo "Added all necessary Qt library search paths"

    # Verify the fix
    grep -c "libqtlabscalendarplugin.a" qdomyoszwift.xcodeproj/project.pbxproj && echo "qtlabscalendarplugin references found"
    grep -c "labs/calendar" qdomyoszwift.xcodeproj/project.pbxproj && echo "labs/calendar path references found"
else
    echo "ERROR: project.pbxproj not found"
    exit 1
fi
cd "$PROJECT_ROOT"

# CRITICAL: Copy ALL generated files from src/ to build directory AFTER git restore
# qmake/make generates many files (moc_*.cpp, qrc_*.cpp, *.o, *.json, qmltyperegistrations, etc.) in src/
# but Xcode project expects them in build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/
# This must happen AFTER git checkout to avoid wiping out the copied files
echo "Copying ALL Qt-generated files from src/ to build directory..."
cd "$PROJECT_ROOT/src"

# Ensure stale qmltyperegistrations objects never leak into archive phase.
rm -f "$PROJECT_ROOT/build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/qdomyos-zwift_qmltyperegistrations.o"
rm -f "$PROJECT_ROOT/build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/qdomyoszwift_qmltyperegistrations.o"

# Copy generated sources (cpp/json/qmltypes) but avoid precompiled objects/archives
echo "Looking for generated files in: $(pwd)"
find . -maxdepth 1 -type f \
    \( -name "moc_*.cpp" -o -name "moc_*.cpp.json" -o -name "qrc_*.cpp" -o -name "*_qmltyperegistrations.cpp" -o -name "*.qmltypes" -o -name "*_metatypes.json" -o -name "*_plugin_import.cpp" \) \
    -print -exec cp {} "$PROJECT_ROOT/build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/" \;

echo "Generated files copied to build directory"

# CRITICAL FIX: Rename qdomyos-zwift_qmltyperegistrations.cpp to qdomyoszwift_qmltyperegistrations.cpp
# qmake generates the file with a hyphen but Xcode project expects it without hyphen
echo "Fixing qmltyperegistrations filename mismatch..."
if [[ -f "$PROJECT_ROOT/build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/qdomyos-zwift_qmltyperegistrations.cpp" ]]; then
    cp "$PROJECT_ROOT/build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/qdomyos-zwift_qmltyperegistrations.cpp" \
       "$PROJECT_ROOT/build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/qdomyoszwift_qmltyperegistrations.cpp"
    echo "Renamed qdomyos-zwift_qmltyperegistrations.cpp -> qdomyoszwift_qmltyperegistrations.cpp"
else
    echo "WARNING: qdomyos-zwift_qmltyperegistrations.cpp not found in build directory"
fi

echo "Skipping qmltyperegistrations object prebuild on purpose (compiled later by xcodebuild)"

echo "Verifying qdomyoszwift_qmltyperegistrations.cpp exists:"
ls -la "$PROJECT_ROOT/build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/qdomyoszwift_qmltyperegistrations.cpp" 2>&1

cd "$PROJECT_ROOT"

# CRITICAL FIX: Delete corrupted project in src/ and symlink to the good one
# qmake regenerates src/qdomyoszwift.xcodeproj without code signing during make
# xcodebuild will build from src/, so we symlink to the correct project in build/
echo "Removing corrupted Xcode project from src/ and creating symlink..."
if [[ -d "src/qdomyoszwift.xcodeproj" ]]; then
    rm -rf src/qdomyoszwift.xcodeproj
    echo "Corrupted project removed from src/"
fi

# Create symlink from src/ to the correct project in build/
ln -s ../build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/qdomyoszwift.xcodeproj src/qdomyoszwift.xcodeproj
echo "Symlink created: src/qdomyoszwift.xcodeproj -> ../build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/qdomyoszwift.xcodeproj"

# Verify symlink
if [[ -L "src/qdomyoszwift.xcodeproj" ]]; then
    echo "Symlink verified successfully"
    ls -la src/qdomyoszwift.xcodeproj
else
    echo "ERROR: Failed to create symlink"
    exit 1
fi

echo "Xcode project fix completed - symlink created to correct project with code signing"

# CRITICAL FIX: Disable legacy build locations to enable Swift Package support
# Create workspace settings to force modern build system
echo "Configuring workspace to disable legacy build locations..."
cd build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug

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

if [[ -f "qdomyoszwift.xcodeproj/project.pbxproj" ]]; then
    echo "Removing SYMROOT settings from project.pbxproj..."

    # Remove all SYMROOT lines completely (they cause the legacy build locations error)
    sed -i '' '/SYMROOT = /d' qdomyoszwift.xcodeproj/project.pbxproj

    # Also remove OBJROOT if present
    sed -i '' '/OBJROOT = /d' qdomyoszwift.xcodeproj/project.pbxproj

    # Ensure new build system is enabled
    sed -i '' 's/UseNewBuildSystem = NO/UseNewBuildSystem = YES/g' qdomyoszwift.xcodeproj/project.pbxproj || echo "New build system already enabled"

    echo "Legacy build locations disabled - Swift packages now supported"
else
    echo "ERROR: Xcode project not found"
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
