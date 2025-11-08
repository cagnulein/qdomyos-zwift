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

# --- Color Definitions for clearer output ---
C_GREEN="\033[0;32m"
C_RED="\033[0;31m"
C_YELLOW="\033[0;33m"
C_CYAN="\033[0;36m"
C_RESET="\033[0m"

# --- Status tracking ---
ALL_CHECKS_PASSED=true

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

echo -e "${C_CYAN}--- QZ ANT+ Runtime Diagnostic (running for user: $TARGET_USER) ---${C_RESET}"
echo

# ---
# CHECK 1: Python 3.11 and Venv Module Installation
# ---
echo -e "${C_YELLOW}[1/6] Checking for Python 3.11 and venv module...${C_RESET}"
PYTHON_OK=false
VENV_PKG_OK=false
PYTHON311_CMD=""

# Priority 1: Check for pyenv installation for the target user
if [ -d "$TARGET_HOME/.pyenv/versions/3.11.9/bin" ]; then
    PYTHON311_CMD="$TARGET_HOME/.pyenv/versions/3.11.9/bin/python3.11"
# Priority 2: Check the system path
elif command -v python3.11 >/dev/null 2>&1; then
    PYTHON311_CMD="python3.11"
fi

if [ -n "$PYTHON311_CMD" ]; then
  PYTHON_VERSION=$($PYTHON311_CMD --version)
  echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} Found $PYTHON_VERSION"
  PYTHON_OK=true
  
  # Check for venv module
  if ! $PYTHON311_CMD -c "import ensurepip" >/dev/null 2>&1; then
    echo -e "${C_RED}✗ ISSUE FOUND:${C_RESET} Python 3.11 is present, but the 'venv' module is missing."
    echo -e "  ${C_CYAN}How to fix:${C_RESET} Install the corresponding venv package:"
    echo -e "  ${C_YELLOW}sudo apt-get install python3.11-venv${C_RESET}"
    ALL_CHECKS_PASSED=false
  else
    echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} Python 3.11 venv module is available."
    VENV_PKG_OK=true
  fi
else
  echo -e "${C_RED}✗ ISSUE FOUND:${C_RESET} Python 3.11 is not installed."
  ALL_CHECKS_PASSED=false
  
  echo -e "  ${C_CYAN}How to fix:${C_RESET}"
  if apt-cache show python3.11-venv >/dev/null 2>&1; then
    echo -e "  Your system provides python3.11 via the package manager. This is the recommended method."
    echo -e "  Run the following command to install:"
    echo -e "  ${C_YELLOW}sudo apt-get install python3.11 python3.11-venv${C_RESET}"
  else
    echo -e "  Your system's package manager does not provide python3.11."
    echo -e "  This can occur on older distributions (e.g., Ubuntu 20.04, Debian Bullseye)"
    echo -e "  or newer distributions with only Python 3.12+ (e.g., Debian Trixie)."
    echo -e "  You will need to install it using 'pyenv'."
    echo
    echo -e "  ${C_YELLOW}# Step 1: Install prerequisites (including git and curl for pyenv installer)${C_RESET}"
    echo -e "  ${C_YELLOW}sudo apt-get install -y git curl build-essential libssl-dev zlib1g-dev libbz2-dev"
    echo -e "    libreadline-dev libsqlite3-dev wget llvm libncurses5-dev"
    echo -e "    libncursesw5-dev xz-utils tk-dev libffi-dev liblzma-dev${C_RESET}"
    echo
    echo -e "  ${C_YELLOW}# Step 2: Install pyenv (as user '${TARGET_USER}')${C_RESET}"
    echo -e "  ${C_YELLOW}curl https://pyenv.run | bash${C_RESET}"
    echo
    echo -e "  ${C_YELLOW}# Step 3: Configure shell and install Python 3.11${C_RESET}"
    echo -e "  ${C_YELLOW}echo 'export PYENV_ROOT=\"\$HOME/.pyenv\"' >> ~/.bashrc${C_RESET}"
    echo -e "  ${C_YELLOW}echo 'command -v pyenv >/dev/null || export PATH=\"\$PYENV_ROOT/bin:\$PATH\"' >> ~/.bashrc${C_RESET}"
    echo -e "  ${C_YELLOW}echo 'eval \"\$(pyenv init -)\"' >> ~/.bashrc${C_RESET}"
    echo -e "  ${C_YELLOW}source ~/.bashrc${C_RESET}"
    echo -e "  ${C_YELLOW}pyenv install 3.11.9${C_RESET}"
    echo -e "  ${C_YELLOW}pyenv global 3.11.9${C_RESET}"
  fi
fi
echo

# ---
# CHECK 2: Virtual Environment
# ---
echo -e "${C_YELLOW}[2/6] Checking for Python virtual environment...${C_RESET}"

VENV_PATH="$TARGET_HOME/ant_venv"

if [ -d "$VENV_PATH" ]; then
  echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} Found virtual environment at $VENV_PATH"
  
  if [ -x "$VENV_PATH/bin/python3" ]; then
    MISSING_PACKAGES=()
    for pkg in pybind11 usb.core openant; do
      if ! sudo -u "$TARGET_USER" "$VENV_PATH/bin/python3" -c "import $pkg" >/dev/null 2>&1; then
        MISSING_PACKAGES+=("$pkg")
      fi
    done
    
    if [ ${#MISSING_PACKAGES[@]} -eq 0 ]; then
      echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} All required Python packages are installed."
    else
      echo -e "${C_RED}✗ ISSUE FOUND:${C_RESET} Missing Python packages: ${MISSING_PACKAGES[*]}"
      echo -e "  ${C_CYAN}How to fix:${C_RESET} Run the following commands as the user '${TARGET_USER}':"
      echo -e "  ${C_YELLOW}$VENV_PATH/bin/pip install openant pyusb pybind11${C_RESET}"
      ALL_CHECKS_PASSED=false
    fi
  fi
else
  echo -e "${C_RED}✗ ISSUE FOUND:${C_RESET} Virtual environment not found at $VENV_PATH"
  echo -e "  ${C_CYAN}How to fix:${C_RESET} Run the following commands as the user '${TARGET_USER}':"
  if [ -n "$PYTHON311_CMD" ]; then
    echo -e "  ${C_YELLOW}${PYTHON311_CMD} -m venv ${VENV_PATH}${C_RESET}"
  else
    echo -e "  ${C_YELLOW}python3.11 -m venv ${VENV_PATH}${C_RESET}"
  fi
  echo -e "  ${C_YELLOW}${VENV_PATH}/bin/pip install --upgrade pip${C_RESET}"
  echo -e "  ${C_YELLOW}${VENV_PATH}/bin/pip install openant pyusb pybind11${C_RESET}"
  ALL_CHECKS_PASSED=false
fi
echo

# ---
# CHECK 3: Qt5 Libraries
# ---
echo -e "${C_YELLOW}[3/6] Checking for Qt5 libraries...${C_RESET}"
MISSING_LIBS=()
QT_LIBS=("libQt5Bluetooth.so.5" "libQt5Charts.so.5" "libQt5Multimedia.so.5" "libQt5NetworkAuth.so.5" "libQt5Positioning.so.5")
for lib in "${QT_LIBS[@]}"; do
  if ! ldconfig -p 2>/dev/null | grep -q "$lib"; then
    MISSING_LIBS+=("$lib")
  fi
done

if [ ${#MISSING_LIBS[@]} -eq 0 ]; then
  echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} All required Qt5 libraries are installed."
else
  echo -e "${C_RED}✗ ISSUE FOUND:${C_RESET} Missing Qt5 libraries: ${MISSING_LIBS[*]}"
  echo -e "  ${C_CYAN}How to fix:${C_RESET} Install the Qt5 libraries by running:"
  echo -e "  ${C_YELLOW}sudo apt-get install libqt5bluetooth5 libqt5charts5 libqt5multimedia5"
  echo -e "    libqt5networkauth5 libqt5positioning5 libqt5sql5 libqt5texttospeech5"
  echo -e "    libqt5websockets5 libqt5xml5${C_RESET}"
  ALL_CHECKS_PASSED=false
fi
echo

# ---
# CHECK 4: USB Permissions (udev rule)
# ---
UDEV_RULE_FILE="/etc/udev/rules.d/99-ant-usb.rules"
echo -e "${C_YELLOW}[4/6] Checking udev rule and group membership...${C_RESET}"

if [ -n "$SUDO_USER" ]; then USER_TO_CHECK="$SUDO_USER"; else USER_TO_CHECK="$USER"; fi

UDEV_OK=false
GROUP_OK=false

if [ -f "$UDEV_RULE_FILE" ]; then
  echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} udev rule file exists."
  UDEV_OK=true
else
  echo -e "${C_RED}✗ ISSUE FOUND:${C_RESET} udev rule file is missing."
  echo -e "  ${C_CYAN}How to fix:${C_RESET} Copy and paste the entire block below to create the rule file:"
  echo
  cat << 'FIX_UDEV'
  sudo tee /etc/udev/rules.d/99-ant-usb.rules > /dev/null << 'EOF'
SUBSYSTEM=="usb", ATTRS{idVendor}=="0fcf", ATTRS{idProduct}=="100?", MODE="0666", GROUP="plugdev"
SUBSYSTEM=="usb", ATTRS{idVendor}=="11fd", ATTRS{idProduct}=="0001", MODE="0666", GROUP="plugdev"
EOF
FIX_UDEV
  echo
fi

if groups "$USER_TO_CHECK" | grep -q '\bplugdev\b'; then
  echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} User '$USER_TO_CHECK' is in 'plugdev' group."
  GROUP_OK=true
else
  echo -e "${C_RED}✗ ISSUE FOUND:${C_RESET} User '$USER_TO_CHECK' is not in 'plugdev' group."
  echo -e "  ${C_CYAN}How to fix:${C_RESET} Add your user to the group:"
  echo -e "  ${C_YELLOW}sudo usermod -aG plugdev $USER_TO_CHECK${C_RESET}"
  echo
  echo -e "  ${C_YELLOW}IMPORTANT:${C_RESET} You must log out and log back in, or reboot, for this change to take effect."
fi

if [ "$UDEV_OK" = false ] || [ "$GROUP_OK" = false ]; then
    if [ "$UDEV_OK" = true ]; then
        echo -e "  After fixing the group, reload the rules:"
    else
        echo -e "  After creating the file and adding the user to the group, reload the rules:"
    fi
    echo -e "  ${C_YELLOW}sudo udevadm control --reload-rules && sudo udevadm trigger${C_RESET}"
    ALL_CHECKS_PASSED=false
fi
echo

# ---
# CHECK 5: ANT+ Hardware Detection
# ---
echo -e "${C_YELLOW}[5/6] Checking for connected ANT+ USB dongle...${C_RESET}"
if ! command -v lsusb >/dev/null 2>&1; then
  echo -e "${C_RED}✗ ISSUE FOUND:${C_RESET} 'lsusb' command not found."
  echo -e "  ${C_CYAN}How to fix:${C_RESET} Run: ${C_YELLOW}sudo apt-get install usbutils${C_RESET}"
  ALL_CHECKS_PASSED=false
else
  ANT_DEVICE_INFO=$(lsusb | grep -E --color=never '0fcf:1009|0fcf:1008|11fd:0001|0fcf:1004')
  if [ -n "$ANT_DEVICE_INFO" ]; then
    echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} Found ANT+ compatible USB dongle:"
    echo "  $ANT_DEVICE_INFO"
  else
    echo -e "${C_RED}✗ ISSUE FOUND:${C_RESET} No ANT+ USB dongle detected."
    echo -e "  ${C_CYAN}How to fix:${C_RESET} Please connect your ANT+ dongle."
    ALL_CHECKS_PASSED=false
  fi
fi
echo

# ---
# CHECK 6: Bluetooth Service
# ---
echo -e "${C_YELLOW}[6/6] Checking for Bluetooth service...${C_RESET}"

BT_SERVICE=""
if systemctl list-unit-files | grep -q '^bluetooth\.service'; then
    BT_SERVICE="bluetooth.service"
elif systemctl list-unit-files | grep -q '^hciuart\.service'; then
    BT_SERVICE="hciuart.service"
else
    echo -e "${C_RED}✗ CRITICAL ISSUE:${C_RESET} Could not find 'bluetooth.service' or 'hciuart.service'."
    echo -e "  The core Bluetooth service is missing from your system."
    echo -e "  ${C_CYAN}How to fix:${C_RESET} Install the BlueZ package:"
    echo -e "  ${C_YELLOW}sudo apt-get install bluez${C_RESET}"
    ALL_CHECKS_PASSED=false
    BT_SERVICE="none"
fi

if [ "$BT_SERVICE" != "none" ]; then
    if systemctl is-active --quiet "$BT_SERVICE"; then
        echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} The '$BT_SERVICE' is active."
    else
        echo -e "${C_RED}✗ ISSUE FOUND:${C_RESET} The Bluetooth service ('$BT_SERVICE') is not running."
        echo -e "  ${C_CYAN}How to fix:${C_RESET} Start and enable the service:"
        echo
        echo -e "  ${C_YELLOW}# 1. Try to start and enable the service:${C_RESET}"
        echo -e "  ${C_YELLOW}sudo systemctl start ${BT_SERVICE} && sudo systemctl enable ${BT_SERVICE}${C_RESET}"
        echo
        echo -e "  ${C_YELLOW}# 2. Wait a few seconds, then run this script again.${C_RESET}"
        echo
        echo -e "  ${C_YELLOW}# 3. If it still fails, check the service's status for errors:${C_RESET}"
        echo -e "  ${C_YELLOW}sudo systemctl status ${BT_SERVICE}${C_RESET}"
        echo
        echo -e "  ${C_YELLOW}# 4. For more detailed logs, run:${C_RESET}"
        echo -e "  ${C_YELLOW}journalctl -u ${BT_SERVICE} -n 20 --no-pager${C_RESET}"
        ALL_CHECKS_PASSED=false
    fi
fi
echo

# ---
# FINAL SUMMARY
# ---
echo -e "${C_CYAN}--- Diagnostic Complete ---${C_RESET}"
if [ "$ALL_CHECKS_PASSED" = true ]; then
  echo -e "${C_GREEN}========================================================${C_RESET}"
  echo -e "${C_GREEN}✓ All checks passed! Your system is ready for ANT+.${C_RESET}"
  echo -e "${C_GREEN}========================================================${C_RESET}"
else
  echo -e "${C_RED}✗ One or more checks failed. Please review the issues above and follow the 'How to fix' instructions.${C_RESET}"
fi