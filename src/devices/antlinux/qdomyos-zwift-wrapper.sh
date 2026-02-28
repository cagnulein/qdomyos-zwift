#!/bin/bash
# shellcheck disable=SC2034,SC2154
# shellcheck enable=require-variable-braces
################################################################################
# QDomyos-Zwift: Universal Runtime Wrapper Script (Multi-Architecture)
#
# Part of QDomyos-Zwift project: https://github.com/cagnulein/qdomyos-zwift
# Contributor: bassai-sho | AI-assisted development | License: GPL-3.0
#
# Runtime validation and dependency checker for QDomyos-Zwift with ANT+ support.
# Auto-detects system architecture (x86-64/ARM64), validates Python 3.11, Qt5,
# and ANT+ dependencies, then launches the binary in a clean environment to
# prevent snap/flatpak library conflicts.
#
# Platform: Multi-arch Linux (x86-64: Ubuntu 20.04+, Debian 11+)
#                            (ARM64: Raspberry Pi 3+, Debian/RPi OS)
# Key checks: Architecture validation, Python 3.11 libs, Qt5 runtime,
#             QML modules, ANT+ venv/USB permissions
#
# Usage:
#   ./qdomyos-zwift [OPTIONS]
#   ./qdomyos-zwift -no-gui -ant-footpod    # Headless with ANT+
################################################################################

declare -r QZ_WRAPPER_VERSION="3.0.0"

set -e
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

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
            echo -e "${C_YELLOW}Options:${C_RESET}"
            echo "  1. Upgrade to Ubuntu 24.04 LTS (recommended)"
            echo "     https://ubuntu.com/download/desktop"
            echo ""
            echo "  2. Use legacy v2.x build for Ubuntu 22.04"
            echo "     https://github.com/cagnulein/qdomyos-zwift/releases"
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
            echo -e "${C_YELLOW}Minimum required:${C_RESET} Debian 12 (Bookworm) / Raspberry Pi OS (Bookworm)"
            echo ""
            echo "QDomyos-Zwift v3.0+ requires Debian 12 or later."
            echo ""
            exit 1
        fi
    else
        # Unknown OS - warn but allow
        echo -e "${C_YELLOW}WARNING: Unsupported OS detected: $os_name${C_RESET}" >&2
        echo "QDomyos-Zwift is tested on Ubuntu 24.04+ and Debian 12+" >&2
        echo "Continuing anyway..." >&2
        echo "" >&2
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
    
    # Check for essential Qt5 libraries
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
        echo -e "${C_YELLOW}Install required packages:${C_RESET}"
        echo ""
        echo "  sudo apt-get update"
        echo "  sudo apt-get install -y \\"
        echo "    qtbase5-dev \\"
        echo "    libqt5qml5 \\"
        echo "    libqt5quick5 \\"
        echo "    libqt5widgets5 \\"
        echo "    libqt5bluetooth5 \\"
        echo "    libqt5charts5 \\"
        echo "    libqt5multimedia5 \\"
        echo "    qml-module-qtquick2 \\"
        echo "    qml-module-qtquick-controls2"
        echo ""
        echo "Then re-run this script."
        echo ""
        exit 1
    fi
    
    return 0
}

################################################################################
# Python 3.12 Check
################################################################################
check_python() {
    # Check for Python 3.12 or 3.11 (Debian 12 uses 3.11)
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
        ERRORS+=("Python 3.11+ not found - install with: sudo apt-get install python3")
    fi
}

################################################################################
# Binary Check
################################################################################
if [[ ! -f "$DIR/qdomyos-zwift-bin" ]]; then
    ERRORS+=("Binary not found: $DIR/qdomyos-zwift-bin")
fi

################################################################################
# Display Authorization (Auto-enable for root)
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
# Create runtime directory for root if needed
if [[ "$EUID" -eq 0 ]]; then
    QZ_RUNTIME_DIR="/tmp/runtime-root-qz"
    mkdir -p "$QZ_RUNTIME_DIR"
    chmod 0700 "$QZ_RUNTIME_DIR"
else
    QZ_RUNTIME_DIR="/run/user/$(id -u)"
fi

# Resolve XAUTHORITY
RESOLVED_XAUTH="${XAUTHORITY:-${USER_HOME}/.Xauthority}"
if [[ -f "$RESOLVED_XAUTH" ]]; then
    chmod 644 "$RESOLVED_XAUTH" 2>/dev/null || true
fi

# Set minimal clean environment (no LD_LIBRARY_PATH needed - uses system Qt5)
LAUNCH_ENV=(
    HOME="$USER_HOME"
    USER="${TARGET_USER}"
    LOGNAME="${TARGET_USER}"
    PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
    DISPLAY="${DISPLAY:-:0}"
    XAUTHORITY="$RESOLVED_XAUTH"
    XDG_RUNTIME_DIR="$QZ_RUNTIME_DIR"
    DBUS_SESSION_BUS_ADDRESS="${DBUS_SESSION_BUS_ADDRESS:-}"
)

# Signal forwarding
forward_signal() {
    local sig="$1"
    if [[ -n "${child_pid:-}" ]]; then
        kill -s "$sig" -- -"$child_pid" 2>/dev/null || true
    fi
}

trap 'forward_signal TERM' TERM
trap 'forward_signal INT' INT
trap 'forward_signal QUIT' QUIT

# Launch with clean environment
setsid env -i "${LAUNCH_ENV[@]}" "$DIR/qdomyos-zwift-bin" "$@" &
child_pid=$!

set +e
wait "$child_pid"
rc=$?
exit $rc