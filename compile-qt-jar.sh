#!/bin/bash

# Script to compile QtBluetoothLE.java and update the JAR
# Usage: ./compile-qt-jar.sh

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
QT_PATCHES_DIR="$SCRIPT_DIR/qt-patches/android/5.15.0"
JAR_FILE="$QT_PATCHES_DIR/jar/QtAndroidBluetooth.jar"
SOURCE_FILE="$QT_PATCHES_DIR/qtconnectivity/src/android/bluetooth/src/org/qtproject/qt5/android/bluetooth/QtBluetoothLE.java"
WORK_DIR="/tmp/qt-jar-compile-$$"
ANDROID_JAR="/tmp/android.jar"

echo "=== QtBluetoothLE JAR Compiler ==="
echo ""

# Check if source file exists
if [ ! -f "$SOURCE_FILE" ]; then
    echo "ERROR: Source file not found: $SOURCE_FILE"
    exit 1
fi

# Download Android SDK jar if not present
if [ ! -f "$ANDROID_JAR" ]; then
    echo "Downloading Android SDK jar..."
    curl -L -o "$ANDROID_JAR" "https://github.com/Sable/android-platforms/raw/master/android-31/android.jar"
fi

# Create backup of original JAR
echo "Creating backup of original JAR..."
cp "$JAR_FILE" "$JAR_FILE.backup"

# Create work directory
echo "Creating work directory..."
mkdir -p "$WORK_DIR"
cd "$WORK_DIR"

# Extract original JAR (to get all other classes and META-INF)
echo "Extracting original JAR..."
jar xf "$JAR_FILE.backup"

# Copy modified source
echo "Copying modified source..."
cp "$SOURCE_FILE" org/qtproject/qt5/android/bluetooth/

# Compile
echo "Compiling QtBluetoothLE.java..."
javac -source 1.8 -target 1.8 \
    -bootclasspath "$ANDROID_JAR" \
    -cp "$JAR_FILE.backup" \
    org/qtproject/qt5/android/bluetooth/QtBluetoothLE.java

if [ $? -ne 0 ]; then
    echo "ERROR: Compilation failed!"
    cd "$SCRIPT_DIR"
    rm -rf "$WORK_DIR"
    exit 1
fi

# Create new JAR
echo "Creating new JAR..."
jar cf QtAndroidBluetooth-new.jar org/ META-INF/

# Replace old JAR
echo "Replacing JAR file..."
cp QtAndroidBluetooth-new.jar "$JAR_FILE"

# Cleanup
echo "Cleaning up..."
cd "$SCRIPT_DIR"
rm -rf "$WORK_DIR"

echo ""
echo "=== SUCCESS ==="
echo "JAR compiled and updated: $JAR_FILE"
echo "Backup saved as: $JAR_FILE.backup"
echo ""
