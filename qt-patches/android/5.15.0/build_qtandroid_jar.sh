#!/bin/bash
# Rebuilds jar/QtAndroid.jar: the QtAndroid.jar of the Qt that CI builds with (stock 5.15.0
# overlaid with qt-5.15.0-android-page16k) where only the classes of the patched sources in
# qtbase/src/android/jar/src/ are replaced by the ones compiled from them (see the 000N-*.patch
# files): QtInputConnection (0005) and QtAccessibilityDelegate with its inner classes (0007).
# Every class of a patched source is replaced, inner and anonymous ones too (QtAccessibilityDelegate$1
# ...), so they come from one compilation; every other class stays byte-for-byte the original.
#
# Usage: build_qtandroid_jar.sh <original QtAndroid.jar> <android.jar, API 34 or newer> <output jar>
# Needs a JDK (javac, jar, javap). In CI the original jar is
# output/android/Qt/5.15.0/android/jar/QtAndroid.jar after the page16k overlay step and before
# the "patching qt for bluetooth" step (that step puts this patched jar there), or jar/QtAndroid.jar
# from the page16k release zip; android.jar is in the Android SDK, platforms/android-35/android.jar.
# Give the stock jar, not jar/QtAndroid.jar of this folder: the classes are compiled anew either way,
# but only the stock one shows in the member diff below what all the patches change.
set -euo pipefail

if [ $# -ne 3 ]; then
    echo "Usage: $0 <original QtAndroid.jar> <android.jar, API 34 or newer> <output jar>" >&2
    exit 1
fi

here=$(cd "$(dirname "$0")" && pwd)
orig=$1
androidjar=$2
out=$3
srcroot="$here/qtbase/src/android/jar/src"

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

# cut drops the leading "./" (find -printf '%P' is GNU only).
(cd "$srcroot" && find . -name '*.java' | cut -c3- | sort) > "$work/sources.txt"
if [ ! -s "$work/sources.txt" ]; then
    echo "No patched sources under $srcroot" >&2
    exit 1
fi

# Same class file version as the original jar (Java 8).
(cd "$srcroot" && javac -source 8 -target 8 -Xlint:-options -bootclasspath "$androidjar" -cp "$orig" \
    -d "$work/classes" @"$work/sources.txt")

# Each compiled class must replace one that the original jar has: a new class name (an anonymous
# class numbered anew) would leave the old one in the jar beside it, out of step with its outer class.
jar tf "$orig" > "$work/orig-entries.txt"
(cd "$work/classes" && find . -name '*.class' | cut -c3- | sort) > "$work/classes.txt"
missing=$(grep -v -x -F -f "$work/orig-entries.txt" "$work/classes.txt" || true)
if [ -n "$missing" ]; then
    echo "Classes not in the original jar (renumbered or new):" >&2
    echo "$missing" >&2
    exit 1
fi

cp "$orig" "$work/QtAndroid.jar"
(cd "$work/classes" && jar uf "$work/QtAndroid.jar" @"$work/classes.txt")

# Expected difference: only the members added by the patches.
while read -r cls; do
    name=${cls%.class}
    name=${name//\//.}
    javap -p -cp "$orig" "$name" > "$work/before.txt"
    javap -p -cp "$work/QtAndroid.jar" "$name" > "$work/after.txt"
    echo "$name members, original -> patched:"
    diff "$work/before.txt" "$work/after.txt" || true
done < "$work/classes.txt"

cp "$work/QtAndroid.jar" "$out"
echo "Replaced classes:"
cat "$work/classes.txt"
echo "Written $out"
