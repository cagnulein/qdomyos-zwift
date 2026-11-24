#!/usr/bin/env bash
# Exercise the installed Qt helper with the same split packaging mechanism as Play.
# No Bluetooth hardware or running QZ UI is required. All existing emulator tests
# subsequently run from a fresh install of the ordinary APK.
set -euo pipefail
package=org.cagnulen.qdomyoszwift
evidence=split_bluetooth_evidence
work=$(mktemp -d)
mkdir -p "$evidence"
cleanup() {
    adb logcat -d > "$evidence/logcat.txt" 2>&1 || true
    adb uninstall "$package" >/dev/null 2>&1 || true
    rm -rf "$work"
}
trap cleanup EXIT
adb uninstall "$package" >/dev/null 2>&1 || true
adb shell cmd location set-location-enabled false
adb logcat -c
keytool -genkeypair -keyalg RSA -keysize 2048 -validity 2 -alias test \
    -keystore "$work/test.keystore" -storepass changeit -keypass changeit \
    -dname "CN=QZ CI Split Test" -noprompt
java -jar bundletool.jar build-apks --bundle=aab-debug/android-debug.aab \
    --output="$work/qz.apks" --connected-device \
    --ks="$work/test.keystore" --ks-key-alias=test --ks-pass=pass:changeit --key-pass=pass:changeit
java -jar bundletool.jar install-apks --apks="$work/qz.apks"
adb shell pm path "$package" | tr -d '\r' | tee "$evidence/installed_paths.txt"
test "$(grep -c '^package:' "$evidence/installed_paths.txt")" -gt 1

sdk=$(adb shell getprop ro.build.version.sdk | tr -d '\r')
if [ "$sdk" -ge 31 ]; then
    adb shell pm grant "$package" android.permission.BLUETOOTH_SCAN
    adb shell pm grant "$package" android.permission.BLUETOOTH_CONNECT
fi
adb shell dumpsys package "$package" > "$evidence/package.txt"
adb shell am instrument -w -r "$package/.SplitBluetoothTestInstrumentation" \
    | tee "$evidence/instrumentation.txt"
grep -Fq 'INSTRUMENTATION_RESULT: qz_split_bluetooth=PASS' "$evidence/instrumentation.txt"
grep -Fq 'INSTRUMENTATION_CODE: -1' "$evidence/instrumentation.txt"
echo "Split APK Bluetooth permission regression passed on API $sdk"
