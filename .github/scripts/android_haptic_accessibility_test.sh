#!/bin/sh
# UIAutomation accessibility regression for optional workout-control haptics.
# Uses simulated equipment only, and does not install real TalkBack or Voice Access.
# QZ_HAPTIC_EVENT indicates a native Android UI-thread request, NOT motor vibration.
set -eu

PKG=org.cagnulen.qdomyoszwift
OUT=haptic_accessibility_test.txt
MARKER=QZ_HAPTIC_EVENT
: > "$OUT"

fail() {
    echo "FAIL $*" | tee -a "$OUT"
    adb shell dumpsys window | grep -m 1 mCurrentFocus || true
    adb logcat -d > full_logcat.txt || true
    exit 1
}
say() { echo "HAPTIC TEST: $*" | tee -a "$OUT"; }
count_haptics() { adb logcat -d -s CustomQtActivity:D | grep -c -F "$MARKER" || true; }
assert_haptics() {
    seen=$(count_haptics)
    [ "$seen" -eq "$1" ] || fail "expected $1 haptic requests, got $seen"
    say "OK $1 native haptic request(s)"
}
assert_qz_foreground() {
    focus=$(adb shell dumpsys window | grep -m 1 mCurrentFocus || true)
    case "$focus" in
        *"$PKG"*) say "OK QZ in foreground" ;;
        *) fail "another app in foreground (unexpected link/dialog?): $focus" ;;
    esac
}
run_a11y() {
    sh .github/scripts/android_accessibility_test.sh "$@" >> "$OUT" ||
        fail "accessibility action failed: $*"
    assert_qz_foreground
}
configure() {
    equipment=$1
    haptic=$2
    look=$3
    say "setup fake=$equipment feedback=$haptic UI=$look"
    adb shell am force-stop "$PKG"
    fake=applewatch_fakedevice
    [ "$equipment" = treadmill ] && fake=fakedevice_treadmill
    modern=false
    [ "$look" = modern ] && modern=true
    printf '[General]\n%s=true\nbluetooth_lastdevice_name=Fake\nskipLocationServicesDialog=true\nhaptic_feedback_enabled=%s\nui_modern=%s\n' \
        "$fake" "$haptic" "$modern" > qz-haptic.conf
    adb push qz-haptic.conf /data/local/tmp/qz-haptic.conf > /dev/null
    adb shell "run-as $PKG mkdir -p 'files/.config/Roberto Viola'"
    adb shell "run-as $PKG cp /data/local/tmp/qz-haptic.conf 'files/.config/Roberto Viola/qDomyos-Zwift.conf'"
    adb shell monkey -p "$PKG" -c android.intent.category.LAUNCHER 1 > /dev/null
    sleep 24
    # Health Connect sometimes takes focus on Android 15+.
    i=0
    while [ "$i" -lt 6 ]; do
        focus=$(adb shell dumpsys window | grep -m 1 mCurrentFocus || true)
        case "$focus" in *"$PKG"*) break ;; *) adb shell input keyevent KEYCODE_BACK; sleep 3 ;; esac
        i=$((i+1))
    done
    assert_qz_foreground
    adb logcat -c
}

# Default-off means focusing and pressing workout controls cannot vibrate.
configure bike false classic
run_a11y see:Increase_Resistance focus:Increase_Resistance
assert_haptics 0
run_a11y Increase_Resistance=Resistance:_1
assert_haptics 0

# A TalkBack-style accessibility activation of the actual option must enable
# feedback immediately, without restarting QZ.
run_a11y '◄=Settings' 'tap:Settings=General_Options' 'General_Options=Vibration_feedback_on_controls'
assert_haptics 0
run_a11y Vibration_feedback_on_controls
assert_haptics 0
run_a11y '◄=Start'
run_a11y Increase_Resistance=Resistance:_2
assert_haptics 1
run_a11y Increase_Resistance=Resistance:_3
assert_haptics 2

# Disable again through the settings page: no stale value in the Home Settings copy.
run_a11y '◄=Settings' 'tap:Settings=General_Options' 'General_Options=Vibration_feedback_on_controls'
run_a11y Vibration_feedback_on_controls
run_a11y '◄=Start'
run_a11y Increase_Resistance=Resistance:_4
assert_haptics 2

# Both tile layouts must work with accessibility ACTION_CLICK.
configure bike true modern
run_a11y see:Increase_Resistance focus:Increase_Resistance
assert_haptics 0
run_a11y Increase_Resistance=Resistance:_1
assert_haptics 1
run_a11y Increase_Resistance=Resistance:_2
assert_haptics 2

# Treadmill actions requested by the user, with NO real treadmill connected.
configure treadmill true classic
run_a11y see:Increase_Speed see:Decrease_Speed see:Increase_Inclination see:Decrease_Inclination
run_a11y focus:Increase_Speed focus:Increase_Inclination
assert_haptics 0
run_a11y Increase_Speed
assert_haptics 1
run_a11y Decrease_Speed
assert_haptics 2
run_a11y Increase_Inclination
assert_haptics 3
run_a11y Decrease_Inclination
assert_haptics 4
# Start must become Pause, not accidentally Start again (double activation).
run_a11y Start=Pause
assert_haptics 5
run_a11y Lap
assert_haptics 6
run_a11y Stop
assert_haptics 7

# Also exercise actual treadmill controls and workout buttons in the modern look.
configure treadmill true modern
run_a11y see:Increase_Speed see:Increase_Inclination
run_a11y focus:Increase_Speed
assert_haptics 0
run_a11y Increase_Speed
assert_haptics 1
run_a11y Increase_Inclination
assert_haptics 2
run_a11y Start=Pause
assert_haptics 3
run_a11y Lap
assert_haptics 4
run_a11y Stop
assert_haptics 5

say "PASS mobile haptic accessibility (Android API $(adb shell getprop ro.build.version.sdk | tr -d '\r'))"
