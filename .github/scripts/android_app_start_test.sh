#!/bin/sh
# App start test for the android-emulator-test job.
# Starts QZ and waits until its process is up, instead of one check after a fixed sleep:
# a start sent while the emulator is still busy after boot is sometimes lost, so it is retried once.

PKG=org.cagnulen.qdomyoszwift
ACTIVITY=$PKG/$PKG.CustomQtActivity
WAIT=90   # seconds to wait for the process after each start

# On failure save the diagnostics first: the job stops here and does not reach its own logcat lines
fail() {
  echo "TEST FAILED: $1"
  echo "=== Activity manager state ==="
  adb shell dumpsys activity activities | grep -E "mResumedActivity|topResumedActivity|$PKG" | head -n 20
  echo "=== Logcat of the app start (ActivityManager, AndroidRuntime, app) ==="
  adb logcat -d | grep -E "ActivityManager|ActivityTaskManager|AndroidRuntime|libc|DEBUG|$PKG|qdomyos" | tail -n 150
  adb shell "ps -A 2>/dev/null || ps" > process_list.txt
  adb shell screencap -p /sdcard/screenshot.png && adb pull /sdcard/screenshot.png
  adb logcat -d > full_logcat.txt
  adb logcat -d | grep -i qdomyos > qdomyos_logcat.txt
  exit 1
}

app_pid() {
  adb shell pidof "$PKG" 2>/dev/null | tr -d '\r'
}

wait_for_app() {
  i=0
  while [ "$i" -lt "$WAIT" ]; do
    PID=$(app_pid)
    if [ -n "$PID" ]; then
      echo "App process is up (pid $PID) after ${i}s"
      return 0
    fi
    sleep 5
    i=$((i + 5))
  done
  return 1
}

# The runner reports the emulator as ready before Android finishes booting
echo "Waiting for the boot to complete..."
i=0
until [ "$(adb shell getprop sys.boot_completed | tr -d '\r')" = "1" ]; do
  i=$((i + 2))
  [ "$i" -le 300 ] || fail "the emulator did not finish booting in 300 s"
  sleep 2
done
adb shell pm path "$PKG" || fail "the app is not installed"

for attempt in 1 2; do
  echo "Starting the app (attempt $attempt)"
  adb shell am start -W -n "$ACTIVITY"
  if wait_for_app; then
    # Still alive a bit later: a start that crashes right away must not pass.
    # On google_apis images Google Play services may restart soon after boot, and Android kills
    # the clients of their providers too ("depends on provider ... in dying proc"): start again then.
    # A real crash happens on the second start as well and fails the test.
    sleep 30
    if [ -n "$(app_pid)" ]; then
      adb shell "ps -A 2>/dev/null || ps" > process_list.txt
      echo "App is running successfully"
      exit 0
    fi
    echo "App process $PID died within 30 s after the start:"
    adb logcat -d | grep -E "Killing $PID:|Process $PKG \(pid $PID\)|FATAL EXCEPTION|Fatal signal" | tail -n 20
  else
    echo "App process not found after ${WAIT}s"
  fi
done
fail "App process not running"
