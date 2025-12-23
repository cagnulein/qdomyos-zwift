#!/usr/bin/env bash
set -euo pipefail

# Local cached get_vis_width (copies logic from setup-dashboard.sh)
declare -gA DISPLAY_CACHE=()

strip_ansi() {
    local text="$1"
    printf '%b' "$text" | sed -r 's/\x1B\[[0-9;]*[mK]//g' | tr -d '\r\n'
}

strip_ansi_cached_local() {
    local text="${1:-}"
    [[ -z "$text" ]] && { printf ''; return 0; }
    local cached="${DISPLAY_CACHE[$text]:-}"
    if [[ -n "$cached" ]]; then
        printf '%s' "${cached%%|*}"
        return 0
    fi
    local stripped
    stripped=$(strip_ansi "$text" )
    DISPLAY_CACHE["$text"]="${stripped}|"
    printf '%s' "$stripped"
}

get_vis_width_cached_local() {
    local text="${1:-}"
    [[ -z "$text" ]] && { echo "0"; return 0; }
    local cached="${DISPLAY_CACHE[$text]:-}"
    if [[ -n "$cached" ]]; then
        local cached_width="${cached##*|}"
        if [[ -n "$cached_width" ]]; then
            echo "$cached_width"
            return 0
        fi
        local stripped="${cached%%|*}"
    else
        local stripped
        stripped=$(strip_ansi "$text")
    fi

    local width
    if command -v perl >/dev/null 2>&1; then
        width=$(printf '%s' "$stripped" | perl -CS -Mutf8 -0777 -ne '
            use utf8;
            my @ea_ranges = (
                [0x1100, 0x115F], [0x2329, 0x232A], [0x2E80, 0x2EFF],
                [0x3000, 0x303E], [0x3040, 0x30FF], [0x3100, 0x319F],
                [0x31A0, 0x31BF], [0x3200, 0x4DBF], [0x4E00, 0xA48C],
                [0xAC00, 0xD7AF], [0xF900, 0xFAFF], [0xFE10, 0xFE19],
                [0xFE30, 0xFE6F], [0xFF01, 0xFF60], [0xFFE0, 0xFFE6]
            );
            my $width = 0;
            for my $char (split //) {
                my $cp = ord($char);
                my $cw = 1;
                for my $range (@ea_ranges) {
                    if ($cp >= $range->[0] && $cp <= $range->[1]) { $cw = 2; last; }
                }
                if ($cw == 1 && (
                    ($cp >= 0x1F300 && $cp <= 0x1F9FF) ||
                    ($cp >= 0x1FA00 && $cp <= 0x1FA6F) ||
                    ($cp >= 0x1F600 && $cp <= 0x1F64F)
                )) { $cw = 2; }
                $width += $cw;
            }
            print $width;
        '
        )
    elif command -v gawk >/dev/null 2>&1; then
        width=$(printf '%s' "$stripped" | gawk 'BEGIN { w = 0 }
        {
            for (i = 1; i <= length($0); i++) {
                c = substr($0, i, 1)
                cw = 1
                if (c ~ /[\u1100-\u115F\u2329-\u232A\u2E80-\u303E\u3040-\u30FF\u4E00-\u9FFF\uAC00-\uD7AF\uFF01-\uFF60]/) { cw = 2 }
                else if (c ~ /[\U0001F300-\U0001F9FF\U0001F600-\U0001F64F]/) { cw = 2 }
                w += cw
            }
        }
        END { print w }')
    else
        width=${#stripped}
    fi

    DISPLAY_CACHE["$text"]="${stripped}|${width}"
    echo "$width"
}

# Test string with ANSI
TEST=$'\033[0;32m'Test$'\033[0m'

# Warm-up: populate cache
get_vis_width_cached_local "$TEST" >/dev/null

echo "Running 1000 cache-hit iterations..."
set +e
TIME_HIT=$( (time for i in {1..1000}; do get_vis_width_cached_local "$TEST" >/dev/null; done) 2>&1 )
set -e

# Measure miss: clear cache each iteration
echo "Running 1000 cache-miss iterations (clearing cache each loop)..."
set +e
TIME_MISS=$( (time for i in {1..1000}; do DISPLAY_CACHE=(); get_vis_width_cached_local "$TEST" >/dev/null; done) 2>&1 )
set -e

printf "\nCache-hit timing summary:\n%s\n\nCache-miss timing summary:\n%s\n" "$TIME_HIT" "$TIME_MISS"
