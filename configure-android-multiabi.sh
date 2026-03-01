#!/bin/bash
# Configure CMake for Android Multi-ABI (WSL/Linux version)
# This script configures the project for all 4 Android architectures: armeabi-v7a, arm64-v8a, x86, x86_64

echo
echo "============================================"
echo "  Configuring Android Multi-ABI Project (WSL)"
echo "============================================"
echo

# Set paths (adjust these to match your WSL setup)
QT_DIR="/mnt/c/Qt/6.8.2"
ANDROID_SDK_ROOT="/mnt/c/Users/$USER/AppData/Local/Android/Sdk"

# For WSL, we need to check if we have Linux NDK or use Windows NDK with explicit compiler
ANDROID_NDK_ROOT="$ANDROID_SDK_ROOT/ndk/25.1.8937393"

# Fallback to other NDK versions if preferred one doesn't exist
if [ ! -d "$ANDROID_NDK_ROOT" ]; then
    ANDROID_NDK_ROOT="$ANDROID_SDK_ROOT/ndk/26.1.10909125"
fi
if [ ! -d "$ANDROID_NDK_ROOT" ]; then
    ANDROID_NDK_ROOT="$ANDROID_SDK_ROOT/ndk/21.3.6528147"
fi

# Check if we have the Linux toolchain, otherwise use Windows toolchain
LINUX_CLANG="$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++"
WINDOWS_CLANG="$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/windows-x86_64/bin/clang++.exe"

if [ -f "$LINUX_CLANG" ]; then
    TOOLCHAIN_HOST="linux-x86_64"
    CLANG_EXECUTABLE="$LINUX_CLANG"
elif [ -f "$WINDOWS_CLANG" ]; then
    TOOLCHAIN_HOST="windows-x86_64"
    CLANG_EXECUTABLE="$WINDOWS_CLANG"
    echo "NOTE: Using Windows NDK toolchain from WSL"
else
    echo "ERROR: Neither Linux nor Windows clang++ found in NDK"
    echo "Looked for:"
    echo "  $LINUX_CLANG"
    echo "  $WINDOWS_CLANG"
    exit 1
fi

# Verify paths
if [ ! -d "$QT_DIR/android_arm64_v8a" ]; then
    echo "ERROR: Qt Android not found at $QT_DIR"
    echo "Please adjust QT_DIR in this script"
    exit 1
fi

if [ ! -d "$ANDROID_NDK_ROOT" ]; then
    echo "ERROR: Android NDK not found at $ANDROID_NDK_ROOT"
    echo "Please install NDK through Android Studio or adjust ANDROID_NDK_ROOT"
    exit 1
fi

echo "Using Qt: $QT_DIR"
echo "Using NDK: $ANDROID_NDK_ROOT"
echo "Using Clang: $CLANG_EXECUTABLE"
echo

# Clean previous build directories
if [ -d "build-android-multiabi" ]; then
    echo "Cleaning previous multi-ABI build..."
    rm -rf build-android-multiabi
fi

# Create build directory
mkdir -p build-android-multiabi
cd build-android-multiabi

echo
echo "============================================"
echo "  Configuring CMake for Multi-ABI"
echo "============================================"
echo

# Configure CMake with explicit multi-ABI settings (WSL paths)
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake" \
    -DQT_HOST_PATH="$QT_DIR/msvc2022_64" \
    -DCMAKE_PREFIX_PATH="$QT_DIR/android_arm64_v8a" \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-23 \
    -DANDROID_STL=c++_shared \
    -DCMAKE_BUILD_TYPE=Debug \
    -DQT_ANDROID_BUILD_ALL_ABIS=ON \
    -DQT_ANDROID_ABIS="armeabi-v7a;arm64-v8a;x86;x86_64" \
    -DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=BOTH \
    -DANDROID_SDK_ROOT="$ANDROID_SDK_ROOT" \
    -DANDROID_NDK_ROOT="$ANDROID_NDK_ROOT" \
    -DCMAKE_CXX_COMPILER="$CLANG_EXECUTABLE" \
    -DCMAKE_C_COMPILER="${CLANG_EXECUTABLE%++}"

if [ $? -ne 0 ]; then
    echo
    echo "ERROR: CMake configuration failed!"
    echo "Check the error messages above."
    cd ..
    exit 1
fi

echo
echo "============================================"
echo "  Configuration successful!"
echo "============================================"
echo

# Go back to project root
cd ..

echo "Project configured for Android Multi-ABI build."
echo "Build directory: build-android-multiabi"
echo
echo "To build from command line:"
echo "  cd build-android-multiabi"
echo "  cmake --build . --parallel"
echo
echo "To open in Qt Creator:"
echo "  1. Open Qt Creator"
echo "  2. File -> Open File or Project"
echo "  3. Select: $(pwd)/build-android-multiabi/CMakeCache.txt"
echo "  4. Or just open the CMakeLists.txt and select the configured build"
echo