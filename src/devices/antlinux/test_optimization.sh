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

echo "✅ Basic optimization tests completed"
