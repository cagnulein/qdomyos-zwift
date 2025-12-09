#!/bin/bash
################################################################################
# QDomyos-Zwift ANT+ Wrapper Script
#
# Purpose:
#   Runtime validation and dependency checker for QDomyos-Zwift with ANT+ support.
#   Creates a clean isolated environment to prevent library conflicts (snap/flatpak).
#
# Features:
#   - Auto-detects Python 3.11 library (pyenv priority, then system paths)
#   - Validates ANT+ virtual environment and required packages
#   - Checks USB permissions (plugdev group, udev rules)
#   - Filters snap paths from LD_LIBRARY_PATH to prevent glibc conflicts
#   - Executes binary with env -i for complete environment isolation
#   - Provides helpful error messages and setup instructions
#
# Usage:
#   ./qdomyos-zwift [OPTIONS]
#   Example: sudo ./qdomyos-zwift -no-gui -ant-footpod
#
# Platform: x86-64 Linux (Ubuntu/Debian)
# Dependencies: Python 3.11, openant, pyusb, pybind11, Qt5 runtime libraries
# Author: bassai-sho
# Development assisted by AI analysis tools
# Documentation: https://github.com/cagnulein/qdomyos-zwift/blob/master/src/devices/antlinux/README.md
################################################################################

set -e
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Color codes for output
C_GREEN="\033[0;32m"
C_RED="\033[0;31m"
C_YELLOW="\033[0;33m"
C_RESET="\033[0m"

WARNINGS=()
ERRORS=()
ANT_WARNINGS=()

# Determine the actual user's home (even when running with sudo)
if [[ "$EUID" -eq 0 && -n "${SUDO_USER}" ]]; then
    USER_HOME=$(getent passwd "${SUDO_USER}" | cut -d: -f6)
    TARGET_USER="$SUDO_USER"
    # Preserve the actual user's XDG_RUNTIME_DIR if it was passed through sudo
    # Otherwise, try to determine it from the user's UID
    if [[ -z "${XDG_RUNTIME_DIR}" ]]; then
        SUDO_UID=$(id -u "${SUDO_USER}")
        if [[ -d "/run/user/${SUDO_UID}" ]]; then
            XDG_RUNTIME_DIR="/run/user/${SUDO_UID}"
        fi
    fi
    # Preserve XAUTHORITY if not set
    if [[ -z "${XAUTHORITY}" ]]; then
        XAUTHORITY="${USER_HOME}/.Xauthority"
    fi
else
    USER_HOME="${HOME}"
    TARGET_USER="$USER"
fi

# Function to check if libpython3.11 exists in a directory
check_libpython() {
    local lib_dir="$1"
    if [[ -d "$lib_dir" ]] && ls "$lib_dir/libpython3.11.so"* &>/dev/null; then
        return 0
    fi
    return 1
}

# === CHECK 1: Python 3.11 Library ===
PYTHON_LIB=""

# 1. Try pyenv first (user-specific installation)
if [[ -d "${USER_HOME}/.pyenv/versions" ]]; then
    for pyver in "${USER_HOME}/.pyenv/versions/3.11."*; do
        if check_libpython "${pyver}/lib"; then
            PYTHON_LIB="${pyver}/lib"
            break
        fi
    done
fi

# 2. Try system locations if not found in pyenv
if [[ -z "$PYTHON_LIB" ]]; then
    for sys_lib in /usr/lib/x86_64-linux-gnu /usr/lib64 /usr/lib /usr/local/lib; do
        if check_libpython "$sys_lib"; then
            PYTHON_LIB="$sys_lib"
            break
        fi
    done
fi

if [[ -z "$PYTHON_LIB" ]]; then
    ERRORS+=("Python 3.11 library (libpython3.11.so) not found!")
else
    # Clean LD_LIBRARY_PATH to remove snap paths that cause glibc conflicts
    CLEAN_LD_PATH=""
    if [[ -n "${LD_LIBRARY_PATH:-}" ]]; then
        IFS=':' read -ra PATHS <<< "$LD_LIBRARY_PATH"
        for path in "${PATHS[@]}"; do
            # Skip snap paths to avoid glibc version conflicts
            if [[ "$path" != *"/snap/"* ]]; then
                CLEAN_LD_PATH="${CLEAN_LD_PATH:+${CLEAN_LD_PATH}:}${path}"
            fi
        done
    fi
    export LD_LIBRARY_PATH="${PYTHON_LIB}:${CLEAN_LD_PATH}"
fi

# Check for -no-gui flag in arguments
USING_NO_GUI=false
for arg in "$@"; do
    if [[ "$arg" == "-no-gui" ]]; then
        USING_NO_GUI=true
        break
    fi
done

# === CHECK 2: Qt5 Runtime Libraries ===
# Qt5 Bluetooth is always required
if ! ldconfig -p 2>/dev/null | grep "libQt5Bluetooth.so.5" >/dev/null 2>&1; then
    ERRORS+=("libQt5Bluetooth.so.5 not found (required for Bluetooth connectivity)")
fi

# Other Qt5 libraries only needed for GUI
MISSING_QT5=()
QT5_LIBS=(
    "libQt5Core.so.5"
    "libQt5Qml.so.5"
    "libQt5Quick.so.5"
    "libQt5QuickWidgets.so.5"
    "libQt5Concurrent.so.5"
    "libQt5Charts.so.5"
    "libQt5Multimedia.so.5"
    "libQt5MultimediaWidgets.so.5"
    "libQt5NetworkAuth.so.5"
    "libQt5Positioning.so.5"
    "libQt5Sql.so.5"
    "libQt5TextToSpeech.so.5"
    "libQt5WebSockets.so.5"
    "libQt5Widgets.so.5"
    "libQt5Xml.so.5"
    "libQt5Location.so.5"
)

for lib in "${QT5_LIBS[@]}"; do
    if ldconfig -p 2>/dev/null | grep "$lib" >/dev/null 2>&1; then
        : # Library found
    else
        MISSING_QT5+=("$lib")
    fi
done

if [[ ${#MISSING_QT5[@]} -gt 0 ]]; then
    ERRORS+=("Missing Qt5 libraries: ${MISSING_QT5[*]}")
fi

# === CHECK 5: QML Modules ===
MISSING_QML=()
QML_MODULES=(
    "/usr/lib/x86_64-linux-gnu/qt5/qml/QtLocation"
    "/usr/lib/x86_64-linux-gnu/qt5/qml/QtPositioning"
    "/usr/lib/x86_64-linux-gnu/qt5/qml/QtQuick.2"
    "/usr/lib/x86_64-linux-gnu/qt5/qml/QtQuick/Controls"
    "/usr/lib/x86_64-linux-gnu/qt5/qml/QtQuick/Controls.2"
    "/usr/lib/x86_64-linux-gnu/qt5/qml/QtQuick/Dialogs"
    "/usr/lib/x86_64-linux-gnu/qt5/qml/QtQuick/Layouts"
    "/usr/lib/x86_64-linux-gnu/qt5/qml/QtQuick/Window.2"
    "/usr/lib/x86_64-linux-gnu/qt5/qml/QtMultimedia"
)

for qml_path in "${QML_MODULES[@]}"; do
    if [[ ! -d "$qml_path" ]]; then
        qml_name=$(basename "$qml_path")
        MISSING_QML+=("$qml_name")
    fi
done

if [[ ${#MISSING_QML[@]} -gt 0 ]]; then
    ERRORS+=("Missing QML modules: ${MISSING_QML[*]}")
fi

# === CHECK 4: ANT+ Dependencies (only when using -ant-footpod) ===
# Check for -ant-footpod flag in arguments
USING_ANT_FOOTPOD=false
for arg in "$@"; do
    if [[ "$arg" == "-ant-footpod" ]]; then
        USING_ANT_FOOTPOD=true
        break
    fi
done

# Only check ANT+ dependencies if using ANT+ footpod
if [[ "$USING_ANT_FOOTPOD" == true ]]; then
    # CHECK 4a: Virtual Environment
    VENV_PATH="${USER_HOME}/ant_venv"
    if [[ ! -d "$VENV_PATH" ]]; then
        ANT_WARNINGS+=("ANT+ virtual environment not found at: ${VENV_PATH}")
    elif [[ -x "$VENV_PATH/bin/python3" ]]; then
        # Check for required packages
        for pkg_import in "openant" "usb.core" "pybind11"; do
            if ! "$VENV_PATH/bin/python3" -c "import $pkg_import" &>/dev/null; then
                ANT_WARNINGS+=("Missing Python package in venv: ${pkg_import}")
                break
            fi
        done
    fi

    # CHECK 4b: USB Permissions
    if [[ -n "$TARGET_USER" ]]; then
        if groups "$TARGET_USER" 2>/dev/null | grep '\bplugdev\b' >/dev/null 2>&1; then
            : # User is in plugdev group
        else
            ANT_WARNINGS+=("User '$TARGET_USER' not in 'plugdev' group (needed for ANT+ USB access)")
        fi
    fi

    if [[ ! -f "/etc/udev/rules.d/99-ant-usb.rules" ]]; then
        ANT_WARNINGS+=("ANT+ udev rules not found (needed for USB dongle access)")
    fi

    # CHECK 4c: lsusb and USB Dongle
    if ! command -v lsusb >/dev/null 2>&1; then
        ERRORS+=("lsusb command not found (usbutils package needed for ANT+ footpod)")
    else
        # Check for ANT+ dongle (grep returns non-zero if not found, so capture result)
        if ! lsusb 2>/dev/null | grep -E '(0fcf:1008|0fcf:1009|0fcf:100c|0fcf:100e|0fcf:88a4|11fd:0001)' >/dev/null 2>&1; then
            ANT_WARNINGS+=("ANT+ USB dongle not detected")
        fi
    fi
fi

# Promote ANT-specific warnings to ERRORS when ANT+ footpod mode requested,
# otherwise treat them as general WARNINGS. This centralizes decision logic
# and avoids scattering guidance messages through the checks above.
if [[ ${#ANT_WARNINGS[@]} -gt 0 ]]; then
    if [[ "$USING_ANT_FOOTPOD" == true ]]; then
        for w in "${ANT_WARNINGS[@]}"; do
            ERRORS+=("$w")
        done
    else
        for w in "${ANT_WARNINGS[@]}"; do
            WARNINGS+=("$w")
        done
    fi
fi

# === CHECK 5: Binary Exists ===
if [[ ! -f "$DIR/qdomyos-zwift-bin" ]]; then
    ERRORS+=("Binary not found at: $DIR/qdomyos-zwift-bin")
fi

# === Display warnings and errors ===
if [[ ${#ERRORS[@]} -gt 0 ]]; then
    echo -e "${C_RED}ERROR: Cannot start QDomyos-Zwift${C_RESET}" >&2
    echo "" >&2
    for err in "${ERRORS[@]}"; do
        echo -e "${C_RED}  $err${C_RESET}" >&2
    done
    echo "" >&2
    echo -e "${C_RED}Run: sudo ./setup.sh --guided${C_RESET}" >&2
    echo "" >&2
    exit 1
fi

if [[ ${#WARNINGS[@]} -gt 0 ]]; then
    echo -e "${C_YELLOW}WARNING: ANT+ functionality may not work:${C_RESET}" >&2
    echo "" >&2
    for warn in "${WARNINGS[@]}"; do
        echo -e "${C_YELLOW}  $warn${C_RESET}" >&2
    done
    echo "" >&2
    echo -e "${C_YELLOW}Run: sudo ./setup.sh --quick${C_RESET}" >&2
    echo "" >&2
fi

# All checks passed or warnings only - execute the binary with clean environment
# Use env -i to avoid snap/flatpak library conflicts, preserve essential variables
exec env -i \
    HOME="$USER_HOME" \
    USER="${TARGET_USER:-$USER}" \
    LOGNAME="${TARGET_USER:-$USER}" \
    PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin" \
    LD_LIBRARY_PATH="$LD_LIBRARY_PATH" \
    DISPLAY="${DISPLAY:-}" \
    XAUTHORITY="${XAUTHORITY:-}" \
    XDG_RUNTIME_DIR="${XDG_RUNTIME_DIR:-}" \
    DBUS_SESSION_BUS_ADDRESS="${DBUS_SESSION_BUS_ADDRESS:-}" \
    "$DIR/qdomyos-zwift-bin" "$@"
