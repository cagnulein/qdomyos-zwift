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
C_BLUE="\033[0;34m"
C_RESET="\033[0m"

# --- GitHub README URL for reference ---
README_URL="https://github.com/cagnulein/qdomyos-zwift/blob/master/README.md"

# --- Status tracking ---
ALL_CHECKS_PASSED=true

echo -e "${C_BLUE}--- QZ ANT+ Runtime Diagnostic ---${C_RESET}"
echo

# ---
# CHECK 1: Python 3.11 Installation
# ---
echo -e "${C_YELLOW}[1/7] Checking for Python 3.11...${C_RESET}"
if command -v python3.11 >/dev/null 2>&1; then
  PYTHON_VERSION=$(python3.11 --version)
  echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} Found $PYTHON_VERSION"
else
  echo -e "${C_RED}✗ ISSUE FOUND:${C_RESET} Python 3.11 is not installed."
  echo -e "  ${C_BLUE}How to fix:${C_RESET} Install Python 3.11 by running:"
  echo -e "  ${C_YELLOW}sudo apt-get install python3.11 python3.11-venv${C_RESET}"
  echo -e "  ${C_BLUE}More details:${C_RESET} See Step 1.1 in ${README_URL}#11-install-system-dependencies"
  ALL_CHECKS_PASSED=false
fi
echo

# ---
# CHECK 2: Virtual Environment
# ---
echo -e "${C_YELLOW}[2/7] Checking for Python virtual environment...${C_RESET}"
VENV_PATH="$HOME/ant_venv"
if [ -d "$VENV_PATH" ]; then
  echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} Found virtual environment at $VENV_PATH"
  
  # Check for required packages
  if [ -x "$VENV_PATH/bin/python3" ]; then
    MISSING_PACKAGES=()
    for pkg in pybind11 usb.core openant; do
      if ! "$VENV_PATH/bin/python3" -c "import $pkg" >/dev/null 2>&1; then
        MISSING_PACKAGES+=("$pkg")
      fi
    done
    
    if [ ${#MISSING_PACKAGES[@]} -eq 0 ]; then
      echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} All required Python packages are installed."
    else
      echo -e "${C_RED}✗ ISSUE FOUND:${C_RESET} Missing Python packages: ${MISSING_PACKAGES[*]}"
      echo -e "  ${C_BLUE}How to fix:${C_RESET} Install the missing packages by running:"
      echo -e "  ${C_YELLOW}~/ant_venv/bin/pip install openant pyusb pybind11${C_RESET}"
      echo -e "  ${C_BLUE}More details:${C_RESET} See Step 1.2 in ${README_URL}#12-create-python-virtual-environment"
      ALL_CHECKS_PASSED=false
    fi
  fi
else
  echo -e "${C_RED}✗ ISSUE FOUND:${C_RESET} Virtual environment not found at $VENV_PATH"
  echo -e "  ${C_BLUE}How to fix:${C_RESET} Create the virtual environment by running:"
  echo -e "  ${C_YELLOW}python3.11 -m venv ~/ant_venv${C_RESET}"
  echo -e "  Then install the required packages:"
  echo -e "  ${C_YELLOW}~/ant_venv/bin/pip install --upgrade pip${C_RESET}"
  echo -e "  ${C_YELLOW}~/ant_venv/bin/pip install openant pyusb pybind11${C_RESET}"
  echo -e "  ${C_BLUE}More details:${C_RESET} See Step 1.2 in ${README_URL}#12-create-python-virtual-environment"
  ALL_CHECKS_PASSED=false
fi
echo

# ---
# CHECK 3: Qt5 Libraries
# ---
echo -e "${C_YELLOW}[3/7] Checking for Qt5 libraries...${C_RESET}"
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
  echo -e "  ${C_BLUE}How to fix:${C_RESET} Install the Qt5 libraries by running:"
  echo -e "  ${C_YELLOW}sudo apt-get install libqt5bluetooth5 libqt5charts5 libqt5multimedia5 \\${C_RESET}"
  echo -e "  ${C_YELLOW}  libqt5networkauth5 libqt5positioning5 libqt5sql5 libqt5texttospeech5 \\${C_RESET}"
  echo -e "  ${C_YELLOW}  libqt5websockets5 libqt5xml5${C_RESET}"
  echo -e "  ${C_BLUE}More details:${C_RESET} See Step 1.1 in ${README_URL}#11-install-system-dependencies"
  ALL_CHECKS_PASSED=false
fi
echo

# ---
# CHECK 4: USB Permissions (udev rule)
# ---
UDEV_RULE_FILE="/etc/udev/rules.d/99-ant-usb.rules"
echo -e "${C_YELLOW}[4/7] Checking udev rule and group membership...${C_RESET}"

# Determine which user to check
if [ -n "$SUDO_USER" ]; then
    USER_TO_CHECK="$SUDO_USER"
else
    USER_TO_CHECK="$USER"
fi

UDEV_OK=false
GROUP_OK=false

if [ -f "$UDEV_RULE_FILE" ]; then
  echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} udev rule file exists at $UDEV_RULE_FILE"
  UDEV_OK=true
else
  echo -e "${C_RED}✗ ISSUE FOUND:${C_RESET} udev rule file is missing: $UDEV_RULE_FILE"
  echo -e "  ${C_BLUE}How to fix:${C_RESET} Create the udev rule by running:"
  echo -e "  ${C_YELLOW}sudo tee /etc/udev/rules.d/99-ant-usb.rules > /dev/null << 'EOF'${C_RESET}"
  echo -e "  ${C_YELLOW}SUBSYSTEM==\"usb\", ATTRS{idVendor}==\"0fcf\", ATTRS{idProduct}==\"100?\", MODE=\"0666\", GROUP=\"plugdev\"${C_RESET}"
  echo -e "  ${C_YELLOW}SUBSYSTEM==\"usb\", ATTRS{idVendor}==\"11fd\", ATTRS{idProduct}==\"0001\", MODE=\"0666\", GROUP=\"plugdev\"${C_RESET}"
  echo -e "  ${C_YELLOW}EOF${C_RESET}"
  echo -e "  ${C_BLUE}More details:${C_RESET} See Step 1.3 in ${README_URL}#13-configure-usb-permissions"
fi

if groups "$USER_TO_CHECK" | grep -q '\bplugdev\b'; then
  echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} User '$USER_TO_CHECK' is in 'plugdev' group."
  GROUP_OK=true
else
  echo -e "${C_RED}✗ ISSUE FOUND:${C_RESET} User '$USER_TO_CHECK' is not in 'plugdev' group."
  echo -e "  ${C_BLUE}How to fix:${C_RESET} Add your user to the plugdev group by running:"
  echo -e "  ${C_YELLOW}sudo usermod -aG plugdev $USER_TO_CHECK${C_RESET}"
  echo -e "  Then log out and back in, or reboot for the change to take effect."
  echo -e "  ${C_BLUE}More details:${C_RESET} See Step 1.3 in ${README_URL}#13-configure-usb-permissions"
fi

if [ "$UDEV_OK" = false ] || [ "$GROUP_OK" = false ]; then
  ALL_CHECKS_PASSED=false
fi
echo

# ---
# CHECK 5: ANT+ Hardware Detection
# ---
echo -e "${C_YELLOW}[5/7] Checking for connected ANT+ USB dongle...${C_RESET}"
if ! command -v lsusb >/dev/null 2>&1; then
  echo -e "${C_YELLOW}ℹ INFO:${C_RESET} 'lsusb' command not found. Skipping hardware detection."
  echo -e "  To install: ${C_YELLOW}sudo apt-get install usbutils${C_RESET}"
else
  ANT_DEVICE_INFO=$(lsusb | grep -E --color=never '0fcf:1009|0fcf:1008|11fd:0001')
  if [ -n "$ANT_DEVICE_INFO" ]; then
    echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} Found ANT+ compatible USB dongle:"
    echo "  $ANT_DEVICE_INFO"
    
    # Check live permissions
    BUS=$(echo "$ANT_DEVICE_INFO" | awk '{print $2}')
    DEVICE=$(echo "$ANT_DEVICE_INFO" | awk '{print $4}' | sed 's/://')
    DEVICE_PATH="/dev/bus/usb/$BUS/$DEVICE"
    
    if [ -e "$DEVICE_PATH" ]; then
      PERMS=$(ls -l "$DEVICE_PATH")
      if [[ "$PERMS" == *"$USER_TO_CHECK"* || "$PERMS" == *"plugdev"* ]]; then
        echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} Device permissions look correct."
      else
        echo -e "${C_YELLOW}⚠ NEEDS ATTENTION:${C_RESET} Device permissions may need adjustment."
        echo -e "  Current permissions: $PERMS"
        echo -e "  ${C_BLUE}How to fix:${C_RESET} Try reloading the udev rules:"
        echo -e "  ${C_YELLOW}sudo udevadm control --reload-rules && sudo udevadm trigger${C_RESET}"
        echo -e "  Or unplug and replug your ANT+ dongle."
        echo -e "  If you just set up udev rules, a reboot may be needed."
      fi
    fi
  else
    echo -e "${C_YELLOW}ℹ INFO:${C_RESET} No ANT+ USB dongle detected."
    echo -e "  Please connect your ANT+ dongle (Garmin USB2 0fcf:1008 or USB-m 0fcf:1009)."
    echo -e "  If already connected, check if it's properly seated in the USB port."
  fi
fi
echo

# ---
# CHECK 6: Bluetooth Service
# ---
echo -e "${C_YELLOW}[6/7] Checking for Bluetooth service...${C_RESET}"
if systemctl is-active --quiet bluetooth 2>/dev/null; then
  echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} The 'bluetooth' service is active."
else
  echo -e "${C_YELLOW}⚠ NEEDS ATTENTION:${C_RESET} The 'bluetooth' service is not running."
  echo -e "  The application may not work properly for treadmill communication."
  echo -e "  ${C_BLUE}How to fix:${C_RESET} Start and enable the bluetooth service:"
  echo -e "  ${C_YELLOW}sudo systemctl start bluetooth${C_RESET}"
  echo -e "  ${C_YELLOW}sudo systemctl enable bluetooth${C_RESET}"
fi
echo

# ---
# CHECK 7: QZ Configuration File
# ---
echo -e "${C_YELLOW}[7/7] Checking for QZ configuration file...${C_RESET}"
ROOT_CONFIG_FILE="/root/.config/Roberto Viola/qDomyos-Zwift.conf"
if [ "$EUID" -ne 0 ]; then
    echo -e "${C_YELLOW}ℹ INFO:${C_RESET} Not running as root, so cannot check root's configuration file."
    echo -e "  This is normal - the application creates this file on its first run with sudo."
    echo -e "  Remember to run the application with 'sudo' to access hardware."
else
    if [ -f "$ROOT_CONFIG_FILE" ]; then
        echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} Found root's config file at:"
        echo -e "  $ROOT_CONFIG_FILE"
        echo -e "  Make sure it contains your treadmill model settings."
        echo -e "  For example: ${C_YELLOW}proform_treadmill_705_cst=true${C_RESET}"
    else
        echo -e "${C_YELLOW}ℹ INFO:${C_RESET} Root's config file not yet created (normal for first-time setup)."
        echo -e "  The application will create a default file at:"
        echo -e "  $ROOT_CONFIG_FILE"
        echo -e "  After the first run, you'll need to edit it with 'sudo' to enable your specific treadmill model."
        echo -e "  ${C_BLUE}More details:${C_RESET} See Troubleshooting in ${README_URL}#troubleshooting"
    fi
fi
echo

# ---
# FINAL SUMMARY
# ---
echo -e "${C_BLUE}--- Diagnostic Complete ---${C_RESET}"
if [ "$ALL_CHECKS_PASSED" = true ]; then
  echo -e "${C_GREEN}✓ All checks passed! Your runtime environment is ready.${C_RESET}"
  echo -e "  You can now proceed to Step 2 to download and install the binary."
  echo -e "  ${C_BLUE}Next steps:${C_RESET} ${README_URL}#step-2-download-and-install-binary"
else
  echo -e "${C_YELLOW}⚠ Some issues were found that need your attention.${C_RESET}"
  echo -e "  Please review the suggestions above and follow the 'How to fix' instructions."
  echo -e "  After making the changes, you can run this script again to verify."
  echo -e "  ${C_BLUE}Full guide:${C_RESET} ${README_URL}"
fi