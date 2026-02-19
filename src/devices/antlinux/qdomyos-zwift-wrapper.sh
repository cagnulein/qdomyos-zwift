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

declare -r QZ_WRAPPER_VERSION="2.0"

set -e
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

declare -r C_RED="\033[0;31m"
declare -r C_YELLOW="\033[0;33m"
declare -r C_CYAN="\033[0;36m"
declare -r C_RESET="\033[0m"

WARNINGS=()
ERRORS=()
ANT_WARNINGS=()

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
        echo -e "${C_RED}Supported: x86-64, ARM64${C_RESET}" >&2
        exit 1
        ;;
esac

################################################################################
# Configuration (sync with setup-dashboard.sh)
################################################################################
declare -r QZ_CONFIG_NAMESPACE="Roberto Viola"
declare -r QZ_CONFIG_FILE="qDomyos-Zwift.conf"

get_config_path() {
    local user_home=${1:-$HOME}
    echo "${user_home}/.config/${QZ_CONFIG_NAMESPACE}/${QZ_CONFIG_FILE}"
}

get_service_config_path() {
    local user_home=${1:-$HOME}
    echo "${user_home}/.config/${QZ_CONFIG_NAMESPACE}/service.conf"
}

################################################################################
# Python Configuration (sync with setup-dashboard.sh)
################################################################################
declare -r PYTHON_VERSION_MAJOR="3.11"
declare -r PYTHON_VERSION_FULL="3.11.9"
declare -r PYTHON_LIB_NAME="libpython${PYTHON_VERSION_MAJOR}.so"
declare -r PYTHON_PACKAGE="python${PYTHON_VERSION_MAJOR}"
declare -r PYENV_VERSION_PATH=".pyenv/versions/${PYTHON_VERSION_FULL}"

# Python resolver: venv -> pyenv -> python3.11 -> python3 (if 3.11.x)
resolve_python_bin() {
    local u_home=${1:-$USER_HOME}
    
    if [[ -x "${u_home}/ant_venv/bin/python3" ]]; then
        echo "${u_home}/ant_venv/bin/python3"
        return 0
    fi
    
    if [[ -x "${u_home}/${PYENV_VERSION_PATH}/bin/python3" ]]; then
        echo "${u_home}/${PYENV_VERSION_PATH}/bin/python3"
        return 0
    fi
    
    if command -v python3.11 >/dev/null 2>&1; then
        command -v python3.11
        return 0
    fi
    
    if command -v python3 >/dev/null 2>&1; then
        local py_ver
        py_ver=$(python3 -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')" 2>/dev/null)
        if [[ "$py_ver" == "${PYTHON_VERSION_MAJOR}" ]]; then
            command -v python3
            return 0
        fi
    fi
    
    echo ""
    return 1
}

################################################################################
# Environment Validation
################################################################################
validate_env_vars() {
    if [[ -n "${DISPLAY:-}" ]]; then
        if [[ ! "$DISPLAY" =~ ^:[0-9]+(\.[0-9]+)?$ ]]; then
            echo -e "${C_RED}ERROR: Invalid DISPLAY format: $DISPLAY${C_RESET}" >&2
            exit 1
        fi
    fi

    if [[ -n "${XAUTHORITY:-}" ]]; then
        if [[ "$XAUTHORITY" != "${USER_HOME}/"* ]] || [[ "$XAUTHORITY" == *".."* ]]; then
            echo -e "${C_YELLOW}WARNING: Unsafe XAUTHORITY path, unsetting.${C_RESET}" >&2
            unset XAUTHORITY
        fi
    fi

    if [[ -n "${DBUS_SESSION_BUS_ADDRESS:-}" ]]; then
        if [[ ! "$DBUS_SESSION_BUS_ADDRESS" =~ ^unix:(path|abstract)= ]]; then
            echo -e "${C_YELLOW}WARNING: Invalid DBUS address, unsetting.${C_RESET}" >&2
            unset DBUS_SESSION_BUS_ADDRESS
        fi
    fi
}

################################################################################
# User Resolution (SUDO_USER -> QZ_USER -> script owner -> venv owner -> fallback)
################################################################################
if [[ -n "${SUDO_USER:-}" ]]; then
    TARGET_USER="$SUDO_USER"
    USER_HOME=$(getent passwd "${SUDO_USER}" | cut -d: -f6)
elif [[ -n "${QZ_USER:-}" ]]; then
    if [[ ! "$QZ_USER" =~ ^[a-zA-Z0-9_-]+$ ]]; then
        echo -e "${C_RED}ERROR: Invalid QZ_USER format: $QZ_USER${C_RESET}" >&2
        exit 1
    fi
    
    TARGET_USER="$QZ_USER"
    USER_HOME=$(getent passwd "${QZ_USER}" 2>/dev/null | cut -d: -f6)
    
    if [[ -z "$USER_HOME" ]]; then
        echo -e "${C_RED}ERROR: User '$QZ_USER' not found on this system.${C_RESET}" >&2
        exit 1
    fi
else
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    REPO_OWNER=$(stat -c '%U' "$SCRIPT_DIR" 2>/dev/null || echo "")
    if [[ -n "$REPO_OWNER" && "$REPO_OWNER" != "root" ]]; then
        TARGET_USER="$REPO_OWNER"
        USER_HOME=$(getent passwd "${REPO_OWNER}" | cut -d: -f6)
    else
        FOUND=false
        for d in /home/*/ant_venv; do
            if [[ -d "$d" ]]; then
                USER_HOME="$(dirname "$d")"
                TARGET_USER=$(stat -c '%U' "$USER_HOME" 2>/dev/null || echo "")
                FOUND=true
                break
            fi
        done
        if [[ "$FOUND" != "true" ]]; then
            TARGET_USER="${USER:-root}"
            USER_HOME="${HOME:-/root}"
        fi
    fi
fi

TARGET_USER="${TARGET_USER:-root}"
USER_HOME="${USER_HOME:-/root}"

validate_env_vars

################################################################################
# Validate Resolved User
################################################################################
validate_resolved_user() {
    if ! id "$TARGET_USER" >/dev/null 2>&1; then
        echo -e "${C_RED}ERROR: User '$TARGET_USER' does not exist.${C_RESET}" >&2
        exit 1
    fi
    
    if [[ ! -d "$USER_HOME" ]]; then
        echo -e "${C_RED}ERROR: Home directory '$USER_HOME' not found.${C_RESET}" >&2
        exit 1
    fi
}

validate_resolved_user

################################################################################
# Dependency Checks
################################################################################

# Python 3.11 shared library
PYTHON_BIN=$(resolve_python_bin "$USER_HOME")

if [[ -z "$PYTHON_BIN" ]]; then
    ERRORS+=("Python ${PYTHON_VERSION_MAJOR} not found (install ${PYTHON_PACKAGE} or use pyenv)")
else
    PYTHON_LIB=""
    case "$PYTHON_BIN" in
        */ant_venv/*)
            PYTHON_LIB="${USER_HOME}/ant_venv/lib"
            ;;
        */.pyenv/*)
            PYENV_PREFIX="${PYTHON_BIN%/bin/python3}"
            PYTHON_LIB="${PYENV_PREFIX}/lib"
            ;;
        *)
            # System Python - robust multi-method search
            PYTHON_LIB=""
            
            # Method 1: Check common locations (fast)
            local common_paths=(
                "/usr/lib/${ARCH_LIB_PATH}"
                "/lib/${ARCH_LIB_PATH}"                    # Debian/Ubuntu often use /lib
                "/usr/lib"
                "/lib"
                "/usr/lib/python${PYTHON_VERSION_MAJOR}/config-${PYTHON_VERSION_MAJOR}-${ARCH_LIB_PATH}"
                "/usr/local/lib"
                "/usr/local/lib/${ARCH_LIB_PATH}"
            )
            
            for path in "${common_paths[@]}"; do
                # Check for exact match or versioned library (e.g., libpython3.11.so.1.0)
                if [[ -f "$path/${PYTHON_LIB_NAME}" ]] || compgen -G "$path/${PYTHON_LIB_NAME}*" >/dev/null 2>&1; then
                    PYTHON_LIB="$path"
                    break
                fi
            done
            
            # Method 2: Query system library cache (fast and reliable)
            if [[ -z "$PYTHON_LIB" ]]; then
                local lib_path
                # Look for versioned or unversioned library
                lib_path=$(ldconfig -p 2>/dev/null | grep -m1 -E "${PYTHON_LIB_NAME}(\.|$)" | awk '{print $NF}')
                if [[ -n "$lib_path" && -f "$lib_path" ]]; then
                    PYTHON_LIB="$(dirname "$lib_path")"
                fi
            fi
            
            # Method 3: Use python-config if available (python knows where its library is)
            if [[ -z "$PYTHON_LIB" ]]; then
                if command -v python${PYTHON_VERSION_MAJOR}-config >/dev/null 2>&1; then
                    local config_flags
                    config_flags=$(python${PYTHON_VERSION_MAJOR}-config --ldflags 2>/dev/null)
                    # Extract -L paths from flags
                    local lib_dir
                    lib_dir=$(echo "$config_flags" | grep -oP '(?<=-L)[^ ]+' | head -1)
                    if [[ -n "$lib_dir" ]] && { [[ -f "$lib_dir/${PYTHON_LIB_NAME}" ]] || compgen -G "$lib_dir/${PYTHON_LIB_NAME}*" >/dev/null 2>&1; }; then
                        PYTHON_LIB="$lib_dir"
                    fi
                fi
            fi
            
            # Method 4: Targeted search in /lib and /usr (last resort, still fast)
            if [[ -z "$PYTHON_LIB" ]]; then
                local found_path
                found_path=$(find /usr/lib /usr/local/lib /lib -name "${PYTHON_LIB_NAME}*" -type f 2>/dev/null | head -1)
                if [[ -n "$found_path" ]]; then
                    PYTHON_LIB="$(dirname "$found_path")"
                fi
            fi
            
            # If still not found, error and use fallback path
            if [[ -z "$PYTHON_LIB" ]]; then
                ERRORS+=("${PYTHON_LIB_NAME} not found (needed for embedded Python)")
                PYTHON_LIB="/usr/lib"  # Fallback to avoid empty path
            fi
            ;;
    esac
fi

LD_LIBRARY_PATH="${PYTHON_LIB}:/usr/local/lib:/usr/lib:/usr/lib/${ARCH_LIB_PATH}"

# Derive PYTHONHOME
# For pyenv: the pyenv prefix IS a full Python install, use it directly.
# For system Python + ant_venv: the venv is NOT a full Python install.
#   PYTHONHOME must point to the BASE Python prefix (e.g. /usr) where
#   encodings/ and lib-dynload/ live.  Using the venv path here causes
#   "failed to get the Python codec of the filesystem encoding" because
#   Py_Initialize can't find the encodings module inside a bare venv.
# For system Python without venv: use sys.base_prefix from the system binary.
PYTHON_HOME=""
if [[ "$PYTHON_LIB" == *"/.pyenv/"* ]]; then
    # pyenv: full standalone install — prefix is correct
    PYTHON_HOME="${PYTHON_LIB%/lib}"
elif [[ "$PYTHON_LIB" == */ant_venv/* ]]; then
    # venv: query base_prefix from the venv's own Python binary
    PYTHON_HOME=$("${USER_HOME}/ant_venv/bin/python3" -c \
        "import sys; print(sys.base_prefix)" 2>/dev/null || echo "")
fi

# Fallback: if still unset, try the ambient PYTHONHOME then sys.base_prefix
if [[ -z "$PYTHON_HOME" ]]; then
    if [[ -n "${PYTHONHOME:-}" ]]; then
        PYTHON_HOME="$PYTHONHOME"
    elif [[ -n "$PYTHON_BIN" ]]; then
        PYTHON_HOME=$("$PYTHON_BIN" -c \
            "import sys; print(sys.base_prefix)" 2>/dev/null || echo "")
    fi
fi

# Verify _struct module
if [[ -n "$PYTHON_HOME" ]] && [[ -x "${PYTHON_HOME}/bin/python3" ]]; then
    if ! "${PYTHON_HOME}/bin/python3" -c "import _struct" 2>/dev/null; then
        ERRORS+=("Python at ${PYTHON_HOME} cannot import _struct (install with --enable-shared)")
    fi
fi

# Qt5 runtime libraries
declare -A _LIB_AVAILABLE
while IFS= read -r line; do
    if [[ "$line" =~ [[:space:]]([^[:space:]]+\.so(\.[0-9]+)*)[[:space:]] ]]; then
        _LIB_AVAILABLE["${BASH_REMATCH[1]}"]=1
    fi
done < <(ldconfig -p 2>/dev/null)

check_lib() { [[ -n "${_LIB_AVAILABLE[$1]:-}" ]]; }

check_lib "libQt5Bluetooth.so.5" || \
    ERRORS+=("libQt5Bluetooth.so.5 not found (required for Bluetooth)")

QT5_LIBS=(
    "libQt5Core.so.5" "libQt5Qml.so.5" "libQt5Quick.so.5" 
    "libQt5QuickWidgets.so.5" "libQt5Concurrent.so.5" "libQt5Charts.so.5" 
    "libQt5Multimedia.so.5" "libQt5MultimediaWidgets.so.5" 
    "libQt5NetworkAuth.so.5" "libQt5Positioning.so.5" "libQt5Sql.so.5" 
    "libQt5TextToSpeech.so.5" "libQt5WebSockets.so.5" "libQt5Widgets.so.5" 
    "libQt5Xml.so.5" "libQt5Location.so.5"
)

MISSING_QT5=()
for lib in "${QT5_LIBS[@]}"; do
    check_lib "$lib" || MISSING_QT5+=("$lib")
done

[[ ${#MISSING_QT5[@]} -gt 0 ]] && ERRORS+=("Missing Qt5 libraries: ${MISSING_QT5[*]}")
unset -f check_lib

# QML modules
MISSING_QML=()
QML_MODULES=(
    "/usr/lib/${ARCH_LIB_PATH}/qt5/qml/QtLocation"
    "/usr/lib/${ARCH_LIB_PATH}/qt5/qml/QtPositioning"
    "/usr/lib/${ARCH_LIB_PATH}/qt5/qml/QtQuick.2"
    "/usr/lib/${ARCH_LIB_PATH}/qt5/qml/QtQuick/Controls"
    "/usr/lib/${ARCH_LIB_PATH}/qt5/qml/QtQuick/Controls.2"
    "/usr/lib/${ARCH_LIB_PATH}/qt5/qml/QtQuick/Dialogs"
    "/usr/lib/${ARCH_LIB_PATH}/qt5/qml/QtQuick/Layouts"
    "/usr/lib/${ARCH_LIB_PATH}/qt5/qml/QtQuick/Window.2"
    "/usr/lib/${ARCH_LIB_PATH}/qt5/qml/QtMultimedia"
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

# ANT+ dependencies (conditional on -ant-footpod flag)
USING_ANT_FOOTPOD=false
for arg in "$@"; do
    if [[ "$arg" == "-ant-footpod" ]]; then
        USING_ANT_FOOTPOD=true
        break
    fi
done

if [[ "$USING_ANT_FOOTPOD" == true ]]; then
    VENV_PATH="${USER_HOME}/ant_venv"
    if [[ ! -d "$VENV_PATH" ]]; then
        ANT_WARNINGS+=("ANT+ virtual environment not found at: ${VENV_PATH}")
    elif [[ -x "$VENV_PATH/bin/python3" ]]; then
        for pkg_import in "openant" "usb.core" "pybind11"; do
            if ! "$VENV_PATH/bin/python3" -c "import $pkg_import" &>/dev/null; then
                ANT_WARNINGS+=("Missing Python package in venv: ${pkg_import}")
                break
            fi
        done
    fi

    if [[ -n "$TARGET_USER" ]]; then
        if ! groups "$TARGET_USER" 2>/dev/null | grep '\bplugdev\b' >/dev/null 2>&1; then
            ANT_WARNINGS+=("User '$TARGET_USER' not in 'plugdev' group (needed for ANT+ USB)")
        fi
    fi

    if [[ ! -f "/etc/udev/rules.d/99-ant-usb.rules" ]]; then
        ANT_WARNINGS+=("ANT+ udev rules not found (needed for USB dongle)")
    fi

    if ! command -v lsusb >/dev/null 2>&1; then
        ERRORS+=("lsusb not found (usbutils package needed for ANT+)")
    else
        if ! lsusb 2>/dev/null | grep -E '(0fcf:1008|0fcf:1009|0fcf:100c|0fcf:100e|0fcf:88a4|11fd:0001)' >/dev/null 2>&1; then
            ANT_WARNINGS+=("ANT+ USB dongle not detected")
        fi
    fi
fi

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

# Binary exists (docker build names it qdomyos-zwift-bin)
if [[ ! -f "$DIR/qdomyos-zwift-bin" ]]; then
    ERRORS+=("Binary not found at: $DIR/qdomyos-zwift-bin")
fi

################################################################################
# Display Results
################################################################################
if [[ ${#ERRORS[@]} -gt 0 ]]; then
    echo -e "${C_RED}ERROR: Cannot start QDomyos-Zwift (Platform: $PLATFORM)${C_RESET}" >&2
    echo "" >&2
    for err in "${ERRORS[@]}"; do
        echo -e "${C_RED}  ✗ $err${C_RESET}" >&2
    done
    echo "" >&2
    echo -e "${C_CYAN}Tip: Run setup-dashboard.sh to fix dependencies${C_RESET}" >&2
    exit 1
fi

if [[ ${#WARNINGS[@]} -gt 0 ]]; then
    echo -e "${C_YELLOW}WARNING: ANT+ functionality may not work (Platform: $PLATFORM):${C_RESET}" >&2
    echo "" >&2
    for warn in "${WARNINGS[@]}"; do
        echo -e "${C_YELLOW}  ⚠ $warn${C_RESET}" >&2
    done
    echo "" >&2
fi

################################################################################
# Launch Binary
################################################################################
LAUNCH_ENV=(
    HOME="$USER_HOME"
    USER="${TARGET_USER:-$USER}"
    LOGNAME="${TARGET_USER:-$USER}"
    PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
    LD_LIBRARY_PATH="$LD_LIBRARY_PATH"
    DISPLAY="${DISPLAY:-}"
    XAUTHORITY="${XAUTHORITY:-}"
    XDG_RUNTIME_DIR="${XDG_RUNTIME_DIR:-}"
    DBUS_SESSION_BUS_ADDRESS="${DBUS_SESSION_BUS_ADDRESS:-}"
)

if [[ -n "${PYTHON_HOME:-}" ]]; then
    LAUNCH_ENV+=(PYTHONHOME="$PYTHON_HOME")
fi

forward_signal() {
    local sig="$1"
    if [[ -n "${child_pid:-}" ]]; then
        kill -s "$sig" -- -"$child_pid" 2>/dev/null || true
    fi
}

trap 'forward_signal TERM' TERM
trap 'forward_signal INT' INT
trap 'forward_signal QUIT' QUIT

setsid env -i "${LAUNCH_ENV[@]}" "$DIR/qdomyos-zwift-bin" "$@" &
child_pid=$!

set +e
wait "$child_pid"
rc=$?
exit $rc