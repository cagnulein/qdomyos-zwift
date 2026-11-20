#!/usr/bin/env bash
# Build a tiny, isolated mail receiver APK for the existing Android emulator matrix.
# Does not need a Gradle project, publish a build, or use production signing keys.
set -euo pipefail

SDK="${ANDROID_SDK_ROOT:-${ANDROID_HOME:-/usr/local/lib/android/sdk}}"
TOOLS="$SDK/build-tools/35.0.0"
ANDROID_JAR="$SDK/platforms/android-35/android.jar"
SRC=".github/android-support-mail-probe"
WORK="build/android-support-mail-probe"
mkdir -p "$WORK/classes" "$WORK/dex"

test -f "$ANDROID_JAR"
test -x "$TOOLS/aapt"
test -x "$TOOLS/d8"
test -x "$TOOLS/apksigner"

javac -source 8 -target 8 -bootclasspath "$ANDROID_JAR" \
  -d "$WORK/classes" "$SRC/src/org/qz/supportmailprobe/MailProbeActivity.java"

"$TOOLS/d8" --min-api 24 --lib "$ANDROID_JAR" \
  --output "$WORK/dex" "$WORK/classes/org/qz/supportmailprobe/MailProbeActivity.class"
"$TOOLS/aapt" package -f -M "$SRC/AndroidManifest.xml" -I "$ANDROID_JAR" \
  -F "$WORK/probe-unsigned.apk"
(cd "$WORK/dex" && zip -q -j "../probe-unsigned.apk" classes.dex)
"$TOOLS/zipalign" -f 4 "$WORK/probe-unsigned.apk" "$WORK/probe-aligned.apk"

# Temporary debug-only key for the mail-probe APK; never shipped in QZ.
keytool -genkeypair -keyalg RSA -keysize 2048 -validity 2 -alias test \
  -keystore "$WORK/test.keystore" -storepass changeit -keypass changeit \
  -dname "CN=QZ CI Mail Probe" -noprompt >/dev/null 2>&1
"$TOOLS/apksigner" sign --ks "$WORK/test.keystore" --ks-key-alias test \
  --ks-pass pass:changeit --key-pass pass:changeit \
  --out support-mail-probe.apk "$WORK/probe-aligned.apk"
"$TOOLS/apksigner" verify --min-sdk-version 24 support-mail-probe.apk
echo "Built support-mail-probe.apk for Android API 24+"
