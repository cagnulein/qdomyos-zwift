#!/bin/sh
# App start test for the ios-simulator-test job.
# Installs the simulator build on an iPhone simulator, starts QZ and checks that it is still running
# a minute later. Screenshots, the QZ debug log, QML warnings and crash reports go to ios-test/.
#
# usage: sh .github/scripts/ios_simulator_start_test.sh <xcodebuild derived data dir>

DD=$1
OUT=ios-test
WAIT=60   # seconds after the start: the x86_64 build runs under Rosetta, the first start is slow
mkdir -p "$OUT"

# Save the evidence; on failure first, the job stops there
collect() {
  if [ -n "$UDID" ]; then
    xcrun simctl io "$UDID" screenshot "$OUT/screenshot-end.png"
  fi
  ls ~/Library/Logs/DiagnosticReports/ > "$OUT/crash-list.txt" 2>&1
  cp ~/Library/Logs/DiagnosticReports/"$EXE"* "$OUT/" 2>/dev/null
  if [ -n "$DATA" ]; then
    find "$DATA" -name 'debug-*.log' -exec cp {} "$OUT/" \;
  fi
  # QML errors and warnings of the debug log, also in the job output
  cat "$OUT"/debug-*.log 2>/dev/null | grep -F "qrc:/" > "$OUT/qml-warnings.txt"
  echo "=== QML warnings: $(wc -l < "$OUT/qml-warnings.txt") (first 50) ==="
  head -n 50 "$OUT/qml-warnings.txt"
}

fail() {
  echo "TEST FAILED: $1"
  collect
  exit 1
}

APP=$(find "$DD/Build/Products" -maxdepth 2 -type d -name '*.app' -path '*iphonesimulator*' | grep -v -i watch | head -n 1)
[ -n "$APP" ] || fail "no simulator app in $DD/Build/Products"
PLIST="$APP/Info.plist"
BID=$(/usr/libexec/PlistBuddy -c 'Print :CFBundleIdentifier' "$PLIST")
EXE=$(/usr/libexec/PlistBuddy -c 'Print :CFBundleExecutable' "$PLIST")
echo "app: $APP ($BID)"
file "$APP/$EXE"

# simctl install refuses an app without a CFBundleVersion; an unsigned app does not start
VERSION=$(/usr/libexec/PlistBuddy -c 'Print :CFBundleVersion' "$PLIST" 2>/dev/null)
case "$VERSION" in
  '' | *'$'*)
    /usr/libexec/PlistBuddy -c 'Delete :CFBundleVersion' "$PLIST" 2>/dev/null
    /usr/libexec/PlistBuddy -c 'Add :CFBundleVersion string 1' "$PLIST"
    ;;
esac
codesign --force --deep --sign - "$APP" || fail "codesign failed"

# The first iPhone of the newest iOS runtime
UDID=$(xcrun simctl list devices available -j | python3 -c '
import json, sys
devices = json.load(sys.stdin)["devices"]
for runtime in sorted(devices, reverse=True):
    if "iOS" in runtime:
        for device in devices[runtime]:
            if device["name"].startswith("iPhone"):
                print(device["udid"], device["name"], runtime, file=sys.stderr)
                print(device["udid"])
                sys.exit()
')
[ -n "$UDID" ] || fail "no iPhone simulator"
xcrun simctl boot "$UDID"
xcrun simctl bootstatus "$UDID" -b || fail "the simulator did not boot"
xcrun simctl install "$UDID" "$APP" || fail "simctl install failed"

# QSettings on iOS: the CFPreferences domain com.<organizationDomain, dots as dashes>.<applicationName>
# (main.cpp) in the app container. A last device closes the first-run wizard, the Apple Watch fake
# device shows the tiles without a bike, log_debug writes the debug log
DATA=$(xcrun simctl get_app_container "$UDID" "$BID" data)
PREFS="$DATA/Library/Preferences/com.robertoviola-cloud.qDomyos-Zwift.plist"
mkdir -p "$DATA/Library/Preferences"
/usr/libexec/PlistBuddy -c 'Add :bluetooth_lastdevice_name string TestBike' "$PREFS"
/usr/libexec/PlistBuddy -c 'Add :applewatch_fakedevice bool true' "$PREFS"
/usr/libexec/PlistBuddy -c 'Add :log_debug bool true' "$PREFS"

# simctl launch prints "<bundle id>: <pid>"
LAUNCH=$(xcrun simctl launch --stdout="$PWD/$OUT/stdout.txt" --stderr="$PWD/$OUT/stderr.txt" "$UDID" "$BID") || fail "simctl launch failed"
echo "$LAUNCH"
PID=${LAUNCH##*: }
sleep "$WAIT"
xcrun simctl io "$UDID" screenshot "$OUT/screenshot.png"
ps -p "$PID" > /dev/null || fail "the app (pid $PID) is not running $WAIT s after the start"
if ls ~/Library/Logs/DiagnosticReports/ | grep -q -F "$EXE"; then
  fail "a crash report of $EXE"
fi
echo "App is running successfully"
collect
