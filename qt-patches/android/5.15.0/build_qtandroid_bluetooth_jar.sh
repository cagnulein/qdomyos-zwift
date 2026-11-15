#!/usr/bin/env bash
# Adds the Qt Bluetooth manifest helper while preserving every existing class
# in the custom QtAndroidBluetooth.jar.
#
# Usage: build_qtandroid_bluetooth_jar.sh <original QtAndroidBluetooth.jar> <android.jar> <output jar>
# The Android platform jar can be API 21 or newer.
set -euo pipefail

if [ $# -ne 3 ]; then
    echo "Usage: $0 <original QtAndroidBluetooth.jar> <android.jar> <output jar>" >&2
    exit 1
fi

here=$(cd "$(dirname "$0")" && pwd)
orig=$1
androidjar=$2
out=$3
src="$here/qtconnectivity/src/android/bluetooth/src/org/qtproject/qt5/android/bluetooth/QtBluetoothUtility.java"
cls=org/qtproject/qt5/android/bluetooth/QtBluetoothUtility.class

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

javac -source 8 -target 8 -Xlint:-options -bootclasspath "$androidjar" -cp "$orig" \
    -d "$work/classes" "$src"

cp "$orig" "$work/QtAndroidBluetooth.jar"
(cd "$work/classes" && jar uf "$work/QtAndroidBluetooth.jar" "$cls")
jar tf "$work/QtAndroidBluetooth.jar" | grep -Fqx "$cls"

cp "$work/QtAndroidBluetooth.jar" "$out"
echo "Written $out"
