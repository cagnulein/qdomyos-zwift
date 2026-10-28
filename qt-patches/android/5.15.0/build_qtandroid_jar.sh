#!/bin/bash
# Rebuilds jar/QtAndroid.jar: the QtAndroid.jar of the Qt that CI builds with (stock 5.15.0
# overlaid with qt-5.15.0-android-page16k) where only QtInputConnection.class is replaced by
# the one compiled from qtbase/src/android/jar/src/ (see 0005-*.patch). Every other class
# stays byte-for-byte the original.
#
# Usage: build_qtandroid_jar.sh <original QtAndroid.jar> <android.jar, API 34 or newer> <output jar>
# Needs a JDK (javac, jar, javap). In CI the original jar is
# output/android/Qt/5.15.0/android/jar/QtAndroid.jar after the page16k overlay step.
set -euo pipefail

if [ $# -ne 3 ]; then
    echo "Usage: $0 <original QtAndroid.jar> <android.jar, API 34 or newer> <output jar>" >&2
    exit 1
fi

here=$(cd "$(dirname "$0")" && pwd)
orig=$1
androidjar=$2
out=$3
src="$here/qtbase/src/android/jar/src/org/qtproject/qt5/android/QtInputConnection.java"
cls=org/qtproject/qt5/android/QtInputConnection.class

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

# Same class file version as the original jar (Java 8).
javac -source 8 -target 8 -Xlint:-options -bootclasspath "$androidjar" -cp "$orig" \
    -d "$work/classes" "$src"

cp "$orig" "$work/QtAndroid.jar"
(cd "$work/classes" && jar uf "$work/QtAndroid.jar" "$cls")

# Expected difference: only the methods added by the patch.
javap -p -cp "$orig" org.qtproject.qt5.android.QtInputConnection > "$work/before.txt"
javap -p -cp "$work/QtAndroid.jar" org.qtproject.qt5.android.QtInputConnection > "$work/after.txt"
echo "QtInputConnection members, original -> patched:"
diff "$work/before.txt" "$work/after.txt" || true

cp "$work/QtAndroid.jar" "$out"
echo "Written $out"
