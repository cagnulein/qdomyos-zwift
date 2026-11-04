#!/bin/sh
# Accessibility test for the android-emulator-test job (PR #5377).
# Expects the app to be started already and a11y-test.jar (built from AccessibilityTest.java) in
# the current folder. Sends QZ the actions of a screen reader through UiAutomation, so it runs
# without TalkBack on every image, and fails on a crash of QZ, a native accessibility function
# that is not found, a press that QZ did not handle or that did not show what it should.
# Usage: android_accessibility_test.sh [--fake-bike] <press> [<press> ...]
# A press is described in AccessibilityTest.java; it goes through the device's shell unquoted, so
# letters, digits and "_ = : - ." only. With --fake-bike QZ is restarted first with only
# the fake bike of its settings turned on, so the home page has tiles with - and + (the settings
# the tests before made are dropped); it comes up without a Bluetooth adapter too, so the run is
# never skipped, and it must check the scroll to the focus. The output of every run is added to
# accessibility_test.txt.

PKG=org.cagnulen.qdomyoszwift
FAKE_BIKE=0
if [ "$1" = "--fake-bike" ]; then
  FAKE_BIKE=1
  shift
fi
PRESSES="$*"
# the tiles of the fake bike reach below the screen: its run must check the scroll to the focus
SCROLL=""
[ "$FAKE_BIKE" = 1 ] && SCROLL="--scroll"

# On failure save the logcat first: the job stops here and does not reach its own logcat lines
fail() {
  echo "TEST FAILED: $1"
  adb logcat -d > full_logcat.txt
  adb logcat -d | grep -i qdomyos > qdomyos_logcat.txt
  exit 1
}

API=$(adb shell getprop ro.build.version.sdk | tr -d '\r')
[ -n "$API" ] || fail "cannot read the API level from the emulator"
echo "Accessibility test on API $API: $PRESSES"

[ -f a11y-test.jar ] || fail "a11y-test.jar was not built"
adb push a11y-test.jar /data/local/tmp/a11y-test.jar > /dev/null || fail "cannot push a11y-test.jar"

# Crash lines of QZ: a Java crash ("Process: <package>, PID"), a native one (">>> <package> <<<"), an
# ANR ("ANR in <package>") and ART's "No implementation found" for a native function CustomQtActivity
# calls (it catches the UnsatisfiedLinkError itself). Counted before the run, the new start of the fake
# bike run included, so an earlier one does not count and one in the first seconds after the start does.
qz_crashes() {
  adb logcat -d | grep -F -e "Process: $PKG" -e ">>> $PKG <<<" -e "ANR in $PKG" \
    -e "No implementation found for" | grep -c -F -e "qdomyoszwift"
}
crashes_before=$(qz_crashes)

if [ "$FAKE_BIKE" = 1 ]; then
  adb shell am force-stop $PKG
  # skipLocationServicesDialog: as the job's own settings, no "Permissions Required" dialog on the images without Bluetooth
  printf '[General]\napplewatch_fakedevice=true\nbluetooth_lastdevice_name=Fake\nskipLocationServicesDialog=true\n' > qz.conf
  adb push qz.conf /data/local/tmp/qz.conf > /dev/null || fail "cannot push qz.conf"
  adb shell "run-as $PKG mkdir -p 'files/.config/Roberto Viola'"
  adb shell "run-as $PKG cp /data/local/tmp/qz.conf 'files/.config/Roberto Viola/qDomyos-Zwift.conf'" \
    || fail "cannot write the settings of QZ"
  adb shell monkey -p $PKG -c android.intent.category.LAUNCHER 1 > /dev/null
  # the fake bike connects and the tiles show some seconds after the start
  sleep 20
fi

# A new start can open a screen of another app over QZ (e.g. Health Connect permissions on
# API 35+) or a system dialog: close it with back presses until a window of QZ has the focus
for i in 1 2 3 4 5; do
  FOCUS=$(adb shell dumpsys window | grep -m 1 mCurrentFocus | tr -d '\r')
  case "$FOCUS" in
    # "qdomyos-zwift has stopped": QZ crashed before this run (a crash dialog of another app is closed below)
    *"Application Error: $PKG"*) fail "a crash dialog is in front, QZ crashed before this run ($FOCUS)" ;;
    *$PKG*) break ;;
    *"=null"*) echo "No window has the focus, waiting ($i)" ;;
    *) echo "Not QZ in front ($FOCUS), pressing back ($i)"; adb shell input keyevent KEYCODE_BACK ;;
  esac
  sleep 5
done

timeout 240 adb shell "CLASSPATH=/data/local/tmp/a11y-test.jar app_process / AccessibilityTest $SCROLL $PRESSES" \
  | tr -d '\r' > accessibility_test_run.txt
cat accessibility_test_run.txt
cat accessibility_test_run.txt >> accessibility_test.txt
sleep 2

grep -q -F "DONE" accessibility_test_run.txt || fail "the accessibility test did not finish"
FIRST_FAIL=$(while read -r word rest; do
  if [ "$word" = "FAIL" ]; then echo "$rest"; break; fi
done < accessibility_test_run.txt)
[ -z "$FIRST_FAIL" ] || fail "$FIRST_FAIL"

crashes_after=$(qz_crashes)
if [ "$crashes_after" -gt "$crashes_before" ]; then
  adb logcat -d | grep -F -B 2 -A 20 -e "Process: $PKG" -e ">>> $PKG <<<" \
    -e "No implementation found for" | tail -n 60
  fail "QZ crashed or missed a native function during the accessibility test"
fi

adb shell "ps -A 2>/dev/null || ps" | grep -q -F $PKG \
  || fail "QZ is no longer running after the accessibility test"

echo "Accessibility test passed"
