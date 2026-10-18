#!/bin/sh
# Accessibility test for the android-emulator-test job (PR #5377).
# Expects the app to be started already and a11y-test.jar (built from AccessibilityTest.java) in
# the current folder. Sends QZ the actions of a screen reader through UiAutomation, so it runs
# without TalkBack on every image, and fails on a crash, an UnsatisfiedLinkError of the native
# accessibility functions or a press that QZ did not handle.
# Usage: android_accessibility_test.sh [description of a button to press]

BUTTON="$1"

# On failure save the logcat first: the job stops here and does not reach its own logcat lines
fail() {
  echo "TEST FAILED: $1"
  adb logcat -d > full_logcat.txt
  adb logcat -d | grep -i qdomyos > qdomyos_logcat.txt
  exit 1
}

API=$(adb shell getprop ro.build.version.sdk | tr -d '\r')
[ -n "$API" ] || fail "cannot read the API level from the emulator"
echo "Accessibility test on API $API"

[ -f a11y-test.jar ] || fail "a11y-test.jar was not built"
adb push a11y-test.jar /data/local/tmp/a11y-test.jar > /dev/null || fail "cannot push a11y-test.jar"

# Count the crash lines before the test, so an earlier one does not count
crashes_before=$(adb logcat -d | grep -c -F -e "FATAL EXCEPTION" -e "UnsatisfiedLinkError")

timeout 180 adb shell "CLASSPATH=/data/local/tmp/a11y-test.jar app_process / AccessibilityTest '$BUTTON'" \
  | tr -d '\r' > accessibility_test.txt
cat accessibility_test.txt
sleep 2

grep -q -F "DONE" accessibility_test.txt || fail "the accessibility test did not finish"
FIRST_FAIL=$(while read -r word rest; do
  if [ "$word" = "FAIL" ]; then echo "$rest"; break; fi
done < accessibility_test.txt)
[ -z "$FIRST_FAIL" ] || fail "$FIRST_FAIL"

crashes_after=$(adb logcat -d | grep -c -F -e "FATAL EXCEPTION" -e "UnsatisfiedLinkError")
if [ "$crashes_after" -gt "$crashes_before" ]; then
  adb logcat -d | grep -F -A 20 -e "FATAL EXCEPTION" -e "UnsatisfiedLinkError" | tail -n 40
  fail "crash or UnsatisfiedLinkError in the logcat during the accessibility test"
fi

adb shell "ps -A 2>/dev/null || ps" | grep -q -F org.cagnulen.qdomyoszwift \
  || fail "QZ is no longer running after the accessibility test"

echo "Accessibility test passed"
