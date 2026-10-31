#!/usr/bin/env bash
# CI-only Android memory test for PR #5172 (regular localipaddress::getIP/JNI).
set -euo pipefail
pkg=org.cagnulen.qdomyoszwift
activity="$pkg/$pkg.CustomQtActivity"
fail() {
  echo "::error::Android IP memory stress: $1"
  adb logcat -d > ip_stress_logcat.txt || true
  tail -n 100 ip_stress_logcat.txt || true
  exit 1
}
adb shell am force-stop "$pkg"
adb logcat -c
adb shell am start -W -n "$activity" --ez qz_ci_ip_stress true
pid=""
for i in $(seq 1 60); do
  pid="$(adb shell pidof "$pkg" 2>/dev/null | tr -d '\r')"
  [ -n "$pid" ] && break
  sleep 2
done
[ -n "$pid" ] || fail "QZ did not start"
for count in 5000 35000 65000; do
  seen=""
  for i in $(seq 1 150); do
    current="$(adb shell pidof "$pkg" 2>/dev/null | tr -d '\r')"
    [ "$current" = "$pid" ] || fail "QZ crashed/restarted during stress (pid $pid -> $current)"
    phases="$(adb logcat -d -s QZIpStress:I '*:S')"
    if [[ "$phases" == *"QZ_IP_STRESS $count"* ]]; then
      seen=yes
      break
    fi
    sleep 2
  done
  [ -n "$seen" ] || fail "checkpoint $count not reached: JNI path not proven"
  adb shell dumpsys meminfo "$pkg" > "ip_mem_$count.txt"
  echo "Checkpoint $count: $(grep -F 'TOTAL PSS:' "ip_mem_$count.txt" | head -n 1 || true)"
done
adb logcat -d > ip_stress_logcat.txt
python3 - <<'PY'
import sys

def pss(count):
    with open(f"ip_mem_{count}.txt", encoding="utf-8") as fh:
        lines = fh.read().splitlines()
    for line in lines:
        if "TOTAL PSS:" in line:
            return int(line.split("TOTAL PSS:", 1)[1].split()[0])
    for line in lines:
        cells = line.split()
        if len(cells) > 1 and cells[0] == "TOTAL" and cells[1].isdigit():
            return int(cells[1])
    raise RuntimeError(f"cannot parse process PSS at {count}")

try:
    warm, mid, end = (pss(n) for n in (5000, 35000, 65000))
except Exception as exc:
    sys.exit(f"Invalid meminfo: {exc}")
print(f"QZ IP JNI stress (KiB PSS): warmup={warm}, mid={mid}, end={end}")
print(f"PSS deltas (KiB): first 30000={mid-warm}, final 30000={end-mid}")
# This is a coarse OOM regression check, not proof of zero leaks on real devices.
if end - mid > 32 * 1024 and end - warm > 32 * 1024:
    sys.exit("FAIL: PSS grew over 32 MiB in final 30000 JNI lookups")
print("PASS: no excessive process PSS growth in emulator test")
PY
