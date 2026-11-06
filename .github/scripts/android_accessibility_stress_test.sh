#!/bin/sh
# Accessibility stress test for the android-emulator-test job, run after the fake bike
# accessibility test, with a11y-stress.jar (built from AccessibilityStressTest.java) in the current
# folder. Opens and closes the tile settings page <cycles> times while another thread reads the
# whole accessibility tree without pause, then turns accessibility on and off <connects> times.
# Fails on a crash or an ANR of QZ, on a page that did not open or close, and when QZ did not
# move Qt's accessibility functions for Java to the Qt thread (main.cpp, androidAccessibility):
# Qt 5.15.0 runs them on the Android UI thread, and QZ crashed in this test (QTBUG-95764,
# QTBUG-144911). The output is in accessibility_stress_test.txt.
# Usage: android_accessibility_stress_test.sh <cycles> <connects>

PKG=org.cagnulen.qdomyoszwift
CYCLES=${1:-40}
CONNECTS=${2:-20}
OUT=accessibility_stress_test.txt

# On failure save the logcat first: the job stops here and does not reach its own logcat lines
fail() {
  echo "STRESS TEST FAILED: $1" | tee -a $OUT
  adb logcat -d > full_logcat.txt
  adb logcat -d | grep -i qdomyos > qdomyos_logcat.txt
  exit 1
}

# Crash lines of QZ: a Java crash ("Process: <package>"), a native one (">>> <package> <<<") and an
# ANR, from the crash and system buffers: the main one turns over quickly while the tree is read
qz_crashes() {
  timeout 60 adb logcat -d -b crash -b system | grep -c -F -e "Process: $PKG" -e ">>> $PKG <<<" -e "ANR in $PKG"
}

# QZ in front, started again if it crashed, so the next part still has something to test
ensure_qz() {
  if ! adb shell pidof $PKG > /dev/null 2>&1; then
    echo "QZ is not running, starting it again" | tee -a $OUT
    adb shell monkey -p $PKG -c android.intent.category.LAUNCHER 1 > /dev/null
    sleep 20
  fi
}

: > $OUT
[ -f a11y-stress.jar ] || fail "a11y-stress.jar was not built"
adb push a11y-stress.jar /data/local/tmp/a11y-stress.jar > /dev/null || fail "cannot push a11y-stress.jar"
adb logcat -G 16M || true

# QZ again with the fake bike and its debug log on: only with it does QZ's own log reach the
# logcat, and with it the line of main.cpp that tells the functions moved to the Qt thread
adb shell am force-stop $PKG
printf '[General]\napplewatch_fakedevice=true\nbluetooth_lastdevice_name=Fake\nskipLocationServicesDialog=true\nlog_debug=true\n' > qz-stress.conf
adb push qz-stress.conf /data/local/tmp/qz.conf > /dev/null || fail "cannot push qz-stress.conf"
adb shell "run-as $PKG mkdir -p 'files/.config/Roberto Viola'"
adb shell "run-as $PKG cp /data/local/tmp/qz.conf 'files/.config/Roberto Viola/qDomyos-Zwift.conf'" \
  || fail "cannot write the settings of QZ"
adb shell monkey -p $PKG -c android.intent.category.LAUNCHER 1 > /dev/null
# The line comes when main() loads the QML. On API 35+ QZ opens the Health Connect permissions
# at its start, and main() waits until that screen is closed (79 s on API 35 until the pages part
# pressed back): close a screen of another app with back presses, as the accessibility test does
moved=0
i=0
while [ $i -lt 60 ]; do
  i=$((i + 1))
  sleep 1
  pid=$(adb shell pidof $PKG | tr -d '\r')
  [ -n "$pid" ] || continue
  moved=$(timeout 30 adb logcat -d --pid="$pid" | grep -c -F "Qt's accessibility functions run on the Qt thread")
  [ "$moved" -gt 0 ] && break
  if [ $((i % 5)) -eq 0 ]; then
    FOCUS=$(adb shell dumpsys window | grep -m 1 mCurrentFocus | tr -d '\r')
    case "$FOCUS" in
      *$PKG*|*"=null"*) ;;
      *) echo "Not QZ in front ($FOCUS), pressing back" | tee -a $OUT; adb shell input keyevent KEYCODE_BACK ;;
    esac
  fi
done
echo "functions on the Qt thread: $moved (after $i s)" | tee -a $OUT
# the fake bike connects and the tiles show some seconds after the start
sleep 15

before=$(qz_crashes)
echo "== tiles page $CYCLES times" | tee -a $OUT
# Each part has a time limit, so that a hang of one of them still ends in fail() with the logcat within
# the 25 min of the emulator step (the whole test usually takes about 3-6 min)
timeout 600 adb shell "CLASSPATH=/data/local/tmp/a11y-stress.jar app_process / AccessibilityStressTest pages $CYCLES" \
  | tr -d '\r' | tee -a $OUT
after_pages=$(qz_crashes)

echo "== accessibility on and off $CONNECTS times" | tee -a $OUT
ensure_qz
i=0
while [ $i -lt "$CONNECTS" ]; do
  i=$((i + 1))
  timeout 15 adb shell "CLASSPATH=/data/local/tmp/a11y-stress.jar app_process / AccessibilityStressTest connect" \
    | tr -d '\r' >> $OUT
  sleep 1
done
sleep 3
after=$(qz_crashes)

# a read that gave up: the Qt thread was busy for 2 s (loading QML), allowed but counted
waits=$(timeout 60 adb logcat -d | grep -c -F "the Qt thread did not answer in 2 s")
echo "STRESS crashes or ANRs of QZ: $((after_pages - before)) while the pages changed, $((after - after_pages)) while accessibility was turned on and off; reads given up after 2 s: $waits" \
  | tee -a $OUT

if [ "$after" -gt "$before" ]; then
  timeout 60 adb logcat -d -b crash | tail -n 40
  fail "QZ crashed or did not answer during the accessibility stress test"
fi
FIRST_FAIL=$(while read -r word rest; do
  if [ "$word" = "FAIL" ]; then echo "$rest"; break; fi
done < $OUT)
[ -z "$FIRST_FAIL" ] || fail "$FIRST_FAIL"
FINISHED=$(while read -r word rest; do
  if [ "$word" = "OK" ]; then echo "$rest"; break; fi
done < $OUT)
[ -n "$FINISHED" ] || fail "the pages part did not finish"
# each connection prints "INFO <n> nodes after <t> ms": one that saw no node of QZ did not turn
# accessibility on in it, and the on/off part proved nothing
CONNECTED=$(n=0; while read -r word count kind rest; do
  if [ "$word" = "INFO" ] && [ "$kind" = "nodes" ] && [ "$count" != "0" ]; then n=$((n + 1)); fi
done < $OUT; echo $n)
[ "$CONNECTED" -eq "$CONNECTS" ] || fail "only $CONNECTED of $CONNECTS connections saw the nodes of QZ"
[ "$moved" -gt 0 ] || fail "QZ did not log that its accessibility functions run on the Qt thread"
echo "Accessibility stress test passed" | tee -a $OUT
