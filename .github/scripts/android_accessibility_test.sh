#!/bin/sh
# Accessibility test for the android-emulator-test job (PR #5377).
# Expects the app to be started already and a11y-test.jar (built from AccessibilityTest.java) in
# the current folder. Sends QZ the actions of a screen reader through UiAutomation, so it runs
# without TalkBack on every image, and fails on a crash of QZ, a native accessibility function
# that is not found or a press that QZ did not handle.
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

# Crash lines of QZ: a Java crash ("Process: <package>, PID"), a native one (">>> <package> <<<"),
# and ART's "No implementation found" for a native function CustomQtActivity calls (it catches the
# UnsatisfiedLinkError itself). Counted before the test, so an earlier one does not count.
qz_crashes() {
  adb logcat -d | grep -F -e "Process: org.cagnulen.qdomyoszwift" -e ">>> org.cagnulen.qdomyoszwift <<<" \
    -e "No implementation found for" | grep -c -F -e "qdomyoszwift"
}
crashes_before=$(qz_crashes)

timeout 180 adb shell "CLASSPATH=/data/local/tmp/a11y-test.jar app_process / AccessibilityTest '$BUTTON'" \
  | tr -d '\r' > accessibility_test.txt
cat accessibility_test.txt
sleep 2

grep -q -F "DONE" accessibility_test.txt || fail "the accessibility test did not finish"
FIRST_FAIL=$(while read -r word rest; do
  if [ "$word" = "FAIL" ]; then echo "$rest"; break; fi
done < accessibility_test.txt)
[ -z "$FIRST_FAIL" ] || fail "$FIRST_FAIL"

crashes_after=$(qz_crashes)
if [ "$crashes_after" -gt "$crashes_before" ]; then
  adb logcat -d | grep -F -B 2 -A 20 -e "Process: org.cagnulen.qdomyoszwift" -e ">>> org.cagnulen.qdomyoszwift <<<" \
    -e "No implementation found for" | tail -n 60
  fail "QZ crashed or missed a native function during the accessibility test"
fi

adb shell "ps -A 2>/dev/null || ps" | grep -q -F org.cagnulen.qdomyoszwift \
  || fail "QZ is no longer running after the accessibility test"

echo "Accessibility test passed"
