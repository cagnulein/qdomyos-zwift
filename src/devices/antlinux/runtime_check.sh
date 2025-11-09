#!/bin/bash
# -----------------------------------------------------------------------------
# QDomyos-Zwift: ANT+ Virtual Footpod Feature
# Runtime Environment Diagnostic Script
#
# Part of QDomyos-Zwift project: https://github.com/cagnulein/qdomyos-zwift
# Contributor(s): bassai-sho
# AI analysis tools (Claude, Gemini) were used to assist coding and debugging
# Licensed under GPL-3.0 - see project repository for full license
#
# This script validates a user's runtime environment for the ANT+ footpod
# feature. It checks Python environment, USB permissions, and system services.
# -----------------------------------------------------------------------------

# --- Color Definitions ---
C_GREEN="\033[0;32m"
C_RED="\033[0;31m"
C_YELLOW="\033[0;33m"
C_CYAN="\033[0;36m"
C_RESET="\033[0m"

# --- Check Results Storage ---
declare -a CHECK_RESULTS
declare -a CHECK_NAMES
FIRST_FAILURE=""
FIRST_FAILURE_FIX=""

# --- Check for sudo ---
if [[ $EUID -ne 0 ]]; then
   echo -e "${C_RED}ERROR: This script must be run with sudo.${C_RESET}" 
   echo "Please run again as: sudo $0"
   exit 1
fi

# --- Determine target user and home directory ---
if [ -n "$SUDO_USER" ]; then
    TARGET_USER="$SUDO_USER"
    TARGET_HOME=$(eval echo "~$SUDO_USER")
else
    TARGET_USER="$USER"
    TARGET_HOME="$HOME"
fi

echo -e "${C_CYAN}========================================================${C_RESET}"
echo -e "${C_CYAN}    QZ ANT+ Runtime Diagnostic${C_RESET}"
echo -e "${C_CYAN}    Running for user: $TARGET_USER${C_RESET}"
echo -e "${C_CYAN}========================================================${C_RESET}"
echo

# --- Helper function to record check results ---
record_check() {
    local check_name="$1"
    local passed="$2"
    local fix_instructions="$3"
    
    CHECK_NAMES+=("$check_name")
    CHECK_RESULTS+=("$passed")
    
    if [ "$passed" = "false" ] && [ -z "$FIRST_FAILURE" ]; then
        FIRST_FAILURE="$check_name"
        FIRST_FAILURE_FIX="$fix_instructions"
    fi
}

# ---
# CHECK 1: Python 3.11 and Venv Module
# ---
check_python() {
    local PYTHON311_CMD=""
    local PYTHON_OK=false
    local VENV_OK=false
    
    # Priority 1: Check for pyenv installation
    if [ -d "$TARGET_HOME/.pyenv/versions/3.11.9/bin" ]; then
        PYTHON311_CMD="$TARGET_HOME/.pyenv/versions/3.11.9/bin/python3.11"
    # Priority 2: Check system path
    elif command -v python3.11 >/dev/null 2>&1; then
        PYTHON311_CMD="python3.11"
    fi
    
    if [ -n "$PYTHON311_CMD" ]; then
        PYTHON_OK=true
        # Check for venv module
        if $PYTHON311_CMD -c "import ensurepip" >/dev/null 2>&1; then
            VENV_OK=true
        fi
    fi
    
    if [ "$PYTHON_OK" = true ] && [ "$VENV_OK" = true ]; then
        record_check "Python 3.11 + venv" "true" ""
        export PYTHON311_CMD
    elif [ "$PYTHON_OK" = true ] && [ "$VENV_OK" = false ]; then
        local fix="Install the venv module:\n\n"
        fix="${fix}${C_YELLOW}sudo apt-get install python3.11-venv${C_RESET}"
        record_check "Python 3.11 + venv" "false" "$fix"
    else
        local fix=""
        if apt-cache show python3.11-venv >/dev/null 2>&1; then
            fix="Your system provides Python 3.11 via the package manager.\n"
            fix="${fix}This is the recommended installation method.\n\n"
            fix="${fix}${C_YELLOW}sudo apt-get install python3.11 python3.11-venv${C_RESET}"
        else
            # Check what Python version is available in apt-cache
            local available_python=""
            for ver in 3.12 3.13 3.10 3.9 3.8; do
                if apt-cache show "python${ver}" >/dev/null 2>&1; then
                    available_python="$ver"
                    break
                fi
            done
            
            fix="Your system does not provide Python 3.11 via package manager.\n"
            if [ -n "$available_python" ]; then
                fix="${fix}Your system provides Python ${available_python}, but the pre-compiled binary requires Python 3.11.\n"
            fi
            fix="${fix}This can occur on older distributions (e.g., Ubuntu 20.04, Debian Bullseye)\n"
            fix="${fix}or newer distributions with only Python 3.12+ (e.g., Debian Trixie).\n\n"
            fix="${fix}Install Python 3.11 using pyenv:\n\n"
            fix="${fix}${C_YELLOW}# Step 1: Install prerequisites (git and curl required for pyenv)${C_RESET}\n"
            
            # Build intelligent package list for ncurses (different names on different distros)
            local ncurses_pkg=""
            if apt-cache show libncurses-dev >/dev/null 2>&1; then
                ncurses_pkg="libncurses-dev"
            else
                ncurses_pkg="libncurses5-dev libncursesw5-dev"
            fi
            
            fix="${fix}${C_YELLOW}sudo apt-get install -y git curl build-essential libssl-dev zlib1g-dev \\\\\n"
            fix="${fix}  libbz2-dev libreadline-dev libsqlite3-dev wget llvm ${ncurses_pkg} \\\\\n"
            fix="${fix}  xz-utils tk-dev libffi-dev liblzma-dev${C_RESET}\n\n"
            fix="${fix}${C_YELLOW}# Step 2: Install pyenv (as user '${TARGET_USER}')${C_RESET}\n"
            fix="${fix}${C_YELLOW}curl https://pyenv.run | bash${C_RESET}\n\n"
            fix="${fix}${C_YELLOW}# Step 3: Configure shell${C_RESET}\n"
            fix="${fix}${C_YELLOW}echo 'export PYENV_ROOT=\"\$HOME/.pyenv\"' >> ~/.bashrc${C_RESET}\n"
            fix="${fix}${C_YELLOW}echo 'command -v pyenv >/dev/null || export PATH=\"\$PYENV_ROOT/bin:\$PATH\"' >> ~/.bashrc${C_RESET}\n"
            fix="${fix}${C_YELLOW}echo 'eval \"\$(pyenv init -)\"' >> ~/.bashrc${C_RESET}\n"
            fix="${fix}${C_YELLOW}source ~/.bashrc${C_RESET}\n\n"
            fix="${fix}${C_YELLOW}# Step 4: Install Python 3.11${C_RESET}\n"
            fix="${fix}${C_YELLOW}pyenv install 3.11.9${C_RESET}\n"
            fix="${fix}${C_YELLOW}pyenv global 3.11.9${C_RESET}"
        fi
        record_check "Python 3.11 + venv" "false" "$fix"
    fi
}

# ---
# CHECK 2: Virtual Environment
# ---
check_venv() {
    local VENV_PATH="$TARGET_HOME/ant_venv"
    
    if [ ! -d "$VENV_PATH" ]; then
        local fix="Create the virtual environment (as user '${TARGET_USER}'):\n\n"
        if [ -n "$PYTHON311_CMD" ]; then
            fix="${fix}${C_YELLOW}${PYTHON311_CMD} -m venv ${VENV_PATH}${C_RESET}\n"
        else
            fix="${fix}${C_YELLOW}python3.11 -m venv ${VENV_PATH}${C_RESET}\n"
        fi
        fix="${fix}${C_YELLOW}${VENV_PATH}/bin/pip install --upgrade pip${C_RESET}\n"
        fix="${fix}${C_YELLOW}${VENV_PATH}/bin/pip install openant pyusb pybind11${C_RESET}"
        record_check "Virtual Environment" "false" "$fix"
        return
    fi
    
    # Check for required packages
    if [ -x "$VENV_PATH/bin/python3" ]; then
        local MISSING_PACKAGES=()
        for pkg in pybind11 usb.core openant; do
            if ! sudo -u "$TARGET_USER" "$VENV_PATH/bin/python3" -c "import $pkg" >/dev/null 2>&1; then
                MISSING_PACKAGES+=("$pkg")
            fi
        done
        
        if [ ${#MISSING_PACKAGES[@]} -eq 0 ]; then
            record_check "Virtual Environment" "true" ""
        else
            local fix="Install missing Python packages (as user '${TARGET_USER}'):\n\n"
            fix="${fix}${C_YELLOW}${VENV_PATH}/bin/pip install openant pyusb pybind11${C_RESET}"
            record_check "Virtual Environment" "false" "$fix"
        fi
    else
        record_check "Virtual Environment" "false" "Virtual environment exists but is corrupted. Remove and recreate it."
    fi
}

# ---
# CHECK 3: Qt5 Libraries
# ---
check_qt5() {
    local MISSING_LIBS=()
    local QT_LIBS=("libQt5Bluetooth.so.5" "libQt5Charts.so.5" "libQt5Multimedia.so.5" "libQt5NetworkAuth.so.5" "libQt5Positioning.so.5")
    
    for lib in "${QT_LIBS[@]}"; do
        if ! ldconfig -p 2>/dev/null | grep -q "$lib"; then
            MISSING_LIBS+=("$lib")
        fi
    done
    
    if [ ${#MISSING_LIBS[@]} -eq 0 ]; then
        record_check "Qt5 Libraries" "true" ""
    else
        local fix="Install Qt5 libraries:\n\n"
        fix="${fix}${C_YELLOW}sudo apt-get install libqt5bluetooth5 libqt5charts5 libqt5multimedia5 \\\\\n"
        fix="${fix}  libqt5networkauth5 libqt5positioning5 libqt5sql5 libqt5texttospeech5 \\\\\n"
        fix="${fix}  libqt5websockets5 libqt5xml5${C_RESET}"
        record_check "Qt5 Libraries" "false" "$fix"
    fi
}

# ---
# CHECK 4: USB Permissions
# ---
check_usb_permissions() {
    local UDEV_RULE_FILE="/etc/udev/rules.d/99-ant-usb.rules"
    local USER_TO_CHECK="${SUDO_USER:-$USER}"
    local udev_ok=false
    local group_ok=false
    
    if [ -f "$UDEV_RULE_FILE" ]; then
        udev_ok=true
    fi
    
    if groups "$USER_TO_CHECK" | grep -q '\bplugdev\b'; then
        group_ok=true
    fi
    
    if [ "$udev_ok" = true ] && [ "$group_ok" = true ]; then
        record_check "USB Permissions" "true" ""
    else
        local fix=""
        if [ "$udev_ok" = false ]; then
            fix="Create udev rule file:\n\n"
            fix="${fix}${C_YELLOW}sudo tee /etc/udev/rules.d/99-ant-usb.rules > /dev/null << 'EOF'\n"
            fix="${fix}SUBSYSTEM==\"usb\", ATTRS{idVendor}==\"0fcf\", ATTRS{idProduct}==\"100?\", MODE=\"0666\", GROUP=\"plugdev\"\n"
            fix="${fix}SUBSYSTEM==\"usb\", ATTRS{idVendor}==\"11fd\", ATTRS{idProduct}==\"0001\", MODE=\"0666\", GROUP=\"plugdev\"\n"
            fix="${fix}EOF${C_RESET}\n\n"
        fi
        if [ "$group_ok" = false ]; then
            fix="${fix}Add user to plugdev group:\n\n"
            fix="${fix}${C_YELLOW}sudo usermod -aG plugdev ${USER_TO_CHECK}${C_RESET}\n\n"
            fix="${fix}${C_RED}IMPORTANT: Log out and log back in (or reboot) for group changes to take effect.${C_RESET}\n\n"
        fi
        if [ "$udev_ok" = false ] || [ "$group_ok" = false ]; then
            fix="${fix}After completing the above, reload udev rules:\n\n"
            fix="${fix}${C_YELLOW}sudo udevadm control --reload-rules && sudo udevadm trigger${C_RESET}"
        fi
        record_check "USB Permissions" "false" "$fix"
    fi
}

# ---
# CHECK 5: ANT+ Hardware
# ---
check_ant_hardware() {
    if ! command -v lsusb >/dev/null 2>&1; then
        local fix="Install usbutils:\n\n"
        fix="${fix}${C_YELLOW}sudo apt-get install usbutils${C_RESET}"
        record_check "ANT+ Hardware" "false" "$fix"
        return
    fi
    
    if lsusb | grep -qE '0fcf:1009|0fcf:1008|11fd:0001|0fcf:1004'; then
        record_check "ANT+ Hardware" "true" ""
    else
        local fix="No ANT+ USB dongle detected.\n\n"
        fix="${fix}Please connect your ANT+ dongle (Garmin USB2 or USB-m).\n"
        fix="${fix}Compatible device IDs: 0fcf:1008, 0fcf:1009, 11fd:0001\n\n"
        fix="${fix}After connecting, run this script again."
        record_check "ANT+ Hardware" "false" "$fix"
    fi
}

# ---
# CHECK 6: Bluetooth Service
# ---
check_bluetooth() {
    local BT_SERVICE=""
    
    if systemctl list-unit-files | grep -q '^bluetooth\.service'; then
        BT_SERVICE="bluetooth.service"
    elif systemctl list-unit-files | grep -q '^hciuart\.service'; then
        BT_SERVICE="hciuart.service"
    else
        local fix="Bluetooth service not found. Install BlueZ:\n\n"
        fix="${fix}${C_YELLOW}sudo apt-get install bluez${C_RESET}"
        record_check "Bluetooth Service" "false" "$fix"
        return
    fi
    
    if systemctl is-active --quiet "$BT_SERVICE"; then
        record_check "Bluetooth Service" "true" ""
    else
        local fix="Bluetooth service ('${BT_SERVICE}') is not running.\n\n"
        fix="${fix}${C_YELLOW}# Step 1: Try to start and enable the service${C_RESET}\n"
        fix="${fix}${C_YELLOW}sudo systemctl start ${BT_SERVICE} && sudo systemctl enable ${BT_SERVICE}${C_RESET}\n\n"
        fix="${fix}${C_YELLOW}# Step 2: Wait a few seconds, then run this script again${C_RESET}\n\n"
        fix="${fix}${C_YELLOW}# If it still fails, check service status:${C_RESET}\n"
        fix="${fix}${C_YELLOW}sudo systemctl status ${BT_SERVICE}${C_RESET}\n\n"
        fix="${fix}${C_YELLOW}# For detailed logs:${C_RESET}\n"
        fix="${fix}${C_YELLOW}journalctl -u ${BT_SERVICE} -n 20 --no-pager${C_RESET}"
        record_check "Bluetooth Service" "false" "$fix"
    fi
}

# --- Run all checks ---
check_python
check_venv
check_qt5
check_usb_permissions
check_ant_hardware
check_bluetooth

# --- Display Results Overview ---
echo -e "${C_CYAN}========================================================${C_RESET}"
echo -e "${C_CYAN}Check Results Overview:${C_RESET}"
echo -e "${C_CYAN}========================================================${C_RESET}"
echo

for i in "${!CHECK_NAMES[@]}"; do
    if [ "${CHECK_RESULTS[$i]}" = "true" ]; then
        echo -e "[${C_GREEN}✓${C_RESET}] ${CHECK_NAMES[$i]}"
    else
        echo -e "[${C_RED}✗${C_RESET}] ${CHECK_NAMES[$i]}"
    fi
done

echo

# --- Handle Results ---
if [ -z "$FIRST_FAILURE" ]; then
    # All checks passed
    echo -e "${C_GREEN}========================================================${C_RESET}"
    echo -e "${C_GREEN}✓ All checks passed! Your system is ready for ANT+.${C_RESET}"
    echo -e "${C_GREEN}========================================================${C_RESET}"
    exit 0
else
    # Show first failure details
    echo -e "${C_CYAN}========================================================${C_RESET}"
    echo -e "${C_RED}First Issue Found: ${FIRST_FAILURE}${C_RESET}"
    echo -e "${C_CYAN}========================================================${C_RESET}"
    echo
    echo -e "${C_CYAN}How to fix:${C_RESET}"
    echo
    echo -e "$FIRST_FAILURE_FIX"
    echo
    echo -e "${C_CYAN}========================================================${C_RESET}"
    echo -e "${C_YELLOW}After fixing this issue, run this script again to check remaining items.${C_RESET}"
    echo -e "${C_CYAN}========================================================${C_RESET}"
    exit 1
fi