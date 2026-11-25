#!/bin/sh
# Back button test for the android-emulator-test job (issue #1386).
# Expects the app to be started already. Single back presses must navigate
# inside the app, only a double back press on the home page may exit.

# On failure save the logcat first: the job stops here and does not reach its own logcat lines
fail() {
  echo "TEST FAILED: $1"
  adb logcat -d > full_logcat.txt
  adb logcat -d | grep -i qdomyos > qdomyos_logcat.txt
  exit 1
}

API=$(adb shell getprop ro.build.version.sdk | tr -d '\r')
[ -n "$API" ] || fail "cannot read the API level from the emulator"
echo "Back button test on API $API"

# Crash lines of QZ: a Java crash ("Process: <package>, PID"), a native one (">>> <package> <<<") and an
# ANR ("ANR in <package>": with hide_error_dialogs below Android kills it without a dialog).
# Counted before the test, so only a crash during the test counts: the exit must be clean too
qz_crashes() {
  adb logcat -d | grep -F -e "Process: org.cagnulen.qdomyoszwift" -e ">>> org.cagnulen.qdomyoszwift <<<" \
    -e "ANR in org.cagnulen.qdomyoszwift" | grep -c -F -e "qdomyoszwift"
}
crashes_before=$(qz_crashes)

qz_in_front() {
  adb shell dumpsys activity activities | grep -E "mResumedActivity|topResumedActivity" | grep -q CustomQtActivity
}

focused_window() {
  adb shell dumpsys window | grep -m 1 mCurrentFocus | tr -d '\r'
}

# An "app isn't responding" dialog of another app (e.g. Pixel Launcher on the google_apis
# images) can cover QZ while its activity stays resumed: back presses then go to the dialog.
# Close it: first as a system dialog, then by tapping its "Wait" button. One of QZ is a failure.
close_anr_dialog() {
  case "$(focused_window)" in
    *"Not Responding"*org.cagnulen.qdomyoszwift*) fail "QZ is not responding: $(focused_window)" ;;
    *"Not Responding"*) ;;
    *) return 0 ;;
  esac
  echo "Closing an \"isn't responding\" dialog: $(focused_window)"
  adb shell am broadcast -a android.intent.action.CLOSE_SYSTEM_DIALOGS > /dev/null
  sleep 2
  case "$(focused_window)" in
    *"Not Responding"*) ;;
    *) return 0 ;;
  esac
  adb shell uiautomator dump /sdcard/ui.xml > /dev/null
  # bounds="[x1,y1][x2,y2]" of the "Wait" button -> its center
  XY=$(adb shell cat /sdcard/ui.xml | tr '>' '\n' | grep 'text="Wait"' | sed -n 's/.*bounds="\[\([0-9]*\),\([0-9]*\)\]\[\([0-9]*\),\([0-9]*\)\]".*/\1 \2 \3 \4/p' | head -n 1)
  if [ -n "$XY" ]; then
    set -- $XY
    adb shell input tap $((($1 + $3) / 2)) $((($2 + $4) / 2))
    sleep 2
  fi
}

# Wait until QZ has the input focus, closing "isn't responding" dialogs on top of it.
# Its own loop variable: sh has no local ones, and "i" would change the count of the caller's loop
qz_focused() {
  for try in 1 2 3; do
    close_anr_dialog
    case "$(focused_window)" in
      *CustomQtActivity*) return 0 ;;
    esac
    echo "QZ has no input focus ($(focused_window)), waiting ($try)"
    sleep 3
  done
  return 1
}

# No "app isn't responding" dialogs from other apps: applied on the next configuration change (the rotation below)
adb shell settings put global hide_error_dialogs 1

shot() {
  adb shell screencap -p "/sdcard/screenshot_back_$1.png"
  adb pull "/sdcard/screenshot_back_$1.png"
}

adb shell settings put system user_rotation 0
sleep 5

# Close system screens opened at startup (e.g. Health Connect permissions) until QZ is in front
for i in 1 2 3 4 5; do
  close_anr_dialog
  qz_in_front && break
  echo "QZ is not in front, pressing back ($i)"
  adb shell input keyevent KEYCODE_BACK
  sleep 5
done
shot 0
qz_in_front || fail "QZ is not in front before the back button test"

# Single presses, more than 2 s apart: the first leaves the wizard,
# the next ones show "Press back again to exit"
for i in 1 2 3; do
  qz_focused || fail "a system window keeps the input focus before back press $i"
  echo "Single back press $i"
  adb shell input keyevent KEYCODE_BACK
  sleep 1
  shot "$i"
  sleep 4
  qz_in_front || fail "single back press $i closed the app"
done
echo "Single back presses kept the app open"

# Two presses in one command, well within 2 s: the app must exit
qz_focused || fail "a system window keeps the input focus before the double back press"
PID=$(adb shell pidof org.cagnulen.qdomyoszwift | tr -d '\r')
[ -n "$PID" ] || fail "no QZ process before the double back press"
adb shell input keyevent KEYCODE_BACK KEYCODE_BACK
sleep 10
shot exit
qz_in_front && fail "double back press did not close the app"

# Off the screen is not enough: the process must end too, and on its own. That process, by its PID:
# Android may start QZ again in the background for a service of a library (seen on API 35 10 s
# after the double back press, for com.google.android.datatransport), and that new process is not a
# failed exit. Only an answer of the emulator counts: no answer (adb lost it) is not "ended"
for i in 1 2 3 4; do
  STATE=$(adb shell "[ -d /proc/$PID ] && echo running || echo gone" | tr -d '\r')
  [ "$STATE" = gone ] && break
  if [ "$i" = 4 ]; then
    [ "$STATE" = running ] && fail "QZ left the screen, but its process $PID still runs 25 s after the double back press"
    fail "cannot tell whether the QZ process $PID ended: the emulator answered '$STATE'"
  fi
  sleep 5
done

# Counted once the process is gone, so a crash late in the exit counts too
if [ "$(qz_crashes)" -gt "$crashes_before" ]; then
  echo "QZ crashed during the back button test (on exit or before), the stack:"
  # the last native crash of QZ: its lines of the DEBUG tag only (other tags are mixed in between)
  adb logcat -d | awk -v p=">>> org.cagnulen.qdomyoszwift <<<" 'index($0, p) { out = ""; on = 1 } on && index($0, " DEBUG ") { out = out $0 "\n" } END { printf "%s", out }' \
    | grep -F -e ">>>" -e "signal " -e "Abort message" -e "Cause:" -e " #" | head -n 30
  # or a Java crash
  adb logcat -d | grep -F -A 10 "Process: org.cagnulen.qdomyoszwift" | tail -n 11
  fail "QZ crashed during the back button test, the exit was not clean"
fi

# Zygote logs how the app process ended: QZ must have called exit(0), not have been ended by a
# signal ("exited due to signal 6 (Aborted)", up to API 28 "signal (6)"). Not the reason of
# "dumpsys activity exit-info": the API 31 emulator had none for QZ, though Zygote logged a clean exit.
# API 24 and 26 log no clean exit with status 0, only a signal or another status: any line fails there.
# Nor do they log SIGKILL, so a kill by Android is found by its ActivityManager line "Killing <pid>:"
if [ "$API" -lt 28 ]; then
  EXIT=$(adb logcat -d -s Zygote | tr -d '\r' | grep -F "Process $PID exited" | tail -n 1)
  [ -z "$EXIT" ] || fail "QZ did not exit on its own with status 0: ${EXIT#*: }"
  KILL=$(adb logcat -d -s ActivityManager | tr -d '\r' | grep -F "Killing $PID:" | tail -n 1)
  [ -z "$KILL" ] || fail "Android killed QZ, it did not exit on its own: ${KILL#*: }"
else
  for i in 1 2 3 4 5; do
    EXIT=$(adb logcat -d -s Zygote | tr -d '\r' | grep -F "Process $PID exited" | tail -n 1)
    [ -n "$EXIT" ] && break
    sleep 2
  done
  echo "Exit of QZ: ${EXIT#*: }"
  case "$EXIT" in
    *"Process $PID exited cleanly (0)") ;;
    "") fail "Zygote logged no exit of the QZ process $PID" ;;
    *) fail "QZ did not exit on its own with status 0" ;;
  esac
fi
echo "Back button test passed"
