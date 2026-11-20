#!/bin/sh
# Experimental CI probe: real Google open-source TalkBack enabled in Android.
# CI checks its accessibility-focus provider integration and TalkBack activation.
# adb shell input swipe is not recognized as a TalkBack gesture on every emulator,
# so swipe navigation still needs the manual simulator check on Claudio's machine.
set -eu

PKG=org.cagnulen.qdomyoszwift
TB_PKG=com.android.talkback
TB_SERVICE=com.google.android.marvin.talkback.TalkBackService
REPORT=talkback_real_probe.txt
: > "$REPORT"

note() { echo "$*" | tee -a "$REPORT"; }
fail() {
    note "FAIL $*"
    adb shell dumpsys accessibility > accessibility_dumpsys.txt || true
    adb logcat -d > talkback_logcat.txt || true
    adb shell screencap -p /sdcard/talkback-failure.png || true
    adb pull /sdcard/talkback-failure.png . >/dev/null 2>&1 || true
    exit 1
}
foreground() {
    focus=$(adb shell dumpsys window | grep -m1 mCurrentFocus || true)
    case "$focus" in *"$PKG"*) return 0;; *) fail "QZ lost foreground to: $focus";; esac
}
# Health Connect can show a permissions activity on first QZ startup (API 35+).
# Dismiss ONLY that known onboarding activity. Once QZ has focus, never
# auto-dismiss external apps: an unexpected Chrome launch is a test failure.
recover_startup_window() {
    i=0
    while [ "$i" -lt 5 ]; do
        focus=$(adb shell dumpsys window | grep -m1 mCurrentFocus || true)
        case "$focus" in
            *"$PKG"*) note "PASS QZ foreground after startup"; return 0 ;;
            *com.google.android.healthconnect.controller/*PermissionsActivity*|*com.android.healthconnect.controller/*PermissionsActivity*)
                note "INFO closing first-run Health Connect permission screen ($((i + 1))/5): $focus"
                adb shell input keyevent KEYCODE_BACK
                ;;
            *com.google.android.permissioncontroller/*GrantPermissionsActivity*|*com.android.permissioncontroller/*GrantPermissionsActivity*)
                # Runtime permissions are granted where possible by the normal
                # Android CI setup. The OS can still show a permission dialog
                # when the fake device initializes. Dismiss it only here,
                # before TalkBack gesture testing; never mask navigation issues.
                note "INFO closing first-run Android runtime permission screen ($((i + 1))/5): $focus"
                adb shell input keyevent KEYCODE_BACK
                ;;
            *"=null"*)
                note "INFO waiting for startup window ($((i + 1))/5)"
                ;;
            *) fail "Unexpected window on QZ startup (not dismissed): $focus" ;;
        esac
        sleep 4
        i=$((i + 1))
    done
    fail "QZ did not regain foreground after startup: $focus"
}
peek() {
    # This flag is essential. Default UiAutomation.connect() temporarily
    # suppresses real accessibility services, creating false TalkBack tests.
    adb shell "CLASSPATH=/data/local/tmp/a11y-test.jar app_process / AccessibilityTest --keep-services $*" 2>&1 | tr -d '\r'
}
focus_name() {
    # The helper is strictly read-only; the preceding swipe must move the focus.
    peek --peek-focused | grep -F "TALKBACK_FOCUS" || true
}
focus_and_activate() {
    name=$1
    result=$(peek --focus-and-activate "$name")
    note "$result"
    echo "$result" | grep -F 'OK TalkBack focus and ACTION_CLICK on' >/dev/null ||
        fail "TalkBack could not focus and activate $name"
    foreground
    sleep 4
    foreground
}
require_named_node() {
    name=$1
    result=$(peek --peek-node "$name")
    note "$result"
    echo "$result" | grep -F 'TALKBACK_NODE' >/dev/null ||
        fail "Toolbar control has no accessible name: $name"
}
require_named_node_either() {
    first=$1
    second=$2
    result=$(peek --peek-node "$first")
    if echo "$result" | grep -F 'TALKBACK_NODE' >/dev/null; then
        note "$result"
        return 0
    fi
    result=$(peek --peek-node "$second")
    note "$result"
    echo "$result" | grep -F 'TALKBACK_NODE' >/dev/null ||
        fail "Toolbar control has no accessible name: $first / $second"
}
focus_without_activate() {
    name=$1
    result=$(peek "focus:$name")
    note "$result"
    expected_name=$(echo "$name" | tr '_' ' ')
    echo "$result" | grep -F "OK accessible focus reached '$expected_name'" >/dev/null ||
        fail "TalkBack could not focus $name without activation"
    foreground
}
note "Starting experimental REAL TalkBack focus and activation check"

# This test MUST fail if the official open-source TalkBack APK did not install.
adb shell pm path "$TB_PKG" | grep -q 'package:' || fail "TalkBack is not installed"
# A clean TalkBack install launches its welcome/onboarding window the first time
# the service starts. QZ is launched over that window below, leaving TalkBack's
# accessibility focus in onboarding instead of QZ. Seed the two first-run
# flags so this probe starts directly in the same service used for navigation.
cat > talkback-default-prefs.xml <<'EOF'
<?xml version='1.0' encoding='utf-8' standalone='yes' ?>
<map>
    <boolean name="first_time_user" value="false" />
    <boolean name="has_training_exit" value="true" />
    <boolean name="has_onboarding_exit" value="true" />
    <boolean name="pref_update_welcome_16_2_shown_key" value="true" />
    <boolean name="pref_update_multi_finger_gestures_shown_key" value="true" />
</map>
EOF
adb push talkback-default-prefs.xml /data/local/tmp/talkback-default-prefs.xml >/dev/null
adb shell "run-as $TB_PKG mkdir -p /data/user_de/0/$TB_PKG/shared_prefs"
adb shell "run-as $TB_PKG cp /data/local/tmp/talkback-default-prefs.xml /data/user_de/0/$TB_PKG/shared_prefs/${TB_PKG}_preferences.xml"
talkback_prefs=$(adb shell "run-as $TB_PKG cat /data/user_de/0/$TB_PKG/shared_prefs/${TB_PKG}_preferences.xml") ||
    fail "Could not read TalkBack's seeded preferences"
for preference in \
    'name="first_time_user" value="false"' \
    'name="has_training_exit" value="true"' \
    'name="has_onboarding_exit" value="true"' \
    'name="pref_update_welcome_16_2_shown_key" value="true"' \
    'name="pref_update_multi_finger_gestures_shown_key" value="true"'; do
    echo "$talkback_prefs" | grep -F "$preference" >/dev/null ||
        fail "TalkBack preference was not seeded: $preference"
done
note "PASS skipped TalkBack first-run tutorial and welcome screens"
SERVICE="$TB_PKG/$TB_SERVICE"
# On a fresh API 37 boot, AccessibilityManager can still be initializing
# after package installation. Retry the *actual* enable operation, not the
# assertion; never report success without the service and process checks.
enabled=0
attempt=1
while [ "$attempt" -le 5 ]; do
    adb shell settings put secure enabled_accessibility_services "$SERVICE"
    adb shell settings put secure accessibility_enabled 1
    sleep 6
    configured=$(adb shell settings get secure enabled_accessibility_services | tr -d '\r')
    running=$(adb shell pidof "$TB_PKG" || true)
    note "INFO TalkBack activation attempt $attempt/5 configured=$configured pid=$running"
    if [ "$configured" = "$SERVICE" ] && [ -n "$running" ]; then
        enabled=1
        break
    fi
    attempt=$((attempt + 1))
done
[ "$enabled" -eq 1 ] ||
    fail "TalkBack did not remain enabled and running after 5 attempts"
adb shell dumpsys accessibility > accessibility_dumpsys.txt
grep -F "$TB_SERVICE" accessibility_dumpsys.txt >/dev/null ||
    fail "TalkBack service not registered in AccessibilityManager"
note "PASS real TalkBack package=$TB_PKG service=$SERVICE enabled and running"

# Pre-grant the runtime permissions needed by the simulated-device startup.
# Qt's Android Bluetooth adapter queries BluetoothAdapter.getName() while
# starting; API 37 aborts the process if BLUETOOTH_CONNECT is not granted.
for permission in android.permission.BLUETOOTH_CONNECT android.permission.POST_NOTIFICATIONS; do
    adb shell pm grant "$PKG" "$permission" ||
        fail "Could not grant $permission in emulator setup"
    adb shell dumpsys package "$PKG" | grep -F "$permission: granted=true" >/dev/null ||
        fail "$permission did not remain granted after setup"
    note "PASS emulator $permission granted before QZ launch"
done

# Use simulated bike; no physical exercise equipment is ever attached to this CI.
adb shell am force-stop "$PKG"
printf '[General]\napplewatch_fakedevice=true\nbluetooth_lastdevice_name=Fake\nskipLocationServicesDialog=true\n' > qz-talkback.conf
adb push qz-talkback.conf /data/local/tmp/qz-talkback.conf >/dev/null
adb shell "run-as $PKG mkdir -p 'files/.config/Roberto Viola'"
adb shell "run-as $PKG cp /data/local/tmp/qz-talkback.conf 'files/.config/Roberto Viola/qDomyos-Zwift.conf'"
adb shell monkey -p "$PKG" -c android.intent.category.LAUNCHER 1 >/dev/null
sleep 25
recover_startup_window
adb push a11y-test.jar /data/local/tmp/a11y-test.jar >/dev/null

# The reported issue includes unlabeled controls in QZ's top toolbar. Check
# the labels a blind user needs to identify the visible toolbar buttons before
# entering the drawer. The stateful buttons have two legitimate names.
require_named_node Main_menu
require_named_node Floating_window
require_named_node_either Lock_tiles Unlock_tiles
require_named_node_either Enable_auto_resistance Disable_auto_resistance

# ADB's shell swipe is not reliably recognized by TalkBack's gesture detector,
# but TalkBack remains active here. In one UiAutomation connection, set and
# verify the accessibility focus, then send ACTION_CLICK, the provider action
# TalkBack sends after a user's double-tap gesture.
adb logcat -c
focus_and_activate Main_menu
# Community points outside QZ. Focusing it must leave QZ in the foreground and
# must not activate the link or launch a browser during TalkBack exploration.
focus_without_activate Community
focus_and_activate Settings
result=$(peek --peek-node General_Options)
note "$result"
echo "$result" | grep -F TALKBACK_NODE >/dev/null ||
    fail "TalkBack activation did not open Settings / General Options absent"
result=$(peek --peek-node Experimental_Features)
note "$result"
echo "$result" | grep -F TALKBACK_NODE >/dev/null ||
    fail "Experimental Features not accessible on settings page"

focus_and_activate General_Options
result=$(peek --peek-node App_Language)
note "$result"
echo "$result" | grep -F TALKBACK_NODE >/dev/null || fail "General Options did not open"
focus_and_activate Experimental_Features
# Claudio needs the debug log for support. Confirm a positional tap on the
# label side of the TalkBack-focused row toggles the setting, then restore OFF.
focus_without_activate Debug_Log
result=$(peek --peek-node Debug_Log)
note "$result"
echo "$result" | grep -F 'checkable=true checked=false' >/dev/null ||
    fail "Debug Log was not OFF before the row tap"
result=$(peek --double-tap-node-left Debug_Log)
note "$result"
echo "$result" | grep -F 'checkable=true checked=true' >/dev/null ||
    fail "TalkBack double tap on the Debug Log label did not toggle the switch ON"
result=$(peek --double-tap-node-left Debug_Log)
note "$result"
echo "$result" | grep -F 'checkable=true checked=false' >/dev/null ||
    fail "TalkBack double tap on the Debug Log label did not toggle the switch OFF again"

# Keep the real TalkBack service running through the delay in which the
# unexpected Chrome launch was reported; any foreground change is a failure.
n=0
while [ "$n" -lt 12 ]; do
    sleep 6
    foreground
    n=$((n + 1))
done
note "PASS TalkBack remained inside QZ for 72+ seconds"
