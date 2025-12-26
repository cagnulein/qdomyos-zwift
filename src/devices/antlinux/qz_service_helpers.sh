#!/usr/bin/env bash
# Non-interactive helpers test — isolates only the service helper functions
set -euo pipefail

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

    # Safety: ensure we never return more than 9 options
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

    printf '%s
' "${opts[@]}"
}

# Print outputs
echo "get_service_status: $(get_service_status 2>/dev/null || echo unknown)"
if is_service_enabled; then echo "is_service_enabled: yes"; else echo "is_service_enabled: no"; fi

echo "build_service_menu_options:" 
build_service_menu_options | sed -n '1,50p'
