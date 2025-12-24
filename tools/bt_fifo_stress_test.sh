#!/bin/bash
# Simple stress test for collect_bt_updates_batch logic (standalone)
set -euo pipefail
FIFO=/tmp/qz_bt_test_fifo_$$
rm -f "$FIFO"
mkfifo "$FIFO"
# Writer: stream many lines quickly
( for i in $(seq 1 2000); do
    # emit a variety of labels including ANSI escapes
    if (( i % 10 == 0 )); then
        printf 'AA:BB:CC:DD:EE:%02d|%d|Device\033[1;32mName\033[0m\n' "$i" "$(( -30 - (i % 40) ))"
    else
        printf 'AA:BB:CC:DD:EE:%02d|%d|NormalDevice%02d\n' "$i" "$(( -30 - (i % 40) ))" "$i"
    fi
    # small jitter
    sleep 0.001
  done ) > "$FIFO" &
WRITER_PID=$!

collect_bt_updates_batch() {
    local fifo_fd="${1:-3}"
    local batch_size="${2:-256}"
    local timeout="${3:-0.05}"

    local -a batch=()
    local raw

    if read -u "$fifo_fd" -t "$timeout" -r raw 2>/dev/null; then
        batch+=("$raw")
        local short_t="0.001"
        while (( ${#batch[@]} < batch_size )); do
            if read -u "$fifo_fd" -t "$short_t" -r raw 2>/dev/null; then
                batch+=("$raw")
            else
                break
            fi
        done
    else
        return 0
    fi

    printf '%s\n' "${batch[@]}"
}

# Open FIFO FD 3 for reading
exec 3<"$FIFO"
start_time=$(date +%s.%N)
count=0
while true; do
    batch=$(collect_bt_updates_batch 3 128 0.02)
    if [[ -z "$batch" ]]; then
        # no data currently; break if writer exited
        if ! kill -0 "$WRITER_PID" 2>/dev/null; then
            break
        fi
        sleep 0.01
        continue
    fi
    # process batch (count lines)
    lines=$(printf '%s' "$batch" | wc -l)
    count=$((count + lines))
    # lightweight parse - ensure fields present
    #printf 'got batch of %d\n' "$lines"
done
end_time=$(date +%s.%N)
# cleanup
kill "$WRITER_PID" 2>/dev/null || true
exec 3<&-
rm -f "$FIFO"
elapsed=$(echo "$end_time - $start_time" | bc -l)
echo "Total lines processed: $count in ${elapsed}s"
