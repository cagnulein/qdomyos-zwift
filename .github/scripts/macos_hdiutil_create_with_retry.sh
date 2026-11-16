#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -eq 0 ]; then
    echo "Usage: $0 command [args...]" >&2
    exit 2
fi

attempt=1
while [ "$attempt" -le 3 ]; do
    log_file="$(mktemp)"
    if "$@" >"$log_file" 2>&1; then
        cat "$log_file"
        rm -f "$log_file"
        exit 0
    else
        command_status=$?
    fi

    cat "$log_file" >&2
    if ! grep -Fq "hdiutil: create failed - Resource busy" "$log_file" || [ "$attempt" -eq 3 ]; then
        rm -f "$log_file"
        exit "$command_status"
    fi

    rm -f "$log_file"
    echo "hdiutil reported a transient Resource busy error; retrying in 10 seconds (attempt $((attempt + 1))/3)." >&2
    sleep 10
    attempt=$((attempt + 1))
done

exit 1
