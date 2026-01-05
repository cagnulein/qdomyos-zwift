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
#   sudo ./setup-dashboard.sh --scan-now
################################################################################

set -uo pipefail

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
    local save_msg_plain="✓ ${1:+$1 }Saved"
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
        sleep 1
        print_at_col "$row" "$col" "$(printf '%*s' "$w" '')"
    ) &
}

# Display temporary cancel feedback
show_cancel_feedback() {
    # Render cancel feedback inside the info panel (right-aligned)
    local cancel_msg_plain="✗ Cancelled"
    local cancel_msg="${RED}${cancel_msg_plain}${NC}"
    local w
    w=$(get_display_width "$cancel_msg_plain")
    local row=$((LOG_BOTTOM))
    local col=$((2 + INFO_WIDTH - w))
    print_at_col "$row" "$col" "$cancel_msg"
    sleep 1
    print_at_col "$row" "$col" "$(printf '%*s' "$w" '')"
}

# Inline field editor - edits numeric values in brackets on the same line
# Usage: inline_edit_field ROW LABEL CURRENT_VALUE MIN MAX MAXLEN
# Returns: new value via stdout, or empty if cancelled/invalid
inline_edit_field() {
    local row="$1"
    local label="$2"
    local current_value="$3"
    local min_val="$4"
    local max_val="$5"
    local max_len="$6"
    local new_value=""  # Start empty
    local placeholder_visible=true
    
    # Flush any buffered input before starting
    flush_input_buffer
    
    while true; do
        # Build display string
        local display_value
        local display_color
        if [[ $placeholder_visible == true && -z "$new_value" ]]; then
            display_value="$current_value"
            display_color="$GRAY"
        else
            display_value="$new_value"
            display_color="$WHITE"
        fi
        
        # Update the row content using safe update function
        local row_content="   ${CYAN}► ${NC}${label}: [${display_color}${display_value}${NC}]"
        update_sealed_row_content "$row" "$row_content"
        
        # Position cursor
        local cursor_col
        if [[ -z "$new_value" ]]; then
            cursor_col=$((5 + ${#label} + 4))  # Start of brackets
        else
            cursor_col=$((5 + ${#label} + 4 + ${#new_value}))  # After typed text
        fi
        move_cursor "$((row + 1))" "$cursor_col"
        printf '\033[?25h'  # Show cursor
        
        # Read one key
        local key=""
        safe_read_key key
        
        if [[ $key == $'\x1b' ]]; then
            read_escape_sequence k2
            if [[ -z "${k2:-}" ]]; then
                # ESC - cancel
                printf '\033[?25l'
                flush_input_buffer
                return 1
            fi
            # Ignore arrow keys
            continue
        elif [[ $key == "" ]]; then
            # Enter - confirm
            printf '\033[?25l'
            
            # If nothing typed, no change
            if [[ -z "$new_value" ]]; then
                flush_input_buffer
                echo "$current_value"
                return 0
            fi
            
            # Validate range
            local num_val=$((new_value))
            if [[ $num_val -lt $min_val ]] || [[ $num_val -gt $max_val ]]; then
                # Show error briefly
                local err_text="✗ Must be ${min_val}-${max_val}"
                update_sealed_row_content "$row" "     ${RED}${err_text}${NC}"
                sleep 1
                flush_input_buffer
                return 1
            fi
            
            # Valid - return new value
            flush_input_buffer
            echo "$new_value"
            return 0
        elif [[ $key == $'\x7f' ]] || [[ $key == $'\x08' ]]; then
            # Backspace
            if [[ ${#new_value} -gt 0 ]]; then
                new_value="${new_value%?}"
                if [[ -z "$new_value" ]]; then
                    placeholder_visible=true
                fi
            fi
        elif [[ $key =~ ^[0-9]$ ]]; then
            # First digit clears placeholder
            if [[ $placeholder_visible == true ]]; then
                placeholder_visible=false
                new_value=""
            fi
            
            # Add digit if not at max length
            if [[ ${#new_value} -lt $max_len ]]; then
                new_value="${new_value}${key}"
            fi
        fi
        # Ignore other keys and loop
    done
}

configure_service_flags_ui() {
    load_service_config >/dev/null 2>&1
    # Work on an in-memory copy so Cancel can revert
    declare -A _SF
    for k in "${!SERVICE_FLAGS[@]}"; do _SF[$k]="${SERVICE_FLAGS[$k]}"; done

    # Suppress global full-screen refreshes while this modal is active
    UI_MODAL_ACTIVE=1
    enter_ui_mode || true
    local selected=0
    local prev_selected=-1
    local first_draw=true
    
    while true; do
        local options=()
        local opt_keys=()
        options+=("Logging: ${_SF[logging]:-false}")
        opt_keys+=("logging")
        options+=("Console: ${_SF[console]:-false}")
        opt_keys+=("console")
        options+=("ANT+ Footpod: ${_SF[ant_footpod]:-false}")
        opt_keys+=("ant_footpod")
        if [[ "${_SF[ant_footpod]}" == "true" ]]; then
            options+=("ANT+ Device ID: [${_SF[ant_device]:-54321}]")
            opt_keys+=("ant_device")
            # Only show ANT Verbose if both ANT+ Footpod AND Logging are true
            if [[ "${_SF[logging]}" == "true" ]]; then
                options+=("ANT Verbose: ${_SF[ant_verbose]:-false}")
                opt_keys+=("ant_verbose")
            fi
        fi
        options+=("Bluetooth Relaxed: ${_SF[bluetooth_relaxed]:-false}")
        opt_keys+=("bluetooth_relaxed")
        options+=("Poll Time (ms): [${_SF[poll_time]:-200}]")
        opt_keys+=("poll_time")

        local num_options=${#options[@]}

        # Full redraw when options change or first render
        if [[ $first_draw == true ]]; then
            draw_bottom_panel_header "SERVICE FLAG CONFIGURATION"
            clear_info_area
            draw_sealed_row $((LOG_TOP)) ""
            for i in "${!options[@]}"; do
                local row=$((LOG_TOP + 1 + i))
                local opt_text="${options[$i]}"
                if [[ $i -eq $selected ]]; then
                    draw_sealed_row "$row" "   ${CYAN}► ${BOLD_CYAN}${opt_text}${NC}"
                else
                    draw_sealed_row "$row" "     ${GRAY}${opt_text}${NC}"
                fi
            done
            draw_bottom_border "Arrows: Up/Down | Enter: Toggle/Edit | Esc: Back"
            prev_num_options=$num_options
            prev_selected=$selected
            first_draw=false
        elif [[ $num_options -ne $prev_num_options ]]; then
            # Menu structure changed (items added/removed) - smooth line-by-line redraw
            local max_rows=$(( num_options > prev_num_options ? num_options : prev_num_options ))
            for ((i=0; i<max_rows; i++)); do
                local row=$((LOG_TOP + 1 + i))
                if (( i < num_options )); then
                    local opt_text="${options[$i]}"
                    if [[ $i -eq $selected ]]; then
                        update_sealed_row_content "$row" "   ${CYAN}► ${BOLD_CYAN}${opt_text}${NC}"
                    else
                        update_sealed_row_content "$row" "     ${GRAY}${opt_text}${NC}"
                    fi
                else
                    # Clear rows that are no longer needed
                    update_sealed_row_content "$row" ""
                fi
            done
            prev_num_options=$num_options
            prev_selected=$selected
        elif [[ $selected -ne $prev_selected ]]; then
            # Selection changed - just update the two affected rows
            if [[ $prev_selected -ge 0 ]] && [[ $prev_selected -lt $num_options ]]; then
                local prev_row=$((LOG_TOP + 1 + prev_selected))
                update_sealed_row_content "$prev_row" "     ${GRAY}${options[$prev_selected]}${NC}"
            fi
            local new_row=$((LOG_TOP + 1 + selected))
            update_sealed_row_content "$new_row" "   ${CYAN}► ${BOLD_CYAN}${options[$selected]}${NC}"
            prev_selected=$selected
        fi

        # Input handling
        local key=""
        safe_read_key key
        if [[ $key == $'\x1b' ]]; then
            read_escape_sequence k2
            if [[ -z "${k2:-}" ]]; then
                # Plain ESC pressed - copy changes back to global array and save
                for k in "${!_SF[@]}"; do
                    SERVICE_FLAGS[$k]="${_SF[$k]}"
                done
                save_service_config
                
                exit_ui_mode || true
                UI_MODAL_ACTIVE=0
                return 0
            fi
            [[ "${k2:-}" == "[A" ]] && ((selected--))
            [[ "${k2:-}" == "[B" ]] && ((selected++))
            flush_input_buffer
        elif [[ $key == "" ]]; then
            # Enter key -> toggle or edit value
            local sel_key="${opt_keys[$selected]}"
            local current_row=$((LOG_TOP + 1 + selected))
            
            case "$sel_key" in
                logging)
                    if [[ "${_SF[logging]}" == "true" ]]; then
                        _SF[logging]="false"
                        _SF[ant_verbose]="false"
                    else
                        _SF[logging]="true"
                    fi
                    # Menu structure may change - will redraw on next loop
                    ;;
                console)
                    [[ "${_SF[console]}" == "true" ]] && _SF[console]="false" || _SF[console]="true"
                    # Simple toggle - just update this row
                    local new_text="Console: ${_SF[console]}"
                    update_sealed_row_content "$current_row" "   ${CYAN}► ${BOLD_CYAN}${new_text}${NC}"
                    ;;
                bluetooth_relaxed)
                    [[ "${_SF[bluetooth_relaxed]}" == "true" ]] && _SF[bluetooth_relaxed]="false" || _SF[bluetooth_relaxed]="true"
                    # Simple toggle - just update this row
                    local new_text="Bluetooth Relaxed: ${_SF[bluetooth_relaxed]}"
                    update_sealed_row_content "$current_row" "   ${CYAN}► ${BOLD_CYAN}${new_text}${NC}"
                    ;;
                ant_footpod)
                    [[ "${_SF[ant_footpod]}" == "true" ]] && _SF[ant_footpod]="false" || _SF[ant_footpod]="true"
                    # Menu structure may change - will redraw on next loop
                    ;;
                ant_verbose)
                    [[ "${_SF[ant_verbose]}" == "true" ]] && _SF[ant_verbose]="false" || _SF[ant_verbose]="true"
                    # Simple toggle - just update this row
                    local new_text="ANT Verbose: ${_SF[ant_verbose]}"
                    update_sealed_row_content "$current_row" "   ${CYAN}► ${BOLD_CYAN}${new_text}${NC}"
                    ;;
                ant_device)
                    # Inline edit ANT+ Device ID (range: 1-65535, max 5 digits)
                    local current_val="${_SF[ant_device]:-54321}"
                    local new_val
                    if new_val=$(inline_edit_field "$current_row" "ANT+ Device ID" "$current_val" 1 65535 5); then
                        if [[ "$new_val" != "$current_val" ]]; then
                            _SF[ant_device]="$new_val"
                            # Just update this row with new value
                            local new_text="ANT+ Device ID: [${new_val}]"
                            update_sealed_row_content "$current_row" "   ${CYAN}► ${BOLD_CYAN}${new_text}${NC}"
                        else
                            # No change - restore normal display
                            local new_text="ANT+ Device ID: [${current_val}]"
                            update_sealed_row_content "$current_row" "   ${CYAN}► ${BOLD_CYAN}${new_text}${NC}"
                        fi
                    else
                        # Cancelled - restore normal display
                        local new_text="ANT+ Device ID: [${current_val}]"
                        update_sealed_row_content "$current_row" "   ${CYAN}► ${BOLD_CYAN}${new_text}${NC}"
                    fi
                    flush_input_buffer
                    ;;
                poll_time)
                    # Inline edit Poll Time (range: 100-999, max 3 digits)
                    local current_val="${_SF[poll_time]:-200}"
                    local new_val
                    if new_val=$(inline_edit_field "$current_row" "Poll Time (ms)" "$current_val" 100 999 3); then
                        if [[ "$new_val" != "$current_val" ]]; then
                            _SF[poll_time]="$new_val"
                            # Just update this row with new value
                            local new_text="Poll Time (ms): [${new_val}]"
                            update_sealed_row_content "$current_row" "   ${CYAN}► ${BOLD_CYAN}${new_text}${NC}"
                        else
                            # No change - restore normal display
                            local new_text="Poll Time (ms): [${current_val}]"
                            update_sealed_row_content "$current_row" "   ${CYAN}► ${BOLD_CYAN}${new_text}${NC}"
                        fi
                    else
                        # Cancelled - restore normal display
                        local new_text="Poll Time (ms): [${current_val}]"
                        update_sealed_row_content "$current_row" "   ${CYAN}► ${BOLD_CYAN}${new_text}${NC}"
                    fi
                    flush_input_buffer
                    ;;
            esac
        fi

        # Wrap selection
        [[ $selected -lt 0 ]] && selected=$((num_options - 1))
        [[ $selected -ge $num_options ]] && selected=0
    done
}

# Parse an existing INI file into the typed arrays
parse_reference_config() {
    local file="$1"
    if [[ -z "$file" || ! -f "$file" ]]; then
        echo "ERROR: Reference config not found: $file" >&2
        return 1
    fi

    local line section key value
    while IFS= read -r line || [[ -n "$line" ]]; do
        # Skip comments and empty lines
        [[ -z "$line" ]] && continue
        [[ "$line" =~ ^[[:space:]]*# ]] && continue

        # Section header (we only care about [General] for now)
        if [[ "$line" =~ ^[[:space:]]*\[([^]]+)\][[:space:]]*$ ]]; then
            section="${BASH_REMATCH[1]}"
            continue
        fi

        # Only parse lines in [General] or no-section files
        if [[ -n "$section" && "$section" != "General" ]]; then
            continue
        fi

        # Key=Value lines
        if [[ "$line" =~ ^[[:space:]]*([^=]+)=(.*)$ ]]; then
            key="${BASH_REMATCH[1]}"
            value="${BASH_REMATCH[2]}"
            classify_and_store "$key" "$value"
        fi
    done < "$file"

    return 0
}


# Load defaults from reference template or hardcoded minimal defaults
initialize_default_config() {
    local reference_file="${1:-qDomyos-Zwift.conf}"
    if [[ -f "$reference_file" ]]; then
        parse_reference_config "$reference_file" || return 1
        return 0
    fi

    # Fallback to minimal hardcoded defaults
    load_hardcoded_defaults
    return 0
}


# Minimal hardcoded defaults (used if no reference template found)
load_hardcoded_defaults() {
    config_set_int "age" 54 || true
    config_set_int "weight" 78 || true
    config_set_int "height" 175 || true
    config_set_int "ftp" 200 || true
    config_set_bool "dircon_yes" true || true
    config_set_int "dircon_server_base_port" 36866 || true
    config_set_string "filter_device" "I_TL" || true
    config_set_bool "virtual_device_enabled" true || true
    config_set_bool "virtual_device_force_treadmill" true || true
    config_set_bool "bluetooth_30m_hangs" false || true
    config_set_bool "bluetooth_no_reconnection" false || true
    config_set_bool "bluetooth_relaxed" false || true
    config_set_bool "fakedevice_elliptical" false || true
    config_set_bool "fakedevice_rower" false || true
    config_set_bool "fakedevice_treadmill" false || true
    config_set_bool "treadmill_difficulty_gain_or_offset" false || true
    config_set_bool "treadmill_follow_wattage" false || true
    config_set_bool "treadmill_force_speed" true || true
    config_set_int  "treadmill_incline_max" 100 || true
    config_set_int  "treadmill_incline_min" -100 || true
    config_set_int  "treadmill_pid_heart_max" 0 || true
    config_set_int  "treadmill_pid_heart_min" 0 || true
    config_set_string "treadmill_pid_heart_zone" "Disabled" || true
    config_set_bool "treadmill_simulate_inclination_with_speed" false || true
    config_set_int  "treadmill_speed_max" 100 || true
    config_set_float "treadmill_step_incline" 0.5 || true
    config_set_float "treadmill_step_speed" 0.5 || true
    config_set_bool "virtual_device_bluetooth" true || true
    config_set_bool "virtual_device_echelon" false || true
    config_set_bool "virtual_device_force_bike" false || true
    config_set_bool "virtual_device_ifit" false || true
    config_set_bool "virtual_device_onlyheart" false || true
    config_set_bool "virtual_device_rower" false || true
    config_set_bool "virtualbike_forceresistance" true || true

    return 0
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

# Ensure TEMP_DIR is a RAM-backed tmpfs; helper to detect tmpfs paths.
_validate_ram_storage() {
    local path="$1"
    [[ ! -d "$path" || ! -w "$path" ]] && return 1
    # Prefer stat filesystem type check; fall back to a write-test if unknown
    local fs_type
    fs_type=$(stat -f -c %T "$path" 2>/dev/null || echo "unknown")
    if [[ "$fs_type" == "tmpfs" ]]; then
        return 0
    fi
    # As an additional check, verify /dev/shm device id matches
    if [[ -e "/dev/shm" ]]; then
        local dev_shm_dev
        dev_shm_dev=$(stat -c %d "/dev/shm" 2>/dev/null || echo "")
        local cand_dev
        cand_dev=$(stat -c %d "$path" 2>/dev/null || echo "")
        [[ -n "$dev_shm_dev" && "$dev_shm_dev" == "$cand_dev" ]] && return 0
    fi
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
            TEMP_DIR="$base/qz_$$"
            if mkdir -p "$TEMP_DIR" 2>/dev/null; then
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

# ==========================================================================
# CONFIG GENERATION - INI File Generator (Milestone 2)
# Generate qDomyos-Zwift.conf from config arrays using atomic write.
# TEMP_DIR should be set (Milestone 5 will enforce /dev/shm), fallback to /tmp.
# ==========================================================================

generate_config_file() {
    local config_path="${1:-${CONFIG_FILE:-$HOME/.config/qdomyos-zwift/qDomyos-Zwift.conf}}"
    local temp_file

    # Ensure TEMP_DIR exists; fallback to /tmp
    : "${TEMP_DIR:=/tmp}"
    mkdir -p "$TEMP_DIR" || { echo "ERROR: Cannot create TEMP_DIR: $TEMP_DIR" >&2; return 1; }

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

        # Populate model keys from devices.ini with default=false so
        # generated config contains boolean-per-model entries matching
        # the available devices. Do not override existing values.
        if [[ -f "${DEVICES_INI:-$SCRIPT_DIR/devices.ini}" ]]; then
            # Extract RHS identifiers (trim whitespace) and add to ALL_KEYS
            awk -F'=' '/=/ { gsub(/^[ \t]+|[ \t]+$/, "", $2); print $2 }' "${DEVICES_INI:-$SCRIPT_DIR/devices.ini}" \
                | while IFS= read -r _mk; do
                    if [[ -n "$_mk" && -z "${ALL_KEYS[$_mk]+x}" ]]; then
                        ALL_KEYS[$_mk]=false
                    fi
                done
        fi

        # Ensure template defaults are present (do not override existing values)
        if [[ -z "${ALL_KEYS[bluetooth_30m_hangs]+x}" ]]; then ALL_KEYS[bluetooth_30m_hangs]=false; fi
        if [[ -z "${ALL_KEYS[bluetooth_no_reconnection]+x}" ]]; then ALL_KEYS[bluetooth_no_reconnection]=false; fi
        if [[ -z "${ALL_KEYS[bluetooth_relaxed]+x}" ]]; then ALL_KEYS[bluetooth_relaxed]=false; fi

        if [[ -z "${ALL_KEYS[fakedevice_elliptical]+x}" ]]; then ALL_KEYS[fakedevice_elliptical]=false; fi
        if [[ -z "${ALL_KEYS[fakedevice_rower]+x}" ]]; then ALL_KEYS[fakedevice_rower]=false; fi
        if [[ -z "${ALL_KEYS[fakedevice_treadmill]+x}" ]]; then ALL_KEYS[fakedevice_treadmill]=false; fi

        if [[ -z "${ALL_KEYS[treadmill_difficulty_gain_or_offset]+x}" ]]; then ALL_KEYS[treadmill_difficulty_gain_or_offset]=false; fi
        if [[ -z "${ALL_KEYS[treadmill_follow_wattage]+x}" ]]; then ALL_KEYS[treadmill_follow_wattage]=false; fi
        if [[ -z "${ALL_KEYS[treadmill_force_speed]+x}" ]]; then ALL_KEYS[treadmill_force_speed]=true; fi
        if [[ -z "${ALL_KEYS[treadmill_incline_max]+x}" ]]; then ALL_KEYS[treadmill_incline_max]=100; fi
        if [[ -z "${ALL_KEYS[treadmill_incline_min]+x}" ]]; then ALL_KEYS[treadmill_incline_min]=-100; fi
        if [[ -z "${ALL_KEYS[treadmill_pid_heart_max]+x}" ]]; then ALL_KEYS[treadmill_pid_heart_max]=0; fi
        if [[ -z "${ALL_KEYS[treadmill_pid_heart_min]+x}" ]]; then ALL_KEYS[treadmill_pid_heart_min]=0; fi
        if [[ -z "${ALL_KEYS[treadmill_pid_heart_zone]+x}" ]]; then ALL_KEYS[treadmill_pid_heart_zone]="Disabled"; fi
        if [[ -z "${ALL_KEYS[treadmill_simulate_inclination_with_speed]+x}" ]]; then ALL_KEYS[treadmill_simulate_inclination_with_speed]=false; fi
        if [[ -z "${ALL_KEYS[treadmill_speed_max]+x}" ]]; then ALL_KEYS[treadmill_speed_max]=100; fi
        if [[ -z "${ALL_KEYS[treadmill_step_incline]+x}" ]]; then ALL_KEYS[treadmill_step_incline]=0.5; fi
        if [[ -z "${ALL_KEYS[treadmill_step_speed]+x}" ]]; then ALL_KEYS[treadmill_step_speed]=0.5; fi

        if [[ -z "${ALL_KEYS[virtual_device_bluetooth]+x}" ]]; then ALL_KEYS[virtual_device_bluetooth]=true; fi
        if [[ -z "${ALL_KEYS[virtual_device_enabled]+x}" ]]; then ALL_KEYS[virtual_device_enabled]=true; fi
        if [[ -z "${ALL_KEYS[virtual_device_force_treadmill]+x}" ]]; then ALL_KEYS[virtual_device_force_treadmill]=true; fi
        if [[ -z "${ALL_KEYS[virtual_device_echelon]+x}" ]]; then ALL_KEYS[virtual_device_echelon]=false; fi
        if [[ -z "${ALL_KEYS[virtual_device_force_bike]+x}" ]]; then ALL_KEYS[virtual_device_force_bike]=false; fi
        if [[ -z "${ALL_KEYS[virtual_device_ifit]+x}" ]]; then ALL_KEYS[virtual_device_ifit]=false; fi
        if [[ -z "${ALL_KEYS[virtual_device_onlyheart]+x}" ]]; then ALL_KEYS[virtual_device_onlyheart]=false; fi
        if [[ -z "${ALL_KEYS[virtual_device_rower]+x}" ]]; then ALL_KEYS[virtual_device_rower]=false; fi
        if [[ -z "${ALL_KEYS[virtualbike_forceresistance]+x}" ]]; then ALL_KEYS[virtualbike_forceresistance]=true; fi

        # Write sorted keys for deterministic output
        for k in $(printf '%s\n' "${!ALL_KEYS[@]}" | sort); do
            printf '%s=%s\n' "$k" "${ALL_KEYS[$k]}"
        done
    } > "$temp_file"

    # Atomic move to destination
    if ! mv -f "$temp_file" "$config_path"; then
        echo "ERROR: Failed to move generated config to $config_path" >&2
        rm -f "$temp_file" 2>/dev/null || true
        return 1
    fi

    # Basic validation
    if [[ ! -f "$config_path" ]]; then
        echo "ERROR: Config file missing after move: $config_path" >&2
        return 1
    fi
    if ! grep -q "^\[General\]$" "$config_path"; then
        echo "ERROR: Generated config missing [General] header" >&2
        return 1
    fi

    return 0
}

# Simple CLI flags: support --version (non-intrusive)
while [ "$#" -gt 0 ]; do
    case "$1" in
        --help|-h)
                printf 'Usage: %s [--version|--scan-now]\n' "${0##*/}"
            exit 0
            ;;
            --scan-now)
                SCAN_NOW=1
                NONINTERACTIVE_SHOW_CURSOR=1
                shift
                ;;
            --no-extract)
                export SKIP_TEST_EXTRACT=1
                shift
                ;;
    esac
    # Leave unknown flags for later parsing
    break
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
        local ui_fd
        ui_fd=$(get_safe_ui_fd)
        ( printf '\033[?25h' >&"${ui_fd}" ) 2>/dev/null || true
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
        ( printf '\033[?25h' >&"${ui_fd}" ) 2>/dev/null || true
    fi

    stty echo 2>/dev/null || true

    # Move cursor to safe row before exit
    local dest_row=24
    if [[ -n "${LOG_BOTTOM:-}" ]]; then
        dest_row=$(( LOG_BOTTOM + 3 ))
    fi
    local ui_fd
    ui_fd=$(get_safe_ui_fd)
    ( printf "\033[%d;1H" "$dest_row" >&"${ui_fd}" ) 2>/dev/null || true

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
        ( printf '\033[?25h' >&"${UI_FD:-2}" ) 2>/dev/null || true
    fi
    # Move the cursor down to a safe row (row 24) before exiting so the
    # interactive shell prompt does not overwrite the drawn UI area.
    local dest_row=24
    if [[ -n "${LOG_BOTTOM:-}" ]]; then
        # Prefer 3 rows below LOG_BOTTOM when available
        dest_row=$(( LOG_BOTTOM + 3 ))
    fi
    ( printf "\033[%d;1H" "$dest_row" >&"${UI_FD:-2}" ) 2>/dev/null || true
    
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
    BOLD=''; BOLD_RED=''; BOLD_BLUE=''; BOLD_CYAN=''; BOLD_WHITE=''
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
    local r=$(( _arg_r + 1 ))
    local c=$(( _arg_c + 1 ))
    local ui_fd
    ui_fd=$(get_safe_ui_fd)
    ( printf "\033[%d;%dH" "$r" "$c" >&"${ui_fd}" ) 2>/dev/null || true
}

 
clear_screen() { local ui_fd; ui_fd=$(get_safe_ui_fd); ( printf "\033[2J\033[H" >&"${ui_fd}" ) 2>/dev/null || true; }
hide_cursor() { local ui_fd; ui_fd=$(get_safe_ui_fd); ( printf "\033[?25l" >&"${ui_fd}" ) 2>/dev/null || true; }
show_cursor() { local ui_fd; ui_fd=$(get_safe_ui_fd); ( printf "\033[?25h" >&"${ui_fd}" ) 2>/dev/null || true; }

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
LOG_TOP=12
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



# Status grid definition: each entry is "Left Label|Left Key|Right Label|Right Key"
declare -a STATUS_GRID=(
    "Python 3.11 Library|python311|Python PIPs|pkg_pips"
    "Qt5 Runtime Libraries|qt5_libs|QML Modules|qml_modules"
    "Python Virtual Environment|venv|Configuration File|config_file"
    "User in plugdev Group|plugdev|USB udev Rules|udev_rules"
    "Bluetooth Service|bluetooth|QZ Service|qz_service"
    "lsusb Command|lsusb|ANT+ USB Dongle|ant_dongle"
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

# Optional Python provider integration (minimal, safe wrappers).
# Note: debug logging removed; `bt_debug` calls were stripped from this file.
# Prefer FIFO in shared memory to avoid SD card writes.
BT_PROVIDER_SUPERVISOR_PID=0
BT_PROVIDER_PIDFILE=""
BT_PROVIDER_STOP_FILE=""
BT_PROVIDER_HEARTBEAT=""

# Standardized temp paths (inside validated TEMP_DIR)
BT_PROVIDER_FIFO="${BT_PROVIDER_FIFO:-$TEMP_DIR/qz_bt_fifo_$$}"
BT_PROVIDER_PIDFILE=${BT_PROVIDER_PIDFILE:-$TEMP_DIR/qz_bt_provider_$$.pid}
BT_PROVIDER_STOP_FILE=${BT_PROVIDER_STOP_FILE:-$TEMP_DIR/qz_bt_provider.stop}
BT_PROVIDER_HEARTBEAT=${BT_PROVIDER_HEARTBEAT:-$TEMP_DIR/qz_bt_heartbeat_$$}

# Resolve BT IPC paths; prefer TEMP_DIR (RAM). Disable stream if unwritable.
resolve_bt_paths() {
    # Resolve provider stream
    BT_PROVIDER_STREAM=${BT_PROVIDER_STREAM:-$TEMP_DIR/qz_bt_stream.log}
    local stream="$BT_PROVIDER_STREAM"
    # Prefer TEMP_DIR (RAM-backed). Only fall back to per-user cache if
    # TEMP_DIR is not writable for some reason.
    mkdir -p "$(dirname "$stream")" 2>/dev/null || true
    : > "$stream" 2>/dev/null || true
    if [ ! -w "$stream" ]; then
        # TEMP_DIR not writable — disable stream file to avoid disk writes
        BT_PROVIDER_STREAM="/dev/null"
    fi

}

# ============================================================================
# ROBUST BLUETOOTH PROVIDER PROCESS MANAGEMENT
# ============================================================================

# Check if a process is the actual bt_provider (not a false positive)
is_bt_provider_process() {
    local pid="$1"
    [[ -z "$pid" ]] && return 1

    # Verify process exists and command matches
    if ps -p "$pid" -o cmd= 2>/dev/null | grep -q "[b]t_provider.py"; then
        return 0
    fi
    return 1
}

# Kill process with graceful fallback to SIGKILL
kill_gracefully() {
    local pid="$1"
    local max_wait="${2:-2}"  # Wait up to 2 seconds

    [[ -z "$pid" ]] && return 0

    # Send SIGTERM first
    kill -TERM "$pid" 2>/dev/null || return 0

    # Wait for graceful shutdown
    local waited=0
    while [[ $waited -lt $((max_wait * 10)) ]]; do
        if ! kill -0 "$pid" 2>/dev/null; then
            return 0  # Process exited cleanly
        fi
        sleep 0.1
        ((waited++))
    done

    # Force kill if still alive
    kill -KILL "$pid" 2>/dev/null || true
    sleep 0.1
    return 0
}

# Override start/stop provider with atomic, robust implementations
start_bt_provider() {
    # Choose python binary inside venv if available
    local pybin="$TARGET_HOME/ant_venv/bin/python3"
    if [ ! -x "$pybin" ]; then
        pybin=$(command -v python3 || true)
    fi
    [ -n "$pybin" ] || return 1

    # Provider path relative to this script
    local prov
    prov="$(dirname "$0")/bt_provider.py"
    if [ ! -f "$prov" ]; then
        prov="$(dirname "${BASH_SOURCE[0]}")/bt_provider.py"
    fi
    if [ ! -f "$prov" ]; then
        return 1
    fi

    # Record absolute provider path for stop logic
    # shellcheck disable=SC2034
    BT_PROVIDER_PROV_PATH=$(readlink -f "$prov" 2>/dev/null || echo "$prov")

    # Pick writable paths for stream/debug
    resolve_bt_paths

    # Prepare IPC in TEMP_DIR (RAM-first)
    BT_PROVIDER_FIFO="${BT_PROVIDER_FIFO:-$TEMP_DIR/qz_bt_fifo_$$}"
    BT_PROVIDER_PIDFILE="${BT_PROVIDER_PIDFILE:-$TEMP_DIR/qz_bt_provider_$$.pid}"
    BT_PROVIDER_HEARTBEAT="${BT_PROVIDER_HEARTBEAT:-$TEMP_DIR/qz_bt_heartbeat_$$}"
    BT_PROVIDER_STOP_FILE="${BT_PROVIDER_STOP_FILE:-$TEMP_DIR/qz_bt_provider.stop}"
    mkdir -p "$(dirname "$BT_PROVIDER_FIFO")" 2>/dev/null || true

    # Ensure old IPC artifacts removed
    rm -f "$BT_PROVIDER_FIFO" "$BT_PROVIDER_HEARTBEAT" "$BT_PROVIDER_STOP_FILE" 2>/dev/null || true

    # Create FIFO (if creation fails, fall back to regular file)
    if ! mkfifo "$BT_PROVIDER_FIFO" 2>/dev/null; then
        : > "$BT_PROVIDER_FIFO" 2>/dev/null || true
    fi
    chmod 0666 "$BT_PROVIDER_FIFO" 2>/dev/null || true

    # If running as root, chown to target user
    if [ "$(id -u)" -eq 0 ] && [ -n "${TARGET_USER:-}" ]; then
        chown "$TARGET_USER":"$TARGET_USER" "$BT_PROVIDER_STREAM" 2>/dev/null || true
    fi

    # Check if supervisor already running
    if [ -n "${BT_PROVIDER_SUPERVISOR_PID:-}" ] && ps -p "${BT_PROVIDER_SUPERVISOR_PID}" >/dev/null 2>&1; then
        return 0
    fi

    # Ensure old stop file removed
    rm -f "$BT_PROVIDER_PIDFILE" 2>/dev/null || true

    # ATOMIC START: Launch provider with supervisor in single subshell
    (
        # Launch provider
        "$pybin" "$prov" --heartbeat="$BT_PROVIDER_HEARTBEAT" >"$BT_PROVIDER_FIFO" 2>/dev/null &
        local pp=$!
        echo "$pp" > "$BT_PROVIDER_PIDFILE" 2>/dev/null || true

        # Watchdog loop with stop-file check
        while true; do
            sleep 5

            # Stop requested via file flag?
            if [ -f "$BT_PROVIDER_STOP_FILE" ]; then
                kill_gracefully "$pp" 2
                break
            fi

            # Provider died unexpectedly?
            if ! is_bt_provider_process "$pp"; then
                break
            fi

            # Heartbeat stale (>15s old)?
            if [ -f "$BT_PROVIDER_HEARTBEAT" ]; then
                local age
                age=$(( $(date +%s) - $(stat -c %Y "$BT_PROVIDER_HEARTBEAT" 2>/dev/null || echo 0) ))
                if [ "$age" -gt 15 ]; then
                    kill_gracefully "$pp" 2
                    break
                fi
            fi
        done

        # Cleanup
        rm -f "$BT_PROVIDER_PIDFILE" "$BT_PROVIDER_HEARTBEAT" "$BT_PROVIDER_FIFO" 2>/dev/null || true
    ) &

    BT_PROVIDER_SUPERVISOR_PID=$!
    return 0
}

stop_bt_provider() {
    # Signal supervisor to stop via file flag (atomic operation)
    touch "$BT_PROVIDER_STOP_FILE" 2>/dev/null || true

    # Kill provider process if pidfile exists
    if [ -f "$BT_PROVIDER_PIDFILE" ]; then
        local ppid
        ppid=$(cat "$BT_PROVIDER_PIDFILE" 2>/dev/null || true)
        if [ -n "$ppid" ] && is_bt_provider_process "$ppid"; then
            kill_gracefully "$ppid" 2
        fi
        rm -f "$BT_PROVIDER_PIDFILE" 2>/dev/null || true
    fi

    # Kill supervisor if present
    if [ -n "${BT_PROVIDER_SUPERVISOR_PID:-}" ]; then
        if ps -p "$BT_PROVIDER_SUPERVISOR_PID" >/dev/null 2>&1; then
            kill_gracefully "$BT_PROVIDER_SUPERVISOR_PID" 1
        fi
        BT_PROVIDER_SUPERVISOR_PID=0
    fi

    # Clean up IPC artifacts
    rm -f "$BT_PROVIDER_STOP_FILE" "$BT_PROVIDER_PIDFILE" "$BT_PROVIDER_FIFO" "$BT_PROVIDER_HEARTBEAT" 2>/dev/null || true
}


# Optional delay between status/UI checks (seconds). Set via env var `CHECK_DELAY`.
# Default is 0 (no artificial delays). Use fractional values like 0.1 if desired.

# ============================================================================
# DISPLAY WIDTH CALCULATION
# ============================================================================

# shellcheck disable=SC2034
# Unified cache: raw_string -> "stripped_text|width"
declare -gA DISPLAY_CACHE
# Cache for pre-computed ANSI cursor position sequences (row -> escape seq)
# shellcheck disable=SC2034
declare -gA ANSI_CACHE

# Eagerly pre-compute common cursor sequences to avoid arithmetic in hot path
# Precompute rows 0..100 which covers typical dashboard sizes
for ((_r=0; _r<=100; _r++)); do
    ANSI_CACHE[$_r]="\033[$(( _r + 1 ));1H"
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
    local now age
    now=$(date +%s)
    age=$(( now - _UI_FD_CACHE_TIME ))
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

# Standard dashboard print (Now unbuffered)
print_at() {
    local row=$1
    shift
    # Ensure the cursor is hidden while we position/print the line
    if command -v hide_cursor >/dev/null 2>&1; then
        hide_cursor || true
    fi
    # Move cursor to Row, Column 1 using centralized helper and print to UI fd
    # (debug logging removed)
    # Use cached UI FD determination to avoid repeated filesystem checks
    UI_FD=$(get_safe_ui_fd)

    # Compose the line to print (remaining args). Use safe printing to
    # avoid interpreting user-supplied content as printf format strings
    # (prevent accidental expansion of percent sequences or backslash
    # escapes that could move the cursor).
    local line esc
    line="$*"
    esc=$(printf '\033[%d;1H' "$((row + 1))")
    # If a modal is active, log calls that would write the full panel
    if [[ "${UI_MODAL_ACTIVE:-0}" -eq 1 ]]; then
        printf '%s\n' "[DEBUG] print_at called row=${row} UI_MODAL_ACTIVE=1 caller=$(caller 0 || true)" >> /tmp/qz_profile_debug.log 2>/dev/null || true
    fi
    # Print cursor position then the literal line string. Use '%s' so
    # printf treats the arguments as data, not format strings.
    ( printf '%s%s' "$esc" "$line" >&"${UI_FD}" ) 2>/dev/null || true
    return 0
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

    # Optional ASCII-only fast-path (disabled by default). Enable by
    # exporting QZ_ASCII_FAST_PATH=1 in the environment. This avoids
    # invoking perl/gawk for the very common case of ASCII-only names.
    local enable_ascii_fast=${QZ_ASCII_FAST_PATH:-0}
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

    # Padlock Logic for GUI environments
    if [ "$HAS_GUI" = true ] && [[ " ${PROTECTED_ITEMS[*]} " == *" $key "* ]] && [ "$status" == "pass" ]; then
        # We use BOLD_GRAY to make the lock distinctive (muted)
        printf '%s' "${BOLD_GRAY}${SYMBOL_LOCKED}${NC}"
        return
    fi

    # If a SYMBOL_CACHE is populated, prefer it to avoid repeated printf calls
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
    
    # 1. Get icons (✓, ✗, or ⚿)
    local L_sym
    local R_sym
    L_sym=$(get_symbol "$L_key")
    R_sym=$(get_symbol "$R_key")
    
    # 2. Get label colors and build strings
    local L_color R_color
    L_color=$(get_status_label_color "$L_key")
    R_color=$(get_status_label_color "$R_key")
    
    local L_content="${L_sym} ${L_color}${L_label}${NC}"
    local R_content="${R_sym} ${R_color}${R_label}${NC}"
    
    # 3. Calculate padding: split INNER_COLS into left/right with center separator
    local L_padded
    local R_padded
    local left_w right_w
    # Total content width: INNER_COLS (area between ║ ║) - 3 (one space, one separator, one space)
    # The original split logic is robust.
    left_w=$(( (INNER_COLS - 3) / 2 ))
    right_w=$(( INNER_COLS - 3 - left_w ))
    L_padded=$(pad_display "$L_content" "$left_w")
    R_padded=$(pad_display "$R_content" "$right_w")
    
    # 4. Print the row
    print_at "$row" "${BLUE}║${NC} ${L_padded}${BLUE}│${NC}${R_padded} ${BLUE}║${NC}"

}

draw_header_config_line() {
    local inner_w=$INNER_COLS
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
    print_at 2 "${BLUE}║${CYAN}$(pad_display "$line" "$inner_w")${BLUE}║${NC}"
}

draw_header_service_line() {
    local inner_w=$INNER_COLS
    # 1. Initialize variables with defaults to prevent 'unbound variable' errors
    local svc_sym="●"
    local svc_color="$GRAY"
    local svc_path="$SERVICE_FILE_QZ"

    # 2. Prefer STATUS_MAP if available so header matches status panel
    local svc_map_status
    svc_map_status=${STATUS_MAP["qz_service"]:-}
    if [[ -n "$svc_map_status" ]]; then
        case "$svc_map_status" in
            pass)
                svc_sym="$SYMBOL_PASS"
                svc_color="$GREEN"
                ;;
            fail)
                svc_sym="$SYMBOL_FAIL"
                svc_color="$RED"
                ;;
            *)
                svc_sym="●"
                svc_color="$GRAY"
                ;;
        esac
        # Determine path if present
        if [[ -n "${ACTIVE_SERVICE_FILE:-}" && -f "$ACTIVE_SERVICE_FILE" ]]; then
            svc_path="$ACTIVE_SERVICE_FILE"
        elif [[ -f "$SERVICE_FILE_QZ" ]]; then
            svc_path="$SERVICE_FILE_QZ"
        fi
    else
        # Fallback to runtime check
        local svc_state
        svc_state=$(get_service_status)
        case "$svc_state" in
            running)
                svc_sym="$SYMBOL_PASS"
                svc_color="$GREEN"
                ;;
            failed)
                svc_sym="$SYMBOL_FAIL"
                svc_color="$RED"
                ;;
            *)
                svc_sym="●"
                svc_color="$GRAY"
                ;;
        esac
        if [[ -f "$SERVICE_FILE_QZ" ]]; then
            svc_path="$SERVICE_FILE_QZ"
        fi
    fi
    
    # 3. Build the line carefully
    # We use explicit colors for the symbol, then switch back to CYAN for the path
    local line="  Service: ${svc_color}${svc_sym}${CYAN} ${svc_path}"
    
    # 4. Print to Row 3 (New Layout)
    print_at 3 "${BLUE}║${CYAN}$(pad_display "$line" "$inner_w")${BLUE}║${NC}"
}

draw_top_panel() {
    local inner_w=$INNER_COLS
    # Top Border
    draw_hr 0 "╔" "╗" "QZ ANT+ BRIDGE SETUP & DIAGNOSTICS UTILITY" ""

    # User/Environment/Paths (Standard rows)
    local env_str
    if [[ "$HAS_GUI" == true ]]; then
        env_str="GUI (X11/Wayland)"
    else
        env_str="Headless"
    fi
    print_at 1 "${BLUE}║${CYAN}$(pad_display "  User: $TARGET_USER | Environment: $env_str" "$inner_w")${BLUE}║${NC}"
    draw_header_config_line
    draw_header_service_line

    # Status Header with Legend
    # Compact status legend: show only colored symbols (no descriptive text)
    # No inline legend in the header — the popup covers explanations.
    local full_legend=""

    # Arguments: Row, LeftCorner, RightCorner, Text, TextColor, Legend
    # Status header shown in the information panel (kept as 'STATUS')
    draw_hr 4 "╠" "╣" "STATUS" "$BOLD_WHITE" "$full_legend"
    render_status_grid 5
}

# Usage: draw_hr <row> <left_corner> <right_corner> <text> <text_color> [legend_text]
draw_hr() {
    local row=$1
    local left_c=$2
    local right_c=$3
    local text="${4:-}"
    local t_color="${5:-$BOLD_WHITE}"
    local legend="${6:-}"
    local inner_w=$INNER_COLS

    # 1. Position cursor at the start of the line
    local ui_fd
    ui_fd=$(get_safe_ui_fd)
    ( printf "\033[%d;1H" "$((row + 1))" >&"${ui_fd}" ) 2>/dev/null || true

    # Left/right visual paddings
    local left_pad="═══  "
    local right_pad="══"
    local sep="  "

    # 2. Calculate visual widths (account for ANSI and Unicode)
    local t_vis l_vis
    t_vis=$(get_vis_width "$text")
    l_vis=$(get_vis_width "$legend")

    # (debug logging removed)

    # If both text and legend are visually empty (may contain ANSI only),
    # render a solid border to avoid gaps caused by color-only strings.
    if [[ $t_vis -eq 0 && $l_vis -eq 0 ]]; then
        local fill=""
        for ((i=0; i<inner_w; i++)); do fill="${fill}═"; done
        local ui_fd
        ui_fd=$(get_safe_ui_fd)
        ( printf '%s%s%s' "${BLUE}" "${left_c}${fill}${right_c}" "${NC}" >&"${ui_fd}" ) 2>/dev/null || true
        # Hide cursor for very low rows as before
        if [[ "$row" -ge 22 ]]; then hide_cursor; fi
        return 0
    fi

    # Otherwise, build header/footer with text/legend
    
    # 3. Calculate visual widths for provided text/legend

        local left_w=${#left_pad}
        local right_w=${#right_pad}
        local sep_w=${#sep}

        # available fill width between text and legend
        local fill_w=$(( inner_w - left_w - t_vis - sep_w - l_vis - right_w ))

        # If not enough space, truncate legend first (keeps it right-aligned), then text
        if [ "$fill_w" -lt 0 ]; then
            local need=$(( -fill_w ))
            if [ "$l_vis" -gt 0 ]; then
                local new_l_vis=$(( l_vis - need ))
                if [ "$new_l_vis" -lt 0 ]; then new_l_vis=0; fi
                legend=$(trunc_vis "$legend" $new_l_vis)
                l_vis=$(get_vis_width "${legend}")
                fill_w=$(( inner_w - left_w - t_vis - sep_w - l_vis - right_w ))
            fi
        fi

        if [ "$fill_w" -lt 0 ]; then
            local need2=$(( -fill_w ))
            if [ "$t_vis" -gt 0 ]; then
                local new_t_vis=$(( t_vis - need2 ))
                if [ "$new_t_vis" -lt 0 ]; then new_t_vis=0; fi
                text=$(trunc_vis "$text" $new_t_vis)
                t_vis=$(get_vis_width "${text}")
                fill_w=$(( inner_w - left_w - t_vis - sep_w - l_vis - right_w ))
            fi
        fi

        if [ $fill_w -lt 0 ]; then fill_w=0; fi

        local fill=""
        for ((i=0; i<fill_w; i++)); do fill="${fill}═"; done

        # 3. Build and Print as one atomic operation to prevent flickering
        # Use the shared builder to render the horizontal rule and print it.
        local _hr
        _hr=$(build_hr_string "$row" "$left_c" "$right_c" "$text" "$t_color" "$legend")
        local ui_fd
        ui_fd=$(get_safe_ui_fd)
        ( printf '%s' "$_hr" >&"${ui_fd}" ) 2>/dev/null || true

    # 4. For very low rows we only need to hide the cursor; avoid moving
    # the cursor position here as it can interfere with subsequent prints
    # and cause overlay/overprint issues on some terminals.
    if [[ "$row" -ge 22 ]]; then
        hide_cursor
    fi
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
        printf "\033[%d;1H%s%s%s" "$((row + 1))" "${BLUE}" "${left_c}${fill}${right_c}" "${NC}"
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

    printf "\033[%d;1H%s" "$((row + 1))" "$line"
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
    # Footer uses BOLD_BLUE for the text, No Legend
    # Build footer via builder and print once for atomicity
    local _foot
    _foot=$(build_hr_string "$b_row" "╚" "╝" "${BOLD_BLUE}${help_text}${NC}" "")
    local ui_fd
    ui_fd=$(get_safe_ui_fd)
    ( printf '%s' "$_foot" >&"${ui_fd}" ) 2>/dev/null || true
}

# Clear the info/interactive area between LOG_TOP and LOG_BOTTOM
clear_info_area() {
    # Use explicit print_at with padded empty content to reliably overwrite
    # any previous characters (including stray control sequences).
    for ((r=LOG_TOP; r<=LOG_BOTTOM; r++)); do
        print_at "$r" "${BLUE}║${NC}$(pad_display "" "$INFO_WIDTH")${BLUE}║${NC}"
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
    # Interpret backslash-escaped sequences (\n) in messages so callers
    # can pass human-friendly \n markers. Use printf '%b' to expand them.
    IFS=$'\n' read -r -d '' -a wrapped < <(printf '%b' "$msg" | fold -s -w $((INFO_WIDTH - 3)) && printf '\0')

    for line in "${wrapped[@]}"; do
        row=$((row + 1))
        draw_sealed_row "$row" "   ${RED}${line}${NC}"
    done

sleep 5
    draw_bottom_border "Press ENTER to continue"

    # Treat any non-empty third argument as a request to pause.
    # If it's a positive integer, sleep that many seconds; otherwise
    # wait for a single keypress (legacy behaviour when callers pass "wait").
    if [[ -n "${wait_enter:-}" && "${wait_enter}" != "0" ]]; then
        if [[ "${wait_enter}" =~ ^[0-9]+$ ]]; then
            sleep "${wait_enter}"
        else
            # Wait for a single keypress without showing the cursor to avoid
            # a flashing cursor in the footer area. Keep UI mode active so the
            # rendered error panel remains visible and consistent.
            local k
            safe_read_key k
        fi
    fi

    # Clear the error area after dismiss
    clear_info_area
    exit_ui_mode
}

# Neutral informational panel (not an error). Usage mirrors draw_error_screen:
# draw_info_screen "TITLE" "Message" [wait]
draw_info_screen() {
    local title="${1:-INFO}"
    local msg="${2:-}"
    local wait_enter=${3:-1}

    enter_ui_mode || true
    clear_info_area

    local row=$((LOG_TOP + 1))
    draw_sealed_row "$row" "   ${BOLD_BLUE}${title}${NC}"

    local wrapped
    IFS=$'\n' read -r -d '' -a wrapped < <(printf '%b' "$msg" | fold -s -w $((INFO_WIDTH - 3)) && printf '\0')
    for line in "${wrapped[@]}"; do
        row=$((row + 1))
        draw_sealed_row "$row" "   ${NC}${line}${NC}"
    done

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

    clear_info_area
    exit_ui_mode || true
}

draw_bottom_panel_header() {
    local raw_title="${1:-INFORMATION}"
    local title
    title=$(echo "$raw_title" | tr '[:lower:]' '[:upper:]')
    # Build header string atomically and print once to avoid interleaved
    # cursor movements from other concurrent prints.
    local _hr
    _hr=$(build_hr_string 11 "╠" "╣" "$title" "$BOLD_WHITE" "${GRAY}L: Legend${NC}")
    local ui_fd
    ui_fd=$(get_safe_ui_fd)
    ( printf '%s' "$_hr" >&"${ui_fd}" ) 2>/dev/null || true
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
        
        print_at $((LOG_TOP + idx)) "${BLUE}║${NC}$(pad_display " $content" "$INFO_WIDTH")${BLUE}║${NC}"
    done
    
    local start_fill=$((LOG_TOP + show_lines))
    for ((r=start_fill; r<=LOG_BOTTOM; r++)); do
        print_at "$r" "${BLUE}║${NC}$(pad_display "" "$INFO_WIDTH")${BLUE}║${NC}"
    done

}

# Display a temporary Legend popup in the info area. This is non-destructive
# and returns immediately after the user presses any key.
show_legend_popup() {
    enter_ui_mode
    clear_info_area
    # Temporarily set the bottom panel header to indicate this is the legend
    draw_bottom_panel_header "STATUS GUIDE"

    local row=$((LOG_TOP + 1))
    row=$((row + 1))
    draw_sealed_row "$row" "   ${GREEN}${SYMBOL_PASS}${NC}  Ready       — Working correctly"
    row=$((row + 1))
    draw_sealed_row "$row" "   ${YELLOW}${SYMBOL_WARN}${NC}  Warning     — Required for full functionality"
    row=$((row + 1))
    draw_sealed_row "$row" "   ${RED}${SYMBOL_FAIL}${NC}  Missing     — Needs installation"
    row=$((row + 1))
    draw_sealed_row "$row" "   ${BOLD_GRAY}${SYMBOL_LOCKED}${NC}  Protected   — Do not remove (system-managed)"
    row=$((row + 1))
    draw_sealed_row "$row" "   ${GRAY}${SYMBOL_PENDING}${NC}  Service     — Background service not setup up"

    draw_bottom_border "Press any key to continue"
    local k
    safe_read_key k

    clear_info_area
    exit_ui_mode
}




update_status() {
    STATUS_MAP["$1"]="$2"
    # Re-render the whole grid so status icons stay consistent
    render_status_grid 5
}

# Atomic status updater: update only the affected cell (left or right)
update_status_atomic() {
    local key="$1"
    local status="$2"
    STATUS_MAP["$key"]="$status"

    # Find which row contains this key
    local row_idx=0
    local start_row=5
    for entry in "${STATUS_GRID[@]}"; do
        IFS='|' read -r L_label L_key R_label R_key <<< "$entry"
        local target_row=$(( start_row + row_idx ))

        local left_w=$(( (INNER_COLS - 3) / 2 ))
        local right_w=$(( INNER_COLS - 3 - left_w ))

        if [[ "$L_key" == "$key" ]]; then
            local L_sym
            L_sym=$(get_symbol "$L_key")
            local L_content="${L_sym} $(get_status_label_color "$L_key")${L_label}${NC}"
            local L_padded
            L_padded=$(pad_display "$L_content" "$left_w")
            # Print left half including left border and separator
            print_at_col "$target_row" 1 "${BLUE}║${NC} ${L_padded}${BLUE}│${NC}"
            return 0
        elif [[ "$R_key" == "$key" ]]; then
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
    render_status_grid 5
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
    # Prefer RAM-backed TEMP_DIR (ensure_ram_temp_dir sets this earlier)
    buf=$(mktemp "${TEMP_DIR:-/tmp}/qz_screen.XXXXXX" 2>/dev/null) || buf=$(mktemp /tmp/qz_screen.XXXXXX 2>/dev/null) || buf="/tmp/qz_screen.$$"

    # Open a dedicated FD for UI output to capture functions that write
    # directly to $UI_FD (print_at, print_at_col, etc.). Use FD 9 to
    # avoid clashing with existing FDs.
    exec 9>"$buf" 2>/dev/null || exec 9>"$buf" || true
    local prev_ui_fd="${UI_FD:-2}"
    UI_FD=9

    # Temporarily override print helpers so they write directly to FD 9
    local orig_print_at orig_print_at_col
    orig_print_at="$(declare -f print_at 2>/dev/null || true)"
    orig_print_at_col="$(declare -f print_at_col 2>/dev/null || true)"

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

    # Restore original print helpers
    if [[ -n "$orig_print_at" ]]; then eval "$orig_print_at"; fi
    if [[ -n "$orig_print_at_col" ]]; then eval "$orig_print_at_col"; fi

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
    # Load and sanitize profile values from $CONFIG_FILE
    
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
            # If stored value is obviously corrupted (too large), try recovering
            # by taking the last 2-3 digits (covers stray '25' CSI digits like ^[[?25l).
            if [[ -n "$_w_sanitized" ]]; then
                local _w_int=${_w_sanitized%%.*}
                if (( _w_int >= 20 && _w_int <= 300 )); then
                    PREV_WEIGHT="$_w_sanitized"
                else
                    # try last 2 then last 3 digits as fallback
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
            local _a_sanitized
            _a_sanitized="${a//[^0-9]/}"
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
    fi
}

 

# Update a key in $CONFIG_FILE (create dir/file if needed). Sanitize values.
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
        # Remove common ANSI/CSI escape sequences first (e.g. ESC[?25l)
        if command -v perl >/dev/null 2>&1; then
            value=$(printf '%s' "$value" | perl -pe 's/\e\[[\d;?;]*[A-Za-z]//g')
        else
            value=$(printf '%s' "$value" | sed -E "s/$(printf '\033')\\[[0-9;?]*[A-Za-z]//g")
        fi
        value=$(echo "$value" | cut -d. -f1)
        # Ensure only digits remain (filter out any remaining control chars)
        value=$(echo "$value" | tr -cd '0-9')
        # If the remaining digits look like a concatenation of an escape
        # parameter (e.g. "25") and the real number (e.g. "78" -> "2578"),
        # prefer the trailing digits that produce a sensible value.
        if [[ "$key" == "weight" ]]; then
            # try last 2 then 3 digits to find a weight between 20 and 300
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
        # If running as root, ensure the target user owns the config dir
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
        # Use @ as sed delimiter to avoid issues with slashes
        sed -i "s@^${key}=.*@${key}=${value}@" "$CONFIG_FILE" 2>/dev/null || true
    else
        echo "${key}=${value}" >> "$CONFIG_FILE" 2>/dev/null || true
    fi
    # Keep typed arrays in sync for generated configs
    # Normalize value for classification (lowercase booleans)
    local _val_norm
    _val_norm="${value,,}"
    classify_and_store "$key" "$_val_norm" || true
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

configure_user_profile() {
    load_current_profile_values
    # Optional debug log for reproducing interactive flow problems
    if [[ "${QZ_DEBUG_PROFILE:-0}" -eq 1 ]]; then
        : > /tmp/qz_profile_debug.log 2>/dev/null || true
        echo "[DEBUG] enter configure_user_profile: $(date +%s)" >> /tmp/qz_profile_debug.log 2>/dev/null || true
    fi
    # Clear ANSI position and display caches to avoid stale mappings from previous UI
    # shellcheck disable=SC2034
    declare -gA ANSI_CACHE=()
    declare -gA DISPLAY_CACHE=()

    # Suppress global full-screen refreshes while this modal is active
    UI_MODAL_ACTIVE=1

    # Consolidated Profile Menu (Unit toggle, Gender toggle, Weight, Age)
    # Initialize local copies using previously loaded profile values
    local _UNIT_IMPERIAL="false"
    local _GENDER_FEMALE="false"
    if [[ "${PREV_MILES:-}" == "true" ]]; then _UNIT_IMPERIAL="true"; fi
    if [[ "${PREV_SEX:-}" == "Female" ]]; then _GENDER_FEMALE="true"; fi

    # Draw menu-style profile editor (matches Service menu UX)
    draw_bottom_panel_header "USER PROFILE"
    clear_info_area

    if [[ "${QZ_DEBUG_PROFILE:-0}" -eq 1 ]]; then
        echo "[DEBUG] preparing profile menu (post selections)" >> /tmp/qz_profile_debug.log 2>/dev/null || true
    fi

    local w_unit="kg"; [[ "${_UNIT_IMPERIAL}" == "true" ]] && w_unit="lbs"

    # Work on a local copy so Cancel can revert
    local _PW="${PREV_WEIGHT:-}" _PA="${PREV_AGE:-}"

    # Initialize selection state for the profile menu
    local selected=0
    local prev_selected=0

    while true; do
        if [[ "${QZ_DEBUG_PROFILE:-0}" -eq 1 ]]; then
            echo "[DEBUG] menu-loop start selected=${selected:-UNSET} _PW=${_PW} _PA=${_PA}" >> /tmp/qz_profile_debug.log 2>/dev/null || true
        fi
        local unit_label="Metric (kg/km)"
        [[ "${_UNIT_IMPERIAL}" == "true" ]] && unit_label="Imperial (lbs/mi)"
        local gender_label="Male"
        [[ "${_GENDER_FEMALE}" == "true" ]] && gender_label="Female"

        local options=()
        options+=("Unit: ${unit_label}")
        options+=("Weight (${w_unit}): [${_PW}]")
        options+=("Age (years): [${_PA}]")
        options+=("Gender: ${gender_label}")
        # Changes auto-save on field edit
        # ESC key exits menu

        local num_options=${#options[@]}
        prev_selected=$selected

        local BASE_PAD="     "
        local ARROW="►"
        local ARROW_POS=2

        # Incremental update: avoid full clears on each loop
        # Only update the interior content of each sealed row so borders remain intact
        draw_sealed_row $((LOG_TOP)) ""
        for i in "${!options[@]}"; do
            local row=$((LOG_TOP + 1 + i))
            local opt_text="${options[$i]}"
            if [[ $i -eq $selected ]]; then
                update_sealed_row_content "$row" "   ${CYAN}► ${BOLD_CYAN}${opt_text}${NC}"
            else
                update_sealed_row_content "$row" "${BASE_PAD}${GRAY}${opt_text}${NC}"
            fi
        done

        draw_bottom_border "Arrows: Up/Down | Enter: Edit | Esc: Back"

        # Input loop
        while true; do
            local key=""
            safe_read_key key
            if [[ "${QZ_DEBUG_PROFILE:-0}" -eq 1 ]]; then
                # show a printable representation of key (show ESC as ESC)
                if [[ -z "$key" ]]; then
                    echo "[DEBUG] safe_read_key -> <EMPTY>" >> /tmp/qz_profile_debug.log 2>/dev/null || true
                else
                    printf -v _k_escaped "%q" "$key"
                    echo "[DEBUG] safe_read_key -> ${_k_escaped}" >> /tmp/qz_profile_debug.log 2>/dev/null || true
                fi
            fi
            if [[ $key == $'\x1b' ]]; then
                read_escape_sequence k2
                if [[ -z "${k2:-}" ]]; then
                    exit_ui_mode || true
                    UI_MODAL_ACTIVE=0
                    return 0
                fi
                [[ "${k2:-}" == "[A" ]] && ((selected--))
                [[ "${k2:-}" == "[B" ]] && ((selected++))
                flush_input_buffer
            elif [[ $key == "" ]]; then
                break
            fi

            [[ $selected -lt 0 ]] && selected=$((num_options - 1))
            [[ $selected -ge $num_options ]] && selected=0

            if [[ $selected -ne $prev_selected ]]; then
                local prow=$((LOG_TOP + 1 + prev_selected))
                update_sealed_row_content "$prow" "     ${GRAY}${options[$prev_selected]}${NC}"
                local nrow=$((LOG_TOP + 1 + selected))
                update_sealed_row_content "$nrow" "   ${CYAN}► ${BOLD_CYAN}${options[$selected]}${NC}"
                prev_selected=$selected
            fi
        done

        local choice_index=$selected
        local choice_text="${options[choice_index]}"

        # No explicit Save/Cancel - changes auto-save on each edit
        # ESC key from menu level will exit (handled by ESC key code)

        case "$choice_text" in
            Unit:*)
                # Toggle unit between metric/imperial
                if [[ "${_UNIT_IMPERIAL}" == "true" ]]; then
                    _UNIT_IMPERIAL="false"
                else
                    _UNIT_IMPERIAL="true"
                fi
                # update display unit immediately
                if [[ "${_UNIT_IMPERIAL}" == "true" ]]; then w_unit="lbs"; else w_unit="kg"; fi
                # Persist immediately
                if [[ "${_UNIT_IMPERIAL}" == "true" ]]; then
                    update_config_key "miles_unit" "true"
                else
                    update_config_key "miles_unit" "false"
                fi
                auto_save_config "profile" "miles_unit" || draw_error_screen "SAVE ERROR" "Failed to save profile." "wait"
                ;;
            Gender:*)
                # Toggle gender
                if [[ "${_GENDER_FEMALE}" == "true" ]]; then
                    _GENDER_FEMALE="false"
                else
                    _GENDER_FEMALE="true"
                fi
                # Persist immediately
                if [[ "${_GENDER_FEMALE}" == "true" ]]; then
                    update_config_key "sex" "Female"
                else
                    update_config_key "sex" "Male"
                fi
                auto_save_config "profile" "sex" || draw_error_screen "SAVE ERROR" "Failed to save profile." "wait"
                ;;
            Weight*)
                local row=$((LOG_TOP + 1 + choice_index))
                local cur="${_PW:-0}"
                local newv
                newv=$(prompt_numeric_input "Weight" "$w_unit" "$cur" "$row" "   ► ")
                if [[ -n "$newv" ]]; then
                    _PW="$newv"
                    update_config_key "weight" "${_PW}"
                    auto_save_config "profile" "weight" || draw_error_screen "SAVE ERROR" "Failed to save profile." "wait"
                else
                    show_cancel_feedback
                fi
                ;;
            Age*)
                local row=$((LOG_TOP + 1 + choice_index))
                local cur="${_PA:-0}"
                local newv
                newv=$(prompt_numeric_input "Age" "years" "$cur" "$row" "   ► ")
                if [[ -n "$newv" ]]; then
                    _PA="$newv"
                    update_config_key "age" "${_PA}"
                    auto_save_config "profile" "age" || draw_error_screen "SAVE ERROR" "Failed to save profile." "wait"
                else
                    show_cancel_feedback
                fi
                ;;
            *)
                draw_error_screen "UNHANDLED" "Action for ${choice_text} not implemented." "wait" ;;
        esac
        # loop and re-render
    done
}

select_equipment_flow() {
    if [ ! -f "$DEVICES_INI" ]; then
        draw_error_screen "MISSING DATABASE" "Error: Equipment database (devices.ini) not found.\nPlease ensure the file exists in the script directory." 1
        return 1
    fi

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
            # Update info header for equipment selection and clear the area
            draw_bottom_panel_header "SELECT DEVICE TYPE"
            clear_info_area
            show_scrollable_menu "SELECT DEVICE TYPE" types "$type_def_idx" "" "pad"
            local t_idx=$?
            if [ "$t_idx" -eq 255 ]; then return 1; fi 
            selected_type="${types[$t_idx]}"
            
            # Show immediate loading feedback
            draw_bottom_panel_header "LOADING ${selected_type^^} MODELS"
            clear_info_area
            local loading_msg="${YELLOW}Loading models...${NC}"
            local loading_plain="Loading models..."
            local w=$(get_display_width "$loading_plain")
            local load_row=$((LOG_TOP + 3))
            local load_col=$(( (INNER_COLS - w) / 2 ))
            print_at_col "$load_row" "$((load_col + 1))" "$loading_msg"
            
            state=1
        fi
        
        if [ "$state" -eq 1 ]; then
            # Prefer precomputed models from devices_optimized.json for snappy UI
            local models=()
            local keys=()
            local json_file="${SCRIPT_DIR}/devices_optimized.json"
            local cache_dir="${SCRIPT_DIR}/.menu_cache"
            local cache_file="$cache_dir/${selected_type}.cache"
            local model_widths=()
            if [[ -f "$cache_file" ]]; then
                # Read cache into temp arrays first, then validate IDs
                local _tmp_models=() _tmp_keys=() _tmp_widths=()
                while IFS=$'\x1f' read -r name id width; do
                    _tmp_models+=("$name")
                    _tmp_keys+=("$id")
                    _tmp_widths+=("$width")
                done < "$cache_file"

                # Validate that keys exist in DEVICES_INI; if validation fails,
                # treat cache as corrupted and fall back to JSON/devices.ini parsing.
                local _valid_cache=1
                if [[ -f "$DEVICES_INI" ]]; then
                    for _id in "${_tmp_keys[@]:-}"; do
                        if ! grep -q "=${_id}$" "$DEVICES_INI"; then
                            _valid_cache=0
                            break
                        fi
                    done
                else
                    _valid_cache=0
                fi

                if [[ $_valid_cache -eq 1 ]]; then
                    models=("${_tmp_models[@]}")
                    keys=("${_tmp_keys[@]}")
                    model_widths=("${_tmp_widths[@]}")
                fi
            elif [[ -f "$json_file" ]]; then
                # Fallback to JSON parsing (should be rare)
                while IFS=$'\x1f' read -r name id width; do
                    models+=("$name")
                    keys+=("$id")
                    model_widths+=("$width")
                done < <(python3 - <<'PY'
import json,sys
f=sys.argv[1]
cat=sys.argv[2]
try:
    with open(f,'r',encoding='utf-8') as fh:
        d=json.load(fh)
    for it in d.get('flat_menu',[]):
        if it.get('category')==cat:
            name=it.get('name') or it.get('line') or ''
            idv=it.get('id') or ''
            width=it.get('width', len(name))
            print(f"{name}\x1f{idv}\x1f{width}")
except Exception:
    pass
PY
"$json_file" "$selected_type")
            fi

            # Fallback: parse `devices.ini` if JSON absent or produced no entries
            if [[ ${#models[@]} -eq 0 ]]; then
                while IFS= read -r line; do
                    if [[ "$line" =~ = ]]; then
                        models+=("$(echo "${line%%=*}" | xargs)")
                        keys+=("$(echo "${line#*=}" | xargs)")
                        # compute length of the just-added model name
                        model_widths+=("${#models[${#models[@]}-1]}")
                    fi
                done < <(awk -v section="[$selected_type]" '$0==section { flag=1; next } /^\[/ { flag=0 } flag && $0!="" && $0!~/^;/ { print $0 }' "$DEVICES_INI")
            fi
            
            local mod_def_idx=0
            if [ -f "$CONFIG_FILE" ]; then
                # Build a set of keys that are true in the config (case-insensitive 'true')
                mapfile -t _true_keys < <(awk -F'=' '{ k=$1; v=$2; gsub(/^[ \t]+|[ \t]+$/, "", k); gsub(/^[ \t]+|[ \t]+$/, "", v); if(tolower(v)=="true") print k }' "$CONFIG_FILE")
                declare -A _TK=()
                for _k in "${_true_keys[@]:-}"; do _TK["$_k"]=1; done
                for i in "${!keys[@]}"; do
                    if [[ -n "${_TK[${keys[$i]}]:-}" ]]; then
                        mod_def_idx=$i
                        break
                    fi
                done
                unset _true_keys _TK
            fi

            # Update info header to reflect model selection
            draw_bottom_panel_header "SELECT $selected_type MODEL"
            clear_info_area

            # If we have a precomputed models list, use the fast renderer
            local m_idx
            if [[ ${#models[@]} -gt 0 ]]; then
                # Prepare per-menu cache arrays for fast rendering
                MENU_CACHE_LINES=()
                MENU_CACHE_WIDTHS=()
                for i in "${!models[@]}"; do
                    name="${models[$i]}"
                    MENU_CACHE_LINES+=("$name")
                    # Prefer width from model_widths if available (from JSON), else fall back
                    if [[ -n "${model_widths[$i]:-}" ]]; then
                        MENU_CACHE_WIDTHS+=("${model_widths[$i]}")
                    elif declare -f get_vis_width >/dev/null 2>&1; then
                        MENU_CACHE_WIDTHS+=("$(get_vis_width "$name")")
                    else
                        MENU_CACHE_WIDTHS+=("${#name}")
                    fi
                done
                MENU_CACHE_LOADED=1

                show_scrollable_menu_fast "SELECT $selected_type MODEL" "models" "$mod_def_idx" "" "pad"
                m_idx=$?

                # Reset the temporary per-menu cache to avoid reuse elsewhere
                MENU_CACHE_LOADED=0
                MENU_CACHE_LINES=()
                MENU_CACHE_WIDTHS=()
            else
                show_scrollable_menu "SELECT $selected_type MODEL" models "$mod_def_idx" "" "pad"
                m_idx=$?
            fi
            if [ "$m_idx" -eq 255 ]; then state=0; continue; fi
            
            # Show immediate feedback that we're processing
            local save_msg="${YELLOW}Saving Equipment${NC}"
            local w=$(get_display_width "Saving Equipment")
            local row=$((LOG_BOTTOM))
            local col=$((2 + INFO_WIDTH - w))
            print_at_col "$row" "$col" "$save_msg"
            
            local selected_key="${keys[$m_idx]}"
            # Defensive lookup: map the displayed model name back to the
            # canonical key in `devices.ini` to avoid mismatches when
            # menu arrays and key arrays become out-of-sync.
            local selected_name="${models[$m_idx]}"
            if [[ -n "$selected_name" && -f "$DEVICES_INI" ]]; then
                local lookup_key
                lookup_key=$(awk -F'=' -v name="$selected_name" '
                    { lhs=$1; gsub(/^[ \t]+|[ \t]+$/, "", lhs); if(lhs==name){ val=$2; gsub(/^[ \t]+|[ \t]+$/, "", val); print val; exit } }
                ' "$DEVICES_INI") || true
                if [[ -n "$lookup_key" ]]; then
                    selected_key="$lookup_key"
                fi
            fi
            
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

            # Persist consolidated config to disk
            generate_config_file "${CONFIG_FILE:-$HOME/.config/qdomyos-zwift/qDomyos-Zwift.conf}"

            # Update to success feedback (replaces the "Saving..." message)
            show_save_feedback "Equipment"
            
            # Return to main menu
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
    local log_file="$TEMP_DIR/qz_setup.log"
    local script_file="$TEMP_DIR/qz_install_step.sh"
    
    # Expand $command_text into the temporary script (allowing embedded vars)
    cat > "$script_file" <<EOF
#!/bin/bash
export DEBIAN_FRONTEND=noninteractive
set -e
$command_text
EOF
    chmod +x "$script_file"
    : > "$log_file"

    draw_bottom_panel_header "INSTALLING..."
    clear_info_area
    draw_bottom_border
    
    # --- MOVED UP BY ONE ROW ---
    # Row 2: Task Label
    draw_sealed_row $((LOG_TOP + 1)) "   ${WHITE}${label}${NC}"
    
    # Row 4: Subtext
    local subtext="Please wait..."
    [[ "$label" == *"pyenv"* ]] && subtext="Please wait... (This may take ~30 minutes on a slow device)"
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
        draw_sealed_row $((LOG_TOP + 5)) "                   ${CYAN}${bar_str}${NC}"
        
        if [ "$dir" -eq 1 ]; then
            ((pos++)); (( pos + pulse_width >= bar_width )) && dir=-1
        else
            ((pos--)); (( pos <= 0 )) && dir=1
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
    local pyenv_bin="$TARGET_HOME/.pyenv/versions/3.11.9/bin/python"
    if [ -f "$pyenv_bin" ]; then
        IS_PYENV_INSTALLED=true
        update_status "python311" "pass"
        return 0
    fi

    # 2. Check standard system path
    if command -v python3.11 >/dev/null 2>&1; then
        IS_PYSYS_INSTALLED=true
        update_status "python311" "pass"
        return 0
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
    # Map systemd unit state to our STATUS_MAP values:
    # - running -> pass
    # - failed  -> fail
    # - stopped/not-installed -> pending
    local svc_state
    svc_state=$(get_service_status)
    local status_map_val="pending"
    case "$svc_state" in
        running) status_map_val="pass" ;;
        failed)  status_map_val="fail" ;;
        *)       status_map_val="pending" ;;
    esac

    STATUS_MAP["qz_service"]="$status_map_val"
    if [[ "$status_map_val" == "fail" ]]; then
        capture_service_failure_info "qz_service" || true
    fi
    draw_header_service_line
    render_status_grid 5
    [[ "$status_map_val" != "fail" ]] && return 0 || return 1
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
    local result
    if result=$(get_cached_check "python311"); then
        echo "$result"
        return 0
    fi

    if [[ -f "${TARGET_HOME:-}/.pyenv/versions/3.11.9/bin/python" ]]; then
        cache_check_result "python311" "pass"
        echo "pass"; return 0
    fi

    if command -v python3.11 >/dev/null 2>&1; then
        cache_check_result "python311" "pass"
        echo "pass"; return 0
    fi

    cache_check_result "python311" "fail"
    echo "fail"; return 1
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
        if ! find /usr/lib* -maxdepth 2 -name "$lib*" -print -quit 2>/dev/null | grep -q .; then
            ((missing++))
        fi
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
    if groups "$TARGET_USER" 2>/dev/null | grep -q '\bplugdev\b'; then
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
            local vid
            vid=$(cat "$dev/idVendor" 2>/dev/null || true)
            local pid
            pid=$(cat "$dev/idProduct" 2>/dev/null || true)
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
    local status="warn"
    if command -v systemctl >/dev/null 2>&1; then
        if systemctl is-active --quiet qz.service 2>/dev/null; then
            status="pass"
        elif systemctl is-failed --quiet qz.service 2>/dev/null; then
            status="fail"
        elif [[ -f "$SERVICE_FILE_QZ" ]]; then
            status="pending"
        else
            status="warn"
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
        "bluetooth" "lsusb" "plugdev" "udev_rules"
        "config_file" "qz_service" "ant_dongle"
    )
    
    # Function names corresponding to keys
    local -a func_names=(
        "python311" "venv" "python_packages" "qt5_libs" "qml_modules"
        "bluetooth" "lsusb" "plugdev" "udev_rules"
        "config_file" "qz_service" "ant_dongle"
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
            update_splash_progress "$finished_count" "$total_checks" "Verifying system components ($finished_count/$total_checks)..."
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

# Generate a fixed-width RSSI signal strength bar with color-coded filled portion
# and greyed-out remainder (similar to ANT+ progress bars)
#
# Usage: draw_rssi_bar_fixed RSSI_VALUE [WIDTH]
# Example: draw_rssi_bar_fixed -65 10
# Output: Two space-separated values:
#   1. Colored bar with grey remainder: [████████░░]
#   2. Colored RSSI text: -65 dBm
#
# RSSI Mapping (typical Bluetooth range):
#   -30 to -50 dBm = Excellent (10-8 bars, GREEN)
#   -51 to -60 dBm = Good (7-6 bars, CYAN)
#   -61 to -70 dBm = Fair (5-4 bars, YELLOW)
#   -71 to -80 dBm = Weak (3-2 bars, ORANGE/RED)
#   -81 to -90 dBm = Very Weak (1 bar, RED)
#   Below -90 dBm = Minimal (1 bar, DIM RED)
draw_rssi_bar_fixed() {
    local rssi="$1"
    local width="${2:-10}"  # Default 10 character width
    
    # Validate RSSI is numeric
    if ! [[ "$rssi" =~ ^-?[0-9]+$ ]]; then
        rssi=-999
    fi
    
    # Map RSSI to bar fill count (0 to width)
    # Linear mapping: -30 dBm (excellent) = full, -90 dBm (weak) = 1 bar
    local fill_count
    if (( rssi >= -30 )); then
        fill_count=$width  # Maximum signal
    elif (( rssi <= -90 )); then
        fill_count=1       # Minimum visible signal
    else
        # Linear interpolation between -30 and -90
        # Formula: fill = width - ((rssi + 30) * width / 60)
        fill_count=$(( width - ((rssi + 30) * width / 60) ))
    fi
    
    # Ensure fill_count is within bounds
    [[ $fill_count -lt 0 ]] && fill_count=0
    [[ $fill_count -gt $width ]] && fill_count=$width
    
    # Determine color based on signal quality
    local bar_color rssi_color
    if (( rssi >= -50 )); then
        bar_color="$GREEN"      # Excellent
        rssi_color="$GREEN"
    elif (( rssi >= -60 )); then
        bar_color="$CYAN"       # Good
        rssi_color="$CYAN"
    elif (( rssi >= -70 )); then
        bar_color="$YELLOW"     # Fair
        rssi_color="$YELLOW"
    elif (( rssi >= -80 )); then
        bar_color="$YELLOW"     # Weak (still yellow)
        rssi_color="$YELLOW"
    else
        bar_color="$RED"        # Very weak
        rssi_color="$RED"
    fi
    
    # Build the bar: filled portion + grey remainder
    local filled_portion=""
    local empty_portion=""
    local empty_count=$(( width - fill_count ))
    
    # Use block characters for filled portion
    if [[ $fill_count -gt 0 ]]; then
        filled_portion=$(printf '█%.0s' $(seq 1 "$fill_count"))
    fi
    
    # Use lighter shade for empty portion
    if [[ $empty_count -gt 0 ]]; then
        empty_portion=$(printf '░%.0s' $(seq 1 "$empty_count"))
    fi
    
    # Assemble final bar with brackets
    local bar_display="${bar_color}${filled_portion}${GRAY}${empty_portion}${NC}"
    
    # Format RSSI text with color
    local rssi_display="${rssi_color}${rssi}${NC}"
    
    # Return both values space-separated
    echo "$bar_display" "$rssi_display"
}

perform_bluetooth_scan() {
    # 1. PREP ENVIRONMENT
    local script_dir
    script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
    local py_script="${script_dir}/bt_provider.py"
    local venv_py="${TARGET_HOME}/ant_venv/bin/python3"
    [[ ! -f "$venv_py" ]] && venv_py=$(command -v python3)
    
    # Use RAM-backed FIFO for zero latency
    local bt_fifo="$TEMP_DIR/qz_bt_fifo_$$"
    local bt_marker="BT_SCAN_$$"  # Unique marker for this scan session

    # Validate script exists BEFORE system changes
    if [[ ! -f "$py_script" ]]; then
        draw_error_screen "BLUETOOTH SCAN" "Error: Script not found: $py_script\nPlease ensure the file exists in the script directory." 1
        return 1
    fi

    # Ensure Bluetooth hardware is awake
    sudo rfkill unblock bluetooth >/dev/null 2>&1
    echo "power on" | bluetoothctl >/dev/null 2>&1

    # Ensure UI output FD is correct and clear any prior menu artifacts
    set_ui_output
    clear_info_area

    # Performance caches for Raspberry Pi
    declare -A ANSI_STRIP_CACHE
    declare -A RSSI_BAR_CACHE
    
    # Pre-compute common RSSI bars (-30 to -90 dBm range) with FIXED WIDTH
    for rssi_val in {-30..-90}; do
        read -r cached_bar cached_num < <(draw_rssi_bar_fixed "$rssi_val" 10)
        RSSI_BAR_CACHE["${rssi_val}_bar"]="$cached_bar"
        RSSI_BAR_CACHE["${rssi_val}_num"]="$cached_num"
    done

    # CLEANUP FUNCTION - careful not to kill parent shell
    stop_bt_engine() {
        # Close FD FIRST to release FIFO before any killing
        exec 4<&- 2>/dev/null || true
        
        # Small delay to let file handles close
        sleep 0.1
        
        # Kill ONLY bt_provider.py processes with specific pattern match
        # This avoids killing the parent bash process
        if pgrep -f "python.*bt_provider\.py" >/dev/null 2>&1; then
            sudo pkill -9 -f "python.*bt_provider\.py" 2>/dev/null || true
        fi
        
        # Remove FIFO (safe after closing FD)
        rm -f "$bt_fifo" 2>/dev/null
        
        # Turn off scanning
        bluetoothctl scan off >/dev/null 2>&1 || true
        
        BT_SCAN_PID=""
    }

    # Set trap ONLY for function return (cleanup happens automatically when function exits)
    trap stop_bt_engine RETURN

    # OUTER LOOP: Handles "Refresh" without crashing or recursion
    while true; do
        local devices=() macs=() rssis=()
        local loop_count=0
        local py_status="STARTING"
        local last_raw="NONE"
        local spin_chars=('⠋' '⠙' '⠹' '⠸' '⠼' '⠴' '⠦' '⠧' '⠇' '⠏')

        # Setup FIFO Pipe
        rm -f "$bt_fifo" && mkfifo "$bt_fifo"
        chmod 0666 "$bt_fifo" 2>/dev/null || sudo chmod 0666 "$bt_fifo" 2>/dev/null || true
        
        # Start Python Provider
        local cmd
        printf -v cmd '%q -u %q' "$venv_py" "$py_script"
        
        # Simple background execution (no setsid - caused issues)
        sudo bash -c "$cmd >\"$bt_fifo\" 2>&1 </dev/null" &
        BT_SCAN_PID=$!
        
        # Give Python time to start
        sleep 0.2
        
        # Open FIFO for reading on FD 4
        if ! exec 4<"$bt_fifo" 2>/dev/null; then
            handle_error "bt_scan" "Failed to open FIFO" "ERROR"
            # Cleanup will happen via trap when we return
            return 1
        fi

        # --- PHASE 1: THE RADAR (Rows 12-21) ---
        local saved_name=""
        if [[ -f "${CONFIG_FILE:-}" ]]; then
            saved_name=$(grep -E '^bluetooth_lastdevice_name=' "${CONFIG_FILE}" 2>/dev/null | tail -n1 | cut -d'=' -f2- | tr -d '\r' | xargs || true)
        fi

        enter_ui_mode
        while true; do
            ((loop_count++))
            
            # Verify at least one bt_provider process is running
            if ! pgrep -f "bt_provider.py" >/dev/null 2>&1; then
                py_status="STOPPED"
            fi

            # Batched non-blocking read from FIFO
            local batch
            batch=$(collect_bt_updates_batch 4)
            if [[ -n "$batch" ]]; then
                while IFS= read -r raw_data; do
                    # Handle STATUS messages robustly
                    if [[ "$raw_data" == STATUS\|* ]]; then
                        # shellcheck disable=SC2034
                        local status_code status_msg
                        IFS='|' read -r _ status_code status_msg <<< "${raw_data%%$'\r'}"
                        : "${status_code:-}" >/dev/null 2>&1
                        [[ -n "$status_msg" ]] && py_status="$status_msg"
                        : "${py_status:-}" >/dev/null 2>&1
                        continue
                    fi
                    [[ "$raw_data" == "HEARTBEAT|"* ]] && { py_status="ACTIVE"; continue; }

                    # Split data: MAC|RSSI|LABEL
                    IFS='|' read -r m r l <<< "$(echo "$raw_data" | tr -d '\r')"
                    last_raw="$raw_data"
                    [[ -z "$m" || "$m" == "ERROR" || "$m" == "HEARTBEAT" || "$m" == "STATUS" ]] && continue
                    
                    local ltrim
                    ltrim=$(printf '%s' "$l" | xargs)
                    if [[ -z "$ltrim" || "${ltrim^^}" == "CONNECTING" || "${ltrim^^}" == "STATUS" ]]; then
                        continue
                    fi
                    
                    if [[ "$r" =~ ^-?[0-9]+$ ]]; then
                        if (( r >= 0 )); then
                            continue
                        fi
                    fi
                    
                    # Update existing device or add new discovery
                    local idx=-1
                    for i in "${!macs[@]}"; do [[ "${macs[$i]}" == "$m" ]] && idx=$i && break; done
                    
                    # PERF: Clean device label with caching
                    local clean_label
                    local cache_key="${ltrim:0:100}"
                    
                    if [[ -n "${ANSI_STRIP_CACHE[$cache_key]:-}" ]]; then
                        clean_label="${ANSI_STRIP_CACHE[$cache_key]}"
                    else
                        clean_label=$(printf '%s' "$ltrim" \
                            | sed -E 's/\\\\033\\\[[0-9;]*[mK]//g; s/\\\\e\\\[[0-9;]*[mK]//g; s/\\\\x1B\\\[[0-9;]*[mK]//g; s/\\\\\[[0-9;]*[mK]//g')
                        clean_label=$(strip_ansi_cached "$clean_label")
                        clean_label=$(printf '%s' "$clean_label" | tr -cd '[:print:]' | xargs)
                        ANSI_STRIP_CACHE[$cache_key]="$clean_label"
                    fi
                    
                    # FIX: Array sync - add missing $
                    if [[ $idx -ge 0 ]]; then
                        rssis[$idx]=$r
                        devices[$idx]="$clean_label"
                    else
                        macs+=("$m")
                        rssis+=("$r")
                        devices+=("$clean_label")
                    fi
                done <<< "$batch"
            fi

            # Sort by Signal Strength using pure bash bubble sort
            if [[ ${#macs[@]} -gt 1 ]]; then
                local n=${#rssis[@]}
                for ((i = 0; i < n - 1; i++)); do
                    for ((j = 0; j < n - i - 1; j++)); do
                        local curr_rssi=${rssis[$j]}
                        local next_rssi=${rssis[$((j + 1))]}
                        
                        [[ ! "$curr_rssi" =~ ^-?[0-9]+$ ]] && curr_rssi=-999
                        [[ ! "$next_rssi" =~ ^-?[0-9]+$ ]] && next_rssi=-999
                        
                        if (( curr_rssi < next_rssi )); then
                            local tmp_mac="${macs[$j]}"
                            local tmp_rssi="${rssis[$j]}"
                            local tmp_dev="${devices[$j]}"
                            
                            macs[$j]="${macs[$((j + 1))]}"
                            rssis[$j]="${rssis[$((j + 1))]}"
                            devices[$j]="${devices[$((j + 1))]}"
                            
                            macs[$((j + 1))]="$tmp_mac"
                            rssis[$((j + 1))]="$tmp_rssi"
                            devices[$((j + 1))]="$tmp_dev"
                        fi
                    done
                done
            fi

            # UI Rendering
            local num_devs=${#macs[@]}
            draw_bottom_panel_header "BLUETOOTH: ${num_devs} NAMED DEVICES"
            
            draw_sealed_row 12 ""
            
            # Rows 13-20: Device slots (8 rows)
            local render_buffer=""
            for ((i=0; i<8; i++)); do
                local row=$((13 + i))
                local row_content=""
                if [ "$i" -lt "$num_devs" ]; then
                    local s=${rssis[$i]}
                    local strength_colored rssi_colored
                    
                    # PERF: Use cache for common RSSI values
                    if [[ -n "${RSSI_BAR_CACHE[${s}_bar]:-}" ]]; then
                        strength_colored="${RSSI_BAR_CACHE[${s}_bar]}"
                        rssi_colored="${RSSI_BAR_CACHE[${s}_num]}"
                    else
                        read -r strength_colored rssi_colored < <(draw_rssi_bar_fixed "$s" 10)
                    fi

                    local name="${devices[$i]}"
                    local color="$BOLD_WHITE"
                    if [[ -n "$saved_name" && "$name" == "$saved_name" ]]; then
                        color="$BOLD_CYAN"
                    fi

                    # PERF: Faster truncation
                    local vis_name
                    local raw_len=${#name}
                    if [[ $raw_len -le 38 ]]; then
                        vis_name="$name"
                    else
                        vis_name=$(trunc_vis "$name" 38)
                    fi
                    
                    local name_col
                    name_col=$(pad_display "  ${color}${vis_name}${NC}" "40")

                    local mac_addr="${macs[$i]}"
                    local mac_col
                    mac_col=$(printf '%-17s' "$mac_addr")
                    
                    row_content="${name_col}  ${mac_col}  ${strength_colored} ${rssi_colored}"
                else
                    row_content=""
                fi

                local vis_row
                vis_row=$(get_vis_width "$row_content")
                local pad_needed=$(( INNER_COLS - vis_row ))
                [[ $pad_needed -lt 0 ]] && pad_needed=0
                local padding
                padding=$(printf '%*s' "$pad_needed" "")

                local line_to_print
                line_to_print="${BLUE}║${NC}${row_content}${padding}${BLUE}║${NC}"
                render_buffer+=$(printf "\033[%d;1H%s" "$((row + 1))" "$line_to_print")
            done

            local ui_fd
            ui_fd=$(get_safe_ui_fd)
            ( printf '%s' "$render_buffer" >&"${ui_fd}" ) 2>/dev/null || true

            draw_bottom_border "Scanning... ${spin_chars[$((loop_count % 10))]} | Any key to stop"

            # IMPROVED: Check for key 3 times per iteration
            local key=""
            for check_attempt in 1 2 3; do
                if read -rsn1 -t 0.033 key </dev/tty; then
                    break 2  # Break out of both loops
                fi
            done
            
            # If we caught a key, exit the scanning loop
            if [[ -n "$key" ]]; then
                # Don't call stop_bt_engine - trap will handle cleanup
                break  # Exit the scanning while loop, proceed to menu
            fi
        done

        # --- PHASE 2: THE SELECTION ---
        local menu_labels=()
        for ((i=0;i<num_devs;i++)); do
            menu_labels+=("$(printf '%-30s [%s]' "${devices[$i]}" "${macs[$i]}")")
        done
        menu_labels+=("Refresh" "Back")
        
        exit_ui_mode || true
        set_ui_output
        clear_info_area
        show_scrollable_menu "SELECT DEVICE" menu_labels 0
        local sanitized=$? 
        
        local num_devs=${#macs[@]}
        local refresh_idx=$num_devs
        local back_idx=$((num_devs + 1))

        # FIX: ESC (255) should go back, not refresh
        if [[ "$sanitized" -eq 255 || "$sanitized" -eq "$back_idx" ]]; then
            exit_ui_mode
            return 1
        elif [[ "$sanitized" -eq "$refresh_idx" ]]; then
            exit_ui_mode
            clear_info_area
            continue 
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
            
            generate_config_file "${CONFIG_FILE:-$HOME/.config/qdomyos-zwift/qDomyos-Zwift.conf}"
            draw_bottom_border ""
            sleep 2
            exit_ui_mode
            return 0
        fi
    done
}

install_python311() {
    local force_pyenv="${1:-false}"

    # Check if Python 3.11 is available in system repositories (unless forced to pyenv)
    if [ "$force_pyenv" != true ] && apt-cache show python3.11 >/dev/null 2>&1; then
        if run_with_progress "Installing Python 3.11 via system package" "apt-get install -y python3.11"; then
            return 0
        fi
    fi

    # Fallback to pyenv
    # 1. Install Build Dependencies
    local deps="git curl build-essential libssl-dev zlib1g-dev libbz2-dev libreadline-dev libsqlite3-dev wget llvm libncurses-dev xz-utils tk-dev libffi-dev liblzma-dev"
    run_with_progress "Installing pyenv build dependencies" "apt-get update && apt-get install -y $deps"

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
    local label
    if [ "$force_pyenv" = true ]; then
        label="Installing Python 3.11 via pyenv"
    else
        label="Python 3.11 not available via system. Using pyenv"
    fi
    
    # We call the binary directly using its absolute path.
    local install_cmd="sudo -u \"$TARGET_USER\" bash -c \"
        export PYENV_ROOT='$p_root'
        export PATH='\$PYENV_ROOT/bin:\$PATH'
        $p_bin install 3.11.9 && $p_bin global 3.11.9
    \""
    
    if run_with_progress "$label" "$install_cmd"; then
        IS_PYENV_INSTALLED=true # Ensure the uninstall logic knows this can be removed
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
    if run_with_progress "Creating Python Virtual Environment" "$cmd"; then
        chown -R "$TARGET_USER:" "$venv_path"
        return 0
    fi
    return 1
}

install_python_packages() {
    local pip_path="$TARGET_HOME/ant_venv/bin/pip"
    # Added bleak to the installation list
    local cmd="sudo -u \"$TARGET_USER\" $pip_path install --upgrade pip && sudo -u \"$TARGET_USER\" $pip_path install openant pyusb pybind11 bleak"
    run_with_progress "Installing Python PIPs" "$cmd"
}

install_qt5_libs() {
    # Install Qt5 libraries, QML modules, and system dependencies
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
        python3-pip
    )
    # Add raspi-config and lsof for Raspberry Pi
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
        draw_bottom_border "Arrows: Up/Down | Enter: Select | Esc: No"

        local key=""
        # Read from controlling TTY to avoid stdin redirections interfering
        safe_read_key key
        if [[ $key == $'\x1b' ]]; then
            read -rsn2 -t 0.06 k2 </dev/tty || true
            # Single ESC -> treat as 'No' for prompt menus (consistent UX)
            if [[ -z "${k2:-}" ]]; then
                exit_ui_mode || true
                return 1
            fi
            if [[ -z "$k2" ]]; then read -rsn1 -t 0.02 k3 </dev/tty || true; fi
            local seq="${k2}${k3:-}"
            case "${seq:-}" in
                '[A') ((selected--)) ;; 
                '[B') ((selected++)) ;;
            esac
        elif [[ $key == [lL] ]]; then
            show_legend_popup
            continue
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
    
    print_at $((prompt_row - 3)) "${BLUE}║${RED}╔════════════════════════════════════════════════════════════════════════════╗${NC}"
    print_at $((prompt_row - 2)) "${BLUE}║${RED}║${NC} ${WHITE}WARNING: This may break your desktop if you are not on a true server.${NC}      ${RED}║${NC}"
    print_at $((prompt_row - 1)) "${BLUE}║${RED}║${NC} ${YELLOW}${prompt}${NC}                                                      ${RED}║${NC}"
    print_at $((prompt_row))     "${BLUE}║${RED}╚════════════════════════════════════════════════════════════════════════════╝${NC}"

    move_cursor $((prompt_row - 1)) $((3 + 21))
    
    local input=""
    while true; do
        read -rsn1 key 2>/dev/tty
        if [[ "$key" == $'\x1b' ]]; then
            # ESC -> cancel prompt
            enter_ui_mode
            return 1
        fi
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

# Initialize width calculator early to avoid first-call detection overhead
init_width_calculator



prompt_success_menu() {
    # Footer hint (for automated checks): Esc: Exit
    local warns=${1:-0}
    enter_ui_mode
    local options=("User Profile" "Equipment Selection" "Bluetooth Scan" "ANT+ Test" "QZ Service" # "Uninstall" (placeholder)
                   "Exit")
    local selected=0
    local num_options=${#options[@]}
    
    local title="SYSTEM READY"
    [[ "$warns" -eq 1 ]] && title="READY WITH 1 WARNING"
    [[ "$warns" -gt 1 ]] && title="READY WITH $warns WARNINGS"
    # Initial full render (draw once)
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

    local prev_selected=$selected

    while true; do
        # Input handling first for responsiveness
        local key=""
        safe_read_key key
        if [[ $key == $'\x1b' ]]; then
            read_escape_sequence k2
            # Single ESC -> ignore (only Exit menu item allowed for exit)
            [[ "${k2:-}" == "[A" ]] && ((selected--))
            [[ "${k2:-}" == "[B" ]] && ((selected++))
            flush_input_buffer
        elif [[ $key == [lL] ]]; then
            show_legend_popup
            # Full redraw after popup
            draw_bottom_panel_header "$title"
            clear_info_area
            draw_sealed_row 12 ""
            for i in "${!options[@]}"; do
                local row=$((13 + i))
                if [[ $i -eq $selected ]]; then
                    draw_sealed_row "$row" "   ${CYAN}► ${BOLD_CYAN}${options[$i]}${NC}"
                else
                    draw_sealed_row "$row" "     ${GRAY}${options[$i]}${NC}"
                fi
            done
            draw_bottom_border "Arrows: Up/Down | Enter: Select"
            prev_selected=$selected
            continue
        elif [[ $key == "" ]]; then
            return "$selected"
        fi

        [[ $selected -lt 0 ]] && selected=$((num_options - 1))
        [[ $selected -ge $num_options ]] && selected=0

        # Only update UI if selection changed
        if [[ $selected -ne $prev_selected ]]; then
            # Deselect previous row
            local prev_row=$((13 + prev_selected))
            draw_sealed_row "$prev_row" "     ${GRAY}${options[$prev_selected]}${NC}"

            # Select new row
            local new_row=$((13 + selected))
            draw_sealed_row "$new_row" "   ${CYAN}► ${BOLD_CYAN}${options[$selected]}${NC}"

            prev_selected=$selected
        fi
    done
}

prompt_action_menu() {
    # Footer hint (for automated checks): Esc: Exit
    local fails=$1
    enter_ui_mode
    local options=("Guided Setup" "Exit")
    local selected=0
    local num_options=${#options[@]}
    
    # Initial render
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

    # Helper text explaining the menu options
    draw_sealed_row 17 "   Guided setup detects missing components (Python, packages, permissions, etc.)"
    draw_sealed_row 18 "   and guides you through resolving them step-by-step."

    local prev_selected=$selected

    while true; do
        # Input-first for responsiveness
        local key=""
        safe_read_key key
        if [[ $key == $'\x1b' ]]; then
            read_escape_sequence k2
            # Single ESC -> ignore
            [[ "${k2:-}" == "[A" ]] && ((selected--))
            [[ "${k2:-}" == "[B" ]] && ((selected++))
            flush_input_buffer
        elif [[ $key == [lL] ]]; then
            show_legend_popup
            # Full redraw after popup
            draw_bottom_panel_header "ISSUES DETECTED ($fails)"
            clear_info_area
            draw_sealed_row 12 ""
            for i in "${!options[@]}"; do
                local row=$((13 + i))
                if [[ $i -eq $selected ]]; then
                    draw_sealed_row "$row" "   ${CYAN}► ${BOLD_CYAN}${options[$i]}${NC}"
                else
                    draw_sealed_row "$row" "     ${GRAY}${options[$i]}${NC}"
                fi
            done
            draw_bottom_border "Arrows: Up/Down | Enter: Select"
            prev_selected=$selected
            continue
        elif [[ $key == "" ]]; then
            return "$selected"
        fi

        [[ $selected -lt 0 ]] && selected=$((num_options - 1))
        [[ $selected -ge $num_options ]] && selected=0

        if [[ $selected -ne $prev_selected ]]; then
            # Deselect previous
            local prev_row=$((13 + prev_selected))
            draw_sealed_row "$prev_row" "     ${GRAY}${options[$prev_selected]}${NC}"

            # Select new
            local new_row=$((13 + selected))
            draw_sealed_row "$new_row" "   ${CYAN}► ${BOLD_CYAN}${options[$selected]}${NC}"

            prev_selected=$selected
        fi
    done
}


# ============================================================================
# show_scrollable_menu()
# ============================================================================
show_scrollable_menu() {
    # Footer hint (for automated checks): Esc: Back
    local title="${1:-INFORMATION}"
    local items_name="$2"
    local selected="${3:-0}"
    local back_label="${4:-}"
    local pad_mode="${5:-}"

    # Fast path disabled: using dynamic arrays for all menus to ensure
    # correct per-menu filtering and behavior. Re-enable only if a
    # dedicated per-menu cache mapping is implemented.

    # Copy the named array into a local array safely
    local menu_list=()
    if declare -p "$items_name" >/dev/null 2>&1; then
        if [[ -n "${BASH_VERSINFO:-}" && ${BASH_VERSINFO[0]} -ge 4 ]]; then
            local -n __ref="$items_name"
            menu_list=( "${__ref[@]}" )
            unset -n __ref 2>/dev/null || true
        else
            eval "menu_list=( \"\\\${${items_name}[@]}\" )"
        fi
    else
        menu_list=("$items_name")
    fi

    [[ -n "$back_label" ]] && menu_list+=("$back_label")
    local total_count=${#menu_list[@]}
    # Precompute visual widths for all menu items once to avoid repeated
    # `get_vis_width` calls in the hot render loop. This leverages the
    # DISPLAY_CACHE inside `get_vis_width` and reduces subprocess churn.
    local item_widths=()
    local i itm w
    for i in "${!menu_list[@]}"; do
    itm="${menu_list[i]}"
    w=$(get_vis_width "$itm")
    item_widths[i]="$w"
    done
    
    # Dynamic space calculation
    local total_info_rows=$(( LOG_BOTTOM - LOG_TOP + 1 ))
    local render_start=$LOG_TOP
    local max_display=$total_info_rows

    # If caller requests padding, leave the very top and bottom rows blank
    # (useful for equipment selection where we want breathing room).
    if [[ "$pad_mode" == "pad" ]]; then
        render_start=$(( LOG_TOP + 1 ))
        max_display=$(( total_info_rows - 2 ))
        [[ $max_display -lt 1 ]] && max_display=1
    else
        if [[ $total_count -le 8 ]]; then
            render_start=$(( LOG_TOP + 1 ))
            max_display=$(( total_info_rows - 1 ))
        fi
    fi

    local display_count=$total_count
    [[ $display_count -gt $max_display ]] && display_count=$max_display

    trap 'trap - RETURN SIGINT SIGTERM; move_cursor $((LOG_BOTTOM + 1)) 0; exit_ui_mode' RETURN SIGINT SIGTERM
    enter_ui_mode

    # Initial sliding window calculation
    local start_idx=0
    if [[ $total_count -gt $display_count ]]; then
        start_idx=$(( selected - (display_count / 2) ))
        [[ $start_idx -lt 0 ]] && start_idx=0
        [[ $start_idx -gt $((total_count - display_count)) ]] && start_idx=$((total_count - display_count))
    fi

    # Initial full render
    local render_buffer=""
    for ((r=LOG_TOP; r<=LOG_BOTTOM; r++)); do
        local row_index=$(( r - render_start ))
        local row_content=""
        if (( row_index >= 0 && row_index < display_count )); then
            local current_idx=$(( start_idx + row_index ))
            local item_text="${menu_list[$current_idx]}"
            local vis_w=${item_widths[$current_idx]:-}
            if [[ -z "${vis_w}" ]]; then
                vis_w=$(get_vis_width "$item_text")
            fi
            if [[ $vis_w -gt $((INNER_COLS - 5)) ]]; then
                item_text=$(trunc_vis "$item_text" $((INNER_COLS - 5)))
                vis_w=$((INNER_COLS - 5))
            fi

            if [[ $current_idx -eq $selected ]]; then
                row_content="   ${CYAN}► ${BOLD_CYAN}${item_text}${NC}"
            else
                row_content="     ${GRAY}${item_text}${NC}"
            fi

            local prefix_chars=5
            if [[ ${vis_w:-0} -gt $((INNER_COLS - prefix_chars)) ]]; then
                vis_w=$((INNER_COLS - prefix_chars))
            fi
            local pad_needed=$(( INNER_COLS - prefix_chars - vis_w ))
            [[ $pad_needed -lt 0 ]] && pad_needed=0
            local padding
            padding=$(printf '%*s' "$pad_needed" "")
            row_content+="$padding"
        else
            row_content=$(printf '%*s' "$INNER_COLS" "")
        fi
        render_buffer+=$(printf "\033[%d;1H${BLUE}║${NC}%s${BLUE}║${NC}" "$((r + 1))" "$row_content")
    done

    local b_row=$((LOG_BOTTOM + 1))
    local help_text="Arrows: Up/Down | Enter: Select | Esc: Back"
    render_buffer+=$(build_hr_string "$b_row" "╚" "╝" "$help_text" "${BOLD_BLUE}" "")
    local ui_fd
    ui_fd=$(get_safe_ui_fd)
    ( printf '%s' "$render_buffer" >&"${ui_fd}" ) 2>/dev/null || true

    local prev_selected=$selected
    local prev_start_idx=$start_idx

    while true; do
        local key=""
        safe_read_key key
        if [[ $key == $'\x1b' ]]; then
            read_escape_sequence k2
            # Single ESC -> treat as Back for scrollable menus
            if [[ -z "${k2:-}" ]]; then
                move_cursor $((LOG_BOTTOM + 1)) 0
                exit_ui_mode
                return 255
            fi
            #if [[ -z "$k2" ]]; then read -rsn1 -t 0.02 k3 </dev/tty 2>/dev/null || true; fi
            if [[ -n "$k2" ]]; then safe_read_key k3 0.02; fi
            local seq="${k2}${k3:-}"
            case "${seq:-}" in
                '[A') ((selected--)) ;; 
                '[B') ((selected++)) ;;
            esac
            flush_input_buffer
        elif [[ $key == "" ]]; then
            if [[ -n "$back_label" ]] && [[ $selected -eq $((total_count - 1)) ]]; then
                move_cursor $((LOG_BOTTOM + 1)) 0
                exit_ui_mode
                return 255
            fi
            move_cursor $((LOG_BOTTOM + 1)) 0
            exit_ui_mode
            return "$selected"
        fi

        # Wrap selection
        [[ $selected -lt 0 ]] && selected=$((total_count - 1))
        [[ $selected -ge $total_count ]] && selected=0

        # Recompute window start index
        start_idx=0
        if [[ $total_count -gt $display_count ]]; then
            start_idx=$(( selected - (display_count / 2) ))
            [[ $start_idx -lt 0 ]] && start_idx=0
            [[ $start_idx -gt $((total_count - display_count)) ]] && start_idx=$((total_count - display_count))
        fi

        # If window scrolled, perform full redraw
        if [[ $start_idx -ne $prev_start_idx ]]; then
            local render_buffer=""
            for ((r=LOG_TOP; r<=LOG_BOTTOM; r++)); do
                local row_index=$(( r - render_start ))
                local row_content=""
                if (( row_index >= 0 && row_index < display_count )); then
                    local current_idx=$(( start_idx + row_index ))
                    local item_text="${menu_list[$current_idx]}"
                    local vis_w=${item_widths[$current_idx]:-}
                    if [[ -z "${vis_w}" ]]; then
                        vis_w=$(get_vis_width "$item_text")
                    fi
                    if [[ $vis_w -gt $((INNER_COLS - 5)) ]]; then
                        item_text=$(trunc_vis "$item_text" $((INNER_COLS - 5)))
                        vis_w=$((INNER_COLS - 5))
                    fi
                    if [[ $current_idx -eq $selected ]]; then
                        row_content="   ${CYAN}► ${BOLD_CYAN}${item_text}${NC}"
                    else
                        row_content="     ${GRAY}${item_text}${NC}"
                    fi
                    local prefix_chars=5
                    if [[ ${vis_w:-0} -gt $((INNER_COLS - prefix_chars)) ]]; then
                        vis_w=$((INNER_COLS - prefix_chars))
                    fi
                    local pad_needed=$(( INNER_COLS - prefix_chars - vis_w ))
                    [[ $pad_needed -lt 0 ]] && pad_needed=0
                    local padding
                    padding=$(printf '%*s' "$pad_needed" "")
                    row_content+="$padding"
                else
                    row_content=$(printf '%*s' "$INNER_COLS" "")
                fi
                render_buffer+=$(printf "\033[%d;1H${BLUE}║${NC}%s${BLUE}║${NC}" "$((r + 1))" "$row_content")
            done
            render_buffer+=$(build_hr_string "$b_row" "╚" "╝" "$help_text" "${BOLD_BLUE}" "")
            ui_fd=$(get_safe_ui_fd)
            ( printf '%s' "$render_buffer" >&"${ui_fd}" ) 2>/dev/null || true
            prev_start_idx=$start_idx
            prev_selected=$selected
            continue
        fi

        # If only selection changed and window stayed same, update two rows
        if [[ $selected -ne $prev_selected ]]; then
            ui_fd=$(get_safe_ui_fd)

            # Deselect previous if visible
            local prev_row_index=$(( prev_selected - start_idx ))
            if (( prev_row_index >= 0 && prev_row_index < display_count )); then
                local prev_row=$(( render_start + prev_row_index ))
                local prev_item="${menu_list[$prev_selected]}"
                local prev_vis_w=${item_widths[$prev_selected]:-}
                if [[ -z "${prev_vis_w}" ]]; then prev_vis_w=$(get_vis_width "$prev_item"); fi
                if [[ $prev_vis_w -gt $((INNER_COLS - 5)) ]]; then
                    prev_item=$(trunc_vis "$prev_item" $((INNER_COLS - 5)))
                    prev_vis_w=$((INNER_COLS - 5))
                fi
                local prev_content="     ${GRAY}${prev_item}${NC}"
                local prev_pad_needed=$(( INNER_COLS - 5 - prev_vis_w ))
                [[ $prev_pad_needed -lt 0 ]] && prev_pad_needed=0
                local prev_padding
                prev_padding=$(printf '%*s' "$prev_pad_needed" "")
                ( printf "\033[%d;1H${BLUE}║${NC}%s%s${BLUE}║${NC}" "$((prev_row + 1))" "$prev_content" "$prev_padding" >&"${ui_fd}" ) 2>/dev/null || true
            fi

            # Select new if visible
            local new_row_index=$(( selected - start_idx ))
            if (( new_row_index >= 0 && new_row_index < display_count )); then
                local new_row=$(( render_start + new_row_index ))
                local new_item="${menu_list[$selected]}"
                local new_vis_w=${item_widths[$selected]:-}
                if [[ -z "${new_vis_w}" ]]; then new_vis_w=$(get_vis_width "$new_item"); fi
                if [[ $new_vis_w -gt $((INNER_COLS - 5)) ]]; then
                    new_item=$(trunc_vis "$new_item" $((INNER_COLS - 5)))
                    new_vis_w=$((INNER_COLS - 5))
                fi
                local new_content="   ${CYAN}► ${BOLD_CYAN}${new_item}${NC}"
                local new_pad_needed=$(( INNER_COLS - 5 - new_vis_w ))
                [[ $new_pad_needed -lt 0 ]] && new_pad_needed=0
                local new_padding
                new_padding=$(printf '%*s' "$new_pad_needed" "")
                ( printf "\033[%d;1H${BLUE}║${NC}%s%s${BLUE}║${NC}" "$((new_row + 1))" "$new_content" "$new_padding" >&"${ui_fd}" ) 2>/dev/null || true
            fi

            prev_selected=$selected
        fi
    done
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

show_scrollable_menu_fast() {
    local title="${1:-INFORMATION}"
    local items_name="$2"
    local selected="${3:-0}"
    local back_label="${4:-}"
    local pad_mode="${5:-}"

    if ! load_menu_cache; then
        return 1
    fi

    local menu_list=("${MENU_CACHE_LINES[@]}")
    local menu_widths=("${MENU_CACHE_WIDTHS[@]}")
    [[ -n "$back_label" ]] && menu_list+=("$back_label") && menu_widths+=("${#back_label}")
    local total_count=${#menu_list[@]}

    local total_info_rows=$(( LOG_BOTTOM - LOG_TOP + 1 ))
    local render_start=$LOG_TOP
    local max_display=$total_info_rows
    if [[ "$pad_mode" == "pad" ]]; then
        render_start=$(( LOG_TOP + 1 ))
        max_display=$(( total_info_rows - 2 ))
        [[ $max_display -lt 1 ]] && max_display=1
    else
        if [[ $total_count -le 8 ]]; then
            render_start=$(( LOG_TOP + 1 ))
            max_display=$(( total_info_rows - 1 ))
        fi
    fi

    local display_count=$total_count
    [[ $display_count -gt $max_display ]] && display_count=$max_display

    enter_ui_mode
    trap 'trap - RETURN SIGINT SIGTERM; move_cursor $((LOG_BOTTOM + 1)) 0; exit_ui_mode' RETURN SIGINT SIGTERM

    # Initial window calculation
    local start_idx=0
    if (( total_count > display_count )); then
        start_idx=$(( selected - (display_count / 2) ))
        [[ $start_idx -lt 0 ]] && start_idx=0
        [[ $start_idx -gt $((total_count - display_count)) ]] && start_idx=$((total_count - display_count))
    fi

    # Initial full render
    local render_buffer=""
    for ((r=render_start; r<=LOG_BOTTOM; r++)); do
        local row_index=$(( r - render_start ))
        if (( row_index >= 0 && row_index < display_count )); then
            local current_idx=$(( start_idx + row_index ))
            local item_text="${menu_list[$current_idx]}"
            local item_width=${menu_widths[$current_idx]:-${#item_text}}
            if (( item_width > (INNER_COLS - 5) )); then
                item_text="${item_text:0:$((INNER_COLS - 5))}"
                item_width=$((INNER_COLS - 5))
            fi
            local row_content
            if [[ $current_idx -eq $selected ]]; then
                row_content="   ${CYAN}► ${BOLD_CYAN}${item_text}${NC}"
            else
                row_content="     ${GRAY}${item_text}${NC}"
            fi
            local pad_needed=$((INNER_COLS - item_width - 5))
            [[ $pad_needed -lt 0 ]] && pad_needed=0
            local padding
            padding=$(printf '%*s' "$pad_needed" "")
            render_buffer+=$(printf "\033[%d;1H${BLUE}║${NC}%s%s${BLUE}║${NC}" "$((r + 1))" "$row_content" "$padding")
        else
            local blank
            blank=$(printf '%*s' "$INNER_COLS" "")
            render_buffer+=$(printf "\033[%d;1H${BLUE}║${NC}%s${BLUE}║${NC}" "$((r + 1))" "$blank")
        fi
    done

    local ui_fd
    ui_fd=$(get_safe_ui_fd)
    ( printf '%s' "$render_buffer" >&"${ui_fd}" ) 2>/dev/null || true
    draw_bottom_border "Arrows: Up/Down | Enter: Select | Esc: Back"

    local prev_selected=$selected
    local prev_start_idx=$start_idx

    while true; do
        local key
        safe_read_key key
        if [[ $key == $'\x1b' ]]; then
            read_escape_sequence k2
            if [[ -z "${k2:-}" ]]; then
                # Single ESC -> treat as Back
                exit_ui_mode
                return 255
            fi
            #if [[ -z "$k2" ]]; then read -rsn1 -t 0.02 k3 </dev/tty 2>/dev/null || true; fi
            if [[ -n "$k2" ]]; then safe_read_key k3 0.02; fi
            local seq="${k2}${k3:-}"
            case "${seq:-}" in
                '[A') ((selected--)) ;; 
                '[B') ((selected++)) ;;
            esac
            flush_input_buffer
        elif [[ $key == "" ]]; then
            if [[ -n "$back_label" ]] && [[ $selected -eq $((total_count - 1)) ]]; then
                exit_ui_mode
                return 255
            fi
            exit_ui_mode
            return "$selected"
        fi

        # Wrap selection
        [[ $selected -lt 0 ]] && selected=$((total_count - 1))
        [[ $selected -ge $total_count ]] && selected=0

        # Recompute window start index
        start_idx=0
        if (( total_count > display_count )); then
            start_idx=$(( selected - (display_count / 2) ))
            [[ $start_idx -lt 0 ]] && start_idx=0
            [[ $start_idx -gt $((total_count - display_count)) ]] && start_idx=$((total_count - display_count))
        fi

        # If window scrolled, full redraw
        if [[ $start_idx -ne $prev_start_idx ]]; then
            local render_buffer=""
            for ((r=render_start; r<=LOG_BOTTOM; r++)); do
                local row_index=$(( r - render_start ))
                if (( row_index >= 0 && row_index < display_count )); then
                    local current_idx=$(( start_idx + row_index ))
                    local item_text="${menu_list[$current_idx]}"
                    local item_width=${menu_widths[$current_idx]:-${#item_text}}
                    if (( item_width > (INNER_COLS - 5) )); then
                        item_text="${item_text:0:$((INNER_COLS - 5))}"
                        item_width=$((INNER_COLS - 5))
                    fi
                    local row_content
                    if [[ $current_idx -eq $selected ]]; then
                        row_content="   ${CYAN}► ${BOLD_CYAN}${item_text}${NC}"
                    else
                        row_content="     ${GRAY}${item_text}${NC}"
                    fi
                    local pad_needed=$((INNER_COLS - item_width - 5))
                    [[ $pad_needed -lt 0 ]] && pad_needed=0
                    local padding
                    padding=$(printf '%*s' "$pad_needed" "")
                    render_buffer+=$(printf "\033[%d;1H${BLUE}║${NC}%s%s${BLUE}║${NC}" "$((r + 1))" "$row_content" "$padding")
                else
                    local blank
                    blank=$(printf '%*s' "$INNER_COLS" "")
                    render_buffer+=$(printf "\033[%d;1H${BLUE}║${NC}%s${BLUE}║${NC}" "$((r + 1))" "$blank")
                fi
            done
            ui_fd=$(get_safe_ui_fd)
            ( printf '%s' "$render_buffer" >&"${ui_fd}" ) 2>/dev/null || true
            draw_bottom_border "Arrows: Up/Down | Enter: Select | Esc: Back"
            prev_start_idx=$start_idx
            prev_selected=$selected
            continue
        fi

        # Only selection changed and window same -> update two rows
        if [[ $selected -ne $prev_selected ]]; then
            ui_fd=$(get_safe_ui_fd)

            # Deselect previous if visible
            local prev_row_index=$(( prev_selected - start_idx ))
            if (( prev_row_index >= 0 && prev_row_index < display_count )); then
                local prev_row=$(( render_start + prev_row_index ))
                local prev_item="${menu_list[$prev_selected]}"
                local prev_w=${menu_widths[$prev_selected]:-${#prev_item}}
                if (( prev_w > (INNER_COLS - 5) )); then
                    prev_item="${prev_item:0:$((INNER_COLS - 5))}"
                    prev_w=$((INNER_COLS - 5))
                fi
                local prev_content="     ${GRAY}${prev_item}${NC}"
                local prev_pad_needed=$(( INNER_COLS - prev_w - 5 ))
                [[ $prev_pad_needed -lt 0 ]] && prev_pad_needed=0
                local prev_padding
                prev_padding=$(printf '%*s' "$prev_pad_needed" "")
                ( printf "\033[%d;1H${BLUE}║${NC}%s%s${BLUE}║${NC}" "$((prev_row + 1))" "$prev_content" "$prev_padding" >&"${ui_fd}" ) 2>/dev/null || true
            fi

            # Select new if visible
            local new_row_index=$(( selected - start_idx ))
            if (( new_row_index >= 0 && new_row_index < display_count )); then
                local new_row=$(( render_start + new_row_index ))
                local new_item="${menu_list[$selected]}"
                local new_w=${menu_widths[$selected]:-${#new_item}}
                if (( new_w > (INNER_COLS - 5) )); then
                    new_item="${new_item:0:$((INNER_COLS - 5))}"
                    new_w=$((INNER_COLS - 5))
                fi
                local new_content="   ${CYAN}► ${BOLD_CYAN}${new_item}${NC}"
                local new_pad_needed=$(( INNER_COLS - new_w - 5 ))
                [[ $new_pad_needed -lt 0 ]] && new_pad_needed=0
                local new_padding
                new_padding=$(printf '%*s' "$new_pad_needed" "")
                ( printf "\033[%d;1H${BLUE}║${NC}%s%s${BLUE}║${NC}" "$((new_row + 1))" "$new_content" "$new_padding" >&"${ui_fd}" ) 2>/dev/null || true
            fi

            prev_selected=$selected
        fi
    done
}

run_guided_mode() {
    # 1. Determine Mode automatically (GUI if available, otherwise headless)
    # Legacy interactive selection removed — mode is inferred from environment.
    SETUP_MODE="${SETUP_MODE:-}"
    if [[ -z "$SETUP_MODE" ]]; then
        if [[ "${HAS_GUI:-}" == true ]]; then
            SETUP_MODE="gui"
        else
            SETUP_MODE="headless"
        fi
    fi

    # The user explicitly selected the installation target; treat this as
    # an action so the guided flow will re-probe / continue rather than
    # immediately fall back to the main menu when no other fixes were
    # applied during this run.
    local action_taken=true

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

    # 1. Python 3.11
    if [ "${STATUS_MAP[python311]:-}" = "fail" ]; then
        if apt-cache show python3.11 >/dev/null 2>&1; then
            # Available via system
            request_fix "PYTHON" "Python 3.11 is missing." "Install via system package?"
            local res=$?
            if [ $res -eq 2 ]; then return 1; fi # Cancel to Main Menu
            if [ $res -eq 0 ]; then
                install_python311 && check_python311
                action_taken=true
            else
                # User rejected system package, offer pyenv
                request_fix "PYTHON" "Python 3.11 is missing." "Install via pyenv (can take ~30 mins on a slow device)?"
                local res2=$?
                if [ $res2 -eq 2 ]; then return 1; fi
                if [ $res2 -eq 0 ]; then
                    install_python311 true && check_python311
                    action_taken=true
                fi
            fi
        else
            # Not available via system, offer pyenv
            request_fix "PYTHON" "Python 3.11 is missing and not available in repository." "Install via pyenv (can take ~30 mins on a slow device)?"
            local res=$?
            if [ $res -eq 2 ]; then return 1; fi
            if [ $res -eq 0 ]; then
                install_python311 && check_python311
                action_taken=true
            fi
        fi
    fi

    # 2. Python Virtual Environment
    if [ "${STATUS_MAP[python311]:-}" != "fail" ] && [ "${STATUS_MAP[venv]:-}" = "fail" ]; then
        request_fix "VENV" "Python Virtual Environment is not configured." "Create environment now?"
        local res=$?
        if [ $res -eq 2 ]; then return 1; fi
        if [ $res -eq 0 ]; then
            install_venv && check_venv
            action_taken=true
        fi
    fi

    # 3. Python Packages (openant, pyusb, pybind11, bleak)
    # We track package state as a grouped `pkg_pips` status. If the
    # combined pip package check failed, offer to install all required
    # packages: openant, pyusb, pybind11, and bleak.
    if [ "${STATUS_MAP[python311]:-}" != "fail" ] && [ "${STATUS_MAP[venv]:-}" != "fail" ] && [ "${STATUS_MAP[pkg_pips]:-}" = "fail" ]; then
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

    # 10. Systemd Service
    # If the service is failed, allow the user to inspect the error and optionally attempt a restart.
    if [ "${STATUS_MAP[qz_service]:-}" = "fail" ]; then
        # Ensure we have captured failure details
        capture_service_failure_info "qz_service" >/dev/null 2>&1 || true

        # Try to detect the exit code; if it's 130 (SIGINT) offer the targeted fix
        local exit_code
        exit_code=$(get_service_exit_code 2>/dev/null || true)
        if [[ "$exit_code" == "130" ]]; then
            # Friendly short explanation and targeted fix
            exit_ui_mode || true
            draw_bottom_panel_header "SERVICE: EXIT CODE 130"
            clear_info_area
            draw_sealed_row $((LOG_TOP + 1)) "   qz.service exited with code 130 (SIGINT)."
            draw_sealed_row $((LOG_TOP + 2)) "   This is expected when systemd stops the service; treat as successful exit."
            if service_needs_exit_130_check; then
                draw_sealed_row $((LOG_TOP + 4)) "   Installed unit missing SuccessExitStatus=130. Apply fix now?"
                if prompt_yes_no 6; then
                    # Apply the patch non-interactively (avoid double confirmation)
                    exit_ui_mode || true
                    draw_info_screen "UPDATING" "Patching service unit..." 2
                    apply_exit_130_fix >/dev/null 2>&1 || true
                    draw_info_screen "UPDATED" "Service unit patched; systemd reloaded." 1
                    action_taken=true
                fi
            else
                draw_sealed_row $((LOG_TOP + 4)) "   Installed unit already declares SuccessExitStatus=130. Restart service now?"
                if prompt_yes_no 6; then
                    restart_service_ui
                    action_taken=true
                fi
            fi
            # Refresh checks
            check_qz_service >/dev/null 2>&1 || true
            enter_ui_mode || true
        else
            # Non-130 failures: offer service menu for regeneration or removal
            service_menu_flow
            action_taken=true
            check_qz_service
        fi
    fi

    # If running in headless mode and service not installed, offer to create it
    if [ "$SETUP_MODE" = "headless" ] && [ -z "$ACTIVE_SERVICE_FILE" ]; then
        request_fix "SERVICE" "Auto-start service is not installed." "Create qz systemd service?"
        local res=$?
        if [ $res -eq 2 ]; then return 1; fi
        if [ $res -eq 0 ]; then
            # Force headless semantics for service creation/installation
            local __prev_setup_mode="$SETUP_MODE"
            SETUP_MODE="headless"
            generate_service_file >/dev/null 2>&1 && check_qz_service
            SETUP_MODE="$__prev_setup_mode"
            action_taken=true
        fi
    fi

    # Return 0 if we fixed things (triggers a re-probe), 1 if we just finished/skipped
    [ "$action_taken" = true ] && return 0 || return 1
}

# Uninstall QDomyos-Zwift installation with system safety checks
run_uninstall_mode() {
    draw_bottom_panel_header "UNINSTALL / RESET"
    clear_info_area
    draw_bottom_border
    # 1. Clear interaction area (Borders only)

    # 2. Restrict uninstall to Raspberry Pi with writable /boot
    if ! [ -f /proc/device-tree/model ] || ! grep -q "Raspberry Pi" /proc/device-tree/model; then
        draw_sealed_row $((LOG_TOP + 1)) "   Uninstall not allowed on this system."
        draw_sealed_row $((LOG_TOP + 2)) "   Protected components prevent system modification."
        sleep 4
        return 1
    fi
    if ! touch /boot/firmware/.test_write 2>/dev/null; then
        draw_sealed_row $((LOG_TOP + 1)) "   Uninstall not allowed: /boot is read-only."
        draw_sealed_row $((LOG_TOP + 2)) "   Protected components prevent system modification."
        sleep 4
        return 1
    else
        rm -f /boot/firmware/.test_write
        BOOT_WRITABLE=true
    fi

    # 3. Placeholder for description, built later after checks

    # 4. Render Description placeholder

    local menu_start=3

    if [ "${HAS_GUI:-false}" = true ]; then
        # Use the muted protected glyph color for consistency with the UI
        draw_sealed_row $((LOG_TOP + menu_start - 1)) "   Note: ${BOLD_GRAY}${SYMBOL_LOCKED}${NC} items preserved for system stability."
        ((menu_start++))
    fi

    # --- INTERNAL TASK RUNNER (STOPS ON FAILURE) ---
    run_step() {
        if ! run_with_progress "$1" "$2"; then
            return 1
        fi
        return 0
    }

    # Build dynamic description based on installed components
    local has_venv=false
    if [ "${STATUS_MAP[venv]:-}" = "pass" ]; then has_venv=true; fi
    local has_service=false
    if [ "${STATUS_MAP[qz_service]:-}" = "pass" ]; then has_service=true; fi
    local has_config=false
    if [ "${STATUS_MAP[config_file]:-}" = "pass" ]; then has_config=true; fi
    local has_python=false
    if [ "${IS_PYENV_INSTALLED:-false}" = true ] || [ "${IS_PYSYS_INSTALLED:-false}" = true ]; then has_python=true; fi
    local rem_list="Removes"
    local first=true
    if $has_venv; then rem_list="${rem_list} venv"; first=false; fi
    if $has_service; then 
        if ! $first; then rem_list="${rem_list},"; fi
        rem_list="${rem_list} service"; first=false; 
    fi
    if $has_config; then 
        if ! $first; then rem_list="${rem_list},"; fi
        rem_list="${rem_list} config"; first=false; 
    fi
    if $has_python; then 
        if ! $first; then rem_list="${rem_list},"; fi
        rem_list="${rem_list} and Python 3.11"
    fi
    if $first; then
        rem_list="No core components detected"
    fi

    # Check if anything to uninstall
    local nothing_to_uninstall=true
    if [ "${STATUS_MAP[venv]:-}" = "pass" ] || \
       [ "${STATUS_MAP[qz_service]:-}" = "pass" ] || \
       [ "${STATUS_MAP[config_file]:-}" = "pass" ] || \
       [ "${STATUS_MAP[qt5_libs]:-}" = "pass" ] || \
       [ "${STATUS_MAP[bluetooth]:-}" = "pass" ] || \
       [ "${STATUS_MAP[lsusb]:-}" = "pass" ] || \
       [ "${STATUS_MAP[qml_modules]:-}" = "pass" ] || \
       [ "${IS_PYENV_INSTALLED:-false}" = true ] || \
       [ "${IS_PYSYS_INSTALLED:-false}" = true ]; then
        nothing_to_uninstall=false
    fi
    if $nothing_to_uninstall; then
        draw_bottom_panel_header "UNINSTALL / RESET"
        clear_info_area
        draw_sealed_row $((LOG_TOP + 1)) "   No components detected for removal."
        sleep 2
        return 0
    fi

    # Render Description
    draw_sealed_row $((LOG_TOP + 1)) "   ${rem_list}."

    # 5. EXECUTE CLEANUP STEPS
    
    # A. Remove Python Virtual Environment
    if [ -d "$TARGET_HOME/ant_venv" ]; then
        run_step "Removing Python Virtual Environment" "rm -rf \"$TARGET_HOME/ant_venv\"" || return 1
        update_status "venv" "fail"
    fi

    # B. Remove Python 3.11
    if [ "${IS_PYENV_INSTALLED:-false}" = true ]; then
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
    if [ -f "$SERVICE_FILE_QZ" ]; then
        local svc_cmd="systemctl disable qz.service 2>/dev/null && rm -f \"$SERVICE_FILE_QZ\" && systemctl daemon-reload"
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
        if [ -f /proc/device-tree/model ] && grep -q "Raspberry Pi" /proc/device-tree/model; then
            if [ "$BOOT_WRITABLE" = true ]; then
                if [ "${IS_RUNTIME_INSTALLED:-false}" = true ]; then
                    draw_sealed_row $((LOG_TOP + 2)) "   Remove system packages (Qt5, bluez, usbutils)?"
                else
                    draw_sealed_row $((LOG_TOP + 2)) "   No system packages detected for removal."
                    sleep 2
                    return 0
                fi
            else
                draw_sealed_row $((LOG_TOP + 2)) "   Deep clean skipped due to read-only /boot."
                sleep 4
                return 0
            fi
            if [ "${IS_RUNTIME_INSTALLED:-false}" = true ] && prompt_yes_no 4; then
                if prompt_input_yes; then
                    local pkgs="libqt5core5a libqt5qml5 libqt5quick5 qml-module-qtquick2 bluez usbutils"
                    run_step "Deep Cleaning System Packages" "apt-get remove -y $pkgs && apt-get autoremove -y" || return 1
                    update_status "qt5_libs" "fail"
                    update_status "bluetooth" "fail"
                    update_status "lsusb" "fail"
                fi
            fi

            # Remove system Python 3.11
            if [ "${IS_PYSYS_INSTALLED:-false}" = true ]; then
                draw_sealed_row $((LOG_TOP + 3)) "   Remove system Python 3.11?"
                if prompt_yes_no 5; then
                    if prompt_input_yes; then
                        run_step "Removing system Python 3.11" "apt-get remove -y python3.11 python3.11-venv python3.11-pip && apt-get autoremove -y" || return 1
                        update_status "python311" "fail"
                    fi
                fi
            fi
        else
            draw_sealed_row $((LOG_TOP + 2)) "   Non-Raspberry Pi system detected. Skipping deep clean."
        fi
    fi

    # 7. FINAL SUCCESS FEEDBACK
    draw_bottom_panel_header "UNINSTALL COMPLETE"
    clear_info_area
    
    draw_sealed_row $((LOG_TOP + 2)) "   All selected components removed."
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
    local title="${3:-QDomyos-Zwift  ANT+ BRIDGE SETUP UTILITY}"

    set_ui_output || true
    clear_screen
    
    local term_rows=${TERM_ROWS:-24}
    local inner_w=${INNER_COLS:-80}
    
    # Text color variables
    local BOLD_WHITE=$'\033[1;37m'
    local NC=$'\033[0m'
    local D_Gray=$'\033[38;5;232m'    #  D_Gray for borders
    local PINK=$'\033[38;5;198m'      # Bright pink from logo
    local YELLOW=$'\033[38;5;190m'    # Yellow from Z
    local Gray_DOT=$'\033[38;5;235m'  # Subtle Gray for spacing dots
    
    # Color palette - maps hex characters to 256-color ANSI codes
    local -a palette=(
        $'\033[38;5;232m'  # 0: Darkest Gray (main structure)
        $'\033[38;5;233m'  # 1: Very dark Gray
        $'\033[38;5;234m'  # 2: Dark Gray
        $'\033[38;5;235m'  # 3: Medium-dark Gray
        $'\033[38;5;236m'  # 4: Light-dark Gray
        $'\033[38;5;237m'  # 5: Lighter-dark Gray
        $'\033[38;5;238m'  # 6: Medium Gray
        $'\033[38;5;240m'  # 7: Light Gray
        $'\033[38;5;16m'   # 8: Dark Gray
        $'\033[38;5;52m'   # 9: Darkest pink
        $'\033[38;5;53m'   # A: Very dark pink
        $'\033[38;5;89m'   # B: Dark pink
        $'\033[38;5;125m'  # C: Medium pink
        $'\033[38;5;161m'  # D: Bright pink
        $'\033[38;5;197m'  # E: Brighter pink
        $'\033[38;5;198m'  # F: Brightest pink/magenta
        $'\033[38;5;64m'   # G: Dark olive
        $'\033[38;5;70m'   # H: Olive green
        $'\033[38;5;106m'  # I: Yellow-olive
        $'\033[38;5;112m'  # J: Light lime
        $'\033[38;5;148m'  # K: Yellow-lime
        $'\033[38;5;154m'  # L: Lime-yellow
        $'\033[38;5;190m'  # M: Bright yellow-lime
    )
    
    # Decode hex string to colored dots
    decode_hex_line() {
        local hex_string="$1"
        local output=""
        local i color_idx
        
        for ((i=0; i<${#hex_string}; i++)); do
            local hex_char="${hex_string:$i:1}"
            
            # Convert hex char to decimal index
            case "$hex_char" in
                0) color_idx=0 ;; 1) color_idx=1 ;; 2) color_idx=2 ;; 3) color_idx=3 ;;
                4) color_idx=4 ;; 5) color_idx=5 ;; 6) color_idx=6 ;; 7) color_idx=7 ;;
                8) color_idx=8 ;; 9) color_idx=9 ;; A|a) color_idx=10 ;; B|b) color_idx=11 ;;
                C|c) color_idx=12 ;; D|d) color_idx=13 ;; E|e) color_idx=14 ;; F|f) color_idx=15 ;;
                G|g) color_idx=16 ;; H|h) color_idx=17 ;; I|i) color_idx=18 ;; J|j) color_idx=19 ;;
                K|k) color_idx=20 ;; L|l) color_idx=21 ;; M|m) color_idx=22 ;; *) color_idx=0 ;;
            esac
            
            output+="${palette[$color_idx]}●"
        done
        
        echo "${output}"
    }
    
    # Logo as HEX strings - PIXEL PERFECT!
    # Each character = 1 colored dot (~87% size reduction!)
    local logo_hex=(
        "0000000000000019BBCCCCBB9100000000000000"
        "00000000009BDFFFFFFFFFFFFFFDB90000000000"
        "00000001CFFFFFFFDDCCCCDDFFFFFFFC10000000"
        "000001DFFFFFC91000000000019CFFFFFD100000"
        "0000BFFFFD90880000000000008809DFFFFB0000"
        "000DFFFF9003HIHHIIIIIIIIIIIIG109FFFFD000"
        "00DFFFE1001MMMMMMMMMMMMMMMMMMJ001EFFFD00"
        "09FFFF100003GKMMMMMMMMMMMMMLJ10001FFFF90"
        "0DFFFB00000000000801JMMMMLJ1000000BFFFD0"
        "1FFFF9000000000001KMMMMK10000000009FFFF1"
        "1FFFF90000000001LMMMMJ1000000000009FFFF1"
        "0DFFFB0000001GLMMMMI20800000000000BFFFD0"
        "09FFFF20001HMMMMMMJHHHHG00GG000002FFFF90"
        "00CFFFF100KMMMMMMMMMMMMJ0GMMG1101FFFFC00"
        "000DFFFF901GHMMMMMMMMMHG02GG29FEFFFFD000"
        "0000BFFFFD908880000000000080DFFFFFFB0000"
        "000001CFFFFFD91000000000019CFFFFFFFD9000"
        "00000001BEFFFFFFEDDCCDDEFFFFFFECDFFFFF10"
        "00000000001BDEFFFFFFFFFFFFEDB10009DFFD10"
        "00000000000000019ABBBBA91000000000000000"
        "0000000000000000000000000000000000000000"
        "0000000000000000000000000000000000000000"
        "0000000000000000000000000000000000000000"
        "0000000000000000000000000000000000000000"
    )
    
    local logo_height=${#logo_hex[@]}
    local start_row=0
    local row_counter=$start_row
    
    # Print centered logo with D_Gray side borders
    for hex_line in "${logo_hex[@]}"; do
        local logo_line=$(decode_hex_line "$hex_line")
        local visible_text=$(echo "$logo_line" | sed 's/\x1b\[[0-9;]*m//g')
        local logo_width=${#visible_text}
        local side_padding=$(( (inner_w - logo_width) / 2 + 1))
        
        local left_border=$(printf "${D_Gray}●%.0s${NC}" $(seq 1 $side_padding))
        local right_border=$(printf "${D_Gray}●%.0s${NC}" $(seq 1 $side_padding))
        
        print_at "$row_counter" "${left_border}${logo_line}${NC}${right_border}"
        ((row_counter++))
    done
    
    # Create gradient title with Gray dots filling spaces
    # Build the title character by character, replacing spaces with Gray dots
    local title_with_dots=""
    local char
    local in_qdomyos=true  # Track which section we're in
    
    # "QDomyos-Zwift  ANT+ BRIDGE SETUP UTILITY"
    # First 13 chars are "QDomyos-Zwift" (pink)
    # Then 2 spaces (Gray dots)
    # Remaining chars are "ANT+ BRIDGE SETUP UTILITY" (yellow)
    
    for ((i=0; i<=${#title}; i++)); do
        char="${title:$i:1}"
        
        if [ $i -lt 13 ]; then
            # QDomyos-Zwift section (pink)
            if [ "$char" = " " ]; then
                title_with_dots+="${D_Gray}●${NC}"
            else
                title_with_dots+="${BOLD_WHITE}${PINK}${char}${NC}"
            fi
        elif [ $i -ge 13 ] && [ $i -lt 15 ]; then
            # Two spaces between sections (Gray dots)
            title_with_dots+="${D_Gray}●${NC}"
        else
            # ANT+ BRIDGE SETUP UTILITY section (yellow)
            if [ "$char" = " " ]; then
                title_with_dots+="${D_Gray}●${NC}"
            else
                title_with_dots+="${BOLD_WHITE}${YELLOW}${char}${NC}"
            fi
        fi
    done
    
    # Calculate padding (using original title width for calculation)
    local title_width=${#title}
    local title_pad=$(( (inner_w - title_width) / 2 + 1))
    
    # D_Gray borders on sides of title (consistent with logo)
    local title_left_border=$(printf "${D_Gray}●%.0s${NC}" $(seq 1 $title_pad))
    local title_right_border=$(printf "${D_Gray}●%.0s${NC}" $(seq 1 $title_pad))
    
    print_at "$((row_counter-3))" "${title_left_border}${title_with_dots}${title_right_border}"
    ((row_counter++))
    
    sleep 2
}

# Update the splash screen progress bar
# Usage: update_splash_progress <current> <total> <text>
update_splash_progress() {
    local current=$1
    local total=$2
    local text="${3:-Loading...}"
    
    # Color definitions
    local NC=$'\033[0m'
    local D_Gray=$'\033[38;5;16m'      #  D_Gray for borders
    local CYAN=$'\033[38;5;51m'       # Cyan for progress
    local DARK_Gray=$'\033[38;5;233m' # Dark Gray for empty portion
    local WHITE=$'\033[1;37m'         # White text
    
    # Calculate percentage and bar width
    local percent=$(( (current * 100) / total ))
    local bar_width=$(( INNER_COLS - 4 ))  # Leave room for D_Gray borders (2 dots each side)
    local filled_width=$(( (current * bar_width) / total + 1))
    
    # Create dot-based progress bar
    local bar_str=""
    for ((i=0; i<=filled_width; i++)); do 
        bar_str+="${CYAN}●${NC}"
    done
    for ((i=filled_width; i<=bar_width; i++)); do 
        bar_str+="${DARK_Gray}●${NC}"
    done
    
    # Position: rows 22-23 (after 20-row logo + 1-row title)
    local row=22
    
    # Text with percentage and D_Gray dot borders
    local text_with_percent="${text} ${percent}%"
    local text_width=${#text_with_percent}
    local text_pad=$(( (INNER_COLS - text_width) / 2 + 1 ))
    
    # D_Gray borders on sides of text (consistent with logo)
    local text_left_border=$(printf "${D_Gray}●%.0s${NC}" $(seq 1 $text_pad))
    local text_right_border=$(printf "${D_Gray}●%.0s${NC}" $(seq 1 $text_pad))
    
    # Draw text centered with borders (row 21)
    print_at $((row)) "${text_left_border}${WHITE}${text_with_percent}${NC}${text_right_border}"
    
    ## Draw progress bar with D_Gray borders (row 22)
    #print_at "$row" "${D_Gray}●●${NC}${bar_str}${D_Gray}●●${NC}"
}

# Simple ANT+ test runner: launches `test_ant.py` to emulate a treadmill
# session and broadcast cadence/pace. Uses the same venv/python selection
# logic as other provider runners and provides a minimal UI to stop the test.
perform_ant_test() {
    local script_dir
    script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
    local py_script="${script_dir}/test_ant.py"
    local venv_py="${TARGET_HOME}/ant_venv/bin/python3"
    [[ ! -f "$venv_py" ]] && venv_py=$(command -v python3)

    set_ui_output
    clear_info_area

    # 1. Hardware Check
    if [[ "${STATUS_MAP[ant_dongle]:-}" != "pass" ]]; then
        draw_error_screen "NO ANT+ DEVICE" "Error: No ANT+ device detected via lsusb." 1
        return 1
    fi

    # 2. Stop Service (only if running)
    local svc_status
    svc_status=$(get_service_status)
    if [[ "$svc_status" == "running" ]]; then
        draw_bottom_panel_header "ANT+ INITIALIZATION"
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
            # Check if stopped
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
        log_file="$TEMP_DIR/qz_ant_test_$$.log"
        : > "$log_file"

        # 3. Launch Python with UNBUFFERED output
        # PYTHONUNBUFFERED=1 is critical for the dashboard to see logs in real-time
        local unit_flag=""
        if [[ "${CONFIG_BOOL[miles_unit]:-false}" == "true" ]]; then
            unit_flag="--imperial"
        fi
        local cmd="PYTHONUNBUFFERED=1 \"$venv_py\" -u \"$py_script\" --dashboard $unit_flag --pidfile \"$TEMP_DIR/qz_ant_test.pid\""

        eval "$cmd >\"$log_file\" 2>&1 &"

        # Wait for PID
        py_pid=""
        for _wait_i in {1..30}; do
            if [ -s "$TEMP_DIR/qz_ant_test.pid" ]; then
                py_pid=$(cat "$TEMP_DIR/qz_ant_test.pid" 2>/dev/null || true)
                # Verify PID actually runs
                if ps -p "$py_pid" >/dev/null 2>&1; then break; fi
            fi
            sleep 0.1
        done

        # 4. Wait for Signal
        # We wait up to 25 seconds for the "Signal:Startup" or data flow
        local seen=0
        for (( i=0; i<250; i++ )); do
            if [[ -n "$py_pid" ]] && ! ps -p "$py_pid" >/dev/null 2>&1; then break; fi

            # Check for success signals
            # "Signal:Startup" = Init successful
            # "| Cadence" = Data is already flowing
            if grep -qE "Signal:Startup|\| Cadence" "$log_file"; then
                seen=1
                break
            fi

            # UI Feedback
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
        draw_error_screen "STARTUP FAILED" "Python process failed to initialize.\n\nLog Output:\n$final_err" 1
        return 1
    fi

    # 5. Live Display Loop
    draw_bottom_panel_header "ANT+ BROADCAST TEST"
    clear_info_area

    # Draw instructions immediately to avoid blank panel
    draw_sealed_row $((LOG_TOP + 6)) "   1. WATCH: Open Sensors & Accessories > Add New."
    draw_sealed_row $((LOG_TOP + 7)) "   2. PAIR: Select Foot Pod and follow pairing prompts."
    draw_sealed_row $((LOG_TOP + 8)) "   3. TEST: Start a Treadmill or Run Indoor activity."

    draw_bottom_border "Any key to stop"

    local last_elapsed="-1"
    local last_stage=""
    local last_displayed_line=""  # FIX: Track last rendered line to avoid redundant redraws
    local stale_count=0
    local spinner=('⠋' '⠙' '⠹' '⠸' '⠼' '⠴' '⠦' '⠧' '⠇' '⠏')
    local sc=0

    while true; do
        if ! ps -p "$py_pid" >/dev/null 2>&1; then
            local excerpt=$(tail -n 5 "$log_file" 2>/dev/null | tr -d '\r')
            # Check if the test completed normally by looking for "Test finished." in the log
            if grep -q "Test finished." "$log_file" 2>/dev/null; then
                # Normal completion - show success message
                draw_info_screen "TEST COMPLETED" "ANT+ test finished successfully.\nAll stages completed including 'Stopping'." "wait"
            else
                # Unexpected exit - show error
                draw_error_screen "TEST STOPPED" "Process exited unexpectedly.\nLog:\n$excerpt" 1
            fi
            break
        fi

        # Read the last line only
        local line=$(tail -n 1 "$log_file" 2>/dev/null | tr -d '\r')

        # FIX: Only process and render if the line actually changed
        # This prevents re-rendering the same progress bar 5 times per second
        if [[ "$line" != "$last_displayed_line" ]]; then
            # Regex: "Stage Name | Cadence:100 Speed:10.0 Pace:6:00 [ 10s / 20s ]"
            if [[ "$line" =~ ^([^|]+)\|[[:space:]]*Cadence:([0-9]+)[[:space:]]*Speed:([0-9.]+)[[:space:]]*Pace:([^ ]+)[[:space:]]*\[[[:space:]]*([0-9]+)s[[:space:]]*/[[:space:]]*([0-9]+)s[[:space:]]*\] ]]; then
                local stage="${BASH_REMATCH[1]}"
                local cad="${BASH_REMATCH[2]}"
                local spd="${BASH_REMATCH[3]}"
                local pace="${BASH_REMATCH[4]}"
                local t_now="${BASH_REMATCH[5]}"
                local t_max="${BASH_REMATCH[6]}"

                # Stage change detection
                stage="$(echo "$stage" | xargs)"
                if [[ "$stage" != "$last_stage" ]]; then
                    last_stage="$stage"
                fi

                # Data Freeze Detection
                if [[ "$t_now" == "$last_elapsed" ]]; then 
                    ((stale_count++))
                else 
                    last_elapsed="$t_now"
                    stale_count=0
                fi

                # Render stage info
                draw_sealed_row $((LOG_TOP + 2)) "   ${BOLD_WHITE}$(printf '%-15s' "$stage")${NC}ID:54321   ${CYAN}Pace:${pace}  Cad:${cad}  Spd:${spd}${NC}"

                # Calculate bar width dynamically
                local line_without_bar="   ${CYAN}[ $(printf '%2d' "$t_now")s / $(printf '%2d' "$t_max")s ]${NC}  "
                local line_vis
                line_vis=$(get_display_width "$line_without_bar")
                local bar_w=$(( INNER_COLS - line_vis - 6 ))
                
                # Calculate fill percentage
                local fill=0
                if [[ $t_max -gt 0 ]]; then
                    fill=$(( (t_now * bar_w) / t_max ))
                fi
                [[ $fill -gt $bar_w ]] && fill=$bar_w
                
                local p_bar="${BG_GREEN}$(printf '%*s' "$fill" "")${BG_GRAY}$(printf '%*s' "$((bar_w - fill))" "")${NC}"
                draw_sealed_row $((LOG_TOP + 3)) "${line_without_bar}${p_bar}"
                
                # Remember this line to avoid re-rendering
                last_displayed_line="$line"
            else
                # Show spinner during the time that no live staging is displayed
                # Only update spinner if we haven't displayed this non-matching line yet
                if [[ "$line" != "$last_displayed_line" ]]; then
                    local spin_char="${spinner[$((sc % 10))]}"
                    draw_sealed_row $((LOG_TOP + 2)) "   ${CYAN}${spin_char}${NC}  Waiting for staging data..."
                    ((sc++))
                    last_displayed_line="$line"
                fi
            fi
        fi
        # Note: If line hasn't changed, we skip all rendering (no flicker, no stutter)

        if [[ $stale_count -gt 25 ]]; then
            draw_error_screen "DATA FREEZE" "Hardware connected but data stream stopped." "wait"
            break
        fi

        safe_read_key stop_key 0.2
        if [[ -n "$stop_key" ]]; then
            draw_bottom_panel_header "STOPPING..."
            sudo kill -TERM "$py_pid" 2>/dev/null || true
            break
        fi
    done

    rm -f "$TEMP_DIR/qz_ant_test.pid" 2>/dev/null || true
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
    while IFS= read -r line || [[ -n "$line" ]]; do
        line="${line%%\#*}"
        [[ -z "$line" ]] && continue
        if [[ "$line" =~ ^\[flags\] ]]; then in_flags=1; continue; fi
        if [[ $in_flags -eq 1 && "$line" =~ ^([^=]+)=(.*)$ ]]; then
            key="${BASH_REMATCH[1]}"; val="${BASH_REMATCH[2]}"
            SERVICE_FLAGS[$key]="$val"
        fi
    done < "$SERVICE_CONF_PATH"
    return 0
}

save_service_config() {
    if ! ensure_ram_temp_dir >/dev/null 2>&1; then TEMP_DIR=/tmp; fi
    local tmp="$TEMP_DIR/service.conf.$$"
    {
        echo "[flags]"
        for k in console logging bluetooth_relaxed ant_footpod ant_device ant_verbose profile poll_time heart_service; do
            printf '%s=%s\n' "$k" "${SERVICE_FLAGS[$k]:-}"
        done
    } > "$tmp" || return 1
    mv -f "$tmp" "$SERVICE_CONF_PATH" || return 1
    return 0
}

# Validators
validate_ant_device_id() {
    local id=$1
    [[ "$id" =~ ^[0-9]+$ ]] || return 1
    (( id>=1 && id<=65535 )) || return 1
    return 0
}

validate_poll_time() {
    local t=$1
    [[ "$t" =~ ^[0-9]+$ ]] || return 1
    (( t>=50 && t<=5000 )) || return 1
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
    if [[ "${SERVICE_FLAGS[logging]}" == "true" ]]; then flags+=("-log"); else flags+=("-no-log"); fi
    if [[ "${SERVICE_FLAGS[console]}" == "true" ]]; then :; else flags+=("-no-console"); fi
    [[ "${SERVICE_FLAGS[bluetooth_relaxed]}" == "true" ]] && flags+=("-bluetooth_relaxed")
    if [[ "${SERVICE_FLAGS[ant_footpod]}" == "true" ]]; then
        flags+=("-ant-footpod" "-ant-device" "$(strip_ansi "${SERVICE_FLAGS[ant_device]:-54321}")")
        [[ "${SERVICE_FLAGS[ant_verbose]}" == "true" ]] && flags+=("-ant-verbose")
    fi
    [[ -n "${SERVICE_FLAGS[profile]:-}" ]] && flags+=("-profile" "$(strip_ansi "${SERVICE_FLAGS[profile]}")")
    [[ -n "${SERVICE_FLAGS[poll_time]:-}" ]] && flags+=("-poll-device-time" "$(strip_ansi "${SERVICE_FLAGS[poll_time]}")")
    [[ "${SERVICE_FLAGS[heart_service]}" == "true" ]] && flags+=("-heart-service")
    printf '%s ' "${flags[@]}" | sed -e 's/ $//'
}

generate_service_file() {
    if [ "$SETUP_MODE" != "headless" ]; then
        clear_info_area
        # Show immediate feedback
        local msg="${YELLOW}Generating service file${NC}"
        local msg_plain="Generating service file"
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
    # Prefer using the runtime wrapper (which sets LD_LIBRARY_PATH etc.)
    local script_dir
    script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    local system_wrapper="/usr/local/bin/qdomyos-zwift-wrapper.sh"
    local wrapper="$script_dir/qdomyos-zwift-wrapper.sh"
    
    msg="${YELLOW}Detecting binary path...${NC}"
    msg_plain="Detecting binary path..."
    w=$(get_display_width "$msg_plain")
    draw_sealed_row "$row" "$(printf '%*s' "$((col-2))" '')${msg}"
    
    # Prefer an installed wrapper (packaged to /usr/local) when present.
    if [[ -x "$system_wrapper" ]]; then
        bin="$system_wrapper"
    elif [[ -f "$wrapper" ]]; then
        # Ensure wrapper has a shebang and is executable
        if ! head -n1 "$wrapper" | grep -q '^#!' >/dev/null 2>&1; then
            sed -i '1i#!/bin/bash' "$wrapper" || true
        fi
        chmod +x "$wrapper" || true
        bin="$wrapper"
    else
        bin=$(detect_binary_path 2>/dev/null) || bin="<binary-not-found>"
    fi

    # Ensure bin is an absolute path for systemd
    if [[ "$bin" != /* ]]; then
        bin="$(cd "$script_dir" && pwd)/${bin#./}"
    fi

    local user="${SUDO_USER:-$USER}"
    
    # Update: Building flags
    msg="${YELLOW}Building service flags...${NC}"
    msg_plain="Building service flags..."
    w=$(get_display_width "$msg_plain")
    draw_sealed_row "$row" "$(printf '%*s' "$((col-2))" '')${msg}"
    
    local flags
    flags=$(build_service_flags)
    
    # Update: Writing service file
    msg="${YELLOW}Writing service file...${NC}"
    msg_plain="Writing service file..."
    w=$(get_display_width "$msg_plain")
    draw_sealed_row "$row" "$(printf '%*s' "$((col-2))" '')${msg}"
    
    if ! ensure_ram_temp_dir >/dev/null 2>&1; then TEMP_DIR=/tmp; fi
    local tmp="$TEMP_DIR/qz.service.$$"
    cat > "$tmp" <<EOF
[Unit]
Description=qdomyos-zwift service
After=multi-user.target

[Service]
User=root
Group=plugdev
Environment="QZ_USER=${user}"
WorkingDirectory=$(dirname "$bin")
# Provide a conservative LD_LIBRARY_PATH fallback to assist systems
# where libpython/Qt may live in standard locations.
Environment="LD_LIBRARY_PATH=/usr/local/lib:/usr/lib"
# Use /bin/bash -c to preserve argument parsing and avoid systemd exec quoting pitfalls
ExecStart=/bin/bash -c '${bin} ${flags}'
KillSignal=SIGINT
SuccessExitStatus=130

[Install]
WantedBy=multi-user.target
EOF
    # If running as root, install to system path
    if [[ $(id -u) -eq 0 ]]; then
        # Update: Installing to system
        msg="${YELLOW}Installing to systemd...${NC}"
        msg_plain="Installing to systemd..."
        w=$(get_display_width "$msg_plain")
        draw_sealed_row "$row" "$(printf '%*s' "$((col-2))" '')${msg}"

        mv -f "$tmp" "$svc_file" || return 1
        systemctl daemon-reload || true

        # Success message
        msg="${GREEN}✓ Service file created${NC}"
        msg_plain="✓ Service file created"
        w=$(get_display_width "$msg_plain")
        draw_sealed_row "$row" "$(printf '%*s' "$((col-2))" '')${msg}"
        sleep 0.5
        
        ACTIVE_SERVICE_FILE="$svc_file"
        echo "$svc_file"
        return 0
    fi
    
    # Success message (non-root)
    msg="${GREEN}✓ Service file created${NC}"
    msg_plain="✓ Service file created"
    w=$(get_display_width "$msg_plain")
    draw_sealed_row "$row" "$(printf '%*s' "$((col-2))" '')${msg}"
    sleep 0.5
    
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
    local gen_out tmp
    gen_out=$(generate_service_file 2>&1) || true
    tmp="${ACTIVE_SERVICE_FILE:-$gen_out}"
    # Normalize tmp to the last non-empty line (generate_service_file may emit warnings before printing the path)
    tmp=$(printf '%s' "$tmp" | tr -d '\r' | awk 'NF{line=$0} END{print line}')

    # Attempt installation as root (using sudo if needed).
    # We previously generated a temp service file as non-root (in $tmp).
    # Move that file into the systemd unit path under root and reload.
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

    if [[ $mv_ok -eq 0 ]]; then
        # Try daemon-reload but don't treat failures as install failures
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
    local cfg="${SERVICE_CONF_PATH:-$HOME/.config/qdomyos-zwift/service.conf}"
    if [[ ! -f "$cfg" ]]; then
        draw_error_screen "SERVICE CONFIGURATION" "Configuration file not found:\n\n$cfg" "wait"
        return 1
    fi

    load_service_config >/dev/null 2>&1 || true
    local config_text=""
    config_text+="Logging: ${SERVICE_FLAGS[logging]:-false}\n"
    config_text+="Console: ${SERVICE_FLAGS[console]:-false}\n"
    config_text+="ANT+ Footpod: ${SERVICE_FLAGS[ant_footpod]:-false}\n"
    config_text+="ANT+ Device ID: ${SERVICE_FLAGS[ant_device]:-54321}\n"
    config_text+="Profile: ${SERVICE_FLAGS[profile]:-(default)}\n"
    config_text+="Poll Time (ms): ${SERVICE_FLAGS[poll_time]:-200}\n"

    exit_ui_mode || true
    draw_info_screen "CURRENT SERVICE CONFIGURATION" "$config_text" "wait"
    enter_ui_mode || true
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

show_service_status_ui() {
    exit_ui_mode || true
    if systemctl status qz.service --no-pager; then
        :
    fi
    enter_ui_mode || true
}

## Service status helpers (STEP 1)
# Returns: not-installed | stopped | running | failed
get_service_status() {
    if [[ -f "$SERVICE_FILE_QZ" ]]; then
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

# Build a context-sensitive list of menu options (one per line)
# Ensures the returned list will fit within the dashboard info area.
build_service_menu_options() {
    local status
    status=$(get_service_status)
    local opts=()

    opts+=("Configure Service Flags")
    opts+=("View Current Configuration")

    case "$status" in
        not-installed)
            opts+=("Generate & Install Service")
            ;;
        stopped)
            opts+=("Start Service")
            opts+=("Regenerate Service File")
            opts+=("Remove Service")
            ;;
        running)
            opts+=("Restart Service")
            opts+=("Stop Service")
            #opts+=("View Service Logs")
            opts+=("Regenerate Service File")
            ;;
        failed)
            #opts+=("View Service Error")
            #opts+=("View Service Logs")
            opts+=("Restart Service")
            opts+=("Regenerate Service File")
            # If the installed unit is missing SuccessExitStatus=130 offer an Update action
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

    # Do not add a dedicated "Back to Main Menu" option; plain ESC exits.

    # Safety: ensure we never return more than 9 options (fits LOG_TOP..LOG_BOTTOM)
    local max=9
    if (( ${#opts[@]} > max )); then
        opts=("${opts[@]:0:max}")
    fi

        printf '%s\n' "${opts[@]}"
    }

    # Map a displayed choice to the corresponding action command
    get_action_for_choice() {
        local choice="$1"
        case "$choice" in
            "View Service Error") printf '%s' "view_service_error_ui" ;; 
            "Configure Service Flags") printf '%s' "configure_service_flags_ui" ;; 
            "View Current Configuration") printf '%s' "view_service_config_ui" ;; 
            "Generate & Install Service") printf '%s' "install_service_ui" ;; 
            "Start Service") printf '%s' "start_service_ui" ;; 
            "Stop Service") printf '%s' "stop_service_ui" ;; 
            "Restart Service") printf '%s' "restart_service_ui" ;; 
            "View Service Logs") printf '%s' "view_service_logs_ui 200" ;; 
            "Regenerate Service File") printf '%s' "regenerate_service_file_ui" ;; 
            "Update Service Configuration") printf '%s' "apply_exit_130_fix_ui" ;; 
            "Enable Auto-Start") printf '%s' "enable_service_ui" ;; 
            "Disable Auto-Start") printf '%s' "disable_service_ui" ;; 
            "Remove Service") printf '%s' "remove_service_ui" ;; 
            # Back handled via ESC; no explicit menu entry
            *) printf '%s' "" ;; 
        esac
    }

service_menu_flow() {

    enter_ui_mode || true
    if [[ ! -f "./qdomyos-zwift-bin" ]]; then
        draw_error_screen "MISSING BINARY" "Cannot configure service: qdomyos-zwift-bin not found in current direcory\n$PWD/qdomyos-zwift-bin\nPlease ensure the binary is in this location." "wait"
        exit_ui_mode || true
        return 1
    fi

    # Track when we need to refresh service status
    local need_status_refresh=true
    
    while true; do
        # Only check service status when needed (not every loop iteration)
        if [[ $need_status_refresh == true ]]; then
            # Show working indicator while checking
            STATUS_MAP["qz_service"]="pending"
            draw_status_panel  # Update the right panel to show ⟳
            
            # Now perform the actual check (may take 4 seconds)
            check_qz_service >/dev/null 2>&1 || true
            
            # Redraw status panel with actual result
            draw_status_panel
            
            need_status_refresh=false
        fi
        
        local svc_map_status
        svc_map_status=${STATUS_MAP["qz_service"]:-}
        local status_display
        if [[ -n "$svc_map_status" ]]; then
            case "$svc_map_status" in
                pass) status_display="ACTIVE" ;;
                fail) status_display="FAILED" ;;
                warn|pending) 
                    # Distinguish between not-installed and stopped where possible
                    local rt
                    rt=$(get_service_status)
                    if [[ "$rt" == "not-installed" ]]; then
                        status_display="NOT CONFIGURED"
                    else
                        status_display="STOPPED"
                    fi
                    ;;
                *) status_display="UNKNOWN" ;;
            esac
        else
            local status
            status=$(get_service_status)
            case "$status" in
                not-installed) status_display="NOT CONFIGURED" ;;
                stopped) status_display="STOPPED" ;;
                running) status_display="ACTIVE" ;;
                failed) status_display="FAILED" ;;
                *) status_display="UNKNOWN" ;;
            esac
        fi

        # Build options array
        mapfile -t options < <(build_service_menu_options)
        local num_options=${#options[@]}
        local selected=0

        # Safety cap (should be enforced by builder)
        if (( num_options > 9 )); then
            num_options=9
            options=("${options[@]:0:8}" "Back to Main Menu")
        fi

        # Initial render
        draw_bottom_panel_header "SERVICE CONFIGURATION"
        clear_info_area
        draw_sealed_row $((LOG_TOP)) ""

        for i in "${!options[@]}"; do
            local row=$((LOG_TOP + 1 + i))
            if (( i == selected )); then
                draw_sealed_row "$row" "   ${CYAN}► ${BOLD_CYAN}${options[i]}${NC}"
            else
                draw_sealed_row "$row" "     ${GRAY}${options[i]}${NC}"
            fi
        done

        draw_bottom_border "Arrows: Up/Down | Enter: Select | Esc: Back"

        local prev_selected=$selected

        # Event loop for navigation (follow existing menu conventions)
        while true; do
            local key=""
            safe_read_key key
            if [[ $key == $'\x1b' ]]; then
                read_escape_sequence k2
                # Plain ESC -> exit menu (Back)
                if [[ -z "${k2:-}" ]]; then
                    exit_ui_mode || true
                    return 0
                fi
                [[ "${k2:-}" == "[A" ]] && ((selected--))
                [[ "${k2:-}" == "[B" ]] && ((selected++))
                flush_input_buffer
            elif [[ $key == [lL] ]]; then
                show_legend_popup
                # Full redraw after popup
                draw_bottom_panel_header "SERVICE CONFIGURATION"
                clear_info_area
                draw_sealed_row $((LOG_TOP)) ""
                for i in "${!options[@]}"; do
                    local row=$((LOG_TOP + 1 + i))
                    if (( i == selected )); then
                        draw_sealed_row "$row" "   ${CYAN}► ${BOLD_CYAN}${options[i]}${NC}"
                    else
                        draw_sealed_row "$row" "     ${GRAY}${options[i]}${NC}"
                    fi
                done
                draw_bottom_border "Arrows: Up/Down | Enter: Select | Esc: Back"
                prev_selected=$selected
                continue
            elif [[ $key == "" ]]; then
                # Enter pressed
                break
            fi

            # wrap
            if (( selected < 0 )); then selected=$((num_options - 1)); fi
            if (( selected >= num_options )); then selected=0; fi

            if (( selected != prev_selected )); then
                # redraw previous and new rows
                local prev_row=$((LOG_TOP + 1 + prev_selected))
                draw_sealed_row "$prev_row" "     ${GRAY}${options[prev_selected]}${NC}"
                local new_row=$((LOG_TOP + 1 + selected))
                draw_sealed_row "$new_row" "   ${CYAN}► ${BOLD_CYAN}${options[selected]}${NC}"
                prev_selected=$selected
            fi

        done

        # Execute selection
        exit_ui_mode || true
        local choice="${options[selected]}"

        # Resolve action for the selected choice and execute it
        local action
        action=$(get_action_for_choice "$choice")
        if [[ "$action" == "__BACK__" ]]; then
            return 0
        fi
        if [[ -z "$action" ]]; then
            draw_error_screen "UNKNOWN ACTION" "No handler for: ${choice}" "wait"
            # No status change, don't refresh
        else
            # action may contain an argument (e.g. 'view_service_logs_ui 200')
            eval "$action"
            
            # Only refresh status for actions that actually change service state
            # Configuration changes, flag edits, and log views don't affect service status
            case "$action" in
                *install_service*|*uninstall_service*|*start_service*|*stop_service*|*restart_service*|*enable_service*|*disable_service*)
                    # These actions change service state - need refresh
                    need_status_refresh=true
                    ;;
                *)
                    # Configuration, flags, logs - no service state change
                    need_status_refresh=false
                    ;;
            esac
        fi

        # Re-enter UI mode and continue loop (no error panel for normal actions)
        enter_ui_mode || true
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
            # --- (Fresh Install) ---
            local choice
            prompt_action_menu "$fails"
            choice=$?
            case $choice in
                0) # Guided Setup
                    # Check /boot writability on RPi before attempting install
                    if [ -f /proc/device-tree/model ] && grep -q "Raspberry Pi" /proc/device-tree/model; then
                        if ! touch /boot/firmware/.test_write 2>/dev/null; then
                            draw_sealed_row $((LOG_TOP + 1)) "Boot partition is read-only."
                            draw_sealed_row $((LOG_TOP + 2)) "Run 'sudo raspi-config' to make /boot writable before installing."
                        else
                            rm -f /boot/firmware/.test_write
                            if run_guided_mode; then
                                draw_verifying_screen "Verifying system state..."
                                run_all_checks
                            fi
                        fi
                    else
                        if run_guided_mode; then
                            draw_verifying_screen "Verifying system state..."
                            run_all_checks
                        fi
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
                3) perform_ant_test; check_config_file ;;
                4) service_menu_flow ;;
                5) finish_and_exit 0 ;;
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
        --scan-now     Start the Bluetooth scan page immediately and exit
        --uninstall    Start the uninstall menu immediately and exit

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
safe_reset_dongle() {
    # Run a small Python snippet that finds known ANT vendor/product IDs
    # and calls .reset() on the device if present. This requires pyusb
    # and typically needs root privileges.
    python3 - <<'PY' || return 1
import sys
try:
    import usb.core
    import usb.util
except Exception as e:
    print('safe_reset_dongle: pyusb not available:', e, file=sys.stderr)
    sys.exit(1)
SUPPORTED = [
    (0x0fcf, 0x1009), (0x0fcf, 0x1008), (0x0fcf, 0x100c),
    (0x0fcf, 0x100e), (0x0fcf, 0x88a4), (0x0fcf, 0x1004), (0x11fd, 0x0001)
]
def find_device():
    for v,p in SUPPORTED:
        d = usb.core.find(idVendor=v, idProduct=p)
        if d is not None:
            return d
    return None

d = find_device()
if d is None:
    print('safe_reset_dongle: no ANT+ dongle found', file=sys.stderr)
    sys.exit(0)
try:
    print('safe_reset_dongle: resetting', file=sys.stderr)
    d.reset()
    print('safe_reset_dongle: reset complete', file=sys.stderr)
except usb.core.USBError as e:
    print('safe_reset_dongle: USBError:', e, file=sys.stderr)
    sys.exit(1)
except Exception as e:
    print('safe_reset_dongle: unexpected error:', e, file=sys.stderr)
    sys.exit(1)
PY
}

# Unified shutdown called from traps: stop test sessions, attempt reset,
# restore terminal and exit with the provided code.
safe_shutdown() {
    local code=${1:-0}
    # stop background test processes first
    stop_test_ant_sessions
    # attempt a safe dongle reset (best-effort)
    safe_reset_dongle >/dev/stderr 2>&1 || true
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
            --scan-now)
                SCAN_NOW=1
                ;;
            --uninstall)
                UNINSTALL_MODE=1
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

# 2. Startup Sequence
if [[ "${QZ_NO_SPLASH:-0}" -eq 0 ]]; then
    # Draw Logo
    draw_splash_screen "QDomyos-Zwift" "ANT+ BRIDGE SETUP UTILITY"
    
    # Run Checks in Splash Mode (Updates progress bar, populates STATUS_MAP)
    run_all_checks "splash"
    
    # Brief pause to show 100%
    update_splash_progress 100 100 "Initialization Complete"
    sleep 2
fi

# Check for uninstall mode
if [ "${UNINSTALL_MODE:-0}" -eq 1 ]; then
    clear_screen
    draw_top_panel
    run_uninstall_mode
    exit 0
fi

# 3. Render the Dashboard
# Now we draw the full dashboard. Since run_all_checks populated STATUS_MAP,
# this function will render with the correct icons immediately.
clear_screen
draw_top_panel
draw_bottom_panel_header "INFORMATION"
draw_instructions_bottom "$CURRENT_INSTRUCTION"
draw_bottom_border

# 4. Handle CLI Scan-Now
if [ "${SCAN_NOW:-0}" -eq 1 ]; then
    perform_bluetooth_scan
    finish_and_exit
fi

# 5. Enter the main menu loop
check_final_status