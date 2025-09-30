#!/bin/bash
# -----------------------------------------------------------------------------
# QDomyos-Zwift: ANT+ Virtual Footpod Feature
# Build Environment Diagnostic Script
#
# Part of QDomyos-Zwift project: https://github.com/cagnulein/qdomyos-zwift
# Contributor(s): bassai-sho
# Licensed under GPL-3.0 - see project repository for full license
#
# This script is a diagnostic tool designed to validate a user's complete
# setup for building the project with ANT+ support. It checks Git submodules,
# Python environment, project file configuration, and USB permissions.
# -----------------------------------------------------------------------------

# --- Color Definitions for clearer output ---
C_GREEN="\033[0;32m"
C_RED="\033[0;31m"
C_YELLOW="\033[0;33m"
C_BLUE="\033[0;34m"
C_RESET="\033[0m"

# --- Status tracking for build-critical checks ---
ALL_CHECKS_PASSED=true

# ---
# INITIAL VALIDATION: Find the Project Root Directory
# ---
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)
PROJECT_ROOT="$SCRIPT_DIR"
while [[ "$PROJECT_ROOT" != "/" ]]; do
    if [[ -d "$PROJECT_ROOT/.git" && -f "$PROJECT_ROOT/src/qdomyos-zwift.pro" ]]; then
        break
    fi
    PROJECT_ROOT=$(dirname "$PROJECT_ROOT")
done

if [[ "$PROJECT_ROOT" == "/" ]]; then
    echo -e "${C_RED}✗ FATAL ERROR:${C_RESET} Could not find the project root directory." >&2
    exit 1
fi

echo -e "${C_BLUE}--- QZ Python Build & Runtime Diagnostic ---${C_RESET}"
echo "Project Root Detected: ${PROJECT_ROOT}"
echo

# ---
# CHECK 1: Git Submodules Initialized
# ---
SUBMODULES_TO_CHECK=("src/smtpclient" "src/qmdnsengine" "tst/googletest")
SUBMODULES_OK=true
echo -e "${C_YELLOW}[1/12] Checking if Git submodules are initialized...${C_RESET}"
for submodule in "${SUBMODULES_TO_CHECK[@]}"; do
    SUBMODULE_PATH="$PROJECT_ROOT/$submodule"
    if [ ! -d "$SUBMODULE_PATH" ] || [ -z "$(ls -A "$SUBMODULE_PATH")" ]; then
        echo -e "${C_RED}  - Submodule not initialized:${C_RESET} ${submodule}"
        SUBMODULES_OK=false
    fi
done

if [ "$SUBMODULES_OK" = true ]; then
  echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} All required Git submodules are present."
else
  echo -e "  ${C_YELLOW}Suggestion:${C_RESET} Run the following command from your project directory:"
  echo -e "  ${C_YELLOW}Command:${C_RESET} cd '$PROJECT_ROOT' && git submodule update --init src/smtpclient src/qmdnsengine tst/googletest"
  ALL_CHECKS_PASSED=false
fi
echo

# ---
# CHECK 2: Project Configuration File (.ant_venv_path)
# ---
CONFIG_FILE="$PROJECT_ROOT/src/.ant_venv_path"
echo -e "${C_YELLOW}[2/12] Checking for project configuration file...${C_RESET}"
if [ ! -f "$CONFIG_FILE" ]; then
  echo -e "${C_RED}✗ FAILURE:${C_RESET} The configuration file '.ant_venv_path' is missing in '$PROJECT_ROOT/src/'."
  ALL_CHECKS_PASSED=false
else
  VENV_PYTHON_EXE=$(cat "$CONFIG_FILE")
  echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} Found configuration file."

  # --- Checks dependent on finding the venv ---
  echo -e "${C_YELLOW}[3/12] Verifying Python interpreter and packages...${C_RESET}"
  if [ ! -x "$VENV_PYTHON_EXE" ]; then
    echo -e "${C_RED}✗ FAILURE:${C_RESET} The Python interpreter is not executable: ${VENV_PYTHON_EXE}"
    ALL_CHECKS_PASSED=false
  elif ! "$VENV_PYTHON_EXE" -c "import pybind11, usb.core, openant" >/dev/null 2>&1; then
    echo -e "${C_RED}✗ FAILURE:${C_RESET} One or more Python packages (pybind11, pyusb, openant) are missing."
    echo -e "  ${C_YELLOW}Suggestion:${C_RESET} Activate your venv and run 'pip install pybind11 pyusb openant'."
    ALL_CHECKS_PASSED=false
  else
    echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} Python interpreter and all required packages are valid."
  fi
fi
echo

# ---
# CHECK 4: Main .pro file includes the ANT+ .pri file
# ---
QMAKE_PRO_FILE="$PROJECT_ROOT/src/qdomyos-zwift.pro"
ANT_INCLUDE_LINE="include(devices/antlinux/antlinux.pri)"
echo -e "${C_YELLOW}[4/12] Verifying main project file includes ANT+ config...${C_RESET}"
if [ ! -f "$QMAKE_PRO_FILE" ]; then
    echo -e "${C_RED}✗ FAILURE:${C_RESET} Main project file not found: ${QMAKE_PRO_FILE}"
    ALL_CHECKS_PASSED=false
elif grep -qF "$ANT_INCLUDE_LINE" "$QMAKE_PRO_FILE"; then
    echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} 'qdomyos-zwift.pro' correctly includes 'antlinux.pri'."
else
    echo -e "${C_RED}✗ FAILURE:${C_RESET} The main project file is missing the include for 'antlinux.pri'."
    echo -e "  ${C_YELLOW}Suggestion:${C_RESET} Add the following line to the end of '$QMAKE_PRO_FILE':"
    echo -e "  ${C_YELLOW}Line:${C_RESET} $ANT_INCLUDE_LINE"
    ALL_CHECKS_PASSED=false
fi
echo

# ---
# CHECK 5: Verifying antlinux.pri for LTO fix
# ---
ANT_PRI_FILE="$PROJECT_ROOT/src/devices/antlinux/antlinux.pri"
echo -e "${C_YELLOW}[5/12] Verifying 'antlinux.pri' for per-file LTO fix...${C_RESET}"
if [ ! -f "$ANT_PRI_FILE" ]; then
  echo -e "${C_RED}✗ FAILURE:${C_RESET} The ANT+ project file was not found: ${ANT_PRI_FILE}"
  ALL_CHECKS_PASSED=false
elif grep -q -- '$$OBJECTS_DIR/AntWorker.o.CXXFLAGS += -fno-lto' "$ANT_PRI_FILE"; then
  echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} The required per-file LTO disable flag is present in 'antlinux.pri'."
else
  echo -e "${C_RED}✗ FAILURE:${C_RESET} The 'antlinux.pri' file is missing the per-file LTO fix."
  echo -e "  ${C_YELLOW}Suggestion:${C_RESET} Ensure 'antlinux.pri' contains '$$OBJECTS_DIR/AntWorker.o.CXXFLAGS += -fno-lto'."
  ALL_CHECKS_PASSED=false
fi
echo

# ---
# CHECK 6: ccache for Faster Compilation (Optional)
# ---
echo -e "${C_YELLOW}[6/12] Checking for ccache (optional)...${C_RESET}"
if command -v ccache >/dev/null 2>&1; then
  echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} Found ccache. Project rebuilds will be faster."
else
  echo -e "${C_YELLOW}ℹ INFO:${C_RESET} ccache not found. For faster rebuilds, consider installing it ('sudo apt-get install ccache')."
fi
echo

# ---
# CHECK 7: System Swap Space
# ---
WARN_SWAP_MB=1024; IDEAL_SWAP_MB=2048
echo -e "${C_YELLOW}[7/12] Checking system swap space...${C_RESET}"
CURRENT_SWAP_MB=$(free -m | awk '/^Swap:/ {print $2}'); CURRENT_SWAP_MB=${CURRENT_SWAP_MB:-0}
if [ "$CURRENT_SWAP_MB" -ge "$IDEAL_SWAP_MB" ]; then
  echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} Found ${CURRENT_SWAP_MB}MB of swap space (ideal)."
elif [ "$CURRENT_SWAP_MB" -lt "$WARN_SWAP_MB" ]; then
  echo -e "${C_YELLOW}⚠ WARNING:${C_RESET} Found only ${CURRENT_SWAP_MB}MB of swap space. Compilation may fail on low-memory systems."
  echo -e "  ${C_YELLOW}Suggestion:${C_RESET} On Raspberry Pi OS / Debian, edit '/etc/dphys-swapfile' to set 'CONF_SWAPSIZE=2048' and restart the service."
else
  echo -e "${C_YELLOW}ℹ INFO:${C_RESET} Found ${CURRENT_SWAP_MB}MB of swap. 2048MB is recommended for best performance."
fi
echo

# ---
# CHECK 8, 9, 10: ANT+ Hardware and USB Permissions
# ---
UDEV_RULE_FILE="/etc/udev/rules.d/99-ant-usb.rules"
echo -e "${C_YELLOW}[8/12] Checking for connected ANT+ USB Stick...${C_RESET}"
if ! command -v lsusb >/dev/null 2>&1; then
  echo -e "${C_YELLOW}ℹ INFO:${C_RESET} 'lsusb' not found. Skipping hardware and permission checks."
else
  ANT_DEVICE_INFO=$(lsusb | grep -E --color=never '0fcf:1009|0fcf:1008|11fd:0001')
  if [ -n "$ANT_DEVICE_INFO" ]; then
    echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} Found ANT+ compatible USB stick:"
    echo "  ${ANT_DEVICE_INFO}"

    BUS=$(echo "$ANT_DEVICE_INFO" | awk '{print $2}')
    DEVICE=$(echo "$ANT_DEVICE_INFO" | awk '{print $4}' | sed 's/://')
    DEVICE_PATH="/dev/bus/usb/$BUS/$DEVICE"

    echo -e "\n${C_YELLOW}[9/12] Checking udev rule and group membership...${C_RESET}"
    
    # --- CORRECTED LOGIC: Check the original user, not 'root' ---
    if [ -n "$SUDO_USER" ]; then
        USER_TO_CHECK="$SUDO_USER"
    else
        USER_TO_CHECK="$USER"
    fi
    echo -e "  (Checking group membership for user: ${C_BLUE}${USER_TO_CHECK}${C_RESET})"
    
    UDEV_OK=false; GROUP_OK=false
    if [ -f "$UDEV_RULE_FILE" ]; then UDEV_OK=true; else echo -e "${C_RED}  - udev rule file missing:${C_RESET} ${UDEV_RULE_FILE}"; fi
    if groups "$USER_TO_CHECK" | grep -q '\bplugdev\b'; then GROUP_OK=true; else echo -e "${C_RED}  - User '${USER_TO_CHECK}' is not a member of the 'plugdev' group.${C_RESET}"; fi
    
    if [ "$UDEV_OK" = true ] && [ "$GROUP_OK" = true ]; then
      echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} udev rule file exists and user '${USER_TO_CHECK}' is in 'plugdev' group."
    else
      ALL_CHECKS_PASSED=false
    fi

    echo -e "\n${C_YELLOW}[10/12] Verifying LIVE USB device permissions...${C_RESET}"
    if [ -e "$DEVICE_PATH" ]; then
      PERMS=$(ls -l "$DEVICE_PATH")
      echo "  Device path: $DEVICE_PATH"
      echo "  Permissions: $PERMS"
      if [[ "$PERMS" == *"$USER_TO_CHECK"* || "$PERMS" == *"plugdev"* ]]; then
        echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} Live permissions are correct (user or group ownership)."
      else
        echo -e "${C_RED}✗ FAILURE:${C_RESET} Live permissions are incorrect (likely owned by root:root)."
        echo -e "  ${C_YELLOW}Suggestion:${C_RESET} Your udev rule may be incorrect or needs to be reloaded."
        echo -e "  ${C_YELLOW}Commands:${C_RESET} sudo udevadm control --reload-rules && sudo udevadm trigger"
        ALL_CHECKS_PASSED=false
      fi
    else
      echo -e "${C_RED}✗ FAILURE:${C_RESET} Could not find device file at ${DEVICE_PATH}. This should not happen if lsusb found it."
      ALL_CHECKS_PASSED=false
    fi
  else
    echo -e "${C_YELLOW}⚠ WARNING:${C_RESET} No ANT+ USB stick detected. Cannot verify runtime permissions."
  fi
fi
echo

# ---
# CHECK 11: QZ Configuration File (Runtime Check)
# ---
echo -e "${C_YELLOW}[11/12] Checking for QZ Configuration File...${C_RESET}"
# When run with sudo, the config file that matters is the root user's.
ROOT_CONFIG_FILE="/root/.config/Roberto Viola/qDomyos-Zwift.conf"
if [ "$EUID" -ne 0 ]; then
    echo -e "${C_YELLOW}ℹ INFO:${C_RESET} Not running as root. Cannot check for root's config file at '${ROOT_CONFIG_FILE}'."
    echo -e "  Remember to run the main application with 'sudo'."
else
    if [ -f "$ROOT_CONFIG_FILE" ]; then
        echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} Found root's config file at '${ROOT_CONFIG_FILE}'."
        echo -e "  Ensure this file contains your model-specific settings (e.g., proform_treadmill_705_cst=true)."
    else
        echo -e "${C_YELLOW}⚠ WARNING:${C_RESET} Root's config file not found at '${ROOT_CONFIG_FILE}'."
        echo -e "  ${C_YELLOW}NOTE:${C_RESET} This is normal for a first-time setup."
        echo -e "  The application will create a default file here on its first run."
        echo -e "  You will need to edit it with 'sudo' to enable your specific treadmill model."
    fi
fi
echo

# ---
# CHECK 12: Bluetooth Service (Runtime Check)
# ---
echo -e "${C_YELLOW}[12/12] Checking for Bluetooth service...${C_RESET}"
if systemctl is-active --quiet bluetooth; then
  echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} The 'bluetooth' service is active."
else
  echo -e "${C_RED}✗ FAILURE:${C_RESET} The 'bluetooth' service is not running. The main application will fail."
  echo -e "  ${C_YELLOW}Suggestion:${C_RESET} Run 'sudo systemctl start bluetooth' and 'sudo systemctl enable bluetooth'."
  ALL_CHECKS_PASSED=false
fi
echo

# ---
# FINAL SUMMARY
# ---
echo -e "${C_BLUE}--- Diagnostic Complete ---${C_RESET}"
if [ "$ALL_CHECKS_PASSED" = true ]; then
  echo -e "${C_GREEN}✓ All critical checks passed! You are ready to build the project.${C_RESET}"
else
  echo -e "${C_RED}✗ One or more critical checks failed. Please review the suggestions above before building.${C_RESET}"
fi