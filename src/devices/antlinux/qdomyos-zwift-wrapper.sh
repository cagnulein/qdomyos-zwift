#!/bin/bash
# shellcheck disable=SC2034,SC2154
# shellcheck enable=require-variable-braces
################################################################################
# QDomyos-Zwift: Universal Runtime Wrapper Script (Multi-Architecture)
#
# Version: 3.1.1 - Added QML subdirectory support
################################################################################

declare -r QZ_WRAPPER_VERSION="3.1.1"

set -e
# Resolve canonical script path — follows symlinks so DIR is the real on-disk
# location regardless of how the script is invoked or what the directory is named
# (arm64-ant, aarch64-ant, x86-64-ant, etc.)
_SCRIPT_PATH="$(readlink -f "${BASH_SOURCE[0]}" 2>/dev/null || realpath "${BASH_SOURCE[0]}" 2>/dev/null)"
if [[ -z "$_SCRIPT_PATH" ]]; then
    _SCRIPT_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)/$(basename "${BASH_SOURCE[0]}")"
fi
DIR="$(dirname "$_SCRIPT_PATH")"

declare -r C_RED="\033[0;31m"
declare -r C_YELLOW="\033[1;33m"
declare -r C_BLUE="\033[0;34m"
declare -r C_GREEN="\033[0;32m"
declare -r C_RESET="\033[0m"

ERRORS=()

################################################################################
# SHARED BLOCK: Configuration Paths 
################################################################################
declare -r QZ_CONFIG_NAMESPACE="Roberto Viola"
declare -r QZ_CONFIG_FILE="qDomyos-Zwift.conf"

get_config_path() {
    local user_home=${1:-$HOME}
    echo "${user_home}/.config/${QZ_CONFIG_NAMESPACE}/${QZ_CONFIG_FILE}"
}

################################################################################
# OS Version Check (Ubuntu 24.04+ / Debian 12+ required)
################################################################################
check_os_version() {
    if [[ ! -f /etc/os-release ]]; then
        ERRORS+=("Cannot detect OS version - /etc/os-release missing")
        return 1
    fi
    
    source /etc/os-release
    local os_id="${ID}"
    local os_version="${VERSION_ID}"
    local os_name="${NAME} ${VERSION_ID}"
    
    # Ubuntu 24.04+ required
    if [[ "$os_id" == "ubuntu" ]]; then
        local major_version="${os_version%%.*}"
        if [[ "$major_version" -lt 24 ]]; then
            echo ""
            echo -e "${C_RED}╔════════════════════════════════════════════════════════════════════╗${C_RESET}"
            echo -e "${C_RED}║  ERROR: Unsupported Ubuntu Version                                ║${C_RESET}"
            echo -e "${C_RED}╚════════════════════════════════════════════════════════════════════╝${C_RESET}"
            echo ""
            echo -e "${C_YELLOW}Your system:${C_RESET} $os_name"
            echo -e "${C_YELLOW}Minimum required:${C_RESET} Ubuntu 24.04 LTS"
            echo ""
            echo "QDomyos-Zwift v3.0+ requires Ubuntu 24.04 or later."
            echo ""
            exit 1
        fi
    # Debian 12+ required
    elif [[ "$os_id" == "debian" ]] || [[ "$os_id" == "raspbian" ]]; then
        local major_version="${os_version%%.*}"
        if [[ -n "$major_version" ]] && [[ "$major_version" -lt 12 ]]; then
            echo ""
            echo -e "${C_RED}╔════════════════════════════════════════════════════════════════════╗${C_RESET}"
            echo -e "${C_RED}║  ERROR: Unsupported Debian/Raspberry Pi OS Version                ║${C_RESET}"
            echo -e "${C_RED}╚════════════════════════════════════════════════════════════════════╝${C_RESET}"
            echo ""
            echo -e "${C_YELLOW}Your system:${C_RESET} $os_name"
            echo -e "${C_YELLOW}Minimum required:${C_RESET} Debian 12 (Bookworm)"
            echo ""
            exit 1
        fi
    fi
    
    return 0
}

################################################################################
# Architecture Detection
################################################################################
ARCH_TYPE="$(uname -m)"
case "$ARCH_TYPE" in
    x86_64|amd64)
        PLATFORM="x86-64"
        ARCH_LIB_PATH="x86_64-linux-gnu"
        ;;
    aarch64|arm64)
        PLATFORM="ARM64"
        ARCH_LIB_PATH="aarch64-linux-gnu"
        ;;
    *)
        echo -e "${C_RED}ERROR: Unsupported architecture: $ARCH_TYPE${C_RESET}" >&2
        exit 1
        ;;
esac

################################################################################
# User & Home Resolution
################################################################################
if [[ -n "${SUDO_USER:-}" ]]; then
    TARGET_USER="$SUDO_USER"
    USER_HOME=$(getent passwd "${SUDO_USER}" | cut -d: -f6)
else
    TARGET_USER="${USER:-$(whoami)}"
    USER_HOME="${HOME:-$(getent passwd "$TARGET_USER" | cut -d: -f6)}"
fi

if ! id "$TARGET_USER" >/dev/null 2>&1; then
    echo -e "${C_RED}ERROR: User '$TARGET_USER' does not exist${C_RESET}" >&2
    exit 1
fi

if [[ ! -d "$USER_HOME" ]]; then
    echo -e "${C_RED}ERROR: Home directory '$USER_HOME' not found${C_RESET}" >&2
    exit 1
fi

################################################################################
# System Qt5 Check
################################################################################
check_system_qt5() {
    local qt_missing=()

    # --- .so library check (unchanged) ---
    for lib in libQt5Core.so.5 libQt5Qml.so.5 libQt5Quick.so.5 libQt5Widgets.so.5 libQt5Bluetooth.so.5; do
        if ! ldconfig -p 2>/dev/null | grep -q "$lib"; then
            qt_missing+=("$lib")
        fi
    done

    if [[ ${#qt_missing[@]} -gt 0 ]]; then
        echo ""
        echo -e "${C_RED}╔════════════════════════════════════════════════════════════════════╗${C_RESET}"
        echo -e "${C_RED}║  ERROR: Qt5 Libraries Not Installed                               ║${C_RESET}"
        echo -e "${C_RED}╚════════════════════════════════════════════════════════════════════╝${C_RESET}"
        echo ""
        echo "Missing Qt5 libraries:"
        for lib in "${qt_missing[@]}"; do
            echo "  ✗ $lib"
        done
        echo ""
        echo "Install with: sudo apt-get install qtbase5-dev libqt5qml5 libqt5quick5 libqt5widgets5"
        echo ""
        exit 1
    fi

    # --- QML module check (delegated to shared script, GUI mode only) ---
    # Skipped when _SKIP_QML=true (headless, -no-gui, or QZ_SKIP_QML_CHECK=1)
    if [[ "${_SKIP_QML:-false}" == "false" ]]; then
        local dep_script="$DIR/check-qml-deps.sh"
        if [[ -f "$dep_script" ]]; then
            local qml_errors
            if ! qml_errors=$("$dep_script" --binary "$DIR/qdomyos-zwift-bin" --fix 2>&1); then
                echo ""
                echo -e "${C_RED}╔════════════════════════════════════════════════════════════════════╗${C_RESET}"
                echo -e "${C_RED}║  ERROR: QML Runtime Modules Not Installed                         ║${C_RESET}"
                echo -e "${C_RED}╚════════════════════════════════════════════════════════════════════╝${C_RESET}"
                echo ""
                echo "$qml_errors"
                echo ""
                echo "Run 'sudo ./setup-dashboard.sh' to install missing modules."
                echo "Or set QZ_SKIP_QML_CHECK=1 to bypass (headless/ANT+ only mode)."
                echo ""
                exit 1
            fi
        fi
    fi

    return 0
}

################################################################################
# Python Check
################################################################################
check_python() {
    local python_found=false
    
    if command -v python3.12 >/dev/null 2>&1; then
        python_found=true
    elif command -v python3.11 >/dev/null 2>&1; then
        python_found=true
    elif command -v python3 >/dev/null 2>&1; then
        local py_version=$(python3 --version 2>&1 | awk '{print $2}')
        if [[ "$py_version" =~ ^3\.(11|12|13) ]]; then
            python_found=true
        fi
    fi
    
    if [[ "$python_found" == "false" ]]; then
        ERRORS+=("Python 3.11+ not found")
    fi
}

################################################################################
# Binary Check
################################################################################
if [[ ! -x "$DIR/qdomyos-zwift-bin" ]]; then
    ERRORS+=("Binary not found or not executable: $DIR/qdomyos-zwift-bin")
fi

################################################################################
# Detect GUI Mode from Arguments
################################################################################
GUI_MODE=false
for arg in "$@"; do
    if [[ "$arg" == "-gui" ]]; then
        GUI_MODE=true
        break
    elif [[ "$arg" == "-no-gui" ]]; then
        GUI_MODE=false
        break
    fi
done

# If no explicit mode, assume GUI if DISPLAY is set
if [[ "$GUI_MODE" == "false" ]] && [[ -z "$(printf '%s\n' "$@" | grep -E '^-no-gui$')" ]]; then
    if [[ -n "${DISPLAY:-}" ]] || command -v Xorg >/dev/null 2>&1; then
        GUI_MODE=true
    fi
fi

################################################################################
# QML Skip Detection
#
# Skip check-qml-deps.sh when any of these is true:
#   a) -no-gui appears in the argument list  (service/headless mode)
#   b) QZ_SKIP_QML_CHECK=1 set in environment (explicit override)
#   c) Neither DISPLAY nor WAYLAND_DISPLAY is set (headless environment)
#
# QML modules are only needed for GUI mode — headless ANT+ operation does not
# use QML at all, so failing here before the binary even starts is wrong.
################################################################################
_SKIP_QML=false
_NO_GUI_ARG=false
for _a in "$@"; do
    [[ "$_a" == "-no-gui" ]] && { _NO_GUI_ARG=true; break; }
done

if [[ "${QZ_SKIP_QML_CHECK:-0}" == "1" ]] || \
   [[ "$_NO_GUI_ARG" == "true" ]] || \
   [[ -z "${DISPLAY:-}" && -z "${WAYLAND_DISPLAY:-}" ]]; then
    _SKIP_QML=true
fi

################################################################################
# Display Authorization
################################################################################
if [[ "$EUID" -eq 0 ]] && [[ -n "${DISPLAY:-}" ]]; then
    xhost +local:root >/dev/null 2>&1 || true
fi

################################################################################
# Run All Checks
################################################################################
check_os_version
check_system_qt5
check_python

if [[ ${#ERRORS[@]} -gt 0 ]]; then
    echo -e "${C_RED}ERROR: Cannot start QDomyos-Zwift${C_RESET}" >&2
    for err in "${ERRORS[@]}"; do 
        echo -e "${C_RED}  ✗ $err${C_RESET}" >&2
    done
    exit 1
fi

################################################################################
# Launch Binary
################################################################################
if [[ "$EUID" -eq 0 ]]; then
    QZ_RUNTIME_DIR="/tmp/runtime-root-qz"
    mkdir -p "$QZ_RUNTIME_DIR"
    chmod 0700 "$QZ_RUNTIME_DIR"
else
    QZ_RUNTIME_DIR="/run/user/$(id -u)"
fi

RESOLVED_XAUTH="${XAUTHORITY:-${USER_HOME}/.Xauthority}"
if [[ -f "$RESOLVED_XAUTH" ]]; then
    chmod 644 "$RESOLVED_XAUTH" 2>/dev/null || true
fi

# Build environment with QML path if using subdirectory
LAUNCH_ENV=(
    HOME="$USER_HOME"
    USER="${TARGET_USER}"
    LOGNAME="${TARGET_USER}"
    PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
    DISPLAY="${DISPLAY:-:0}"
    XAUTHORITY="$RESOLVED_XAUTH"
    XDG_RUNTIME_DIR="$QZ_RUNTIME_DIR"
    DBUS_SESSION_BUS_ADDRESS="${DBUS_SESSION_BUS_ADDRESS:-}"
    QZ_USER="${QZ_USER:-}"
    LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-}"
    PYTHONHOME="${PYTHONHOME:-}"
)

# Add QML path if using subdirectory structure
if [[ -n "${QML2_IMPORT_PATH:-}" ]]; then
    LAUNCH_ENV+=("QML2_IMPORT_PATH=$QML2_IMPORT_PATH")
fi

# exec replaces this shell process so signals (SIGINT, SIGTERM, SIGQUIT) go
# directly to qdomyos-zwift-bin — required for clean systemctl stop shutdown
# via KillSignal=SIGINT in the systemd unit (bash was absorbing signals before)
exec env -i "${LAUNCH_ENV[@]}" "$DIR/qdomyos-zwift-bin" "$@"