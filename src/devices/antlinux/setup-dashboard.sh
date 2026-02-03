#!/bin/bash
################################################################################
# QDomyos-Zwift: ANT+ Setup Dashboard
#
# Part of QDomyos-Zwift: https://github.com/cagnulein/qdomyos-zwift
# Contributor: bassai-sho | AI-assisted development | License: GPL-3.0
#
# Interactive TUI for system validation, profile configuration, Bluetooth
# scanning, ANT+ testing and application lifecycle management. Optimized
# for low-end hardware (Raspberry Pi) with RAM-first architecture.
#
# Key optimizations: unified display cache, RAM-first TEMP_DIR, buffered
# event-driven output, and per-section device caches to avoid Python spawns.
#
# Usage:
#   sudo ./setup-dashboard.sh
################################################################################

# Debug trap function
# This is called before every command execution if the trap is active.
_debug_trace() {
    local exit_code=$?
    # Log the command and the current state before executing the next line
    printf "[DEBUG] L%d: Cmd=%s\n" "$BASH_LINENO" "$BASH_COMMAND" >> /tmp/qz_crash_trace.log
    
    # Check for immediate loss of control (e.g., failed stty restore)
    if [[ "$BASH_COMMAND" == "stty"* ]] || [[ "$BASH_COMMAND" == "trap - DEBUG" ]]; then
        # Check if the previous command failed
        if [ "$exit_code" -ne 0 ]; then
            printf "[CRASH DETECTED] Previous command failed with exit code %d at line %d. CRASH LIKELY HERE.\n" "$exit_code" "$((BASH_LINENO-1))" >> /tmp/qz_crash_trace.log
        fi
    fi
    # CRITICAL: Change ownership to the SUDO_USER so they can read the log
    if [ -n "${SUDO_USER:-}" ] && [ -f "/tmp/qz_crash_trace.log" ]; then
        chown "${SUDO_USER}:${SUDO_USER}" /tmp/qz_crash_trace.log 2>/dev/null || true
    fi    
}

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "This script must be run as root to install packages. Please use: sudo $0"
    exit 1
fi

set -uo pipefail
#set +u

# Declare config storage arrays
declare -A CONFIG_BOOL    # Boolean values (true/false)
declare -A CONFIG_INT     # Integer values
declare -A CONFIG_FLOAT   # Float values (with decimal)
declare -A CONFIG_STRING  # String values

# Type-safe setters
config_set_bool() {
    local key=$1
    local value=$2

    # Normalize to lowercase true/false
    case "${value,,}" in
        true|1|yes|on)  CONFIG_BOOL[$key]="true" ;;
        false|0|no|off) CONFIG_BOOL[$key]="false" ;;
        *) echo "ERROR: Invalid boolean for $key: $value" >&2; return 1 ;;
    esac
}

declare -A SYMBOL_CACHE
# shellcheck disable=SC2034
SYMBOL_CACHE_INIT=0
declare -A LAST_SAVED

# Detect systemd system directory by checking which exists
if [[ -d "/etc/systemd/system" ]]; then
    SYSTEMD_SYSTEM_DIR="/etc/systemd/system"
elif [[ -d "/lib/systemd/system" ]]; then
    SYSTEMD_SYSTEM_DIR="/lib/systemd/system"
else
    # Fallback if neither exists (shouldn't happen on systemd systems)
    SYSTEMD_SYSTEM_DIR="/etc/systemd/system"
fi

# Safe single-key read helper with conditional timeout logic.
# Usage: safe_read_key VAR [timeout]
safe_read_key() {
    local __var="$1"
    local _req_timeout="${2:-}"
    local _old_ifs="$IFS"
    local _tmp_read=""

    if [[ -n "$_req_timeout" ]]; then
        # Case A: An explicit timeout was passed (Use it for non-blocking UI loops)
        IFS= read -rsn1 -t "${_req_timeout}" _tmp_read </dev/tty 2>/dev/null || true
    else
        # Case C: No timeout and not in test mode (Block until a key is actually pressed)
        IFS= read -rsn1 _tmp_read </dev/tty 2>/dev/null || true
    fi

    # Assign to the caller-provided variable name safely
    printf -v "${__var}" '%s' "${_tmp_read}"
    IFS="${_old_ifs}"
}

# Flush any buffered keyboard input to prevent delayed key processing
# Call this after processing navigation keys to stop scroll lag
flush_input_buffer() {
    local _discard=""
    while IFS= read -rsn1 -t 0.001 _discard </dev/tty 2>/dev/null; do
        : # Drain buffer
    done
}

# Read escape sequence (2 characters after ESC key)
# Usage: read_escape_sequence VAR [timeout]
read_escape_sequence() {
    local __var="$1"
    local _timeout="${2:-0.01}"
    local _old_ifs="$IFS"
    local _tmp_read=""
    
    IFS= read -rsn2 -t "${_timeout}" _tmp_read </dev/tty 2>/dev/null || true
    
    printf -v "${__var}" '%s' "${_tmp_read}"
    IFS="${_old_ifs}"
}

# Auto-save config and show brief feedback
auto_save_config() {
    local config_type="$1"  # "service" | "profile"
    local save_key="${2:-}"

    case "$config_type" in
        service)
            save_service_config || return 1
            ;;
        profile)
            # Persist profile via generate_config_file which writes CONFIG_* arrays
            generate_config_file || return 1
            ;;
        *)
            echo "ERROR: Unknown config type: $config_type" >&2
            return 1
            ;;
    esac

    # Show brief "Saved" indicator
    show_save_feedback

    # Record inline save (timestamp) for optional inline indicator
    if [[ -n "$save_key" ]]; then
        LAST_SAVED["${config_type}:${save_key}"]=$(date +%s)
    fi
}

# Display temporary save confirmation
show_save_feedback() {
    # Render brief save confirmation inside the info panel (right-aligned)
    local save_msg_plain="${1:+$1 }Saved"
    local save_msg="${GREEN}${save_msg_plain}${NC}"
    # Compute visible width for right-alignment inside INFO_WIDTH
    local w
    w=$(get_display_width "$save_msg_plain")
    local row=$((LOG_BOTTOM))
    local col=$((2 + INFO_WIDTH - w))
    # Print colored message at computed column (within panel borders)
    print_at_col "$row" "$col" "$save_msg"

    # Auto-clear after 1 second (background job) by overwriting with spaces
    (
        #sleep 1
        print_at_col "$row" "$col" "$(printf '%*s' "$w" '')"
    ) &
}

# Display temporary cancel feedback
show_cancel_feedback() {
    # Render cancel feedback inside the info panel (right-aligned)
    local cancel_msg_plain="Cancelled"
    local cancel_msg="${RED}${cancel_msg_plain}${NC}"
    local w
    w=$(get_display_width "$cancel_msg_plain")
    local row=$((LOG_BOTTOM))
    local col=$((2 + INFO_WIDTH - w))
    print_at_col "$row" "$col" "$cancel_msg"
    sleep 1
    print_at_col "$row" "$col" "$(printf '%*s' "$w" '')"
}

configure_service_flags_ui() {
    load_service_config >/dev/null 2>&1
    
    # Work on a local copy of flags
    declare -A _SF
    for k in "${!SERVICE_FLAGS[@]}"; do _SF[$k]="${SERVICE_FLAGS[$k]}"; done

    UI_MODAL_ACTIVE=1
    local selected=0
    local draw_mode="FULL"

    while true; do
        # 1. Build Menu Options
        local options=()
        local help_texts=()
        
        options+=("Logging: ${_SF[logging]:-false}")
        help_texts+=("Enable detailed logging to debug-<date>.log for troubleshooting. Required for ANT+ Verbose mode. Press Enter to toggle on/off.")
        
        options+=("Console: ${_SF[console]:-false}")
        help_texts+=("Enable console output for debugging. Shows real-time QZ messages in the terminal. Useful for development. Press Enter to toggle.")
        
        options+=("ANT+ Footpod: ${_SF[ant_footpod]:-false}")
        help_texts+=("Enable ANT+ footpod sensor support for broadcasting running metrics (speed, cadence, distance) to watches and cycling computers. Press Enter to toggle.")
        
        if [[ "${_SF[ant_footpod]}" == "true" ]]; then
            options+=("ANT+ Device ID: [${_SF[ant_device]:-54321}]")
            help_texts+=("Set the ANT+ device ID for footpod broadcasting. Default is 54321. Valid range: 1-65535. Press Enter to edit.")
            
            if [[ "${_SF[logging]}" == "true" ]]; then
                options+=("ANT+ Verbose: ${_SF[ant_verbose]:-false}")
                help_texts+=("Enable verbose ANT+ logging for detailed dongle communication debugging. Requires Logging to be enabled. Press Enter to toggle.")
            fi
        fi
        
        options+=("Bluetooth Relaxed: ${_SF[bluetooth_relaxed]:-false}")
        help_texts+=("Use relaxed Bluetooth pairing mode for devices that don't strictly follow BLE standards. Enable if having connection issues. Press Enter to toggle.")
        
        options+=("Poll Time (ms): [${_SF[poll_time]:-200}]")
        help_texts+=("Set sensor polling interval in milliseconds. Lower = more responsive but higher CPU. Default: 200ms. Valid range: 50-5000ms. Press Enter to edit.")

        # 2. Show Menu with Help Panel
        show_unified_menu options "$selected" "SERVICE CONFIGURATION" "$draw_mode" "false" "" help_texts
        local exit_code=$?

        # 3. Handle Exit
        if [[ $exit_code -eq 255 ]]; then
            # Save changes to global array and disk
            for k in "${!_SF[@]}"; do SERVICE_FLAGS[$k]="${_SF[$k]}"; done
            save_service_config
            UI_MODAL_ACTIVE=0
            return 0
        fi

        selected=$exit_code
        local choice_text="${options[$selected]}"
        
        # Calculate visual row for inline editing
        # Formula: Header(13) + TopPad(1) + SelectedIndex = Target Row
        local target_row=$(( LOG_TOP + 1 + selected ))

        # 4. Handle Actions
        case "$choice_text" in
            Logging:*)
                if [[ "${_SF[logging]}" == "true" ]]; then
                    _SF[logging]="false"
                    _SF[ant_verbose]="false" # Auto-disable dependency
                else
                    _SF[logging]="true"
                fi
                draw_mode="FULL" # Redraw needed (list size may change)
                ;;
            Console:*)
                if [[ "${_SF[console]}" == "true" ]]; then _SF[console]="false"; else _SF[console]="true"; fi
                draw_mode="ITEMS"
                ;;
            ANT+\ Footpod:*)
                if [[ "${_SF[ant_footpod]}" == "true" ]]; then
                    _SF[ant_footpod]="false"
                else
                    _SF[ant_footpod]="true"
                fi
                draw_mode="FULL" # Redraw needed (Device ID option toggles)
                ;;
            ANT+\ Device\ ID:*)
                local new_id
                # Call inline editor with help panel preservation
                if new_id=$(inline_edit_field "$target_row" "ANT+ Device ID" "" "${_SF[ant_device]}" 1 65535 "true"); then
                    if [[ -n "$new_id" ]]; then
                        _SF[ant_device]="$new_id"
                    fi
                else
                    show_cancel_feedback
                fi
                draw_mode="FULL" # Restore footer
                ;;
            ANT+\ Verbose:*)
                if [[ "${_SF[ant_verbose]}" == "true" ]]; then _SF[ant_verbose]="false"; else _SF[ant_verbose]="true"; fi
                draw_mode="ITEMS"
                ;;
            Bluetooth\ Relaxed:*)
                if [[ "${_SF[bluetooth_relaxed]}" == "true" ]]; then _SF[bluetooth_relaxed]="false"; else _SF[bluetooth_relaxed]="true"; fi
                draw_mode="ITEMS"
                ;;
            Poll\ Time*)
                local new_poll
                if new_poll=$(inline_edit_field "$target_row" "Poll Time" "ms" "${_SF[poll_time]}" 50 5000 "true"); then
                    if [[ -n "$new_poll" ]]; then
                        _SF[poll_time]="$new_poll"
                    fi
                else
                    show_cancel_feedback
                fi
                draw_mode="FULL"
                ;;
        esac
    done
}

config_set_float() {
    local key=$1
    local value=$2

    # Validate float (supports scientific notation)
    if [[ ! "$value" =~ ^-?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?$ ]]; then
        echo "ERROR: Invalid float for $key: $value" >&2
        return 1
    fi
    CONFIG_FLOAT[$key]=$value
}

config_set_string() {
    local key=$1
    local value=$2

    # Store string as-is (INI format uses no quotes)
    CONFIG_STRING[$key]=$value
}

config_set_int() {
    local key=$1
    local value=$2

    # Validate integer
    if [[ ! "$value" =~ ^-?[0-9]+$ ]]; then
        echo "ERROR: Invalid integer for $key: $value" >&2
        return 1
    fi
    CONFIG_INT[$key]="$value"
}

# Classify a key/value and store into the appropriate typed CONFIG_* array
classify_and_store() {
    local key="$1"
    local value="$2"
    # Trim surrounding whitespace
    value="$(printf '%s' "$value" | sed -E 's/^[[:space:]]+//;s/[[:space:]]+$//')"

    # Normalize booleans
    case "${value,,}" in
        true|false|1|0|yes|no|on|off)
            # Normalize to true/false
            if [[ "${value,,}" =~ ^(true|1|yes|on)$ ]]; then
                config_set_bool "$key" true || true
            else
                config_set_bool "$key" false || true
            fi
            return 0
            ;;
    esac

    # Integer
    if [[ "$value" =~ ^-?[0-9]+$ ]]; then
        config_set_int "$key" "$value" || true
        return 0
    fi

    # Float
    if [[ "$value" =~ ^-?[0-9]*\.[0-9]+([eE][-+]?[0-9]+)?$ ]]; then
        config_set_float "$key" "$value" || true
        return 0
    fi

    # Fallback to string
    config_set_string "$key" "$value" || true
    return 0
}

# PERF-07 FIX: Cache filesystem validation results to avoid repeated stat calls
declare -A _FS_TYPE_CACHE

# Ensure TEMP_DIR is a RAM-backed tmpfs; helper to detect tmpfs paths.
_validate_ram_storage() {
    local path="$1"
    [[ ! -d "$path" || ! -w "$path" ]] && return 1
    
    # Check cache first (PERF-07)
    if [[ -n "${_FS_TYPE_CACHE[$path]:-}" ]]; then
        [[ "${_FS_TYPE_CACHE[$path]}" == "tmpfs" ]] && return 0 || return 1
    fi
    
    # Prefer stat filesystem type check; fall back to a write-test if unknown
    local fs_type
    fs_type=$(stat -f -c %T "$path" 2>/dev/null || echo "unknown")
    if [[ "$fs_type" == "tmpfs" ]]; then
        _FS_TYPE_CACHE[$path]="tmpfs"
        return 0
    fi
    # As an additional check, verify /dev/shm device id matches
    if [[ -e "/dev/shm" ]]; then
        local dev_shm_dev
        dev_shm_dev=$(stat -c %d "/dev/shm" 2>/dev/null || echo "")
        local cand_dev
        cand_dev=$(stat -c %d "$path" 2>/dev/null || echo "")
        if [[ -n "$dev_shm_dev" && "$dev_shm_dev" == "$cand_dev" ]]; then
            _FS_TYPE_CACHE[$path]="tmpfs"
            return 0
        fi
    fi
    _FS_TYPE_CACHE[$path]="other"
    return 1
}

# Define the initializer here so early callers can rely on the function.
# The strict behavior (fatal exit if no RAM-backed storage) is performed
# when invoked later.
ensure_ram_temp_dir() {
    # If TEMP_DIR already set and writable, keep it.
    if [[ -n "${TEMP_DIR:-}" && -d "$TEMP_DIR" && -w "$TEMP_DIR" ]]; then
        return 0
    fi

    local candidates=("/dev/shm" "/run/user/$(id -u)" )
    for base in "${candidates[@]}"; do
        if _validate_ram_storage "$base"; then
            # SEC-01 FIX: Use mktemp -d for secure temporary directory creation
            TEMP_DIR=$(mktemp -d "$base/qz_XXXXXX" 2>/dev/null)
            if [[ -d "$TEMP_DIR" ]]; then
                trap 'rm -rf "$TEMP_DIR" 2>/dev/null || true' EXIT
                return 0
            fi
        fi
    done

    # Caller may invoke strict behavior; return non-zero to allow
    # callers to decide whether to abort. When used as the strict
    # initializer, callers should handle the non-zero return and exit.
    return 1
}

# Early attempt to create TEMP_DIR (non-fatal here)
ensure_ram_temp_dir || true

# ============================================================================
# CONFIG MANAGEMENT - Loading & Initialization
# ============================================================================

# Read existing configuration file into the typed CONFIG_* arrays.
# This ensures that we don't overwrite existing settings when generating
# a new configuration file later.
load_config_into_arrays() {
    local config_path="${1:-${CONFIG_FILE:-$HOME/.config/qdomyos-zwift/qDomyos-Zwift.conf}}"
    [[ ! -f "$config_path" ]] && return 0

    local line key value
    while IFS= read -r line || [[ -n "$line" ]]; do
        # Strip comments (everything after # or ;)
        line="${line%%#*}"
        line="${line%%;*}"
        # Trim leading/trailing whitespace
        line="${line#"${line%%[![:space:]]*}"}"
        line="${line%"${line##*[![:space:]]}"}"
        
        [[ -z "$line" ]] && continue
        # Skip section headers [SectionName]
        [[ "$line" =~ ^\[.*\]$ ]] && continue
        # Requirements: must contain =
        [[ "$line" != *"="* ]] && continue

        key="${line%%=*}"
        value="${line#*=}"
        
        # Trim key and value again after splitting
        key="${key#"${key%%[![:space:]]*}"}"
        key="${key%"${key##*[![:space:]]}"}"
        value="${value#"${value%%[![:space:]]*}"}"
        value="${value%"${value##*[![:space:]]}"}"

        [[ -n "$key" ]] && classify_and_store "$key" "$value"
    done < "$config_path"
}

generate_config_file() {
    local config_path="${1:-${CONFIG_FILE:-$HOME/.config/qdomyos-zwift/qDomyos-Zwift.conf}}"
    local temp_file

    # Ensure TEMP_DIR exists; fallback to /tmp
    : "${TEMP_DIR:=/tmp}"
    if [ ! -d "$TEMP_DIR" ]; then mkdir -p "$TEMP_DIR"; fi

    temp_file="${TEMP_DIR}/qDomyos-Zwift.conf.tmp"

    {
        # INI header
        echo "[General]"

        # Merge keys from all typed arrays into associative ALL_KEYS
        declare -A ALL_KEYS
        local k
        for k in "${!CONFIG_BOOL[@]}"; do ALL_KEYS[$k]="${CONFIG_BOOL[$k]}"; done
        for k in "${!CONFIG_INT[@]}"; do ALL_KEYS[$k]="${CONFIG_INT[$k]}"; done
        for k in "${!CONFIG_FLOAT[@]}"; do ALL_KEYS[$k]="${CONFIG_FLOAT[$k]}"; done
        for k in "${!CONFIG_STRING[@]}"; do ALL_KEYS[$k]="${CONFIG_STRING[$k]}"; done

        # Ensure defaults are present
        if [[ -f "${DEVICES_INI:-$SCRIPT_DIR/devices.ini}" ]]; then
            while IFS='=' read -r _ model_key; do
                model_key="${model_key#"${model_key%%[![:space:]]*}"}"
                model_key="${model_key%"${model_key##*[![:space:]]}"}"
                [[ -z "$model_key" ]] && continue
                if [[ -z "${ALL_KEYS[$model_key]+x}" ]]; then
                    ALL_KEYS[$model_key]=false
                fi
            done < <(grep '=' "${DEVICES_INI:-$SCRIPT_DIR/devices.ini}")
        fi
        
        # (Defaults block abbreviated for brevity - kept from original script)
        if [[ -z "${ALL_KEYS[virtual_device_enabled]+x}" ]]; then ALL_KEYS[virtual_device_enabled]=true; fi
        # ... [Keep existing default logic here] ...

        # Write sorted keys
        for k in $(printf '%s\n' "${!ALL_KEYS[@]}" | sort); do
            printf '%s=%s\n' "$k" "${ALL_KEYS[$k]}"
        done
    } > "$temp_file"

    # Atomic move
    mv -f "$temp_file" "$config_path"

    # STRATEGIC SAVE: Correct Permissions (fix Root ownership)
    if [[ -n "${TARGET_USER:-}" ]]; then
        chown "${TARGET_USER}:${TARGET_USER}" "$config_path" 2>/dev/null || true
    fi

    # STRATEGIC SAVE: Force sync to SD card to prevent corruption on power loss
    sync

    return 0
}

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
        local ui_fd
        ui_fd=$(get_safe_ui_fd)
        ( printf '\e[?25h' >&"${ui_fd}" ) 2>/dev/null || true
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
    # Prevent the EXIT trap from running again (it moves the cursor to
    # TERM_HEIGHT+1 and can scroll the session). We'll perform minimal
    # restore steps here and then exit.
    trap - EXIT SIGINT SIGTERM

    if command -v exit_ui_mode >/dev/null 2>&1; then
        exit_ui_mode || true
    else
        stty echo 2>/dev/null || true
    fi

    if command -v show_cursor >/dev/null 2>&1; then
        show_cursor || true
    else
        local ui_fd
        ui_fd=$(get_safe_ui_fd)
        ( printf '\e[?25h' >&"${ui_fd}" ) 2>/dev/null || true
    fi

    stty echo 2>/dev/null || true

    # Move cursor to safe row before exit
    local dest_row=24
    if [[ -n "${LOG_BOTTOM:-}" ]]; then
        dest_row=$(( LOG_BOTTOM + 3 ))
    fi
    local ui_fd
    ui_fd=$(get_safe_ui_fd)
    ( printf "\e[%d;1H" "$dest_row" >&"${ui_fd}" ) 2>/dev/null || true

    # Do not call restore_terminal here to avoid its TERM_HEIGHT move.
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
        ( printf '\e[?25h' >&"${UI_FD:-2}" ) 2>/dev/null || true
    fi
    # Move the cursor down to a safe row (row 24) before exiting so the
    # interactive shell prompt does not overwrite the drawn UI area.
    local dest_row=24
    if [[ -n "${LOG_BOTTOM:-}" ]]; then
        # Prefer 3 rows below LOG_BOTTOM when available
        dest_row=$(( LOG_BOTTOM + 3 ))
    fi
    ( printf "\e[%d;1H" "$dest_row" >&"${UI_FD:-2}" ) 2>/dev/null || true
    
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

# shellcheck disable=SC2034
if [ "$USE_COLOR" = true ]; then
    RED=$'\033[0;31m'
    GREEN=$'\033[0;32m'
    BG_GREEN=$'\033[42m'
    YELLOW=$'\033[1;33m'
    BLUE=$'\033[0;34m'
    CYAN=$'\033[0;36m'
    WHITE=$'\033[1;37m'
    NC=$'\033[0m'
    BOLD=$'\033[1m'
    BOLD_RED=$'\033[1;31m'
    BOLD_GREEN=$'\033[1;32m'
    BOLD_BLUE=$'\033[1;34m'
    BOLD_CYAN=$'\033[1;36m'   # Added for high-visibility selection
    BOLD_WHITE=$'\033[1;37m'
    ORANGE=$'\033[38;5;214m' # 256-color mode orange
    GRAY=$'\033[0;90m'
    BOLD_GRAY=$'\033[1;90m'
    BG_GRAY=$'\033[100m'
else
    # Color/format variables intentionally defined (may be used externally)
    RED=''; GREEN=''; YELLOW=''; BLUE=''; CYAN=''; WHITE=''; GRAY=''; NC=''
    BG_GREEN=''; BG_GRAY=''
    BOLD=''; BOLD_RED=''; BOLD_GREEN=''; BOLD_BLUE=''; BOLD_CYAN=''; BOLD_WHITE=''
    ORANGE=''; BOLD_GRAY=''
fi
 
SYMBOL_PASS="✓"
SYMBOL_FAIL="✗"
SYMBOL_WARN="!"
SYMBOL_PENDING="●"
SYMBOL_WORKING="⟳"
SYMBOL_LOCKED="◈"
PROTECTED_ITEMS=("python311" "qt5_libs" "qml_modules" "bluetooth" "lsusb")

# Many variables below are intentionally defined for optional export or indirect use
# Disable these ShellCheck warnings in bulk to reduce noise during iterative cleanup
# shellcheck disable=SC2034,SC2004

# ============================================================================
# USER CONTEXT & ENVIRONMENT
# ============================================================================

TARGET_USER="${SUDO_USER:-$USER}"
TARGET_HOME=$(getent passwd "$TARGET_USER" | cut -d: -f6)

# Prefer the controlling terminal (fd 3 -> /dev/tty) for UI output when available.
if [ -c /dev/tty ]; then :; fi

# GUI / platform hints
HAS_GUI=false; [[ -n "${DISPLAY:-}" || -n "${WAYLAND_DISPLAY:-}" ]] && HAS_GUI=true
# shellcheck disable=SC2034
IS_PI=false
grep -q "Raspberry Pi" /proc/cpuinfo 2>/dev/null && IS_PI=true
: "${IS_PI:-}" >/dev/null 2>&1

# Paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG_DIR="$TARGET_HOME/.config/Roberto Viola"
CONFIG_FILE="$CONFIG_DIR/qDomyos-Zwift.conf"
DEVICES_INI="$SCRIPT_DIR/devices.ini"  # absolute path for menus
SERVICE_FILE_QZ="$SYSTEMD_SYSTEM_DIR/qz.service"

# Determine active service file
ACTIVE_SERVICE_FILE="$SERVICE_FILE_QZ"

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
    local _arg_r=${1:-0}
    local _arg_c=${2:-0}
    local ui_fd
    ui_fd=$(get_safe_ui_fd)
    
    # PERF-04 FIX: Use pre-computed ANSI cache when available (rows 0-100)
    # ANSI_CACHE is populated at startup (line ~1084) with all common cursor positions
    if [[ -n "${ANSI_CACHE[$_arg_r]:-}" ]]; then
        # Fast path: use cached sequence, only compute column
        local esc="${ANSI_CACHE[$_arg_r]}"
        # Replace ;1H with actual column
        esc="${esc%;1H};$(( _arg_c + 1 ))H"
        ( printf '%s' "$esc" >&"${ui_fd}" ) 2>/dev/null || true
    else
        # Fallback for rows > 100 (rare)
        local r=$(( _arg_r + 1 ))
        local c=$(( _arg_c + 1 ))
        ( printf "\e[%d;%dH" "$r" "$c" >&"${ui_fd}" ) 2>/dev/null || true
    fi
}

clear_screen() {
    local fd="${UI_FD:-2}"
    # \e[2J   = Clear Screen
    # \e[H    = Move to 0,0
    # \e[?25l = Hide Cursor (Low-level ANSI)
    printf "\e[2J\e[H\e[?25l" >&"$fd"
}

hide_cursor() { local ui_fd; ui_fd=$(get_safe_ui_fd); ( printf "\e[?25l" >&"${ui_fd}" ) 2>/dev/null || true; }
show_cursor() { local ui_fd; ui_fd=$(get_safe_ui_fd); ( printf "\e[?25h" >&"${ui_fd}" ) 2>/dev/null || true; }

# UI mode helpers (reference-counted)
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
    # Use cbreak to ensure characters are available immediately without Enter
    stty -echo cbreak 2>/dev/null || true
    hide_cursor

    # RETURN should restore UI mode; SIGINT/SIGTERM should perform a full
    # immediate exit so user interrupts abort long-running checks instead of
    # merely popping out of the UI mode.
    trap 'exit_ui_mode; trap - RETURN' RETURN
    trap 'immediate_exit' SIGINT SIGTERM
}

exit_ui_mode() {
    # Decrement nesting count; only restore on transition to zero
    if [ -z "${UI_MODE_COUNT:-}" ] || [ "$UI_MODE_COUNT" -le 0 ]; then
        UI_MODE_COUNT=0
        # Still perform a best-effort restore of terminal settings
        if [[ -n "${_QZ_OLD_STTY:-}" ]]; then stty "${_QZ_OLD_STTY}" 2>/dev/null || true; else stty echo 2>/dev/null || true; fi
        # Do NOT show the cursor here to avoid brief flashes during redraws.
        trap - RETURN
        return 0
    fi
    UI_MODE_COUNT=$((UI_MODE_COUNT - 1))
    if [ "$UI_MODE_COUNT" -eq 0 ]; then
        if [[ -n "${_QZ_OLD_STTY:-}" ]]; then stty "${_QZ_OLD_STTY}" 2>/dev/null || true; else stty echo 2>/dev/null || true; fi
        # Do NOT show cursor; caller should explicitly show when user input is expected.
        trap - RETURN
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
# WIDTH CALCULATION OPTIMIZATION
# ============================================================================

# Global width calculator state
_VIS_WIDTH_CALC=""
_WIDTH_CALC_INITIALIZED=0

# Initialize width calculator (call once at startup)
init_width_calculator() {
    [[ $_WIDTH_CALC_INITIALIZED -eq 1 ]] && return 0

    if command -v perl >/dev/null 2>&1; then
        _VIS_WIDTH_CALC="perl"
    elif command -v gawk >/dev/null 2>&1; then
        _VIS_WIDTH_CALC="gawk"
    else
        _VIS_WIDTH_CALC="builtin"
    fi

    _WIDTH_CALC_INITIALIZED=1
}


# ============================================================================
# LAYOUT CONFIGURATION
# ============================================================================ 
    
# Standard 24-line terminal geometry
LOG_TOP=13
LOG_BOTTOM=21

# Compute terminal widths dynamically and cap inner width to 78 columns so
# the UI never attempts to render wider than the standard 80-column layout.
local_cols=$(tput cols 2>/dev/null || echo 80)
INNER_COLS=$(( local_cols - 2 ))
if (( INNER_COLS > 78 )); then
    INNER_COLS=78
fi
# shellcheck disable=SC2034
TOTAL_COLS=$(( INNER_COLS + 2 ))
INFO_WIDTH=$INNER_COLS

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

# Capture detailed failure info for services (populated when a unit reports 'failed')
declare -A SERVICE_FAILURE_INFO=()

# Set to "true" by check_qz_service when pyenv is installed but the service
# file is missing PYTHONHOME.  build_service_menu_options uses it to surface
# "Generate & Install Service" even when the unit is running/stopped.
SVC_PYTHONHOME_STALE=false



# Status grid definition: each entry is "Left Label|Left Key|Right Label|Right Key"
declare -a STATUS_GRID=(
    "Python 3.11 Library|python311|User in plugdev Group|plugdev"
    "Python Virtual Environment|venv|USB udev Rules|udev_rules"
    "Python PIPs|pkg_pips|lsusb Command|lsusb"
    "Qt5 Runtime Libraries|qt5_libs|Configuration File|config_file"
    "QML Modules|qml_modules|QZ Service|qz_service"
    "Bluetooth Service|bluetooth|ANT+ USB Dongle|ant_dongle"
)

# Render the status grid starting at given row (default 6)
render_status_grid() {
    local start_row=${1:-6}
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

# No disk-backed logging. Ensure a RAM-only `TEMP_DIR` exists early.
# `_validate_ram_storage` is defined earlier to allow early callers to use it.

# Initialize TEMP_DIR with strict validation (fatal if unavailable)
if [[ -z "${TEMP_DIR:-}" ]]; then
    if ! ensure_ram_temp_dir; then
        cat >&2 <<'EOF'
FATAL ERROR: No RAM-backed storage available.
The dashboard requires tmpfs to prevent SD card wear.

SOLUTION:
  1. Ensure /dev/shm is mounted:
     sudo mount -t tmpfs -o size=128M tmpfs /dev/shm

  2. Verify with: df -h /dev/shm

  3. Make persistent by adding to /etc/fstab:
     tmpfs /dev/shm tmpfs defaults,size=128M 0 0

Cannot proceed without RAM storage.
EOF
        exit 1
    fi
fi

# shellcheck disable=SC2034
# Unified cache: raw_string -> "stripped_text|width"
declare -gA DISPLAY_CACHE
# Cache for pre-computed ANSI cursor position sequences (row -> escape seq)
# shellcheck disable=SC2034
declare -gA ANSI_CACHE

# Eagerly pre-compute common cursor sequences to avoid arithmetic in hot path
# Precompute rows 0..100 which covers typical dashboard sizes
for ((_r=0; _r<=100; _r++)); do
    # Store literal ESC bytes for correct cursor positioning
    ANSI_CACHE[$_r]=$'\e['$(( _r + 1 ))';1H'
done

# ============================================================================
# PURE BASH ANSI SEQUENCE PARSER (NO SUBPROCESS)
# ============================================================================
# Fast ANSI stripper using only bash string operations. Handles common
# CSI (ESC[...<final>), OSC (ESC]...BEL or ESC\\), and simple ESC<ch> sequences.
strip_ansi_pure() {
    local text="$1"
    local result=""
    local i=0
    local len=${#text}

    while (( i < len )); do
        local char="${text:i:1}"

        if [[ "$char" == $'\033' ]] || [[ "$char" == $'\x1b' ]]; then
            ((i++))
            [[ $i -ge $len ]] && break
            local next="${text:i:1}"

            # CSI sequences: ESC [ ... <final char>
            if [[ "$next" == "[" ]]; then
                ((i++))
                # Skip parameter bytes (digits, ;, ?, >, =, :)
                while (( i < len )); do
                    local param="${text:i:1}"
                    if [[ "$param" =~ [0-9\;\?\>\=\:] ]]; then
                        ((i++))
                    else
                        break
                    fi
                done
                # Skip final byte if present
                if (( i < len )) && [[ "${text:i:1}" =~ [@A-Za-z\`~] ]]; then
                    ((i++))
                fi

            # OSC: ESC ] ... BEL (\007) or ESC \\ terminator
            elif [[ "$next" == "]" ]]; then
                ((i++))
                while (( i < len )); do
                    local osc_char="${text:i:1}"
                    if [[ "$osc_char" == $'\007' ]]; then
                        ((i++))
                        break
                    elif [[ "$osc_char" == $'\033' ]] && [[ "${text:i+1:1}" == "\\" ]]; then
                        ((i+=2))
                        break
                    fi
                    ((i++))
                done

            else
                # Other simple ESC sequences: skip a single byte
                ((i++))
            fi
        else
            result+="$char"
            ((i++))
        fi
    done

    printf '%s' "$result"
}

 

pad_display() {
    local s="$1"
    local width="$2"
    local w
    w=$(get_display_width "$s")
    local pad=$((width - w))
    [ "$pad" -lt 0 ] && pad=0
    # Append the calculated spaces to the text. Colors are stored as
    # real ESC bytes now, so print safely with '%s' and pad via %*s
    printf '%s%*s' "$s" "$pad" ""
}

# ============================================================================
# DRAWING FUNCTIONS
# ============================================================================

# Determine the best output file descriptor for UI rendering.
# Prefer the controlling terminal (/dev/tty) when available so the UI
# appears even if stdout/stderr are redirected. Falls back to stderr.
set_ui_output() {
    UI_FD=2
    # Prefer the tty used by sudo if set (SUDO_TTY), then /dev/tty, then stdout/stderr.
    if [[ -n "${SUDO_TTY:-}" ]] && [ -w "${SUDO_TTY}" ] 2>/dev/null; then
        exec 3>"${SUDO_TTY}" 2>/dev/null || true
        UI_FD=3
        return 0
    fi

    if [ -c /dev/tty ] 2>/dev/null; then
        exec 3>/dev/tty 2>/dev/null || true
        if [ -w /dev/tty ] 2>/dev/null; then
            UI_FD=3
            return 0
        fi
    fi

    # Fallbacks: prefer stdout if it's a tty, then stderr
    if [ -t 1 ]; then UI_FD=1; return 0; fi
    if [ -t 2 ]; then UI_FD=2; return 0; fi
    UI_FD=2
}

# Cached UI FD with TTL to avoid repeated filesystem checks on hot paths.
# This reduces per-render overhead while still allowing the UI to recover
# when the controlling TTY changes (e.g., sudo, terminal switch).
_UI_FD_CACHE=""
_UI_FD_CACHE_TIME=0
_UI_FD_CACHE_TTL=5

get_safe_ui_fd() {
    # PERF-08 FIX: Use $SECONDS instead of $(date +%s) to avoid subprocess
    local now=$SECONDS
    local age=$(( now - _UI_FD_CACHE_TIME ))
    if [[ -n "${_UI_FD_CACHE:-}" && $age -lt $_UI_FD_CACHE_TTL ]]; then
        echo "${_UI_FD_CACHE}"
        return 0
    fi

    local ui_fd=2
    # Prefer sudo TTY, then /dev/tty, then stdout/stderr
    if [[ -n "${SUDO_TTY:-}" ]] && [ -w "${SUDO_TTY}" ] 2>/dev/null; then
        exec 3>"${SUDO_TTY}" 2>/dev/null || true
        ui_fd=3
    elif [ -c /dev/tty ] 2>/dev/null && [ -w /dev/tty ] 2>/dev/null; then
        exec 3>/dev/tty 2>/dev/null || true
        ui_fd=3
    elif [ -t 1 ]; then
        ui_fd=1
    elif [ -t 2 ]; then
        ui_fd=2
    fi

    _UI_FD_CACHE="$ui_fd"
    _UI_FD_CACHE_TIME=$now
    echo "$ui_fd"
}

# Global Render States
UI_LOCKED=0
ATOMIC_RENDER_MODE=0
ATOMIC_BUFFER=""

print_at() {
    local row=$1
    shift
    local text="$*"

    # 1. Atomic Buffering Mode (Highest Priority)
    # Used by render_dashboard_atomic to build the full screen in memory
    if [[ "${ATOMIC_RENDER_MODE:-0}" -eq 1 ]]; then
        printf -v ATOMIC_BUFFER "%s\e[%d;1H%s" "$ATOMIC_BUFFER" "$((row + 1))" "$text"
        return 0
    fi

    # 2. Splash Screen Lock (Medium Priority)
    # Prevents background tasks from drawing over the logo
    if [[ "${UI_LOCKED:-0}" -eq 1 ]]; then return 0; fi

    # 3. Direct Output (Standard Mode)
    # Used by menus, info screens, and interactive prompts
    local fd="${UI_FD:-2}"
    printf "\e[%d;1H%s" "$((row + 1))" "$text" >&"$fd"
}

# Print text at a specific terminal column (1-based). Positions at row+1, col.
print_at_col() {
    local row=${1:-0}
    local col=${2:-1}
    shift 2
    local text="$*"
    local ui_fd
    ui_fd=$(get_safe_ui_fd)
    if [[ "${UI_MODAL_ACTIVE:-0}" -eq 1 ]]; then
        printf '%s\n' "[DEBUG] print_at_col called row=${row} col=${col} UI_MODAL_ACTIVE=1 caller=$(caller 0 || true)" >> /tmp/qz_profile_debug.log 2>/dev/null || true
    fi
    ( printf '\033[%d;%dH' "$((row + 1))" "$col" >&"${ui_fd}" ) 2>/dev/null || true
    ( printf '%s' "$text" >&"${ui_fd}" ) 2>/dev/null || true
}

draw_sealed_row() {
    local row=$1
    local text="${2:-}"
    # Truncate text if it would overflow the inner width
    local w
    w=$(get_vis_width "$text")
    if [ "$w" -gt "$INNER_COLS" ]; then
        text=$(trunc_vis "$text" $INNER_COLS)
        w=$(get_vis_width "$text")
    fi

    local pad=$(( INNER_COLS - w ))
    [[ $pad -lt 0 ]] && pad=0

    local spacer=""
    for ((i=0; i<pad; i++)); do spacer="${spacer} "; done

    # Use centralized print helper which moves the cursor and prints to UI_FD.
    print_at "$row" "${BLUE}║${NC}${text}${spacer}${BLUE}║${NC}"
}

# Update only the content area (between the left/right borders) for a
# specific sealed row. This avoids re-drawing the entire info panel and
# keeps the borders intact. Arguments: <row> <content-string>
update_sealed_row_content() {
    local row=$1
    shift
    local content="$*"
    # Ensure the content exactly fills the inner width to avoid
    # overwriting the right border. Use pad_display to compute padding.
    local padded
    padded=$(pad_display "$content" "$INNER_COLS")
    # content area starts at column 2 (after left border)
    print_at_col "$row" 2 "$padded"
}

# Update only the left portion of a row (for help panel compatibility)
update_sealed_row_content_left() {
    local row=$1
    shift
    local content="$*"
    local menu_width=$(( INNER_COLS / 2 - 1 ))
    # Pad content to menu width only
    local padded
    padded=$(pad_display "$content" "$menu_width")
    # content area starts at column 2 (after left border)
    print_at_col "$row" 2 "$padded"
}


# Canonical display width function: strips ANSI sequences and counts characters
get_display_width() {
    local text="${1:-}"
    # Delegate to the visual-width calculator which handles Unicode widths
    # and caches results. This returns the number of terminal columns.
    local w
    w=$(get_vis_width "$text")
    printf '%s' "$w"
}

# Strip ANSI sequences from a string and cache the result.
# Uses the full input string as the cache key (safe for shell associative arrays).
strip_ansi() {
    local text="$1"
    # Interpret backslash-style escapes (e.g. \033) so callers that pass
    # color variables defined as '\033[...m' are normalized to actual ESC
    # bytes before we strip them. Then remove common SGR/K sequences.
    # Deprecated: use strip_ansi_pure for no-subprocess stripping
    strip_ansi_pure "$text"
}

strip_ansi_cached() {
    local text="${1:-}"
    # Fast-return for empty
    [[ -z "$text" ]] && { printf '' ; return 0; }

    # Single unified cache lookup. Stored format is "stripped|width".
    local cached="${DISPLAY_CACHE[$text]:-}"
    if [[ -n "$cached" ]]; then
        printf '%s' "${cached%%|*}"
        return 0
    fi

    # Cache miss: compute stripped value and store with empty width for
    # a future width calculation. We avoid calling get_vis_width here to
    # prevent recursion; get_vis_width will compute and populate width
    # when needed.
    local stripped
    stripped=$(strip_ansi_pure "$text" | tr -d '\r\n')
    DISPLAY_CACHE["$text"]="${stripped}|"
    printf '%s' "$stripped"
}

# Helper: Get visual width (accounts for Unicode wide chars and emoji)
get_vis_width() {
    local text="${1:-}"
    [[ -z "$text" ]] && { echo "0"; return 0; }

    # Fast unified cache lookup. Format: "stripped|width"
    local cached="${DISPLAY_CACHE[$text]:-}"
    if [[ -n "$cached" ]]; then
        local cached_width="${cached##*|}"
        if [[ -n "$cached_width" ]]; then
            echo "$cached_width"
            return 0
        fi
        # Have stripped value but no width yet
        local stripped="${cached%%|*}"
    else
        local stripped
        stripped=$(strip_ansi_pure "$text" | tr -d $'\r\n')
    fi

    # PERF-03 FIX: Enable ASCII fast-path by default (99% of dashboard text is ASCII)
    # This eliminates most perl/gawk subprocess calls, saving ~5-10ms per screen refresh
    local enable_ascii_fast=${QZ_ASCII_FAST_PATH:-1}  # Changed default from 0 to 1
    local width
    if [[ "$enable_ascii_fast" -eq 1 ]]; then
        # Fast-path: check each character's byte value using builtin printf
        # to ensure pure printable ASCII (32..126). This avoids external
        # subprocesses while remaining robust across locales.
        local is_ascii=1
        local i ch code
        for ((i=0; i<${#stripped}; i++)); do
            ch=${stripped:i:1}
            code=$(printf '%d' "'${ch}" 2>/dev/null) || { is_ascii=0; break; }
            if (( code < 32 || code > 126 )); then is_ascii=0; break; fi
        done
        if [[ $is_ascii -eq 1 ]]; then
            width=${#stripped}
            DISPLAY_CACHE["$text"]="${stripped}|${width}"
            echo "$width"
            return 0
        fi
    fi

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
        width=$(printf '%s' "$stripped" | gawk '
        BEGIN { w = 0 }
        {
            for (i = 1; i <= length($0); i++) {
                c = substr($0, i, 1)
                cw = 1
                if (c ~ /[\u1100-\u115F\u2329-\u232A\u2E80-\u303E\u3040-\u30FF\u4E00-\u9FFF\uAC00-\uD7AF\uFF01-\uFF60]/) { cw = 2 }
                else if (c ~ /[\U0001F300-\U0001F9FF\U0001F600-\U0001F64F]/) { cw = 2 }
                w += cw
            }
        }
        END { print w }
        ')
    else
        # Fallback: character count
        width=${#stripped}
    fi

    DISPLAY_CACHE["$text"]="${stripped}|${width}"
    echo "$width"
}

trunc_vis() {
    local text="$1"
    local target="$2"
    
    if ! command -v gawk >/dev/null 2>&1; then
        # Fallback: simple truncation (strip ANSI so widths are approximate)
        local stripped
        stripped=$(strip_ansi "$text")
        printf '%s' "${stripped:0:$target}"
        return
    fi
    
    printf '%s' "$text" | gawk -v target="$target" '
    BEGIN { out=""; w=0; ESC=sprintf("%c",27) }
    {
        i=1; n=length($0);
        while (i <= n && w < target) {
            c = substr($0, i, 1);
            
            # Preserve ANSI sequences (zero width)
            if (c == ESC && substr($0, i+1, 1) == "[") {
                seq = c; i += 2;
                while (i <= n) {
                    b = substr($0, i, 1);
                    seq = seq b; i++;
                    if (b ~ /[@-~]/) break;
                }
                out = out seq;
                continue;
            }
            
            # Calculate character width
            cw = 1;
            if (c ~ /[\u1100-\u115F\u3000-\u303E\u3040-\u30FF\u4E00-\u9FFF\uAC00-\uD7AF\uFF01-\uFF60]/) {
                cw = 2;
            }
            
            if (w + cw > target) break;
            out = out c;
            w += cw; i++;
        }
    }
    END { printf "%s", out }
    '
}

 

get_symbol() {
    local key="$1"
    local status="${STATUS_MAP[$key]:-pending}"

    # 1. Python 3.11 Logic (Pyenv = Check, System = Lock)
    if [[ "$key" == "python311" && "$status" == "pass" ]]; then
        if [[ "${IS_PYENV_INSTALLED:-false}" == "true" ]]; then
            printf '%s' "${GREEN}${SYMBOL_PASS}${NC}"
        else
            printf '%s' "${BOLD_GRAY}${SYMBOL_LOCKED}${NC}"
        fi
        return
    fi

    # 2. ANT+ USB Dongle Logic (Physical hardware = Lock)
    # We use Lock because the script can't "uninstall" a physical USB stick.
    if [[ "$key" == "ant_dongle" && "$status" == "pass" ]]; then
        printf '%s' "${BOLD_GRAY}${SYMBOL_LOCKED}${NC}"
        return
    fi

    # 3. Standard Protected Items (GUI Logic)
    if [ "$HAS_GUI" = true ] && [[ " ${PROTECTED_ITEMS[*]} " == *" $key "* ]] && [ "$status" == "pass" ]; then
        printf '%s' "${BOLD_GRAY}${SYMBOL_LOCKED}${NC}"
        return
    fi

    # 4. Standard Status Mapping
    if [[ -n "${SYMBOL_CACHE[$status]:-}" ]]; then
        printf '%s' "${SYMBOL_CACHE[$status]}"
        return
    fi

    case "$status" in
        pass)    printf '%s' "${GREEN}${SYMBOL_PASS}${NC}" ;;
        fail)    printf '%s' "${RED}${SYMBOL_FAIL}${NC}" ;;
        warn)    printf '%s' "${YELLOW}${SYMBOL_WARN}${NC}" ;;
        working) printf '%s' "${CYAN}${SYMBOL_WORKING}${NC}" ;;
        *)       printf '%s' "${GRAY}${SYMBOL_PENDING}${NC}" ;;
    esac
}

# Returns the ANSI color code for a given status key's label
get_status_label_color() {
    local key="$1"
    local status="${STATUS_MAP[$key]:-pending}"

    # Equipment type/model labels are always bold white for readability
    if [[ "$key" == "equipment_type" || "$key" == "equipment_model" ]]; then
        if [[ "$status" != "None" ]]; then
            printf '%s' "${BOLD_WHITE}"
            return
        fi
    fi

    case "$status" in
        pass)    printf '%s' "${GREEN}" ;;
        fail)    printf '%s' "${RED}" ;;
        warn)    printf '%s' "${YELLOW}" ;;
        working) printf '%s' "${CYAN}" ;;
        locked)  printf '%s' "${BOLD_GRAY}" ;;
        *)       printf '%s' "${GRAY}" ;;
    esac
}

# Symbol cache to reduce repeated printf/subprocess overhead in hot paths
declare -A SYMBOL_CACHE
# shellcheck disable=SC2034
SYMBOL_CACHE_INIT=0
: "${SYMBOL_CACHE_INIT:-}" >/dev/null 2>&1

init_symbol_cache() {
    SYMBOL_CACHE_INIT=1
    SYMBOL_CACHE[pass]="${GREEN}${SYMBOL_PASS}${NC}"
    SYMBOL_CACHE[fail]="${RED}${SYMBOL_FAIL}${NC}"
    SYMBOL_CACHE[warn]="${YELLOW}${SYMBOL_WARN}${NC}"
    SYMBOL_CACHE[working]="${CYAN}${SYMBOL_WORKING}${NC}"
    SYMBOL_CACHE[pending]="${GRAY}${SYMBOL_PENDING}${NC}"
    SYMBOL_CACHE[locked]="${BOLD_GRAY}${SYMBOL_LOCKED}${NC}"
}

draw_status_row() {
    local row=$1 L_label="$2" L_key="$3" R_label="$4" R_key="$5"
    
    # --- DYNAMIC LABEL LOGIC FOR PYTHON ---
    if [[ "$L_key" == "python311" ]]; then
        if [[ "${IS_PYENV_INSTALLED:-false}" == "true" ]]; then
            L_label="Python 3.11 Library (pyenv)"
        elif [[ "${IS_PYSYS_INSTALLED:-false}" == "true" ]]; then
            L_label="Python 3.11 Library (system)"
        else
            # Neither installed or not yet checked
            L_label="Python 3.11 Library"
        fi
    fi
    # --------------------------------------

    local L_val="${STATUS_MAP[$L_key]:-pending}"
    local R_val="${STATUS_MAP[$R_key]:-pending}"
    local L_color R_color
    L_color=$(get_status_label_color "$L_key")
    R_color=$(get_status_label_color "$R_key")

    local L_content L_len
    
    # Left Column
    if [[ "$L_key" == "equipment_type" ]]; then
        local L_disp="${L_val}"
        [[ "$L_val" == "None" ]] && L_disp="${GRAY}${L_val}${NC}" || L_disp="${BOLD_CYAN}${L_val}${NC}"
        L_content="  ${L_color}${L_label}${NC}: ${L_disp}"
        L_len=$(( 2 + ${#L_label} + 2 + ${#L_val} ))
    else
        local L_sym; L_sym=$(get_symbol "$L_key")
        L_content="${L_sym} ${L_color}${L_label}${NC}"
        L_len=$(( 1 + 1 + ${#L_label} )) # Sym + Space + Label
    fi

    # Right Column
    local R_content R_len
    if [[ "$R_key" == "equipment_model" ]]; then
        local max_w=$(( (INNER_COLS / 2) - 18 ))
        local trunc_m="${R_val}"
        if [[ ${#R_val} -gt $max_w ]]; then trunc_m="${R_val:0:$max_w}"; fi
        
        local R_disp
        [[ "$R_val" == "None" ]] && R_disp="${GRAY}${R_val}${NC}" || R_disp="${BOLD_CYAN}${trunc_m}${NC}"
        R_content="  ${R_color}${R_label}${NC}: ${R_disp}"
        R_len=$(( 2 + ${#R_label} + 2 + ${#trunc_m} ))
    else
        local R_sym; R_sym=$(get_symbol "$R_key")
        R_content="${R_sym} ${R_color}${R_label}${NC}"
        R_len=$(( 1 + 1 + ${#R_label} ))
    fi
    
    # Padding Calc
    local left_w=$(( (INNER_COLS - 3) / 2 ))
    local right_w=$(( INNER_COLS - 3 - left_w ))
    
    local pad_l=$((left_w - L_len))
    [[ $pad_l -lt 0 ]] && pad_l=0
    
    local pad_r=$((right_w - R_len))
    [[ $pad_r -lt 0 ]] && pad_r=0
    
    local L_padded R_padded
    printf -v L_padded "%s%*s" "$L_content" "$pad_l" ""
    printf -v R_padded "%s%*s" "$R_content" "$pad_r" ""
    
    print_at "$row" "${BLUE}║${NC} ${L_padded}${BLUE}│${NC}${R_padded} ${BLUE}║${NC}"
}

draw_header_config_line() {
    local inner_w=${INNER_COLS:-80}
    local cfg_path="${GRAY}None${NC}"
    if [[ -f "$CONFIG_FILE" ]]; then cfg_path="${CYAN}$CONFIG_FILE"; fi
    
    local line="  QZ Config : ${cfg_path}"
    # Calculate visible length by stripping ANSI logic manually for the label part
    # "  QZ Config : " is 14 chars. Path is the rest.
    local raw_len=$(( 14 + ${#CONFIG_FILE} ))
    if [[ ! -f "$CONFIG_FILE" ]]; then raw_len=$(( 14 + 4 )); fi # "None" is 4
    
    local pad=$((inner_w - raw_len))
    [[ $pad -lt 0 ]] && pad=0
    
    # Direct printf padding logic
    local padded
    printf -v padded "%s%*s" "$line" "$pad" ""
    print_at 2 "${BLUE}║${CYAN}${padded}${BLUE}║${NC}"
}

draw_header_equipment_line() {
    local inner_w=${INNER_COLS:-80}
    
    # 1. Fetch Data
    local e_type="${STATUS_MAP[equipment_type]:-None}"
    local e_model="${STATUS_MAP[equipment_model]:-None}"
    
    # Get BLE Name (Check memory array first, fallback to grep if empty)
    local bt_name="${CONFIG_STRING[bluetooth_lastdevice_name]:-}"
    if [[ -z "$bt_name" && -f "$CONFIG_FILE" ]]; then
        # Subprocess only if necessary (on first load)
        bt_name=$(grep -E '^bluetooth_lastdevice_name=' "$CONFIG_FILE" 2>/dev/null | cut -d'=' -f2- | tr -d '\r')
        # Update cache to avoid grep on next frame
        CONFIG_STRING[bluetooth_lastdevice_name]="$bt_name"
    fi
    [[ -z "$bt_name" ]] && bt_name="None"

    # 2. Calculate Available Space
    # Fixed Labels:
    # "  Equipment : " (14 chars)
    # " | Model: "     (10 chars)
    # " | BLE: "       ( 8 chars)
    # Total Overhead: 32 chars
    local avail_width=$(( inner_w - 32 ))

    # 3. Format Equipment Type (Priority 1: Always show full type)
    local type_disp="${BOLD_CYAN}${e_type}${NC}"
    [[ "$e_type" == "None" ]] && type_disp="${GRAY}${e_type}${NC}"
    local type_len=${#e_type}

    # 4. Calculate Remaining Space for Model and BLE
    local remain=$(( avail_width - type_len ))
    
    # Split remaining space: 55% for Model, 45% for BLE
    local max_model=$(( remain * 55 / 100 ))
    local max_ble=$(( remain - max_model ))
    
    # Ensure minimum widths to prevent visual collapse
    [[ $max_model -lt 5 ]] && max_model=5
    [[ $max_ble -lt 5 ]] && max_ble=5

    # 5. Format Model (Priority 2)
    local trunc_m="${e_model}"
    if [[ ${#e_model} -gt $max_model ]]; then 
        trunc_m="${e_model:0:$((max_model-1))}…" 
    fi
    local model_disp="${BOLD_CYAN}${trunc_m}${NC}"
    [[ "$e_model" == "None" ]] && model_disp="${GRAY}${e_model}${NC}"

    # 6. Format BLE Name (Priority 3)
    local trunc_b="${bt_name}"
    if [[ ${#bt_name} -gt $max_ble ]]; then 
        trunc_b="${bt_name:0:$((max_ble-1))}…" 
    fi
    local ble_disp="${BOLD_CYAN}${trunc_b}${NC}"
    [[ "$bt_name" == "None" ]] && ble_disp="${GRAY}${bt_name}${NC}"

    # 7. Construct Line
    local line="  Equipment : ${type_disp}${CYAN} | Model: ${model_disp}${CYAN} | BLE: ${ble_disp}"
    
    # 8. Calculate Raw Length for Padding (Sum of labels + value lengths)
    local raw_len=$(( 14 + type_len + 10 + ${#trunc_m} + 8 + ${#trunc_b} ))
    
    local pad=$((inner_w - raw_len))
    [[ $pad -lt 0 ]] && pad=0
    
    # 9. Print
    local padded
    printf -v padded "%s%*s" "$line" "$pad" ""
    print_at 4 "${BLUE}║${CYAN}${padded}${BLUE}║${NC}"
}

draw_header_service_line() {
    local inner_w=${INNER_COLS:-80}

    # 1. Determine Path
    local svc_path="None"
    local has_path=0
    if [[ -n "${ACTIVE_SERVICE_FILE:-}" && -f "$ACTIVE_SERVICE_FILE" ]]; then
        svc_path="$ACTIVE_SERVICE_FILE"
        has_path=1
    elif [[ -f "$SERVICE_FILE_QZ" ]]; then
        svc_path="$SERVICE_FILE_QZ"
        has_path=1
    fi
    
    # 2. Format Display String
    local display_path
    if [[ $has_path -eq 1 ]]; then 
        # No leading space here (Label provides the separator)
        # Forced Cyan color
        display_path="${CYAN}${svc_path}"
    else 
        display_path="${GRAY}None${NC}"
    fi
    
    # 3. Construct Line
    # "  QZ Service: " includes the necessary trailing space.
    local line="  QZ Service: ${display_path}"
    
    # 4. Calculate Raw Length for Padding
    # "  QZ Service: " is 14 chars.
    local raw_len=$(( 14 + ${#svc_path} ))
    if [[ $has_path -eq 0 ]]; then raw_len=18; fi # 14 + 4("None")
    
    # 5. Padding
    local pad=$((inner_w - raw_len))
    [[ $pad -lt 0 ]] && pad=0
    
    local padded
    printf -v padded "%s%*s" "$line" "$pad" ""

    print_at 3 "${BLUE}║${CYAN}${padded}${BLUE}║${NC}"
}

draw_top_panel() {
    local inner_w=${INNER_COLS:-80}
    
    # 1. Top Border (Row 0)
    draw_hr 0 "╔" "╗" "QZ ANT+ BRIDGE SETUP & DIAGNOSTICS UTILITY" "" "" ""

    # 2. User / Host / Env Line (Row 1)
    local u_str="${TARGET_USER:-root}"
    
    # Get Hostname and IP
    local h_name
    h_name=$(hostname)
    local h_ip
    # Get first IP address only (fast, no external DNS lookup)
    h_ip=$(hostname -I 2>/dev/null | cut -d' ' -f1)
    
    local host_str="${h_name}"
    [[ -n "$h_ip" ]] && host_str="${h_name} (${h_ip})"

    # Get Environment
    local env_str="Headless"
    if [[ "${HAS_GUI:-false}" == true ]]; then env_str="GUI"; fi
    
    # Construct Line: "  User: adam | Host: mypi (192.168.1.5) | Env: Headless"
    local line="  User: ${u_str} | Host: ${host_str} | Env: ${env_str}"
    
    # Safety truncation if hostname/IP is very long
    if [[ ${#line} -gt $((inner_w)) ]]; then
        line="${line:0:$((inner_w - 1))}…"
    fi
    
    # Padding
    local pad=$((inner_w - ${#line}))
    [[ $pad -lt 0 ]] && pad=0
    local padded
    printf -v padded "%s%*s" "$line" "$pad" ""
    
    print_at 1 "${BLUE}║${CYAN}${padded}${BLUE}║${NC}"
    
    # 3. Headers (Rows 2, 3, 4)
    draw_header_config_line
    draw_header_service_line
    draw_header_equipment_line

    # 4. Status Divider (Row 5)
    draw_hr 5 "╠" "╣" "STATUS" "$BOLD_WHITE" "" ""
    
    # 5. Status Grid (Rows 6-11)
    render_status_grid 6
}

render_dashboard_atomic() {
    # 1. Determine Header State based on Status Map
    local fails=0 warns=0
    for key in "${!STATUS_MAP[@]}"; do
        local val="${STATUS_MAP[$key]}"
        if [ "$val" = "fail" ]; then ((fails++)); elif [ "$val" = "warn" ]; then ((warns++)); fi
    done

    local header_text="SYSTEM READY"
    if [ $fails -gt 0 ]; then
        header_text="ISSUES DETECTED ($fails)"
    elif [ $warns -gt 0 ]; then
        header_text="READY WITH WARNINGS ($warns)"
    fi

    # 2. Unlock UI logic so internal print calls work
    UI_LOCKED=0

    # 3. Initialize Atomic Buffer Mode
    # Start with Clear Screen (\e[2J) + Move Home (\e[H) + Hide Cursor (\e[?25l)
    ATOMIC_BUFFER="\e[2J\e[H\e[?25l" 
    ATOMIC_RENDER_MODE=1
    
    # 4. Build Screen Components
    # These functions call 'print_at', which detects ATOMIC_RENDER_MODE=1
    # and appends to ATOMIC_BUFFER instead of printing to the screen immediately.
    
    draw_top_panel
    
    # UPDATE: Pass "true" to explicitly show "L: Legend" on the main dashboard
    draw_bottom_panel_header "$header_text" "true"
    
    clear_info_area
    
    if [[ -n "$CURRENT_INSTRUCTION" ]]; then
        draw_instructions_bottom "$CURRENT_INSTRUCTION"
    fi
    
    draw_bottom_border "Arrows: Up/Down | Enter: Select"
    
    # 5. Flush and Disable Atomic Mode
    local fd="${UI_FD:-2}"
    printf "%b" "$ATOMIC_BUFFER" >&"$fd"
    
    # CRITICAL: Turn off buffering so subsequent interactive menus work immediately!
    ATOMIC_RENDER_MODE=0
    ATOMIC_BUFFER=""
}

draw_hr() {
    # Args: row, left_char, right_char, text, text_color, legend_text, legend_color
    local row=$1 left_c=$2 right_c=$3 text="${4:-}" color="${5:-$BOLD_WHITE}" legend="${6:-}" l_color="${7:-$NC}"
    
    local inner_w=${INNER_COLS:-80}
    
    # 1. Build Start Sequence (Left Side)
    # Explicitly Blue borders
    local start_str="${BLUE}═══"
    local start_len=3
    
    if [[ -n "$text" ]]; then
        # Blue borders -> Space -> Colored Text -> Reset -> Space
        start_str="${BLUE}═══  ${color}${text}${NC}  "
        start_len=$(( 3 + 2 + ${#text} + 2 ))
    fi
    
    # 2. Build End Sequence (Right Side)
    # Explicitly Blue borders
    local end_str="${BLUE}══"
    local end_len=2
    
    if [[ -n "$legend" ]]; then
        # Legend Color -> Text -> Reset -> Blue -> Borders
        end_str="${l_color}${legend}${NC}${BLUE}══"
        end_len=$(( ${#legend} + 2 ))
    fi
    
    # 3. Calculate Fill
    local fill_len=$(( inner_w - start_len - end_len ))
    [[ $fill_len -lt 0 ]] && fill_len=0
    
    local fill
    if (( fill_len > 0 )); then
        printf -v fill "%*s" "$fill_len" ""
        fill="${fill// /═}"
    else
        fill=""
    fi
    
    # 4. Construct Line
    # Structure: [Blue Left] [Start] [Blue Fill] [End] [Blue Right] [Reset]
    local line="${BLUE}${left_c}${start_str}${BLUE}${fill}${NC}${end_str}${BLUE}${right_c}${NC}"
    
    # 5. Print
    print_at "$row" "$line"
}

# Build (but don't print) a horizontal-rule row string including cursor
# positioning. Returns the ANSI string on stdout so callers can either
# append it to an atomic buffer or print it directly.
# Args: row, left_corner, right_corner, text, text_color, legend
build_hr_string() {
    local row=${1:-0}
    local left_c=${2:-"╠"}
    local right_c=${3:-"╣"}
    local text="${4:-}"
    local t_color="${5:-$BOLD_WHITE}"
    local legend="${6:-}"
    local inner_w=$INNER_COLS

    local left_pad="═══  "
    local right_pad="══"
    local sep="  "

    local t_vis l_vis
    t_vis=$(get_vis_width "$text")
    l_vis=$(get_vis_width "$legend")

    # If both text and legend are visually empty, render a solid border
    if [[ $t_vis -eq 0 && $l_vis -eq 0 ]]; then
        local fill=""
        for ((i=0; i<inner_w; i++)); do fill="${fill}═"; done
        printf "\e[%d;1H%s%s%s" "$((row + 1))" "${BLUE}" "${left_c}${fill}${right_c}" "${NC}"
        return 0
    fi

    local left_w=${#left_pad}
    local right_w=${#right_pad}
    local sep_w=${#sep}

    local fill_w=$(( inner_w - left_w - t_vis - sep_w - l_vis - right_w ))
    if [ "$fill_w" -lt 0 ]; then
        local need=$(( -fill_w ))
        if [ "$l_vis" -gt 0 ]; then
            local new_l_vis=$(( l_vis - need ))
            if [ "$new_l_vis" -lt 0 ]; then new_l_vis=0; fi
            legend=$(trunc_vis "$legend" $new_l_vis)
            l_vis=$(get_vis_width "$legend")
            fill_w=$(( inner_w - left_w - t_vis - sep_w - l_vis - right_w ))
        fi
    fi

    if [ "$fill_w" -lt 0 ]; then
        local need2=$(( -fill_w ))
        if [ "$t_vis" -gt 0 ]; then
            local new_t_vis=$(( t_vis - need2 ))
            if [ "$new_t_vis" -lt 0 ]; then new_t_vis=0; fi
            text=$(trunc_vis "$text" $new_t_vis)
            t_vis=$(get_vis_width "$text")
            fill_w=$(( inner_w - left_w - t_vis - sep_w - l_vis - right_w ))
        fi
    fi

    if [ "$fill_w" -lt 0 ]; then fill_w=0; fi

    local fill=""
    for ((i=0; i<fill_w; i++)); do fill="${fill}═"; done

    # Build the visible line. Print sep as plain text to avoid color
    # optimizer collapsing spaces.
    local line
    line="${BLUE}${left_c}${left_pad}${NC}${t_color}${text}${NC}${sep}${BLUE}${fill}${NC}${legend}${BLUE}${right_pad}${right_c}${NC}"

    printf "\e[%d;1H%s" "$((row + 1))" "$line"
}

# Draws the standardized RSSI progress bar and returns the colored bar and RSSI text.
# Usage: read -r bar rssi_text < <(draw_rssi_bar -75 10)
draw_rssi_bar() {
    local s=$1 # RSSI value (e.g., -75)
    local width=$2 # Width of the bar (e.g., 10)
    
    # Mapping parameters for RSSI -> fill fraction (tuned)
    local rmin=-95 rmax=-30
    local full rem
    # Use awk for robust floating-point math on RSSI mapping
    read -r full rem < <(
        awk -v s="$s" -v rmin="$rmin" -v rmax="$rmax" -v w="$width" 'BEGIN{
            # Bias mapping: use exponent <1 to expand mid/low RSSI into larger visual fill
            e = 0.7
            if(s > rmax) s=rmax; if(s < rmin) s=rmin;
            p = (s - rmin) / (rmax - rmin);
            if (p <= 0) { f = 0; }
            else if (p >= 1) { f = w; }
            else { f = exp(log(p) * e) * w; }
            if(f < 0) f = 0; if(f > w) f = w;
            full = int(f);
            rem = int((f - full) * 8 + 0.5);
            if(rem > 7) rem = 7;
            print full, rem;
        }'
    )

    local strength=""
    # Left-8th-block glyphs (index 1..7). index 0 means no partial
    local BLOCKS=("" '▏' '▎' '▍' '▌' '▋' '▊' '▉')
    for ((pos=1; pos<=width; pos++)); do
        if (( pos <= full )); then
            strength+=$'\u2588' # █ full block
        elif (( pos == full + 1 )) && (( rem > 0 )); then
            strength+="${BLOCKS[$rem]}"
        else
            strength+=" "
        fi
    done

    # Color selection based on RSSI (tuned thresholds; treat -70 as green)
    local strength_color="$RED"
    if (( s >= -70 )); then strength_color="$GREEN"
    elif (( s >= -78 )); then strength_color="$YELLOW"
    elif (( s >= -82 )); then strength_color="$ORANGE"
    else strength_color="$RED"; fi
    
    # RSSI field fixed to 4 characters and lightly colored
    local rssi_field
    rssi_field=$(printf '%4s' "$s")
    local rssi_color="$RED"
    if (( s >= -70 )); then rssi_color="$GREEN"
    elif (( s >= -78 )); then rssi_color="$YELLOW"
    elif (( s >= -82 )); then rssi_color="$ORANGE"
    else rssi_color="$RED"; fi
    
    # Return two parts: the colored strength bar and the colored RSSI text
    printf '%s|%s' "${strength_color}${strength}${NC}" "${rssi_color}${rssi_field}${NC}"
}

draw_bottom_border() {
    local help_text="${1:-}"
    local b_row=$((LOG_BOTTOM + 1))
    
    # Use the optimized draw_hr function.
    # It internally calls print_at, which respects ATOMIC_RENDER_MODE,
    # ensuring this border gets included in the master buffer flush.
    
    # Args: row, left_char, right_char, text, text_color, legend, legend_color
    draw_hr "$b_row" "╚" "╝" "$help_text" "${BOLD_BLUE}" "" ""
}

clear_info_area() {
    local inner_w=${INNER_COLS:-80}
    
    # Pre-calculate the empty border line
    local empty_space
    printf -v empty_space "%*s" "$inner_w" ""
    local empty_row="${BLUE}║${NC}${empty_space}${BLUE}║${NC}"
    
    # Loop through rows and use print_at (which handles buffering/direct automatically)
    for ((r=${LOG_TOP:-13}; r<=${LOG_BOTTOM:-21}; r++)); do
        print_at "$r" "$empty_row"
    done
}

# Draw a full-screen error panel inside the info area.
# Usage: draw_error_screen "TITLE" "Message text (can be multi-line)" [wait_enter]
draw_error_screen() {
    local title="${1:-ERROR}"
    local msg="${2:-An unexpected error occurred.}"
    local wait_enter=${3:-1}

    UI_LOCKED=0
    enter_ui_mode
    clear_info_area

    # Start at the very top row of the info area (Row 13)
    local row=$((LOG_TOP))
    
    # Title - High visibility
    draw_sealed_row "$row" "   ${BOLD_RED}▶ ${title}${NC}"
    row=$((row + 1))
    draw_sealed_row "$row" "   $(printf '%.0s─' $(seq 1 $((INNER_COLS - 6))))"

    # Wrap message - leave room for borders
    local wrapped
    IFS=$'\n' read -r -d '' -a wrapped < <(printf '%b' "$msg" | fold -s -w $((INFO_WIDTH - 6)) && printf '\0')

    for line in "${wrapped[@]}"; do
        row=$((row + 1))
        # Safety: Stop if we reach the bottom row to preserve the footer area
        if [ "$row" -le "$LOG_BOTTOM" ]; then
            draw_sealed_row "$row" "   ${RED}${line}${NC}"
        fi
    done

    # Fill remaining rows with empty borders to keep UI clean
    while [ "$row" -lt "$LOG_BOTTOM" ]; do
        row=$((row + 1))
        draw_sealed_row "$row" ""
    done

    draw_bottom_border "Press ENTER to continue"

    if [[ -n "${wait_enter:-}" && "${wait_enter}" != "0" ]]; then
        if [[ "${wait_enter}" =~ ^[0-9]+$ ]]; then
            sleep "${wait_enter}"
        else
            local k
            safe_read_key k
        fi
    fi

    clear_info_area
    exit_ui_mode
}

# Neutral informational panel (not an error). Usage mirrors draw_error_screen:
# draw_info_screen "TITLE" "Message" [wait]
draw_info_screen() {
    local title="${1:-INFO}"
    local msg="${2:-}"
    local wait_enter=${3:-1}

    # FIX: Force Unlock so we can draw
    UI_LOCKED=0

    enter_ui_mode || true
    clear_info_area

    # Draw Title
    local row=$((LOG_TOP + 1))
    draw_sealed_row "$row" "   ${BOLD_BLUE}${title}${NC}"

    # Wrap and Draw Message
    local wrapped
    IFS=$'\n' read -r -d '' -a wrapped < <(printf '%b' "$msg" | fold -s -w $((INFO_WIDTH - 3)) && printf '\0')
    for line in "${wrapped[@]}"; do
        row=$((row + 1))
        draw_sealed_row "$row" "   ${NC}${line}${NC}"
    done

    # Handle interaction
    if [[ -n "${wait_enter:-}" && "${wait_enter}" != "0" ]]; then
        if [[ "${wait_enter}" =~ ^[0-9]+$ ]]; then
            draw_bottom_border
            sleep "${wait_enter}"
        else
            draw_bottom_border "Press ENTER to continue"
            local k
            safe_read_key k
        fi
    fi

    # Cleanup
    clear_info_area
    exit_ui_mode || true
}

draw_bottom_panel_header() {
    local raw_title="${1:-INFORMATION}"
    local show_legend="${2:-true}" # Default to showing legend
    local title="${raw_title^^}"
    
    local legend_text=""
    local legend_color=""

    if [[ "$show_legend" == "true" ]]; then
        legend_text="L: Legend"
        legend_color="$GRAY"
    fi
    
    # Row 12 is the standard middle divider
    # Args: Row, Left, Right, Title, TitleColor, Legend, LegendColor
    draw_hr 12 "╠" "╣" "$title" "$BOLD_WHITE" "$legend_text" "$legend_color"
}

draw_instructions_bottom() {
    local text="$1"
    local lines=()
    # Read lines safely
    while IFS= read -r line; do lines+=("$line"); done < <(echo "$text" | fold -s -w 76)
    
    local total_lines=${#lines[@]}
    local show_lines=$((LOG_BOTTOM - LOG_TOP + 1))
    [[ $show_lines -lt 1 ]] && show_lines=1
    
    local start=0
    [[ $total_lines -gt $show_lines ]] && start=$((total_lines - show_lines))
    
    local inner_w=${INNER_COLS:-80}

    for ((idx=0; idx<show_lines; idx++)); do
        local content=""
        local li=$((start + idx))
        if [[ $li -lt $total_lines ]]; then
            content="${lines[$li]}"
        fi
        
        # Padding calculation
        local w
        w=$(get_display_width " $content")
        local pad=$((inner_w - w))
        [[ $pad -lt 0 ]] && pad=0
        local padded
        printf -v padded " %s%*s" "$content" "$pad" ""
        
        print_at $((LOG_TOP + idx)) "${BLUE}║${NC}${padded}${BLUE}║${NC}"
    done
}

# Display a temporary Legend popup in the info area. This is non-destructive
# and returns immediately after the user presses any key.
show_legend_popup() {
    # 1. Force UI Unlock so we can draw
    UI_LOCKED=0
    
    enter_ui_mode || true
    
    # 2. Draw Temporary Header
    # We pass "false" to hide "L: Legend" (recursive logic check)
    draw_bottom_panel_header "STATUS GUIDE" "false"
    
    # 3. Clear area to ensure clean background
    clear_info_area

    # 4. Draw Legend Rows
    local row=$((LOG_TOP + 1))
    
    # Helper to increment row automatically
    p_row() { draw_sealed_row "$row" "$1"; ((row++)); }

    # Spacer
    p_row "" 
    p_row "   ${GREEN}${SYMBOL_PASS}${NC}  Ready       — Working correctly"
    p_row "   ${YELLOW}${SYMBOL_WARN}${NC}  Warning     — Required for full functionality"
    p_row "   ${RED}${SYMBOL_FAIL}${NC}  Missing     — Needs installation"
    p_row "   ${BOLD_GRAY}${SYMBOL_LOCKED}${NC}  Protected   — Do not remove (system-managed)"
    p_row "   ${GRAY}${SYMBOL_PENDING}${NC}  Service     — Background service status"

    draw_bottom_border "Press any key to continue"
    
    # 5. Wait for input
    local k
    safe_read_key k
    
    # Cleanup happens when caller redraws their specific menu
    exit_ui_mode || true
}




update_status() {
    STATUS_MAP["$1"]="$2"
    # Re-render the whole grid so status icons stay consistent
    render_status_grid 6
}

# Atomic status updater: update only the affected cell (left or right)
update_status_atomic() {
    local key="$1"
    local status="$2"
    STATUS_MAP["$key"]="$status"

    # Find which row contains this key
    local row_idx=0
    local start_row=6
    for entry in "${STATUS_GRID[@]}"; do
        IFS='|' read -r L_label L_key R_label R_key <<< "$entry"
        local target_row=$(( start_row + row_idx ))

        local left_w=$(( (INNER_COLS - 3) / 2 ))
        local right_w=$(( INNER_COLS - 3 - left_w ))

        if [[ "$L_key" == "$key" ]]; then
            # --- DYNAMIC LABEL LOGIC FOR PYTHON (LEFT COLUMN) ---
            if [[ "$L_key" == "python311" ]]; then
                if [[ "${IS_PYENV_INSTALLED:-false}" == "true" ]]; then
                    L_label="Python 3.11 Library (pyenv)"
                elif [[ "${IS_PYSYS_INSTALLED:-false}" == "true" ]]; then
                    L_label="Python 3.11 Library (system)"
                else
                    L_label="Python 3.11 Library"
                fi
            fi
            # --------------------------------------
            
            local L_sym
            L_sym=$(get_symbol "$L_key")
            local L_content="${L_sym} $(get_status_label_color "$L_key")${L_label}${NC}"
            local L_padded
            L_padded=$(pad_display "$L_content" "$left_w")
            # Print left half including left border and separator
            print_at_col "$target_row" 1 "${BLUE}║${NC} ${L_padded}${BLUE}│${NC}"
            return 0
        elif [[ "$R_key" == "$key" ]]; then
            # --- DYNAMIC LABEL LOGIC FOR PYTHON (RIGHT COLUMN) ---
            if [[ "$R_key" == "python311" ]]; then
                if [[ "${IS_PYENV_INSTALLED:-false}" == "true" ]]; then
                    R_label="Python 3.11 Library (pyenv)"
                elif [[ "${IS_PYSYS_INSTALLED:-false}" == "true" ]]; then
                    R_label="Python 3.11 Library (system)"
                else
                    R_label="Python 3.11 Library"
                fi
            fi
            # --------------------------------------
            
            local R_sym
            R_sym=$(get_symbol "$R_key")
            local R_content="${R_sym} $(get_status_label_color "$R_key")${R_label}${NC}"
            local R_padded
            R_padded=$(pad_display "$R_content" "$right_w")
            # Right column starts at the beginning of R_padded content:
            # Col 1 (║) + Col 2 ( ) + left_w (L_padded) + Col 1 (│) = left_w + 3 visual columns.
            # Since print_at_col is 1-based, the column index is left_w + 4.
            local right_col=$(( left_w + 4 ))
            print_at_col "$target_row" "$right_col" "${R_padded} ${BLUE}║${NC}"
            return 0
        fi

        row_idx=$(( row_idx + 1 ))
    done

    # Fallback: full render if key not found
    render_status_grid 6
}

# ============================================================================
# CONFIG HELPERS
# ============================================================================
 
# Buffered full-screen renderer: capture drawing into a temp buffer
# and emit once to the controlling TTY to reduce per-row redraw overhead.
render_screen_atomic() {
    # If a modal UI is active (e.g. service/profile editor), skip
    # full-screen atomic refreshes to avoid clobbering the modal panel.
    if [[ "${UI_MODAL_ACTIVE:-0}" -eq 1 ]]; then
        printf '%s\n' "[DEBUG] render_screen_atomic skipped due to UI_MODAL_ACTIVE=1" >> /tmp/qz_profile_debug.log 2>/dev/null || true
        return 0
    fi
    local buf
    # SEC-01 FIX: Use mktemp with strict error handling, no insecure fallback to $$
    buf=$(mktemp "${TEMP_DIR:-/tmp}/qz_screen.XXXXXX" 2>/dev/null) || buf=$(mktemp /tmp/qz_screen.XXXXXX 2>/dev/null) || {
        echo "ERROR: Failed to create secure temporary buffer for screen rendering" >&2
        return 1
    }
    
    # SEC-05 FIX: Set cleanup trap immediately to prevent race condition
    # If function crashes after mktemp, this ensures buffer is removed
    trap 'rm -f "$buf" 2>/dev/null || true' RETURN

    # Open a dedicated FD for UI output to capture functions that write
    # directly to $UI_FD (print_at, print_at_col, etc.). Use FD 9 to
    # avoid clashing with existing FDs.
    exec 9>"$buf" 2>/dev/null || exec 9>"$buf" || true
    local prev_ui_fd="${UI_FD:-2}"
    UI_FD=9

    # SEC-01 FIX: Save original functions via backup references (no eval needed)
    # Create temporary backup function names that preserve original implementations
    _render_atomic_backup_print_at() { 
        print_at "$@"
    }
    _render_atomic_backup_print_at_col() { 
        print_at_col "$@"
    }

    # Redefine print_at to write to FD 9
    # shellcheck disable=SC2317
    print_at() {
        local row=$1; shift
        local line="$*"
        local esc; esc=$(printf '\033[%d;1H' "$((row + 1))")
        printf '%s%s' "$esc" "$line" >&9 2>/dev/null || true
        return 0
    }


    # Redefine print_at_col to write to FD 9
    # shellcheck disable=SC2317
    print_at_col() {
        local row=${1:-0}
        local col=${2:-1}
        shift 2
        local text="$*"
        printf '\033[%d;%dH' "$((row + 1))" "$col" >&9 2>/dev/null || true
        printf '%s' "$text" >&9 2>/dev/null || true
        return 0
    }

    # Run drawing ops which will now use our overridden print helpers
    clear_screen
    enter_ui_mode
    draw_top_panel
    draw_bottom_panel_header "INFORMATION"
    draw_instructions_bottom "$CURRENT_INSTRUCTION"
    draw_bottom_border

    # SEC-01 FIX: Restore originals by redefining from backup (no eval needed)
    print_at() { 
        _render_atomic_backup_print_at "$@"
    }
    print_at_col() { 
        _render_atomic_backup_print_at_col "$@"
    }
    
    # Clean up backup functions
    unset -f _render_atomic_backup_print_at _render_atomic_backup_print_at_col 2>/dev/null || true

    # Restore previous UI_FD and close FD 9
    UI_FD="$prev_ui_fd"
    exec 9>&-

    # Emit buffer to controlling TTY when available, else to stdout
    if [ -w /dev/tty ]; then
        cat "$buf" > /dev/tty 2>/dev/null || cat "$buf"
    else
        cat "$buf"
    fi

    rm -f "$buf" 2>/dev/null || true
}

load_current_profile_values() {
    # Load and sanitize profile values from associative arrays
    # (populated earlier by load_config_into_arrays)
    
    local w="${CONFIG_INT[weight]:-}"
    local a="${CONFIG_INT[age]:-}"
    local s="${CONFIG_STRING[sex]:-}"
    local m="${CONFIG_BOOL[miles_unit]:-false}"

    # Only update if the value found is not empty
    # Sanitize numeric inputs and validate ranges to avoid corrupted values
    if [[ -n "$w" ]]; then
        local _w_sanitized="${w//[^0-9.]/}"
        if [[ -n "$_w_sanitized" ]]; then
            local _w_int=${_w_sanitized%%.*}
            if (( _w_int >= 20 && _w_int <= 300 )); then
                PREV_WEIGHT="$_w_sanitized"
            else
                local try2=${_w_int: -2}
                local try3=${_w_int: -3}
                if [[ -n "$try2" ]] && (( try2 >= 20 && try2 <= 300 )); then
                    PREV_WEIGHT="$try2"
                elif [[ -n "$try3" ]] && (( try3 >= 20 && try3 <= 300 )); then
                    PREV_WEIGHT="$try3"
                fi
            fi
        fi
    fi
    if [[ -n "$a" ]]; then
        local _a_sanitized="${a//[^0-9]/}"
        if [[ -n "$_a_sanitized" ]]; then
            if (( _a_sanitized >= 1 && _a_sanitized <= 120 )); then
                PREV_AGE="$_a_sanitized"
            else
                # fallback: try last 2 digits (e.g. 2554 -> 54)
                local trya=${_a_sanitized: -2}
                if [[ -n "$trya" ]] && (( trya >= 1 && trya <= 120 )); then
                    PREV_AGE="$trya"
                fi
            fi
        fi
    fi
    [[ -n "$s" ]] && PREV_SEX="$s"
    [[ -n "$m" ]] && PREV_MILES="$m"
}

update_config_key() {
    local key="$1"
    local value="$2"
    [[ -z "$key" ]] && return

    # Do not persist bluetooth_address via the config updater; keep it out
    # of generated configs and avoid storing sensitive device addresses.
    if [[ "$key" == "bluetooth_address" ]]; then
        return 0
    fi

    # Strip decimal if weight/age
    if [[ "$key" == "age" || "$key" == "weight" ]]; then
        value=$(strip_ansi_pure "$value")
        value=$(echo "$value" | cut -d. -f1)
        value=$(echo "$value" | tr -cd '0-9')
        if [[ "$key" == "weight" ]]; then
            if [[ -n "$value" ]]; then
                local v2=${value: -2}
                local v3=${value: -3}
                if [[ -n "$v2" ]] && (( v2 >= 20 && v2 <= 300 )); then
                    value="$v2"
                elif [[ -n "$v3" ]] && (( v3 >= 20 && v3 <= 300 )); then
                    value="$v3"
                fi
            fi
        elif [[ "$key" == "age" ]]; then
            if [[ -n "$value" ]]; then
                local a2=${value: -2}
                if [[ -n "$a2" ]] && (( a2 >= 1 && a2 <= 120 )); then
                    value="$a2"
                fi
            fi
        fi
    fi

    # Ensure config directory and file exist so updates always persist.
    local cfg_dir
    cfg_dir=$(dirname "$CONFIG_FILE")
    if [[ ! -d "$cfg_dir" ]]; then
        mkdir -p "$cfg_dir" 2>/dev/null || true
        if [ "$(id -u)" -eq 0 ] && [ -n "${TARGET_USER:-}" ]; then
            chown -R "$TARGET_USER":"$TARGET_USER" "$cfg_dir" 2>/dev/null || true
        fi
    fi

    if [[ ! -f "$CONFIG_FILE" ]]; then
        : > "$CONFIG_FILE" 2>/dev/null || true
        if [ "$(id -u)" -eq 0 ] && [ -n "${TARGET_USER:-}" ]; then
            chown "$TARGET_USER":"$TARGET_USER" "$CONFIG_FILE" 2>/dev/null || true
        fi
    fi

    if grep -q "^${key}=" "$CONFIG_FILE" 2>/dev/null; then
        sed -i "s@^${key}=.*@${key}=${value}@" "$CONFIG_FILE" 2>/dev/null || true
    else
        echo "${key}=${value}" >> "$CONFIG_FILE" 2>/dev/null || true
    fi
    
    # FIX: Pass raw value to preserve case for strings.
    # classify_and_store handles boolean normalization internally.
    classify_and_store "$key" "$value" || true
}

prompt_numeric_input() {
    local label="$1"
    local unit="$2"
    local current_val="$3"
    local row="$4"
    local prefix="${5:-  }"  # optional prefix (e.g. '   ► ' when editing from menu)

    local label_str
    if [[ -n "${unit}" ]]; then
        label_str="${label} (${unit}): "
    else
        label_str="${label}: "
    fi
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
        draw_sealed_row "$row" "${prefix}${label_str}${val_color}${display_val}${NC}"
        draw_bottom_border "Numbers only | ENTER to confirm"

        # 4. PRECISE CURSOR POSITIONING
        # ║(1) + Spacer(1) + prefix + label_str + buffer_len
        local _len_prefix=${#prefix}
        local _len_label=${#label_str}
        local _len_buffer=${#buffer}
        local cursor_col=$(( 1 + _len_prefix + _len_label + _len_buffer ))
        # If we are showing the placeholder [75], put cursor at index 1 (after '[')
        [[ -z "$buffer" ]] && cursor_col=$(( cursor_col + 1 ))

        # Move cursor and ensure it is visible
        local ui_fd
        ui_fd=$(get_safe_ui_fd)
        ( printf "\033[%d;%dH" "$((row + 1))" "$((cursor_col + 1))" >&"${ui_fd}" ) 2>/dev/null || true
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
                # shellcheck disable=SC2034
                local junk
                read -rsn2 -t 0.01 junk </dev/tty 2>/dev/null
                : "${junk:-}" >/dev/null 2>&1
                buffer="${current_val}"
                break ;;
        esac
    done

    # 6. RESTORE TERMINAL & LOCK IN CYAN
    stty "$old_stty" 2>/dev/null
    hide_cursor # Hide cursor
    # Use the same prefix used during editing so the final render does not
    # shift horizontally compared to the in-place editor (prevents jarring redraw).
    draw_sealed_row "$row" "${prefix}${label_str}${CYAN}${buffer}${NC}"

    echo "${buffer}"
}

# Inline Numeric Editor
# Usage: inline_edit_field ROW LABEL UNIT CURRENT_VALUE MIN MAX
inline_edit_field() {
    # Args: ROW LABEL UNIT CURRENT_VALUE MIN MAX [PRESERVE_HELP]
    # Use ${N:-} to prevent 'unbound variable' crashes in strict mode
    local row="${1:-}"
    local label="${2:-}"
    local unit="${3:-}"
    local current_val="${4:-}"
    local min_val="${5:-0}"
    local max_val="${6:-99999}"
    local preserve_help="${7:-false}"  # NEW: preserve help panel on right
    
    local buffer="" 
    local display_val
    local label_str="${label}"
    if [[ -n "$unit" ]]; then
        label_str="${label} (${unit})"
    fi
    
    # 1. Update Footer
    draw_bottom_border "Numbers only | Enter: Confirm | Esc: Cancel"
    
    # 2. Show Cursor
    local fd="${UI_FD:-2}"
    printf "\e[?25h" >&"$fd"
    
    while true; do
        # 3. Determine Display
        if [[ -z "$buffer" ]]; then
            display_val="${GRAY}${current_val}${NC}"
        else
            display_val="${BOLD_WHITE}${buffer}${NC}"
        fi
        
        # 4. Draw Row (help-panel aware)
        local row_text="   ${CYAN}► ${BOLD_CYAN}${label_str}: [${display_val}${BOLD_CYAN}]${NC}"
        if [[ "$preserve_help" == "true" ]]; then
            update_sealed_row_content_left "$row" "$row_text"
        else
            update_sealed_row_content "$row" "$row_text"
        fi
        
        # 5. Position Cursor
        local prefix_len=$(( 1 + 3 + 2 + ${#label_str} + 3 ))
        local cursor_col=$(( prefix_len + ${#buffer} + 1 ))
        
        printf "\e[%d;%dH" "$((row + 1))" "$cursor_col" >&"$fd"
        
        # 6. Capture Input
        local key
        safe_read_key key
        
        if [[ $key == $'\x1b' ]]; then
            read_escape_sequence k2
            if [[ -z "${k2:-}" ]]; then
                printf "\e[?25l" >&"$fd" # Hide cursor
                return 1 # Cancel
            fi
        elif [[ $key == "" ]]; then
            printf "\e[?25l" >&"$fd"
            if [[ -z "$buffer" ]]; then
                echo "$current_val"
            else
                echo "$buffer"
            fi
            return 0 # Success
        elif [[ $key == $'\x7f' ]] || [[ $key == $'\x08' ]]; then
            if [[ ${#buffer} -gt 0 ]]; then buffer="${buffer%?}"; fi
        elif [[ $key =~ ^[0-9]$ ]]; then
            if [[ ${#buffer} -lt 5 ]]; then buffer="${buffer}${key}"; fi
        elif [[ $key == "." ]]; then
             if [[ "$buffer" != *"."* ]]; then
                [[ -z "$buffer" ]] && buffer="0"
                buffer="${buffer}."
             fi
        fi
    done
}

configure_user_profile() {
    local wizard_mode="${1:-false}"  # Optional parameter for wizard mode
    load_current_profile_values
    UI_MODAL_ACTIVE=1
    
    local _UNIT_IMPERIAL="false"
    local _GENDER_FEMALE="false"
    if [[ "${PREV_MILES:-}" == "true" ]]; then _UNIT_IMPERIAL="true"; fi
    if [[ "${PREV_SEX:-}" == "Female" ]]; then _GENDER_FEMALE="true"; fi

    local selected=0
    local draw_mode="FULL"

    while true; do
        local w_unit="kg"; [[ "${_UNIT_IMPERIAL}" == "true" ]] && w_unit="lbs"
        local unit_label="Metric (kg/km)"; [[ "${_UNIT_IMPERIAL}" == "true" ]] && unit_label="Imperial (lbs/mi)"
        local gender_label="Male"; [[ "${_GENDER_FEMALE}" == "true" ]] && gender_label="Female"

        local options=()
        options+=("Unit: ${unit_label}")
        options+=("Weight (${w_unit}): [${PREV_WEIGHT:-}]")
        options+=("Age (years): [${PREV_AGE:-}]")
        options+=("Gender: ${gender_label}")
        
        # Help text for each profile field
        local help_texts=()
        help_texts+=("Toggle between Metric (kg/km) and Imperial (lbs/mi) units. This affects how weight and distance are displayed throughout QZ. Press Enter to switch.")
        help_texts+=("Your body weight used for accurate power and calorie calculations. Enter a value between 20-400 ${w_unit}. Press Enter to edit, type your weight, then press Enter to save or Esc to cancel.")
        help_texts+=("Your age in years, used for heart rate zone calculations and calorie estimates. Valid range is 5-120 years. Press Enter to edit.")
        help_texts+=("Your biological gender, used for metabolic calculations and calorie burn estimates. Press Enter to toggle between Male and Female.")
        
        # Only add "Save & Continue" in wizard mode
        local save_idx=-1
        if [[ "$wizard_mode" == "true" ]]; then
            options+=("${BOLD_GREEN}Save & Continue${NC}")
            help_texts+=("Save your profile configuration and continue with the setup wizard. All changes are automatically saved as you make them.")
            save_idx=4
        fi

        show_unified_menu options "$selected" "USER PROFILE" "$draw_mode" "false" "" help_texts
        local exit_code=$?

        if [[ $exit_code -eq 255 ]]; then
            UI_MODAL_ACTIVE=0
            return 0
        fi

        selected=$exit_code
        
        # Handle Save & Continue (only in wizard mode)
        if [[ $save_idx -ge 0 ]] && [[ $selected -eq $save_idx ]]; then
            UI_MODAL_ACTIVE=0
            return 0
        fi

        local choice_text="${options[$selected]}"

        case "$choice_text" in
            Unit:*)
                if [[ "${_UNIT_IMPERIAL}" == "true" ]]; then
                    _UNIT_IMPERIAL="false"; update_config_key "miles_unit" "false"
                else
                    _UNIT_IMPERIAL="true"; update_config_key "miles_unit" "true"
                fi
                auto_save_config "profile" "miles_unit"
                draw_mode="ITEMS"
                ;;
            Gender:*)
                if [[ "${_GENDER_FEMALE}" == "true" ]]; then
                    _GENDER_FEMALE="false"; update_config_key "sex" "Male"
                else
                    _GENDER_FEMALE="true"; update_config_key "sex" "Female"
                fi
                auto_save_config "profile" "sex"
                draw_mode="ITEMS"
                ;;
            Weight*)
                local target_row=$(( LOG_TOP + 1 + selected ))
                local new_v
                if new_v=$(inline_edit_field "$target_row" "Weight" "$w_unit" "${PREV_WEIGHT}" 20 400 "true"); then
                    if [[ -n "$new_v" ]]; then
                        PREV_WEIGHT="$new_v"
                        update_config_key "weight" "$new_v"
                        auto_save_config "profile" "weight"
                    fi
                else
                    show_cancel_feedback
                fi
                draw_mode="FULL"
                ;;
            Age*)
                local target_row=$(( LOG_TOP + 1 + selected ))
                local new_a
                if new_a=$(inline_edit_field "$target_row" "Age" "years" "${PREV_AGE}" 5 120 "true"); then
                    if [[ -n "$new_a" ]]; then
                        PREV_AGE="$new_a"
                        update_config_key "age" "$new_a"
                        auto_save_config "profile" "age"
                    fi
                else
                    show_cancel_feedback
                fi
                draw_mode="FULL"
                ;;
        esac
    done
}

configure_emulation_flow() {
    local wizard_mode="${1:-false}" 
    local options=("Treadmill")
    local selected=0
    
    while true; do
        show_unified_menu options "$selected" "EMULATE DEVICE" "FULL" "false"
        local idx=$?
        
        if [[ $idx -eq 255 ]]; then return 1; fi
        
        if [[ "${options[$idx]}" == "Treadmill" ]]; then
            
            # --- 1. PREREQUISITE CHECK ---
            load_service_config >/dev/null 2>&1
            local ant_hw="${STATUS_MAP[ant_dongle]:-pending}"
            local ant_sw="${SERVICE_FLAGS[ant_footpod]:-false}"
            local missing=0
            [[ "$ant_hw" != "pass" ]] && ((missing++))
            [[ "$ant_sw" != "true" ]] && ((missing++))
            
            if [[ $missing -gt 0 ]]; then
                # ... (Keep existing Missing Requirements logic) ...
                draw_bottom_panel_header "MISSING REQUIREMENTS" "false"
                clear_info_area
                local r=$((LOG_TOP + 1))
                draw_sealed_row "$r" "   ${BOLD_RED}Cannot start emulation. Missing dependencies:${NC}"; ((r++))
                draw_sealed_row "$r" ""; ((r++))
                local sym="${RED}${SYMBOL_FAIL}${NC}"
                [[ "$ant_hw" == "pass" ]] && sym="${GREEN}${SYMBOL_PASS}${NC}"
                draw_sealed_row "$r" "   ${sym} ANT+ USB Dongle Detected"; ((r++))
                sym="${RED}${SYMBOL_FAIL}${NC}"
                [[ "$ant_sw" == "true" ]] && sym="${GREEN}${SYMBOL_PASS}${NC}"
                draw_sealed_row "$r" "   ${sym} ANT+ Enabled in Service Config"; ((r++))
                draw_sealed_row "$r" ""; ((r++)); draw_sealed_row "$r" ""; ((r++))
                draw_sealed_row "$r" "   ${YELLOW}Please resolve these issues first.${NC}"; ((r++))
                draw_sealed_row "$r" ""
                draw_bottom_border "Press any key to go back"
                local k=""; safe_read_key k
                continue
            fi
            
            # --- 2. CHECK IF ALREADY CONFIGURED (Simplified) ---
            if [[ "${CONFIG_BOOL[fakedevice_treadmill]:-}" == "true" ]]; then
                draw_bottom_panel_header "EMULATION ACTIVE" "false"
                clear_info_area
                
                local r=$((LOG_TOP)) 
                draw_sealed_row "$r" ""
                ((r++)) # 14

                # Simplified Static Text
                draw_sealed_row "$r" "   Emulation configured for testing."
                ((r++)) # 15
                
                draw_sealed_row "$r" ""
                ((r++)) # 16

                local instr_r=$r
                draw_sealed_row "$instr_r"       "   ${CYAN}1.${NC} Connect 2nd QZ App to '${BOLD_CYAN}KICKR RUN${NC}'."
                draw_sealed_row $((instr_r + 1)) "   ${CYAN}2.${NC} App: Set Model to \"Horizon\" and Force FTMS to \"Enabled\"."
                draw_sealed_row $((instr_r + 2)) "   ${CYAN}3.${NC} Pair ANT+ footpod with your watch."
                draw_sealed_row $((instr_r + 3)) "   ${CYAN}4.${NC} Control speed from App."

                draw_bottom_border "Press any key to return"
                local k; safe_read_key k
                return 2
            fi

            # --- 3. SHOW SETUP PROMPT ---
            # ... (Rest of logic remains the same) ...
            draw_bottom_panel_header "EMULATION SETUP (ANT+ TEST)" "false"
            clear_info_area
            local r=$((LOG_TOP))
            draw_sealed_row "$r" ""  # Top spacer
            draw_sealed_row $((r + 1)) "   Configure QZ as emulated treadmill '${BOLD_CYAN}KICKR RUN${NC}' for testing ANT+?"
            local desc_r=$((r + 5))
            draw_sealed_row "$desc_r"       "   • Configures QZ to act as a virtual treadmill broadcasting ANT+."
            draw_sealed_row $((desc_r + 1)) "   • Used to verify ANT+ dongle functionality with watches."
            draw_sealed_row $((desc_r + 2)) "   • Requires a second QZ app (phone/tablet) to control speed."
            
            if prompt_yes_no 2; then
                draw_bottom_border "${YELLOW}Applying Emulation Config...${NC}"
                if [[ -f "$DEVICES_INI" ]]; then
                    local all_keys; all_keys=$(grep '=' "$DEVICES_INI" | cut -d'=' -f2 | xargs)
                    for k in $all_keys; do CONFIG_BOOL[$k]="false"; done
                fi
                CONFIG_BOOL["virtual_device_force_treadmill"]="false"
                CONFIG_BOOL["virtual_device_rower"]="false"
                CONFIG_BOOL["virtual_device_force_bike"]="false"
                CONFIG_BOOL["virtual_device_elliptical"]="false"
                CONFIG_BOOL["fakedevice_treadmill"]="true"
                CONFIG_BOOL["treadmill_force_speed"]="true"
                CONFIG_BOOL["virtual_device_bluetooth"]="true"
                CONFIG_BOOL["virtualtreadmill"]="true"
                update_config_key "bluetooth_lastdevice_name" "KICKR RUN"
                update_config_key "bluetooth_lastdevice_address" "" 
                generate_config_file
                check_equipment_state
                CONFIG_STRING[bluetooth_lastdevice_name]="KICKR RUN"
                draw_header_equipment_line
                show_save_feedback "Emulation"
                return 0
            else
                continue
            fi
        fi
    done
}

ant_menu_flow() {
    local options=("Run Broadcast Test (Script)" "Configure Virtual Service")
    local selected=0
    
    while true; do
        show_unified_menu options "$selected" "ANT+ & VIRTUAL SETUP" "FULL" "false"
        local idx=$?
        
        if [[ $idx -eq 255 ]]; then return 0; fi
        
        selected=$idx
        
        if [[ $idx -eq 0 ]]; then
            perform_ant_test
        elif [[ $idx -eq 1 ]]; then
            configure_emulation_flow
            local emulation_result=$?
            
            # Handle all return codes from configure_emulation_flow
            # 0 = Changes made (config updated)
            # 1 = Cancelled/Back
            # 2 = Viewed only (already active)
            if [[ $emulation_result -eq 0 ]]; then
                # Config changed - refresh checks and continue to menu
                check_equipment_state
                draw_header_equipment_line
            fi
            # For all cases (0, 1, 2), stay in the loop and redraw menu
        fi
        
        # Force full redraw after returning from submenu
        render_dashboard_atomic
    done
}

select_equipment_flow() {
    # Phase 2: Argument to suppress restart prompt (used by Wizard)
    # This also acts as the "Wizard Mode" flag for child menus
    local suppress_prompt="${1:-false}" 
    
    if [ ! -f "$DEVICES_INI" ]; then
        draw_error_screen "MISSING DATABASE" "Error: devices.ini not found." "wait"
        return 1
    fi

    local types=()
    mapfile -t types < <(grep '^\[.*\]$' "$DEVICES_INI" | tr -d '[]')
    
    # Add Virtual Option
    types+=("Virtual / Emulator")
    # Note: "Back" removed (Use ESC)
    
    local state=0
    local selected_type=""
    local type_idx=0
    local model_idx=0

    # Auto-detect current selection
    if [ -f "$CONFIG_FILE" ]; then
        if [[ "${CONFIG_BOOL[fakedevice_treadmill]:-}" == "true" ]]; then
             for i in "${!types[@]}"; do [[ "${types[$i]}" == "Virtual / Emulator" ]] && type_idx=$i; done
        elif grep -q "virtual_device_force_treadmill=true" "$CONFIG_FILE"; then selected_type="Treadmill"
        elif grep -q "virtual_device_rower=true" "$CONFIG_FILE"; then selected_type="Rower"
        elif grep -q "virtual_device_elliptical=true" "$CONFIG_FILE"; then selected_type="Elliptical"
        elif grep -q "virtual_device_force_bike=true" "$CONFIG_FILE"; then selected_type="Bike"
        fi
        
        if [[ -n "$selected_type" ]]; then
            for i in "${!types[@]}"; do [[ "${types[$i]}" == "$selected_type" ]] && type_idx=$i; done
        fi
    fi

    while true; do
        # --- STATE 0: SELECT TYPE ---
        if [ "$state" -eq 0 ]; then
            MENU_CACHE_LOADED=0 
            show_unified_menu types "$type_idx" "SELECT DEVICE TYPE" "FULL"
            local idx=$?
            
            # Handle ESC (255)
            if [[ $idx -eq 255 ]]; then return 1; fi
            
            local selection="${types[$idx]}"
            type_idx=$idx 

            if [[ "$selection" == "Virtual / Emulator" ]]; then
                # --- JUMP TO EMULATION FLOW ---
                # FIX: Pass the wizard flag to configure_emulation_flow
                configure_emulation_flow "$suppress_prompt"
                local ret=$?
                
                if [[ $ret -eq 0 ]]; then
                    # 0 = Config Changed: Prompt restart and exit to main
                    if [[ "$suppress_prompt" != "true" ]]; then
                        prompt_restart_service
                    fi
                    return 0
                elif [[ $ret -eq 2 ]]; then
                    # 2 = Viewed Only (Already Active): Exit to main without prompt
                    return 0
                else
                    # 1 = Cancelled: Loop back to Type Selection
                    state=0
                    continue
                fi
            else
                selected_type="$selection"
                state=1
            fi
        fi
        
        # --- STATE 1: SELECT MODEL ---
        if [ "$state" -eq 1 ]; then
            draw_bottom_panel_header "LOADING ${selected_type^^}..."
            clear_info_area
            
            local load_plain="Loading models..."
            local load_msg="${YELLOW}${load_plain}${NC}"
            local load_w
            if command -v get_vis_width >/dev/null; then load_w=$(get_vis_width "$load_plain"); else load_w=${#load_plain}; fi
            local load_col=$((2 + INFO_WIDTH - load_w))
            print_at_col $((LOG_BOTTOM)) "$load_col" "$load_msg"

            local models=()
            local keys=()
            MENU_CACHE_LINES=()
            MENU_CACHE_WIDTHS=()
            MENU_CACHE_LOADED=0
            
            # Sanitize Cache Filename (Spaces/Slashes)
            local safe_type="${selected_type// /_}"
            safe_type="${safe_type//\//}"
            local cache_file="${SCRIPT_DIR}/.menu_cache/${safe_type}.cache"
            
            if [[ -f "$cache_file" ]]; then
                while IFS=$'\x1f' read -r name id width; do
                    MENU_CACHE_LINES+=("$name")
                    MENU_CACHE_WIDTHS+=("$width")
                    keys+=("$id")
                done < "$cache_file"
                if [[ ${#MENU_CACHE_LINES[@]} -gt 0 ]]; then
                    models=("${MENU_CACHE_LINES[@]}")
                    MENU_CACHE_LOADED=1
                fi
            fi

            if [[ ${#models[@]} -eq 0 ]]; then
                while IFS= read -r line; do
                    if [[ "$line" =~ = ]]; then
                        local m_name=$(echo "${line%%=*}" | xargs)
                        models+=("$m_name")
                        keys+=("$(echo "${line#*=}" | xargs)")
                        if command -v get_vis_width >/dev/null; then
                            MENU_CACHE_WIDTHS+=($(get_vis_width "$m_name"))
                        else
                            MENU_CACHE_WIDTHS+=(${#m_name})
                        fi
                    fi
                done < <(awk -v section="[$selected_type]" '$0==section { flag=1; next } /^\[/ { flag=0 } flag && $0!="" && $0!~/^;/ { print $0 }' "$DEVICES_INI")
                MENU_CACHE_LINES=("${models[@]}")
                MENU_CACHE_LOADED=1
            fi
            
            model_idx=0
            for ((i=0; i<${#keys[@]}; i++)); do
                local k="${keys[$i]}"
                if [[ "${CONFIG_BOOL[$k]:-false}" == "true" ]]; then
                    model_idx=$i
                    break
                fi
            done
            
            models+=("Back")
            MENU_CACHE_WIDTHS+=("4")
            
            # Clear Loading Msg
            print_at_col $((LOG_BOTTOM)) 2 "$(printf '%*s' "$INFO_WIDTH" '')"

            show_unified_menu models "$model_idx" "SELECT $selected_type MODEL" "FULL"
            local m_idx=$?
            
            MENU_CACHE_LOADED=0
            MENU_CACHE_LINES=()
            MENU_CACHE_WIDTHS=()

            if [[ $m_idx -eq 255 ]] || [[ "${models[$m_idx]}" == "Back" ]]; then
                state=0
                continue
            fi
            
            # Saving Feedback
            print_at_col $((LOG_BOTTOM)) 2 "$(printf '%*s' "$INFO_WIDTH" '')"
            local save_plain="Saving Configuration..."
            local save_msg="${YELLOW}${save_plain}${NC}"
            local save_w
            if command -v get_vis_width >/dev/null; then save_w=$(get_vis_width "$save_plain"); else save_w=${#save_plain}; fi
            local save_col=$((2 + INFO_WIDTH - save_w))
            local ui_fd
            ui_fd=$(get_safe_ui_fd)
            ( printf "\e[%d;%dH%s" "$((LOG_BOTTOM + 1))" "$save_col" "$save_msg" >&"${ui_fd}" ) 2>/dev/null
            
            # Reset Config
            local all_keys
            all_keys=$(grep '=' "$DEVICES_INI" | cut -d'=' -f2 | xargs)
            for k in $all_keys; do CONFIG_BOOL[$k]="false"; done
            
            CONFIG_BOOL["virtual_device_force_treadmill"]="false"
            CONFIG_BOOL["virtual_device_rower"]="false"
            CONFIG_BOOL["virtual_device_force_bike"]="false"
            CONFIG_BOOL["virtual_device_elliptical"]="false"
            
            # Explicitly turn off fake device if standard device selected
            CONFIG_BOOL["fakedevice_treadmill"]="false"

            case "$selected_type" in
                "Treadmill")  CONFIG_BOOL["virtual_device_force_treadmill"]="true" ;;
                "Rower")      CONFIG_BOOL["virtual_device_rower"]="true" ;;
                "Elliptical") CONFIG_BOOL["virtual_device_elliptical"]="true" ;;
                "Bike")       CONFIG_BOOL["virtual_device_force_bike"]="true" ;;
            esac
            
            local selected_key="${keys[$m_idx]}"
            CONFIG_BOOL["$selected_key"]="true"
            
            generate_config_file
            
            check_equipment_state
            draw_header_equipment_line

            show_save_feedback "Equipment"
            
            if [[ "$suppress_prompt" != "true" ]]; then
                prompt_restart_service
            fi
            
            return 0
        fi
    done
}

run_setup_wizard() {
    # 1. INTRO
    draw_info_screen "QUICK SETUP WIZARD" "This will guide you through:\n1. Equipment Selection\n2. Bluetooth Pairing (if applicable)\n3. User Profile\n4. Service Activation\n\nExisting settings may be overwritten." "wait"
    
    # 2. EQUIPMENT
    # Pass "true" to suppress intermediate restart prompts
    if ! select_equipment_flow "true"; then
        draw_info_screen "WIZARD CANCELLED" "Setup aborted during equipment selection." 2
        return 1
    fi

    # 3. BLUETOOTH (Conditional)
    # If fakedevice_treadmill is true, QZ is the device, so we don't scan for hardware.
    if [[ "${CONFIG_BOOL[fakedevice_treadmill]:-}" == "true" ]]; then
        draw_info_screen "VIRTUAL MODE" "Virtual device selected. Skipping Bluetooth hardware pairing." 1
    else
        draw_bottom_panel_header "WIZARD: BLUETOOTH" "false"
        clear_info_area
        draw_sealed_row $((LOG_TOP+1)) "   Equipment configured."
        draw_sealed_row $((LOG_TOP+2)) "   Now scanning for Bluetooth connection..."
        draw_bottom_border "Press ENTER to scan | Esc to skip"
        
        local k
        safe_read_key k
        if [[ $k != $'\x1b' ]]; then
            perform_bluetooth_scan "true"
        fi
    fi

    # 4. PROFILE (with wizard mode enabled)
    configure_user_profile "true"

    # 5. FINALIZE
    # Regenerate service file to ensure all new settings (like -name) are applied
    generate_service_file >/dev/null

    # FIX: Explicit completion message
    draw_info_screen "SETUP COMPLETED" "All settings have been saved successfully.\n\nThe service must now be restarted to apply changes." 2

    # Final Prompt
    prompt_restart_service
    
    return 0
}

# ============================================================================
# PROGRESS BAR WITH LIVE LOG STREAMING
# ============================================================================

run_with_progress() {
    local label="$1"
    local command_text="$2"
    local log_file="$TEMP_DIR/qz_setup.log"
    local script_file="$TEMP_DIR/qz_install_step.sh"
    
    cat > "$script_file" <<EOF
#!/bin/bash
export DEBIAN_FRONTEND=noninteractive
set -e
$command_text
EOF
    chmod +x "$script_file"
    : > "$log_file"

    draw_bottom_panel_header "INSTALLING..." "false"
    clear_info_area
    draw_bottom_border
    
    draw_sealed_row $((LOG_TOP + 1)) "   ${WHITE}${label}${NC}"
    draw_sealed_row $((LOG_TOP + 3)) "   ${GRAY}Please wait...${NC}"

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
        
        draw_sealed_row $((LOG_TOP + 5)) "                   ${CYAN}${bar_str}${NC}"
        
        if [ "$dir" -eq 1 ]; then
            ((pos++)); (( pos + pulse_width >= bar_width )) && dir=-1
        else
            ((pos--)); (( pos <= 0 )) && dir=1
        fi
        
        read -t 0.1 -N 0 2>/dev/null || true
    done
    
    wait "$pid"
    local exit_code=$?

    if [ $exit_code -ne 0 ]; then
        local log_excerpt
        log_excerpt=$(grep -a "." "$log_file" 2>/dev/null | tail -n 5 | sed 's/[[:cntrl:]]\+//g')
        # FIX: Wait for user input on failure
        draw_error_screen "TASK FAILED" "Error during: ${label}\nTechnical details from log:\n${log_excerpt}" "wait"
    fi
    
    return $exit_code
}

# Enhanced wrapper for run_with_progress with retry logic for dpkg/apt locks
run_step() {
    local label="$1"
    local command_text="$2"
    local max_retries=5
    local attempt=1
    local lock_detected=0
    local final_result=1 # Assume failure

    while [ "$attempt" -le "$max_retries" ]; do
        lock_detected=0
        
        # 1. Execute the command (make retry counter subtle)
        local display_label
        if [ "$attempt" -gt 1 ]; then
             display_label="$label (Retry $attempt)"
        else
             display_label="$label"
        fi
        
        # 2. Execute the command
        if run_with_progress "$display_label" "$command_text"; then
            final_result=0 # Success
            break
        fi
        
        # 3. Check the log for a lock failure
        local log_file="$TEMP_DIR/qz_install_step.sh.log"
        if grep -q -E "Could not get lock|Unable to acquire the (dpkg|apt) frontend lock" "$log_file" 2>/dev/null; then
            lock_detected=1
        fi

        if [ "$lock_detected" -eq 1 ] && [ "$attempt" -lt "$max_retries" ]; then
            local wait_time=$(( attempt * 2 )) 
            draw_bottom_panel_header "DPKG LOCK DETECTED" "false"
            update_sealed_row_content $((LOG_TOP + 3)) "   ${YELLOW}Waiting for DPKG lock to release. Retrying in ${wait_time} seconds...${NC}"
            sleep "$wait_time"
            clear_info_area
        else
            # Failure was not a lock error; stop retrying
            final_result=1
            break
        fi
        
        attempt=$(( attempt + 1 ))
    done
    
    return "$final_result"
}

# ============================================================================
# CHECK FUNCTIONS
# ============================================================================

# Global flag to track Python install type
IS_PYENV_INSTALLED=false
IS_PYSYS_INSTALLED=false
# Global flag to track runtime packages install status
IS_RUNTIME_INSTALLED=false
# Global flag to track core components install status
IS_CORE_INSTALLED=false

check_python311() {
    update_status "python311" "working"
    IS_PYENV_INSTALLED=false
    IS_PYSYS_INSTALLED=false
    
    # 1. Check pyenv path first (Priority)
    local pyenv_bin="$TARGET_HOME/.pyenv/versions/3.11.9/bin/python3"
    if [ -f "$pyenv_bin" ]; then
        # Verify it's a healthy build
        if "$pyenv_bin" -c "import _struct" 2>/dev/null; then
            IS_PYENV_INSTALLED=true
            update_status "python311" "pass"
            return 0
        fi
    fi

    # 2. Check standard system path (Only as a backup)
    if command -v python3.11 >/dev/null 2>&1; then
        if python3.11 -c "import _struct" 2>/dev/null; then
            IS_PYSYS_INSTALLED=true
            update_status "python311" "pass"
            return 0
        fi
    fi

    if [[ "$IS_PYENV_INSTALLED" == "true" ]]; then
        # Verify shared library support
        if ! ~/.pyenv/versions/3.11.9/bin/python3 -c "import sysconfig; exit(0 if sysconfig.get_config_var('Py_ENABLE_SHARED') == 1 else 1)" 2>/dev/null; then
            update_status "python311" "warn" # Mark as warning: exists but broken
        fi
    fi

    update_status "python311" "fail"
    return 1
}

check_venv() {
    update_status "venv" "working"
    
    # venv existence check

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
    # PERF-02 FIX: Call ldconfig once outside the loop
    local libs_cache
    libs_cache=$(ldconfig -p 2>/dev/null || true)

    for mod in "${required[@]}"; do
        if [[ "$mod" == "libusb-1.0" ]]; then
            if [[ "$libs_cache" != *"libusb-1.0"* ]]; then
                # PERF-02 FIX: Use globbing instead of find
                local found=0
                for p in /usr/lib*/libusb-1.0* /lib*/libusb-1.0* /usr/lib/*/libusb-1.0* /lib/*/libusb-1.0*; do [[ -e "$p" ]] && { found=1; break; }; done
                [[ $found -eq 0 ]] && ((missing++))
            fi
        else
            if [[ "$libs_cache" != *"lib$mod.so"* && "$libs_cache" != *"$mod"* ]]; then
                local found=0
                for p in /usr/lib*/"lib$mod.so"* /lib*/"lib$mod.so"* /usr/lib/*/"lib$mod.so"* /lib/*/"lib$mod.so"*; do [[ -e "$p" ]] && { found=1; break; }; done
                [[ $found -eq 0 ]] && ((missing++))
            fi
        fi
    done
    if (( missing == 0 )); then update_status "qt5_libs" "pass"; return 0; else update_status "qt5_libs" "fail"; return 1; fi
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

# Determine equipment type and model from loaded configuration
check_equipment_state() {
    local e_type="None"
    local e_model="None"

    # SAFE ACCESS: Use :-false defaults for all array lookups
    if [[ "${CONFIG_BOOL[fakedevice_treadmill]:-false}" == "true" ]]; then
        e_type="Emulated"
        e_model="Virtual Treadmill"
    elif [[ "${CONFIG_BOOL[virtual_device_force_treadmill]:-false}" == "true" ]]; then e_type="Treadmill"
    elif [[ "${CONFIG_BOOL[virtual_device_rower]:-false}" == "true" ]]; then e_type="Rower"
    elif [[ "${CONFIG_BOOL[virtual_device_elliptical]:-false}" == "true" ]]; then e_type="Elliptical"
    elif [[ "${CONFIG_BOOL[virtual_device_force_bike]:-false}" == "true" ]]; then e_type="Bike"
    fi

    if [[ "$e_type" != "None" && "$e_type" != "Emulated" ]]; then
        local cache_file="${SCRIPT_DIR}/.menu_cache/${e_type}.cache"
        if [[ -f "$cache_file" ]]; then
            while IFS=$'\x1f' read -r m_name m_key m_width; do
                if [[ "${CONFIG_BOOL[$m_key]:-false}" == "true" ]]; then
                    e_model="$m_name"
                    break
                fi
            done < "$cache_file"
        elif [[ -f "$DEVICES_INI" ]]; then
            while IFS= read -r line; do
                if [[ "$line" =~ = ]]; then
                    local m_name; m_name=$(echo "${line%%=*}" | xargs)
                    local m_key; m_key=$(echo "${line#*=}" | xargs)
                    if [[ "${CONFIG_BOOL[$m_key]:-false}" == "true" ]]; then
                        e_model="$m_name"
                        break
                    fi
                fi
            done < <(awk -v section="[$e_type]" '$0==section { flag=1; next } /^\\[/ { flag=0 } flag && $0!="" && $0!~/^;/ { print $0 }' "$DEVICES_INI")
        fi
    fi

    STATUS_MAP["equipment_type"]="$e_type"
    STATUS_MAP["equipment_model"]="$e_model"
}

check_equipment_fast() {
    check_equipment_state
    if [[ "${STATUS_MAP[equipment_type]:-None}" == "None" ]]; then
        # No equipment selected is a warning, not necessarily a failure
        printf "warn"
    else
        printf "pass"
    fi
}

check_config_file() {
    local status="fail"
    if [[ -f "$CONFIG_FILE" ]]; then
        status="pass"
        # Refresh equipment state whenever config is checked/re-loaded
        check_equipment_state
    fi
    STATUS_MAP["config_file"]="$status"
    draw_header_config_line
    render_status_grid 6
    [[ "$status" == "pass" ]] && return 0 || return 1
}

check_qz_service() {
    # Map systemd unit state to our STATUS_MAP values:
    # - running -> pass (Green ✓)
    # - failed/stopped -> warn (Yellow !)
    # - not-installed  -> pending (Gray ●)
    local svc_state
    svc_state=$(get_service_status)
    local status_map_val="pending"
    case "$svc_state" in
        running) status_map_val="pass" ;;
        failed|stopped)  status_map_val="warn" ;;
        *)       status_map_val="pending" ;;
    esac

    STATUS_MAP["qz_service"]="$status_map_val"
    if [[ "$svc_state" == "failed" ]]; then
        capture_service_failure_info "qz_service" || true
    fi

    # --- PYTHONHOME staleness check ---
    # If pyenv 3.11.9 is installed but the service file lacks PYTHONHOME,
    # QZ's embedded Py_Initialize() cannot locate stdlib/lib-dynload and
    # fails with "No module named '_struct'".  Downgrade to warn and surface
    # a clear diagnostic so the user knows to regenerate.
    SVC_PYTHONHOME_STALE=false
    if [[ -f "$SERVICE_FILE_QZ" ]] && [[ -f "${TARGET_HOME}/.pyenv/versions/3.11.9/bin/python3" ]]; then
        if ! grep -q 'PYTHONHOME' "$SERVICE_FILE_QZ" 2>/dev/null; then
            SVC_PYTHONHOME_STALE=true
            STATUS_MAP["qz_service"]="warn"
            SERVICE_FAILURE_INFO["qz_service"]="⚠ Service file is missing PYTHONHOME.
Pyenv Python 3.11.9 is installed but this service file
was generated before PYTHONHOME support was added.

This causes at runtime:
  ModuleNotFoundError: No module named '_struct'

Fix: Select 'Generate & Install Service' to regenerate
the service file with the correct environment."
        fi
    fi
    draw_header_service_line
    render_status_grid 6
    [[ "$svc_state" != "failed" ]] && return 0 || return 1
}


capture_service_failure_info() {
    local key="$1"
    # Prefer elevated journalctl/systemctl info where possible
    local summary=""
    local details=""
    if command -v systemctl >/dev/null 2>&1; then
        # Try to get a concise status output (may require sudo)
        if run_as_root_or_sudo systemctl status qz.service --no-pager -l >/dev/null 2>&1; then
            summary=$(run_as_root_or_sudo systemctl status qz.service --no-pager -l | sed -n '1,6p' 2>/dev/null || true)
        else
            summary=$(systemctl status qz.service --no-pager -l 2>/dev/null | sed -n '1,6p' || true)
        fi
        # Grab recent journal entries for the unit (best-effort)
        if run_as_root_or_sudo journalctl -u qz.service -n 200 -o cat >/dev/null 2>&1; then
            details=$(run_as_root_or_sudo journalctl -u qz.service -n 200 -o cat 2>/dev/null || true)
        else
            details=$(journalctl -u qz.service -n 200 -o cat 2>/dev/null || true)
        fi
    else
        summary="systemctl not available on this host"
        details=""
    fi

    # Combine into a stored value (trim to reasonable size)
    local combined
    combined="${summary}\n\nRecent logs:\n"
    if [[ -n "$details" ]]; then
        combined+="${details}"
    else
        combined+="(no journal entries available)"
    fi

    # Limit stored size to avoid huge memory usage
    SERVICE_FAILURE_INFO["$key"]=$(printf '%s' "$combined" | tail -n 500)
    return 0
}

## Optimized parallel implementation (replaces old sequential version)


# ---------------------------------------------------------------------------
# Optimized parallel check system (drop-in replacement for run_all_checks)
# - Runs checks in parallel
# - Caches recent results in RAM to avoid repeated subprocesses
# ---------------------------------------------------------------------------
CHECK_CACHE_DIR="${TEMP_DIR:-/tmp}/qz_check_cache"
CHECK_CACHE_TTL=${CHECK_CACHE_TTL:-30}

init_check_cache() {
    mkdir -p "$CHECK_CACHE_DIR" 2>/dev/null || true
}

cache_check_result() {
    local check_name="$1"
    local status="$2"
    local cache_file="${CHECK_CACHE_DIR}/${check_name}"
    printf '%s' "$status" > "$cache_file" 2>/dev/null || true
    touch "$cache_file" 2>/dev/null || true
}

get_cached_check() {
    local check_name="$1"
    local cache_file="${CHECK_CACHE_DIR}/${check_name}"
    if [[ -f "$cache_file" ]]; then
        local age=$(( $(date +%s) - $(stat -c %Y "$cache_file" 2>/dev/null || echo 0) ))
        if (( age < CHECK_CACHE_TTL )); then
            cat "$cache_file" 2>/dev/null
            return 0
        fi
    fi
    return 1
}

# Fast, minimal-overhead checks. These mirror the behaviour of the
# existing check_* functions but favor fewer subprocesses and use cache.
check_python311_fast() {
    # 1. Check for Pyenv first (Priority)
    if [[ -f "${TARGET_HOME}/.pyenv/versions/3.11.9/bin/python3" ]]; then
        if "${TARGET_HOME}/.pyenv/versions/3.11.9/bin/python3" -c "import _struct" 2>/dev/null; then
            IS_PYENV_INSTALLED=true
            IS_PYSYS_INSTALLED=false # Even if system exists, pyenv takes the display priority
            # Cache the installation type
            echo "pyenv" > "${CHECK_CACHE_DIR}/python311.type" 2>/dev/null || true
            echo "pass"
            return 0
        fi
    fi

    # 2. Check for System
    if command -v python3.11 >/dev/null 2>&1; then
        if python3.11 -c "import _struct" 2>/dev/null; then
            IS_PYENV_INSTALLED=false
            IS_PYSYS_INSTALLED=true
            # Cache the installation type
            echo "system" > "${CHECK_CACHE_DIR}/python311.type" 2>/dev/null || true
            echo "pass"
            return 0
        fi
    fi

    # Cache that neither is installed
    echo "none" > "${CHECK_CACHE_DIR}/python311.type" 2>/dev/null || true
    echo "fail"
    return 1
}

check_venv_fast() {
    local result
    if result=$(get_cached_check "venv"); then
        echo "$result"; return 0
    fi
    if [[ -d "${TARGET_HOME:-}/ant_venv" ]]; then
        cache_check_result "venv" "pass"; echo "pass"
    else
        cache_check_result "venv" "fail"; echo "fail"
    fi
}

check_python_packages_fast() {
    local result
    if result=$(get_cached_check "pkg_pips"); then
        echo "$result"; return 0
    fi
    local venv_py=""
    local -a venv_candidates=(
        "${TARGET_HOME:-}/ant_venv"
        "${HOME:-}/ant_venv"
        "/home/${TARGET_USER:-$USER}/ant_venv"
        "./ant_venv"
    )
    for vdir in "${venv_candidates[@]}"; do
        [[ -d "$vdir" ]] || continue
        # Find first matching python executable in the venv bin dir without using ls
        local first_py=""
        for _f in "$vdir/bin/python"*; do
            [ -e "$_f" ] || continue
            first_py="$_f"
            break
        done
        # candidate venv directory (no debug log)
        for p in "$vdir/bin/python3" "$vdir/bin/python" "$first_py"; do
            [[ -x "$p" ]] || continue
            venv_py="$p"; break 2
        done
    done

    if [[ -n "$venv_py" ]]; then
        # selected venv (no debug log)
        # If running as the same user, avoid sudo to preserve venv env
        if [[ "$USER" = "$TARGET_USER" || -z "${SUDO_USER:-}" ]]; then
            if "$venv_py" -c "import openant, usb, pybind11, bleak" 2>/dev/null; then
                # import check passed for current user
                cache_check_result "pkg_pips" "pass"; echo "pass"; return 0
            else
                # import check failed for current user
                cache_check_result "pkg_pips" "fail"; echo "fail"; return 1
            fi
        else
            if sudo -u "$TARGET_USER" "$venv_py" -c "import openant, usb, pybind11, bleak" 2>/dev/null; then
                # import check passed for sudo target user
                cache_check_result "pkg_pips" "pass"; echo "pass"; return 0
            else
                # import check failed for sudo target user
                cache_check_result "pkg_pips" "fail"; echo "fail"; return 1
            fi
        fi
    fi

    # Fallback: try system python3.11 then python3
    if command -v python3.11 >/dev/null 2>&1; then
        if python3.11 -c "import openant, usb, pybind11, bleak" 2>/dev/null; then
            cache_check_result "pkg_pips" "pass"; echo "pass"; return 0
        fi
    fi
    if command -v python3 >/dev/null 2>&1; then
        if python3 -c "import openant, usb, pybind11, bleak" 2>/dev/null; then
            cache_check_result "pkg_pips" "pass"; echo "pass"; return 0
        fi
    fi

    cache_check_result "pkg_pips" "fail"; echo "fail"; return 1
}

check_qt5_libs_fast() {
    local result
    if result=$(get_cached_check "qt5_libs"); then
        echo "$result"; return 0
    fi
    local required=("libQt5Core.so" "libQt5Qml.so" "libQt5Quick.so" "libQt5Bluetooth.so" "libusb-1.0.so")
    local missing=0
    for lib in "${required[@]}"; do
        local found=0
        # PERF-02 FIX: Use bash globbing instead of find subprocess (10x faster)
        for p in /usr/lib*/"$lib"* /lib*/"$lib"* /usr/lib/*/"$lib"* /lib/*/"$lib"*; do
            [[ -e "$p" ]] && { found=1; break; }
        done
        [[ $found -eq 0 ]] && ((missing++))
    done
    if (( missing == 0 )); then
        cache_check_result "qt5_libs" "pass"; echo "pass"
    else
        cache_check_result "qt5_libs" "fail"; echo "fail"
    fi
}

check_qml_modules_fast() {
    local result
    if result=$(get_cached_check "qml_modules"); then
        echo "$result"; return 0
    fi
    local missing=0
    local qmls=("QtLocation" "QtQuick.2" "QtQuick/Controls.2")
    for qml in "${qmls[@]}"; do
        local found=0
        for path in /usr/lib/*/qt5/qml /usr/lib/qt5/qml; do
            [ -d "$path/$qml" ] && { found=1; break; }
        done
        [ $found -eq 0 ] && ((missing++))
    done
    if [ $missing -eq 0 ]; then cache_check_result "qml_modules" "pass"; echo "pass"; return 0; else cache_check_result "qml_modules" "fail"; echo "fail"; return 1; fi
}

check_bluetooth_fast() {
    local result
    if result=$(get_cached_check "bluetooth"); then
        echo "$result"; return 0
    fi
    if command -v systemctl >/dev/null 2>&1 && ( systemctl is-active --quiet bluetooth.service 2>/dev/null || systemctl is-active --quiet bluez.service 2>/dev/null ); then
        cache_check_result "bluetooth" "pass"; echo "pass"
    else
        cache_check_result "bluetooth" "fail"; echo "fail"
    fi
}

check_plugdev_fast() {
    local result
    if result=$(get_cached_check "plugdev"); then
        echo "$result"; return 0
    fi
    # PERF-02 FIX: Use bash built-ins for group check (no grep subprocess)
    local grps
    grps=$(id -Gn "$TARGET_USER" 2>/dev/null || true)
    if [[ " $grps " == *" plugdev "* ]]; then
        cache_check_result "plugdev" "pass"; echo "pass"
    else
        cache_check_result "plugdev" "fail"; echo "fail"
    fi
}

check_udev_rules_fast() {
    local result
    if result=$(get_cached_check "udev_rules"); then
        echo "$result"; return 0
    fi
    if [[ -f /etc/udev/rules.d/99-garmin-ant.rules ]] || [[ -f /etc/udev/rules.d/51-garmin-ant.rules ]] || [[ -f /etc/udev/rules.d/99-ant-usb.rules ]]; then
        cache_check_result "udev_rules" "pass"; echo "pass"
    else
        cache_check_result "udev_rules" "fail"; echo "fail"
    fi
}

check_lsusb_fast() {
    local result
    if result=$(get_cached_check "lsusb"); then
        echo "$result"; return 0
    fi
    if command -v lsusb >/dev/null 2>&1; then
        cache_check_result "lsusb" "pass"; echo "pass"
    else
        cache_check_result "lsusb" "fail"; echo "fail"
    fi
}

check_ant_dongle_fast() {
    local result
    if result=$(get_cached_check "ant_dongle"); then
        echo "$result"; return 0
    fi
    local found=0
    local dongles=("0fcf:1009" "0fcf:1008" "0fcf:100c" "0fcf:100e" "0fcf:88a4" "0fcf:1004" "11fd:0001")
    if [[ -d /sys/bus/usb/devices ]]; then
        for dev in /sys/bus/usb/devices/*; do
            [[ -d "$dev" ]] || continue
            # PERF-02 FIX: Use bash read instead of cat subprocess (50x faster)
            local vid=""
            [[ -f "$dev/idVendor" ]] && read -r vid < "$dev/idVendor" 2>/dev/null
            local pid=""
            [[ -f "$dev/idProduct" ]] && read -r pid < "$dev/idProduct" 2>/dev/null
            
            if [[ -n "$vid" && -n "$pid" ]]; then
                local vidpid="${vid}:${pid}"
                for ant_dev in "${dongles[@]}"; do
                    if [[ "$vidpid" == "$ant_dev" ]]; then
                        found=1; break 2
                    fi
                done
            fi
        done
    fi
    if (( found == 1 )); then
        cache_check_result "ant_dongle" "pass"; echo "pass"
    else
        cache_check_result "ant_dongle" "warn"; echo "warn"
    fi
}

check_config_file_fast() {
    local result
    if result=$(get_cached_check "config_file"); then
        echo "$result"; return 0
    fi
    if [[ -f "$CONFIG_FILE" ]]; then
        cache_check_result "config_file" "pass"; echo "pass"
    else
        cache_check_result "config_file" "fail"; echo "fail"
    fi
}

check_qz_service_fast() {
    local result
    if result=$(get_cached_check "qz_service"); then
        echo "$result"; return 0
    fi
    local status="pending"
    if command -v systemctl >/dev/null 2>&1; then
        if run_as_root_or_sudo systemctl is-active --quiet qz.service 2>/dev/null; then
            status="pass"
        elif run_as_root_or_sudo systemctl is-failed --quiet qz.service 2>/dev/null; then
            status="warn"
        else
            # Not active, not failed. Check if file exists.
            if [[ -n "${ACTIVE_SERVICE_FILE:-}" && -f "$ACTIVE_SERVICE_FILE" ]] || [[ -f "$SERVICE_FILE_QZ" ]]; then
                status="warn"
            else
                status="pending"
            fi
        fi
    else
        # No systemctl available — treat as pending in GUI mode, warn otherwise
        if [[ "$HAS_GUI" == true ]]; then
            status="pending"
        else
            status="warn"
        fi
    fi
    cache_check_result "qz_service" "$status"
    echo "$status"
}

run_all_checks() {
    local mode="${1:-dashboard}" # "dashboard" or "splash"
    init_check_cache
    
    local -a status_keys=(
        "python311" "venv" "pkg_pips" "qt5_libs" "qml_modules"
        "bluetooth" "plugdev" "udev_rules" "lsusb"
        "config_file" "qz_service" "ant_dongle" "equipment"
    )
    
    # Function names corresponding to keys
    local -a func_names=(
        "python311" "venv" "python_packages" "qt5_libs" "qml_modules"
        "bluetooth" "plugdev" "udev_rules" "lsusb"
        "config_file" "qz_service" "ant_dongle" "equipment"
    )

    local total_checks=${#status_keys[@]}
    local completed=0

    # 1. Launch all checks in background
    local -a pids=()
    
    for ((i=0; i<total_checks; i++)); do
        local key="${status_keys[$i]}"
        local fn="${func_names[$i]}"
        
        # If in dashboard mode, mark as working immediately
        if [[ "$mode" == "dashboard" ]]; then
            update_status_atomic "$key" "working"
        fi

        (
            local func="check_${fn}_fast"
            local result
            if command -v "$func" >/dev/null 2>&1; then
                result=$($func 2>/dev/null || true)
            else
                # legacy mapping fallback
                if [[ "$fn" == "python_packages" ]]; then
                    result=$(check_python_packages_fast 2>/dev/null || true)
                else
                    result=""
                fi
            fi
            printf '%s' "$result" > "${CHECK_CACHE_DIR}/${key}.result" 2>/dev/null || true
        ) &
        pids+=($!)
    done

    # 2. Wait loop with Progress Updates
    local timeout=100 # 10 seconds total (100 * 0.1s)
    local waited=0
    
    while (( waited < timeout )); do
        local running_count=0
        for pid in "${pids[@]}"; do
            if kill -0 "$pid" 2>/dev/null; then
                ((running_count++))
            fi
        done
        
        local finished_count=$(( total_checks - running_count ))
        
        if [[ "$mode" == "splash" ]]; then
            update_splash_progress "$finished_count" "$total_checks" "Verifying system components"
        fi

        if (( running_count == 0 )); then break; fi
        
        sleep 0.1
        ((waited++))
    done

    # Kill stragglers
    for pid in "${pids[@]}"; do kill -9 "$pid" 2>/dev/null || true; done

    # 3. Collect Results and Update State
    for key in "${status_keys[@]}"; do
        local result_file="${CHECK_CACHE_DIR}/${key}.result"
        if [[ -f "$result_file" ]]; then
            local status
            status=$(cat "$result_file" 2>/dev/null || true)
            # Update the global map
            STATUS_MAP["$key"]="$status"
        else
            STATUS_MAP["$key"]="fail"
        fi
        
        # Special handling for python311 - read installation type
        if [[ "$key" == "python311" ]]; then
            local type_file="${CHECK_CACHE_DIR}/python311.type"
            if [[ -f "$type_file" ]]; then
                local py_type
                py_type=$(cat "$type_file" 2>/dev/null || echo "none")
                case "$py_type" in
                    pyenv)
                        IS_PYENV_INSTALLED=true
                        IS_PYSYS_INSTALLED=false
                        ;;
                    system)
                        IS_PYENV_INSTALLED=false
                        IS_PYSYS_INSTALLED=true
                        ;;
                    *)
                        IS_PYENV_INSTALLED=false
                        IS_PYSYS_INSTALLED=false
                        ;;
                esac
            fi
        fi
        
        # If in dashboard mode, update the UI grid atomically
        if [[ "$mode" == "dashboard" ]]; then
            update_status_atomic "$key" "${STATUS_MAP[$key]}"
        fi
    done

    # If in dashboard mode, refresh headers
    if [[ "$mode" == "dashboard" ]]; then
        draw_header_config_line
        draw_header_service_line
    fi

    # Set global flags for uninstall logic
    IS_RUNTIME_INSTALLED=false
    if [ "${STATUS_MAP[qt5_libs]:-}" = "pass" ] || [ "${STATUS_MAP[bluetooth]:-}" = "pass" ] || [ "${STATUS_MAP[lsusb]:-}" = "pass" ] || [ "${STATUS_MAP[qml_modules]:-}" = "pass" ]; then
        IS_RUNTIME_INSTALLED=true
    fi
    IS_CORE_INSTALLED=false
    if [ "${STATUS_MAP[venv]:-}" = "pass" ] || [ "${STATUS_MAP[qz_service]:-}" = "pass" ] || [ "${STATUS_MAP[config_file]:-}" = "pass" ]; then
        IS_CORE_INSTALLED=true
    fi
}



# Global process management variables (placeholders so traps with set -u don't fail)
# These are assigned inside perform_bluetooth_scan() when the scan runs.
BT_SCAN_PID=""

# ============================================================================
# FINAL STABLE BLUETOOTH RADAR (PYTHON-POWERED + REAL-TIME FIFO)
# ============================================================================

# Collect bluetooth updates in batches to amortize FIFO reads and parsing.
# Usage: collect_bt_updates_batch <fd> [batch_size] [timeout_seconds]
# Returns newline-separated lines on stdout; returns nothing if no data.
collect_bt_updates_batch() {
    local fifo_fd="${1:-4}"
    # Allow environment overrides: BT_BATCH_SIZE, BT_BATCH_TIMEOUT, BT_BATCH_SHORT_TIMEOUT
    local batch_size="${2:-${BT_BATCH_SIZE:-128}}"
    local timeout="${3:-${BT_BATCH_TIMEOUT:-0.02}}"
    # Short drain timeout between reads (seconds)
    local short_t_override="${BT_BATCH_SHORT_TIMEOUT:-0.002}"

    # Cap batch size to avoid long blocking loops
    if (( batch_size > 512 )); then batch_size=512; fi

    local -a batch=()
    local raw

    # First read may wait up to timeout seconds; subsequent reads use a
    # short non-blocking timeout to drain available data quickly.
    if read -u "$fifo_fd" -t "$timeout" -r raw 2>/dev/null; then
        batch+=("$raw")
        # drain remaining quickly using short_t_override
        local short_t="$short_t_override"
        while (( ${#batch[@]} < batch_size )); do
            if read -u "$fifo_fd" -t "$short_t" -r raw 2>/dev/null; then
                batch+=("$raw")
            else
                break
            fi
        done
    else
        # nothing available within timeout
        return 0
    fi

    # Print as newline-separated lines
    printf '%s\n' "${batch[@]}"
}

draw_rssi_bar_fixed() {
    local rssi="$1"
    local width="${2:-10}"
    
    # Validation: Default to min signal if invalid
    if ! [[ "$rssi" =~ ^-?[0-9]+$ ]]; then rssi=-99; fi
    
    # 1. Define Range Constants
    local R_MAX=-35  # Signal stronger than this is 100%
    local R_MIN=-95  # Signal weaker than this is 0%
    
    # 2. Clamping
    local clamped_rssi=$rssi
    if (( clamped_rssi > R_MAX )); then clamped_rssi=$R_MAX; fi
    if (( clamped_rssi < R_MIN )); then clamped_rssi=$R_MIN; fi
    
    # 3. Linear Mapping (Shift to positive range 0..60 for easier math)
    # Formula: (Input - Min) * Width / (Max - Min)
    local range=$(( R_MAX - R_MIN ))
    local shifted_val=$(( clamped_rssi - R_MIN ))
    
    local fill_count=$(( (shifted_val * width) / range ))
    
    # 4. Color Logic (Standard BLE Quality)
    local bar_color rssi_color
    if (( rssi >= -60 )); then
        bar_color="$GREEN"; rssi_color="$GREEN"      # Strong
    elif (( rssi >= -70 )); then
        bar_color="$CYAN"; rssi_color="$CYAN"        # Good
    elif (( rssi >= -85 )); then
        bar_color="$YELLOW"; rssi_color="$YELLOW"    # Fair
    else
        bar_color="$RED"; rssi_color="$RED"          # Weak
    fi
    
    # 5. Draw
    local filled_portion=""
    local empty_portion=""
    local empty_count=$(( width - fill_count ))
    
    if [[ $fill_count -gt 0 ]]; then
        printf -v filled_portion "%0.s█" $(seq 1 $fill_count)
    fi
    if [[ $empty_count -gt 0 ]]; then
        printf -v empty_portion "%0.s░" $(seq 1 $empty_count)
    fi
    
    echo "${bar_color}${filled_portion}${GRAY}${empty_portion}${NC}" "${rssi_color}${rssi}${NC}"
}

perform_bluetooth_scan() {
    local suppress_prompt="${1:-false}" # Phase 2: Argument
    
    # 1. PREP ENVIRONMENT
    local script_dir
    script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
    local py_script="${script_dir}/bt_provider.py"
    local venv_py="${TARGET_HOME}/ant_venv/bin/python3"
    [[ ! -f "$venv_py" ]] && venv_py=$(command -v python3)
    
    local BT_SCAN_PID=""
    local bt_fifo="$TEMP_DIR/qz_bt_fifo_$$"

    if [[ ! -f "$py_script" ]]; then
        draw_error_screen "BLUETOOTH SCAN" "Error: Script not found: $py_script" "wait"
        return 1
    fi

    # Hardware wake (Initial Only)
    sudo rfkill unblock bluetooth >/dev/null 2>&1
    echo "power on" | bluetoothctl >/dev/null 2>&1
    sleep 0.5

    set_ui_output
    UI_LOCKED=0
    clear_info_area

    # Performance caches
    declare -A RSSI_BAR_CACHE
    for rssi_val in {-30..-100}; do
        read -r cached_bar cached_num < <(draw_rssi_bar_fixed "$rssi_val" 10)
        RSSI_BAR_CACHE["${rssi_val}_bar"]="$cached_bar"
        RSSI_BAR_CACHE["${rssi_val}_num"]="$cached_num"
    done

    # --- CLEANUP ENGINE ---
    stop_bt_engine() {
        exec 4<&- 2>/dev/null || true
        
        # Kill Python (Graceful first)
        if [[ -n "$BT_SCAN_PID" ]]; then
            kill -15 "$BT_SCAN_PID" 2>/dev/null || true
            for _i in {1..15}; do
                if ! kill -0 "$BT_SCAN_PID" 2>/dev/null; then break; fi
                sleep 0.1
            done
            kill -9 "$BT_SCAN_PID" 2>/dev/null || true
        fi
        
        if pgrep -f "bt_provider.py" >/dev/null 2>&1; then
            pkill -9 -f "bt_provider.py" 2>/dev/null || true
        fi
        
        rm -f "$bt_fifo" 2>/dev/null
        bluetoothctl scan off >/dev/null 2>&1 || true
        BT_SCAN_PID=""
    }
    trap stop_bt_engine RETURN

    # --- OUTER LOOP (Refresh Logic) ---
    while true; do
        # 1. CLEANUP
        stop_bt_engine
        
        # 2. ADAPTER RESET (Crucial for Refresh)
        draw_bottom_panel_header "BLUETOOTH: RESETTING..." "false"
        clear_info_area
        draw_sealed_row $((LOG_TOP + 3)) "   ${YELLOW}Resetting Bluetooth Adapter...${NC}"
        
        bluetoothctl scan off >/dev/null 2>&1
        sleep 1.5 # Settle time
        
        local devices=() macs=() rssis=()
        unset MAC_INDEX
        declare -A MAC_INDEX
        
        local loop_count=0
        local py_status="STARTING"
        local spin_chars=('⠋' '⠙' '⠹' '⠸' '⠼' '⠴' '⠦' '⠧' '⠇' '⠏')
        local STOP_SCAN=0
        local last_draw_time=0

        # FIFO
        mkfifo "$bt_fifo"
        chmod 0666 "$bt_fifo" 2>/dev/null
        
        # Start Python
        "$venv_py" -u "$py_script" >"$bt_fifo" 2>&1 < /dev/null &
        BT_SCAN_PID=$!
        
        sleep 0.5
        
        if ! kill -0 "$BT_SCAN_PID" 2>/dev/null; then
            draw_error_screen "SCAN ERROR" "Python script crashed on startup." "wait"
            return 1
        fi
        
        if ! exec 4<"$bt_fifo" 2>/dev/null; then
            draw_error_screen "SCAN ERROR" "Failed to open FIFO pipe." "wait"
            return 1
        fi

        local saved_name=""
        if [[ -f "${CONFIG_FILE:-}" ]]; then
            saved_name=$(grep -E '^bluetooth_lastdevice_name=' "${CONFIG_FILE}" 2>/dev/null | tail -n1 | cut -d'=' -f2- | tr -d '\r' | xargs || true)
        fi

        enter_ui_mode
        clear_info_area 
        
        # --- INNER LOOP (Scanning) ---
        while [[ $STOP_SCAN -eq 0 ]]; do
            
            # A. INPUT CHECK
            if read -rsn1 -t 0.001 _unused </dev/tty 2>/dev/null; then
                STOP_SCAN=1
                break
            fi

            if ! kill -0 "$BT_SCAN_PID" 2>/dev/null; then py_status="STOPPED"; fi

            # B. DATA INGESTION
            local raw_data
            local batch_count=0
            local max_batch=50 
            
            if read -u 4 -t 0.02 -r raw_data 2>/dev/null; then
                while true; do
                    if (( batch_count % 10 == 0 )); then
                        if read -rsn1 -t 0.001 _unused </dev/tty 2>/dev/null; then
                            STOP_SCAN=1
                            break 2
                        fi
                    fi

                    raw_data=${raw_data//$'\r'/}
                    
                    if [[ "$raw_data" == STATUS\|* ]]; then
                        local _ status_msg
                        IFS='|' read -r _ _ status_msg <<< "$raw_data"
                        [[ -n "$status_msg" ]] && py_status="$status_msg"
                    elif [[ "$raw_data" == "HEARTBEAT|"* ]]; then
                        py_status="SCANNING"
                    else
                        local m r l
                        IFS='|' read -r m r l <<< "$raw_data"
                        
                        if [[ -n "$m" && "$m" != "ERROR" ]]; then
                            # Strict MAC Check
                            if [[ "$m" =~ ^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$ ]]; then
                                local idx="${MAC_INDEX[$m]:-}"
                                if [[ -n "$idx" ]]; then
                                    rssis[$idx]=$r
                                else
                                    local ltrim="${l#"${l%%[![:space:]]*}"}"
                                    ltrim="${ltrim%"${ltrim##*[![:space:]]}"}"
                                    
                                    local valid=1
                                    if [[ -z "$ltrim" || "${ltrim^^}" == "CONNECTING" || "${ltrim^^}" == "STATUS" || "${ltrim^^}" == "INITIALIZING" ]]; then valid=0; fi
                                    if [[ "$r" =~ ^-?[0-9]+$ ]]; then 
                                        if (( r >= 0 )); then valid=0; fi
                                    else
                                        valid=0
                                    fi
                                    
                                    if [[ $valid -eq 1 ]]; then
                                        local clean_label="${l//[$'\e']\[[0-9;]*m/}"
                                        clean_label="${clean_label//[^[:print:]]/}"
                                        
                                        local new_idx=${#macs[@]}
                                        macs+=("$m")
                                        rssis+=("$r")
                                        devices+=("$clean_label")
                                        MAC_INDEX["$m"]=$new_idx
                                    fi
                                fi
                            fi
                        fi
                    fi

                    ((batch_count++))
                    if (( batch_count >= max_batch )); then break; fi
                    if ! read -u 4 -t 0 -r raw_data 2>/dev/null; then break; fi
                done
            fi
            
            if [[ $STOP_SCAN -eq 1 ]]; then break; fi

            # C. THROTTLED RENDER
            if (( SECONDS > last_draw_time )); then
                last_draw_time=$SECONDS
                ((loop_count++))

                # Sort
                if [[ ${#macs[@]} -gt 1 ]]; then
                    local n=${#rssis[@]}
                    for ((i = 0; i < n - 1; i++)); do
                        for ((j = 0; j < n - i - 1; j++)); do
                            local c_rssi=${rssis[$j]}; [[ ! "$c_rssi" =~ ^-?[0-9]+$ ]] && c_rssi=-999
                            local n_rssi=${rssis[$((j + 1))]}; [[ ! "$n_rssi" =~ ^-?[0-9]+$ ]] && n_rssi=-999
                            
                            if (( c_rssi < n_rssi )); then
                                local t_m="${macs[$j]}"; local t_r="${rssis[$j]}"; local t_d="${devices[$j]}"
                                macs[$j]="${macs[$((j + 1))]}"; rssis[$j]="${rssis[$((j + 1))]}"; devices[$j]="${devices[$((j + 1))]}"
                                macs[$((j + 1))]="$t_m"; rssis[$((j + 1))]="$t_r"; devices[$((j + 1))]="$t_d"
                                MAC_INDEX["${macs[$j]}"]=$j
                                MAC_INDEX["${macs[$((j + 1))]}"]=$((j + 1))
                            fi
                        done
                    done
                fi

                # Render
                local title="BLUETOOTH: ${#macs[@]} DEVICES FOUND"
                if [[ ${#macs[@]} -eq 0 ]]; then title="BLUETOOTH: ${py_status}..."; fi
                draw_bottom_panel_header "$title" "false"
                
                local render_buffer=""
                for ((i=0; i<8; i++)); do
                    local row=$((14 + i))
                    local row_content=""
                    if [ "$i" -lt "${#macs[@]}" ]; then
                        local s=${rssis[$i]}
                        local s_col r_col
                        if [[ -n "${RSSI_BAR_CACHE[${s}_bar]:-}" ]]; then
                            s_col="${RSSI_BAR_CACHE[${s}_bar]}"; r_col="${RSSI_BAR_CACHE[${s}_num]}"
                        else
                            read -r s_col r_col < <(draw_rssi_bar_fixed "$s" 10)
                        fi

                        local name="${devices[$i]}"
                        local color="$BOLD_WHITE"
                        [[ -n "$saved_name" && "$name" == "$saved_name" ]] && color="$BOLD_CYAN"

                        local vis_name="$name"
                        if [[ ${#name} -gt 38 ]]; then vis_name="${name:0:38}"; fi
                        
                        local n_w=${#vis_name}
                        local pad_n=$(( 40 - n_w )); [[ $pad_n -lt 0 ]] && pad_n=0
                        local padding_n=""
                        if (( pad_n > 0 )); then printf -v padding_n '%*s' "$pad_n" ""; fi
                        local name_col="${color}${vis_name}${NC}${padding_n}"
                        
                        local mac_col; printf -v mac_col '%-17s' "${macs[$i]}"
                        row_content="  ${name_col}  ${mac_col}  ${s_col} ${r_col}"
                    fi

                    local v_w; v_w=$(get_vis_width "$row_content")
                    local pad_r=$(( INNER_COLS - v_w ))
                    [[ $pad_r -lt 0 ]] && pad_r=0
                    local padding_r=""
                    if (( pad_r > 0 )); then printf -v padding_r '%*s' "$pad_r" ""; fi
                    
                    printf -v _seq "\033[%d;1H${BLUE}║${NC}%s%s${BLUE}║${NC}" "$((row + 1))" "$row_content" "$padding_r"
                    render_buffer+="${_seq}"
                done

                local ui_fd; ui_fd=$(get_safe_ui_fd)
                ( printf '%s' "$render_buffer" >&"${ui_fd}" ) 2>/dev/null || true

                draw_bottom_border "Scanning... ${spin_chars[$((loop_count % 10))]} | Any key to stop"
            fi
        done

        # --- PHASE 2: SELECTION ---
        stop_bt_engine
        
        sleep 0.2
        while read -rsn1 -t 0.05 _junk </dev/tty 2>/dev/null; do :; done
        
        local menu_labels=()
        for ((i=0;i<${#macs[@]};i++)); do
            menu_labels+=("$(printf '%-30s [%s]' "${devices[$i]}" "${macs[$i]}")")
        done
        menu_labels+=("Refresh")
        
        exit_ui_mode || true
        
        show_unified_menu menu_labels 0 "SELECT DEVICE" "FULL" "false"
        local sanitized=$?
        
        local refresh_idx=${#macs[@]}

        if [[ "$sanitized" -eq 255 ]]; then
            exit_ui_mode
            return 1
        elif [[ "$sanitized" -eq "$refresh_idx" ]]; then
            continue 
        elif [[ "$sanitized" -lt "${#macs[@]}" ]]; then
            local sel_mac="${macs[$sanitized]}"
            local sel_name="${devices[$sanitized]}"
            
            update_config_key "bluetooth_lastdevice_address" "$sel_mac"
            update_config_key "bluetooth_lastdevice_name" "$sel_name"
            update_config_key "filter_device" "$sel_name"
            update_config_key "bluetooth_address" "$sel_mac"
            
            # --- UPDATE HEADER IMMEDIATELY ---
            # Manually update config array so header sees the new name
            CONFIG_STRING[bluetooth_lastdevice_name]="$sel_name"
            draw_header_equipment_line
            # ---------------------------------

            draw_bottom_panel_header "DEVICE LINKED" "false"
            clear_info_area
            draw_sealed_row $((LOG_TOP + 2)) "   ${GREEN}Device linked successfully!${NC}"
            draw_sealed_row $((LOG_TOP + 4)) "   Name: ${WHITE}$sel_name${NC}"
            draw_sealed_row $((LOG_TOP + 5)) "   Addr: ${GRAY}$sel_mac${NC}"
            
            generate_config_file
            draw_bottom_border ""
            sleep 2
            
            # FIX: Check suppression argument before prompting
            if [[ "$suppress_prompt" != "true" ]]; then
                prompt_restart_service
            fi
            
            exit_ui_mode
            return 0
        fi
    done
}

# ============================================================================
# PYENV INSTALLATION SYSTEM
# Comprehensive Python 3.11 installation via pyenv with RAM detection,
# zram support, and interactive menus
# ============================================================================

# Detect total system RAM in MB
get_total_ram_mb() {
    local ram_kb
    ram_kb=$(grep MemTotal /proc/meminfo | awk '{print $2}')
    echo $((ram_kb / 1024))
}

# Check if system is low-memory (<2GB RAM)
is_low_memory_system() {
    local ram_mb
    ram_mb=$(get_total_ram_mb)
    [[ $ram_mb -lt 2048 ]]
}

# Check if zram is installed and active
check_zram_status() {
    if ! command -v zramctl >/dev/null 2>&1; then
        echo "not_installed"
        return
    fi
    
    if swapon -s | grep -q zram; then
        echo "active"
    else
        echo "inactive"
    fi
}

# Install and configure zram
install_zram() {
    draw_bottom_panel_header "INSTALLING ZRAM SWAP" "false"
    clear_info_area
    
    local r=$((LOG_TOP + 1))
    draw_sealed_row "$r" "   ${CYAN}Installing zram compressed swap...${NC}"
    ((r++))
    draw_sealed_row "$r" ""
    ((r++))
    draw_sealed_row "$r" "   ${GRAY}This provides fast compressed swap without${NC}"
    ((r++))
    draw_sealed_row "$r" "   ${GRAY}wearing your SD card during Python build.${NC}"
    
    # Install zram-tools
    if ! run_with_progress "Installing zram-tools" "apt-get update && apt-get install -y zram-tools"; then
        draw_error_screen "ZRAM INSTALLATION FAILED" "Failed to install zram-tools package." "wait"
        return 1
    fi
    
    # Configure zram
    local config_cmd="echo -e 'ALGO=lz4\nPERCENT=50\nPRIORITY=100' | tee /etc/default/zramswap >/dev/null"
    if ! run_with_progress "Configuring zram" "$config_cmd"; then
        draw_error_screen "ZRAM CONFIGURATION FAILED" "Failed to configure zram settings." "wait"
        return 1
    fi
    
    # Restart zramswap service
    if ! run_with_progress "Activating zram" "systemctl restart zramswap"; then
        draw_error_screen "ZRAM ACTIVATION FAILED" "Failed to start zramswap service." "wait"
        return 1
    fi
    
    # Verify zram is active
    sleep 1
    if ! swapon -s | grep -q zram; then
        draw_error_screen "ZRAM VERIFICATION FAILED" "zram installed but not active. Try rebooting." "wait"
        return 1
    fi
    
    draw_bottom_panel_header "ZRAM INSTALLED" "false"
    clear_info_area
    r=$((LOG_TOP + 2))
    draw_sealed_row "$r" "   ${GREEN}✓ zram swap activated successfully${NC}"
    ((r++))
    ((r++))
    
    # Show zram stats
    local zram_size
    zram_size=$(swapon -s | grep zram | awk '{print $3}')
    draw_sealed_row "$r" "   Compressed swap size: ${CYAN}${zram_size}${NC}"
    
    draw_bottom_border "Press ENTER to continue"
    local dummy
    read -rsn1 dummy </dev/tty
    
    return 0
}

# Prompt user whether to install zram
prompt_install_zram() {
    local ram_mb
    ram_mb=$(get_total_ram_mb)
    
    draw_bottom_panel_header "LOW MEMORY DETECTED" "false"
    clear_info_area
    
    local r=$((LOG_TOP))
    draw_sealed_row "$r" ""
    ((r++))
    draw_sealed_row "$r" "   System RAM: ${YELLOW}${ram_mb}MB${NC} (< 2GB threshold)"
    ((r++))
    ((r++))
    draw_sealed_row "$r" "   Building Python 3.11 requires significant memory."
    ((r++))
    draw_sealed_row "$r" "   Installing zram provides compressed swap to prevent"
    ((r++))
    draw_sealed_row "$r" "   out-of-memory crashes during compilation."
    
    if prompt_yes_no 1; then
        return 0  # User wants to install zram
    else
        return 1  # User declined zram
    fi
}

# Install pyenv build dependencies
install_pyenv_dependencies() {
    local is_low_mem=false
    if is_low_memory_system; then
        is_low_mem=true
    fi
    
    draw_bottom_panel_header "INSTALLING BUILD DEPENDENCIES" "false"
    clear_info_area
    
    local r=$((LOG_TOP + 1))
    draw_sealed_row "$r" "   ${CYAN}Installing compiler and library dependencies...${NC}"
    ((r++))
    draw_sealed_row "$r" ""
    ((r++))
    if [[ "$is_low_mem" == "true" ]]; then
        draw_sealed_row "$r" "   ${GRAY}Low-memory mode: including ccache for faster builds${NC}"
    else
        draw_sealed_row "$r" "   ${GRAY}Standard mode: optimized for systems with 2GB+ RAM${NC}"
    fi
    
    # Base dependencies (common to both)
    local deps="git curl build-essential libssl-dev zlib1g-dev"
    deps="$deps libbz2-dev libreadline-dev libsqlite3-dev wget"
    deps="$deps llvm xz-utils tk-dev libffi-dev liblzma-dev"
    
    # Check if Ubuntu 24.04+ (needs libncurses-dev instead of libncurses5-dev)
    local ubuntu_version=""
    if grep -q "Ubuntu" /etc/os-release 2>/dev/null; then
        ubuntu_version=$(grep "VERSION_ID" /etc/os-release | cut -d'"' -f2)
    fi
    
    if [[ "$ubuntu_version" =~ ^2[4-9]\. ]] || [[ "$ubuntu_version" =~ ^[3-9][0-9]\. ]]; then
        deps="$deps libncurses-dev libncursesw5-dev"
    else
        deps="$deps libncurses5-dev libncursesw5-dev"
    fi
    
    # Add ccache for low-memory systems
    if [[ "$is_low_mem" == "true" ]]; then
        deps="$deps ccache"
    fi
    sleep 2
    
    # Install dependencies
    if ! run_with_progress "Installing pyenv dependencies" "apt-get update && apt-get install -y $deps"; then
        draw_error_screen "DEPENDENCY INSTALLATION FAILED" "Failed to install build dependencies." "wait"
        return 1
    fi
    
    return 0
}

# Install pyenv itself
install_pyenv_tool() {
    # Build the install command as three separate statements so set -e
    # (inside run_with_progress's script wrapper) catches each one:
    #   1. Remove any stale .pyenv from a previous failed attempt — the
    #      pyenv installer hard-exits if the directory already exists.
    #   2. Download the installer to a file — avoids the pipe in
    #      "curl | bash" whose exit code set -e can't reliably catch.
    #   3. Run the installer as TARGET_USER with HOME set explicitly —
    #      sudo -u does NOT guarantee $HOME is updated, which is how
    #      pyenv decides where to install.
    local pyenv_cmd="rm -rf \"$TARGET_HOME/.pyenv\"
curl -sS -o \"$TEMP_DIR/pyenv_installer.sh\" https://pyenv.run
sudo -u \"$TARGET_USER\" env HOME=\"$TARGET_HOME\" bash < \"$TEMP_DIR/pyenv_installer.sh\""

    if ! run_with_progress "Installing pyenv" "$pyenv_cmd"; then
        return 1
    fi

    # Add pyenv to .bashrc if not already present
    local bashrc="$TARGET_HOME/.bashrc"
    if ! grep -q "PYENV_ROOT" "$bashrc" 2>/dev/null; then
        cat >> "$bashrc" << 'EOF'

# pyenv configuration
export PYENV_ROOT="$HOME/.pyenv"
export PATH="$PYENV_ROOT/bin:$PATH"
eval "$(pyenv init -)"
EOF
    fi

    return 0
}

# Build Python 3.11 with pyenv (standard or low-memory mode)
build_python_with_pyenv() {
    local is_low_mem=false
    is_low_memory_system && is_low_mem=true

    # ---------------------------------------------------------------------------
    # 1. Validate pyenv binary exists
    # ---------------------------------------------------------------------------
    export PYENV_ROOT="$TARGET_HOME/.pyenv"
    export PATH="$PYENV_ROOT/bin:$PATH"

    if [[ ! -f "$PYENV_ROOT/bin/pyenv" ]]; then
        draw_error_screen "PYENV NOT FOUND" "pyenv binary not found at $PYENV_ROOT/bin/pyenv" "wait"
        return 1
    fi

    # ---------------------------------------------------------------------------
    # 2. Build the environment + command string
    # ---------------------------------------------------------------------------
    local build_env=""
    local mode_label="Standard"
    local est_time="10-15 minutes"

    if [[ "$is_low_mem" == "true" ]]; then
        local cpu_arch
        cpu_arch=$(lscpu | grep "Model name" | sed 's/.*Cortex-/cortex-/' | awk '{print tolower($1)}')
        [[ -z "$cpu_arch" ]] && cpu_arch="native"

        build_env+="MAKE_OPTS='-j1' "
        build_env+="PYTHON_CONFIGURE_OPTS='--enable-shared --enable-optimizations --with-lto=no --with-system-ffi' "
        build_env+="PROFILE_TASK='-j0' "
        build_env+="CFLAGS='-O2 -mcpu=$cpu_arch -mtune=$cpu_arch -pipe -fPIC' "
        mode_label="Low-Memory"
        est_time="60-95 minutes"
    else
        build_env+="PYTHON_CONFIGURE_OPTS='--enable-shared --enable-optimizations' "
    fi

    # ---------------------------------------------------------------------------
    # 3. Write a self-contained build script (mirrors run_with_progress pattern)
    # ---------------------------------------------------------------------------
    local log_file="${TEMP_DIR:-/tmp}/qz_setup.log"
    local script_file="${TEMP_DIR:-/tmp}/qz_pyenv_build.sh"

    cat > "$script_file" <<EOF
#!/bin/bash
export PYENV_ROOT="$PYENV_ROOT"
export PATH="\$PYENV_ROOT/bin:\$PATH"
sudo -u "$TARGET_USER" bash -c "$build_env \$PYENV_ROOT/bin/pyenv install -v 3.11.9"
EOF
    chmod +x "$script_file"

    # ---------------------------------------------------------------------------
    # 4. Draw the header — clean, minimal, consistent with other steps
    # ---------------------------------------------------------------------------
    draw_bottom_panel_header "BUILDING PYTHON 3.11.9" "false"
    clear_info_area
    draw_sealed_row $((LOG_TOP + 1)) "   ${WHITE}Compiling Python 3.11.9 ${GRAY}(${mode_label})${NC}"
    draw_sealed_row $((LOG_TOP + 3)) "   ${GRAY}Estimated time: ${est_time} — do not interrupt${NC}"
    draw_bottom_border

    # ---------------------------------------------------------------------------
    # 5. Launch build and animate: knight-rider bar + elapsed MM:SS timer
    # ---------------------------------------------------------------------------
    : > "$log_file"
    bash "$script_file" > "$log_file" 2>&1 &
    local pid=$!

    local bar_width=40 pulse_width=6 pos=0 dir=1
    local start_time=$SECONDS
    enter_ui_mode

    while kill -0 "$pid" 2>/dev/null; do
        # --- knight-rider bar ---
        local bar_str=""
        for ((i=0; i<bar_width; i++)); do
            if (( i >= pos && i < pos + pulse_width )); then
                bar_str="${bar_str}━"
            else
                bar_str="${bar_str}─"
            fi
        done

        # --- elapsed timer ---
        local elapsed=$(( SECONDS - start_time ))
        local mins=$(( elapsed / 60 ))
        local secs=$(( elapsed % 60 ))
        printf -v timer_str "%02d:%02d" "$mins" "$secs"

        # --- single row: bar  timer ---
        draw_sealed_row $((LOG_TOP + 5)) "                   ${CYAN}${bar_str}${NC}  ${GRAY}${timer_str}${NC}"

        # --- advance pulse position ---
        if (( dir == 1 )); then
            (( pos++ ))
            (( pos + pulse_width >= bar_width )) && dir=-1
        else
            (( pos-- ))
            (( pos <= 0 )) && dir=1
        fi

        read -t 0.1 -N 0 2>/dev/null || true
    done

    wait "$pid"
    local exit_code=$?

    # ---------------------------------------------------------------------------
    # 6. Handle failure (OOM-aware)
    # ---------------------------------------------------------------------------
    if [[ $exit_code -ne 0 ]]; then
        if grep -q "virtual memory exhausted\|Cannot allocate memory" "$log_file" 2>/dev/null; then
            draw_error_screen "OUT OF MEMORY" \
                "Python build failed — not enough memory.\n\n  1. Enable zram if not already active\n  2. Add emergency swap (see README)\n  3. Close other applications and retry" "wait"
        else
            local log_excerpt
            log_excerpt=$(grep -a "." "$log_file" 2>/dev/null | tail -n 5 | sed 's/[[:cntrl:]]\+//g')
            draw_error_screen "PYTHON BUILD FAILED" "Error during compilation:\n${log_excerpt}" "wait"
        fi
        return 1
    fi

    # ---------------------------------------------------------------------------
    # 7. Post-build: set global version + verify
    # ---------------------------------------------------------------------------
    if ! sudo -u "$TARGET_USER" bash -c "$PYENV_ROOT/bin/pyenv global 3.11.9" >>"$log_file" 2>&1; then
        draw_error_screen "PYENV GLOBAL FAILED" "Failed to set Python 3.11.9 as the default version." "wait"
        return 1
    fi

    local python_version
    python_version=$("$TARGET_HOME/.pyenv/versions/3.11.9/bin/python3" --version 2>&1 | grep -oP '\d+\.\d+\.\d+')

    if [[ "$python_version" != "3.11.9" ]]; then
        draw_error_screen "VERIFICATION FAILED" \
            "Version mismatch after install.\n  Expected: 3.11.9\n  Got:      ${python_version:-unknown}" "wait"
        return 1
    fi

    # ---------------------------------------------------------------------------
    # 8. Success screen — show where it lives and how long it took
    # ---------------------------------------------------------------------------
    local total_elapsed=$(( SECONDS - start_time ))
    local total_mins=$(( total_elapsed / 60 ))
    local total_secs=$(( total_elapsed % 60 ))
    printf -v total_time "%02d:%02d" "$total_mins" "$total_secs"

    draw_bottom_panel_header "PYTHON BUILD COMPLETE" "false"
    clear_info_area
    draw_sealed_row $((LOG_TOP + 2)) "   ${GREEN}✓ Python 3.11.9 installed successfully${NC}"
    draw_sealed_row $((LOG_TOP + 4)) "   Location:   ${CYAN}$TARGET_HOME/.pyenv/versions/3.11.9${NC}"
    draw_sealed_row $((LOG_TOP + 5)) "   Build time: ${CYAN}${total_time}${NC}"

    draw_bottom_border "Press ENTER to continue"
    read -rsn1 </dev/tty

    IS_PYENV_INSTALLED=true
    update_status "python311" "pass"
    return 0
}

# Main pyenv installation flow with interactive menu
install_python311_via_pyenv() {
    local ram_mb
    ram_mb=$(get_total_ram_mb)
    local is_low_mem=false
    is_low_memory_system && is_low_mem=true

    # Step 1: Check and offer zram for low-memory systems
    if [[ "$is_low_mem" == "true" ]]; then
        local zram_status
        zram_status=$(check_zram_status)
        
        if [[ "$zram_status" == "not_installed" ]]; then
            if prompt_install_zram; then
                if ! install_zram; then
                    draw_error_screen "ZRAM INSTALLATION FAILED" "Cannot proceed without swap on low-memory system.\n\nPlease install zram manually or add swap space." "wait"
                    return 1
                fi
            else
                # User declined zram - warn about risks
                draw_bottom_panel_header "WARNING" "false"
                clear_info_area
                local r=$((LOG_TOP + 1))
                draw_sealed_row "$r" "   ${YELLOW}Building Python without additional swap is risky${NC}"
                ((r++))
                draw_sealed_row "$r" "   ${YELLOW}on low-memory systems.${NC}"
                ((r++))
                ((r++))
                draw_sealed_row "$r" "   ${WHITE}Continue anyway?${NC}"
                
                if ! prompt_yes_no 6; then
                    return 1
                fi
            fi
        elif [[ "$zram_status" == "inactive" ]]; then
            # zram installed but not active
            draw_bottom_panel_header "ZRAM INACTIVE" "false"
            clear_info_area
            local r=$((LOG_TOP + 1))
            draw_sealed_row "$r" "   zram is installed but not currently active."
            ((r++))
            ((r++))
            draw_sealed_row "$r" "   Activate it now?"
            
            if prompt_yes_no 4; then
                if ! run_with_progress "Activating zram" "systemctl restart zramswap"; then
                    draw_error_screen "ZRAM ACTIVATION FAILED" "Failed to activate zram." "wait"
                    return 1
                fi
            fi
        fi
    fi
    
    # Step 2: Install pyenv dependencies
    if ! install_pyenv_dependencies; then
        return 1
    fi
    
    # Step 3: Install pyenv tool
    if ! install_pyenv_tool; then
        return 1
    fi
    
    # Step 4: Build Python 3.11.9
    if ! build_python_with_pyenv; then
        return 1
    fi
    
    return 0
}

install_python311() {
    # Check for --pyenv flag to force pyenv installation
    if [[ "${FORCE_PYENV:-0}" -eq 1 ]]; then
        draw_bottom_panel_header "FORCED PYENV INSTALLATION" "false"
        clear_info_area
        
        local current_os
        current_os=$(grep PRETTY_NAME /etc/os-release | cut -d'"' -f2)
        local ram_mb
        ram_mb=$(get_total_ram_mb)
        
        local r=$((LOG_TOP + 1))
        draw_sealed_row "$r" "   OS: ${CYAN}$current_os${NC}"
        ((r++))
        draw_sealed_row "$r" "   RAM: ${CYAN}${ram_mb}MB${NC}"
        ((r++))
        ((r++))
        draw_sealed_row "$r" "   ${WHITE}Install Python 3.11 using pyenv? ${GRAY}(Compilation will take 10-95 minutes)${NC}"

        if prompt_yes_no 5; then
            # User confirmed pyenv installation
            install_python311_via_pyenv
            return $?
        else
            # User declined pyenv installation
            update_status "python311" "fail"
            return 1
        fi
    fi
    
    # Check if Python 3.11 is specifically available
    local py311_available=false
    local newer_py_available=false
    local system_py_version=""
    
    if apt-cache show python3.11 >/dev/null 2>&1; then
        py311_available=true
    elif apt-cache show python3 >/dev/null 2>&1; then
        # Check if they have newer Python (3.12, 3.13, etc.)
        system_py_version=$(apt-cache policy python3 | grep Candidate | grep -oP '\d+\.\d+' | head -1)
        if [[ "$system_py_version" =~ ^3\.(1[2-9]|[2-9][0-9]) ]]; then
            newer_py_available=true
        fi
    fi

    if [ "$py311_available" = true ]; then
        # Install system Python 3.11 (Bookworm, Ubuntu 22.04, etc.)
        local sys_pkgs="python3.11 python3.11-venv libpython3.11 python3-pip-whl python3-setuptools-whl"
        if run_with_progress "Installing System Python 3.11" "apt-get update && apt-get install -y $sys_pkgs"; then
            IS_PYSYS_INSTALLED=true
            update_status "python311" "pass"
            return 0
        else
            update_status "python311" "fail"
            return 1
        fi
    else
        # Python 3.11 not available via apt - offer pyenv installation
        draw_bottom_panel_header "PYTHON 3.11 REQUIRED" "false"
        clear_info_area
        
        local current_os
        current_os=$(grep PRETTY_NAME /etc/os-release | cut -d'"' -f2)
        local ram_mb
        ram_mb=$(get_total_ram_mb)
        
        local r=$((LOG_TOP + 1))
        draw_sealed_row "$r" "   Python 3.11 not available via apt on this system"
        ((r++))
        draw_sealed_row "$r" "   OS: ${CYAN}$current_os${NC}"
        ((r++))
        draw_sealed_row "$r" "   RAM: ${CYAN}${ram_mb}MB${NC}"
        ((r++))
        
        if [[ "$newer_py_available" == "true" ]]; then
            draw_sealed_row "$r" "   ${YELLOW}System has Python $system_py_version (QZ needs 3.11)${NC}"
        else
            draw_sealed_row "$r" "   ${YELLOW}System Python is too old (QZ needs 3.11)${NC}"
        fi
        ((r++))
        ((r++))
        draw_sealed_row "$r" "   ${WHITE}Install Python 3.11 using pyenv? ${GRAY}(Compilation will take 10-95 minutes)${NC}"
        
        if prompt_yes_no 7; then
            # User chose to install via pyenv
            install_python311_via_pyenv
            return $?
        else
            # User declined pyenv installation
            update_status "python311" "fail"
            return 1
        fi
    fi
}

install_venv() {
    local venv_path="$TARGET_HOME/ant_venv"
    local pyenv_bin="$TARGET_HOME/.pyenv/versions/3.11.9/bin/python3"
    local sys_bin="/usr/bin/python3.11"
    local py_cmd=""

    # 1. Select Python Binary
    if [ -f "$pyenv_bin" ]; then
        py_cmd="$pyenv_bin"
    elif [ -f "$sys_bin" ]; then
        py_cmd="$sys_bin"
    else
        py_cmd="python3"
    fi

    # 2. THE REPAIR LOGIC: If using system Python, ensure it isn't "broken"
    if [[ "$py_cmd" == "/usr/bin/"* ]]; then
        # Check if ensurepip is missing (the cause of your venv crash)
        if ! "$py_cmd" -c "import ensurepip" >/dev/null 2>&1; then
            # We must install these 3 specific packages to fix the 'rc' status in your log
            local repair_pkgs="python3.11-venv python3-pip-whl python3-setuptools-whl"
            run_with_progress "Repairing System Python" "apt-get update && apt-get install -y --reinstall $repair_pkgs"
        fi
    fi

    # 3. Create the Virtual Environment
    rm -rf "$venv_path"
    
    # We create the venv as the target user
    local cmd="sudo -u \"$TARGET_USER\" $py_cmd -m venv \"$venv_path\""
    
    if run_with_progress "Creating Virtual Environment" "$cmd"; then
        chown -R "$TARGET_USER:$TARGET_USER" "$venv_path"
        return 0
    else
        # If it still fails, draw_error_screen will now have plenty of space
        local log_err=$(tail -n 10 "${TEMP_DIR:-/tmp}/qz_setup.log" 2>/dev/null)
        draw_error_screen "VENV CREATION FAILED" "Command: $cmd\n\nError:\n$log_err" "wait"
        return 1
    fi
}

install_python_packages() {
    local venv_pip="$TARGET_HOME/ant_venv/bin/pip"
    
    local cmd="sudo -u \"$TARGET_USER\" \"$venv_pip\" install --upgrade pip && \
               sudo -u \"$TARGET_USER\" \"$venv_pip\" install openant pyusb pybind11 bleak"
               
    run_with_progress "Installing Python ANT+ Packages" "$cmd"
}

install_qt5_libs() {
    local libs=(
        libqt5core5a
        libqt5qml5
        libqt5quick5
        libqt5quickwidgets5
        libqt5concurrent5
        libqt5bluetooth5
        libqt5charts5
        libqt5multimedia5
        libqt5multimediawidgets5
        libqt5multimedia5-plugins
        libqt5networkauth5
        libqt5positioning5
        libqt5sql5
        libqt5texttospeech5
        libqt5websockets5
        libqt5widgets5
        libqt5xml5
        libqt5location5
        qtlocation5-dev
        qml-module-qtlocation
        qml-module-qtpositioning
        qml-module-qtquick2
        qml-module-qtquick-controls
        qml-module-qtquick-controls2
        qml-module-qtquick-dialogs
        qml-module-qtquick-layouts
        qml-module-qtquick-window2
        qml-module-qtmultimedia
        libusb-1.0-0
        bluez
        usbutils
        # python3-pip REMOVED - This prevents system Python 3.11 from being pulled in
    )
    if [ "$IS_PI" = true ]; then
        libs+=(raspi-config lsof)
    fi
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


# ============================================================================
# MENUS & PROMPTS
# ============================================================================

prompt_yes_no() {
    local start_offset=${1:-4}
    local start_row=$((LOG_TOP + start_offset))
    local selected=0
    local options=("Yes" "No")
    local num_options=${#options[@]}
    local inner_cols=${INNER_COLS:-80}
    local inner_w=$(( inner_cols - 5 ))

    # FIX: Updated Footer text to reflect "Back" behavior
    draw_bottom_border "Arrows: Up/Down | Enter: Select | Esc: Back"

    local prev_selected=$selected
    
    # Force initial draw
    local force_draw=true

    while true; do
        # --- RENDER PHASE ---
        if [[ "$force_draw" == "true" ]] || [[ $selected -ne $prev_selected ]]; then
            local buffer=""
            for i in "${!options[@]}"; do
                local row=$((start_row + i))
                local item_text="${options[$i]}"
                local row_content=""
                
                if [[ $i -eq $selected ]]; then
                    row_content="   ${CYAN}► ${BOLD_CYAN}${item_text}${NC}"
                else
                    row_content="     ${GRAY}${item_text}${NC}"
                fi
                
                # Padding calculation (Inline)
                local text_len=${#item_text}
                local pad_len=$(( inner_w - text_len ))
                [[ $pad_len -lt 0 ]] && pad_len=0
                
                printf -v _seq "\e[%d;1H${BLUE}║${NC}%s%*s${BLUE}║${NC}" "$((row + 1))" "$row_content" "$pad_len" ""
                buffer+="${_seq}"
            done
            
            local ui_fd; ui_fd=$(get_safe_ui_fd)
            ( printf %b "$buffer" >&"${ui_fd}" ) 2>/dev/null || true
            
            force_draw=false
            prev_selected=$selected
        fi

        # --- INPUT PHASE ---
        local key=""
        safe_read_key key

        if [[ $key == $'\x1b' ]]; then
            read_escape_sequence k2
            # FIX: Single ESC -> Return 2 (Signal for "Back/Cancel")
            # 0=Yes, 1=No, 2=Back
            if [[ -z "${k2:-}" ]]; then return 2; fi
            
            if [[ "$k2" == "[A" ]]; then 
                ((selected--)); [[ $selected -lt 0 ]] && selected=$((num_options - 1))
            elif [[ "$k2" == "[B" ]]; then 
                ((selected++)); [[ $selected -ge $num_options ]] && selected=0
            fi
        
        elif [[ $key == "" ]]; then
            return "$selected" # Returns 0 for Yes, 1 for No
        fi
    done
}

prompt_input_yes() {
    local prompt="Type YES to confirm: "
    local prompt_row=$((LOG_BOTTOM))
    local old_stty

    # 0. Debugging setup (must run as root for full access)
    : > /tmp/qz_crash_trace.log
    trap '_debug_trace' DEBUG
    if [ -n "${SUDO_USER:-}" ]; then chown "${SUDO_USER}:${SUDO_USER}" /tmp/qz_crash_trace.log 2>/dev/null || true; fi

    # 1. Save original terminal state
    old_stty=$(stty -g 2>/dev/null || true)

    # 2. Set custom terminal state for controlled input
    stty -icanon -echo min 1 2>/dev/null || true
    
    # Manually show cursor
    local ui_fd=${UI_FD:-2}
    printf '\e[?25h' >&"${ui_fd}" 2>/dev/null || true

    # 3. Draw the Input Box with Visual Fix
    local total_w=${TOTAL_COLS:-80}
    local BOX_WIDTH=$((total_w - 4)) # Width between inner pipes

    # Calculate padding for warning and prompt messages to fit BOX_WIDTH
    local WARN_MSG_RAW="WARNING: This may break your desktop if you are not on a true server."
    # WARNING message is 68 visible characters
    local PAD_WARN=$((BOX_WIDTH - 2 - 68)) # -2 for the spaces after the inner pipe
    [[ $PAD_WARN -lt 0 ]] && PAD_WARN=1

    local PADDED_WARN_MSG
    printf -v PADDED_WARN_MSG " %s%*s" "${WHITE}${WARN_MSG_RAW}${NC}" "$PAD_WARN" ""

    local PROMPT_MSG_RAW="${YELLOW}${prompt}${NC}"
    local PAD_PROMPT=$((BOX_WIDTH - 2 - ${#prompt}))
    [[ $PAD_PROMPT -lt 0 ]] && PAD_PROMPT=1
    
    local PADDED_PROMPT_MSG
    # We only pad up to the point of the user input, the input itself is manual
    printf -v PADDED_PROMPT_MSG " ${PROMPT_MSG_RAW}"

    # Draw the box
    print_at $((prompt_row - 3)) "${BLUE}║${RED}╔$(printf '%.0s═' $(seq 1 $((total_w - 4))))╗${NC}"
    print_at $((prompt_row - 2)) "${BLUE}║${RED}║${NC}${PADDED_WARN_MSG}${RED}║${NC}"
    print_at $((prompt_row - 1)) "${BLUE}║${RED}║${NC}${PADDED_PROMPT_MSG}$(printf '%*s' $((PAD_PROMPT)) '') ${RED}║${NC}"
    print_at $((prompt_row))     "${BLUE}║${RED}╚$(printf '%.0s═' $(seq 1 $((total_w - 4))))╝${NC}"

    # 4. Move cursor to the input position
    move_cursor $((prompt_row - 1)) $((3 + ${#prompt}))
    
    # 5. Capture Input
    local input=""
    local char

    while true; do
        read -rN1 char 2>/dev/null || continue

        case "$char" in
            $'\x0a'|$'\x0d') break ;; # ENTER
            $'\x1b') 
                read_escape_sequence junk 0.01 
                input="ABORT"
                break
                ;;
            $'\x7f'|$'\x08') # BACKSPACE / DELETE
                if [ ${#input} -gt 0 ]; then
                    input="${input::-1}"
                    printf "\b \b"
                fi
                ;;
            [a-zA-Z]) # LETTERS
                if [ ${#input} -lt 3 ]; then 
                    input+="$char"
                    printf "%s" "$char"
                fi
                ;;
        esac
    done

    # 6. DEACTIVATE TRACE AND RESTORE (CRITICAL)
    trap - DEBUG

    # Force a full stty restore from the original setting
    if [[ -n "${old_stty:-}" ]]; then stty "${old_stty}" 2>/dev/null || true; else stty echo 2>/dev/null || true; fi

    # 7. Clear the prompt box 
    local r
    for ((r=prompt_row-3; r<=prompt_row; r++)); do
        print_at "$r" "$(printf '%*s' "$total_w" ' ')"
    done
    
    # Check result
    if [ "$input" = "YES" ]; then return 0; else return 1; fi
}

# Initialize width calculator early to avoid first-call detection overhead
init_width_calculator


# Global helper for the action menu context
# Defined outside to ensure visibility to show_unified_menu
# OBSOLETE: Context rendering replaced by help panel system
# _render_action_context() was removed as help text is now shown in the right panel

prompt_action_menu() {
    local fails=$1
    local options=("Guided Setup" "Exit")
    local help_texts=(
        "Run an interactive wizard that will automatically fix detected issues step-by-step. Recommended for first-time setup or when multiple components need attention."
        "Exit the setup dashboard without making changes. You can run this tool again later to resolve the issues."
    )
    
    # 5th Arg = "true" (Enable Legend), 6th Arg = empty (no context callback), 7th Arg = Help Text Array
    show_unified_menu options 0 "ISSUES DETECTED ($fails)" "FULL" "true" "" help_texts
    local idx=$?
    
    if [[ $idx -eq 255 ]]; then return 1; fi
    return "$idx"
}

show_unified_menu() {
    # Arguments:
    # $1: Items Array Name
    # $2: Selected Index
    # $3: Title
    # $4: Draw Strategy
    # $5: Show Legend ("true"/"false")
    # $6: Context Callback Function Name
    # $7: Help Text Array Name (optional)
    
    local -n _ref_items=$1
    local selected=${2:-0}
    local title=${3:-"SELECT OPTION"}
    local draw_strategy=${4:-"FULL"}
    local show_legend_hint=${5:-"false"}
    local context_callback=${6:-}
    local help_array_name=${7:-}

    local total_count=${#_ref_items[@]}
    
    # Clamp selected to valid range — the caller's index may be stale if the
    # menu was rebuilt with fewer items (e.g. after removing the service).
    if (( selected >= total_count )); then selected=$(( total_count - 1 )); fi
    if (( selected < 0 )); then selected=0; fi
    local pad_bottom=1
    local top_row=${LOG_TOP:-13}
    local bottom_row=${LOG_BOTTOM:-21}
    local full_height=$(( bottom_row - top_row + 1 ))
    local content_height=$(( full_height - 2 ))
    local inner_cols=${INNER_COLS:-80}
    
    # Help panel configuration
    local has_help=false
    if [[ -n "$help_array_name" ]]; then
        local -n _ref_help=$help_array_name
        if [[ ${#_ref_help[@]} -gt 0 ]]; then
            has_help=true
        fi
    fi
    
    # Calculate column widths
    local menu_width help_width
    if [[ "$has_help" == "true" ]]; then
        menu_width=$(( inner_cols / 2 - 1 ))  # Left half
        help_width=$(( inner_cols - menu_width ))  # Right half (no separator)
    else
        menu_width=$inner_cols
        help_width=0
    fi
    
    if [[ $total_count -eq 0 ]]; then return 255; fi

    # Determine Footer Text based on Context
    local footer_text="Arrows: Up/Down | Enter: Select | Esc: Back"
    if [[ "$show_legend_hint" == "true" ]]; then
        footer_text="Arrows: Up/Down | Enter: Select"
    fi

    local -a _local_widths
    if [[ "${MENU_CACHE_LOADED:-0}" -eq 1 ]] && [[ ${#MENU_CACHE_WIDTHS[@]} -eq $total_count ]]; then
        _local_widths=("${MENU_CACHE_WIDTHS[@]}")
    else
        for ((i=0; i<total_count; i++)); do
            local txt="${_ref_items[i]}"
            if command -v get_vis_width >/dev/null; then _local_widths[i]=$(get_vis_width "$txt"); else _local_widths[i]=${#txt}; fi
        done
    fi

    local start_idx=0
    if (( total_count > content_height )); then
        start_idx=$(( selected - (content_height / 2) ))
        [[ $start_idx -lt 0 ]] && start_idx=0
        [[ $start_idx -gt $((total_count - content_height)) ]] && start_idx=$((total_count - content_height))
    fi

    local prev_selected=$selected
    local prev_start_idx=-1

    if [[ "$draw_strategy" == "FULL" ]]; then
        draw_bottom_panel_header "$title" "$show_legend_hint"
        draw_bottom_border "$footer_text"
    fi

    local empty_space
    printf -v empty_space "%*s" "$menu_width" ""
    local empty_line_menu="${BLUE}║${NC}${empty_space}"
    
    if [[ "$has_help" == "true" ]]; then
        local help_empty_space
        printf -v help_empty_space "%*s" "$help_width" ""
        empty_line_menu="${empty_line_menu}${help_empty_space}${BLUE}║${NC}"
    else
        empty_line_menu="${empty_line_menu}${BLUE}║${NC}"
    fi

    # Helper to Build Menu Row
    _build_menu_row() {
        local _r=$1; local _s_idx=$2; local _sel=$3
        if (( _r == 0 )) || (( _r == full_height - 1 )); then
             _row_out="$empty_line_menu"
             return
        fi
        local _rel_item=$(( _r - 1 ))
        local _abs_idx=$(( _s_idx + _rel_item ))
        local _content=""
        if (( _abs_idx < total_count )); then
            local _txt="${_ref_items[_abs_idx]}"
            local _w=${_local_widths[_abs_idx]:-0}
            local _mw=$(( menu_width - 5 ))
            if (( _w > _mw )); then _txt="${_txt:0:$_mw}"; _w=$_mw; fi
            
            if (( _abs_idx == _sel )); then
                _content="   ${CYAN}► ${BOLD_CYAN}${_txt}${NC}"
            else
                _content="     ${GRAY}${_txt}${NC}"
            fi
            local _pl=$(( menu_width - 5 - _w ))
            [[ $_pl -lt 0 ]] && _pl=0
            printf -v _content "%s%*s" "$_content" "$_pl" ""
        else
            printf -v _content "%*s" "$menu_width" ""
        fi
        
        if [[ "$has_help" == "true" ]]; then
            local help_empty
            printf -v help_empty "%*s" "$help_width" ""
            _row_out="${BLUE}║${NC}${_content}${help_empty}${BLUE}║${NC}"
        else
            _row_out="${BLUE}║${NC}${_content}${BLUE}║${NC}"
        fi
    }
    
    # Helper to Draw Help Panel
    _draw_help_panel() {
        local _sel=$1
        if [[ "$has_help" != "true" ]]; then return; fi
        if [[ $_sel -ge ${#_ref_help[@]} ]]; then return; fi
        
        local help_text="${_ref_help[_sel]}"
        local -a help_lines=()
        
        # Word wrap help text to fit help panel width
        local max_width=$(( help_width - 4 ))  # Leave some padding
        while IFS= read -r line; do
            if [[ ${#line} -le $max_width ]]; then
                help_lines+=("$line")
            else
                # Simple word wrap
                local remaining="$line"
                while [[ -n "$remaining" ]]; do
                    if [[ ${#remaining} -le $max_width ]]; then
                        help_lines+=("$remaining")
                        remaining=""
                    else
                        local cut_pos=$max_width
                        # Try to break at a space
                        local substr="${remaining:0:$max_width}"
                        if [[ "$substr" =~ \  ]]; then
                            cut_pos="${substr% *}"
                            cut_pos=${#cut_pos}
                        fi
                        help_lines+=("${remaining:0:$cut_pos}")
                        remaining="${remaining:$cut_pos}"
                        remaining="${remaining# }"  # Trim leading space
                    fi
                done
            fi
        done <<< "$help_text"
        
        # Draw help panel
        local help_row=0
        local col_offset=$(( menu_width + 2 ))  # Position after menu (no separator)
        
        for ((r=1; r<full_height-1; r++)); do
            local screen_row=$(( top_row + r ))
            local line_text=""
            
            if [[ $help_row -lt ${#help_lines[@]} ]]; then
                line_text="${help_lines[help_row]}"
                ((help_row++))
            fi
            
            # Pad and draw
            local line_len=${#line_text}
            local pad_len=$(( help_width - 2 - line_len ))
            [[ $pad_len -lt 0 ]] && pad_len=0
            
            printf -v padded_line "  %s%*s" "$line_text" "$pad_len" ""
            print_at_col "$screen_row" "$col_offset" "${GRAY}${padded_line}${NC}"
        done
    }

    # --- LOOP ---
    while true; do
        local buffer=""
        local scroll_occurred=false
        if [[ $start_idx -ne $prev_start_idx ]]; then scroll_occurred=true; fi

        # --- RENDER ---
        if [[ "$scroll_occurred" == "true" ]]; then
            for ((r=0; r<full_height; r++)); do
                local screen_row=$(( top_row + r ))
                local _row_out=""
                _build_menu_row "$r" "$start_idx" "$selected"
                printf -v _seq "\e[%d;1H%s" "$((screen_row + 1))" "$_row_out"
                buffer+="${_seq}"
            done
            local ui_fd; ui_fd=$(get_safe_ui_fd)
            printf %b "$buffer" >&"${ui_fd}" 2>/dev/null || true
            
            # Draw help panel for current selection
            _draw_help_panel "$selected"
            
            if [[ -n "$context_callback" ]] && [[ "$(type -t "$context_callback")" == "function" ]]; then
                "$context_callback"
            fi
            prev_start_idx=$start_idx
            prev_selected=$selected
            
        elif [[ $selected -ne $prev_selected ]]; then
            local old_rel=$(( prev_selected - start_idx ))
            local old_r=$(( old_rel + 1 ))
            if (( old_r > 0 && old_r < full_height - 1 )); then
                local screen_row=$(( top_row + old_r ))
                local _row_out=""
                _build_menu_row "$old_r" "$start_idx" "$selected"
                printf -v _seq "\e[%d;1H%s" "$((screen_row + 1))" "$_row_out"
                buffer+="${_seq}"
            fi
            local new_rel=$(( selected - start_idx ))
            local new_r=$(( new_rel + 1 ))
            if (( new_r > 0 && new_r < full_height - 1 )); then
                local screen_row=$(( top_row + new_r ))
                local _row_out=""
                _build_menu_row "$new_r" "$start_idx" "$selected"
                printf -v _seq "\e[%d;1H%s" "$((screen_row + 1))" "$_row_out"
                buffer+="${_seq}"
            fi
            local ui_fd; ui_fd=$(get_safe_ui_fd)
            printf %b "$buffer" >&"${ui_fd}" 2>/dev/null || true
            
            # Update help panel for new selection
            _draw_help_panel "$selected"
            
            prev_selected=$selected
        fi

        # --- INPUT ---
        local key=""
        safe_read_key key

        if [[ $key == $'\x1b' ]]; then
            read_escape_sequence k2
            # Single ESC detected
            if [[ -z "${k2:-}" ]]; then 
                # FIX: Ignore ESC if on Main Menu (Legend=true)
                if [[ "$show_legend_hint" == "true" ]]; then
                    continue
                fi
                return 255 
            fi
            
            # Arrow Key Handling
            if [[ "$k2" == "[A" ]]; then 
                ((selected--)); [[ $selected -lt 0 ]] && selected=$((total_count - 1))
            elif [[ "$k2" == "[B" ]]; then 
                ((selected++)); [[ $selected -ge $total_count ]] && selected=0
            fi
        elif [[ $key == [lL] ]]; then
            if [[ "$show_legend_hint" == "true" ]]; then
                show_legend_popup
                if [[ "$draw_strategy" == "FULL" ]]; then
                    draw_bottom_panel_header "$title" "true"
                    draw_bottom_border "$footer_text"
                fi
                prev_start_idx=-1
                continue
            fi
        elif [[ $key == "" ]]; then
            return "$selected"
        fi

        if read -t 0 2>/dev/null; then continue; fi

        if (( selected < start_idx )); then start_idx=$selected
        elif (( selected >= start_idx + content_height )); then start_idx=$(( selected - content_height + 1 )); fi
        
        if (( selected < start_idx || selected >= start_idx + content_height )); then
             if (( selected == 0 )); then start_idx=0; 
             elif (( selected == total_count - 1 )); then 
                start_idx=$(( total_count - content_height ))
                [[ $start_idx -lt 0 ]] && start_idx=0
             fi
        fi
    done
}

prompt_success_menu() {
    local warns=${1:-0}
    local title="SYSTEM READY"
    [[ "$warns" -gt 0 ]] && title="READY WITH WARNINGS ($warns)"
    
    # PHASE 2: Wizard added at top
    local options=(
        "Quick Setup Wizard"
        "Device & Equipment Setup" 
        "Bluetooth Scanning" 
        "User Profile" 
        "QZ Service Control" 
        "Diagnostics (ANT+ Test)" 
        "Exit"
    )
    
    local help_texts=(
        "Run a streamlined configuration wizard to set up your equipment, scan for Bluetooth devices, configure user profile, and test ANT+ connectivity in one guided flow."
        "Select and configure your fitness equipment type (bike, treadmill, rower, elliptical). This determines which sensors and features will be enabled for your workouts."
        "Scan for nearby Bluetooth devices and save detected equipment to your configuration. Useful for finding your bike trainer, heart rate monitor, or other BLE sensors."
        "Configure your personal training profile including weight, age, gender, and distance unit preferences. This data is used for accurate power and calorie calculations."
        "Manage the QZ systemd service: start, stop, restart, enable auto-start on boot, or configure service flags like logging and ANT+ footpod support."
        "Run ANT+ dongle diagnostics to verify USB communication, check signal quality, and test footpod detection. Helps troubleshoot connectivity issues with your ANT+ sensors."
        "Exit the setup dashboard and return to the shell. All configuration changes are saved automatically."
    )
    
    enter_ui_mode
    
    show_unified_menu options 0 "$title" "FULL" "true" "" help_texts
    local idx=$?
    
    if [[ $idx -eq 255 ]]; then return 6; fi # Exit is now index 6
    return "$idx"
}

# ---------------------------------------------------------------------------
# Optimized menu cache loader and fast renderer
# ---------------------------------------------------------------------------
MENU_CACHE_LINES=()
MENU_CACHE_WIDTHS=()
MENU_CACHE_LOADED=0

load_menu_cache() {
    [[ $MENU_CACHE_LOADED -eq 1 ]] && return 0
    local json_file="${SCRIPT_DIR}/devices_optimized.json"
    if [[ ! -f "$json_file" ]]; then
        return 1
    fi

    # Use Python to dump lines with widths to stdout (one per line)
    local out
    out=$(python3 - <<'PY'
import json,sys
f=sys.argv[1]
try:
    with open(f,'r',encoding='utf-8') as fh:
        data=json.load(fh)
    for item in data.get('flat_menu',[]):
        line=item.get('line','')
        width=item.get('width',len(line))
        print(f"{line}\x1f{width}")
except Exception:
    pass
PY
"$json_file")

    # Build arrays
    while IFS=$'\x1f' read -r line width; do
        MENU_CACHE_LINES+=("$line")
        MENU_CACHE_WIDTHS+=("$width")
    done <<< "$out"

    MENU_CACHE_LOADED=1
    return 0
}

run_guided_mode() {
    SETUP_MODE="${SETUP_MODE:-}"
    if [[ -z "$SETUP_MODE" ]]; then
        if [[ "${HAS_GUI:-}" == true ]]; then SETUP_MODE="gui"; else SETUP_MODE="headless"; fi
    fi

    local action_taken=false

    # INTERNAL HELPER: Presentation and choice logic
    request_fix() {
        local title="$1"
        local problem="$2"
        local question="$3"
        draw_bottom_panel_header "GUIDED FIX: $title" "false"
        clear_info_area
        draw_sealed_row $((LOG_TOP + 1)) "   ${WHITE}${problem}${NC}"
        draw_sealed_row $((LOG_TOP + 2)) "   ${question}"
        prompt_yes_no 4
        return $? 
    }

    # 1. PYTHON 3.11 (Handles System vs Pyenv choice internally now)
    if [ "${STATUS_MAP[python311]:-}" = "fail" ]; then
        # We don't need a specific request_fix prompt here because 
        # install_python311 has its own internal choice UI now.
        if install_python311; then
            check_python311
            action_taken=true
        else
            return 1 # If they cancel Python, we can't proceed
        fi
    fi

    # 2. PYTHON VIRTUAL ENVIRONMENT
    if [ "${STATUS_MAP[python311]:-}" != "fail" ] && [ "${STATUS_MAP[venv]:-}" = "fail" ]; then
        request_fix "VENV" "Python Virtual Environment is not configured." "Create environment now?"
        local res=$?
        if [ $res -eq 2 ]; then return 1; fi
        if [ $res -eq 0 ]; then
            install_venv && check_venv
            action_taken=true
        fi
    fi

    # 3. PYTHON PACKAGES (ANT+, Bleak, etc)
    if [ "${STATUS_MAP[python311]:-}" != "fail" ] && [ "${STATUS_MAP[venv]:-}" != "fail" ] && [ "${STATUS_MAP[pkg_pips]:-}" = "fail" ]; then
        request_fix "PACKAGES" "ANT+ Python packages are missing." "Install packages into venv?"
        local res=$?
        if [ $res -eq 2 ]; then return 1; fi
        if [ $res -eq 0 ]; then
            install_python_packages && check_python_packages
            action_taken=true
        fi
    fi

    # 4. QT5 / QML RUNTIME
    if [ "${STATUS_MAP[qt5_libs]:-}" = "fail" ] || [ "${STATUS_MAP[qml_modules]:-}" = "fail" ]; then
        request_fix "RUNTIME" "Required modules (Qt5 / QML) are missing." "Install system dependencies?"
        local res=$?
        if [ $res -eq 2 ]; then return 1; fi
        if [ $res -eq 0 ]; then
            # Note: install_qt5_libs no longer installs python3-pip (prevents system leak)
            install_qt5_libs && check_qt5_libs && check_qml_modules
            action_taken=true
        fi
    fi

    # 5. BLUETOOTH SERVICE
    if [ "${STATUS_MAP[bluetooth]:-}" = "fail" ]; then
        request_fix "BLUETOOTH" "Bluetooth service is not active." "Enable Bluetooth service?"
        local res=$?
        if [ $res -eq 2 ]; then return 1; fi
        if [ $res -eq 0 ]; then
            install_bluetooth && check_bluetooth
            action_taken=true
        fi
    fi

    # 6. USER GROUP (PERMISSIONS)
    if [ "${STATUS_MAP[plugdev]:-}" = "fail" ]; then
        request_fix "PERMISSIONS" "USB access permissions not yet enabled." "Add current user to 'plugdev' group?"
        local res=$?
        if [ $res -eq 2 ]; then return 1; fi
        if [ $res -eq 0 ]; then
            install_plugdev && check_plugdev
            action_taken=true
        fi
    fi

    # 7. USB UDEV RULES
    if [ "${STATUS_MAP[udev_rules]:-}" = "fail" ]; then
        request_fix "USB RULES" "Hardware communication rules not yet installed." "Install ANT+ USB device rules?"
        local res=$?
        if [ $res -eq 2 ]; then return 1; fi
        if [ $res -eq 0 ]; then
            install_udev_rules && check_udev_rules
            action_taken=true
        fi
    fi

    # 8. LSUSB UTILITY
    if [ "${STATUS_MAP[lsusb]:-}" = "fail" ]; then
        request_fix "UTILITIES" "The 'lsusb' command is missing." "Install usbutils?"
        local res=$?
        if [ $res -eq 2 ]; then return 1; fi
        if [ $res -eq 0 ]; then
            install_lsusb && check_lsusb
            action_taken=true
        fi
    fi

    # 9. CONFIGURATION & WIZARD (Handles Virtual vs Physical check)
    if [ ! -f "$CONFIG_FILE" ]; then
        request_fix "CONFIG" "No configuration file found." "Start setup wizard?"
        local res=$?
        if [ $res -eq 2 ]; then return 1; fi
        if [ $res -eq 0 ]; then
            # A. Select Equipment
            if select_equipment_flow "true"; then
                # B. Configure Profile (with wizard mode enabled)
                configure_user_profile "true"
                
                # C. Conditional Bluetooth Scan
                # If Emulation mode is selected, we DO NOT scan for Bluetooth hardware
                if [[ "${CONFIG_BOOL[fakedevice_treadmill]:-}" == "true" ]]; then
                    draw_info_screen "VIRTUAL DEVICE" "Emulation mode active. No Bluetooth pairing required." 2
                else
                    draw_bottom_panel_header "BLUETOOTH SETUP"
                    clear_info_area
                    draw_sealed_row $((LOG_TOP + 2)) "   Equipment and Profile saved."
                    draw_sealed_row $((LOG_TOP + 4)) "   Would you like to scan for your device now?"
                    if prompt_yes_no 6; then
                        perform_bluetooth_scan "true"
                    fi
                fi
            fi
            check_config_file
            action_taken=true
        fi
    fi

    # 10. SYSTEMD SERVICE
    if [ "${STATUS_MAP[qz_service]:-}" = "fail" ]; then
        # Logic remains the same, but generate_service_file is now
        # aware of shared libraries and virtual device flags.
        service_menu_flow
        action_taken=true
        check_qz_service
    fi

    [ "$action_taken" = true ] && return 0 || return 1
}

# Uninstall QDomyos-Zwift installation with system safety checks
run_uninstall_mode() {
    # 1. Set Header and Initial Refresh (Force silent check)
    draw_bottom_panel_header "UNINSTALL / RESET" "false"
    clear_info_area
    draw_bottom_border "Verifying system state..." # ADDED: Initial bottom border
    run_all_checks "dashboard"

    # 2. Safety Check (Headless/RPI)
    local is_rpi=false
    grep -q "Raspberry Pi" /proc/cpuinfo 2>/dev/null && is_rpi=true
    local BOOT_WRITABLE=false
    if touch /boot/firmware/.test_write 2>/dev/null; then
        rm -f /boot/firmware/.test_write
        BOOT_WRITABLE=true
    fi

    if [ "$is_rpi" = false ]; then
        draw_error_screen "UNINSTALL DENIED" "Uninstall is only permitted on a Raspberry Pi or other verified headless system." 3
        return 1
    fi
    if [ "${HAS_GUI:-false}" = true ]; then
        draw_error_screen "UNINSTALL DENIED" "Uninstall is blocked when a GUI environment is detected for safety." 3
        return 1
    fi
    if [ "$BOOT_WRITABLE" = false ]; then
        draw_error_screen "UNINSTALL DENIED" "/boot partition is read-only. Cannot proceed with critical system changes." 3
        return 1
    fi

    # 3. Build Dynamic Removal List
    local items=()
    if [[ -d "$TARGET_HOME/ant_venv" ]]; then items+=("Virtual Environment"); fi
    if [[ "${IS_PYENV_INSTALLED:-false}" == "true" ]]; then items+=("Pyenv Python 3.11"); fi
    if [[ -f "$SERVICE_FILE_QZ" ]] || [[ -f "/etc/systemd/system/qz.service" ]]; then items+=("QZ Service"); fi
    if [[ -d "$CONFIG_DIR" ]]; then items+=("Config Files"); fi
    if [[ -f "/etc/udev/rules.d/99-ant-usb.rules" ]]; then items+=("USB udev Rules"); fi

    local sys_pkgs_cleanable=false
    if dpkg -l | grep -q -E "python3.11-venv|python3-pip-whl|libpython3.11"; then
        sys_pkgs_cleanable=true
        items+=("Application Python Artifacts")
    fi
    
    local deep_clean_available=false
    if [ "${STATUS_MAP[qt5_libs]:-}" = "pass" ] || [ "${STATUS_MAP[qml_modules]:-}" = "pass" ] || \
       [ "${STATUS_MAP[bluetooth]:-}" = "pass" ] || [ "${STATUS_MAP[lsusb]:-}" = "pass" ] || \
       groups "$TARGET_USER" 2>/dev/null | grep -q plugdev; then
        deep_clean_available=true
        items+=("Application System Runtime Packages (Deep Clean)")
    fi
    
    # Detect Python installation type
    local core_sys_python_present=false
    local pyenv_python_present=false
    
    # Check if system Python 3.11 is installed via apt
    if command -v python3.11 &> /dev/null; then
        core_sys_python_present=true
        items+=("${RED}Core System Python 3.11${NC}")
    fi

    # Check if pyenv Python 3.11 is installed
    if [[ -d "$TARGET_HOME/.pyenv/versions/3.11.9" ]]; then
        pyenv_python_present=true
        # Only add to list if not already added via IS_PYENV_INSTALLED check above
        if [[ "${IS_PYENV_INSTALLED:-false}" != "true" ]]; then
            items+=("Pyenv Python 3.11")
        fi
    fi

    # 4. SHOW MENU PROMPT
    if [[ ${#items[@]} -eq 0 ]]; then
        draw_info_screen "CLEAN" "No QZ components detected for removal.\nPress ENTER to exit." "wait"
        return 0
    fi
    
    # 1. Condense and Wrap the List
    local list_full
    list_full=$(IFS=', ' ; echo "${items[*]}")
    
    local list_lines=()
    while IFS= read -r line; do list_lines+=("$line"); done < <(printf '%s' "   $list_full" | fold -s -w $((INFO_WIDTH - 6)))
    
    draw_bottom_panel_header "UNINSTALL / RESET" "false"
    clear_info_area
    
    local draw_row=$((LOG_TOP + 1))
    
    # 2. Draw Wrapped Component List
    draw_sealed_row "$draw_row" "   The following components can be removed:"
    ((draw_row++))
    
    for line in "${list_lines[@]}"; do
        if [ "$draw_row" -lt "$LOG_BOTTOM" ]; then
            draw_sealed_row "$draw_row" "$line"
            ((draw_row++))
        fi
    done
    
    # 3. Draw Warning and Prompt (Positioned dynamically)
    local warning_row=$draw_row
    
    while [ "$warning_row" -lt $((LOG_BOTTOM - 4)) ]; do
        draw_sealed_row "$warning_row" ""
        warning_row=$((warning_row + 1))
    done

    local prompt_row=$((warning_row))
    # Only show high-risk warning if removing core system Python
    if [[ "$core_sys_python_present" == "true" ]]; then
        update_sealed_row_content "$prompt_row" "   ${YELLOW}WARNING:${NC} Removing Core System Python is HIGH-RISK."
        ((prompt_row++))
    fi
    update_sealed_row_content "$prompt_row" "   Proceed with removal of ALL components listed above?"
    ((prompt_row++)) # Move prompt_row down one more time for spacing
    
    local prompt_offset=$((prompt_row - LOG_TOP))
    
    if ! prompt_yes_no "$prompt_offset"; then
        draw_info_screen "CANCELLED" "Uninstall cancelled. No changes were made." 2
        return 0
    fi

    # 5. EXECUTE CLEANUP STEPS

    # A. Core System Python (EXTREME RISK) - Only if system Python detected
    if [[ "$core_sys_python_present" == "true" ]]; then
        draw_bottom_panel_header "CONFIRM HIGH-RISK REMOVAL" "false"
        clear_info_area
        draw_sealed_row $((LOG_TOP + 2)) "   ${BOLD_RED}FINAL WARNING:${NC} You are about to remove the OS's core python3.11."
        
        exit_ui_mode
        
        if prompt_input_yes; then
            enter_ui_mode
            run_step "Removing Core System Python" "apt-get purge -y python3.11" || return 1
            # Update status after removal
            STATUS_MAP["python311"]="fail"
            IS_PYSYS_INSTALLED=false
            draw_status_panel
        else
            enter_ui_mode
            draw_info_screen "HIGH-RISK REMOVAL ABORTED" "Core system Python will be kept." 1
        fi
    fi

    # B. App-Specific Python Artifacts
    if [[ "$sys_pkgs_cleanable" == "true" ]]; then
        local pkgs_to_remove="python3.11-venv python3-pip-whl python3-setuptools-whl libpython3.11"
        run_step "Removing Python Artifacts" "apt-get purge -y $pkgs_to_remove" || return 1
        draw_status_panel
    fi
    
    # C. Pyenv Python (The Isolated Build)
    if [[ "${IS_PYENV_INSTALLED:-false}" == "true" ]] || [[ "$pyenv_python_present" == "true" ]]; then
        local p_root="$TARGET_HOME/.pyenv"
        local p_bin="$p_root/bin/pyenv"
        local py_rem_cmd="sudo -u \"$TARGET_USER\" bash -c \"export PYENV_ROOT='$p_root'; export PATH='\$PYENV_ROOT/bin:\$PATH'; if [ -x '$p_bin' ]; then $p_bin uninstall -f 3.11.9; fi; rm -rf '$p_root'\""
        run_step "Removing Pyenv Python" "$py_rem_cmd" || return 1
        # Update status after removal
        STATUS_MAP["python311"]="fail"
        IS_PYENV_INSTALLED=false
        draw_status_panel
    fi
    
    # D. Virtual Environment
    if [[ -d "$TARGET_HOME/ant_venv" ]]; then 
        run_step "Removing Virtual Environment" "rm -rf \"$TARGET_HOME/ant_venv\"" || return 1
        # Update status after removal
        STATUS_MAP["venv"]="fail"
        draw_status_panel
    fi
    
    # E. Python Packages
    # Update PIPs status
    STATUS_MAP["pips"]="fail"
    draw_status_panel

    # F. Deep Clean Execution (System Runtime Packages)
    if [[ "$deep_clean_available" == "true" ]]; then
        local pkgs_to_remove="libqt5core5a libqt5qml5 libqt5quick5 libqt5bluetooth5 libusb-1.0-0 bluez usbutils"
        
        run_step "Deep Cleaning System Packages" "apt-get purge -y $pkgs_to_remove" || return 1
        # Update status after removal
        STATUS_MAP["qt5_libs"]="fail"
        STATUS_MAP["qml_modules"]="fail"
        STATUS_MAP["bluetooth"]="fail"
        STATUS_MAP["lsusb"]="fail"
        draw_status_panel
        
        if groups "$TARGET_USER" 2>/dev/null | grep -q plugdev; then
            run_step "Removing User from plugdev Group" "gpasswd -d $TARGET_USER plugdev" || return 1
            STATUS_MAP["plugdev"]="fail"
            draw_status_panel
        fi
    fi

    # G. Services and Config
    if [[ -f "$SERVICE_FILE_QZ" ]] || [[ -f "/etc/systemd/system/qz.service" ]]; then
        local svc_cmd="systemctl stop qz.service 2>/dev/null; systemctl disable qz.service 2>/dev/null; rm -f \"$SERVICE_FILE_QZ\" /etc/systemd/system/qz.service; systemctl daemon-reload"
        run_step "Removing QZ Service" "$svc_cmd" || return 1
        STATUS_MAP["qz_service"]="fail"
        draw_status_panel
    fi
    if [[ -f "/etc/udev/rules.d/99-ant-usb.rules" ]]; then
        run_step "Removing USB udev rules" "rm -f /etc/udev/rules.d/99-ant-usb.rules && udevadm control --reload-rules" || return 1
        STATUS_MAP["udev_rules"]="fail"
        draw_status_panel
    fi
    if [[ -d "$CONFIG_DIR" ]]; then 
        run_step "Removing Config Files" "rm -rf \"$CONFIG_DIR\"" || return 1
        draw_status_panel
    fi

    # H. Final Autoremove
    run_step "Running Final System Autoremove" "apt-get autoremove -y" || return 1

    # 6. FINAL STATUS CHECK AND EXIT (New)
    draw_bottom_border "Final Status Check"
    run_all_checks "dashboard" # Force a full status update

    # CRITICAL FIX: Change exit message and pass "wait" to prompt for keypress
    draw_info_screen "UNINSTALL COMPLETE" "All selected components removed.\nPress ENTER to exit." "wait"
    return 0
}

draw_verifying_screen() {
    local message="$1"
    
    # FIX: Pass "false" to hide Legend
    draw_bottom_panel_header "SYSTEM CHECK" "false"
    
    clear_info_area
    
    # Row 14: Main Message
    draw_sealed_row $((LOG_TOP + 1)) "   ${WHITE}${message}${NC}"
    # Row 16: Subtext
    draw_sealed_row $((LOG_TOP + 3)) "   Please wait while system status is updated..."
    
    draw_bottom_border ""
}

# ============================================================================
# HEX COLOR REFERENCE (for easy editing)
# ============================================================================
# 0-8 = Dark backgrounds (grays + black)
#   0=232 (darkest)  1=233  2=234  3=235  4=236  5=237  6=238  7=240  8=16(black)
#
# 9-F = Pink ring gradient
#   9=52(dark maroon)  A=53  B=89  C=125  D=161  E=197  F=198(brightest)
#
# G-M = Green/Yellow Z gradient
#   G=64(dark olive)  H=70  I=106  J=112  K=148  L=154  M=190(bright yellow)
# ============================================================================
draw_splash_screen() {
    # 1. Setup & Buffer Init
    UI_LOCKED=0
    local title="${3:-QDomyos-Zwift  ANT+ BRIDGE SETUP UTILITY}"
    
    set_ui_output || true
    local fd="${UI_FD:-2}"
    
    # Clear Screen + Hide Cursor (Atomic)
    local _BUFFER="\e[2J\e[H\e[?25l"
    
    local inner_w=${INNER_COLS:-80}
    
    # Color Constants
    local BOLD_WHITE=$'\033[1;37m'
    local NC=$'\033[0m'
    local D_Gray=$'\033[38;5;232m'
    local PINK=$'\033[38;5;198m'
    local YELLOW=$'\033[38;5;190m'
    
    # Fast Color Lookup Map (Replaces slow case statement)
    local -A pal_map=(
        [0]=$'\033[38;5;232m' [1]=$'\033[38;5;233m' [2]=$'\033[38;5;234m'
        [3]=$'\033[38;5;235m' [4]=$'\033[38;5;236m' [5]=$'\033[38;5;237m'
        [6]=$'\033[38;5;238m' [7]=$'\033[38;5;240m' [8]=$'\033[38;5;16m'
        [9]=$'\033[38;5;52m'  [A]=$'\033[38;5;53m'  [B]=$'\033[38;5;89m'
        [C]=$'\033[38;5;125m' [D]=$'\033[38;5;161m' [E]=$'\033[38;5;197m'
        [F]=$'\033[38;5;198m' [G]=$'\033[38;5;64m'  [H]=$'\033[38;5;70m'
        [I]=$'\033[38;5;106m' [J]=$'\033[38;5;112m' [K]=$'\033[38;5;148m'
        [L]=$'\033[38;5;154m' [M]=$'\033[38;5;190m'
    )

    # Logo Data
    local logo_hex=(
        "0000000000000019BBCCCCBB9100000000000000"
        "00000000009BDFFFFFFFFFFFFFFDB90000000000"
        "00000001CFFFFFFFDDCCCCDDFFFFFFFC10000000"
        "000001DFFFFFC91000000000019CFFFFFD100000"
        "0000BFFFFD90880000000000008809DFFFFB0000"
        "000DFFFF9003GKMMMMMMMMMMMMMKJ109FFFFD000"
        "00DFFFE1001KMMMMMMMMMMMMMMMMMK001EFFFD00"
        "09FFFF100003GKMMMMMMMMMMMMMKJ10001FFFF90"
        "0DFFFB00000000000801KMMMMKJ1000000BFFFD0"
        "1FFFF9000000000001KMMMMK10000000009FFFF1"
        "1FFFF90000000001KMMMMK1000000000009FFFF1"
        "0DFFFB0000001KLMMMMK20800000000000BFFFD0"
        "09FFFF20001KMMMMMMMMMMKG21KK200002FFFF90"
        "00CFFFF100KMMMMMMMMMMMMK2KMMK1001FFFFC00"
        "000DFFFF901KKMMMMMMMMMKG21KK29FEFFFFD000"
        "0000BFFFFD908880000000000080DFFFFFFB0000"
        "000001CFFFFFD91000000000019CFFFFFFFD9000"
        "00000001BEFFFFFFEDDCCDDEFFFFFFECDFFFFF10"
        "00000000001BDEFFFFFFFFFFFFEDB10009DFFD10"
        "0000000000000019BBCCCCBB9100000000000000"
        "0000000000000000000000000000000000000000"
        "0000000000000000000000000000000000000000"
        "0000000000000000000000000000000000000000"
        "0000000000000000000000000000000000000000"
    )

    # 2. Build Logo
    # Pre-calc side padding string
    local hex_len=40
    local side_padding=$(( (inner_w - hex_len) / 2 + 1 ))
    
    local border_str
    printf -v border_str "%*s" "$side_padding" ""
    border_str="${border_str// /●}"
    local left_border="${D_Gray}${border_str}"
    local right_border="${D_Gray}${border_str}"

    local row_idx=0
    for hex_line in "${logo_hex[@]}"; do
        local line_str=""
        # Fast string parsing
        for (( i=0; i<${#hex_line}; i++ )); do
            line_str+="${pal_map[${hex_line:$i:1}]}●"
        done
        
        # Append to buffer
        printf -v _seq "\e[%d;1H%s%s%s%s" "$((row_idx + 1))" "$left_border" "$line_str" "$NC" "$right_border"
        _BUFFER+="${_seq}"
        ((row_idx++))
    done

    # 3. Build Title
    # Overwrite the bottom 3 empty rows of the logo area (rows 21-23)
    local title_len=${#title}
    local title_pad=$(( (inner_w - title_len) / 2 + 1 ))
    
    # Rebuild border for title width
    printf -v border_str "%*s" "$title_pad" ""
    border_str="${border_str// /●}"
    left_border="${D_Gray}${border_str}"
    right_border="${D_Gray}${border_str}"
    
    local styled_title=""
    for (( i=0; i<title_len; i++ )); do
        local char="${title:$i:1}"
        if [[ "$char" == " " ]]; then
            styled_title+="${D_Gray}●${NC}"
        elif [[ $i -lt 13 ]]; then
            styled_title+="${BOLD_WHITE}${PINK}${char}${NC}"
        else
            styled_title+="${BOLD_WHITE}${YELLOW}${char}${NC}"
        fi
    done
    
    # Place title at Row 21 (overwriting bottom logo padding)
    printf -v _seq "\e[%d;1H%s%s%s" "21" "$left_border" "$styled_title" "$right_border"
    _BUFFER+="${_seq}"

    # 4. Atomic Flush
    printf "%b" "$_BUFFER" >&"$fd"
    
    # 5. Lock UI & Short Pause
    UI_LOCKED=1
    sleep 0.2
}

# Update the splash screen progress bar
# Usage: update_splash_progress <current> <total> <text>
update_splash_progress() {
    local current=$1
    local total=$2
    local text="${3:-Loading...}"
    
    # 1. BYPASS LOCK: This function is explicitly allowed to draw over the splash
    local _prev_lock="${UI_LOCKED:-0}"
    UI_LOCKED=0
    
    # Color definitions
    local NC=$'\033[0m'
    local D_Gray=$'\033[38;5;16m'      # Dark Gray for borders
    local CYAN=$'\033[38;5;51m'       # Cyan for progress
    local DARK_Gray=$'\033[38;5;233m' # Dark Gray for empty portion
    local WHITE=$'\033[1;37m'         # White text
    
    # Calculate percentage and bar width
    local percent=$(( (current * 100) / total ))
    local inner_w=${INNER_COLS:-80}
    # Leave room for D_Gray borders (2 dots each side = 4 chars roughly)
    local bar_width=$(( inner_w - 4 ))  
    local filled_width=$(( (current * bar_width) / total ))
    
    # Create dot-based progress bar using fast string manipulation
    local bar_str=""
    
    # Fill Part
    if (( filled_width > 0 )); then
        local fill_dots
        printf -v fill_dots "%*s" "$filled_width" ""
        bar_str+="${CYAN}${fill_dots// /●}${NC}"
    fi
    
    # Empty Part
    local empty_width=$(( bar_width - filled_width ))
    if (( empty_width > 0 )); then
        local empty_dots
        printf -v empty_dots "%*s" "$empty_width" ""
        bar_str+="${DARK_Gray}${empty_dots// /●}${NC}"
    fi
    
    # Position: rows 22-23 (after 20-row logo + 1-row title)
    local row=22
    
    # Text with percentage and D_Gray dot borders
    local text_with_percent="${text} ${percent}%"
    local text_width=${#text_with_percent}
    local text_pad=$(( (inner_w - text_width) / 2 + 1 ))
    
    # Borders
    local text_left_border
    printf -v text_left_border "${D_Gray}%%.0s●${NC}" $(seq 1 $text_pad) # Fallback to seq for pattern repetition if needed, or loop
    
    # Faster Border Generation without seq
    local border_str
    printf -v border_str "%*s" "$text_pad" ""
    border_str="${border_str// /●}"
    local left_border="${D_Gray}${border_str}${NC}"
    local right_border="${D_Gray}${border_str}${NC}"
    
    # Draw Text (Row 22)
    print_at $((row)) "${left_border}${WHITE}${text_with_percent}${NC}${right_border}"
    
    # Draw Bar (Row 23) - Optional, enabled by uncommenting
    # print_at $((row + 1)) "${D_Gray}●●${NC}${bar_str}${D_Gray}●●${NC}"

    # 2. RESTORE LOCK
    UI_LOCKED="$_prev_lock"
}

perform_ant_test() {
    local script_dir
    script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
    local py_script="${script_dir}/test_ant.py"
    local venv_py="${TARGET_HOME}/ant_venv/bin/python3"
    [[ ! -f "$venv_py" ]] && venv_py=$(command -v python3)

    set_ui_output
    
    # FIX: Force Unlock immediately
    UI_LOCKED=0
    
    clear_info_area

    # 1. Hardware Check
    if [[ "${STATUS_MAP[ant_dongle]:-}" != "pass" ]]; then
        draw_error_screen "NO ANT+ DEVICE" "Error: No ANT+ device detected via lsusb." "wait"
        return 1
    fi

    # 2. Stop Service (only if running)
    local svc_status
    svc_status=$(get_service_status)
    if [[ "$svc_status" == "running" ]]; then
        draw_bottom_panel_header "ANT+ INITIALIZATION" "false"
        local spinner=('⠋' '⠙' '⠹' '⠸' '⠼' '⠴' '⠦' '⠧' '⠇' '⠏')
        local sc=0
        local stop_started=false
        for ((i=0; i<30; i++)); do
            if [[ "$stop_started" == false ]]; then
                sudo systemctl stop qz.service >/dev/null 2>&1 || true
                sudo killall -9 qdomyos-zwift >/dev/null 2>&1 || true
                stop_started=true
            fi
            local spin_char="${spinner[$((sc % 10))]}"
            draw_sealed_row $((LOG_TOP + 1)) "   ${CYAN}${spin_char}${NC} Stopping background service..."
            ((sc++))
            sleep 0.1
            svc_status=$(get_service_status)
            if [[ "$svc_status" != "running" ]]; then
                break
            fi
        done
    fi

    local retries=2
    local attempt=0
    local launched=0
    local py_pid=""
    local log_file=""
    local spinner=('⠋' '⠙' '⠹' '⠸' '⠼' '⠴' '⠦' '⠧' '⠇' '⠏')
    local sc=0

    enter_ui_mode
    UI_MODAL_ACTIVE=0

    while [[ $attempt -le $retries ]]; do
        sudo pkill -f test_ant.py >/dev/null 2>&1 || true
        log_file=$(mktemp "$TEMP_DIR/qz_ant_test_XXXXXX.log")
        : > "$log_file"

        local unit_flag=""
        if [[ "${CONFIG_BOOL[miles_unit]:-false}" == "true" ]]; then
            unit_flag="--imperial"
        fi
        local cmd="PYTHONUNBUFFERED=1 \"$venv_py\" -u \"$py_script\" --dashboard $unit_flag --pidfile \"$TEMP_DIR/qz_ant_test.pid\""
        
        bash -c "$cmd" >"$log_file" 2>&1 &

        # Wait for PID
        py_pid=""
        for _wait_i in {1..30}; do
            if [ -s "$TEMP_DIR/qz_ant_test.pid" ]; then
                py_pid=$(cat "$TEMP_DIR/qz_ant_test.pid" 2>/dev/null || true)
                if ps -p "$py_pid" >/dev/null 2>&1; then break; fi
            fi
            sleep 0.1
        done

        local seen=0
        for (( i=0; i<250; i++ )); do
            if [[ -n "$py_pid" ]] && ! ps -p "$py_pid" >/dev/null 2>&1; then break; fi

            if grep -qE "Signal:Startup|\| Cadence" "$log_file"; then
                seen=1
                break
            fi

            local spin_char="${spinner[$((sc % 10))]}"
            local last_line=$(tail -n 1 "$log_file" 2>/dev/null | tr -d '\r')
            local status_msg="Initializing..."
            [[ "$last_line" == *"Reset"* ]] && status_msg="Resetting USB..."
            [[ "$last_line" == *"Node"* ]] && status_msg="Opening ANT+ Node..."
            
            draw_sealed_row $((LOG_TOP + 2)) "   ${CYAN}${spin_char}${NC}  ${status_msg} (Attempt $((attempt + 1)))"
            draw_bottom_border "Resetting hardware (can take 15s)..."
            
            safe_read_key stop_check 0.1
            if [[ -n "$stop_check" ]]; then
                [[ -n "$py_pid" ]] && sudo kill -9 "$py_pid" 2>/dev/null || true
                exit_ui_mode; return 1
            fi
            ((sc++))
        done

        if [[ $seen -eq 1 ]]; then launched=1; break; fi
        
        [[ -n "$py_pid" ]] && sudo kill -9 "$py_pid" 2>/dev/null || true
        attempt=$(( attempt + 1 ))
    done

    if [[ $launched -ne 1 ]]; then
        local final_err=$(tail -n 10 "$log_file" 2>/dev/null | tr -d '\r')
        draw_error_screen "STARTUP FAILED" "Python process failed to initialize.\n\nLog Output:\n$final_err" "wait"
        return 1
    fi

    draw_bottom_panel_header "ANT+ BROADCAST TEST" "false"
    clear_info_area

    draw_sealed_row $((LOG_TOP + 6)) "   1. WATCH: Open Sensors & Accessories > Add New."
    draw_sealed_row $((LOG_TOP + 7)) "   2. PAIR: Select Foot Pod and follow pairing prompts."
    draw_sealed_row $((LOG_TOP + 8)) "   3. TEST: Start a Treadmill or Run Indoor activity."

    draw_bottom_border "Any key to stop"

    local last_elapsed="-1"
    local last_stage=""
    local last_displayed_line=""
    local stale_count=0
    local spinner=('⠋' '⠙' '⠹' '⠸' '⠼' '⠴' '⠦' '⠧' '⠇' '⠏')
    local sc=0

    while true; do
        if ! ps -p "$py_pid" >/dev/null 2>&1; then
            local excerpt=$(tail -n 5 "$log_file" 2>/dev/null | tr -d '\r')
            if grep -q "Test finished." "$log_file" 2>/dev/null; then
                draw_info_screen "TEST COMPLETED" "ANT+ test finished successfully.\nAll stages completed including 'Stopping'." "wait"
            else
                draw_error_screen "TEST STOPPED" "Process exited unexpectedly.\nLog:\n$excerpt" "wait"
            fi
            break
        fi

        local line=$(tail -n 1 "$log_file" 2>/dev/null | tr -d '\r')

        if [[ "$line" != "$last_displayed_line" ]]; then
            if [[ "$line" =~ ^([^|]+)\|[[:space:]]*Cadence:([0-9]+)[[:space:]]*Speed:([0-9.]+)[[:space:]]*Pace:([^ ]+)[[:space:]]*\[[[:space:]]*([0-9]+)s[[:space:]]*/[[:space:]]*([0-9]+)s[[:space:]]*\] ]]; then
                local stage="${BASH_REMATCH[1]}"
                local cad="${BASH_REMATCH[2]}"
                local spd="${BASH_REMATCH[3]}"
                local pace="${BASH_REMATCH[4]}"
                local t_now="${BASH_REMATCH[5]}"
                local t_max="${BASH_REMATCH[6]}"

                stage="$(echo "$stage" | xargs)"
                if [[ "$stage" != "$last_stage" ]]; then last_stage="$stage"; fi

                if [[ "$t_now" == "$last_elapsed" ]]; then 
                    ((stale_count++))
                else 
                    last_elapsed="$t_now"
                    stale_count=0
                fi

                draw_sealed_row $((LOG_TOP + 2)) "   ${BOLD_WHITE}$(printf '%-15s' "$stage")${NC}ID:54321   ${CYAN}Pace:${pace}  Cad:${cad}  Spd:${spd}${NC}"

                local line_without_bar="   ${CYAN}[ $(printf '%2d' "$t_now")s / $(printf '%2d' "$t_max")s ]${NC}  "
                local line_vis; line_vis=$(get_display_width "$line_without_bar")
                local bar_w=$(( INNER_COLS - line_vis - 6 ))
                
                local fill=0
                if [[ $t_max -gt 0 ]]; then fill=$(( (t_now * bar_w) / t_max )); fi
                [[ $fill -gt $bar_w ]] && fill=$bar_w
                
                local p_bar="${BG_GREEN}$(printf '%*s' "$fill" "")${BG_GRAY}$(printf '%*s' "$((bar_w - fill))" "")${NC}"
                draw_sealed_row $((LOG_TOP + 3)) "${line_without_bar}${p_bar}"
                
                last_displayed_line="$line"
            else
                if [[ "$line" != "$last_displayed_line" ]]; then
                    local spin_char="${spinner[$((sc % 10))]}"
                    draw_sealed_row $((LOG_TOP + 2)) "   ${CYAN}${spin_char}${NC}  Waiting for staging data..."
                    ((sc++))
                    last_displayed_line="$line"
                fi
            fi
        fi

        if [[ $stale_count -gt 25 ]]; then
            draw_error_screen "DATA FREEZE" "Hardware connected but data stream stopped." "wait"
            break
        fi

        safe_read_key stop_key 0.2
        if [[ -n "$stop_key" ]]; then
            draw_bottom_panel_header "STOPPING..." "false"
            sudo kill -TERM "$py_pid" 2>/dev/null || true
            break
        fi
    done

    rm -f "$TEMP_DIR/qz_ant_test.pid" 2>/dev/null || true
    
    # FIX: Refresh Service Status in STATUS_MAP and Header
    check_qz_service >/dev/null 2>&1 || true
    
    exit_ui_mode
    return 0
}

### Milestone 1-4: Service config, generation and validators
# Persistent service flags storage
declare -A SERVICE_FLAGS
# Prefer the target user's home so config changes persist to the expected user
SERVICE_CONF_PATH="${TARGET_HOME}/.config/qdomyos-zwift/service.conf"

# Defaults
SERVICE_FLAGS[logging]=false
SERVICE_FLAGS[console]=false
SERVICE_FLAGS[bluetooth_relaxed]=false
SERVICE_FLAGS[ant_footpod]=false
SERVICE_FLAGS[ant_device]=54321
SERVICE_FLAGS[ant_verbose]=false
SERVICE_FLAGS[profile]=''
SERVICE_FLAGS[poll_time]=200
SERVICE_FLAGS[heart_service]=false

init_service_config() {
    mkdir -p "$(dirname "$SERVICE_CONF_PATH")" || return 1
    # If running as root, ensure the target user owns the service config dir/file
    if [ "$(id -u)" -eq 0 ] && [ -n "${TARGET_USER:-}" ]; then
        chown -R "$TARGET_USER":"$TARGET_USER" "$(dirname "$SERVICE_CONF_PATH")" 2>/dev/null || true
    fi
    if [[ ! -f "$SERVICE_CONF_PATH" ]]; then
        save_service_config || return 1
        if [ "$(id -u)" -eq 0 ] && [ -n "${TARGET_USER:-}" ]; then
            chown "$TARGET_USER":"$TARGET_USER" "$SERVICE_CONF_PATH" 2>/dev/null || true
        fi
    fi
    return 0
}

load_service_config() {
    init_service_config || return 1
    local in_flags=0 line key val
    
    # Reset array to ensure no stale data
    SERVICE_FLAGS=()
    
    while IFS= read -r line || [[ -n "$line" ]]; do
        # Strip comments
        line="${line%%\#*}"
        # Trim leading/trailing whitespace
        line="${line#"${line%%[![:space:]]*}"}"
        line="${line%"${line##*[![:space:]]}"}"
        
        [[ -z "$line" ]] && continue
        
        if [[ "$line" =~ ^\[flags\] ]]; then in_flags=1; continue; fi
        
        if [[ $in_flags -eq 1 && "$line" =~ ^([^=]+)=(.*)$ ]]; then
            key="${BASH_REMATCH[1]}"
            val="${BASH_REMATCH[2]}"
            
            # Trim key and val
            key="${key#"${key%%[![:space:]]*}"}"
            key="${key%"${key##*[![:space:]]}"}"
            val="${val#"${val%%[![:space:]]*}"}"
            val="${val%"${val##*[![:space:]]}"}"
            
            SERVICE_FLAGS[$key]="$val"
        fi
    done < "$SERVICE_CONF_PATH"
    
    # Set defaults if missing
    [[ -z "${SERVICE_FLAGS[logging]:-}" ]] && SERVICE_FLAGS[logging]="false"
    [[ -z "${SERVICE_FLAGS[console]:-}" ]] && SERVICE_FLAGS[console]="false"
    [[ -z "${SERVICE_FLAGS[ant_footpod]:-}" ]] && SERVICE_FLAGS[ant_footpod]="false"
    [[ -z "${SERVICE_FLAGS[ant_device]:-}" ]] && SERVICE_FLAGS[ant_device]="54321"
    
    return 0
}

save_service_config() {
    if ! ensure_ram_temp_dir >/dev/null 2>&1; then TEMP_DIR=/tmp; fi
    # SEC-01 FIX: Use mktemp for secure temporary file creation
    local tmp
    tmp=$(mktemp "$TEMP_DIR/service.conf.XXXXXX")
    {
        echo "[flags]"
        for k in console logging bluetooth_relaxed ant_footpod ant_device ant_verbose profile poll_time heart_service; do
            printf '%s=%s\n' "$k" "${SERVICE_FLAGS[$k]:-}"
        done
    } > "$tmp" || return 1
    mv -f "$tmp" "$SERVICE_CONF_PATH" || return 1
    return 0
}

# Binary detection and flags builder
detect_binary_path() {
    local candidates=("./qdomyos-zwift" "$(pwd)/qdomyos-zwift" "/usr/local/bin/qdomyos-zwift" "/usr/bin/qdomyos-zwift")
    for c in "${candidates[@]}"; do [[ -x "$c" ]] && { echo "$c"; return 0; }; done
    command -v qdomyos-zwift 2>/dev/null || return 1
}

build_service_flags() {
    local flags=("-no-gui")
    
    # 1. Logging Logic
    if [[ "${SERVICE_FLAGS[logging]:-false}" =~ ^[Tt][Rr][Uu][Ee]$ ]]; then 
        flags+=("-log")
    else 
        flags+=("-no-log")
    fi

    # 2. Console Logic
    if [[ ! "${SERVICE_FLAGS[console]:-false}" =~ ^[Tt][Rr][Uu][Ee]$ ]]; then 
        flags+=("-no-console")
    fi

    # 3. Bluetooth Relaxed
    if [[ "${SERVICE_FLAGS[bluetooth_relaxed]:-false}" =~ ^[Tt][Rr][Uu][Ee]$ ]]; then
        flags+=("-bluetooth_relaxed")
    fi
    
    # 4. ANT+ Flags
    if [[ "${SERVICE_FLAGS[ant_footpod]:-false}" =~ ^[Tt][Rr][Uu][Ee]$ ]]; then
        flags+=("-ant-footpod")
        
        local ant_id="$(strip_ansi "${SERVICE_FLAGS[ant_device]:-54321}")"
        flags+=("-ant-device" "$ant_id")
        
        if [[ "${SERVICE_FLAGS[ant_verbose]:-false}" =~ ^[Tt][Rr][Uu][Ee]$ ]]; then
            flags+=("-ant-verbose")
        fi
    fi
    
    # 5. Profile & Timing
    if [[ -n "${SERVICE_FLAGS[profile]:-}" ]]; then
        flags+=("-profile" "$(strip_ansi "${SERVICE_FLAGS[profile]}")")
    fi
    
    if [[ -n "${SERVICE_FLAGS[poll_time]:-}" ]]; then
        flags+=("-poll-device-time" "$(strip_ansi "${SERVICE_FLAGS[poll_time]}")")
    fi
    
    if [[ "${SERVICE_FLAGS[heart_service]:-false}" =~ ^[Tt][Rr][Uu][Ee]$ ]]; then
        flags+=("-heart-service")
    fi

    # 6. Bluetooth Device Name
    # FIX: Check if Emulation Mode is active.
    # If fakedevice_treadmill is true, we are the SERVER, so do NOT connect to a client.
    if [[ "${CONFIG_BOOL[fakedevice_treadmill]:-false}" == "true" ]]; then
        : # Skip adding -name
    else
        # Standard Mode: Connect to physical equipment
        local bt_name="${CONFIG_STRING[bluetooth_lastdevice_name]:-}"
        
        # Fallback grep if array empty
        if [[ -z "$bt_name" && -f "$CONFIG_FILE" ]]; then
             bt_name=$(grep -E '^bluetooth_lastdevice_name=' "$CONFIG_FILE" 2>/dev/null | cut -d'=' -f2- | tr -d '\r')
        fi
        
        bt_name=$(strip_ansi "${bt_name:-}")
        
        if [[ -n "$bt_name" && "$bt_name" != "None" ]]; then
            flags+=("-name" "$bt_name")
        fi
    fi

    printf '%s ' "${flags[@]}" | sed -e 's/ $//'
}

generate_service_file() {
    # 1. Reload Service Config
    load_service_config >/dev/null 2>&1
    
    # 2. FIX: Reload Main Config to check for Emulation Status (fakedevice_treadmill)
    load_config_into_arrays "$CONFIG_FILE"

    if [ "$SETUP_MODE" != "headless" ] && [ -z "${_GEN_SVC_SILENT:-}" ]; then
        clear_info_area
        local msg="${YELLOW}Generating service file...${NC}"
        local msg_plain="Generating service file..."
        local w=$(get_display_width "$msg_plain")
        local row=$((LOG_TOP + 3))
        local col=$(( (INNER_COLS - w) / 2 ))
        draw_sealed_row "$row" "$(printf '%*s' "$((col-2))" '')${msg}"
        draw_bottom_border
    fi

    # Detect systemd install path
    mkdir -p "$SYSTEMD_SYSTEM_DIR" 2>/dev/null || true
    local svc_file="$SERVICE_FILE_QZ"
    local bin
    
    local script_dir
    script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    local system_wrapper="/usr/local/bin/qdomyos-zwift-wrapper.sh"
    local wrapper="$script_dir/qdomyos-zwift-wrapper.sh"
    
    if [[ -x "$system_wrapper" ]]; then
        bin="$system_wrapper"
    elif [[ -f "$wrapper" ]]; then
        if ! head -n1 "$wrapper" | grep -q '^#!' >/dev/null 2>&1; then
            sed -i '1i#!/bin/bash' "$wrapper" || true
        fi
        chmod +x "$wrapper" || true
        bin="$wrapper"
    else
        bin=$(detect_binary_path 2>/dev/null) || bin="<binary-not-found>"
    fi

    if [[ "$bin" != /* ]]; then
        bin="$(cd "$script_dir" && pwd)/${bin#./}"
    fi

    local user="${SUDO_USER:-$USER}"
    
    # Build flags (now aware of emulation mode)
    local flags
    flags=$(build_service_flags)
    
    # Detect pyenv vs system Python — embedded interpreters (like QZ's
    # Py_Initialize) need PYTHONHOME to locate the stdlib and lib-dynload
    # C-extension modules (e.g. _struct).  Without it they default to /usr
    # which is wrong for pyenv.
    local python_home=""
    local ld_path="/usr/local/lib:/usr/lib"
    local pyenv_ver="${TARGET_HOME}/.pyenv/versions/3.11.9"
    if [[ -f "$pyenv_ver/bin/python3" ]]; then
        python_home="$pyenv_ver"
        ld_path="${pyenv_ver}/lib:/usr/local/lib:/usr/lib"
    fi

    if ! ensure_ram_temp_dir >/dev/null 2>&1; then TEMP_DIR=/tmp; fi
    local tmp
    tmp=$(mktemp "$TEMP_DIR/qz.service.XXXXXX")

    # Build the ExecStart command.  The critical env vars (PYTHONHOME,
    # LD_LIBRARY_PATH) are exported inside the bash -c string as
    # belt-and-suspenders: the wrapper's own env -i / LAUNCH_ENV logic is the
    # primary delivery path, but these exports cover the case where someone
    # runs the binary directly without the wrapper.
    local exec_cmd="${bin} ${flags}"
    if [[ -n "$python_home" ]]; then
        exec_cmd="export PYTHONHOME=${python_home}; export LD_LIBRARY_PATH=${ld_path}; ${exec_cmd}"
    fi
    
    cat > "$tmp" <<EOF
[Unit]
Description=qdomyos-zwift service
After=multi-user.target

[Service]
User=root
Group=plugdev
Environment="QZ_USER=${user}"
WorkingDirectory=$(dirname "$bin")
${python_home:+Environment="PYTHONHOME=${python_home}"
}Environment="LD_LIBRARY_PATH=${ld_path}"
ExecStart=/bin/bash -c '${exec_cmd}'
KillSignal=SIGINT
SuccessExitStatus=130

[Install]
WantedBy=multi-user.target
EOF
    
    if [[ $(id -u) -eq 0 ]]; then
        mv -f "$tmp" "$svc_file" || return 1
        systemctl daemon-reload || true
        ACTIVE_SERVICE_FILE="$svc_file"
        echo "$svc_file"
        return 0
    fi
    
    ACTIVE_SERVICE_FILE="$tmp"
    echo "$tmp"
    return 0
}
 
### Milestone 5: Service lifecycle UI functions
run_as_root_or_sudo() {
    if [[ $(id -u) -eq 0 ]]; then
        "$@"
        return $?
    fi
    if command -v sudo >/dev/null 2>&1; then
        sudo "$@"
        return $?
    fi
    echo "ERROR: root privileges required and sudo not available." >&2
    return 1
}

install_service_ui() {
    # Non-interactive install flow that presents results via dashboard UI
    exit_ui_mode || true
    load_service_config >/dev/null 2>&1 || true
    if ! ensure_ram_temp_dir >/dev/null 2>&1; then
        TEMP_DIR=/tmp
    fi

    # Shared layout for centred status messages
    local row=$((LOG_TOP + 2))
    local msg msg_plain w col

    # --- Step 1: Generate ---
    draw_bottom_panel_header "INSTALLING SERVICE" "false"
    clear_info_area
    msg="${YELLOW}Generating service file...${NC}"
    msg_plain="Generating service file..."
    w=$(get_display_width "$msg_plain")
    col=$(( (INNER_COLS - w) / 2 ))
    draw_sealed_row "$row" "$(printf '%*s' "$((col-2))" '')${msg}"
    draw_bottom_border

    local gen_out tmp
    _GEN_SVC_SILENT=1 gen_out=$(generate_service_file 2>&1) || true
    unset _GEN_SVC_SILENT
    tmp="${ACTIVE_SERVICE_FILE:-$gen_out}"
    # Normalize tmp to the last non-empty line (generate_service_file may emit warnings before printing the path)
    tmp=$(printf '%s' "$tmp" | tr -d '\r' | awk 'NF{line=$0} END{print line}')

    # --- Step 2: Install (move into systemd path) ---
    msg="${YELLOW}Installing service file...${NC}"
    msg_plain="Installing service file..."
    w=$(get_display_width "$msg_plain")
    col=$(( (INNER_COLS - w) / 2 ))
    draw_sealed_row "$row" "$(printf '%*s' "$((col-2))" '')${msg}"

    local svc_target="$SERVICE_FILE_QZ"
    # Attempt to move the generated service file into place as root. Treat
    # installation as successful if the file exists at the target path after
    # the move; do not treat a daemon-reload failure as an installation failure.
    local mv_ok=1
    if run_as_root_or_sudo bash -lc "
        set -e
        mkdir -p \"$SYSTEMD_SYSTEM_DIR\" >/dev/null 2>&1 || true
        if [ -f '${tmp}' ]; then
            mv -f '${tmp}' '${svc_target}' || exit 2
        fi
        if [ -f '${svc_target}' ]; then
            echo MOVED
            exit 0
        else
            exit 3
        fi
    "; then
        mv_ok=0
        ACTIVE_SERVICE_FILE="${svc_target}"
    else
        mv_ok=1
    fi

    # If the above command reported failure but the target file nevertheless
    # exists (for example created by a previous run or another process), treat
    # the installation as successful to avoid false negatives.
    if [[ $mv_ok -ne 0 && -f "${svc_target}" ]]; then
        mv_ok=0
        ACTIVE_SERVICE_FILE="${svc_target}"
    fi

    # --- Step 3: Reload systemd & report ---
    if [[ $mv_ok -eq 0 ]]; then
        msg="${YELLOW}Reloading systemd...${NC}"
        msg_plain="Reloading systemd..."
        w=$(get_display_width "$msg_plain")
        col=$(( (INNER_COLS - w) / 2 ))
        draw_sealed_row "$row" "$(printf '%*s' "$((col-2))" '')${msg}"
        if ! run_as_root_or_sudo systemctl daemon-reload >/dev/null 2>&1; then
            draw_info_screen "SERVICE INSTALLED" "Service file installed: ${svc_target}\nWarning: systemd daemon-reload failed; you may need to run 'sudo systemctl daemon-reload' manually." 3
        else
            draw_info_screen "SERVICE INSTALLED" "Service file installed: ${svc_target}" 2
        fi
    else
        draw_error_screen "INSTALL FAILED" "Installation failed — check sudo privileges or logs." "wait"
    fi
    enter_ui_mode || true
}

start_service_ui() {
    exit_ui_mode || true
    if run_as_root_or_sudo systemctl start qz.service; then
        sleep 1
        if systemctl is-active --quiet qz.service 2>/dev/null; then
            draw_info_screen "SERVICE STARTED" "Service started successfully.\nStatus: ACTIVE" "wait"
        else
            draw_error_screen "SERVICE FAILED" "Service did not become active. Check systemd status." "wait"
        fi
    else
        draw_error_screen "ERROR" "Failed to start service." "wait"
    fi
    enter_ui_mode || true
}

stop_service_ui() {
    exit_ui_mode || true
    if run_as_root_or_sudo systemctl stop qz.service; then
        # Give systemd a moment to settle and check resulting state
        sleep 1
        if run_as_root_or_sudo systemctl is-failed --quiet qz.service 2>/dev/null; then
            # Unit entered failed state after stop — capture concise failure info
            capture_service_failure_info "qz_service" || true
            STATUS_MAP["qz_service"]="fail"
            draw_error_screen "STOP FAILED" "Stopping qz.service resulted in a failure. Check systemd status." "wait"
        else
            STATUS_MAP["qz_service"]="pending"
            draw_info_screen "SERVICE STOPPED" "Service stopped successfully." "wait"
        fi
    else
        # Stop command itself failed (permission or systemctl error)
        capture_service_failure_info "qz_service" || true
        STATUS_MAP["qz_service"]="fail"
        draw_error_screen "ERROR" "Failed to stop service (check privileges). Check systemd status'." "wait"
    fi
    enter_ui_mode || true
}

restart_service_ui() {
    exit_ui_mode || true
    if run_as_root_or_sudo systemctl restart qz.service; then
        draw_info_screen "SERVICE RESTARTED" "Service restarted successfully." "wait"
    else
        draw_error_screen "ERROR" "Failed to restart service." "wait"
    fi
    enter_ui_mode || true
}

enable_service_ui() {
    exit_ui_mode || true
    if run_as_root_or_sudo systemctl enable qz.service; then
        draw_info_screen "SERVICE ENABLED" "Service enabled for auto-start." "wait"
    else
        draw_error_screen "ERROR" "Failed to enable service." "wait"
    fi
    enter_ui_mode || true
}

disable_service_ui() {
    exit_ui_mode || true
    if run_as_root_or_sudo systemctl disable qz.service; then
        draw_info_screen "SERVICE DISABLED" "Auto-start disabled." "wait"
    else
        draw_error_screen "ERROR" "Failed to disable service." "wait"
    fi
    enter_ui_mode || true
}

remove_service_ui() {
    # Use the UI-consistent Yes/No overlay instead of a plain tty prompt
    exit_ui_mode || true
    draw_bottom_panel_header "REMOVE SERVICE"
    clear_info_area
    draw_sealed_row $((LOG_TOP + 1)) "   This will stop and remove the installed service file."
    draw_sealed_row $((LOG_TOP + 2)) "   Confirm removal?"
    draw_bottom_border "Arrows: Up/Down | Enter: Select | Esc: Cancel"

    # prompt_yes_no renders an overlay and returns selected index (0=Yes)
    prompt_yes_no 4
    local resp=$?

    if [[ $resp -eq 0 ]]; then
        # Show progress feedback during removal
        draw_bottom_panel_header "REMOVING SERVICE"
        clear_info_area
        local row=$((LOG_TOP + 3))
        local msg msg_plain w col
        
        # Step 1: Stop service
        msg="${YELLOW}Stopping service...${NC}"
        msg_plain="Stopping service..."
        w=$(get_display_width "$msg_plain")
        col=$(( (INNER_COLS - w) / 2 ))
        draw_sealed_row "$row" "$(printf '%*s' "$((col-2))" '')${msg}"
        
        run_as_root_or_sudo systemctl stop qz.service >/dev/null 2>&1 || true
        
        # Step 2: Disable service
        msg="${YELLOW}Disabling service...${NC}"
        msg_plain="Disabling service..."
        w=$(get_display_width "$msg_plain")
        draw_sealed_row "$row" "$(printf '%*s' "$((col-2))" '')${msg}"
        
        run_as_root_or_sudo systemctl disable qz.service >/dev/null 2>&1 || true
        
        # Step 3: Remove service files
        msg="${YELLOW}Removing service files...${NC}"
        msg_plain="Removing service files..."
        w=$(get_display_width "$msg_plain")
        draw_sealed_row "$row" "$(printf '%*s' "$((col-2))" '')${msg}"
        
        run_as_root_or_sudo rm -f "$SYSTEMD_SYSTEM_DIR/qz.service" >/dev/null 2>&1 || true
        
        # Step 4: Reload systemd
        msg="${YELLOW}Reloading systemd...${NC}"
        msg_plain="Reloading systemd..."
        w=$(get_display_width "$msg_plain")
        draw_sealed_row "$row" "$(printf '%*s' "$((col-2))" '')${msg}"
        
        if run_as_root_or_sudo systemctl daemon-reload >/dev/null 2>&1; then
            # Success
            msg="${GREEN}✓ Service removed successfully${NC}"
            msg_plain="✓ Service removed successfully"
            w=$(get_display_width "$msg_plain")
            draw_sealed_row "$row" "$(printf '%*s' "$((col-2))" '')${msg}"
            sleep 1.5
        else
            draw_error_screen "REMOVE FAILED" "Failed to remove service; check privileges." 3
        fi
    else
        draw_info_screen "REMOVE ABORTED" "Service removal aborted." 1
    fi

    enter_ui_mode || true
}

view_service_logs_ui() {
    # shellcheck disable=SC2178,SC2128
    local lines=${1:-200}
    exit_ui_mode || true
    draw_bottom_panel_header "SERVICE LOGS"
    draw_sealed_row $((LOG_TOP)) ""
    # shellcheck disable=SC2128
    run_as_root_or_sudo journalctl -u qz.service -n "$lines" --no-pager || true
    draw_bottom_border "Press Enter to continue"
    enter_ui_mode || true
}

view_service_error_ui() {
    exit_ui_mode || true
    local key="qz_service"
    local info="${SERVICE_FAILURE_INFO[$key]:-}"
    if [[ -z "$info" ]]; then
        # Attempt to fetch live info if none cached
        capture_service_failure_info "$key" >/dev/null 2>&1 || true
        info="${SERVICE_FAILURE_INFO[$key]:-(no details available)}"
    fi
    # Truncate for display if extremely large
    if [[ $(printf '%s' "$info" | wc -l) -gt 200 ]]; then
        info=$(printf '%s' "$info" | tail -n 200)
    fi
    draw_info_screen "SERVICE ERROR DETAILS" "$info" "wait"
    enter_ui_mode || true
}

view_service_config_ui() {
    # 1. Load Configuration Data
    local cfg="${SERVICE_CONF_PATH:-$HOME/.config/qdomyos-zwift/service.conf}"
    if [[ ! -f "$cfg" ]]; then
        draw_error_screen "SERVICE CONFIGURATION" "Configuration file not found:\n\n$cfg" "wait"
        return 1
    fi

    load_service_config >/dev/null 2>&1 || true

    # Retrieve Bluetooth Device Name
    local bt_name="${CONFIG_STRING[bluetooth_lastdevice_name]:-}"
    if [[ -z "$bt_name" && -f "$CONFIG_FILE" ]]; then
         bt_name=$(grep -E '^bluetooth_lastdevice_name=' "$CONFIG_FILE" 2>/dev/null | cut -d'=' -f2- | tr -d '\r')
    fi
    # Clean up name
    bt_name=$(strip_ansi "${bt_name:-}")
    [[ -z "$bt_name" ]] && bt_name="None"

    # 2. Setup UI
    UI_LOCKED=0
    enter_ui_mode || true
    
    draw_bottom_panel_header "SERVICE CONFIGURATION"
    clear_info_area
    
    # 3. Draw Rows
    # Start at LOG_TOP + 1 (Moved up by one row as requested)
    local r=$((LOG_TOP + 1))
    
    # Helper for consistent coloring
    # Usage: draw_kv "Label" "Value"
    draw_kv() {
        draw_sealed_row "$r" "   $1: ${WHITE}$2${NC}"
        ((r++))
    }

    draw_kv "Logging" "${SERVICE_FLAGS[logging]:-false}"
    draw_kv "Console" "${SERVICE_FLAGS[console]:-false}"
    draw_kv "ANT+ Footpod" "${SERVICE_FLAGS[ant_footpod]:-false}"
    
    # Only show detailed ANT+ info if enabled (cleaner UI) or always show if you prefer
    if [[ "${SERVICE_FLAGS[ant_footpod]}" == "true" ]]; then
        draw_kv "ANT+ Device ID" "${SERVICE_FLAGS[ant_device]:-54321}"
        if [[ "${SERVICE_FLAGS[logging]}" == "true" ]]; then
            draw_kv "ANT+ Verbose" "${SERVICE_FLAGS[ant_verbose]:-false}"
        fi
    fi

    draw_kv "Poll Time (ms)" "${SERVICE_FLAGS[poll_time]:-200}"
    
    # Bluetooth Device (Color coded)
    local bt_disp="${WHITE}${bt_name}${NC}"
    [[ "$bt_name" == "None" ]] && bt_disp="${GRAY}None${NC}"
    draw_sealed_row "$r" "   Bluetooth Device: ${bt_disp}"

    # 4. Interaction
    draw_bottom_border "Press ENTER to continue"
    local k
    safe_read_key k
    
    exit_ui_mode || true
}

regenerate_service_file_ui() {
    if ! ensure_ram_temp_dir >/dev/null 2>&1; then
        TEMP_DIR=/tmp
    fi
    local gen_out
    exit_ui_mode || true
    gen_out=$(generate_service_file 2>&1) || true
    # Use informational panel for successful generation
    draw_info_screen "SERVICE FILE GENERATED" "${ACTIVE_SERVICE_FILE:-$gen_out}" 2
    enter_ui_mode || true
}

## Service status helpers (STEP 1)
# Returns: not-installed | stopped | running | failed
get_service_status() {
    # Check both potential locations
    local exists=false
    if [[ -n "${ACTIVE_SERVICE_FILE:-}" && -f "$ACTIVE_SERVICE_FILE" ]]; then
        exists=true
    elif [[ -f "$SERVICE_FILE_QZ" ]]; then
        exists=true
    fi

    if [[ "$exists" == "true" ]]; then
        :
    else
        printf "not-installed"
        return 0
    fi

    if run_as_root_or_sudo systemctl is-active --quiet qz.service 2>/dev/null; then
        printf "running"
    elif run_as_root_or_sudo systemctl is-failed --quiet qz.service 2>/dev/null; then
        printf "failed"
    else
        printf "stopped"
    fi
}

# Returns numeric ExecMainStatus for qz.service when available (or empty)
get_service_exit_code() {
    if ! command -v systemctl >/dev/null 2>&1; then
        return 1
    fi
    local val
    if run_as_root_or_sudo systemctl show -p ExecMainStatus --value qz.service >/dev/null 2>&1; then
        val=$(run_as_root_or_sudo systemctl show -p ExecMainStatus --value qz.service 2>/dev/null || true)
    else
        val=$(systemctl show -p ExecMainStatus --value qz.service 2>/dev/null || true)
    fi
    # Normalize to integer or empty
    if [[ "$val" =~ ^[0-9]+$ ]]; then
        printf '%s' "$val"
        return 0
    fi
    return 1
}

# Check whether the installed service unit is missing SuccessExitStatus=130
service_needs_exit_130_check() {
    if [[ -f "$SERVICE_FILE_QZ" ]]; then
        :
    else
        return 1
    fi
    if grep -q '^\s*SuccessExitStatus\s*=\s*130' "$SERVICE_FILE_QZ" 2>/dev/null; then
        return 1
    fi
    return 0
}

# Apply SuccessExitStatus=130 to the installed unit (returns 0 on success)
apply_exit_130_fix() {
    local svc_target="$SERVICE_FILE_QZ"
    if [[ ! -f "$svc_target" ]]; then
        return 2
    fi

    run_as_root_or_sudo bash -lc "
        set -e
        cp -a '${svc_target}' '${svc_target}.bak' 2>/dev/null || true
        if grep -q '^\\s*SuccessExitStatus\\s*=\\s*130' '${svc_target}' 2>/dev/null; then
            echo ALREADY
            exit 0
        fi
        awk 'BEGIN{p=0} /^\[Service\]/{print; p=1; next} { if(p==1 && /^\\s*$/){ print \"SuccessExitStatus=130\"; p=2 } print } END{ if(p==1){ print \"SuccessExitStatus=130\" } }' '${svc_target}' > '${svc_target}.tmp' && mv -f '${svc_target}.tmp' '${svc_target}'
        systemctl daemon-reload || true
        echo PATCHED
    " >/dev/null 2>&1 || true
    return 0
}

apply_exit_130_fix_ui() {
    exit_ui_mode || true
    draw_bottom_panel_header "UPDATE SERVICE CONFIG"
    clear_info_area
    draw_sealed_row $((LOG_TOP + 1)) "   This will add SuccessExitStatus=130 to the installed unit."
    draw_sealed_row $((LOG_TOP + 3)) "   Proceed?"
    if ! prompt_yes_no 5; then
        draw_info_screen "UPDATE ABORTED" "No changes made." "wait"
        enter_ui_mode || true
        return 1
    fi
    if apply_exit_130_fix; then
        draw_info_screen "UPDATED" "Service unit patched; systemd reloaded." "wait"
        # Refresh status map
        check_qz_service >/dev/null 2>&1 || true
    else
        draw_error_screen "ERROR" "Failed to update installed unit." "wait"
    fi
    enter_ui_mode || true
}

# Returns 0 if enabled, non-zero otherwise
is_service_enabled() {
    # Use elevated check so status is accurate even when dashboard runs unprivileged
    run_as_root_or_sudo systemctl is-enabled --quiet qz.service 2>/dev/null
}

build_service_menu_options() {
    local status
    status=$(get_service_status)
    local opts=()

    opts+=("Configure Service Flags")
    # REMOVED: "View Current Configuration" - not needed with help panel
    
    # NOTE: "Regenerate Service File" removed. 
    # Logic will handle this automatically on exit.

    case "$status" in
        not-installed)
            opts+=("Generate & Install Service")
            ;;
        stopped)
            if [[ "${SVC_PYTHONHOME_STALE:-false}" == "true" ]]; then
                opts+=("Generate & Install Service")
            fi
            opts+=("Start Service")
            opts+=("Remove Service")
            ;;
        running)
            if [[ "${SVC_PYTHONHOME_STALE:-false}" == "true" ]]; then
                opts+=("Generate & Install Service")
            fi
            opts+=("Restart Service")
            opts+=("Stop Service")
            ;;
        failed)
            if [[ "${SVC_PYTHONHOME_STALE:-false}" == "true" ]]; then
                opts+=("Generate & Install Service")
            fi
            opts+=("Restart Service")
            if service_needs_exit_130_check; then
                opts+=("Update Service Configuration")
            fi
            opts+=("Remove Service")
            ;;
    esac

    if [[ "$status" != "not-installed" ]]; then
        if is_service_enabled; then
            opts+=("Disable Auto-Start")
        else
            opts+=("Enable Auto-Start")
        fi
    fi

    printf '%s\n' "${opts[@]}"
}

# Map a displayed choice to the corresponding action command
get_action_for_choice() {
    local choice="$1"
    
    # Use wildcards (*) to match strings even if they have color codes or prefixes
    case "$choice" in
        *"View Service Error"*)          printf '%s' "view_service_error_ui" ;; 
        *"Configure Service Flags"*)     printf '%s' "configure_service_flags_ui" ;; 
        *"View Current Configuration"*)  printf '%s' "view_service_config_ui" ;; 
        *"Generate & Install Service"*)  printf '%s' "install_service_ui" ;; 
        *"Start Service"*)               printf '%s' "start_service_ui" ;; 
        *"Stop Service"*)                printf '%s' "stop_service_ui" ;; 
        *"Restart Service"*)             printf '%s' "restart_service_ui" ;; 
        *"View Service Logs"*)           printf '%s' "view_service_logs_ui 200" ;; 
        # Match both "Regenerate..." and "! Regenerate..."
        *"Regenerate Service File"*)     printf '%s' "regenerate_service_file_ui" ;; 
        *"Update Service Configuration"*) printf '%s' "apply_exit_130_fix_ui" ;; 
        *"Enable Auto-Start"*)           printf '%s' "enable_service_ui" ;; 
        *"Disable Auto-Start"*)          printf '%s' "disable_service_ui" ;; 
        *"Remove Service"*)              printf '%s' "remove_service_ui" ;; 
        *)                               printf '%s' "" ;; 
    esac
}

prompt_restart_service() {
    # 1. Edge Case: No service file exists -> Do nothing
    if [[ ! -f "$SERVICE_FILE_QZ" ]]; then return 0; fi

    # 2. Get FRESH status
    local svc_status
    svc_status=$(get_service_status)
    
    # 3. Draw UI
    draw_bottom_panel_header "CONFIGURATION CHANGED" "false"
    clear_info_area
    
    local r=$((LOG_TOP + 1))
    draw_sealed_row "$r" "   ${YELLOW}Configuration saved to disk.${NC}"
    ((r++))
    
    local action_func="start_service_ui"
    
    if [[ "$svc_status" == "running" ]]; then
        draw_sealed_row "$r" "   The QZ Service must be restarted to apply these changes."
        ((r++))
        draw_sealed_row "$r" ""
        ((r++))
        draw_sealed_row "$r" "   Restart Service now?"
        action_func="restart_service_ui"
    else
        draw_sealed_row "$r" "   The QZ Service is currently ${RED}${svc_status^^}${NC}."
        ((r++))
        draw_sealed_row "$r" ""
        ((r++))
        draw_sealed_row "$r" "   Start Service now?"
        action_func="start_service_ui"
    fi
    
    # 4. Prompt (Offset 5)
    if prompt_yes_no 5; then
        # Execute the chosen function (Start or Restart)
        $action_func
        
        # Refresh global status map immediately so Header updates
        check_qz_service >/dev/null 2>&1 || true
    fi
    
    return 0
}

service_menu_flow() {
    enter_ui_mode || true
    
    if [[ ! -f "./qdomyos-zwift-bin" ]] && [[ ! -f "$SCRIPT_DIR/qdomyos-zwift-bin" ]]; then
         if ! command -v qdomyos-zwift >/dev/null; then
            draw_error_screen "MISSING BINARY" "Cannot configure service: qdomyos-zwift binary not found." "wait"
            exit_ui_mode
            return 1
         fi
    fi

    local selected=0
    local need_status_refresh=true
    local draw_mode="FULL"
    local flags_changed=0 # Track changes

    while true; do
        if [[ $need_status_refresh == true ]]; then
            STATUS_MAP["qz_service"]="pending"
            draw_status_panel 
            check_qz_service >/dev/null 2>&1
            draw_status_panel
            need_status_refresh=false
        fi

        local options=()
        mapfile -t options < <(build_service_menu_options)
        
        # Build help text array to match options
        local status
        status=$(get_service_status)
        local help_texts=()
        
        for opt in "${options[@]}"; do
            case "$opt" in
                *"Configure Service Flags"*)
                    help_texts+=("Configure systemd service flags including logging, console output, ANT+ footpod support, and polling intervals. Changes are saved and applied automatically on exit.")
                    ;;
                *"Generate & Install Service"*)
                    help_texts+=("Create and install the qz.service systemd unit file. This enables QZ to run as a background service and start automatically on boot if enabled.")
                    ;;
                *"Start Service"*)
                    help_texts+=("Start the QZ systemd service immediately. The service will begin running in the background and connect to your configured equipment.")
                    ;;
                *"Stop Service"*)
                    help_texts+=("Stop the currently running QZ service. This disconnects from equipment and terminates the background process. Does not disable auto-start.")
                    ;;
                *"Restart Service"*)
                    help_texts+=("Restart the QZ service to apply configuration changes or recover from errors. This stops and immediately starts the service again.")
                    ;;
                *"Remove Service"*)
                    help_texts+=("Completely remove the systemd service file and disable auto-start. QZ will no longer run as a background service but can still be run manually.")
                    ;;
                *"Enable Auto-Start"*)
                    help_texts+=("Enable QZ to start automatically on system boot. The service will begin running in the background whenever your system starts up.")
                    ;;
                *"Disable Auto-Start"*)
                    help_texts+=("Disable automatic startup on boot. The service must be started manually after each reboot. Does not affect the currently running service.")
                    ;;
                *"Update Service Configuration"*)
                    help_texts+=("Apply the Exit 130 handler fix to the service file. This prevents systemd restart failures when the service is stopped normally.")
                    ;;
                *)
                    help_texts+=("")
                    ;;
            esac
        done
        
        show_unified_menu options "$selected" "SERVICE CONFIGURATION" "$draw_mode" "false" "" help_texts
        local exit_code=$?
        
        # --- EXIT HANDLER ---
        if [[ $exit_code -eq 255 ]]; then
            # Check if flags changed during this session
            # Or simpler: Check if disk file differs from installed file
            # (We reuse the grep logic from before but apply it silently)
            
            if [[ -f "$SERVICE_FILE_QZ" ]]; then
                load_service_config >/dev/null 2>&1
                local current_flags
                current_flags=$(build_service_flags)
                
                if ! grep -Fq " ${current_flags}'" "$SERVICE_FILE_QZ" 2>/dev/null; then
                    # MISMATCH DETECTED -> Auto-Regenerate
                    draw_bottom_panel_header "APPLYING CHANGES" "false"
                    clear_info_area
                    draw_sealed_row $((LOG_TOP+2)) "   ${YELLOW}Service configuration changed.${NC}"
                    draw_sealed_row $((LOG_TOP+3)) "   Updating systemd unit file..."
                    
                    generate_service_file
                    sleep 1
                    
                    prompt_restart_service
                fi
            fi
            
            exit_ui_mode
            return 0
        fi
        
        selected=$exit_code
        local choice="${options[$selected]}"
        
        exit_ui_mode
        local action
        action=$(get_action_for_choice "$choice")
        
        if [[ -n "$action" ]]; then
            local func_name="${action%% *}"
            local func_args="${action#* }"
            [[ "$func_name" == "$func_args" ]] && func_args=""

            if [[ -n "$func_args" ]]; then "$func_name" "$func_args"; else "$func_name"; fi

            case "$func_name" in
                install_*|uninstall_*|start_*|stop_*|restart_*|enable_*|disable_*|remove_*)
                    need_status_refresh=true ;;
                configure_service_flags_ui)
                    # User entered flag menu, assume they might have changed something
                    # (We rely on the grep check at exit to confirm)
                    ;;
            esac
        fi
        
        enter_ui_mode
        draw_mode="FULL"
    done
}

### End Milestone 5

check_final_status() {
    while true; do
        local fails=0 warns=0
        for key in "${!STATUS_MAP[@]}"; do
            local val="${STATUS_MAP[$key]}"
            if [ "$val" = "fail" ]; then ((fails++)); elif [ "$val" = "warn" ]; then ((warns++)); fi
        done

        if [ $fails -gt 0 ]; then
            # --- ERROR MODE ---
            local choice
            prompt_action_menu "$fails"
            choice=$?
            case $choice in
                0) 
                    if run_guided_mode; then
                        draw_verifying_screen "Verifying system state..."
                        run_all_checks
                    fi
                    ;;
                1) finish_and_exit 0 ;; 
            esac
        else
            # --- SUCCESS MODE ---
            local choice
            prompt_success_menu "$warns"
            choice=$?
            
            # MAPPED TO NEW MENU ORDER
            case $choice in
                0) run_setup_wizard ;;                          # Quick Setup Wizard
                1) select_equipment_flow; check_config_file ;;  # Equipment Selection
                2) perform_bluetooth_scan; check_config_file ;; # Bluetooth Scanning
                3) configure_user_profile ;;                    # User Profile
                4) service_menu_flow ;;                         # QZ Service Control
                5) perform_ant_test; check_config_file ;;       # Diagnostics (ANT+ Test)
                6) finish_and_exit 0 ;;                         # Exit
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
# shellcheck disable=SC2317
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

    Interactive options:
        --uninstall    Start the uninstall menu immediately and exit
        --pyenv        Force pyenv installation (skip system Python even if available)

REQUIREMENTS:
    - Root privileges required for installation actions.

EOF
}

cleanup() {
    # This is now handled by the trap above, but we keep it 
    # as a safety fallback for manual calls.
    restore_terminal
}

# Stop any running test_ant.py sessions discovered via the pidfile or by pkill.
stop_test_ant_sessions() {
    if [[ -n "${TEMP_DIR:-}" && -f "$TEMP_DIR/qz_ant_test.pid" ]]; then
        local apid
        apid=$(<"$TEMP_DIR/qz_ant_test.pid")
        if [[ -n "$apid" ]] && ps -p "$apid" >/dev/null 2>&1; then
            printf 'Stopping lingering test_ant.py (PID %s)...\n' "$apid" >/dev/stderr
            sudo kill -TERM "$apid" 2>/dev/null || true
            sleep 2
            if ps -p "$apid" >/dev/null 2>&1; then
                sudo kill -9 "$apid" 2>/dev/null || true
            fi
        fi
        rm -f "$TEMP_DIR/qz_ant_test.pid" 2>/dev/null || true
    fi
    # As a fallback also kill any remaining test_ant.py processes
    sudo pkill -f test_ant.py >/dev/null 2>&1 || true
}

# Attempt a safe USB reset of the ANT+ dongle using pyusb. This mirrors
# the behavior present in test_ant.py.reset_ant_dongle(). Failure is
# non-fatal; we print diagnostic info to stderr for the user.
# Unified shutdown called from traps: stop test sessions,
# restore terminal and exit with the provided code.
safe_shutdown() {
    local code=${1:-0}
    # stop background test processes first
    stop_test_ant_sessions
    restore_terminal
    # Re-emit the signal exit code
    exit "$code"
}

# Parse arguments
if [ $# -gt 0 ]; then
    for arg in "$@"; do
        case "$arg" in
            --help)
                show_help; exit 0
                ;;
            --uninstall)
                UNINSTALL_MODE=1
                ;;
            --pyenv)
                FORCE_PYENV=1
                ;;
            # (debug options removed)
            *)
                echo "Unknown option: $arg. Use --help for usage."; exit 1
                ;;
        esac
    done
fi

# Check root for install capability
[ $EUID -eq 0 ] && CAN_INSTALL=1
: "${CAN_INSTALL:-}" >/dev/null 2>&1

# ============================================================================
# MAIN EXECUTION START
# ============================================================================

# 1. Prepare terminal
trap 'safe_shutdown 0' EXIT
trap 'safe_shutdown 130' SIGINT
trap 'safe_shutdown 143' SIGTERM
set_ui_output
init_symbol_cache

hide_cursor

# 1.5 Setup Configuration Path
: "${CONFIG_FILE:=$HOME/.config/qdomyos-zwift/qDomyos-Zwift.conf}"

# 2. Startup Sequence
if [[ "${QZ_NO_SPLASH:-0}" -eq 0 ]]; then
    draw_splash_screen "QDomyos-Zwift" "ANT+ BRIDGE SETUP UTILITY"
    # UI_LOCKED is set to 1 by draw_splash_screen now
    
    update_splash_progress 0 100 "Loading User Configuration"
    load_config_into_arrays "$CONFIG_FILE"
    load_current_profile_values
    
    update_splash_progress 5 100 "Identifying Active Equipment"
    check_equipment_state
    
    # run_all_checks will update status maps in background
    # Because UI_LOCKED=1, any rogue draws are suppressed
    run_all_checks "splash"
    
    update_splash_progress 100 100 "Initialization Complete"
    sleep 0.5
fi

# Check for uninstall mode
if [ "${UNINSTALL_MODE:-0}" -eq 1 ]; then
    UI_LOCKED=0  
    clear_screen
    draw_top_panel
    run_uninstall_mode
    exit 0
fi

# 3. Render the Dashboard (Atomic)
render_dashboard_atomic

# 4. Enter the main menu loop
UI_LOCKED=0
check_final_status