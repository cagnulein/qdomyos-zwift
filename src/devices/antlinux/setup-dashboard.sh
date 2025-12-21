#!/bin/bash
################################################################################
# setup_dashboard.sh - QDomyos-Zwift ANT+ Setup Dashboard
#
# Features: Guided/Auto Fix, Deep Uninstall, Live Log, Smart Config Wizard
# Updates:
#  - Fixed User Profile Input Layout (Row positioning & Border alignment)
#  - Fixed Persistence: Properly reads existing config values before regeneration
#  - Added Unit labels to input prompts
################################################################################

set -uo pipefail

# Script versioning: update when you deploy/copy this script to another host.
# Prefer semantic or date-based strings. You can also set the env var
# QZ_SETUP_DASHBOARD_VERSION to override at runtime.
SCRIPT_VERSION="2025.12.19-feat/ant_footpod-1"
SCRIPT_VERSION=${QZ_SETUP_DASHBOARD_VERSION:-$SCRIPT_VERSION}

# Compute a short, human-friendly version for display in tight headers.
# Priority: use env `QZ_SETUP_DASHBOARD_VERSION_SHORT` if set; otherwise
# extract a YYYY.MM.DD prefix if present; else take the part before the
# first hyphen.
if [[ -n "${QZ_SETUP_DASHBOARD_VERSION_SHORT:-}" ]]; then
    SCRIPT_VERSION_SHORT="$QZ_SETUP_DASHBOARD_VERSION_SHORT"
elif [[ "$SCRIPT_VERSION" =~ ([0-9]{4}\.[0-9]{2}\.[0-9]{2}) ]]; then
    SCRIPT_VERSION_SHORT="${BASH_REMATCH[1]}"
else
    SCRIPT_VERSION_SHORT="${SCRIPT_VERSION%%-*}"
fi

# Simple CLI flags: support --version (non-intrusive)
while [ "$#" -gt 0 ]; do
    case "$1" in
        --version)
            printf 'SCRIPT_VERSION=%s\n' "$SCRIPT_VERSION"
            printf 'SCRIPT_VERSION_SHORT=%s\n' "$SCRIPT_VERSION_SHORT"
            exit 0
            ;;
        --help|-h)
                printf 'Usage: %s [--version|--scan-now]\n' "${0##*/}"
            exit 0
            ;;
            --scan-now)
                SCAN_NOW=1
                NONINTERACTIVE_SHOW_CURSOR=1
                shift
                ;;
        *)
            # ignore unknown flags; the dashboard will continue normally
            shift
            ;;
    esac
done

    # Unknown flags are ignored; the dashboard will continue normally

# Emergency crash catcher
trap 'echo "SCRIPT CRASHED on line $LINENO! (Check for unbound variables if set -u is on)" >&2' ERR

# Restore terminal to a usable state
restore_terminal() {
    # Best-effort restore: some helper functions/vars may not be defined if
    # the script is terminated early. Guard all calls so the restore handler
    # never fails with 'command not found' or unbound variable errors.
    if command -v exit_ui_mode >/dev/null 2>&1; then
        exit_ui_mode || true
    else
        # Fall back to a sane stty restore
        stty echo 2>/dev/null || true
    fi
    if command -v show_cursor >/dev/null 2>&1; then
        show_cursor || true
    else
        printf '\033[?25h' >&${UI_FD:-2} 2>/dev/null || true
    fi
    # If TERM_HEIGHT is not yet set, default to 24
    local _th=${TERM_HEIGHT:-24}
    if command -v move_cursor >/dev/null 2>&1; then
        move_cursor $(( _th + 1 )) 0 2>/dev/null || true
    else
        printf "\n"
    fi
}

# Always attempt to restore the terminal on normal exit as well. immediate_exit
# will clear traps as needed to avoid recursion during forced interrupts.
trap restore_terminal EXIT

# Unified exit function
finish_and_exit() {
    restore_terminal
    exit 0
}

# This function does NOTHING but fix the terminal and exit immediately.
# No prompts, no messages.
immediate_exit() {
    # Best-effort restore (guard helpers that may not exist yet)
    if command -v exit_ui_mode >/dev/null 2>&1; then
        exit_ui_mode || true
    else
        stty echo 2>/dev/null || true
    fi
    if command -v show_cursor >/dev/null 2>&1; then
        show_cursor || true
    else
        printf '\033[?25h' >&${UI_FD:-2} 2>/dev/null || true
    fi
    if [[ -n "${LOG_BOTTOM:-}" ]]; then
        if command -v move_cursor >/dev/null 2>&1; then
            move_cursor $((LOG_BOTTOM + 1)) 0 2>/dev/null || true
        else
            printf "\n"
        fi
    else
        printf "\n"
    fi
    
    # Exit with the standard 'Interrupted' exit code
    # We un-trap first to prevent a loop if exit itself triggers EXIT
    trap - SIGINT SIGTERM EXIT
    exit 130
}

# Trap Ctrl+C (SIGINT) and Kill signals
trap immediate_exit SIGINT SIGTERM

# Disable echo for the UI
stty -echo

# ============================================================================
# COLOR DETECTION & DEFINITIONS
# ============================================================================

USE_COLOR=true
if [ ! -t 1 ] || [ "${TERM:-}" = "dumb" ]; then
    USE_COLOR=false
elif ! tput setaf 1 &>/dev/null; then
    USE_COLOR=false
fi

if [ "$USE_COLOR" = true ]; then
    RED='\033[0;31m'
    GREEN='\033[0;32m'
    YELLOW='\033[1;33m'
    BLUE='\033[0;34m'
    CYAN='\033[0;36m'
    WHITE='\033[1;37m'
    GRAY='\033[0;90m'
    NC='\033[0m'
    BOLD='\033[1m'
    BOLD_RED='\033[1;31m'
    BOLD_BLUE='\033[1;34m'
    BOLD_CYAN='\033[1;36m'   # Added for high-visibility selection
    BOLD_WHITE='\033[1;37m'
    ORANGE='\033[38;5;214m' # 256-color mode orange
    MAGENTA='\033[0;35m'
    BOLD_MAGENTA='\033[1;35m'
else
    RED=''; GREEN=''; YELLOW=''; BLUE=''; CYAN=''; WHITE=''; GRAY=''; NC=''
    BOLD=''; BOLD_RED=''; BOLD_BLUE=''; BOLD_CYAN=''; BOLD_WHITE=''
fi


SYMBOL_PASS="✓"
SYMBOL_FAIL="✗"
SYMBOL_WARN="!"
SYMBOL_PENDING="●"
SYMBOL_WORKING="⟳"
SYMBOL_LOCKED="⛊"
PROTECTED_ITEMS=("python311" "qt5_libs" "qml_modules" "bluetooth" "lsusb")

# ============================================================================
# USER CONTEXT & ENVIRONMENT
# ============================================================================

TARGET_USER="${SUDO_USER:-$USER}"
TARGET_HOME=$(getent passwd "$TARGET_USER" | cut -d: -f6)

# Attempt to open the controlling terminal early so the TUI can write
# directly to the user's terminal even when stdout/stderr are redirected.
# We open fd 3 to /dev/tty and prefer it as the UI output FD.
if [ -c /dev/tty ]; then
    # Safe non-fatal open
    exec 3>/dev/tty 2>/dev/null || true
    if [ -w /dev/tty ] 2>/dev/null; then
        UI_FD=3
    fi
fi

# Detect GUI environment
HAS_GUI=false
if [ -n "${DISPLAY:-}" ] || [ -n "${WAYLAND_DISPLAY:-}" ]; then
    HAS_GUI=true
fi

# Detect Raspberry Pi for optimization
IS_PI=false
if grep -q "Raspberry Pi" /proc/cpuinfo 2>/dev/null; then
    IS_PI=true
fi

# Configuration paths
CONFIG_DIR="$TARGET_HOME/.config/Roberto Viola"
CONFIG_FILE="$CONFIG_DIR/qDomyos-Zwift.conf"
DEVICES_INI="./devices.ini"
SERVICE_FILE_1="/etc/systemd/system/qz.service"
SERVICE_FILE_2="/lib/systemd/system/qz.service"

# Determine active service file
ACTIVE_SERVICE_FILE=""
if [ -f "$SERVICE_FILE_1" ]; then
    ACTIVE_SERVICE_FILE="$SERVICE_FILE_1"
elif [ -f "$SERVICE_FILE_2" ]; then
    ACTIVE_SERVICE_FILE="$SERVICE_FILE_2"
fi

# Defaults (Initialize as empty so we can detect if the file load fails)
PREV_WEIGHT=""
PREV_AGE=""
PREV_SEX=""
PREV_MILES=""

# ============================================================================
# TERMINAL CONTROL
# ============================================================================

# Standard cursor movement (Row, Column) - 1-based for ANSI
move_cursor() {
    local r=$(( ${1:-0} + 1 ))
    local c=$(( ${2:-0} + 1 ))
    printf "\033[%d;%dH" "$r" "$c" >&${UI_FD:-2}
}

clear_line() { printf "\033[0K" >&${UI_FD:-2}; }
clear_screen() { printf "\033[2J\033[H" >&${UI_FD:-2}; }
hide_cursor() { printf "\033[?25l" >&${UI_FD:-2}; }
show_cursor() { printf "\033[?25h" >&${UI_FD:-2}; }

# UI mode helpers: centralize terminal state changes for interactive UI sections.
# Use a simple reference count so nested callers are safe.
UI_MODE_COUNT=${UI_MODE_COUNT:-0}
_QZ_OLD_STTY=${_QZ_OLD_STTY:-}

enter_ui_mode() {
    # Increment nesting count; only perform actions on transition 0->1
    UI_MODE_COUNT=$((UI_MODE_COUNT + 1))
    if [ "$UI_MODE_COUNT" -ne 1 ]; then return 0; fi

    # Save current stty state so we can fully restore it later
    if stty -g >/dev/null 2>&1; then
        _QZ_OLD_STTY=$(stty -g 2>/dev/null || true)
    else
        _QZ_OLD_STTY=""
    fi
    # Disable local echo and hide the cursor for the UI
    # If requested (non-interactive scan), keep cursor visible and enable echo
    if [ "${NONINTERACTIVE_SHOW_CURSOR:-0}" -eq 1 ]; then
        stty echo 2>/dev/null || true
        if command -v show_cursor >/dev/null 2>&1; then
            show_cursor || true
        else
            printf '\033[?25h' 2>/dev/null || true
        fi
    else
        stty -echo 2>/dev/null || true
        hide_cursor
    fi

    # Ensure exit_ui_mode runs on unexpected RETURN/SIGINT/SIGTERM within this UI
    trap 'exit_ui_mode; trap - RETURN SIGINT SIGTERM' RETURN SIGINT SIGTERM
}

exit_ui_mode() {
    # Decrement nesting count; only restore on transition to zero
    if [ -z "${UI_MODE_COUNT:-}" ] || [ "$UI_MODE_COUNT" -le 0 ]; then
        UI_MODE_COUNT=0
        # Still perform a best-effort restore of terminal settings
        if [[ -n "${_QZ_OLD_STTY:-}" ]]; then stty "${_QZ_OLD_STTY}" 2>/dev/null || true; else stty echo 2>/dev/null || true; fi
        # Do NOT show the cursor here to avoid brief flashes during redraws.
        trap - RETURN SIGINT SIGTERM
        return 0
    fi
    UI_MODE_COUNT=$((UI_MODE_COUNT - 1))
    if [ "$UI_MODE_COUNT" -eq 0 ]; then
        if [[ -n "${_QZ_OLD_STTY:-}" ]]; then stty "${_QZ_OLD_STTY}" 2>/dev/null || true; else stty echo 2>/dev/null || true; fi
        # Do NOT show cursor; caller should explicitly show when user input is expected.
        trap - RETURN SIGINT SIGTERM
    fi
    return 0
}

TERM_HEIGHT=$(tput lines 2>/dev/null || echo 24)

# Require that an external command exists at runtime.
# Usage: require_command git || return 1
require_command() {
    local cmd
    for cmd in "$@"; do
        if ! command -v "$cmd" >/dev/null 2>&1; then
            printf '%s\n' "Missing required command: $cmd" >&2
            return 1
        fi
    done
    return 0
}


# ============================================================================
# LAYOUT CONFIGURATION
# ============================================================================ 
    
# Standard 24-line terminal geometry
LOG_TOP=12
LOG_BOTTOM=21
INFO_WIDTH=78

# ============================================================================
# STATE MANAGEMENT
# ============================================================================

declare -A STATUS_MAP=(
    ["python311"]="pending"
    ["venv"]="pending"
    ["pkg_pips"]="pending"
    ["qt5_libs"]="pending"
    ["qml_modules"]="pending"
    ["bluetooth"]="pending"
    ["lsusb"]="pending"
    ["plugdev"]="pending"
    ["udev_rules"]="pending"
    ["config_file"]="pending"
    ["qz_service"]="pending"
    ["ant_dongle"]="pending"
)


# Status grid definition: each entry is "Left Label|Left Key|Right Label|Right Key"
declare -a STATUS_GRID=(
    "Python 3.11 Library|python311|lsusb Command|lsusb"
    "Python Virtual Environment|venv|User in plugdev Group|plugdev"
    "Python PIPs|pkg_pips|USB udev Rules|udev_rules"
    "Qt5 Runtime Libraries|qt5_libs|Configuration File|config_file"
    "QML Modules|qml_modules|QZ Service|qz_service"
    "Bluetooth Service|bluetooth|ANT+ USB Dongle|ant_dongle"
)

# Render the status grid starting at given row (default 5)
render_status_grid() {
    local start_row=${1:-5}
    local row=$start_row
    for entry in "${STATUS_GRID[@]}"; do
        IFS='|' read -r L_label L_key R_label R_key <<< "$entry"
        draw_status_row "$row" "$L_label" "$L_key" "$R_label" "$R_key"
        ((row++))
    done
}

CURRENT_INSTRUCTION=""
# shellcheck disable=SC2034
FINISH_DONE=0
# shellcheck disable=SC2034
CAN_INSTALL=0
SETUP_MODE=""

# Debug logging: use an in-memory ring buffer by default to avoid disk I/O.
# Enable disk-backed debug by setting DEBUG_BT=1 in the environment.
BT_DEBUG_LOG=${BT_DEBUG_LOG:-/tmp/qz_bt_scan_debug.log}
DEBUG_BT=${DEBUG_BT:-0}
declare -a DEBUG_RING_BUFFER
DEBUG_RING_SIZE=${DEBUG_RING_SIZE:-100}
bt_debug() {
    # Early return if debugging disabled
    if [[ "${DEBUG_BT}" -eq 0 ]]; then
        return 0
    fi
    local ts
    ts=$(date +"%H:%M:%S")
    local msg="[$ts] $*"
    DEBUG_RING_BUFFER+=("$msg")
    # Keep only the last N entries
    if [[ ${#DEBUG_RING_BUFFER[@]} -gt $DEBUG_RING_SIZE ]]; then
        DEBUG_RING_BUFFER=("${DEBUG_RING_BUFFER[@]: -$DEBUG_RING_SIZE}")
    fi
}

bt_debug_dump() {
    # Dump ring buffer to a RAM-backed file for post-mortem analysis
    local dump_file="/dev/shm/qz_debug_dump_$$.log"
    mkdir -p "$(dirname "$dump_file")" 2>/dev/null || true
    printf "%s\n" "${DEBUG_RING_BUFFER[@]:-}" > "$dump_file" 2>/dev/null || true
    echo "Debug log dumped to: $dump_file" >&2
}

# Optional Python provider integration (minimal, safe wrappers).
# Prefer FIFO in shared memory to avoid SD card writes; fall back to file
BT_PROVIDER_FIFO="${BT_PROVIDER_FIFO:-/dev/shm/qz_bt_fifo_$$}"
BT_PROVIDER_SUPERVISOR_PID=0
BT_PROVIDER_PIDFILE=${BT_PROVIDER_PIDFILE:-/dev/shm/qz_bt_provider_$$.pid}
BT_PROVIDER_STOP_FILE=${BT_PROVIDER_STOP_FILE:-/dev/shm/qz_bt_provider.stop}
BT_PROVIDER_HEARTBEAT=${BT_PROVIDER_HEARTBEAT:-/dev/shm/qz_bt_heartbeat_$$}

# Ensure BT file paths are writable by this user. If /tmp files are owned by
# root (sticky /tmp prevents non-owners truncating them), fall back to per-user
# cache files under the user's home directory so non-root runs won't hit
# 'Permission denied' when truncating or appending.
resolve_bt_paths() {
    # Resolve provider stream
    BT_PROVIDER_STREAM=${BT_PROVIDER_STREAM:-/tmp/qz_bt_stream.log}
    local stream="$BT_PROVIDER_STREAM"
    # If the file exists but isn't writable by us, pick a per-user fallback
    if [ -e "$stream" ] && [ ! -w "$stream" ]; then
        local fallback="$TARGET_HOME/.cache/qz_bt_stream.log"
        mkdir -p "$(dirname "$fallback")" 2>/dev/null || true
        : > "$fallback" 2>/dev/null || true
        chmod 0666 "$fallback" 2>/dev/null || true
        BT_PROVIDER_STREAM="$fallback"
    else
        # Try to create it; if creation fails or it's not writable, use fallback
        mkdir -p "$(dirname "$stream")" 2>/dev/null || true
        : > "$stream" 2>/dev/null || true
        if [ ! -w "$stream" ]; then
            local fallback="$TARGET_HOME/.cache/qz_bt_stream.log"
            mkdir -p "$(dirname "$fallback")" 2>/dev/null || true
            : > "$fallback" 2>/dev/null || true
            chmod 0666 "$fallback" 2>/dev/null || true
            BT_PROVIDER_STREAM="$fallback"
        fi
    fi

    # Resolve debug log path similarly (kept for explicit disk dumps)
    BT_DEBUG_LOG=${BT_DEBUG_LOG:-/tmp/qz_bt_scan_debug.log}
    local dbg="$BT_DEBUG_LOG"
    if [ -e "$dbg" ] && [ ! -w "$dbg" ]; then
        local dfallback="$TARGET_HOME/.cache/qz_bt_scan_debug.log"
        mkdir -p "$(dirname "$dfallback")" 2>/dev/null || true
        : > "$dfallback" 2>/dev/null || true
        chmod 0666 "$dfallback" 2>/dev/null || true
        BT_DEBUG_LOG="$dfallback"
    else
        mkdir -p "$(dirname "$dbg")" 2>/dev/null || true
        : > "$dbg" 2>/dev/null || true
        if [ ! -w "$dbg" ]; then
            local dfallback="$TARGET_HOME/.cache/qz_bt_scan_debug.log"
            mkdir -p "$(dirname "$dfallback")" 2>/dev/null || true
            : > "$dfallback" 2>/dev/null || true
            chmod 0666 "$dfallback" 2>/dev/null || true
            BT_DEBUG_LOG="$dfallback"
        fi
    fi
}

start_bt_provider() {
    # choose a python binary inside venv if available
    local pybin="$TARGET_HOME/ant_venv/bin/python3"
    if [ ! -x "$pybin" ]; then
        pybin=$(command -v python3 || true)
    fi
    [ -n "$pybin" ] || return 1
    # provider path relative to this script
    local prov="$(dirname "$0")/bt_provider.py"
    if [ ! -f "$prov" ]; then
        # try same dir as this file path
        prov="$(dirname /home/adam/github/qdomyos-zwift/src/devices/antlinux/setup-dashboard.sh)/bt_provider.py"
    fi
    if [ ! -f "$prov" ]; then
        bt_debug "PY_PROVIDER_MISSING prov=$prov"
        return 1
    fi
    # Record an absolute provider path so stop logic can safely target it.
    BT_PROVIDER_PROV_PATH=$(readlink -f "$prov" 2>/dev/null || echo "$prov")
    # Pick writable paths for stream/debug before attempting truncation
    resolve_bt_paths

    # Prepare IPC in RAM: create FIFO and heartbeat file paths under /dev/shm
    if [[ -d /dev/shm ]]; then
        BT_PROVIDER_FIFO="/dev/shm/qz_bt_fifo_$$"
        BT_PROVIDER_PIDFILE="/dev/shm/qz_bt_provider_$$.pid"
        BT_PROVIDER_HEARTBEAT="/dev/shm/qz_bt_heartbeat_$$"
    else
        # Fallback to per-user cache directory
        BT_PROVIDER_FIFO="$TARGET_HOME/.cache/qz_bt_fifo_$$"
        BT_PROVIDER_PIDFILE="$TARGET_HOME/.cache/qz_bt_provider_$$.pid"
        BT_PROVIDER_HEARTBEAT="$TARGET_HOME/.cache/qz_bt_heartbeat_$$"
        mkdir -p "$(dirname "$BT_PROVIDER_FIFO")" 2>/dev/null || true
    fi

    # Ensure old IPC artifacts removed
    rm -f "$BT_PROVIDER_FIFO" "$BT_PROVIDER_HEARTBEAT" 2>/dev/null || true
    # Create FIFO (if creation fails, fall back to a regular file stream)
    if ! mkfifo "$BT_PROVIDER_FIFO" 2>/dev/null; then
        : > "$BT_PROVIDER_FIFO" 2>/dev/null || true
    fi
    chmod 0666 "$BT_PROVIDER_FIFO" 2>/dev/null || true
    # If running as root, try to chown the stream to the target user so
    # non-root provider processes can write without permission issues.
    if [ "$(id -u)" -eq 0 ] && [ -n "${TARGET_USER:-}" ]; then
        chown "$TARGET_USER":"$TARGET_USER" "$BT_PROVIDER_STREAM" 2>/dev/null || true
    fi
    # Ensure debug log exists and is writable by the target user as well
    BT_DEBUG_LOG=${BT_DEBUG_LOG:-/tmp/qz_bt_scan_debug.log}
    : > "$BT_DEBUG_LOG" 2>/dev/null || true
    chmod 0666 "$BT_DEBUG_LOG" 2>/dev/null || true
    if [ "$(id -u)" -eq 0 ] && [ -n "${TARGET_USER:-}" ]; then
        chown "$TARGET_USER":"$TARGET_USER" "$BT_DEBUG_LOG" 2>/dev/null || true
    fi
    # If a supervisor is already running, nothing to do
    if [ -n "${BT_PROVIDER_SUPERVISOR_PID:-}" ] && ps -p "${BT_PROVIDER_SUPERVISOR_PID}" >/dev/null 2>&1; then
        bt_debug "PY_PROVIDER_SUPERVISOR_ALREADY_RUNNING pid=${BT_PROVIDER_SUPERVISOR_PID}"
        return 0
    fi

    # Ensure old stop file removed
    rm -f "$BT_PROVIDER_STOP_FILE" 2>/dev/null || true
    rm -f "$BT_PROVIDER_PIDFILE" 2>/dev/null || true

    # Start provider once and run a lightweight watchdog that checks heartbeat
    (
        # Launch provider and pass heartbeat path; provider writes heartbeat periodically
        "$pybin" "$prov" --heartbeat="$BT_PROVIDER_HEARTBEAT" >"$BT_PROVIDER_FIFO" 2>/dev/null &
        local pp=$!
        echo "$pp" > "$BT_PROVIDER_PIDFILE" 2>/dev/null || true
        bt_debug "PY_PROVIDER_STARTED pid=$pp fifo=$BT_PROVIDER_FIFO hb=$BT_PROVIDER_HEARTBEAT"

        # Watchdog: check every 5s for heartbeat freshness and process liveness
        while true; do
            sleep 5
            # Stop requested?
            [ -f "$BT_PROVIDER_STOP_FILE" ] && break

            # If process died, break and allow supervisor to exit
            if ! ps -p "$pp" >/dev/null 2>&1; then
                bt_debug "PY_PROVIDER_EXITED pid=$pp"
                break
            fi

            # Check heartbeat timestamp (age in seconds)
            if [ -f "$BT_PROVIDER_HEARTBEAT" ]; then
                local age
                age=$(( $(date +%s) - $(stat -c %Y "$BT_PROVIDER_HEARTBEAT" 2>/dev/null || echo 0) ))
                if [ "$age" -gt 10 ]; then
                    bt_debug "PROVIDER_STALLED age=${age}s pid=$pp"
                    kill "$pp" 2>/dev/null || true
                    break
                fi
            else
                # Missing heartbeat file may indicate provider failed to initialize
                bt_debug "PROVIDER_NO_HEARTBEAT pid=$pp"
            fi
        done

        # Cleanup provider if still running
        if ps -p "$pp" >/dev/null 2>&1; then
            kill "$pp" 2>/dev/null || true
        fi
        rm -f "$BT_PROVIDER_PIDFILE" "$BT_PROVIDER_HEARTBEAT" "$BT_PROVIDER_FIFO" 2>/dev/null || true
    ) &
    BT_PROVIDER_SUPERVISOR_PID=$!
    bt_debug "PY_PROVIDER_SUPERVISOR_STARTED pid=$BT_PROVIDER_SUPERVISOR_PID"
    return 0
}

stop_bt_provider() {
    # Signal supervisor to stop
    touch "$BT_PROVIDER_STOP_FILE" 2>/dev/null || true

    # Kill provider if pidfile exists
    if [ -f "$BT_PROVIDER_PIDFILE" ]; then
        local ppid
        ppid=$(cat "$BT_PROVIDER_PIDFILE" 2>/dev/null || true)
        if [ -n "$ppid" ]; then
            kill "$ppid" 2>/dev/null || true
            for i in {1..10}; do
                if ps -p "$ppid" >/dev/null 2>&1; then sleep 0.05; else break; fi
            done
        fi
        rm -f "$BT_PROVIDER_PIDFILE" 2>/dev/null || true
    else
        # No pidfile; try a conservative kill that only targets processes
        # whose command line contains the exact provider path and are owned
        # by the target user. This avoids killing unrelated apps with
        # similar names.
        local owner_user="${TARGET_USER:-$(id -un)}"
        if [ -n "${BT_PROVIDER_PROV_PATH:-}" ]; then
            # pgrep for processes matching the full path owned by owner_user
            local pids
            pids=$(pgrep -u "$owner_user" -f "${BT_PROVIDER_PROV_PATH}" || true)
            if [ -n "$pids" ]; then
                kill $pids 2>/dev/null || true
                for pid in $pids; do
                    for i in {1..10}; do
                        if ps -p "$pid" >/dev/null 2>&1; then sleep 0.05; else break; fi
                    done
                done
            else
                # As a last-resort conservative fallback, look for python processes
                # that reference a bt_provider.py basename and are owned by owner_user.
                local basename
                basename=$(basename "${BT_PROVIDER_PROV_PATH}" || true)
                if [ -n "$basename" ]; then
                    pids=$(pgrep -u "$owner_user" -f "[p]ython.*${basename}" || true)
                    if [ -n "$pids" ]; then
                        kill $pids 2>/dev/null || true
                    fi
                fi
            fi
        else
            # No prov path known; do nothing rather than risk killing unrelated processes
            bt_debug "PY_PROVIDER_NO_PROV_PATH_SKIP_KILL"
        fi
    fi

    # Kill supervisor if present
    if [ -n "${BT_PROVIDER_SUPERVISOR_PID:-}" ]; then
        kill "$BT_PROVIDER_SUPERVISOR_PID" 2>/dev/null || true
        for i in {1..10}; do
            if ps -p "$BT_PROVIDER_SUPERVISOR_PID" >/dev/null 2>&1; then sleep 0.05; else break; fi
        done
        BT_PROVIDER_SUPERVISOR_PID=0
    fi

    # Clean up stop file and any RAM IPC artifacts
    rm -f "$BT_PROVIDER_STOP_FILE" "$BT_PROVIDER_PIDFILE" "$BT_PROVIDER_FIFO" "$BT_PROVIDER_HEARTBEAT" 2>/dev/null || true
    bt_debug "PY_PROVIDER_STOPPED"
}


# Optional delay between status/UI checks (seconds). Set via env var `CHECK_DELAY`.
# Default is 0 (no artificial delays). Use fractional values like 0.1 if desired.

# ============================================================================
# DISPLAY WIDTH CALCULATION
# ============================================================================

# shellcheck disable=SC2034
# Cache mapping: stripped_string -> numeric display width
declare -gA WIDTH_CACHE
# Cache for ANSI-stripped strings to avoid repeated sed work in UI rendering
declare -gA STRIPPED_CACHE
# Cache for pre-computed ANSI cursor position sequences (row -> escape seq)
declare -gA ANSI_CACHE

# Eagerly pre-compute common cursor sequences to avoid arithmetic in hot path
# Precompute rows 0..100 which covers typical dashboard sizes
for ((_r=0; _r<=100; _r++)); do
    ANSI_CACHE[$_r]="\033[$(( _r + 1 ));1H"
done

display_width() {
    local s="$1"
    # Remove ANSI color codes
    # Backwards-compatible wrapper for legacy callers
    get_display_width "$s"
}

pad_display() {
    local s="$1"
    local width="$2"
    local w
    w=$(get_display_width "$s")
    local pad=$((width - w))
    [ "$pad" -lt 0 ] && pad=0
    # Append the calculated spaces to the text
    printf "%s%${pad}s" "$s" ""
}

# ============================================================================
# DRAWING FUNCTIONS
# ============================================================================

# Standard dashboard print (Now unbuffered)
print_at() {
    local row=$1
    shift
    # Ensure the cursor is hidden while we position/print the line
    if command -v hide_cursor >/dev/null 2>&1; then
        hide_cursor || true
    fi
    # Move cursor to Row, Column 1 using centralized helper and print to UI fd
    move_cursor "$row" 0
    [ "$#" -gt 0 ] && printf "%b" "$@" >&${UI_FD:-2}
}

# Determine the best output file descriptor for UI rendering.
# Prefer the controlling terminal (/dev/tty) when available so the UI
# appears even if stdout/stderr are redirected. Falls back to stderr.
set_ui_output() {
    UI_FD=2
    # If /dev/tty exists and is a character device, open fd 3 to it
    if [ -c /dev/tty ] && [ -w /dev/tty ] 2>/dev/null; then
        # Try to open fd 3 for writing to /dev/tty
        exec 3>/dev/tty 2>/dev/null || true
        if [ -w /dev/tty ] 2>/dev/null; then
            UI_FD=3
            return 0
        fi
    fi
    # If stderr is a tty, use it
    if [ -t 2 ]; then UI_FD=2; return 0; fi
    # If stdout is a tty, use it
    if [ -t 1 ]; then UI_FD=1; return 0; fi
    # Default to stderr
    UI_FD=2
}

draw_sealed_row() {
    local row=$1
    local text="${2:-}"
    local w=$(get_vis_width "$text")
    local pad=$(( 78 - w ))
    [[ $pad -lt 0 ]] && pad=0
    
    local spacer=""
    for ((i=0; i<pad; i++)); do spacer="${spacer} "; done
    
    # Unbuffered print to stderr
    printf "\033[%d;1H${BLUE}║${NC}${text}${spacer}${BLUE}║${NC}" "$((row + 1))" >&${UI_FD:-2}
}

# Canonical display width function: strips ANSI sequences and counts characters
get_display_width() {
    local text="${1:-}"
    # Use cached ANSI-stripped value when available to avoid repeated sed calls
    local plain
    plain=$(strip_ansi_cached "$text")

    # If the stripped string is empty, return 0 (avoid empty-key array access)
    if [[ -z "$plain" ]]; then
        printf '0'
        return 0
    fi

    # Check WIDTH_CACHE for numeric width keyed by the stripped text
    if [[ -n "${WIDTH_CACHE[$plain]:-}" ]]; then
        printf '%s' "${WIDTH_CACHE[$plain]}"
        return 0
    fi

    local len
    len=$(printf '%s' "$plain" | wc -m)
    WIDTH_CACHE["$plain"]="$len"
    printf '%s' "${len:-0}"
}

# Strip ANSI sequences from a string and cache the result.
# Uses the full input string as the cache key (safe for shell associative arrays).
strip_ansi_cached() {
    local text="${1:-}"
    # Fast-return for empty
    [[ -z "$text" ]] && { printf '' ; return 0; }

    # If we already cached the stripped version, return it
    if [[ -n "${STRIPPED_CACHE[$text]:-}" ]]; then
        printf '%s' "${STRIPPED_CACHE[$text]}"
        return 0
    fi

    # Strip ANSI escapes and control newlines, then cache
    local stripped
    stripped=$(printf '%b' "$text" | sed 's/\x1b\[[0-9;]*[a-zA-Z]//g' | tr -d '\r\n')
    STRIPPED_CACHE["$text"]="$stripped"
    printf '%s' "$stripped"
}

# Helper: Get visual width (ignores ANSI; counts Unicode display columns)
# Uses Python's unicodedata.east_asian_width() when available to count
# fullwidth/wide characters as 2 columns. Falls back to wc -m if Python
# is not present. Always strips ANSI sequences before measuring.
get_vis_width() {
    local raw="${1:-}"
    # Expand escape sequences then strip common CSI/OSC sequences robustly
    local expanded stripped
    expanded=$(printf '%b' "$raw" 2>/dev/null || printf '%s' "$raw")
    # Use a conservative ANSI/C0 strip that covers CSI and many terminals
    stripped=$(printf '%s' "$expanded" | sed -r 's/\x1B\[[0-9;?]*[ -/]*[@-~]//g' | tr -d '\r\n')

    # Empty -> width 0
    if [[ -z "$stripped" ]]; then
        echo -n 0
        return 0
    fi

    # Prefer Python for accurate column widths (handles East Asian width)
    if command -v python3 >/dev/null 2>&1; then
        local w
        w=$(printf '%s' "$stripped" | python3 - <<'PY'
import sys,unicodedata
text=sys.stdin.read()
width=0
for ch in text:
    ea=unicodedata.east_asian_width(ch)
    width += 2 if ea in 'WF' else 1
print(width)
PY
)
        echo -n "$w"
        return 0
    fi

    # Fallback: character count (may be incorrect for some CJK glyphs)
    echo -n "$(printf '%s' "$stripped" | wc -m)"
}

get_width()     { get_display_width "$1"; }

get_symbol() {
    local key="$1"
    local status="${STATUS_MAP[$key]:-pending}"

    # Padlock Logic for GUI environments
    if [ "$HAS_GUI" = true ] && [[ " ${PROTECTED_ITEMS[*]} " == *" $key "* ]] && [ "$status" == "pass" ]; then
        # We use BOLD_MAGENTA to make the lock distinctive
        printf "${BOLD_MAGENTA}${SYMBOL_LOCKED}${NC}"
        return
    fi

    case "$status" in
        pass)    printf "${GREEN}${SYMBOL_PASS}${NC}" ;;
        fail)    printf "${RED}${SYMBOL_FAIL}${NC}" ;;
        warn)    printf "${YELLOW}${SYMBOL_WARN}${NC}" ;;
        working) printf "${CYAN}${SYMBOL_WORKING}${NC}" ;;
        *)       printf "${GRAY}${SYMBOL_PENDING}${NC}" ;;
    esac
}

draw_status_row() {
    local row=$1 L_label="$2" L_key="$3" R_label="$4" R_key="$5"
    
    # 1. Get icons (✓, ✗, or ⚿)
    local L_sym
    local R_sym
    L_sym=$(get_symbol "$L_key")
    R_sym=$(get_symbol "$R_key")
    
    # 2. Build strings using CYAN for labels to harmonize with System Info
    local L_content="${L_sym} ${CYAN}${L_label}${NC}"
    local R_content="${R_sym} ${CYAN}${R_label}${NC}"
    
    # 3. Calculate padding (37 left, 38 right to fill 78 chars exactly)
    local L_padded
    local R_padded
    L_padded=$(pad_display "$L_content" 37)
    R_padded=$(pad_display "$R_content" 38)
    
    # 4. Print the row
    print_at "$row" "${BLUE}║${NC} ${L_padded}${BLUE}│${NC}${R_padded} ${BLUE}║${NC}"
}

draw_header_config_line() {
    local inner_w=78
    # Initialize defaults
    local cfg_sym="$SYMBOL_FAIL"
    local cfg_color="$RED"
    local cfg_path="$CONFIG_FILE"
    
    if [[ -f "$CONFIG_FILE" ]]; then 
        cfg_sym="$SYMBOL_PASS"
        cfg_color="$GREEN"
    fi
    
    local line="  Config:  ${cfg_color}${cfg_sym}${CYAN} ${cfg_path}"
    # Target Row 2
    print_at 2 "${BLUE}║${CYAN}$(pad_display "$line" $inner_w)${BLUE}║${NC}"
}

draw_header_service_line() {
    local inner_w=78
    # 1. Initialize variables with defaults to prevent 'unbound variable' errors
    local svc_sym="●"
    local svc_color="$GRAY"
    local svc_path="/etc/systemd/system/qz.service"
    
    # 2. Logic to determine service state
    if [[ -n "${ACTIVE_SERVICE_FILE:-}" ]] && [[ -f "$ACTIVE_SERVICE_FILE" ]]; then
        svc_sym="$SYMBOL_PASS"
        svc_color="$GREEN"
        svc_path="$ACTIVE_SERVICE_FILE"
    elif [[ "$HAS_GUI" = false ]]; then
        # On headless systems, missing service is a warning
        svc_sym="$SYMBOL_WARN"
        svc_color="$YELLOW"
    fi
    
    # 3. Build the line carefully
    # We use explicit colors for the symbol, then switch back to CYAN for the path
    local line="  Service: ${svc_color}${svc_sym}${CYAN} ${svc_path}"
    
    # 4. Print to Row 3 (New Layout)
    print_at 3 "${BLUE}║${CYAN}$(pad_display "$line" $inner_w)${BLUE}║${NC}"
}

draw_top_panel() {
    local inner_w=78
    # Top Border (use a dedicated routine that pads naked text first)
    draw_top_title 0 "╔" "╗" "QZ ANT+ BRIDGE SETUP & DIAGNOSTICS UTILITY" "$BOLD_WHITE"

    # User/Environment/Paths (Standard rows)
    local env_str=$([[ "$HAS_GUI" == true ]] && echo "GUI (X11/Wayland)" || echo "Headless")
    print_at 1 "${BLUE}║${CYAN}$(pad_display "  User: $TARGET_USER | Environment: $env_str" $inner_w)${BLUE}║${NC}"
    draw_header_config_line
    draw_header_service_line

    # Status Header with Legend
    local l_pass="${GREEN}${SYMBOL_PASS}${BLUE} Ready"
    local l_lock="${BOLD_MAGENTA}${SYMBOL_LOCKED}${BLUE} Protected"
    local l_warn="${YELLOW}${SYMBOL_WARN}${BLUE} Warning"
    local l_serv="${GRAY}${SYMBOL_PENDING}${BLUE} Service"
    local l_fail="${RED}${SYMBOL_FAIL}${BLUE} Missing"
    local full_legend="${l_pass}  ${l_lock}  ${l_warn}  ${l_serv}  ${l_fail} "

    # Arguments: Row, LeftCorner, RightCorner, Text, TextColor, Legend
    draw_hr 4 "╠" "╣" "STATUS" "$BOLD_WHITE" "$full_legend"
    render_status_grid 5
}

# Draw the top title row using naked text for width math, then apply color.
# Usage: draw_top_title <row> <left_corner> <right_corner> <text> <text_color>
draw_top_title() {
    local row=$1 left_c=$2 right_c=$3 text=${4:-} t_color=${5:-}
    local inner_w=78
    # Build naked inner content (no ANSI) that will be padded to inner_w
    local naked_inner="═══  ${text}  "
    local padded
    padded=$(pad_display "$naked_inner" $inner_w)

    # Simpler: build colored inner including ANSI and let pad_display account for it
    local inner_content colored_inner
    inner_content="═══  ${t_color}${text}${NC}  "
    colored_inner=$(pad_display "$inner_content" $inner_w)

    # Print the framed line (corner + inner + corner) atomically
    printf "%s%s%s" "${BLUE}${left_c}" "$colored_inner" "${right_c}${NC}" >&${UI_FD:-2}
}

# Usage: draw_hr <row> <left_corner> <right_corner> <text> <text_color> [legend_text]
draw_hr() {
    local row=$1
    local left_c=$2
    local right_c=$3
    local text="${4:-}"
    local t_color="${5:-$BOLD_WHITE}"
    local legend="${6:-}"
    local inner_w=78

    # 1. Position cursor at the start of the line
    printf "\033[%d;1H" "$((row + 1))" >&${UI_FD:-2}

    if [[ -z "$text" && -z "$legend" ]]; then
        # Solid Blue border
        local fill=""
        for ((i=0; i<inner_w; i++)); do fill="${fill}═"; done
        printf "${BLUE}${left_c}${fill}${right_c}${NC}" >&${UI_FD:-2}
    else
        # 2. Prepare stripped text and legend, then calculate visible widths
        local expanded stripped_text stripped_legend
        expanded=$(printf '%b' "${text:-}" 2>/dev/null || printf '%s' "${text:-}")
        stripped_text=$(printf '%s' "$expanded" | sed -r 's/\x1B\[[0-9;?]*[ -/]*[@-~]//g' | tr -d '\r\n')
        expanded=$(printf '%b' "${legend:-}" 2>/dev/null || printf '%s' "${legend:-}")
        stripped_legend=$(printf '%s' "$expanded" | sed -r 's/\x1B\[[0-9;?]*[ -/]*[@-~]//g' | tr -d '\r\n')

        # Compute visible column widths from stripped strings (prefer Python)
        local t_vis l_vis
        if command -v python3 >/dev/null 2>&1; then
            t_vis=$(printf '%s' "$stripped_text" | python3 -c "import sys,unicodedata; s=sys.stdin.read(); print(sum(2 if unicodedata.east_asian_width(ch) in 'WF' else 1 for ch in s))")
            l_vis=$(printf '%s' "$stripped_legend" | python3 -c "import sys,unicodedata; s=sys.stdin.read(); print(sum(2 if unicodedata.east_asian_width(ch) in 'WF' else 1 for ch in s))")
        else
            t_vis=$(printf '%s' "$stripped_text" | wc -m)
            l_vis=$(printf '%s' "$stripped_legend" | wc -m)
        fi

        # consumed: left corner + fixed decorations + text + legend + right decorations
        local left_pad=5   # e.g. '╔═══  '
        local right_pad=3  # e.g. '══╗'
        local sep_pad=2    # spaces between parts
        local fill_len=$(( inner_w - left_pad - t_vis - sep_pad - l_vis - right_pad ))
        [[ $fill_len -lt 0 ]] && fill_len=0
        
        local fill=""
        for ((i=0; i<fill_len; i++)); do fill="${fill}═"; done

        # 3a. Build stripped (no-ANSI) version for logging/measurement
        local expanded stripped_text stripped_legend left_piece right_piece
        expanded=$(printf '%b' "${text:-}" 2>/dev/null || printf '%s' "${text:-}")
        stripped_text=$(printf '%s' "$expanded" | sed -r 's/\x1B\[[0-9;?]*[ -/]*[@-~]//g' | tr -d '\r\n')
        expanded=$(printf '%b' "${legend:-}" 2>/dev/null || printf '%s' "${legend:-}")
        stripped_legend=$(printf '%s' "$expanded" | sed -r 's/\x1B\[[0-9;?]*[ -/]*[@-~]//g' | tr -d '\r\n')
        left_piece="${left_c}═══  "
        right_piece="══${right_c}"
        local nocolor_line
        # Construct the no-color line and enforce exact target width (80 cols)
        local target_total=80
        nocolor_line="${left_piece}${stripped_text}  ${fill}${stripped_legend}${right_piece}"
        # Measure and adjust if necessary: pad or trim the filler to reach target_total
        local actual_len
        actual_len=$(printf '%s' "$nocolor_line" | wc -m)
        if [[ $actual_len -ne $target_total ]]; then
            local delta=$(( target_total - actual_len ))
            if [[ $delta -gt 0 ]]; then
                # Need to pad with spaces before the right_piece
                nocolor_line="${left_piece}${stripped_text}  ${fill}${stripped_legend}$(printf '%*s' "$delta" '')${right_piece}"
            else
                # Too long: try to reduce filler first, else truncate stripped_text
                delta=$(( -delta ))
                local f_len
                f_len=$(printf '%s' "$fill" | wc -m)
                if [[ $f_len -ge $delta ]]; then
                    # remove delta chars from fill
                    local new_f_len=$(( f_len - delta ))
                    fill=$(printf '%*s' "$new_f_len" '' | tr ' ' '═')
                    nocolor_line="${left_piece}${stripped_text}  ${fill}${stripped_legend}${right_piece}"
                else
                    # not enough in fill: truncate stripped_text to fit
                    local keep=$(( ${#stripped_text} - (delta - f_len) ))
                    if [[ $keep -lt 0 ]]; then keep=0; fi
                    stripped_text=$(printf '%s' "${stripped_text}" | cut -c 1-$keep)
                    fill=""
                    nocolor_line="${left_piece}${stripped_text}  ${fill}${stripped_legend}${right_piece}"
                    # final pad if still short
                    actual_len=$(printf '%s' "$nocolor_line" | wc -m)
                    if [[ $actual_len -lt $target_total ]]; then
                        local pad=$(( target_total - actual_len ))
                        nocolor_line="${nocolor_line:0:-${#right_piece}}$(printf '%*s' "$pad" '')${right_piece}"
                    fi
                fi
            fi
        fi

        # Log the stripped horizontal line to a temp file for inspection if enabled
        # Default file: /tmp/qz_drawn_lines.log (append)
        if [[ -n "${QZ_HR_LOG:-}" ]]; then
            printf '%s\n' "$nocolor_line" >> "$QZ_HR_LOG" 2>/dev/null
        else
            printf '%s\n' "$nocolor_line" >> /tmp/qz_drawn_lines.log 2>/dev/null
        fi
        # Also log numeric diagnostics to help trace overlong fills
        local diag_log=${QZ_HR_LOG:-/tmp/qz_drawn_lines.log}
        local _tlen _llen _fill
        _tlen=$(printf '%s' "$stripped_text" | wc -m)
        _llen=$(printf '%s' "$stripped_legend" | wc -m)
        _fill=$(printf '%s' "$fill" | wc -m)
        printf 'DEBUG row=%d inner_w=%d t_vis=%s l_vis=%s fill_len=%d t_len=%d l_len=%d fill_chars=%d\n' \
            "$row" "$inner_w" "$t_vis" "$l_vis" "$fill_len" "$_tlen" "$_llen" "$_fill" >> "$diag_log"
        # Log stripped text and legend snippets for debugging (safe truncated)
        printf 'DEBUG_TEXT:"%s"\n' "$(printf '%s' "$stripped_text" | cut -c1-80)" >> "$diag_log"
        printf 'DEBUG_LEGEND:"%s"\n' "$(printf '%s' "$stripped_legend" | cut -c1-160)" >> "$diag_log"

        # 3b. Build and Print as one atomic operation to prevent flickering
        # [Corner] + [  ] + [COLOR+TEXT+NC] + [  ] + [FILL] + [LEGEND] + [══] + [Corner]
        printf "%s%s%s" "${BLUE}${left_c}" "$colored_inner" "${right_c}${NC}" >&${UI_FD:-2}
    fi

    # 4. CRITICAL: Always restore cursor to the top of the log area to
    # prevent terminal scrolling/push-up after drawing a horizontal rule.
    printf "\033[%d;1H" "$((LOG_TOP + 1))" >&${UI_FD:-2}
    hide_cursor
}

draw_bottom_border() {
    local help_text="${1:-}"
    local b_row=$((LOG_BOTTOM + 1))
    # Footer uses BOLD_BLUE for the text, No Legend
    # Pass naked help text and color separately so width math ignores ANSI
    draw_hr "$b_row" "╚" "╝" "$help_text" "$BOLD_BLUE" ""
}

# Clear the info/interactive area between LOG_TOP and LOG_BOTTOM
clear_info_area() {
    for ((r=LOG_TOP; r<=LOG_BOTTOM; r++)); do
        draw_sealed_row "$r" ""
    done
}
# Draw a full-screen error panel inside the info area.
# Usage: draw_error_screen "TITLE" "Message text (can be multi-line)" [wait_enter]
draw_error_screen() {
    local title="${1:-ERROR}"
    local msg="${2:-An unexpected error occurred.}"
    local wait_enter=${3:-1}

    # Enter UI mode (hide cursor) while we render
    enter_ui_mode

    # Clear interactive area
    clear_info_area

    # Title row
    local row=$((LOG_TOP + 1))
    draw_sealed_row "$row" "   ${BOLD_RED}${title}${NC}"

    # Wrap the message to the info width (leave padding)
    local wrapped
    IFS=$'\n' read -r -d '' -a wrapped < <(printf '%s' "$msg" | fold -s -w $((INFO_WIDTH - 3)) && printf '\0')

    for line in "${wrapped[@]}"; do
        row=$((row + 1))
        draw_sealed_row "$row" "   ${RED}${line}${NC}"
    done

    draw_bottom_border "Press ENTER to continue"

    if [[ "$wait_enter" -eq 1 ]]; then
        # Restore cursor for typing and wait for ENTER
        exit_ui_mode
        show_cursor
        move_cursor $((LOG_BOTTOM + 1)) 0
        local k
        IFS= read -rsn1 k
        # Re-enter UI mode for subsequent redraws
        enter_ui_mode
    fi

    # Clear the error area after dismiss
    clear_info_area
    exit_ui_mode
}

draw_bottom_panel_header() {
    local raw_title="${1:-INFORMATION}"
    local title=$(echo "$raw_title" | tr '[:lower:]' '[:upper:]')
    # Row 11, Tab Style, No Legend
    draw_hr 11 "╠" "╣" "$title" ""
}

draw_instructions_bottom() {
    local text="$1"
    local lines=()
    while IFS= read -r line; do
        lines+=("$line")
    done < <(echo "$text" | fold -s -w 76)
    
    local total_lines=${#lines[@]}
    local show_lines=$((LOG_BOTTOM - LOG_TOP + 1))
    [ "$show_lines" -lt 1 ] && show_lines=1
    
    local start=0
    [ "$total_lines" -gt "$show_lines" ] && start=$((total_lines - show_lines))
    
    for ((idx=0; idx<show_lines; idx++)); do
        local content=""
        local li=$((start + idx))
        [ "$li" -lt "${#lines[@]}" ] && content="${lines[$li]}"
        
        print_at $((LOG_TOP + idx)) "${BLUE}║${NC}$(pad_display " $content" $INFO_WIDTH)${BLUE}║${NC}"
    done
    
    local start_fill=$((LOG_TOP + show_lines))
    for ((r=start_fill; r<=LOG_BOTTOM; r++)); do
        print_at "$r" "${BLUE}║${NC}$(pad_display "" $INFO_WIDTH)${BLUE}║${NC}"
    done
}


refresh_dashboard() {
    clear_screen
    enter_ui_mode
    draw_top_panel
    draw_bottom_panel_header "INFORMATION"
    draw_instructions_bottom "$CURRENT_INSTRUCTION"
    draw_bottom_border
}

update_status() {
    STATUS_MAP["$1"]="$2"
    # Re-render the whole grid so status icons stay consistent
    render_status_grid 5
}

# ============================================================================
# CONFIG HELPERS
# ============================================================================

get_config_key() {
    local key="$1"
    if [ -f "$CONFIG_FILE" ]; then
        grep -E "^${key}=" "$CONFIG_FILE" | tail -n 1 | cut -d'=' -f2 | tr -d '[:space:]\r"'
    else
        echo ""
    fi
}

load_current_profile_values() {
    # dbg_row was used during debugging; keep layout stable but not needed now.
    
    if [ -f "$CONFIG_FILE" ]; then
        extract_val() {
            local key="$1"
            # Grep, tail, cut, and strip decimals/spaces/quotes
            grep -E "^${key}=" "$CONFIG_FILE" | tail -n 1 | cut -d'=' -f2 | tr -d '[:space:]\r"' | cut -d. -f1
        }

        local w
        local a
        local s
        local m
        w=$(extract_val "weight")
        a=$(extract_val "age")
        s=$(extract_val "sex")
        m=$(extract_val "miles_unit")

        # Only update if the value found is not empty
        # Sanitize numeric inputs and validate ranges to avoid corrupted values
        if [[ -n "$w" ]]; then
            local _w_sanitized
            _w_sanitized="${w//[^0-9.]/}"
            # Accept weights between 20 and 300; otherwise ignore
            if [[ -n "$_w_sanitized" ]]; then
                if (( ${_w_sanitized%%.*} >= 20 && ${_w_sanitized%%.*} <= 300 )); then
                    PREV_WEIGHT="$_w_sanitized"
                fi
            fi
        fi
        if [[ -n "$a" ]]; then
            local _a_sanitized
            _a_sanitized="${a//[^0-9]/}"
            # Accept ages between 1 and 120; otherwise ignore
            if [[ -n "$_a_sanitized" ]]; then
                if (( _a_sanitized >= 1 && _a_sanitized <= 120 )); then
                    PREV_AGE="$_a_sanitized"
                fi
            fi
        fi
        [[ -n "$s" ]] && PREV_SEX="$s"
        [[ -n "$m" ]] && PREV_MILES="$m"
    fi
}

write_base_config() {
    # If the file already exists, do NOT overwrite it with defaults
    if [ -f "$CONFIG_FILE" ]; then
        return 0
    fi

    mkdir -p "$(dirname "$CONFIG_FILE")"
    
    cat > "$CONFIG_FILE" <<EOF
[General]
# --- Core Settings ---
virtual_device_enabled=true
virtual_device_bluetooth=true
wahoo_rgt_dircon=false
miles_unit=false
log_debug=false
top_bar_enabled=true

# --- User Profile ---
weight=75
age=35
sex=Male
height=175.0
EOF
    
    chown "$TARGET_USER": "$(dirname "$CONFIG_FILE")"
    chown "$TARGET_USER": "$CONFIG_FILE"
}

# The fix for path-based configuration updates
update_config_key() {
    local key="$1"
    local value="$2"
    [[ -z "$key" ]] && return

    # Strip decimal if weight/age
    if [[ "$key" == "age" || "$key" == "weight" ]]; then
        value=$(echo "$value" | cut -d. -f1)
    fi

    if [[ -f "$CONFIG_FILE" ]]; then
        if grep -q "^${key}=" "$CONFIG_FILE"; then
            # THE FIX: Use a non-slash delimiter (@) and DO NOT 
            # put the variable in the regex part.
            sed -i "s@^${key}=.*@${key}=${value}@" "$CONFIG_FILE"
        else
            echo "${key}=${value}" >> "$CONFIG_FILE"
        fi
    fi
}

prompt_numeric_input() {
    local label="$1"
    local unit="$2"
    local current_val="$3"
    local row="$4"

    local label_str="  ${label} (${unit}): "
    local buffer=""

    # 1. Enter RAW mode (The Research Method)
    local old_stty
    old_stty=$(stty -g)
    stty -icanon -echo min 1 time 0 2>/dev/null

    while true; do
        # 2. Preparation for Display
        local display_val="${buffer:-[${current_val}]}"
        local val_color="${WHITE}"
        [[ -z "$buffer" ]] && val_color="${GRAY}"

        # 3. UNBUFFERED REDRAW
        # We draw the row and the border instructions in one loop pass
        draw_sealed_row "$row" "${label_str}${val_color}${display_val}${NC}"
        draw_bottom_border "Numbers only | ENTER to confirm"

        # 4. PRECISE CURSOR POSITIONING
        # ║(1) + Spacer(1) + label_str + buffer_len
        local cursor_col=$(( 2 + ${#label_str} + ${#buffer} ))
        # If we are showing the placeholder [75], put cursor at index 1 (after '[')
        [[ -z "$buffer" ]] && cursor_col=$(( cursor_col + 1 ))

        # Move cursor and ensure it is visible
        printf "\033[%d;%dH" "$((row + 1))" "$((cursor_col + 1))" >&${UI_FD:-2}
        show_cursor

        # 5. CAPTURE RAW BYTE
        local char
        char=$(dd bs=1 count=1 2>/dev/null </dev/tty)

        case "${char}" in
            $'\x0a'|$'\x0d'|"") # ENTER
                [[ -z "${buffer}" ]] && buffer="${current_val}"
                break ;;
            $'\x7f'|$'\x08') # BACKSPACE
                [[ ${#buffer} -gt 0 ]] && buffer="${buffer%?}" ;;
            [0-9.]) # NUMBERS / DOT
                [[ ${#buffer} -lt 10 ]] && buffer+="${char}" ;;
            $'\x1b') # ESCAPE (Skip)
                # Check if it's just ESC or an arrow sequence
                read -rsn2 -t 0.01 junk </dev/tty 2>/dev/null
                buffer="${current_val}"
                break ;;
        esac
    done

    # 6. RESTORE TERMINAL & LOCK IN CYAN
    stty "$old_stty" 2>/dev/null
    hide_cursor # Hide cursor
    draw_sealed_row "$row" "${label_str}${CYAN}${buffer}${NC}"

    echo "${buffer}"
}

configure_user_profile() {
    load_current_profile_values
    # Clear ANSI position and width caches to avoid stale mappings from previous UI
    declare -gA ANSI_CACHE=()
    declare -gA WIDTH_CACHE=()

    # Selection Menus (Units/Gender)
    # shellcheck disable=SC2034
    local unit_opts=("Metric (kg/km)" "Imperial (lbs/mi)")
    show_scrollable_menu "SELECT UNIT SYSTEM" unit_opts "$( [ "$PREV_MILES" == "true" ] && echo 1 || echo 0)"
    [[ $? -eq 255 ]] && return 0
    
    # shellcheck disable=SC2034
    local sex_opts=("Male" "Female")
    show_scrollable_menu "SELECT GENDER" sex_opts "$( [ "$PREV_SEX" == "Female" ] && echo 1 || echo 0)"
    [[ $? -eq 255 ]] && return 0

    # Draw Form Skeleton
    draw_bottom_panel_header "USER PROFILE"
    clear_info_area
    
    local w_unit="kg"; [[ "$PREV_MILES" == "true" ]] && w_unit="lbs"
    
    # VERTICAL LAYOUT (Row 13 = Spacer)
    local row_instr=$((LOG_TOP + 1))  # Row 14
    local row_w=$((LOG_TOP + 3))      # Row 16
    local row_a=$((LOG_TOP + 5))      # Row 18

    draw_sealed_row $((LOG_TOP)) ""
    draw_sealed_row "$row_instr" "  Enter measurements (or press ENTER to keep current):"
    draw_sealed_row "$row_w"     "  Weight (${w_unit}): ${GRAY}[${PREV_WEIGHT}]${NC}"
    draw_sealed_row "$row_a"     "  Age (years): ${GRAY}[${PREV_AGE}]${NC}"
    
    # Move instructions to the border
    draw_bottom_border "Numbers only | ENTER to confirm"

    # Step 3: Interactive Inputs
    local new_w
    new_w=$(prompt_numeric_input "Weight" "$w_unit" "$PREV_WEIGHT" "$row_w")
    update_config_key "weight" "$new_w"
    
    local new_a
    new_a=$(prompt_numeric_input "Age" "years" "$PREV_AGE" "$row_a")
    update_config_key "age" "$new_a"
    
    draw_bottom_panel_header "PROFILE SAVED"
    sleep 1
}

select_equipment_flow() {
    # --- THE FIX: CONSISTENT ERROR MESSAGE FOR MISSING DEVICES.INI ---
    if [ ! -f "$DEVICES_INI" ]; then
        draw_error_screen "MISSING DATABASE" "Error: Equipment database (devices.ini) not found.\nPlease ensure the file exists in the script directory." 1
        return 1
    fi

    # --- CONTINUING NORMAL FLOW ---
    local types=()
    mapfile -t types < <(grep '^\[.*\]$' "$DEVICES_INI" | tr -d '[]')
    local state=0
    local selected_type=""
    
    # --- AUTO-DETECT CURRENT TYPE ---
    local type_def_idx=0
    if [ -f "$CONFIG_FILE" ]; then
        if grep -q "virtual_device_force_treadmill=true" "$CONFIG_FILE"; then selected_type="Treadmill"
        elif grep -q "virtual_device_rower=true" "$CONFIG_FILE"; then selected_type="Rower"
        elif grep -q "virtual_device_elliptical=true" "$CONFIG_FILE"; then selected_type="Elliptical"
        elif grep -q "virtual_device_force_bike=true" "$CONFIG_FILE"; then selected_type="Bike"
        fi
        
        for i in "${!types[@]}"; do
            [[ "${types[$i]}" == "$selected_type" ]] && type_def_idx=$i
        done
    fi

    while true; do
        if [ "$state" -eq 0 ]; then
             show_scrollable_menu "SELECT DEVICE TYPE" types "$type_def_idx" "Back"
             local t_idx=$?
             if [ "$t_idx" -eq 255 ]; then return 1; fi 
             selected_type="${types[$t_idx]}"
             state=1
        fi
        
        if [ "$state" -eq 1 ]; then
            local models=()
            local keys=()
            while IFS= read -r line; do
                if [[ "$line" =~ = ]]; then
                    models+=("$(echo "${line%%=*}" | xargs)")
                    keys+=("$(echo "${line#*=}" | xargs)")
                fi
            done < <(awk -v section="[$selected_type]" '$0==section { flag=1; next } /^\[/ { flag=0 } flag && $0!="" && $0!~/^;/ { print $0 }' "$DEVICES_INI")
            
            local mod_def_idx=0
            if [ -f "$CONFIG_FILE" ]; then
                for i in "${!keys[@]}"; do
                    if grep -q "^${keys[$i]}=true" "$CONFIG_FILE"; then
                        mod_def_idx=$i
                        break
                    fi
                done
            fi

            show_scrollable_menu "SELECT $selected_type MODEL" models "$mod_def_idx" "Back"
            local m_idx=$?
            if [ "$m_idx" -eq 255 ]; then state=0; continue; fi
            
            local selected_key="${keys[$m_idx]}"
            
            # --- SAVE CONFIGURATION ---
            local all_possible_keys
            all_possible_keys=$(grep '=' "$DEVICES_INI" | cut -d'=' -f2 | xargs)
            for k in $all_possible_keys; do update_config_key "$k" "false"; done
            
            update_config_key "virtual_device_force_treadmill" "false"
            update_config_key "virtual_device_rower" "false"
            update_config_key "virtual_device_force_bike" "false"
            update_config_key "virtual_device_elliptical" "false"

            case "$selected_type" in
                "Treadmill")  update_config_key "virtual_device_force_treadmill" "true" ;;
                "Rower")      update_config_key "virtual_device_rower" "true" ;;
                "Elliptical") update_config_key "virtual_device_elliptical" "true" ;;
                "Bike")       update_config_key "virtual_device_force_bike" "true" ;;
            esac
            update_config_key "$selected_key" "true"
            
            draw_bottom_panel_header "EQUIPMENT SAVED"
            clear_info_area
            draw_sealed_row $((LOG_TOP + 3)) "   ${GREEN}Equipment updated successfully!${NC}"
            draw_bottom_border ""
            sleep 1.5
            
            return 0
        fi
    done
}

# ============================================================================
# PROGRESS BAR WITH LIVE LOG STREAMING
# ============================================================================

run_with_progress() {
    local label="$1"
    local command_text="$2"
    local log_file="/tmp/qz_setup.log"
    local script_file="/tmp/qz_install_step.sh"
    
    echo "#!/bin/bash" > "$script_file"
    echo "export DEBIAN_FRONTEND=noninteractive" >> "$script_file"
    echo "set -e" >> "$script_file"
    echo "$command_text" >> "$script_file"
    chmod +x "$script_file"
    : > "$log_file"

    draw_bottom_panel_header "INSTALLING..."
    clear_info_area
    
    # --- MOVED UP BY ONE ROW ---
    # Row 2: Task Label
    draw_sealed_row $((LOG_TOP + 1)) "   ${WHITE}${label}${NC}"
    
    # Row 4: Subtext
    local subtext="Please wait..."
    [[ "$label" == *"pyenv"* ]] && subtext="Please wait... (This may take 10-15 minutes)"
    draw_sealed_row $((LOG_TOP + 3)) "   ${GRAY}${subtext}${NC}"

    bash "$script_file" > "$log_file" 2>&1 &
    local pid=$!
    
    local bar_width=40 pulse_width=6 pos=0 dir=1
    enter_ui_mode
    
    while kill -0 "$pid" 2>/dev/null; do
        local bar_str=""
        for ((i=0; i<bar_width; i++)); do
            if [ "$i" -ge "$pos" ] && [ "$i" -lt "$((pos + pulse_width))" ]; then
                bar_str="${bar_str}━"
            else
                bar_str="${bar_str}─"
            fi
        done
        
        # Row 6: Progress Bar
        draw_sealed_row $((LOG_TOP + 5)) "                  [${CYAN}${bar_str}${NC}]"
        
        if [ "$dir" -eq 1 ]; then
            ((pos++)); [ "$((pos + pulse_width))" -ge "$bar_width" ] && dir=-1
        else
            ((pos--)); [ "$pos" -le 0 ] && dir=1
        fi
        
    done
    
    wait "$pid"
    local exit_code=$?

    if [ $exit_code -ne 0 ]; then
        # Compose a compact log excerpt to include in the error screen
        local log_excerpt
        log_excerpt=$(grep -a "." "$log_file" 2>/dev/null | tail -n 5 | sed 's/[[:cntrl:]]\+//g' | sed ':a;N;$!ba;s/\n/\n/g')
        draw_error_screen "TASK FAILED" "Error during: ${label}\nTechnical details from log:\n${log_excerpt}" 1
    fi
    
    return $exit_code
}

# ============================================================================
# CHECK FUNCTIONS
# ============================================================================

# Global flag to track Python install type
IS_PYENV_INSTALL=false

check_python311() {
    update_status "python311" "working"
    IS_PYENV_INSTALL=false
    
    # 1. Check pyenv path first (Priority)
    local pyenv_bin="$TARGET_HOME/.pyenv/versions/3.11.9/bin/python"
    if [ -f "$pyenv_bin" ]; then
        IS_PYENV_INSTALL=true
        update_status "python311" "pass"
        return 0
    fi

    # 2. Check standard system path
    if command -v python3.11 >/dev/null 2>&1; then
        update_status "python311" "pass"
        return 0
    fi

    update_status "python311" "fail"
    return 1
}

check_venv() {
    update_status "venv" "working"
    
    if [ -d "$TARGET_HOME/ant_venv" ]; then
        update_status "venv" "pass"
        return 0
    else
        update_status "venv" "fail"
        return 1
    fi
}

check_python_packages() {
    local venv_py="$TARGET_HOME/ant_venv/bin/python3"
    update_status "pkg_pips" "working"
    
    if [ ! -f "$venv_py" ]; then
        update_status "pkg_pips" "fail"
        return 1
    fi

    # Check all 4 required packages
    local missing=0
    for pkg in openant usb pybind11 bleak; do
        if ! sudo -u "$TARGET_USER" "$venv_py" -c "import $pkg" 2>/dev/null; then
            ((missing++))
        fi
    done

    if [ "$missing" -eq 0 ]; then
        update_status "pkg_pips" "pass"
        return 0
    else
        update_status "pkg_pips" "fail"
        return 1
    fi
}

check_qt5_libs() {
    update_status "qt5_libs" "working"
    
    local required=("Qt5Core" "Qt5Qml" "Qt5Quick" "Qt5Bluetooth" "Qt5Charts" "Qt5Multimedia" "Qt5Widgets" "libusb-1.0")
    local missing=0
    for mod in "${required[@]}"; do
        if [ "$mod" = "libusb-1.0" ]; then
            if ! ldconfig -p 2>/dev/null | grep -q "libusb-1.0"; then
                if ! find /usr/lib /usr/lib64 -maxdepth 3 -type f -name 'libusb-1.0*' -print -quit 2>/dev/null | grep -q .; then ((missing++)); fi
            fi
        else
            if ! ldconfig -p 2>/dev/null | grep -qE "(lib${mod}\.so|${mod})"; then
                if ! find /usr/lib /usr/lib64 -maxdepth 3 -type f -name "lib${mod}.so*" -print -quit 2>/dev/null | grep -q .; then ((missing++)); fi
            fi
        fi
    done
    if [ $missing -eq 0 ]; then update_status "qt5_libs" "pass"; return 0; else update_status "qt5_libs" "fail"; return 1; fi
}

check_qml_modules() {
    update_status "qml_modules" "working"
    
    local missing=0
    local qmls=("QtLocation" "QtQuick.2" "QtQuick/Controls.2")
    for qml in "${qmls[@]}"; do
        local found=0
        for path in /usr/lib/*/qt5/qml /usr/lib/qt5/qml; do
            [ -d "$path/$qml" ] && { found=1; break; }
        done
        [ $found -eq 0 ] && ((missing++))
    done
    if [ $missing -eq 0 ]; then update_status "qml_modules" "pass"; return 0; else update_status "qml_modules" "fail"; return 1; fi
}

check_bluetooth() {
    update_status "bluetooth" "working"
    
    local found=0
    if command -v systemctl >/dev/null 2>&1; then
        for unit in bluetooth.service bluez.service; do
            if systemctl is-active --quiet "$unit" 2>/dev/null; then found=1; break; fi
        done
    fi
    if [ $found -eq 0 ] && command -v bluetoothctl >/dev/null 2>&1; then
        bluetoothctl show 2>/dev/null | grep -q "Controller" && found=1
    fi
    if [ $found -eq 1 ]; then update_status "bluetooth" "pass"; return 0; else update_status "bluetooth" "fail"; return 1; fi
}

check_plugdev() {
    update_status "plugdev" "working"
    
    if groups "$TARGET_USER" 2>/dev/null | grep -q plugdev; then update_status "plugdev" "pass"; return 0; else update_status "plugdev" "fail"; return 1; fi
}

check_udev_rules() {
    update_status "udev_rules" "working"
    
    if [ -f /etc/udev/rules.d/99-garmin-ant.rules ] || [ -f /etc/udev/rules.d/51-garmin-ant.rules ] || [ -f /etc/udev/rules.d/99-ant-usb.rules ]; then
        update_status "udev_rules" "pass"; return 0;
    else
        update_status "udev_rules" "fail"; return 1;
    fi
}

check_lsusb() {
    update_status "lsusb" "working"
    
    if command -v lsusb >/dev/null 2>&1; then update_status "lsusb" "pass"; return 0; else update_status "lsusb" "fail"; return 1; fi
}

check_ant_dongle() {
    update_status "ant_dongle" "working"
    
    if ! command -v lsusb >/dev/null 2>&1; then update_status "ant_dongle" "warn"; return 1; fi
    if lsusb 2>/dev/null | grep -qE '0fcf:1009|0fcf:1008|0fcf:100c|0fcf:88a4'; then
        update_status "ant_dongle" "pass"; return 0;
    else
        update_status "ant_dongle" "warn"; return 1;
    fi
}

check_config_file() {
    local status="fail"
    [[ -f "$CONFIG_FILE" ]] && status="pass"
    STATUS_MAP["config_file"]="$status"
    draw_header_config_line
    render_status_grid 5
    [[ "$status" == "pass" ]] && return 0 || return 1
}

check_qz_service() {
    local status="warn"
    if command -v systemctl >/dev/null 2>&1 && systemctl is-enabled --quiet qz.service 2>/dev/null; then
        status="pass"
    elif [[ "$HAS_GUI" == true ]]; then
        status="pending"
    fi
    STATUS_MAP["qz_service"]="$status"
    draw_header_service_line
    render_status_grid 5
    [[ "$status" != "fail" ]] && return 0 || return 1
}

run_all_checks() {
    check_python311
    check_venv
    check_python_packages
    check_qt5_libs
    check_qml_modules
    check_bluetooth
    check_lsusb
    check_plugdev
    check_udev_rules 
    check_config_file
    check_qz_service
    check_ant_dongle
}

# Initialize these globally to prevent "unbound variable" errors in the trap
#scan_pid=""
#bt_log=""
#bt_pipe=""

# Global process management variables (placeholders so traps with set -u don't fail)
# These are assigned inside perform_bluetooth_scan() when the scan runs.
BT_SCAN_PID=""

# ============================================================================
# ROBUST CLEANUP HANDLER (set -u safe & prevents freezes)
# ============================================================================
cleanup_bt_engine() {
    # Prefer the provider stop logic for consistent cleanup
    stop_bt_provider 2>/dev/null || true
    [[ -t 0 ]] && stty echo 2>/dev/null
    printf "\033[?25h" >&${UI_FD:-2}
    printf "\033[23;1H\n" >&${UI_FD:-2}
}
 
# Start the Bluetooth scanner engine: create fifo/log, start the background
# tail|script pipeline, set BT_SCAN_PID and enable scanning on controller.
start_bt_engine() {
    # Provider-only start: prefer Python provider when enabled
    # Start the Python provider by default
    start_bt_provider
    BT_SCAN_PID=${BT_PROVIDER_SUPERVISOR_PID:-}
    if [ -n "$BT_SCAN_PID" ]; then
        return 0
    fi
    return 1
}

# Stop the Bluetooth scanner engine: kill background processes and remove
# temp files (does NOT modify terminal settings).
stop_bt_engine() {
    bt_debug "stop_bt_engine: stopping pid=${BT_SCAN_PID:-}"
    # Prefer stopping provider if active
    stop_bt_provider || true
    BT_SCAN_PID=""
    bluetoothctl scan off >/dev/null 2>&1 || true
    bt_debug "stop_bt_engine: stopped"
}

# Trap Ctrl+C and standard terminations
trap 'cleanup_bt_engine; exit 130' SIGINT SIGTERM

# Helper: Strictly validates if a string is a real broadcast name
is_real_name() {
    local n="${1:-}"
    [[ -z "$n" ]] && return 1
    # Fast whitelist
    [[ "$n" == "I_TL" || "$n" == "N025E" ]] && return 0
    # Reject internal markers and common noise strings using pure bash
    [[ "$n" =~ ^~ ]] && return 1
    [[ "$n" == *RSSI* ]] && return 1
    [[ "$n" == *Manufacturer* ]] && return 1
    [[ "$n" == *ServiceData* ]] && return 1
    [[ "$n" == *TxPower* ]] && return 1
    [[ "$n" == *Alias* ]] && return 1
    [[ "$n" == *Attributes* ]] && return 1
    [[ "$n" == *Pairable* ]] && return 1
    [[ "$n" == *Connected* ]] && return 1
    [[ "$n" == *Searching* ]] && return 1
    [[ "$n" == *Privacy* ]] && return 1
    [[ "$n" == *Device* ]] && return 1
    # MAC address pattern check (no external grep)
    if [[ "$n" =~ ^([0-9A-Fa-f]{2}[:\-]){5}[0-9A-Fa-f]{2}$ ]]; then
        return 1
    fi
    # Separator count heuristic (reject MAC-like strings)
    local sep_count="${n//[^:-]}"
    [[ ${#sep_count} -ge 4 ]] && return 1
    return 0
}

 

# ============================================================================
# FINAL STABLE BLUETOOTH RADAR (PYTHON-POWERED + REAL-TIME FIFO)
# ============================================================================

perform_bluetooth_scan() {
    # 1. PREP ENVIRONMENT
    local script_dir
    script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
    local py_script="${script_dir}/bt_provider.py"
    local venv_py="${TARGET_HOME}/ant_venv/bin/python3"
    [[ ! -f "$venv_py" ]] && venv_py=$(command -v python3)
    
    # Use RAM-backed FIFO for zero latency
    local bt_fifo="/dev/shm/qz_bt_fifo_$$"
    if [[ ! -d /dev/shm ]]; then bt_fifo="/tmp/qz_bt_fifo_$$"; fi

    # Ensure Bluetooth hardware is awake
    sudo rfkill unblock bluetooth >/dev/null 2>&1
    echo "power on" | bluetoothctl >/dev/null 2>&1

    # OUTER LOOP: Handles "Refresh" without crashing or recursion
    while true; do
        local devices=() macs=() rssis=()
        local loop_count=0
        local py_status="STARTING"
        local last_raw="NONE"
        local spin_chars=('⠋' '⠙' '⠹' '⠸' '⠼' '⠴' '⠦' '⠧' '⠇' '⠏')

        # Setup FIFO Pipe
        rm -f "$bt_fifo" && mkfifo "$bt_fifo"
        
        # Start Python Provider (Unbuffered)
        # We use 'exec' to ensure the PID is the Python process itself
        sudo "$venv_py" -u "$py_script" > "$bt_fifo" 2>&1 &
        local py_pid=$!
        BT_SCAN_PID=$py_pid
        
        # Open FIFO for reading on FD 4 to keep it persistent
        exec 4<"$bt_fifo"

        stop_bt_engine() {
            if [[ -n "${BT_SCAN_PID:-}" ]]; then
                # Shielded Kill: Verify process name before sending signal
                if ps -p "$BT_SCAN_PID" -o cmd= 2>/dev/null | grep -q "bt_provider"; then
                    sudo kill -9 "$BT_SCAN_PID" 2>/dev/null || true
                fi
            fi
            exec 4<&-
            rm -f "$bt_fifo"
            BT_SCAN_PID=""
        }

        # --- PHASE 1: THE RADAR (Rows 12-21) ---
        enter_ui_mode
        while true; do
            ((loop_count++))
            
            # Verify Python process health
            if ! ps -p "$py_pid" >/dev/null 2>&1; then py_status="STOPPED"; fi

            # Non-blocking read from FIFO
            while read -u 4 -t 0.02 -r raw_data; do
                # Handle STATUS messages robustly
                if [[ "$raw_data" == STATUS\|* ]]; then
                    IFS='|' read -r _ status_code status_msg <<< "${raw_data%%$'\r'}"
                    [[ -n "$status_msg" ]] && py_status="$status_msg"
                    continue
                fi
                [[ "$raw_data" == "HEARTBEAT|"* ]] && { py_status="ACTIVE"; continue; }

                # Split data: MAC|RSSI|LABEL
                IFS='|' read -r m r l <<< "$(echo "$raw_data" | tr -d '\r')"
                # Defensive filters: ignore obvious non-device lines
                [[ -z "$m" || "$m" == "ERROR" || "$m" == "HEARTBEAT" || "$m" == "STATUS" ]] && continue
                # Trim label and skip status-like names (CONNECTING, STATUS)
                local ltrim
                ltrim=$(printf '%s' "$l" | xargs)
                if [[ -z "$ltrim" || "${ltrim^^}" == "CONNECTING" || "${ltrim^^}" == "STATUS" ]]; then
                    continue
                fi
                # Ignore suspicious RSSI values (non-negative dBm) which likely indicate placeholders
                if [[ "$r" =~ ^-?[0-9]+$ ]]; then
                    if (( r >= 0 )); then
                        continue
                    fi
                fi
                
                # Update existing device or add new discovery
                local idx=-1
                for i in "${!macs[@]}"; do [[ "${macs[$i]}" == "$m" ]] && idx=$i && break; done
                if [[ $idx -ge 0 ]]; then
                    rssis[$idx]=$r; devices[$idx]=$l
                else
                    macs+=("$m"); rssis+=("$r"); devices+=("$l")
                fi
            done

            # Sort by Signal Strength (Closest devices at top)
            for ((i=0; i<${#macs[@]}-1; i++)); do
                for ((j=i+1; j<${#macs[@]}; j++)); do
                    if [[ ${rssis[i]} -lt ${rssis[j]} ]]; then
                        local tr=${rssis[i]}; rssis[i]=${rssis[j]}; rssis[j]=$tr
                        local tm=${macs[i]}; macs[i]=${macs[j]}; macs[j]=$tm
                        local td=${devices[i]}; devices[i]=${devices[j]}; devices[j]=$td
                    fi
                done
            done

            # UI Rendering
            draw_bottom_panel_header "RADAR: ${#devices[@]} NAMED DEVICES"
            
            # Row 12: Consistent Spacer
            draw_sealed_row 12 ""
            
            # Rows 13-20: Device slots (8 rows)
            for ((i=0; i<8; i++)); do
                local row=$((13 + i))
                if [ $i -lt ${#devices[@]} ]; then
                    local s=${rssis[$i]}
                    local bar="${RED}[#   ]${NC}"
                    if (( s >= -60 )); then bar="${GREEN}[####]${NC}"
                    elif (( s >= -75 )); then bar="${YELLOW}[### ]${NC}"
                    elif (( s >= -85 )); then bar="${ORANGE}[##  ]${NC}"; fi
                    
                    local name="${devices[$i]}"
                    local color="$CYAN"
                    # Highlight specific targets
                    [[ "$name" =~ (I_TL|N025E|Forerunner) ]] && color="$BOLD_WHITE"
                    
                    local n_fmt=$(printf '%-30.30s' "$name")
                    draw_sealed_row "$row" " $bar $color$n_fmt$NC ${macs[$i]} ($s dBm)"
                else draw_sealed_row "$row" ""; fi
            done

            # Row 21: High-Density Diagnostics
            draw_sealed_row 21 "${GRAY}Engine: $py_status | PID: $py_pid | Active Discovery${NC}"
            
            # Border Footer
            draw_bottom_border "Scanning... ${spin_chars[$((loop_count % 10))]} | Any key to stop"

            if read -rsn1 -t 0.1 key; then
                stop_bt_engine
                break # Transition to Selection Phase
            fi
        done

        # --- PHASE 2: THE SELECTION ---
        local menu_labels=()
        for i in "${!macs[@]}"; do
            menu_labels+=("$(printf '%-30s [%s]' "${devices[$i]}" "${macs[$i]}")")
        done
        menu_labels+=("Scan" "Back")
        
        # Call directly (returns selection via exit code $?)
        show_scrollable_menu "SELECT DEVICE" menu_labels 0
        local sanitized=$? 
        
        local num_devs=${#macs[@]}
        local refresh_idx=$num_devs
        local skip_idx=$((num_devs + 1))

        if [[ "$sanitized" -eq 255 || "$sanitized" -eq "$refresh_idx" ]]; then
            # User chose REFRESH: Loop restarts, clearing data
            exit_ui_mode
            clear_info_area
            continue 
        elif [[ "$sanitized" -eq "$skip_idx" ]]; then
            exit_ui_mode; return 1
        elif [[ "$sanitized" -lt "$num_devs" ]]; then
            # SUCCESS: Save Selection
            local sel_mac="${macs[$sanitized]}"
            local sel_name="${devices[$sanitized]}"
            update_config_key "bluetooth_lastdevice_address" "$sel_mac"
            update_config_key "bluetooth_lastdevice_name" "$sel_name"
            update_config_key "filter_device" "$sel_name"
            update_config_key "bluetooth_address" "$sel_mac"
            
            draw_bottom_panel_header "DEVICE LINKED"
            clear_info_area
            draw_sealed_row $((LOG_TOP + 2)) "   ${GREEN}Device linked successfully!${NC}"
            draw_sealed_row $((LOG_TOP + 4)) "   Name: ${WHITE}$sel_name${NC}"
            draw_sealed_row $((LOG_TOP + 5)) "   Addr: ${GRAY}$sel_mac${NC}"
            draw_bottom_border ""; sleep 2
            exit_ui_mode; return 0
        fi
    done
}

install_python311() {
    # 1. Install Build Dependencies
    local deps="git curl build-essential libssl-dev zlib1g-dev libbz2-dev libreadline-dev libsqlite3-dev wget llvm libncurses-dev xz-utils tk-dev libffi-dev liblzma-dev"
    run_with_progress "Installing Build Dependencies" "apt-get update && apt-get install -y $deps"

    local p_root="${TARGET_HOME:-}/.pyenv"
    local p_bin="$p_root/bin/pyenv"

    # 2. Handle pyenv installation
    if [ ! -d "$p_root" ]; then
        run_with_progress "Installing pyenv tool" "sudo -u \"$TARGET_USER\" bash -c 'curl https://pyenv.run | bash'"
        
        local bashrc="${TARGET_HOME:-}/.bashrc"
        sudo -u "$TARGET_USER" bash -c "echo 'export PYENV_ROOT=\"\$HOME/.pyenv\"' >> $bashrc"
        sudo -u "$TARGET_USER" bash -c "echo 'command -v pyenv >/dev/null || export PATH=\"\$PYENV_ROOT/bin:\$PATH\"' >> $bashrc"
        sudo -u "$TARGET_USER" bash -c "echo 'eval \"\$(pyenv init -)\"' >> $bashrc"
    fi

    # 3. Compile Python 3.11.9
    local label="Python 3.11 not available via system. Using pyenv"
    
    # THE FIX: We use a clean export block. No more escaped backslashes for eval.
    # We call the binary directly using its absolute path.
    local install_cmd="sudo -u \"$TARGET_USER\" bash -c \"
        export PYENV_ROOT='$p_root'
        export PATH='\$PYENV_ROOT/bin:\$PATH'
        $p_bin install 3.11.9 && $p_bin global 3.11.9
    \""
    
    if run_with_progress "$label" "$install_cmd"; then
        IS_PYENV_INSTALL=true # Ensure the uninstall logic knows this can be removed
        return 0
    else
        return 1
    fi
}

install_venv() {
    local venv_path="$TARGET_HOME/ant_venv"
    
    # Define the pyenv binary path
    local pyenv_bin="$TARGET_HOME/.pyenv/versions/3.11.9/bin/python"
    local py_cmd=""

    # Find which 3.11 to use
    if [ -f "$pyenv_bin" ]; then
        py_cmd="$pyenv_bin"
    elif command -v python3.11 >/dev/null 2>&1; then
        py_cmd="python3.11"
    else
        # If both missing, trigger pyenv install
        if install_python311; then
            py_cmd="$pyenv_bin"
        else
            return 1
        fi
    fi

    # Create venv as the target user
    local cmd="sudo -u \"$TARGET_USER\" $py_cmd -m venv \"$venv_path\""
    if run_with_progress "Creating Virtual Environment" "$cmd"; then
        chown -R "$TARGET_USER:" "$venv_path"
        return 0
    fi
    return 1
}

install_python_packages() {
    local pip_path="$TARGET_HOME/ant_venv/bin/pip"
    # Added bleak to the installation list
    local cmd="sudo -u \"$TARGET_USER\" $pip_path install --upgrade pip && sudo -u \"$TARGET_USER\" $pip_path install openant pyusb pybind11 bleak"
    run_with_progress "Installing Python PIPs (incl. Bleak)" "$cmd"
}

install_qt5_libs() {
    # README: Specific list of libqt5 and qml modules
    local libs=(
        libqt5core5a libqt5qml5 libqt5quick5 libqt5quickwidgets5 libqt5concurrent5
        libqt5bluetooth5 libqt5charts5 libqt5multimedia5 libqt5multimediawidgets5
        libqt5multimedia5-plugins libqt5networkauth5 libqt5positioning5 libqt5sql5
        libqt5texttospeech5 libqt5websockets5 libqt5widgets5 libqt5xml5 libqt5location5
        qtlocation5-dev qml-module-qtlocation qml-module-qtpositioning qml-module-qtquick2
        qml-module-qtquick-controls qml-module-qtquick-controls2 qml-module-qtquick-dialogs
        qml-module-qtquick-layouts qml-module-qtquick-window2 qml-module-qtmultimedia
        libusb-1.0-0 bluez usbutils python3-pip
    )
    local cmd="apt-get update && apt-get install -y ${libs[*]}"
    run_with_progress "Installing System Dependencies" "$cmd"
}

install_bluetooth() {
    # README: sudo systemctl start bluetooth, sudo systemctl enable bluetooth
    local cmd="systemctl start bluetooth && systemctl enable bluetooth"
    run_with_progress "Enabling Bluetooth Service" "$cmd"
}

install_plugdev() {
    # README: sudo usermod -aG plugdev $USER
    local cmd="groupadd -f plugdev && usermod -aG plugdev $TARGET_USER"
    run_with_progress "Updating USB Permissions (plugdev)" "$cmd"
}

install_udev_rules() {
    # README: Create udev rule with specific Vendor/Product IDs
    local rules_file="/etc/udev/rules.d/99-ant-usb.rules"
    local cmd="cat > $rules_file <<'EOF'
SUBSYSTEM==\"usb\", ATTRS{idVendor}==\"0fcf\", ATTRS{idProduct}==\"100?\", MODE=\"0666\", GROUP=\"plugdev\"
SUBSYSTEM==\"usb\", ATTRS{idVendor}==\"0fcf\", ATTRS{idProduct}==\"88a4\", MODE=\"0666\", GROUP=\"plugdev\"
SUBSYSTEM==\"usb\", ATTRS{idVendor}==\"11fd\", ATTRS{idProduct}==\"0001\", MODE=\"0666\", GROUP=\"plugdev\"
EOF
udevadm control --reload-rules && udevadm trigger"
    run_with_progress "Installing ANT+ USB Rules" "$cmd"
}

install_lsusb() {
    # README: usbutils
    local cmd="apt-get update && apt-get install -y usbutils"
    run_with_progress "Installing USB Utilities" "$cmd"
}

create_systemd_service() {
    [ "$SETUP_MODE" != "headless" ] && return 0
    
    local script_dir
    script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    local install_dir
    install_dir="$(cd "$script_dir/../../.." && pwd)"
    
    local arch
    arch=$(uname -m)
    local bin_dir
    if [ "$arch" = "aarch64" ]; then
        bin_dir="$install_dir/qdomyos-zwift-arm64-ant"
    else
        bin_dir="$install_dir/qdomyos-zwift-x86-64-ant"
    fi
    
    local service_file="/etc/systemd/system/qz.service"
    
    local cmd="cat > \"$service_file\" <<EOF
[Unit]
Description=qdomyos-zwift service
After=multi-user.target

[Service]
User=root
Group=plugdev
Environment=\"QZ_USER=${TARGET_USER}\"
WorkingDirectory=${bin_dir}
ExecStart=${bin_dir}/qdomyos-zwift -no-gui -log -ant-footpod
KillSignal=SIGINT

[Install]
WantedBy=multi-user.target
EOF
    systemctl daemon-reload"
    
    run_with_progress "Creating systemd service..." "$cmd"
    ACTIVE_SERVICE_FILE="$service_file"
}

# ============================================================================
# MENUS & PROMPTS
# ============================================================================

prompt_yes_no() {
    local start_offset=${1:-4}
    local start_row=$((LOG_TOP + start_offset))
    local selected=0
    local options=("Yes" "No" "Cancel")
    local num_options=${#options[@]}
    
    enter_ui_mode
    while true; do
        for i in "${!options[@]}"; do
            local row=$((start_row + i))
            if [ "$i" -eq "$selected" ]; then
                draw_sealed_row "$row" "   ${CYAN}► ${BOLD_CYAN}${options[$i]}${NC}"
            else
                draw_sealed_row "$row" "     ${GRAY}${options[$i]}${NC}"
            fi
        done
        
        # Border call
        draw_bottom_border "Arrows: Up/Down | Enter: Select"

        local key=""
        IFS= read -rsn1 key
        if [[ $key == $'\x1b' ]]; then
            read -rsn2 -t 0.01 k2
            case "${k2:-}" in
                '[A') ((selected--)) ;; 
                '[B') ((selected++)) ;;
            esac
        elif [[ $key == "" ]]; then
            return "$selected"
        fi
        [ $selected -lt 0 ] && selected=$((num_options - 1))
        [ "$selected" -ge "$num_options" ] && selected=0
    done
}

prompt_input_yes() {
    local prompt="Type YES to confirm: "
    # Restore terminal to input mode (enable echo and show cursor)
    exit_ui_mode
    show_cursor
    local prompt_row=$((LOG_BOTTOM))
    
    print_at $((prompt_row - 2)) "${RED}╔════════════════════════════════════════════════════════════════════════════╗${NC}"
    print_at $((prompt_row - 1)) "${RED}║${NC} ${WHITE}WARNING: This may break your desktop if you are not on a true server.${NC}      ${RED}║${NC}"
    print_at $((prompt_row))     "${RED}║${NC} ${YELLOW}${prompt}${NC}                                                      ${RED}║${NC}"
    print_at $((prompt_row + 1)) "${RED}╚════════════════════════════════════════════════════════════════════════════╝${NC}"
    
    move_cursor "$prompt_row" $((2 + 21))
    
    local input=""
    while true; do
        read -rsn1 key 2>/dev/tty
        if [[ "$key" == "" ]]; then break; fi
        if [[ "$key" == $'\x7f' ]]; then
            if [ ${#input} -gt 0 ]; then input="${input::-1}"; printf "\b \b"; fi
        elif [[ "$key" =~ [a-zA-Z] ]]; then
            if [ ${#input} -lt 3 ]; then input+="$key"; printf "%s" "$key"; fi
        fi
    done
    
    enter_ui_mode
    for ((r=prompt_row-2; r<=prompt_row+1; r++)); do
        print_at "$r" "${BLUE}║${NC}$(pad_display "" $INFO_WIDTH)${BLUE}║${NC}"
    done
    
    if [ "$input" = "YES" ]; then return 0; else return 1; fi
}

prompt_setup_mode() {
    enter_ui_mode
    local options=("GUI Mode (Desktop)" "Headless Mode (Server)" "Back")
    local selected=0
    [ "$HAS_GUI" = false ] && selected=1
    local num_options=${#options[@]}
    
    while true; do
        draw_bottom_panel_header "SELECT INSTALLATION TARGET"
        clear_info_area
        
        for i in "${!options[@]}"; do
            local row=$((LOG_TOP + 1 + i))
            if [ "$i" -eq "$selected" ]; then
                draw_sealed_row "$row" "   ${CYAN}► ${BOLD_CYAN}${options[$i]}${NC}"
            else
                draw_sealed_row "$row" "     ${GRAY}${options[$i]}${NC}"
            fi
        done
        
        draw_bottom_border "Arrows: Up/Down | Enter: Select"
        
        local key=""
        IFS= read -rsn1 key
        if [[ $key == $'\x1b' ]]; then
            read -rsn2 -t 0.01 k2
            case "${k2:-}" in
                '[A') ((selected--)) ;; 
                '[B') ((selected++)) ;;
            esac
        elif [[ $key == "" ]]; then
            if [ $selected -eq 2 ]; then return 1; fi
            [ $selected -eq 0 ] && SETUP_MODE="gui" || SETUP_MODE="headless"
            return 0
        fi
        [ $selected -lt 0 ] && selected=$((num_options - 1))
        [ "$selected" -ge "$num_options" ] && selected=0
    done
}

prompt_success_menu() {
    local warns=${1:-0}
    enter_ui_mode
    local options=("User Profile" "Equipment Selection" "Bluetooth Scan" "Uninstall" "Exit")
    local selected=0
    local num_options=${#options[@]}
    
    local title="SYSTEM READY"
    [[ "$warns" -eq 1 ]] && title="READY WITH 1 WARNING"
    [[ "$warns" -gt 1 ]] && title="READY WITH $warns WARNINGS"

    while true; do
        draw_bottom_panel_header "$title"
        clear_info_area
        draw_sealed_row 12 "" # Spacer
        
        for i in "${!options[@]}"; do
            local row=$((13 + i))
            if [[ $i -eq $selected ]]; then
                draw_sealed_row "$row" "   ${CYAN}► ${BOLD_CYAN}${options[$i]}${NC}"
            else
                draw_sealed_row "$row" "     ${GRAY}${options[$i]}${NC}"
            fi
        done
        
        draw_bottom_border "Arrows: Up/Down | Enter: Select"
        
        local key=""
        IFS= read -rsn1 key
        if [[ $key == $'\x1b' ]]; then
            read -rsn2 -t 0.01 k2
            [[ "${k2:-}" == "[A" ]] && ((selected--))
            [[ "${k2:-}" == "[B" ]] && ((selected++))
        elif [[ $key == "" ]]; then
            return "$selected"
        fi
        [[ $selected -lt 0 ]] && selected=$((num_options - 1))
        [[ $selected -ge $num_options ]] && selected=0
    done
}

prompt_action_menu() {
    local fails=$1
    enter_ui_mode
    local options=("Guided Fix" "Exit")
    local selected=0
    local num_options=${#options[@]}
    
    while true; do
        draw_bottom_panel_header "ISSUES DETECTED ($fails)"
        clear_info_area
        
        # Spacer at Row 12
        draw_sealed_row 12 ""
        
        # Options start at Row 13
        for i in "${!options[@]}"; do
            local row=$((13 + i))
            if [[ $i -eq $selected ]]; then
                draw_sealed_row "$row" "   ${CYAN}► ${BOLD_CYAN}${options[$i]}${NC}"
            else
                draw_sealed_row "$row" "     ${GRAY}${options[$i]}${NC}"
            fi
        done
        
        draw_bottom_border "Arrows: Up/Down | Enter: Select"
        
        local key=""
        IFS= read -rsn1 key
        if [[ $key == $'\x1b' ]]; then
            read -rsn2 -t 0.01 k2
            [[ "${k2:-}" == "[A" ]] && ((selected--))
            [[ "${k2:-}" == "[B" ]] && ((selected++))
        elif [[ $key == "" ]]; then
            return "$selected"
        fi
        [[ $selected -lt 0 ]] && selected=$((num_options - 1))
        [[ $selected -ge $num_options ]] && selected=0
    done
}


show_scrollable_menu() {
    local title="${1:-INFORMATION}"
    local -n items_ref=$2
    local selected="${3:-0}"
    local back_label="${4:-}"

    # 1. Prepare the full list
    local menu_list=("${items_ref[@]}")
    [[ -n "$back_label" ]] && menu_list+=("$back_label")
    local total_count=${#menu_list[@]}
    
    # 2. DYNAMIC SPACE CALCULATION
    local total_info_rows=$(( LOG_BOTTOM - LOG_TOP + 1 ))
    
    # SMART OFFSET LOGIC:
    # If count > 8, we use the top row (LOG_TOP).
    # If count <= 8, we start one row lower (LOG_TOP + 1) to keep the air gap.
    local render_start=$LOG_TOP
    local max_display=$total_info_rows

    if [[ $total_count -le 8 ]]; then
        render_start=$(( LOG_TOP + 1 ))
        max_display=$(( total_info_rows - 1 ))
    fi
    
    local display_count=$total_count
    [[ $display_count -gt $max_display ]] && display_count=$max_display

    # Ensure the cursor is hidden while the menu is active and restore it
    # on any function return or interrupt. The trap clears itself immediately
    # so it does not persist beyond this menu invocation.
    trap 'trap - RETURN SIGINT SIGTERM; move_cursor $((LOG_BOTTOM + 1)) 0; exit_ui_mode' RETURN SIGINT SIGTERM
    enter_ui_mode
    while true; do
        draw_bottom_panel_header "$title"
        
        # Clear the area
        clear_info_area

        # 5. Sliding Window Logic
        local start_idx=0
        if [[ $total_count -gt $display_count ]]; then
            start_idx=$(( selected - (display_count / 2) ))
            [[ $start_idx -lt 0 ]] && start_idx=0
            [[ $start_idx -gt $((total_count - display_count)) ]] && start_idx=$((total_count - display_count))
        fi

        # 6. Render the Menu Items
        for ((i=0; i<display_count; i++)); do
            local current_idx=$((start_idx + i))
            # Row is now relative to our smart render_start
            local row=$((render_start + i))
            local item_text="${menu_list[$current_idx]}"
            
            if [[ $current_idx -eq $selected ]]; then
                draw_sealed_row "$row" "   ${CYAN}► ${BOLD_CYAN}${item_text}${NC}"
            else
                draw_sealed_row "$row" "     ${GRAY}${item_text}${NC}"
            fi
        done

        # 7. Draw the Embedded Border Footer
        draw_bottom_border "Arrows: Up/Down | Enter: Select"

        # 8. Input Handling
        local key=""
        IFS= read -rsn1 key
        if [[ $key == $'\x1b' ]]; then
            read -rsn2 -t 0.01 k2
            case "${k2:-}" in
                '[A') ((selected--)) ;; 
                '[B') ((selected++)) ;;
            esac
        elif [[ $key == "" ]]; then
            # If the "Back" label was chosen, return 255 to signal the caller
            if [[ -n "$back_label" ]] && [[ $selected -eq $((total_count - 1)) ]]; then
                # Move cursor below dashboard and restore terminal before returning
                move_cursor $((LOG_BOTTOM + 1)) 0
                exit_ui_mode
                return 255
            fi
            move_cursor $((LOG_BOTTOM + 1)) 0
            exit_ui_mode
            return "$selected"
        fi

        # Wrap selection loop
        [[ $selected -lt 0 ]] && selected=$((total_count - 1))
        [[ $selected -ge $total_count ]] && selected=0
    done
}

run_guided_mode() {
    # 1. Select Mode (GUI vs Headless)
    # Returns 1 if user hits 'Back' in the selection menu
    if ! prompt_setup_mode; then return 1; fi 

    local action_taken=false

    # INTERNAL HELPER: Presentation and choice logic
    # Returns: 0 (Yes), 1 (No), 2 (Cancel/Exit Wizard)
    request_fix() {
        local title="$1"
        local problem="$2"
        local question="$3"
        
        draw_bottom_panel_header "GUIDED FIX: $title"
        
        # Clear interaction area
        clear_info_area
        
        # Row 1 & 2: Problem and Question
        draw_sealed_row $((LOG_TOP + 1)) "   ${WHITE}${problem}${NC}"
        draw_sealed_row $((LOG_TOP + 2)) "   ${question}"
        
        # Row 5, 6, 7: Yes/No/Cancel menu (using offset 4)
        prompt_yes_no 4
        return $? 
    }

    # --- SEQUENTIAL RESOLUTION STEPS ---

    # 1. Python 3.11 (via pyenv)
    if [ "${STATUS_MAP[python311]:-}" = "fail" ]; then
        request_fix "PYTHON" "Python 3.11 is missing or not reachable." "Install via pyenv (compilation)?"
        local res=$?
        if [ $res -eq 2 ]; then return 1; fi # Cancel to Main Menu
        if [ $res -eq 0 ]; then
            install_python311 && check_python311
            action_taken=true
        fi
    fi

    # 2. Virtual Environment
    if [ "${STATUS_MAP[venv]:-}" = "fail" ]; then
        request_fix "VENV" "Virtual Environment is not configured." "Create environment now?"
        local res=$?
        if [ $res -eq 2 ]; then return 1; fi
        if [ $res -eq 0 ]; then
            install_venv && check_venv
            action_taken=true
        fi
    fi

    # 3. Python Packages (openant, pyusb, pybind11)
    if [ "${STATUS_MAP[pkg_openant]:-}" = "fail" ] || [ "${STATUS_MAP[pkg_pyusb]:-}" = "fail" ]; then
        request_fix "PACKAGES" "ANT+ Python packages are missing." "Install packages into venv?"
        local res=$?
        if [ $res -eq 2 ]; then return 1; fi
        if [ $res -eq 0 ]; then
            install_python_packages && check_python_packages
            action_taken=true
        fi
    fi

    # 4. Qt5 Runtime & QML Modules
    if [ "${STATUS_MAP[qt5_libs]:-}" = "fail" ] || [ "${STATUS_MAP[qml_modules]:-}" = "fail" ]; then
        request_fix "RUNTIME" "Required Qt5 or QML modules are missing." "Install system dependencies?"
        local res=$?
        if [ $res -eq 2 ]; then return 1; fi
        if [ $res -eq 0 ]; then
            install_qt5_libs && check_qt5_libs && check_qml_modules
            action_taken=true
        fi
    fi

    # 5. Bluetooth Service
    if [ "${STATUS_MAP[bluetooth]:-}" = "fail" ]; then
        request_fix "BLUETOOTH" "Bluetooth service is not active." "Enable Bluetooth service?"
        local res=$?
        if [ $res -eq 2 ]; then return 1; fi
        if [ $res -eq 0 ]; then
            install_bluetooth && check_bluetooth
            action_taken=true
        fi
    fi

    # 6. User Group (plugdev)
    if [ "${STATUS_MAP[plugdev]:-}" = "fail" ]; then
        request_fix "PERMISSIONS" "User lacks USB access permissions." "Add user to plugdev group?"
        local res=$?
        if [ $res -eq 2 ]; then return 1; fi
        if [ $res -eq 0 ]; then
            install_plugdev && check_plugdev
            action_taken=true
        fi
    fi

    # 7. USB udev Rules
    if [ "${STATUS_MAP[udev_rules]:-}" = "fail" ]; then
        request_fix "USB RULES" "ANT+ USB rules are not installed." "Install hardware rules?"
        local res=$?
        if [ $res -eq 2 ]; then return 1; fi
        if [ $res -eq 0 ]; then
            install_udev_rules && check_udev_rules
            action_taken=true
        fi
    fi

    # 8. lsusb command
    if [ "${STATUS_MAP[lsusb]:-}" = "fail" ]; then
        request_fix "UTILITIES" "The 'lsusb' command is missing." "Install usbutils?"
        local res=$?
        if [ $res -eq 2 ]; then return 1; fi
        if [ $res -eq 0 ]; then
            install_lsusb && check_lsusb
            action_taken=true
        fi
    fi

    
    # 9. Configuration File & Initial Setup Wizard
    if [ ! -f "$CONFIG_FILE" ]; then
        request_fix "CONFIG" "No configuration file found." "Start setup wizard?"
        local res=$?
        if [ $res -eq 2 ]; then return 1; fi # Cancel to Main Menu
        if [ $res -eq 0 ]; then
            # A. Select the Treadmill/Bike model
            if select_equipment_flow; then
                # B. Set Weight and Age
                configure_user_profile
                
                # C. Ask to scan for the device now
                draw_bottom_panel_header "BLUETOOTH SETUP"
                clear_info_area
                draw_sealed_row $((LOG_TOP + 2)) "   Equipment and Profile saved."
                draw_sealed_row $((LOG_TOP + 4)) "   Would you like to scan for your device now?"
                
                if prompt_yes_no 6; then
                    perform_bluetooth_scan
                fi
            fi
            
            check_config_file
            action_taken=true
        fi
    fi

    # 10. Systemd Service (Headless only)
    if [ "$SETUP_MODE" = "headless" ] && [ -z "$ACTIVE_SERVICE_FILE" ]; then
        request_fix "SERVICE" "Auto-start service is not installed." "Create qz systemd service?"
        local res=$?
        if [ $res -eq 2 ]; then return 1; fi
        if [ $res -eq 0 ]; then
            create_systemd_service && check_qz_service
            action_taken=true
        fi
    fi

    # Return 0 if we fixed things (triggers a re-probe), 1 if we just finished/skipped
    [ "$action_taken" = true ] && return 0 || return 1
}

run_uninstall_mode() {
    draw_bottom_panel_header "UNINSTALL / RESET"
    
    # 1. Clear interaction area (Borders only)
    clear_info_area
    
    # 2. Build dynamic description based on Python install type
    local rem_list="Removes config, service, and venv"
    # IS_PYENV_INSTALL is set globally during the check_python311 function
    if [ "${IS_PYENV_INSTALL:-false}" = true ]; then
        rem_list="${rem_list}, and Python 3.11"
    fi
    
    # 3. Render Description (Start at Row 1 for density)
    draw_sealed_row $((LOG_TOP + 1)) "   ${rem_list}."
    
    local menu_start=3
    if [ "${HAS_GUI:-false}" = true ]; then
        draw_sealed_row $((LOG_TOP + 2)) "   Note: ${CYAN}${SYMBOL_LOCKED}${NC} items preserved for system stability."
        menu_start=4
    fi

    # 4. Request Confirmation
    draw_sealed_row $((LOG_TOP + menu_start)) "   Proceed with uninstall?"
    
    # Pass offset to prompt_yes_no to place Yes/No on Rows 6 and 7
    if ! prompt_yes_no $((menu_start + 1)); then
        return 1
    fi

    # --- INTERNAL TASK RUNNER (STOPS ON FAILURE) ---
    run_step() {
        if ! run_with_progress "$1" "$2"; then 
            return 1 
        fi
        return 0
    }

    # 5. EXECUTE CLEANUP STEPS
    
    # A. Remove Virtual Environment
    if [ -d "$TARGET_HOME/ant_venv" ]; then
        run_step "Removing Virtual Environment" "rm -rf \"$TARGET_HOME/ant_venv\"" || return 1
        update_status "venv" "fail"
    fi
    
    # B. Remove Python 3.11 (ONLY if it was a pyenv install)
    if [ "${IS_PYENV_INSTALL:-false}" = true ]; then
        local p_root="$TARGET_HOME/.pyenv"
        local p_bin="$p_root/bin/pyenv"
        # Explicit pathing to avoid "command not found" errors
        local py_rem_cmd="sudo -u \"$TARGET_USER\" bash -c \"export PYENV_ROOT='$p_root'; export PATH='\$PYENV_ROOT/bin:\$PATH'; if [ -x '$p_bin' ]; then $p_bin uninstall -f 3.11.9; fi\""
        
        run_step "Removing pyenv Python 3.11.9" "$py_rem_cmd" || return 1
        update_status "python311" "fail"
    fi

    # C. Remove User from plugdev Group
    if groups "$TARGET_USER" | grep -q "\bplugdev\b"; then
        run_step "Removing User from plugdev group" "gpasswd -d $TARGET_USER plugdev" || return 1
        update_status "plugdev" "fail"
    fi
    
    # D. Remove Systemd Service
    if [ -f "/etc/systemd/system/qz.service" ] || [ -f "/lib/systemd/system/qz.service" ]; then
        local svc_cmd="systemctl disable qz.service 2>/dev/null && rm -f /etc/systemd/system/qz.service /lib/systemd/system/qz.service && systemctl daemon-reload"
        run_step "Removing QZ Service" "$svc_cmd" || return 1
        ACTIVE_SERVICE_FILE=""
        update_status "qz_service" "pending"
    fi
    
    # E. Remove USB udev rules
    local rules_file="/etc/udev/rules.d/99-ant-usb.rules"
    if [ -f "$rules_file" ]; then
        run_step "Removing USB udev rules" "rm -f $rules_file && udevadm control --reload-rules" || return 1
        update_status "udev_rules" "fail"
    fi

    # F. Remove Config Directory
    if [ -d "$CONFIG_DIR" ]; then
        run_step "Removing Config Files" "rm -rf \"$CONFIG_DIR\"" || return 1
        update_status "config_file" "fail"
    fi
    
    # 6. DEEP CLEAN (Optional - Headless only)
    if [ "${HAS_GUI:-false}" = false ]; then
        draw_bottom_panel_header "DEEP CLEAN"
        clear_info_area
        draw_sealed_row $((LOG_TOP + 1)) "   Headless System Detected."
        draw_sealed_row $((LOG_TOP + 2)) "   Remove system packages (Qt5, Bluez, usbutils)?"
        
        if prompt_yes_no 4; then
            if prompt_input_yes; then
                local pkgs="libqt5core5a libqt5qml5 libqt5quick5 qml-module-qtquick2 bluez usbutils"
                run_step "Deep Cleaning System Packages" "apt-get remove -y $pkgs && apt-get autoremove -y" || return 1
                update_status "qt5_libs" "fail"
                update_status "bluetooth" "fail"
                update_status "lsusb" "fail"
            fi
        fi
    fi

    # 7. FINAL SUCCESS FEEDBACK
    draw_bottom_panel_header "UNINSTALL COMPLETE"
    clear_info_area
    
    draw_sealed_row $((LOG_TOP + 2)) "   All selected components have been removed."
    draw_sealed_row $((LOG_TOP + 4)) "   Returning to Dashboard..."
    draw_bottom_border
    
    sleep 2
    return 0
}

draw_verifying_screen() {
    local message="$1"
    draw_bottom_panel_header "SYSTEM CHECK"
    
    # Fill interaction area
    clear_info_area
    
    # Row 13: Spacer
    # Row 14: Main Message
    # Row 16: Subtext
    draw_sealed_row $((LOG_TOP + 1)) "   ${WHITE}${message}${NC}"
    draw_sealed_row $((LOG_TOP + 3)) "   Please wait while system status is updated..."
    
    draw_bottom_border ""
    
}

check_final_status() {
    while true; do
        local fails=0 warns=0
        for key in "${!STATUS_MAP[@]}"; do
            local val="${STATUS_MAP[$key]}"
            if [ "$val" = "fail" ]; then ((fails++)); elif [ "$val" = "warn" ]; then ((warns++)); fi
        done

        if [ $fails -gt 0 ]; then
            # --- FAILURE MODE (Fresh Install) ---
            local choice
            prompt_action_menu "$fails"
            choice=$?
            case $choice in
                0) # Guided Fix
                    if run_guided_mode; then
                        draw_verifying_screen "Verifying repairs..."
                        run_all_checks
                    fi
                    ;;
                1) finish_and_exit 0 ;; # Exit
            esac
        else
            # --- SUCCESS MODE (Flattened Setup) ---
            local choice
            prompt_success_menu "$warns"
            choice=$?
            case $choice in
                0) configure_user_profile; check_config_file ;;
                1) select_equipment_flow; check_config_file ;;
                2) perform_bluetooth_scan; check_config_file ;;
                3) # Uninstall
                    if run_uninstall_mode; then
                        draw_verifying_screen "Verifying system state..."
                        run_all_checks
                    fi
                    ;;
                4) finish_and_exit 0 ;; # Exit
            esac
        fi
        sleep 0.05
    done
}

# ============================================================================
# MAIN
# ============================================================================

# Allow tests to source this file without running the full interactive main loop.
# Set QZ_NO_MAIN=1 in the environment when sourcing to prevent main execution.
if [ "${QZ_NO_MAIN:-0}" -eq 1 ]; then
    # If this file is sourced, `return` will work; if executed, fall back to exit.
    return 0 2>/dev/null || exit 0
fi

show_help() {
    cat << 'EOF'
QDomyos-Zwift ANT+ Setup Dashboard

USAGE:
    sudo ./setup_dashboard.sh       # Run validation & interactive setup
    sudo ./setup_dashboard.sh --help

    Non-interactive options:
        --scan-now     Start the Bluetooth scan page immediately and exit

DESCRIPTION:
    Checks system prerequisites and offers a Menu to:
    1. Guided Fix: Step-by-step confirmation for missing items.
    2. Uninstall: Reset config/venv/services (keeps Qt5/Python pkg).

REQUIREMENTS:
    - Root privileges required for installation actions.

EOF
}

cleanup() {
    # This is now handled by the trap above, but we keep it 
    # as a safety fallback for manual calls.
    restore_terminal
}

# Parse arguments
if [ $# -gt 0 ]; then
    for arg in "$@"; do
        case "$arg" in
            --help)
                show_help; exit 0
                ;;
            --scan-now)
                SCAN_NOW=1
                ;;
            --debug-bt)
                DEBUG_BT=1
                ;;
            --bt-log=*)
                BT_DEBUG_LOG="${arg#--bt-log=}"
                ;;
            *)
                echo "Unknown option: $arg. Use --help for usage."; exit 1
                ;;
        esac
    done
fi

# Check root for install capability
[ $EUID -eq 0 ] && CAN_INSTALL=1

# ============================================================================
# MAIN EXECUTION START
# ============================================================================

# 1. Prepare terminal
set_ui_output
refresh_dashboard # Draws the top status table and global frame

# 2. Draw the new consistent startup screen
draw_verifying_screen "Verifying system status..."

# 3. Perform the checks (The status icons will update live in the top panel)
run_all_checks

# If requested via CLI, jump directly to the Bluetooth scanning page and
# perform a scan non-interactively, then exit. This is useful for automated
# captures and avoids the interactive main menu.
if [ "${SCAN_NOW:-0}" -eq 1 ]; then
    perform_bluetooth_scan
    finish_and_exit
fi

# 4. Enter the main menu loop
check_final_status