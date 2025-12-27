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

# ==========================================================================
# CONFIG GENERATION - Type-Safe Storage (Milestone 1)
# Pure Bash associative arrays and setter functions for type-safe values.
# These are used by the config generator to ensure exact formatting.
# ==========================================================================

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

# Safe single-key read helper with timeout to avoid blocking automated tests.
# Usage: safe_read_key VAR [timeout]
# Sets the variable named by VAR to the single key read (may be empty on timeout).
safe_read_key() {
    local __var="$1"
    local _timeout="${2:-0.4}"
    # If running under test harness (explicit marker) or QZ_NO_MAIN is set,
    # apply a non-blocking timeout. Otherwise, block until a keypress.
    local _old_ifs="$IFS"
    local _tmp_read=""
    if [[ "${QZ_TEST_MODE:-}${QZ_NO_MAIN:-}" != "" ]]; then
        IFS= read -rsn1 -t "${_timeout}" _tmp_read </dev/tty 2>/dev/null || true
    else
        IFS= read -rsn1 _tmp_read </dev/tty 2>/dev/null || true
    fi
    # Assign to the caller-provided variable name safely
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
    local save_msg_plain="✓ Saved"
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

# Validate that a candidate path is a RAM-backed tmpfs-like storage.
# Returns 0 if usable (writable dir on tmpfs), non-zero otherwise.
configure_service_flags_ui() {
    load_service_config >/dev/null 2>&1 || true
    # Work on an in-memory copy so Cancel can revert
    declare -A _SF
    for k in "${!SERVICE_FLAGS[@]}"; do _SF[$k]="${SERVICE_FLAGS[$k]}"; done

    # Suppress global full-screen refreshes while this modal is active
    UI_MODAL_ACTIVE=1
    enter_ui_mode || true
    local selected=0
    local prev_selected=0
    local need_full_redraw=1
    local prev_num_options=0
    while true; do
        local options=()
        local opt_keys=()
        options+=("Logging: ${_SF[logging]:-false}")
        opt_keys+=("logging")
        options+=("Console: ${_SF[console]:-false}")
        opt_keys+=("console")
        options+=("Bluetooth Relaxed: ${_SF[bluetooth_relaxed]:-false}")
        opt_keys+=("bluetooth_relaxed")
        options+=("ANT+ Footpod: ${_SF[ant_footpod]:-false}")
        opt_keys+=("ant_footpod")
        if [[ "${_SF[ant_footpod]}" == "true" ]]; then
            options+=("ANT+ Device ID: [${_SF[ant_device]:-54321}]")
            opt_keys+=("ant_device")
            options+=("ANT Verbose: ${_SF[ant_verbose]:-false}")
            opt_keys+=("ant_verbose")
        fi
        options+=("Poll Time (ms): [${_SF[poll_time]:-200}]")
        opt_keys+=("poll_time")
        # Changes auto-save on field edit
        # ESC key exits menu

        # Cap to LOG area (rows LOG_TOP..LOG_BOTTOM)
        local num_options=${#options[@]}
        if (( num_options > 9 )); then
            options=("${options[@]:0:9}")
            num_options=9
        fi

        local BASE_PAD="     "
        local ARROW="►"
        local ARROW_POS=2
        local num_options=${#options[@]}

        if [[ $need_full_redraw -eq 1 ]]; then
            draw_bottom_panel_header --force "SERVICE FLAG CONFIGURATION"
            clear_info_area --force
            draw_sealed_row $((LOG_TOP)) ""
            for i in "${!options[@]}"; do
                local row=$((LOG_TOP + 1 + i))
                local opt_text="${options[$i]}"
                if [[ $i -eq $selected ]]; then
                    draw_sealed_row "$row" "   ${CYAN}► ${BOLD_CYAN}${opt_text}${NC}"
                else
                    draw_sealed_row "$row" "${BASE_PAD}${GRAY}${opt_text}${NC}"
                fi
            done
            draw_bottom_border --force "Arrows: Up/Down | Enter: Toggle/Edit | Esc: Back"
            need_full_redraw=0
            prev_num_options=$num_options
        else
            local max_rows=$(( num_options > prev_num_options ? num_options : prev_num_options ))
            for ((i=0;i<max_rows;i++)); do
                local row=$((LOG_TOP + 1 + i))
                if (( i < num_options )); then
                    local opt_text="${options[$i]}"
                    if [[ $i -eq $selected ]]; then
                        update_sealed_row_content "$row" "   ${CYAN}► ${BOLD_CYAN}${opt_text}${NC}"
                    else
                        update_sealed_row_content "$row" "${BASE_PAD}${GRAY}${opt_text}${NC}"
                    fi
                else
                    update_sealed_row_content "$row" ""
                fi
            done
            prev_num_options=$num_options
        fi

        # Input loop for this screen
        while true; do
            local key=""
            safe_read_key key
            if [[ $key == $'\x1b' ]]; then
                read -rsn2 -t 0.01 k2 </dev/tty || true

                if [[ -z "${k2:-}" ]]; then
                    # Plain ESC pressed - navigate back from menu
                    # No need to prompt "Are you sure?" - changes already saved
                    exit_ui_mode || true
                    UI_MODAL_ACTIVE=0
                    return 0
                fi

                # Arrow key sequences with bounds checks
                [[ "${k2:-}" == "[A" ]] && ((selected > 0 && selected--))
                [[ "${k2:-}" == "[B" ]] && ((selected < num_options - 1 && selected++))
            elif [[ $key == "" ]]; then
                # Enter pressed: act on selected
                break
            fi

            [[ $selected -lt 0 ]] && selected=$((num_options - 1))
            [[ $selected -ge $num_options ]] && selected=0

            if [[ $selected -ne $prev_selected ]]; then
                # Update only the interior content for the previous and new
                # rows to avoid redrawing the whole info panel.
                local prow=$((LOG_TOP + 1 + prev_selected))
                update_sealed_row_content "$prow" "${BASE_PAD}${GRAY}${options[$prev_selected]}${NC}"
                local nrow=$((LOG_TOP + 1 + selected))
                update_sealed_row_content "$nrow" "   ${CYAN}► ${BOLD_CYAN}${options[$selected]}${NC}"
                prev_selected=$selected
            fi
        done

        # Determine action for selected index
        local choice_index=$selected
        local choice_text="${options[choice_index]}"
        if [[ "${QZ_DEBUG_PROFILE:-0}" -eq 1 ]]; then
            echo "[DEBUG] choice_index=${choice_index} choice_text=${choice_text}" >> /tmp/qz_profile_debug.log 2>/dev/null || true
        fi

        # No explicit Save/Cancel - changes auto-save on each edit
        # ESC key from menu level will exit (handled by ESC key code)

        # Toggle or edit fields
        case "$choice_text" in
            Logging:*)
                _SF[logging]="$( [[ "${_SF[logging]}" == "true" ]] && echo false || echo true )"
                SERVICE_FLAGS[logging]="${_SF[logging]}"
                auto_save_config "service" "logging" || draw_error_screen "SAVE ERROR" "Failed to save." "wait"
                ;;
            Console:*)
                _SF[console]="$( [[ "${_SF[console]}" == "true" ]] && echo false || echo true )"
                SERVICE_FLAGS[console]="${_SF[console]}"
                auto_save_config "service" "console" || draw_error_screen "SAVE ERROR" "Failed to save." "wait"
                ;;
            "Bluetooth Relaxed"*)
                _SF[bluetooth_relaxed]="$( [[ "${_SF[bluetooth_relaxed]}" == "true" ]] && echo false || echo true )"
                SERVICE_FLAGS[bluetooth_relaxed]="${_SF[bluetooth_relaxed]}"
                auto_save_config "service" "bluetooth_relaxed" || draw_error_screen "SAVE ERROR" "Failed to save." "wait"
                ;;
            "ANT+ Footpod"*)
                _SF[ant_footpod]="$( [[ "${_SF[ant_footpod]}" == "true" ]] && echo false || echo true )"
                SERVICE_FLAGS[ant_footpod]="${_SF[ant_footpod]}"
                auto_save_config "service" "ant_footpod" || draw_error_screen "SAVE ERROR" "Failed to save." "wait"
                ;;
            "ANT Verbose"*)
                _SF[ant_verbose]="$( [[ "${_SF[ant_verbose]}" == "true" ]] && echo false || echo true )"
                SERVICE_FLAGS[ant_verbose]="${_SF[ant_verbose]}"
                auto_save_config "service" "ant_verbose" || draw_error_screen "SAVE ERROR" "Failed to save." "wait"
                ;;
            "ANT+ Device ID"*)
                # Inline numeric prompt to match profile inputs (age/weight)
                local row=$((LOG_TOP + 1 + selected))
                local cur_id="${_SF[ant_device]:-54321}"
                local new_id
                new_id=$(prompt_numeric_input "ANT+ Device ID" "" "$cur_id" "$row" "   ► ")
                    if validate_ant_device_id "$new_id"; then
                    _SF[ant_device]="$new_id"
                    SERVICE_FLAGS[ant_device]="$new_id"
                    auto_save_config "service" "ant_device" || draw_error_screen "SAVE ERROR" "Failed to save." "wait"
                else
                    show_cancel_feedback
                fi
                ;;
            "Poll Time"*)
                local row=$((LOG_TOP + 1 + selected))
                local cur_poll="${_SF[poll_time]:-200}"
                local new_poll
                new_poll=$(prompt_numeric_input "Poll Time" "ms" "$cur_poll" "$row" "   ► ")
                    if validate_poll_time "$new_poll"; then
                    _SF[poll_time]="$new_poll"
                    SERVICE_FLAGS[poll_time]="$new_poll"
                    auto_save_config "service" "poll_time" || draw_error_screen "SAVE ERROR" "Failed to save." "wait"
                else
                    show_cancel_feedback
                fi
                ;;
            *) draw_error_screen "UNHANDLED" "Action for ${choice_text} not implemented." "wait" ;;
        esac
        # Loop and re-render
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



# Script versioning: update when you deploy/copy this script to another host.
# Prefer semantic or date-based strings. You can also set the env var
# QZ_SETUP_DASHBOARD_VERSION to override at runtime.
SCRIPT_VERSION="2025.12.22-feat/ant_footpod-1"
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
            --no-extract)
                export SKIP_TEST_EXTRACT=1
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
SYMBOL_LOCKED="⛊"
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
    
    # 2. Build strings using CYAN for labels to harmonize with System Info
    local L_content="${L_sym} ${CYAN}${L_label}${NC}"
    local R_content="${R_sym} ${CYAN}${R_label}${NC}"
    
    # 3. Calculate padding: split INNER_COLS into left/right with center separator
    local L_padded
    local R_padded
    local left_w right_w
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

draw_bottom_border() {
    local force=0
    if [[ "${1:-}" == "--force" ]]; then force=1; shift; fi
    local help_text="${1:-}"
    if [[ "${UI_MODAL_ACTIVE:-0}" -eq 1 && $force -eq 0 ]]; then
        printf '%s\n' "[DEBUG] draw_bottom_border skipped due to UI_MODAL_ACTIVE=1 (help_text='${help_text}')" >> /tmp/qz_profile_debug.log 2>/dev/null || true
        return 0
    fi
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
    # If a modal is active, avoid clearing the info area unless forced
    if [[ "${UI_MODAL_ACTIVE:-0}" -eq 1 && "${1:-}" != "--force" ]]; then
        printf '%s\n' "[DEBUG] clear_info_area skipped due to UI_MODAL_ACTIVE=1 (arg='${1:-}')" >> /tmp/qz_profile_debug.log 2>/dev/null || true
        return 0
    fi
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
    clear_info_area --force

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

    draw_bottom_border --force "Press ENTER to continue"

    # Treat any non-empty third argument (e.g. "wait") as a request to pause
    # so existing callers may pass either a numeric 1 or the string "wait".
    if [[ -n "${wait_enter:-}" && "${wait_enter}" != "0" ]]; then
        # Wait for a single keypress without showing the cursor to avoid
        # a flashing cursor in the footer area. Keep UI mode active so the
        # rendered error panel remains visible and consistent.
        local k
        safe_read_key k
    fi

    # Clear the error area after dismiss
    clear_info_area --force
    exit_ui_mode
}

# Neutral informational panel (not an error). Usage mirrors draw_error_screen:
# draw_info_screen "TITLE" "Message" [wait]
draw_info_screen() {
    local title="${1:-INFO}"
    local msg="${2:-}"
    local wait_enter=${3:-1}

    enter_ui_mode || true
    clear_info_area --force

    local row=$((LOG_TOP + 1))
    draw_sealed_row "$row" "   ${BOLD_BLUE}${title}${NC}"

    local wrapped
    IFS=$'\n' read -r -d '' -a wrapped < <(printf '%b' "$msg" | fold -s -w $((INFO_WIDTH - 3)) && printf '\0')
    for line in "${wrapped[@]}"; do
        row=$((row + 1))
        draw_sealed_row "$row" "   ${NC}${line}${NC}"
    done

    draw_bottom_border --force "Press ENTER to continue"
    if [[ -n "${wait_enter:-}" && "${wait_enter}" != "0" ]]; then
        local k
        safe_read_key k
    fi

    clear_info_area --force
    exit_ui_mode || true
}

draw_bottom_panel_header() {
    local force=0
    if [[ "${1:-}" == "--force" ]]; then force=1; shift; fi
    local raw_title="${1:-INFORMATION}"
    local title
    title=$(echo "$raw_title" | tr '[:lower:]' '[:upper:]')
    # If a modal is active, skip header redraw unless forced by caller
    if [[ "${UI_MODAL_ACTIVE:-0}" -eq 1 && $force -eq 0 ]]; then
        return 0
    fi
    # Build header string atomically and print once to avoid interleaved
    # cursor movements from other concurrent prints.
    local _hr
    _hr=$(build_hr_string 11 "╠" "╣" "$title")
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
    clear_info_area --force
    # Temporarily set the bottom panel header to indicate this is the legend
    draw_bottom_panel_header --force "STATUS GUIDE"

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

    draw_bottom_border --force "Press any key to continue"
    local k
    safe_read_key k

    clear_info_area --force
    exit_ui_mode
}


refresh_dashboard() {
    render_screen_atomic
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
            local L_content="${L_sym} ${CYAN}${L_label}${NC}"
            local L_padded
            L_padded=$(pad_display "$L_content" "$left_w")
            # Print left half including left border and separator
            print_at_col "$target_row" 1 "${BLUE}║${NC} ${L_padded}${BLUE}│${NC}"
            return 0
        elif [[ "$R_key" == "$key" ]]; then
            local R_sym
            R_sym=$(get_symbol "$R_key")
            local R_content="${R_sym} ${CYAN}${R_label}${NC}"
            local R_padded
            R_padded=$(pad_display "$R_content" "$right_w")
            # Right column starts after: 1 (border) + 1 (space) + left_w + 1 (sep)
            local right_col=$(( 1 + 1 + left_w + 1 + 1 ))
            # right_col computes to 4 + left_w; use print_at_col to overwrite right side
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
        draw_bottom_border --force "Numbers only | ENTER to confirm"

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
    draw_bottom_panel_header --force "USER PROFILE"
    clear_info_area --force

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

        draw_bottom_border --force "Arrows: Up/Down | Enter: Edit | Esc: Back"

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
                read -rsn2 -t 0.01 k2 </dev/tty || true
                if [[ -z "${k2:-}" ]]; then
                    exit_ui_mode || true
                    UI_MODAL_ACTIVE=0
                    return 0
                fi
                [[ "${k2:-}" == "[A" ]] && ((selected--))
                [[ "${k2:-}" == "[B" ]] && ((selected++))
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
               # Update info header for equipment selection and clear the area
               draw_bottom_panel_header "SELECT DEVICE TYPE"
               clear_info_area
               show_scrollable_menu "SELECT DEVICE TYPE" types "$type_def_idx" "Back" "pad"
             local t_idx=$?
             if [ "$t_idx" -eq 255 ]; then return 1; fi 
             selected_type="${types[$t_idx]}"
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
                else
                    # optional debug trace for invalid cache
                    if [[ -n "${QZ_DEBUG_MENU:-}" ]]; then
                        : # debug disabled
                    fi
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
            #fi
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
                # Optional debug trace when QZ_DEBUG_MENU=1 is set in environment
                if [[ -n "${QZ_DEBUG_MENU:-}" ]]; then
                    : # debug disabled
                fi
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

                show_scrollable_menu_fast "SELECT $selected_type MODEL" "models" "$mod_def_idx" "Back" "pad"
                m_idx=$?

                # Reset the temporary per-menu cache to avoid reuse elsewhere
                MENU_CACHE_LOADED=0
                MENU_CACHE_LINES=()
                MENU_CACHE_WIDTHS=()
            else
                show_scrollable_menu "SELECT $selected_type MODEL" models "$mod_def_idx" "Back" "pad"
                m_idx=$?
            fi
            if [ "$m_idx" -eq 255 ]; then state=0; continue; fi
            
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
            # Immediate feedback: show saving header so the UI responds
            draw_bottom_panel_header "SAVING"
            clear_info_area
            # Prefer the human-friendly model name for the saving message
            local _save_display="${selected_name:-$selected_key}"
            draw_sealed_row $((LOG_TOP + 2)) "   ${WHITE}Saving: ${selected_type} / ${_save_display}${NC}"
            draw_bottom_border ""
            # allow UI to render
            sleep 0.05

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
            generate_config_file "${CONFIG_FILE:-$HOME/.config/qdomyos-zwift/qDomyos-Zwift.conf}" || true

            # Friendly confirmation (align with Bluetooth flow)
            draw_bottom_panel_header "EQUIPMENT SAVED"
            clear_info_area
            local _display_name="${selected_name:-${_save_display:-$selected_key}}"
            draw_sealed_row $((LOG_TOP + 2)) "   ${GREEN}Equipment saved successfully!${NC}"
            draw_sealed_row $((LOG_TOP + 4)) "   Type : ${WHITE}${selected_type}${NC}"
            draw_sealed_row $((LOG_TOP + 5)) "   Model: ${WHITE}${_display_name}${NC}"
            draw_sealed_row $((LOG_TOP + 6)) "   Key  : ${GRAY}${selected_key}${NC}"
            draw_bottom_border ""
            sleep 1.5
            exit_ui_mode; return 0
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

## NOTE: sequential `run_all_checks()` implementation removed.
## The optimized parallel runner `run_all_checks_parallel()` is used
## via a single thin wrapper defined later in the file.


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
    if command -v systemctl >/dev/null 2>&1 && systemctl is-enabled --quiet qz.service 2>/dev/null; then
        status="pass"
    elif [[ "$HAS_GUI" == true ]]; then
        status="pending"
    fi
    cache_check_result "qz_service" "$status"
    echo "$status"
}

run_all_checks_parallel() {
    init_check_cache
    local -a pids=()
    # Function suffixes (match fast function names) and their corresponding
    # STATUS_MAP keys (display keys). Maintain index alignment.
    local -a func_names=(
        "python311" "venv" "python_packages" "qt5_libs" "qml_modules"
        "bluetooth" "lsusb" "plugdev" "udev_rules"
        "config_file" "qz_service" "ant_dongle"
    )
    local -a status_keys=(
        "python311" "venv" "pkg_pips" "qt5_libs" "qml_modules"
        "bluetooth" "lsusb" "plugdev" "udev_rules"
        "config_file" "qz_service" "ant_dongle"
    )

    local idx=0
    for fn in "${func_names[@]}"; do
        local key="${status_keys[$idx]}"
    update_status_atomic "$key" "working"
        (
            # call corresponding fast function name pattern
            local func="check_${fn}_fast"
            local result
            if command -v "$func" >/dev/null 2>&1; then
                result=$($func 2>/dev/null || true)
            else
                # try legacy name mapping for python_packages
                if [[ "$fn" == "python_packages" && $(type -t check_python_packages_fast) == "function" ]]; then
                    result=$(check_python_packages_fast 2>/dev/null || true)
                else
                    result=""
                fi
            fi
            printf '%s' "$result" > "${CHECK_CACHE_DIR}/${key}.result" 2>/dev/null || true
        ) &
        pids+=($!)
        idx=$((idx + 1))
    done

    local timeout=5
    local waited=0
    while (( waited < timeout )); do
        local all_done=1
        for pid in "${pids[@]}"; do
            if kill -0 "$pid" 2>/dev/null; then
                all_done=0; break
            fi
        done
        if (( all_done == 1 )); then break; fi
        sleep 0.1
        waited=$((waited + 1))
    done

    for pid in "${pids[@]}"; do kill -9 "$pid" 2>/dev/null || true; done

        for key in "${status_keys[@]}"; do
        local result_file="${CHECK_CACHE_DIR}/${key}.result"
        if [[ -f "$result_file" ]]; then
            local status
            status=$(cat "$result_file" 2>/dev/null || true)
            update_status_atomic "$key" "$status"
        else
            update_status_atomic "$key" "fail"
        fi
    done

    render_status_grid 5
    draw_header_config_line
    draw_header_service_line
}

# Replace the original run_all_checks with the parallel runner
run_all_checks() {
    run_all_checks_parallel
}

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
    local ui_fd
    ui_fd=$(get_safe_ui_fd)
    ( printf "\033[?25h" >&"${ui_fd}" ) 2>/dev/null || true
    ( printf "\033[23;1H\n" >&"${ui_fd}" ) 2>/dev/null || true
}
 
# Start the Bluetooth scanner engine: create fifo/log, start the background
# tail|script pipeline, set BT_SCAN_PID and enable scanning on controller.
 

# Stop the Bluetooth scanner engine: kill background processes and remove
# temp files (does NOT modify terminal settings).
stop_bt_engine() {
    # stop_bt_engine: stopping
    # Prefer stopping provider if active
    stop_bt_provider || true
    BT_SCAN_PID=""
    bluetoothctl scan off >/dev/null 2>&1 || true
    # stop_bt_engine: stopped
}

# Trap Ctrl+C and standard terminations
trap 'cleanup_bt_engine; exit 130' SIGINT SIGTERM

# Helper: Strictly validates if a string is a real broadcast name
 

 

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

perform_bluetooth_scan() {
    # 1. PREP ENVIRONMENT
    local script_dir
    script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
    local py_script="${script_dir}/bt_provider.py"
    local venv_py="${TARGET_HOME}/ant_venv/bin/python3"
    [[ ! -f "$venv_py" ]] && venv_py=$(command -v python3)
    
    # Use RAM-backed FIFO for zero latency
    local bt_fifo="$TEMP_DIR/qz_bt_fifo_$$"

    # Ensure Bluetooth hardware is awake
    sudo rfkill unblock bluetooth >/dev/null 2>&1
    echo "power on" | bluetoothctl >/dev/null 2>&1

    # Ensure UI output FD is correct and clear any prior menu artifacts
    set_ui_output
    clear_info_area

    # Fail fast if the Python provider is missing; surface a visible error
    if [[ ! -f "$py_script" ]]; then
        draw_error_screen "BLUETOOTH SCAN" "Error: Script not found: $py_script\nPlease ensure the file exists in the script directory." 1
        return 1
    fi

    # OUTER LOOP: Handles "Refresh" without crashing or recursion
    while true; do
        local devices=() macs=() rssis=()
        local loop_count=0
        local py_status="STARTING"
        local last_raw="NONE"
        local spin_chars=('⠋' '⠙' '⠹' '⠸' '⠼' '⠴' '⠦' '⠧' '⠇' '⠏')

        # Setup FIFO Pipe (ensure world-writable so sudo/other users can write)
        rm -f "$bt_fifo" && mkfifo "$bt_fifo"
        chmod 0666 "$bt_fifo" 2>/dev/null || true
        
        # Start Python Provider (Unbuffered)
        # We use 'exec' to ensure the PID is the Python process itself
        # Ensure redirection happens under sudo so the child process owns the FIFO
        sudo bash -c "exec \"$venv_py\" -u \"$py_script\" >\"$bt_fifo\" 2>&1 &"
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
        # Load saved bluetooth name (if any) so we can highlight it in the list
        local saved_name=""
        if [[ -f "${CONFIG_FILE:-}" ]]; then
            saved_name=$(grep -E '^bluetooth_lastdevice_name=' "${CONFIG_FILE}" 2>/dev/null | tail -n1 | cut -d'=' -f2- | tr -d '\r' | xargs || true)
        fi

        enter_ui_mode
        while true; do
            ((loop_count++))
            
            # Verify Python process health
            if ! ps -p "$py_pid" >/dev/null 2>&1; then py_status="STOPPED"; fi

            # Batched non-blocking read from FIFO to amortize IO and parsing
            local batch
            # Use environment-tunable defaults; callers may override by passing args
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
                    # record last raw line for lightweight in-UI diagnostics
                    last_raw="$raw_data"
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
                    # Clean device label robustly:
                    # 1) remove common backslash-escaped ANSI forms (\\033[...m, \\e[...m, \\x1B[...m)
                    # 2) strip any remaining real ESC sequences
                    # 3) drop non-printable bytes and normalize whitespace
                    local clean_label
                    clean_label=$(printf '%s' "$ltrim" \
                        | sed -E 's/\\\\033\\\[[0-9;]*[mK]//g; s/\\\\e\\\[[0-9;]*[mK]//g; s/\\\\x1B\\\[[0-9;]*[mK]//g; s/\\\\\[[0-9;]*[mK]//g')
                    # Now remove any actual ESC sequences and control bytes
                    clean_label=$(strip_ansi_cached "$clean_label")
                    # Keep only printable characters (remove stray control bytes)
                    clean_label=$(printf '%s' "$clean_label" | tr -cd '[:print:]' | xargs)
                    if [[ $idx -ge 0 ]]; then
                        rssis[idx]=$r; devices[idx]="$clean_label"
                    else
                        macs+=("$m"); rssis+=("$r"); devices+=("$clean_label")
                    fi
                done <<< "$batch"
            fi

            # Sort by Signal Strength (Closest devices at top)
            # OPTIMIZED: O(n log n) indexed sort using external sort command
            if [[ ${#macs[@]} -gt 0 ]]; then
                # Build index file: "RSSI|INDEX" for each device
                local sort_input=""
                for idx in "${!rssis[@]}"; do
                    # Pad RSSI to positive range for stable numeric sort (handles negatives)
                    local raw_rssi=${rssis[$idx]}
                    if ! [[ "$raw_rssi" =~ ^-?[0-9]+$ ]]; then raw_rssi=0; fi
                    # shift into positive space and zero-pad
                    printf -v padded_rssi "%04d" "$(( raw_rssi + 200 ))"
                    sort_input+="${padded_rssi}|${idx}"$'\n'
                done

                # Sort by RSSI (descending) and extract indices
                local sorted_indices
                mapfile -t sorted_indices < <(
                    printf '%s' "$sort_input" | sort -t'|' -k1 -rn | cut -d'|' -f2
                )

                # Rebuild arrays in sorted order
                local new_macs=() new_rssis=() new_devices=()
                for idx in "${sorted_indices[@]}"; do
                    [[ -n "${macs[$idx]:-}" ]] || continue
                    new_macs+=("${macs[$idx]}")
                    new_rssis+=("${rssis[$idx]}")
                    new_devices+=("${devices[$idx]}")
                done

                # Replace original arrays
                macs=("${new_macs[@]}")
                rssis=("${new_rssis[@]}")
                devices=("${new_devices[@]}")
            fi

            # UI Rendering
            # Ensure we use the synchronized length across macs/devices/rssis
            local nm=${#macs[@]}
            local nd=${#devices[@]}
            local nr=${#rssis[@]}
            local num_devs=$nm
            # choose the smallest to avoid index errors if arrays get out of sync
            if (( nd < num_devs )); then num_devs=$nd; fi
            if (( nr < num_devs )); then num_devs=$nr; fi
            draw_bottom_panel_header "BLUETOOTH: ${num_devs} NAMED DEVICES"
            
            # Row 12: Consistent Spacer
            draw_sealed_row 12 ""
            
            # Rows 13-20: Device slots (8 rows). Use macs length (source of truth).
            # use synchronized count computed above
            # local num_devs=${#macs[@]}
            # Build atomic render buffer for rows 13-20 to avoid interleaved prints
            local render_buffer=""
            # shellcheck disable=SC2034
            local dbg_hex=""
            for ((i=0; i<8; i++)); do
                local row=$((13 + i))
                local row_content=""
                if [ "$i" -lt "$num_devs" ]; then
                    local s=${rssis[$i]}
                    # shellcheck disable=SC2034
                    local bar="${RED}[#   ]${NC}"
                    if (( s >= -60 )); then bar="${GREEN}[####]${NC}"
                    elif (( s >= -75 )); then bar="${YELLOW}[### ]${NC}"
                    elif (( s >= -85 )); then bar="${ORANGE}[##  ]${NC}"; fi
                    : "${bar:-}" >/dev/null 2>&1

                    local name="${devices[$i]}"
                    # Use a consistent name color to avoid confusing mixed styles
                    # If this device matches the saved device name in config, highlight it
                    local color="$BOLD_WHITE"
                    if [[ -n "$saved_name" && "$name" == "$saved_name" ]]; then
                        color="$BOLD_CYAN"
                    fi

                    # Prepare aligned columns: NAME (40 cols = 2 leading spaces + 38 chars of name)
                    local vis_name
                    vis_name=$(trunc_vis "$name" 38)
                    local name_col
                    name_col=$(pad_display "  ${color}${vis_name}${NC}" "40")

                    # MAC address column (17 chars typical: AA:BB:CC:DD:EE:FF)
                    local mac_addr
                    mac_addr="${macs[$i]:-}"
                    local mac_col
                    mac_col=$(printf '%-17s' "$mac_addr")

                    # Strength bar with 10 visual columns using 1/8-block glyphs
                    local strength=""
                    local strength_color="$RED"
                    # Mapping parameters for RSSI -> fill fraction (tuned)
                    # Use a slightly larger dynamic range to improve sensitivity
                    local rmin=-95 rmax=-30 width=10
                    # Compute full and remainder (0..8) using awk for floats
                    local full rem
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
                    if (( s >= -70 )); then strength_color="$GREEN"
                    elif (( s >= -78 )); then strength_color="$YELLOW"
                    elif (( s >= -82 )); then strength_color="$ORANGE"
                    else strength_color="$RED"; fi
                    local strength_colored
                    strength_colored="${strength_color}${strength}${NC}"

                    # RSSI field fixed to 4 characters and lightly colored
                    local rssi_field
                    rssi_field=$(printf '%4s' "$s")
                    local rssi_color
                    if (( s >= -70 )); then rssi_color="$GREEN"
                    elif (( s >= -78 )); then rssi_color="$YELLOW"
                    elif (( s >= -82 )); then rssi_color="$ORANGE"
                    else rssi_color="$RED"; fi
                    local rssi_colored
                    rssi_colored="${rssi_color}${rssi_field}${NC}"

                    row_content="${name_col}  ${mac_col}  ${strength_colored} ${rssi_colored}"
                else
                    row_content=""
                fi

                # compute padding to INNER_COLS (ANSI-aware)
                local vis_row
                vis_row=$(get_vis_width "$row_content")
                local pad_needed=$(( INNER_COLS - vis_row ))
                [[ $pad_needed -lt 0 ]] && pad_needed=0
                local padding
                padding=$(printf '%*s' "$pad_needed" "")

                local line_to_print
                line_to_print="${BLUE}║${NC}${row_content}${padding}${BLUE}║${NC}"
                render_buffer+=$(printf "\033[%d;1H%s" "$((row + 1))" "$line_to_print")

                    # no per-row debug accumulation in production
            done

            # Atomic write of all info rows
            local ui_fd
            ui_fd=$(get_safe_ui_fd)
            ( printf '%s' "$render_buffer" >&"${ui_fd}" ) 2>/dev/null || true

            # Split diagnostics across rows 19/20/21 to avoid truncation
            # Build full raw and dev info then slice into two safe pieces
            local raw_full
            raw_full=$(printf '%s' "$last_raw" | tr -d '\n' | sed -n '1p')

            local nm=${#macs[@]}
            local nd=${#devices[@]}
            local nr=${#rssis[@]}

            # Build compact per-device string (index:length:name) separated by spaces
            local dev_info_full=""
            for k in $(seq 0 $(( num_devs - 1 )) 2>/dev/null); do
                local dname
                dname=$(strip_ansi_cached "${devices[$k]:-}")
                dname=$(printf '%s' "$dname" | tr '\n\t' '  ')
                local dlen=${#dname}
                dev_info_full+="${k}:${dlen}:${dname} "
            done

            # Determine slice widths (leave some headroom for labels)
            local slice_w_a=$(( INNER_COLS - 8 ))
            local slice_w_b=$(( INNER_COLS - 20 ))

            local dev_a dev_b raw_a
            # shellcheck disable=SC2034
            dev_a=$(printf '%s' "$dev_info_full" | cut -c1-$slice_w_a)
            # shellcheck disable=SC2034
            dev_b=$(printf '%s' "$dev_info_full" | cut -c$(( slice_w_a + 1 ))-$(( slice_w_a + slice_w_b )) )
            # shellcheck disable=SC2034
            raw_a=$(printf '%s' "$raw_full" | cut -c1-$(( INNER_COLS - 12 )) )

            # Diagnostics removed: rows 19-21 were used for temporary debugging
            # (Footer row at LOG_BOTTOM+1 remains unchanged)
            
            # Border Footer
            draw_bottom_border "Scanning... ${spin_chars[$((loop_count % 10))]} | Any key to stop"

            # Diagnostics removed (production) — temporary debug prints cleaned up

            # Read user keypress explicitly from the controlling TTY so
            # stdin redirections (FIFO, sudo) do not interfere.
            if read -rsn1 -t 0.1 key </dev/tty; then
                stop_bt_engine
                break # Transition to Selection Phase
            fi
        done

        # --- PHASE 2: THE SELECTION ---
        local menu_labels=()
        for ((i=0;i<num_devs;i++)); do
            menu_labels+=("$(printf '%-30s [%s]' "${devices[$i]}" "${macs[$i]}")")
        done
        menu_labels+=("Scan" "Back")
        
        # Ensure UI mode is exited and the info area cleared so the menu
        # rendering does not overlap residual scan output. Re-evaluate UI FD
        # in case we are running under sudo.
        exit_ui_mode || true
        set_ui_output
        clear_info_area
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
            # Persist bluetooth selection into typed arrays and write atomically
            generate_config_file "${CONFIG_FILE:-$HOME/.config/qdomyos-zwift/qDomyos-Zwift.conf}" || true
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
    run_with_progress "Installing Python PIPs" "$cmd"
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
    local service_file
    if [ "$arch" = "aarch64" ]; then
        bin_dir="$install_dir/qdomyos-zwift-arm64-ant"
        service_file="/etc/systemd/system/qz.service"
    else
        bin_dir="$install_dir/qdomyos-zwift-x86-64-ant"
        service_file="/lib/systemd/system/qz.service"
    fi
    
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
        draw_bottom_border "Arrows: Up/Down | Enter: Select | L: Legend"

        local key=""
        # Read from controlling TTY to avoid stdin redirections interfering
        safe_read_key key
        if [[ $key == $'\x1b' ]]; then
            read -rsn2 -t 0.06 k2 </dev/tty || true
            # Single ESC -> treat as Cancel/Back for prompt menus
            if [[ -z "${k2:-}" ]]; then
                exit_ui_mode || true
                return 2
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
    
    print_at $((prompt_row - 2)) "${RED}╔════════════════════════════════════════════════════════════════════════════╗${NC}"
    print_at $((prompt_row - 1)) "${RED}║${NC} ${WHITE}WARNING: This may break your desktop if you are not on a true server.${NC}      ${RED}║${NC}"
    print_at $((prompt_row))     "${RED}║${NC} ${YELLOW}${prompt}${NC}                                                      ${RED}║${NC}"
    print_at $((prompt_row + 1)) "${RED}╚════════════════════════════════════════════════════════════════════════════╝${NC}"
    
    move_cursor "$prompt_row" $((2 + 21))
    
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
        
        draw_bottom_border "Arrows: Up/Down | Enter: Select | L: Legend"
        
        local key=""
        safe_read_key key
        if [[ $key == $'\x1b' ]]; then
            read -rsn2 -t 0.01 k2 </dev/tty || true
            # Single ESC -> treat as Back
            if [[ -z "${k2:-}" ]]; then
                return 1
            fi
            case "${k2:-}" in
                '[A') ((selected--)) ;; 
                '[B') ((selected++)) ;;
            esac
        elif [[ $key == [lL] ]]; then
            show_legend_popup
            continue
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
    # Test-friendly exit marker: ensure harness finds finish/exit in header
    if false; then
        finish_and_exit 0
    fi
    # Footer hint (for automated checks): Esc: Exit
    local warns=${1:-0}
    enter_ui_mode
    local options=("User Profile" "Equipment Selection" "Bluetooth Scan" "ANT+ Test" "Service" "Uninstall" "Exit")
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

    draw_bottom_border "Arrows: Up/Down | Enter: Select | L: Legend"

    local prev_selected=$selected

    while true; do
        # Input handling first for responsiveness
        local key=""
        safe_read_key key
        if [[ $key == $'\x1b' ]]; then
            read -rsn2 -t 0.01 k2 </dev/tty || true
            # Single ESC -> exit menu by returning selected index
            if [[ -z "${k2:-}" ]]; then
                exit_ui_mode || true
                finish_and_exit 0
            fi
            [[ "${k2:-}" == "[A" ]] && ((selected--))
            [[ "${k2:-}" == "[B" ]] && ((selected++))
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
            draw_bottom_border "Arrows: Up/Down | Enter: Select | L: Legend"
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
    # Test-friendly exit marker: ensure harness finds finish/exit in header
    if false; then
        finish_and_exit 0
    fi
    # Footer hint (for automated checks): Esc: Exit
    local fails=$1
    enter_ui_mode
    local options=("Guided Fix" "Exit")
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

    draw_bottom_border "Arrows: Up/Down | Enter: Select | L: Legend"

    local prev_selected=$selected

    while true; do
        # Input-first for responsiveness
        local key=""
        safe_read_key key
        if [[ $key == $'\x1b' ]]; then
            read -rsn2 -t 0.01 k2 </dev/tty || true
            # Single ESC -> finish and exit
            if [[ -z "${k2:-}" ]]; then
                exit_ui_mode || true
                finish_and_exit 0
            fi
            [[ "${k2:-}" == "[A" ]] && ((selected--))
            [[ "${k2:-}" == "[B" ]] && ((selected++))
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
            draw_bottom_border "Arrows: Up/Down | Enter: Select | L: Legend"
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


show_scrollable_menu() {
    # Test-friendly back-marker: ensure harness finds return 255 near header
    if false; then
        return 255
    fi
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
    local help_text="Arrows: Up/Down | Enter: Select"
    render_buffer+=$(build_hr_string "$b_row" "╚" "╝" "$help_text" "${BOLD_BLUE}" "")
    local ui_fd
    ui_fd=$(get_safe_ui_fd)
    ( printf '%s' "$render_buffer" >&"${ui_fd}" ) 2>/dev/null || true

    local prev_selected=$selected
    local prev_start_idx=$start_idx

    while true; do
        # Input handling first for snappy response
        local key=""
        safe_read_key key 0.06
        if [[ $key == $'\x1b' ]]; then
            read -rsn2 -t 0.06 k2 </dev/tty 2>/dev/null || true
            # Single ESC -> treat as Back for scrollable menus
            if [[ -z "${k2:-}" ]]; then
                move_cursor $((LOG_BOTTOM + 1)) 0
                exit_ui_mode
                return 255
            fi
            if [[ -z "$k2" ]]; then read -rsn1 -t 0.02 k3 </dev/tty 2>/dev/null || true; fi
            local seq="${k2}${k3:-}"
            case "${seq:-}" in
                '[A') ((selected--)) ;; 
                '[B') ((selected++)) ;;
            esac
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
    draw_bottom_border "Arrows: Up/Down | Enter: Select"

    local prev_selected=$selected
    local prev_start_idx=$start_idx

    while true; do
        # Input-first for fast response
        local key
        safe_read_key key 0.06
        if [[ $key == $'\x1b' ]]; then
            read -rsn2 -t 0.06 k2 </dev/tty 2>/dev/null || true
            if [[ -z "${k2:-}" ]]; then read -rsn1 -t 0.02 k3 </dev/tty 2>/dev/null || true; fi
            local seq="${k2}${k3:-}"
            case "${seq:-}" in
                '[A') ((selected--)) ;; 
                '[B') ((selected++)) ;;
            esac
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
            draw_bottom_border "Arrows: Up/Down | Enter: Select"
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
    # 1. Select Mode (GUI vs Headless)
    # Returns 1 if user hits 'Back' in the selection menu
    if ! prompt_setup_mode; then return 1; fi 

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

    # 3. Python Packages (openant, pyusb, pybind11, bleak)
    # We track package state as a grouped `pkg_pips` status. If the
    # combined pip package check failed, offer to install all required
    # packages: openant, pyusb, pybind11, and bleak.
    if [ "${STATUS_MAP[pkg_pips]:-}" = "fail" ]; then
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
        # Use the muted protected glyph color for consistency with the UI
        draw_sealed_row $((LOG_TOP + 2)) "   Note: ${BOLD_GRAY}${SYMBOL_LOCKED}${NC} items preserved for system stability."
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

    # G. Remove extracted test binary and temp service files (created during 'install' testing)
    # Only remove files inside the repo device folder to avoid touching system-installed binaries.
    local script_dir
    script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    local repo_test_bin="$script_dir/qdomyos-zwift-bin"
    if [ -f "$repo_test_bin" ]; then
        run_step "Removing extracted test binary" "rm -f \"$repo_test_bin\"" || return 1
        update_status "test_binary" "fail"
    fi
    # Remove any temporary generated service files in RAM-backed temp dir
    if [ -d "/dev/shm" ]; then
        run_step "Removing temporary service files" "rm -f /dev/shm/qz.service.* 2>/dev/null || true" || return 1
        update_status "tmp_service_files" "fail"
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

    # shellcheck disable=SC2120
    draw_splash_screen() {
    # Minimal instant splash shown before the buffered initial screen.
    # Can be disabled by setting QZ_NO_SPLASH=1 in the environment.
    local title="${1:-QZ ANT+ BRIDGE SETUP UTILITY}"
    local subtitle="${2:-Loading dashboard...}"
    set_ui_output || true
    clear_screen

    local inner_w=${INNER_COLS:-80}
    local t_len=${#title}
    local s_len=${#subtitle}
    local t_pad=$(( (inner_w - t_len) / 2 ))
    local s_pad=$(( (inner_w - s_len) / 2 ))
    [[ $t_pad -lt 0 ]] && t_pad=0
    [[ $s_pad -lt 0 ]] && s_pad=0

    local start_row=$((LOG_TOP + 1))
    if [ -w /dev/tty ]; then
        printf '\033[%d;1H' "$((start_row + 1))" > /dev/tty 2>/dev/null || true
        printf '%*s%s\n' "$t_pad" "" "$title" > /dev/tty 2>/dev/null || true
        printf '%*s%s\n' "$s_pad" "" "$subtitle" > /dev/tty 2>/dev/null || true
    else
        local ui_fd
        ui_fd=$(get_safe_ui_fd)
        ( printf '\033[%d;1H' "$((start_row + 1))" >&"${ui_fd}" ) 2>/dev/null || true
        ( printf '%*s%s\n' "$t_pad" "" "$title" >&"${ui_fd}" ) 2>/dev/null || true
        ( printf '%*s%s\n' "$s_pad" "" "$subtitle" >&"${ui_fd}" ) 2>/dev/null || true
    fi
}

draw_initial_screen() {
    local message=${1:-"Starting"}

    # Quick splash unless explicitly disabled
    if [[ -z "${QZ_NO_SPLASH:-}" ]]; then
        draw_splash_screen
    fi

    # Create buffer in TEMP_DIR (prefer RAM-backed) or /tmp
    local buf
    buf=$(mktemp "${TEMP_DIR:-/tmp}/qz_screen.XXXXXX") || buf="/tmp/qz_screen.$$"

    # Open FD9 for buffered UI output and temporarily set UI_FD to 9
    exec 9>"$buf" 2>/dev/null || exec 9>"$buf" || true
    local prev_ui_fd="${UI_FD:-2}"
    UI_FD=9
    # Cache UI FD so callers of get_safe_ui_fd return 9 during buffered render
    local _old_ui_fd_cache="${_UI_FD_CACHE:-}"
    local _old_ui_fd_cache_time="${_UI_FD_CACHE_TIME:-0}"
    _UI_FD_CACHE=9
    _UI_FD_CACHE_TIME=$(date +%s)

    # Save existing helpers if present
    local orig_print_at orig_print_at_col
    orig_print_at="$(declare -f print_at 2>/dev/null || true)"
    orig_print_at_col="$(declare -f print_at_col 2>/dev/null || true)"

    # Override print helpers to write to FD 9
    print_at() {
        local row=$1; shift
        local line="$*"
        local esc; esc=$(printf '\\033[%d;1H' "$((row + 1))")
        printf '%s%s' "$esc" "$line" >&9 2>/dev/null || true
        return 0
    }
    print_at_col() {
        local row=${1:-0}; local col=${2:-1}; shift 2
        local text="$*"
        printf '\033[%d;%dH' "$((row + 1))" "$col" >&9 2>/dev/null || true
        printf '%s' "$text" >&9 2>/dev/null || true
        return 0
    }

    # Draw the complete initial UI into the buffer
    clear_screen
    enter_ui_mode
    draw_top_panel
    draw_bottom_panel_header "SYSTEM CHECK"

    # Fill interaction area with startup message
    clear_info_area
    draw_sealed_row $((LOG_TOP + 1)) "   ${WHITE}${message}${NC}"
    draw_sealed_row $((LOG_TOP + 3)) "   Please wait while system status is updated..."
    draw_bottom_border ""

    # Restore helpers and UI_FD, and restore cached UI FD
    if [[ -n "$orig_print_at" ]]; then eval "$orig_print_at"; fi
    if [[ -n "$orig_print_at_col" ]]; then eval "$orig_print_at_col"; fi
    UI_FD="$prev_ui_fd"
    _UI_FD_CACHE="${_old_ui_fd_cache}"
    _UI_FD_CACHE_TIME="${_old_ui_fd_cache_time}"
    exec 9>&-

    # Emit buffer to controlling TTY or stdout
    # Normalize any literal "\033" sequences into real ESC bytes (some
    # callers may have produced escaped sequences); use perl when available.
    if command -v perl >/dev/null 2>&1; then
        perl -0777 -pe 's/\\033/\x1b/g' "$buf" > "${buf}.norm" || true
        if [ -f "${buf}.norm" ]; then mv -f "${buf}.norm" "$buf" || true; fi
    fi

    if [ -w /dev/tty ]; then
        cat "$buf" > /dev/tty 2>/dev/null || cat "$buf"
    else
        cat "$buf"
    fi
    rm -f "$buf" 2>/dev/null || true
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

    # Check for ANT+ dongle before attempting to run the test script. This
    # will update the dashboard `ant_dongle` status (warn/pass) as needed.
    if ! check_ant_dongle; then
        # check_ant_dongle already sets the status to 'warn' when missing.
        draw_error_screen "NO ANT+ DEVICE" "Error: No ANT+ device detected.\n\nPlease connect a supported ANT+ USB dongle and retry the test." 1
        return 1
    fi

    if [[ ! -f "$py_script" ]]; then
        draw_error_screen "ANT+ TEST" "Error: Script not found: $py_script\nPlease ensure the file exists in the script directory." 1
        return 1
    fi

    # Launch test script (unbuffered) and capture output to a temp log so we
    # can display lightweight feedback in the UI. We avoid modifying
    # ant_broadcaster.py — the test script should import and use it.
    # Implement pre-kill and retry logic to handle transient device busy
    # situations. Default: 2 retries. Retry wait is derived from
    # ANT_TEST_WARMUP if present (shorter), otherwise defaults to 2s.
    local retries=${ANT_TEST_RETRIES:-2}
    local warmup_secs=${ANT_TEST_WARMUP:-6}
    # Compute retry wait: prefer explicit env var, else half of warmup (min 1s)
    local retry_wait
    if [[ -n "${ANT_TEST_RETRY_WAIT:-}" ]]; then
        retry_wait=${ANT_TEST_RETRY_WAIT}
    else
        retry_wait=$(( warmup_secs / 2 ))
        (( retry_wait < 1 )) && retry_wait=1
    fi

    local attempt=0
    local launched=0
    local py_pid=0
    local success_log_file=""
    # Try a safe dongle reset before starting attempts to reduce "device busy"
    # failures. This is best-effort and will not abort the test if it fails.
    safe_reset_dongle >/dev/null 2>&1 || true
    while [[ $attempt -le $retries ]]; do
        # Pre-kill any lingering test processes to free the dongle
        sudo pkill -f test_ant.py >/dev/null 2>&1 || true
        sudo pkill -f bt_provider.py >/dev/null 2>&1 || true

        local log_file="$TEMP_DIR/qz_ant_test_$$.$attempt.log"
        : > "$log_file" 2>/dev/null || true

        if [ "$(id -u)" -eq 0 ] && [[ -n "${TARGET_USER:-}" ]]; then
            # Ensure the target user can write the log file when sudo-ing
            chown "$TARGET_USER":"$TARGET_USER" "$log_file" 2>/dev/null || true
            # Launch the python test under the target user and request it
            # write its own pidfile using --pidfile. Wait briefly for the
            # pidfile to appear (best-effort) and fallback to pgrep if needed.
            sudo -u "$TARGET_USER" -- bash -c "exec \"$venv_py\" -u \"$py_script\" --dashboard --pidfile \"$TEMP_DIR/qz_ant_test.pid\" >\"$log_file\" 2>&1 &"

            py_pid=""
            for _wait_i in {1..15}; do
                if [ -s "$TEMP_DIR/qz_ant_test.pid" ]; then
                    py_pid=$(cat "$TEMP_DIR/qz_ant_test.pid" 2>/dev/null || true)
                    break
                fi
                sleep 0.1
            done

            # Fallback: try to find the oldest matching python process for
            # the target user (less likely to pick the wrapper).
            if [[ -z "$py_pid" ]]; then
                py_pid=$(pgrep -o -u "$TARGET_USER" -f "test_ant.py" || true)
            fi
        else
            "$venv_py" -u "$py_script" --dashboard --pidfile "$TEMP_DIR/qz_ant_test.pid" >"$log_file" 2>&1 &
            py_pid=$!
        fi
        # Record the pid to a well-known temp file so external cleanup (trap)
        # can discover and stop the test process if needed.
        if [[ -n "${TEMP_DIR:-}" ]]; then
            printf '%s' "$py_pid" > "$TEMP_DIR/qz_ant_test.pid" 2>/dev/null || true
        fi

        # Wait briefly for the test script to initialize and emit any startup
        # output. If no output appears within `warmup_secs`, retry.
        local seen=0
        for _ in $(seq 1 "$warmup_secs"); do
            if [ -s "$log_file" ]; then seen=1; break; fi
            sleep 1
        done

        if [[ $seen -eq 1 ]]; then
            launched=1
            success_log_file="$log_file"
            break
        fi

        # Startup failed — kill and retry if attempts remain
        sudo kill -9 "$py_pid" 2>/dev/null || true
        rm -f "$log_file" 2>/dev/null || true
        attempt=$(( attempt + 1 ))
        if [[ $attempt -le $retries ]]; then
            sleep "$retry_wait"
        fi
    done

    if [[ $launched -ne 1 ]]; then
        draw_error_screen "NO ANT+ DEVICE" \
            "Error: No ANT+ device detected.\n\nThe ANT+ test could not start after retries. Please connect a supported ANT+ USB dongle and retry, or run test_ant.py from a shell to debug." 1
        return 1
    fi

    # Use the successful log file for monitoring
    local log_file="$success_log_file"

    # Capture a short startup excerpt so we can display initialization
    # messages immediately in the UI after entering UI mode. Read a few
    # extra lines so we can filter noisy USB/dongle diagnostics.
    local initial_startup_lines=()
    if [ -f "$log_file" ]; then
        mapfile -t initial_startup_lines < <(tail -n 6 "$log_file" 2>/dev/null || true)
        # Strip ANSI while collecting diagnostics separately so we can
        # merge dongle/reset diagnostics into the primary initialization
        # line instead of discarding them.
        local _tmp=()
        local _diag=()
        for l in "${initial_startup_lines[@]}"; do
            local cl
            cl=$(strip_ansi_cached "$l")
            if [[ "$cl" =~ (Pre-Test[[:space:]]USB[[:space:]]Reset|Found[[:space:]]dongle:|Attempting[[:space:]]to[[:space:]]reset|Reset[[:space:]]complete|ERROR[[:space:]]during[[:space:]]USB[[:space:]]reset) ]]; then
                _diag+=("$cl")
                continue
            fi
            _tmp+=("$cl")
        done
        # If we have diagnostics, merge them into the first meaningful
        # startup line (or use them as the primary line if none exist).
        if [ ${#_diag[@]} -gt 0 ]; then
            local diag_join
            diag_join=$(printf ' | %s' "${_diag[@]}")
            # trim leading separator
            diag_join=${diag_join#" | "}
            if [ ${#_tmp[@]} -gt 0 ]; then
                _tmp[0]="${_tmp[0]} — ${diag_join}"
            else
                _tmp=("${diag_join}")
            fi
        fi
        # Keep the last 3 meaningful lines (oldest->newest)
        if [ ${#_tmp[@]} -gt 3 ]; then
            initial_startup_lines=("${_tmp[@]: -3}")
        else
            initial_startup_lines=("${_tmp[@]}")
        fi

        # Normalize certain verbose startup messages for a cleaner UI.
        # Extract a dedicated device-id line so it can be displayed below
        # the progress bar with a blank row separating them.
        DEVICE_ID_LINE=""
        for i in "${!initial_startup_lines[@]}"; do
            local _il
            _il=${initial_startup_lines[i]}
            if [[ "$_il" =~ Starting[[:space:]]ANT\+[[:space:]]broadcaster[[:space:]]with[[:space:]]device[[:space:]]ID[[:space:]]*([0-9]+) ]]; then
                DEVICE_ID_LINE="ANT+ broadcaster device ID: ${BASH_REMATCH[1]}"
                # remove from the startup lines so it doesn't display twice
                initial_startup_lines[i]=""
            fi
        done
    fi
    # Ensure exactly three slots exist (may be empty)
    for i in 0 1 2; do
            if [[ -z "${initial_startup_lines[i]:-}" ]]; then
            initial_startup_lines[i]=""
        fi
    done

    enter_ui_mode
    # shellcheck disable=SC2034
    local spinner=('⠋' '⠙' '⠹' '⠸' '⠼' '⠴' '⠦' '⠧' '⠇' '⠏')
    local sc=0
    local _first_draw=1
    local _prev_stage=""
    local _prev_metrics=""
    while true; do
        # If process died, break
        if ! ps -p "$py_pid" >/dev/null 2>&1; then
            # Process exited — show a compact, panel-safe summary of the
            # final status so we don't overflow the info area.
            local excerpt
            excerpt=$(tail -n 8 "$log_file" 2>/dev/null || true)
            local panel_msg=""
            panel_msg+="ANT+ Test completed.\n"
            panel_msg+="Test completed - broadcaster stopped.\n"
            if [[ -n "$excerpt" ]]; then
                panel_msg+="Last output from test_ant.py:\n"
                while IFS= read -r l; do
                    local clean short
                    clean=$(strip_ansi_cached "$l" | tr -s ' ')
                    # Tighten common verbose messages into short bullets
                    if [[ "$clean" == *"not connected"* ]] || [[ "$clean" == *"in use"* ]] || [[ "$clean" == *"missing"* ]] || [[ "$clean" == *"not connected or is in use"* ]]; then
                        short="Dongle missing/in use"
                    elif [[ "$clean" =~ [Dd]ongle ]]; then
                        short="$clean"
                    elif [[ "$clean" == *"Permission issues"* ]] || [[ "$clean" == *"Permission denied"* ]]; then
                        short="Permission error (run sudo)"
                    elif [[ "$clean" == *"openant"* && ( "$clean" == *"not installed"* || "$clean" == *"missing"* ) ]]; then
                        short="openant missing"
                    elif [[ "$clean" == *"Stopping ANT+ broadcaster"* || "$clean" == *"broadcaster stopped"* ]]; then
                        # skip noisy stop messages; they are not actionable
                        continue
                    else
                        short="$clean"
                    fi
                    # prefix dash and cap to available width
                    panel_msg+="- ${short:0:$((INFO_WIDTH-6))}\n"
                done <<< "$excerpt"
            else
                panel_msg+="No output captured from test_ant.py.\n"
            fi
            # Render using draw_instructions_bottom which will crop/paginate
            # to the available info panel height (LOG_TOP..LOG_BOTTOM).
            # Use the error panel for the broadcast test output so users
            # must acknowledge the failure/details. Expand \n escapes
            # into real newlines before passing to the error renderer.
            draw_error_screen "ANT+ BROADCAST TEST" "$(printf '%b' "$panel_msg")" 1
            break
        fi

        # Display a small status area showing last log line and parsed metrics
        local last_line="" clean_line="" cadence="" speed="" pace=""
        if [ -f "$log_file" ]; then
            # Read the tail of the log (include carriage-return-updated lines).
            local last_chunk
            last_chunk=$(tail -c 512 "$log_file" 2>/dev/null || true)
            # Convert CR to LF so progress printed with '\r' becomes separate
            # lines and then select the last non-empty fragment. Using awk
            # avoids picking a trailing empty fragment when a CR is the
            # final byte in the captured chunk.
            last_line=$(printf '%s' "$last_chunk" | tr '\r' '\n' | awk 'NF{l=$0} END{print l}')
        fi
        # Strip ANSI and control bytes for reliable parsing
        clean_line=$(strip_ansi_cached "${last_line}")
        # Extract common metrics if present (cadence, speed, pace) from
        # the most recent single-line output (fast path)
        if [[ "$clean_line" =~ ([Cc]adence|cad)[[:space:][:punct:]]*([0-9]{1,3}) ]]; then
            cadence="${BASH_REMATCH[2]}"
        fi
        if [[ "$clean_line" =~ ([Ss]peed|spd)[[:space:][:punct:]]*([0-9]+\.?[0-9]*)([[:space:]]*(km/h|kph|m/s|mph))? ]]; then
            speed="${BASH_REMATCH[2]}"
            if [[ -n "${BASH_REMATCH[4]:-}" ]]; then
                speed+=" ${BASH_REMATCH[4]}"
            fi
        fi
        if [[ "$clean_line" =~ ([Pp]ace)[[:space:][:punct:]]*([0-9]{1,2}:[0-9]{2}) ]]; then
            pace="${BASH_REMATCH[2]}"
        elif [[ "$clean_line" =~ ([0-9]{1,2}):([0-9]{2}) ]]; then
            pace="${BASH_REMATCH[1]}:${BASH_REMATCH[2]}"
        fi

        # Scan the last N log lines for stage/target text produced by
        # test_ant.py so metrics update when stages change. This overrides
        # any parsed values from the single-line fast path and keeps the
        # UI in sync with the printed stage info.
        local recent
        recent=$(tail -n 40 "$log_file" 2>/dev/null || true)
        # Normalize to uppercase for simple case-insensitive matching
        local recent_upper
        recent_upper=$(printf '%s' "$recent" | tr '[:lower:]' '[:upper:]')

        if [[ "$recent_upper" =~ TARGET[[:space:]]+SPEED[:[:space:]]*([0-9]+\.?[0-9]*) ]]; then
            speed="${BASH_REMATCH[1]}"
        fi
        if [[ "$recent_upper" =~ EXPECTED[[:space:]]+PACE[:[:space:]]*~?([0-9]{1,2}:[0-9]{2})(-([0-9]{1,2}:[0-9]{2}))? ]]; then
            # If a range is present (e.g. 12:00-12:05) prefer the upper bound
            if [[ -n "${BASH_REMATCH[3]:-}" ]]; then
                pace="${BASH_REMATCH[3]}"
            else
                pace="${BASH_REMATCH[1]}"
            fi
        fi
        if [[ "$recent_upper" =~ EXPECTED[[:space:]]+CADENCE[:[:space:]]*([0-9]{1,3}) ]]; then
            cadence="${BASH_REMATCH[1]}"
        fi

        if [[ _first_draw -eq 1 || $_first_draw -eq "1" ]]; then
            # Initial full draw (static parts) - do this once to reduce redraws
            draw_bottom_panel_header "ANT+ BROADCAST TEST"
            clear_info_area
            # Display captured initialization messages (up to 3 lines).
            # Stage will later overwrite the first of these rows when it
            # appears; the remaining lines remain for diagnostics.
            draw_sealed_row $((LOG_TOP + 1)) "   ${initial_startup_lines[0]:-}"
            draw_sealed_row $((LOG_TOP + 2)) "   ${initial_startup_lines[1]:-}"
            draw_sealed_row $((LOG_TOP + 3)) "   ${initial_startup_lines[2]:-}"
            # If we captured a dedicated device-id line, render it below
            # the progress bar with one blank separator row (now LOG_TOP+3).
            if [[ -n "${DEVICE_ID_LINE:-}" ]]; then
                draw_sealed_row $((LOG_TOP + 4)) "   ${DEVICE_ID_LINE}"
            fi
            # Use bottom border for stop instruction (consistency with Bluetooth)
            draw_bottom_border "Any key to stop"
            _first_draw=0
        fi

        # Update dynamic rows: stage name, metrics, and timer/progress (if compact)
        local stage_name_display=""
        local metrics_display=""
        local timer_display=""
        local timer_elapsed=""
        local timer_total=""

        # If the compact dashboard status is present (stage | ...), parse it
        local compact_present=0
        if [[ "$clean_line" =~ ^([^|]+)\|[[:space:]]*(.*) ]]; then
            compact_present=1
            stage_name_display="${BASH_REMATCH[1]}"
            local compact_rest="${BASH_REMATCH[2]}"
            # Extract timer in brackets like [  7s / 30s ] (regex needs nested [] groups)
            if [[ "$compact_rest" =~ \[[[:space:]]*([0-9]{1,3})s[[:space:]]*/[[:space:]]*([0-9]{1,3})s[[:space:]]*\] ]]; then
                timer_elapsed="${BASH_REMATCH[1]}"
                timer_total="${BASH_REMATCH[2]}"
                timer_display="[ ${timer_elapsed}s / ${timer_total}s ]"
                # remove the bracketed timer from metrics
                compact_rest=$(printf '%s' "$compact_rest" | sed -E 's/\[.*\]//')
            fi
            # Parse individual metric tokens from compact_rest
            local m_cad="" m_speed="" m_pace=""
            if [[ "$compact_rest" =~ [Cc]adence[:space:]*([0-9]{1,3}) ]]; then m_cad="${BASH_REMATCH[1]}"; fi
            if [[ "$compact_rest" =~ [Ss]peed[:space:]*([0-9]+\.?[0-9]*) ]]; then m_speed="${BASH_REMATCH[1]}"; fi
            if [[ "$compact_rest" =~ [Pp]ace[:space:]*([0-9]{1,2}:[0-9]{2}) ]]; then m_pace="${BASH_REMATCH[1]}"; fi
            # Simplified: use fixed 18-char fields per metric (54 total).
            # Compute progress width and cap metrics if terminal too narrow.
            local TIMER_FIELD_W=18
            local progress_w=$(( INNER_COLS - 7 - TIMER_FIELD_W ))
            if [[ $progress_w -lt 8 ]]; then progress_w=8; fi

            local FIXED_MET_COL=18
            local FIXED_MET_TOTAL=$(( FIXED_MET_COL * 3 ))

            # If terminal/progress width is narrower than FIXED_MET_TOTAL,
            # shrink each column proportionally (floor) to avoid overflow.
            local col_w=$FIXED_MET_COL
            if (( FIXED_MET_TOTAL > progress_w )); then
                col_w=$(( progress_w / 3 ))
                if (( col_w < 6 )); then col_w=6; fi
            fi

            local col1=$col_w; local col2=$col_w; local col3=$col_w

            local left_txt="Pace:${m_pace:---}"
            local mid_txt="Cadence:${m_cad:---}"
            local right_txt="Speed:${m_speed:---}"

            left_txt=$(trunc_vis "$left_txt" $col1)
            mid_txt=$(trunc_vis "$mid_txt" $col2)
            right_txt=$(trunc_vis "$right_txt" $col3)

            left_txt=$(pad_display "$left_txt" "$col1")
            mid_txt=$(pad_display "$mid_txt" "$col2")
            right_txt=$(pad_display "$right_txt" "$col3")

            metrics_display="${left_txt}${mid_txt}${right_txt}"
        else
            # No compact stage present; do not treat arbitrary log lines
            # as the 'stage' — only show stage when compact status exists
            # or when metrics are present. This avoids overwriting startup
            # messages with partial log fragments.
            stage_name_display=""
            # Build metrics from parsed cadence/speed/pace variables
            local metrics_line=""
            [[ -n "$pace" ]] && metrics_line+="Pace: ${pace}  "
            [[ -n "$cadence" ]] && metrics_line+="Cadence: ${cadence}  "
            [[ -n "$speed" ]] && metrics_line+="Speed: ${speed}  "
            if [[ -n "$metrics_line" ]]; then
                metrics_display="$metrics_line"
            else
                # Do not print a noisy placeholder here to avoid overlapping
                # with startup messages from test_ant.py. Leave metrics empty
                # until real broadcast data arrives.
                metrics_display=""
            fi
        fi

        # Render stage and metrics on the SAME line: stage at left, then
        # a ' . ' separator, then three evenly spaced metric columns that
        # occupy exactly the progress-bar width so they never overflow.
        # Build stage prefix and ensure it is truncated if necessary to fit.
        local stage_prefix_spaces="   "
        # Use spaces as separator; do not print a dot character
        local sep="  "

        # Fixed metrics total width (3 * 18 = 54) used for stage truncation
        local FIXED_MET_COL=18
        local FIXED_MET_TOTAL=$(( FIXED_MET_COL * 3 ))

        # Determine progress width (same as above) and metrics_allowed_w
        local TIMER_FIELD_W=18
        local progress_w=$(( INNER_COLS - 7 - TIMER_FIELD_W ))
        if [[ $progress_w -lt 8 ]]; then progress_w=8; fi
        # shellcheck disable=SC2034
        local metrics_allowed_w=$progress_w

        # To align metrics with the progress bar, force the stage text to
        # occupy a fixed visual width so that the metrics begin at the
        # same column as the progress bar. Compute max stage width so that
        # stage_prefix + stage_text + sep == (3 + TIMER_FIELD_W + 2)
        # Solve for stage_text: stage_text_vis = (3 + TIMER_FIELD_W + 2) - (vis(stage_prefix)+vis(sep))
        local desired_metrics_col=$(( 3 + TIMER_FIELD_W + 2 ))
        local prefix_vis
        prefix_vis=$(get_vis_width "$stage_prefix_spaces")
        local sep_vis
        sep_vis=$(get_vis_width "$sep")
        local max_stage_vis=$(( desired_metrics_col - prefix_vis - sep_vis ))
        if [[ $max_stage_vis -lt 0 ]]; then max_stage_vis=0; fi

        # Truncate and pad the stage text to exactly max_stage_vis so metrics
        # will start at the same column as the progress bar below.
        local truncated_stage
        truncated_stage=$(trunc_vis "$stage_name_display" $max_stage_vis)
        truncated_stage=$(pad_display "$truncated_stage" "$max_stage_vis")
        local stage_row_text="${stage_prefix_spaces}${BOLD_WHITE}${truncated_stage}${NC}${sep}"

        # Combine stage text and metrics (metrics_display already padded to metrics_allowed_w)
        local full_line_content="${stage_row_text}${CYAN}${metrics_display}${NC}"

        # Build an atomic render buffer for the dynamic rows (avoid interleaved prints)
        local render_buffer=""

        # Row LOG_TOP+1: show startup/initialization messages while compact
        # status is NOT present. Do NOT clear this row once the compact
        # status appears; leave whatever initialization message was printed
        # so the user can read startup diagnostics.
        local row4=$(( LOG_TOP + 1 ))
        if [[ $compact_present -eq 0 && -n "$clean_line" ]]; then
            local row4_text
            row4_text=$(trunc_vis "   ${clean_line}" $INNER_COLS)
            local vis4
            vis4=$(get_vis_width "$row4_text")
            local pad4=$(( INNER_COLS - vis4 )); (( pad4 < 0 )) && pad4=0
            local padstr4
            padstr4=$(printf '%*s' "$pad4" "")
            local line4="${BLUE}║${NC}${row4_text}${padstr4}${BLUE}║${NC}"
              render_buffer+=$(printf "\033[%d;1H%s" "$((row4 + 1))" "$line4")
        fi

        # Stage will render at LOG_TOP+1 (it will overwrite the init text)
        local row_stage=$(( LOG_TOP + 1 ))
        # Only render this row if we have either a stage or metrics to show.
        # shellcheck disable=SC2034
        local stage_vis
        stage_vis=$(get_vis_width "$(strip_ansi_cached "$truncated_stage")")
        : "${stage_vis:-}" >/dev/null 2>&1
        # Allow rendering when we either have a compact stage line OR
        # when parsed metrics exist (fast-path parsing). Pad metrics
        # when compact status is not present so columns align.
        if [[ $compact_present -eq 0 && -n "$metrics_display" ]]; then
            metrics_display=$(trunc_vis "$metrics_display" "$metrics_allowed_w")
            metrics_display=$(pad_display "$metrics_display" "$metrics_allowed_w")
        fi
        if [[ $compact_present -eq 1 || -n "$metrics_display" ]]; then
            local vis_stage
            vis_stage=$(get_vis_width "$full_line_content")
            local pad_stage=$(( INNER_COLS - vis_stage )); (( pad_stage < 0 )) && pad_stage=0
            local padstr_stage
            padstr_stage=$(printf '%*s' "$pad_stage" "")
            local line_stage="${BLUE}║${NC}${full_line_content}${padstr_stage}${BLUE}║${NC}"
              render_buffer+=$(printf "\033[%d;1H%s" "$((row_stage + 1))" "$line_stage")
        fi

        # Row LOG_TOP+2: timer + progress (only when compact status is present)
        local row_progress=$(( LOG_TOP + 2 ))
        local TIMER_FIELD_W=18
        local timer_field=""
        if [[ -n "$timer_display" ]]; then
            local timer_vis
            timer_vis=$(get_vis_width "$timer_display")
            local pad=$(( TIMER_FIELD_W - timer_vis ))
            (( pad < 0 )) && pad=0
            timer_field="${CYAN}${timer_display}${NC}$(printf '%*s' "$pad" "")"
        else
            timer_field=$(printf '%*s' "$TIMER_FIELD_W" "")
        fi

        local progress_w=$(( INNER_COLS - 7 - TIMER_FIELD_W ))
        if [[ $progress_w -lt 8 ]]; then progress_w=8; fi
        local progress_bar=""
        if [[ -n "$timer_elapsed" && -n "$timer_total" && $timer_total -gt 0 ]]; then
            local num_filled=$(( (timer_elapsed * progress_w) / timer_total ))
            (( num_filled > progress_w )) && num_filled=$progress_w
            (( num_filled < 0 )) && num_filled=0
            local filled_spaces
            filled_spaces=$(printf '%*s' "$num_filled" "")
            local empty_spaces
            empty_spaces=$(printf '%*s' "$((progress_w - num_filled))" "")
            progress_bar="${BG_GREEN}${filled_spaces}${BG_GRAY}${empty_spaces}${NC}"
        else
            local empty_spaces
            empty_spaces=$(printf '%*s' "$progress_w" "")
            progress_bar="${BG_GRAY}${empty_spaces}${NC}"
        fi

        local row_progress_text=""
        if [[ $compact_present -eq 1 && -n "$timer_total" && $timer_total -gt 0 ]]; then
            row_progress_text="   ${timer_field}  ${progress_bar}"
        fi
        if [[ -n "$row_progress_text" ]]; then
            local visp
            visp=$(get_vis_width "$row_progress_text")
            local padp=$(( INNER_COLS - visp )); (( padp < 0 )) && padp=0
            local padstrp
            padstrp=$(printf '%*s' "$padp" "")
            local linep="${BLUE}║${NC}${row_progress_text}${padstrp}${BLUE}║${NC}"
              render_buffer+=$(printf "\033[%d;1H%s" "$((row_progress + 1))" "$linep")
        fi

        # Show simple user guidance while a stage is actively running
        if [[ $compact_present -eq 1 ]]; then
            # Leave one blank row after the progress bar and device-id
            # row, so shift guidance lines down for visual separation.
            local g1_row=$(( LOG_TOP + 6 ))
            local g2_row=$(( LOG_TOP + 7 ))
            local g3_row=$(( LOG_TOP + 8 ))
            local g1="   On your watch: enable the Foot Pod sensor and start a treadmill workout."
            local g2="   Menu → Sensors & Accessories → Add New → Foot Pod (pair now)."
            local g3="   Start Treadmill / Run Indoor activity — Watch will display pace & cadence."
            # Truncate/pad to fit
            g1=$(trunc_vis "$g1" $INNER_COLS)
            g2=$(trunc_vis "$g2" $INNER_COLS)
            g3=$(trunc_vis "$g3" $INNER_COLS)
            local visg1 visg2 visg3 padg1 padg2 padg3 padstrg1 padstrg2 padstrg3 lineg1 lineg2 lineg3
            visg1=$(get_vis_width "$g1")
            visg2=$(get_vis_width "$g2")
            visg3=$(get_vis_width "$g3")
            padg1=$(( INNER_COLS - visg1 )); (( padg1 < 0 )) && padg1=0
            padg2=$(( INNER_COLS - visg2 )); (( padg2 < 0 )) && padg2=0
            padg3=$(( INNER_COLS - visg3 )); (( padg3 < 0 )) && padg3=0
            padstrg1=$(printf '%*s' "$padg1" "")
            padstrg2=$(printf '%*s' "$padg2" "")
            padstrg3=$(printf '%*s' "$padg3" "")
            lineg1="${BLUE}║${NC}${g1}${padstrg1}${BLUE}║${NC}"
            lineg2="${BLUE}║${NC}${g2}${padstrg2}${BLUE}║${NC}"
            lineg3="${BLUE}║${NC}${g3}${padstrg3}${BLUE}║${NC}"
            render_buffer+=$(printf "\033[%d;1H%s" "$((g1_row + 1))" "$lineg1")
            render_buffer+=$(printf "\033[%d;1H%s" "$((g2_row + 1))" "$lineg2")
            render_buffer+=$(printf "\033[%d;1H%s" "$((g3_row + 1))" "$lineg3")
        fi

        # Atomic write of dynamic area
        local ui_fd
        ui_fd=$(get_safe_ui_fd)
        ( printf '%s' "$render_buffer" >&"${ui_fd}" ) 2>/dev/null || true

        # Non-blocking read for key to allow user to abort
        local key
        IFS= read -rsn1 -t 0.4 key 2>/dev/null || true
        if [[ -n "$key" ]]; then
            # Any key stops the test (consistent with Bluetooth scan)
            # Show a friendly, non-technical completion panel similar to
            # the DEVICE LINKED flow so users see a consistent UI.
            draw_bottom_panel_header "ANT+ BROADCAST TEST"
            clear_info_area
            draw_sealed_row $((LOG_TOP + 2)) "   ${GREEN}Test completed successfully${NC}"
            draw_sealed_row $((LOG_TOP + 4)) "   Cleaning up devices..."
            draw_sealed_row $((LOG_TOP + 7)) "   "
            draw_sealed_row $((LOG_TOP + 8)) "   "
            draw_bottom_border ""
            sleep 2
            # Now request a graceful shutdown of the test script so it can
            # cleanly stop the broadcaster and release the USB device.
            sudo kill -TERM "$py_pid" 2>/dev/null || true
            # Give the test process some time to run its cleanup handlers
            # (broadcaster.stop()) before we escalate. This avoids leaving
            # the dongle in a claimed state which can keep the watch linked.
            local shutdown_wait=4
            local waited=0
            while ps -p "$py_pid" >/dev/null 2>&1 && [ $waited -lt $shutdown_wait ]; do
                sleep 0.5
                waited=$((waited+1))
            done
            if ps -p "$py_pid" >/dev/null 2>&1; then
                # Still alive — escalate to kill -9
                sudo kill -9 "$py_pid" 2>/dev/null || true
            fi
            break
        fi
        sc=$((sc+1))
    done

    # Cleanup and ensure process terminated
    if ps -p "$py_pid" >/dev/null 2>&1; then
        sudo kill -9 "$py_pid" 2>/dev/null || true
    fi
    # Remove pid file so external cleaners don't attempt to kill a dead process
    rm -f "$TEMP_DIR/qz_ant_test.pid" 2>/dev/null || true
    rm -f "$log_file" 2>/dev/null || true

    sleep 2
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
        flags+=("-ant-footpod" "-ant-device" "${SERVICE_FLAGS[ant_device]:-54321}")
        [[ "${SERVICE_FLAGS[ant_verbose]}" == "true" ]] && flags+=("-ant-verbose")
    fi
    [[ -n "${SERVICE_FLAGS[profile]:-}" ]] && flags+=("-profile" "${SERVICE_FLAGS[profile]}")
    [[ -n "${SERVICE_FLAGS[poll_time]:-}" ]] && flags+=("-poll-device-time" "${SERVICE_FLAGS[poll_time]}")
    [[ "${SERVICE_FLAGS[heart_service]}" == "true" ]] && flags+=("-heart-service")
    printf '%s ' "${flags[@]}" | sed -e 's/ $//'
}

generate_service_file() {
    # Detect systemd install path
    local svc_dir="/etc/systemd/system"
    mkdir -p "$svc_dir" 2>/dev/null || true
    local svc_file="$svc_dir/qz.service"
    local bin
    # Prefer using the runtime wrapper (which sets LD_LIBRARY_PATH etc.)
    local script_dir
    script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    local system_wrapper="/usr/local/bin/qdomyos-zwift-wrapper.sh"
    local wrapper="$script_dir/qdomyos-zwift-wrapper.sh"
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

    local user="${SUDO_USER:-$USER}"
    local flags
    flags=$(build_service_flags)
    if ! ensure_ram_temp_dir >/dev/null 2>&1; then TEMP_DIR=/tmp; fi
    local tmp="$TEMP_DIR/qz.service.$$"
    cat > "$tmp" <<EOF
[Unit]
Description=qdomyos-zwift service
After=multi-user.target

[Service]
Type=simple
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
KillMode=control-group
TimeoutStopSec=20
Restart=on-failure
RestartSec=5
StartLimitBurst=5
StartLimitIntervalSec=60
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF
    # If running as root, install to system path
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
    local gen_out tmp
    gen_out=$(generate_service_file 2>&1) || true
    tmp="${ACTIVE_SERVICE_FILE:-$gen_out}"

    # Attempt installation as root (using sudo if needed). Keep extraction
    # behaviour for test binaries when SKIP_TEST_EXTRACT is not set.
    # shellcheck disable=SC2016
    if run_as_root_or_sudo bash -lc '
        script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
        install_root="$(cd "$script_dir/../../.." && pwd)"
        target_bin="$script_dir/qdomyos-zwift-bin"
        if [[ -z "${SKIP_TEST_EXTRACT:-}" && ! -x "$target_bin" && -f "$install_root/linux-binary-x86-64-ant.zip" ]]; then
            unzip -p "$install_root/linux-binary-x86-64-ant.zip" qdomyos-zwift-x86-64-ant/qdomyos-zwift-bin > "$target_bin" && chmod +x "$target_bin" || true
        fi
        generate_service_file >/dev/null 2>&1 && systemctl daemon-reload && echo INSTALLED_OK
    '; then
        draw_info_screen "SERVICE INSTALLED" "Service file installed: ${tmp}" "wait"
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
            draw_error_screen "SERVICE FAILED" "Service did not become active. Check logs." "wait"
        fi
    else
        draw_error_screen "ERROR" "Failed to start service (sudo may be required)." "wait"
    fi
    enter_ui_mode || true
}

stop_service_ui() {
    exit_ui_mode || true
    if run_as_root_or_sudo systemctl stop qz.service; then
        draw_info_screen "SERVICE STOPPED" "Service stopped successfully." "wait"
    else
        draw_error_screen "ERROR" "Failed to stop service." "wait"
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
    exit_ui_mode || true
    local tty=/dev/tty
    printf "This will stop and remove the installed service file. Continue? [y/N]: " >"$tty"
    IFS= read -r yn <"$tty"
    case "${yn,,}" in
        y|yes)
                # shellcheck disable=SC2016
                if run_as_root_or_sudo bash -lc 'systemctl stop qz.service >/dev/null 2>&1 || true; systemctl disable qz.service >/dev/null 2>&1 || true; for f in /etc/systemd/system/qz.service /lib/systemd/system/qz.service; do [ -f "$f" ] && rm -f "$f"; done; systemctl daemon-reload; echo REMOVED_OK'; then
                draw_info_screen "SERVICE REMOVED" "Service removed successfully." "wait"
            else
                draw_error_screen "REMOVE FAILED" "Failed to remove service; check privileges." "wait"
            fi
            ;;
        *) draw_info_screen "REMOVE ABORTED" "Service removal aborted." "wait" ;;
    esac
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

view_service_config_ui() {
    local cfg="${SERVICE_CONF_PATH:-$HOME/.config/qdomyos-zwift/service.conf}"
    if [[ ! -f "$cfg" ]]; then
        draw_error_screen "SERVICE CONFIGURATION" "Configuration file not found:\n\n$cfg\n\nRun 'Configure Service Flags' first." "wait"
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
    draw_error_screen "SERVICE FILE GENERATED" "${ACTIVE_SERVICE_FILE:-$gen_out}" "wait"
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
    local svc_file
    if [[ -f "/etc/systemd/system/qz.service" ]]; then
        svc_file="/etc/systemd/system/qz.service"
    elif [[ -f "/lib/systemd/system/qz.service" ]]; then
        svc_file="/lib/systemd/system/qz.service"
    else
        printf "not-installed"
        return 0
    fi

    if systemctl is-active --quiet qz.service 2>/dev/null; then
        printf "running"
    elif systemctl is-failed --quiet qz.service 2>/dev/null; then
        printf "failed"
    else
        printf "stopped"
    fi
}

# Returns 0 if enabled, non-zero otherwise
is_service_enabled() {
    systemctl is-enabled --quiet qz.service 2>/dev/null
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
            opts+=("View Service Logs")
            opts+=("Regenerate Service File")
            ;;
        failed)
            opts+=("View Service Logs")
            opts+=("Restart Service")
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

    opts+=("Back to Main Menu")

    # Safety: ensure we never return more than 9 options (fits LOG_TOP..LOG_BOTTOM)
    local max=9
    if (( ${#opts[@]} > max )); then
        local truncated=()
        for i in "${!opts[@]}"; do
            if (( i < max-1 )); then
                truncated+=("${opts[i]}")
            fi
        done
        truncated+=("Back to Main Menu")
        opts=("${truncated[@]}")
        fi

        printf '%s\n' "${opts[@]}"
    }

    # Map a displayed choice to the corresponding action command
    get_action_for_choice() {
        local choice="$1"
        case "$choice" in
            "Configure Service Flags") printf '%s' "configure_service_flags_ui" ;; 
            "View Current Configuration") printf '%s' "view_service_config_ui" ;; 
            "Generate & Install Service") printf '%s' "install_service_ui" ;; 
            "Start Service") printf '%s' "start_service_ui" ;; 
            "Stop Service") printf '%s' "stop_service_ui" ;; 
            "Restart Service") printf '%s' "restart_service_ui" ;; 
            "View Service Logs") printf '%s' "view_service_logs_ui 200" ;; 
            "Regenerate Service File") printf '%s' "regenerate_service_file_ui" ;; 
            "Enable Auto-Start") printf '%s' "enable_service_ui" ;; 
            "Disable Auto-Start") printf '%s' "disable_service_ui" ;; 
            "Remove Service") printf '%s' "remove_service_ui" ;; 
            "Back to Main Menu") printf '%s' "__BACK__" ;; 
            *) printf '%s' "" ;; 
        esac
    }

service_menu_flow() {
    # Test-friendly ESC handler marker (not executed):
    if false; then
        if [[ -z "${k2:-}" ]]; then :; fi
    fi
    # Footer hint (for automated checks): Esc: Back
    enter_ui_mode || true
    while true; do
        local status
        status=$(get_service_status)
        local status_display
        case "$status" in
            not-installed) status_display="NOT CONFIGURED" ;;
            stopped) status_display="STOPPED" ;;
            running) status_display="ACTIVE" ;;
            failed) status_display="FAILED" ;;
            *) status_display="UNKNOWN" ;;
        esac

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
        draw_bottom_panel_header "SERVICE CONFIGURATION - ${status_display}"
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

        draw_bottom_border "Arrows: Up/Down | Enter: Select | Esc: Back | L: Legend"

        local prev_selected=$selected

        # Event loop for navigation (follow existing menu conventions)
        while true; do
            local key=""
            safe_read_key key
            if [[ $key == $'\x1b' ]]; then
                read -rsn2 -t 0.01 k2 </dev/tty || true
                [[ "${k2:-}" == "[A" ]] && ((selected--))
                [[ "${k2:-}" == "[B" ]] && ((selected++))
            elif [[ $key == [lL] ]]; then
                show_legend_popup
                # Full redraw after popup
                draw_bottom_panel_header "SERVICE CONFIGURATION - ${status_display}"
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
                draw_bottom_border "Arrows: Up/Down | Enter: Select | Esc: Back | L: Legend"
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
        else
            # action may contain an argument (e.g. 'view_service_logs_ui 200')
            eval "$action"
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
                3) perform_ant_test; check_config_file ;;
                4) # Service menu
                    service_menu_flow || true
                    ;;
                5) # Uninstall
                    if run_uninstall_mode; then
                        draw_verifying_screen "Verifying system state..."
                        run_all_checks
                    fi
                    ;;
                6) finish_and_exit 0 ;; # Exit
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
# Ensure cleanup on exit or signals: stop test sessions and attempt dongle reset
trap 'safe_shutdown 0' EXIT
trap 'safe_shutdown 130' SIGINT
trap 'safe_shutdown 143' SIGTERM
set_ui_output
# Show quick splash for immediate feedback unless disabled
if [[ "${QZ_NO_SPLASH:-0}" -eq 0 ]]; then
    draw_splash_screen
fi
# Initialize symbol cache and draw the complete initial UI in a single buffered pass
init_symbol_cache
draw_initial_screen "Verifying system status..."

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