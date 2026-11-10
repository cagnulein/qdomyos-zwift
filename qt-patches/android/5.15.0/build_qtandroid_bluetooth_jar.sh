#!/usr/bin/env bash
# Rebuild the Qt Bluetooth LE implementation and manifest helper while preserving
# all unrelated classes in the custom QtAndroidBluetooth.jar.
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
src="$here/qtconnectivity/src/android/bluetooth/src/org/qtproject/qt5/android/bluetooth"
package=org/qtproject/qt5/android/bluetooth

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

javac -source 8 -target 8 -Xlint:-options -bootclasspath "$androidjar" \
    -cp "$orig:$(dirname "$orig")/QtAndroid.jar" -d "$work/classes" \
    "$src/QtBluetoothLE.java" "$src/QtBluetoothUtility.java"

# Do not leave old inner classes behind if the source and input jar differ.
jar tf "$orig" | grep -E "^$package/QtBluetoothLE(\\\$[^/]+)?\\.class$" | sort > "$work/old-classes"
(cd "$work/classes" && find "$package" -type f -name 'QtBluetoothLE*.class' | sort) > "$work/new-classes"
diff -u "$work/old-classes" "$work/new-classes"

cp "$orig" "$work/QtAndroidBluetooth.jar"
(cd "$work/classes" && jar uf "$work/QtAndroidBluetooth.jar" \
    $(cat "$work/new-classes") "$package/QtBluetoothUtility.class")
jar tf "$work/QtAndroidBluetooth.jar" | grep -Fqx "$package/QtBluetoothUtility.class"

cp "$work/QtAndroidBluetooth.jar" "$out"
echo "Written $out"
