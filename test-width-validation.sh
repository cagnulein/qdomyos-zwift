#!/usr/bin/env bash
set -euo pipefail

SD="src/devices/antlinux/setup-dashboard.sh"
SD_BAK="src/devices/antlinux/setup-dashboard.sh_bck"

calc_ref() {
    local text="$1"
    local stripped
    stripped=$(printf '%b' "$text" | sed -r 's/\x1B\[[0-9;]*[mK]//g' | tr -d '\r\n')
    if command -v perl >/dev/null 2>&1; then
        printf '%s' "$stripped" | perl -CS -Mutf8 -0777 -ne '
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
    elif command -v gawk >/dev/null 2>&1; then
        printf '%s' "$stripped" | gawk 'BEGIN { w = 0 }
        {
            for (i = 1; i <= length($0); i++) {
                c = substr($0, i, 1)
                cw = 1
                if (c ~ /[\u1100-\u115F\u2329-\u232A\u2E80-\u303E\u3040-\u30FF\u4E00-\u9FFF\uAC00-\uD7AF\uFF01-\uFF60]/) { cw = 2 }
                else if (c ~ /[\U0001F300-\U0001F9FF\U0001F600-\U0001F64F]/) { cw = 2 }
                w += cw
            }
        }
        END { print w }'
    else
        printf '%s' "${#stripped}"
    fi
}

get_from_script() {
    local script="$1"; shift
    local text="$*"
    # Run in a subshell to avoid polluting caller terminal state
    bash -c "source \"$script\" >/dev/null 2>&1 || true; printf '%s' \"\
\$(get_vis_width \"\$1\")\"" _ "$text"
}

run_case() {
    local label="$1"; local s="$2"; shift 2
    printf 'Test: %s\n' "$label"
    local ref
    ref=$(calc_ref "$s")
    local new
    new=$(get_from_script "$SD" "$s")
    local old
    old=$(get_from_script "$SD_BAK" "$s") || old="(old-script-failed)"

    printf '  input: %s\n' "$s"
    printf '  ref:   %s\n' "$ref"
    printf '  new:   %s\n' "$new"
    printf '  old:   %s\n' "$old"

    if [[ "$ref" != "$new" ]]; then
        echo "MISMATCH (ref vs new) for '$s'"
        return 2
    fi
    if [[ "$old" != "(old-script-failed)" && "$old" != "$new" ]]; then
        echo "MISMATCH (old vs new) for '$s'"
        return 3
    fi
    echo "  OK"
}

# Basic test cases
run_case "ASCII" "Hello World"
run_case "ANSI color" "\033[0;31mRed\033[0m"
run_case "checkmark" "✓"
run_case "heavy" "⛔"
run_case "emoji" "😀"
run_case "cjk" "中文"

# Dashboard-like samples (examples)
run_case "menu" "${BLUE}╔═╗${NC} Menu: ${BOLD}Start${NC}"

echo "All tests executed.\nNote: 'old' may fail to run if sourcing the backup changed terminal state; check manually if needed."
