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
if [[ ! -f "$DIR/qdomyos-zwift-bin" ]]; then
    ERRORS+=("Binary not found: $DIR/qdomyos-zwift-bin")
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
# QML Assets Check (GUI Mode Only)
# Supports both root directory and qml/ subdirectory
################################################################################
if [[ "$GUI_MODE" == "true" ]]; then
    # Check qml/ subdirectory first (preferred structure)
    QML_DIR="$DIR/qml"
    if [[ -d "$QML_DIR" ]]; then
        QML_COUNT=$(find "$QML_DIR" -maxdepth 1 -name "*.qml" -type f 2>/dev/null | wc -l)
        QML_LOCATION="qml subdirectory"
        
        # Set QML import path for subdirectory structure
        export QML2_IMPORT_PATH="$QML_DIR:${QML2_IMPORT_PATH:-}"
        
        # Ensure binary can find main.qml in subdirectory
        # Some Qt applications look for main.qml in current directory
        if [[ -f "$QML_DIR/main.qml" ]] && [[ ! -f "$DIR/main.qml" ]]; then
            # Create symlink to help binary find it
            ln -sf "qml/main.qml" "$DIR/main.qml" 2>/dev/null || true
        fi
    else
        # Fall back to root directory (legacy structure)
        QML_COUNT=$(find "$DIR" -maxdepth 1 -name "*.qml" -type f 2>/dev/null | wc -l)
        QML_LOCATION="root directory"
    fi
    
    if [[ $QML_COUNT -eq 0 ]]; then
        echo ""
        echo -e "${C_RED}╔════════════════════════════════════════════════════════════════════╗${C_RESET}"
        echo -e "${C_RED}║  ERROR: QML UI Assets Missing                                     ║${C_RESET}"
        echo -e "${C_RED}╚════════════════════════════════════════════════════════════════════╝${C_RESET}"
        echo ""
        echo "GUI mode detected, but no *.qml files found in:"
        echo "  $DIR (root directory)"
        echo "  $DIR/qml (subdirectory)"
        echo ""
        echo -e "${C_YELLOW}This causes the binary to exit silently!${C_RESET}"
        echo ""
        echo "The Qt QML engine cannot load the UI without these files."
        echo "The binary performs a clean abort (exit code 0377/-1)."
        echo ""
        echo -e "${C_YELLOW}Solutions:${C_RESET}"
        echo ""
        echo "  1. Download complete package with UI files:"
        echo "     https://github.com/cagnulein/qdomyos-zwift/releases"
        echo ""
        echo "     Required files: main.qml, settings.qml, and 40+ other *.qml files"
        echo "     Preferred location: $DIR/qml/"
        echo "     Alternative: $DIR/"
        echo ""
        echo "  2. Run in headless mode:"
        echo "     sudo $0 -no-gui -ant-footpod"
        echo ""
        exit 1
    fi
    
    # Show where QML files were found
    if [[ "${VERBOSE:-0}" == "1" ]]; then
        echo -e "${C_GREEN}Found $QML_COUNT QML files in $QML_LOCATION${C_RESET}" >&2
    fi
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
)

# Add QML path if using subdirectory structure
if [[ -n "${QML2_IMPORT_PATH:-}" ]]; then
    LAUNCH_ENV+=("QML2_IMPORT_PATH=$QML2_IMPORT_PATH")
fi

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

# Launch
setsid env -i "${LAUNCH_ENV[@]}" "$DIR/qdomyos-zwift-bin" "$@" &
child_pid=$!

set +e
wait "$child_pid"
rc=$?
exit $rc