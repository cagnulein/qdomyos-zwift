#!/bin/sh
# Experimental CI probe: real Google open-source TalkBack enabled in Android.
# Not equivalent to Claudio's Play Store TalkBack build or a human usability test.
# The existing UiAutomation regression test stays the main deterministic CI gate.
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
peek() {
    adb shell "CLASSPATH=/data/local/tmp/a11y-test.jar app_process / AccessibilityTest $*" 2>&1 | tr -d '\r'
}
focus_name() {
    # The helper is strictly read-only; the preceding swipe must move the focus.
    peek --peek-focused | grep -F "TALKBACK_FOCUS" || true
}
swipe_to() {
    name=$1; direction=$2
    n=0
    while [ "$n" -lt 35 ]; do
        current=$(focus_name)
        note "focus[$n]: $current"
        case "$current" in *"$name"*) note "PASS reached $name by TalkBack swipes"; return 0;; esac
        if [ "$direction" = backward ]; then
            adb shell input swipe 270 850 780 850 160
        else
            adb shell input swipe 780 850 270 850 160
        fi
        sleep 1
        foreground
        n=$((n + 1))
    done
    fail "TalkBack swipes did not reach $name in $n steps"
}
note "Starting experimental REAL TalkBack gesture check"

# This test MUST fail if the official open-source TalkBack APK did not install.
adb shell pm path "$TB_PKG" | grep -q 'package:' || fail "TalkBack is not installed"
SERVICE="$TB_PKG/$TB_SERVICE"
adb shell settings put secure enabled_accessibility_services "$SERVICE"
adb shell settings put secure accessibility_enabled 1
sleep 12
adb shell settings get secure enabled_accessibility_services | grep -F "$SERVICE" ||
    fail "TalkBack not in enabled accessibility services"
adb shell pidof "$TB_PKG" >/dev/null ||
    fail "TalkBack service process is not running"
adb shell dumpsys accessibility > accessibility_dumpsys.txt
grep -F "$TB_SERVICE" accessibility_dumpsys.txt >/dev/null ||
    fail "TalkBack service not registered in AccessibilityManager"
note "PASS real TalkBack package=$TB_PKG service=$SERVICE enabled and running"

# Use simulated bike; no physical exercise equipment is ever attached to this CI.
adb shell am force-stop "$PKG"
printf '[General]\napplewatch_fakedevice=true\nbluetooth_lastdevice_name=Fake\nskipLocationServicesDialog=true\n' > qz-talkback.conf
adb push qz-talkback.conf /data/local/tmp/qz-talkback.conf >/dev/null
adb shell "run-as $PKG mkdir -p 'files/.config/Roberto Viola'"
adb shell "run-as $PKG cp /data/local/tmp/qz-talkback.conf 'files/.config/Roberto Viola/qDomyos-Zwift.conf'"
adb shell monkey -p "$PKG" -c android.intent.category.LAUNCHER 1 >/dev/null
sleep 25
foreground
adb push a11y-test.jar /data/local/tmp/a11y-test.jar >/dev/null

# Prove TalkBack is changing accessibility focus when finger swipes are injected.
before=$(focus_name)
note "initial focus: $before"
adb shell input swipe 780 850 270 850 160
sleep 2
after=$(focus_name)
note "focus after swipe: $after"
[ -n "$after" ] || fail "No TalkBack accessibility focus after swipe"
[ "$before" != "$after" ] || fail "TalkBack swipe did not move accessibility focus"
foreground

# The deterministic harness opens the drawer without altering QZ source code;
# from here navigation to Settings is performed using REAL TalkBack swipes.
result=$(peek 'Main_menu=Settings')
echo "$result" >> "$REPORT"
echo "$result" | grep -F 'OK pressed' >/dev/null ||
    fail "Cannot open QZ drawer for TalkBack navigation"
foreground
swipe_to Settings backward

# Physical double-tap is interpreted by running TalkBack, unlike ACTION_CLICK.
adb shell 'input tap 530 1100; input tap 530 1100'
sleep 4
foreground
result=$(peek --peek-node General_Options)
note "$result"
echo "$result" | grep -F TALKBACK_NODE >/dev/null ||
    fail "TalkBack double-tap did not open Settings / General Options absent"
result=$(peek --peek-node Experimental_Features)
note "$result"
echo "$result" | grep -F TALKBACK_NODE >/dev/null ||
    fail "Experimental Features not accessible on settings page"

# Reproduce the user's unwanted Chrome launch after about 30 seconds.
n=0
while [ "$n" -lt 12 ]; do
    adb shell input swipe 780 850 270 850 160
    sleep 6
    foreground
    n=$((n + 1))
done
note "PASS TalkBack remained inside QZ for 72+ seconds while navigating"
