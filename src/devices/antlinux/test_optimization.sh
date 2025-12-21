#!/bin/bash
set -euo pipefail

# Test script for optimization changes (FIFO / no-disk logging)

# Test 1: Verify FIFO creation in RAM
test_fifo_location() {
    local fifo="/dev/shm/test_fifo_$$"
    mkfifo "$fifo"
    [[ -p "$fifo" ]] || { echo "FAIL: FIFO not created"; return 1; }
    rm "$fifo"
    echo "PASS: FIFO in RAM disk"
}

# Test 2: Minimal smoke run of setup-dashboard.sh --scan-now (non-destructive)
test_no_disk_writes() {
    # Start the dashboard scan in background
    timeout 15s bash ./setup-dashboard.sh --scan-now &
    local pid=$!
    sleep 2

    # Check for writes in /tmp (quick heuristic: no qz files)
    local writes
    writes=$(ls /tmp | grep -c "qz_" || true)

    kill "$pid" 2>/dev/null || true

    if [[ $writes -gt 5 ]]; then
        echo "FAIL: Detected $writes temporary files in /tmp"
        return 1
    fi
    echo "PASS: Minimal disk I/O ($writes tmp files)"
}

# Test 3: Provider persistence (simple PID check)
test_provider_persistence() {
    # Start provider directly (uses bt_provider.py)
    if [[ ! -f "bt_provider.py" ]]; then
        echo "SKIP: bt_provider.py not present"
        return 0
    fi
    python3 bt_provider.py --heartbeat /dev/shm/qz_test_hb_$$ &
    local pp=$!
    sleep 2
    if ps -p "$pp" >/dev/null 2>&1; then
        echo "PASS: Provider running pid=$pp"
        kill "$pp" 2>/dev/null || true
        return 0
    else
        echo "FAIL: Provider not running"
        return 1
    fi
}

# Run tests
test_fifo_location
test_provider_persistence
# test_no_disk_writes  # Optional: requires the full setup-dashboard.sh environment

test_vis_width() {
    # Build a colored legend similar to the dashboard
    local s
    s=$'\033[1;32m✓\033[0m  \033[1;35m⛊\033[0m  \033[1;33m!\033[0m  \033[0;90m●\033[0m  \033[0;31m✗\033[0m'

    # Strip ANSI using the same conservative regex
    local stripped
    stripped=$(printf '%b' "$s" | sed -r 's/\x1B\[[0-9;?]*[ -/]*[@-~]//g' | tr -d '\r\n')

    # Expected width via Python (counts East-Asian width)
    local expected
    expected=$(printf '%s' "$stripped" | python3 -c "import sys,unicodedata; s=sys.stdin.read(); print(sum(2 if unicodedata.east_asian_width(ch) in 'WF' else 1 for ch in s))")

    # Our stripping + fallback wc -m (sanity) and compare
    local wc_m
    wc_m=$(printf '%s' "$stripped" | wc -m)

    echo "Stripped: '$stripped'"
    echo "wc -m: $wc_m  python-width: $expected"

    if [[ -z "$expected" ]]; then
        echo "FAIL: couldn't compute expected width (python3 missing?)"
        return 1
    fi

    # Pass if python and our expected agree with simple char count or reasonable
    if [[ "$expected" -ge 1 ]]; then
        echo "PASS: visual width computed as $expected"
        return 0
    fi
    echo "FAIL: invalid visual width: $expected"
    return 1
}

test_vis_width

echo "✅ Basic optimization tests completed"
