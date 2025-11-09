#!/bin/bash
# -----------------------------------------------------------------------------
# QDomyos-Zwift: ANT+ Virtual Footpod Feature
# Build Environment Diagnostic Script
#
# Part of QDomyos-Zwift project: https://github.com/cagnulein/qdomyos-zwift
# Contributor(s): bassai-sho
# AI analysis tools (Claude, Gemini) were used to assist coding and debugging
# Licensed under GPL-3.0 - see project repository for full license
#
# This script validates a user's build environment for compiling the project
# with ANT+ support. It checks Git submodules, Python environment, and project
# file configuration.
# -----------------------------------------------------------------------------

# --- Color Definitions ---
C_GREEN="\033[0;32m"
C_RED="\033[0;31m"
C_YELLOW="\033[0;33m"
C_CYAN="\033[0;36m"
C_RESET="\033[0m"

# --- GitHub README URLs ---
README_URL="https://github.com/cagnulein/qdomyos-zwift/blob/master/README.md"
COMPILE_URL="https://github.com/cagnulein/qdomyos-zwift/blob/master/COMPILE.md"

# --- Status tracking ---
ALL_CHECKS_PASSED=true

# --- Find Project Root ---
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

echo -e "${C_CYAN}========================================================${C_RESET}"
echo -e "${C_CYAN}    QZ ANT+ Build Environment Diagnostic${C_RESET}"
echo -e "${C_CYAN}    Project Root: ${PROJECT_ROOT}${C_RESET}"
echo -e "${C_CYAN}========================================================${C_RESET}"
echo

# ---
# CHECK 1: Git Submodules
# ---
echo -e "${C_YELLOW}[1/8] Checking Git submodules...${C_RESET}"
SUBMODULES_TO_CHECK=("src/smtpclient" "src/qmdnsengine" "tst/googletest")
SUBMODULES_OK=true

for submodule in "${SUBMODULES_TO_CHECK[@]}"; do
    SUBMODULE_PATH="$PROJECT_ROOT/$submodule"
    if [ ! -d "$SUBMODULE_PATH" ] || [ -z "$(ls -A "$SUBMODULE_PATH")" ]; then
        echo -e "${C_RED}  ✗ Submodule not initialized:${C_RESET} ${submodule}"
        SUBMODULES_OK=false
    fi
done

if [ "$SUBMODULES_OK" = true ]; then
    echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} All required Git submodules are present."
else
    echo -e "  ${C_CYAN}How to fix:${C_RESET}"
    echo -e "  ${C_YELLOW}cd '$PROJECT_ROOT' && git submodule update --init src/smtpclient src/qmdnsengine tst/googletest${C_RESET}"
    ALL_CHECKS_PASSED=false
fi
echo

# ---
# CHECK 2: Project Configuration File
# ---
echo -e "${C_YELLOW}[2/8] Checking project configuration file...${C_RESET}"
CONFIG_FILE="$PROJECT_ROOT/src/.ant_venv_path"

if [ ! -f "$CONFIG_FILE" ]; then
    echo -e "${C_RED}✗ ISSUE FOUND:${C_RESET} Configuration file '.ant_venv_path' is missing."
    echo -e "  ${C_CYAN}How to fix:${C_RESET}"
    echo -e "  ${C_YELLOW}echo \"\$HOME/ant_venv/bin/python3\" > $PROJECT_ROOT/src/.ant_venv_path${C_RESET}"
    ALL_CHECKS_PASSED=false
else
    VENV_PYTHON_EXE=$(cat "$CONFIG_FILE")
    echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} Found configuration file pointing to: ${VENV_PYTHON_EXE}"
fi
echo

# ---
# CHECK 3: Python Virtual Environment
# ---
echo -e "${C_YELLOW}[3/8] Verifying Python virtual environment...${C_RESET}"

if [ -f "$CONFIG_FILE" ]; then
    VENV_PYTHON_EXE=$(cat "$CONFIG_FILE")
    
    if [ ! -x "$VENV_PYTHON_EXE" ]; then
        echo -e "${C_RED}✗ ISSUE FOUND:${C_RESET} Python interpreter not executable: ${VENV_PYTHON_EXE}"
        echo -e "  ${C_CYAN}How to fix:${C_RESET} Create the virtual environment:"
        echo -e "  ${C_YELLOW}python3 -m venv ~/ant_venv${C_RESET}"
        echo -e "  ${C_YELLOW}~/ant_venv/bin/pip install --upgrade pip${C_RESET}"
        echo -e "  ${C_YELLOW}~/ant_venv/bin/pip install pybind11 pyusb openant${C_RESET}"
        ALL_CHECKS_PASSED=false
    elif ! "$VENV_PYTHON_EXE" -c "import pybind11, usb.core, openant" >/dev/null 2>&1; then
        echo -e "${C_RED}✗ ISSUE FOUND:${C_RESET} Missing Python packages (pybind11, pyusb, openant)."
        echo -e "  ${C_CYAN}How to fix:${C_RESET}"
        echo -e "  ${C_YELLOW}$VENV_PYTHON_EXE -m pip install pybind11 pyusb openant${C_RESET}"
        ALL_CHECKS_PASSED=false
    else
        # Show Python version being used
        PYVER=$("$VENV_PYTHON_EXE" --version 2>&1)
        echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} Python interpreter valid (${PYVER})"
        echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} All required packages present (pybind11, pyusb, openant)"
    fi
fi
echo

# ---
# CHECK 4: Python Development Headers
# ---
echo -e "${C_YELLOW}[4/8] Checking Python development headers...${C_RESET}"

if [ -f "$CONFIG_FILE" ] && [ -x "$VENV_PYTHON_EXE" ]; then
    PYVER=$("$VENV_PYTHON_EXE" -c 'import sys; print(f"{sys.version_info.major}.{sys.version_info.minor}")')
    
    if dpkg -l 2>/dev/null | grep -q "python${PYVER}-dev"; then
        echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} Python ${PYVER} development headers installed."
    else
        echo -e "${C_RED}✗ ISSUE FOUND:${C_RESET} Python ${PYVER} development headers missing."
        echo -e "  ${C_CYAN}How to fix:${C_RESET}"
        echo -e "  ${C_YELLOW}sudo apt-get install python${PYVER}-dev${C_RESET}"
        echo -e "  ${C_CYAN}Note:${C_RESET} If using pyenv, development headers are already included."
        ALL_CHECKS_PASSED=false
    fi
fi
echo

# ---
# CHECK 5: Main .pro File
# ---
echo -e "${C_YELLOW}[5/8] Verifying main project file...${C_RESET}"
QMAKE_PRO_FILE="$PROJECT_ROOT/src/qdomyos-zwift.pro"
ANT_INCLUDE_LINE="include(devices/antlinux/antlinux.pri)"

if [ ! -f "$QMAKE_PRO_FILE" ]; then
    echo -e "${C_RED}✗ ISSUE FOUND:${C_RESET} Main project file not found: ${QMAKE_PRO_FILE}"
    ALL_CHECKS_PASSED=false
elif grep -qF "$ANT_INCLUDE_LINE" "$QMAKE_PRO_FILE"; then
    echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} Project file correctly includes 'antlinux.pri'."
else
    echo -e "${C_RED}✗ ISSUE FOUND:${C_RESET} Project file missing 'antlinux.pri' include."
    echo -e "  ${C_CYAN}How to fix:${C_RESET}"
    echo -e "  ${C_YELLOW}echo 'include(devices/antlinux/antlinux.pri)' >> $QMAKE_PRO_FILE${C_RESET}"
    ALL_CHECKS_PASSED=false
fi
echo

# ---
# CHECK 6: ANT+ .pri File
# ---
echo -e "${C_YELLOW}[6/8] Verifying 'antlinux.pri' for LTO fix...${C_RESET}"
ANT_PRI_FILE="$PROJECT_ROOT/src/devices/antlinux/antlinux.pri"

if [ ! -f "$ANT_PRI_FILE" ]; then
    echo -e "${C_RED}✗ ISSUE FOUND:${C_RESET} ANT+ project file not found: ${ANT_PRI_FILE}"
    ALL_CHECKS_PASSED=false
elif grep -q -- '$$OBJECTS_DIR/AntWorker.o.CXXFLAGS += -fno-lto' "$ANT_PRI_FILE"; then
    echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} Required per-file LTO disable flag present."
else
    echo -e "${C_YELLOW}⚠ NOTICE:${C_RESET} LTO fix may be missing from 'antlinux.pri'."
    echo -e "  This is usually handled by the repository."
    echo -e "  If compilation fails with LTO errors, ensure 'antlinux.pri' contains:"
    echo -e "  ${C_YELLOW}\$\$OBJECTS_DIR/AntWorker.o.CXXFLAGS += -fno-lto${C_RESET}"
fi
echo

# ---
# CHECK 7: Qt5 Development Libraries
# ---
echo -e "${C_YELLOW}[7/8] Checking Qt5 development libraries...${C_RESET}"
MISSING_QT_DEV=()
QT_DEV_PACKAGES=("qtbase5-dev" "qtconnectivity5-dev" "qtpositioning5-dev" "libqt5charts5-dev" "qtmultimedia5-dev")

for pkg in "${QT_DEV_PACKAGES[@]}"; do
    if ! dpkg -l 2>/dev/null | grep -q "^ii  $pkg"; then
        MISSING_QT_DEV+=("$pkg")
    fi
done

if [ ${#MISSING_QT_DEV[@]} -eq 0 ]; then
    echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} All required Qt5 development packages installed."
else
    echo -e "${C_RED}✗ ISSUE FOUND:${C_RESET} Missing Qt5 packages: ${MISSING_QT_DEV[*]}"
    echo -e "  ${C_CYAN}How to fix:${C_RESET}"
    echo -e "  ${C_YELLOW}sudo apt-get install qtbase5-dev qtbase5-private-dev qtconnectivity5-dev \\"
    echo -e "    qtpositioning5-dev libqt5charts5-dev libqt5networkauth5-dev \\"
    echo -e "    libqt5websockets5-dev qtmultimedia5-dev qtlocation5-dev \\"
    echo -e "    qtquickcontrols2-5-dev libqt5texttospeech5-dev${C_RESET}"
    ALL_CHECKS_PASSED=false
fi
echo

# ---
# CHECK 8: USB Development Libraries
# ---
echo -e "${C_YELLOW}[8/8] Checking USB development libraries...${C_RESET}"
MISSING_USB_DEV=()
USB_DEV_PACKAGES=("libusb-1.0-0-dev" "libudev-dev")

for pkg in "${USB_DEV_PACKAGES[@]}"; do
    if ! dpkg -l 2>/dev/null | grep -q "^ii  $pkg"; then
        MISSING_USB_DEV+=("$pkg")
    fi
done

if [ ${#MISSING_USB_DEV[@]} -eq 0 ]; then
    echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} All required USB development libraries installed."
else
    echo -e "${C_RED}✗ ISSUE FOUND:${C_RESET} Missing USB libraries: ${MISSING_USB_DEV[*]}"
    echo -e "  ${C_CYAN}How to fix:${C_RESET}"
    echo -e "  ${C_YELLOW}sudo apt-get install libusb-1.0-0-dev libudev-dev${C_RESET}"
    ALL_CHECKS_PASSED=false
fi
echo

# ---
# FINAL SUMMARY
# ---
echo -e "${C_CYAN}========================================================${C_RESET}"
if [ "$ALL_CHECKS_PASSED" = true ]; then
    echo -e "${C_GREEN}✓ All checks passed! Ready to build.${C_RESET}"
    echo
    echo -e "${C_CYAN}Next steps:${C_RESET}"
    echo -e "  1. ${C_YELLOW}cd $PROJECT_ROOT/src && qmake qdomyos-zwift.pro${C_RESET}"
    echo -e "  2. Look for: ${C_GREEN}'Project MESSAGE: >>> ANT+ ENABLED for build <<<'${C_RESET}"
    echo -e "  3. ${C_YELLOW}make${C_RESET}"
    echo
    echo -e "${C_CYAN}Full guide:${C_RESET} ${COMPILE_URL}"
else
    echo -e "${C_RED}✗ Some issues need attention.${C_RESET}"
    echo
    echo -e "Please review the suggestions above and follow the 'How to fix' instructions."
    echo -e "After making changes, run this script again to verify."
    echo
    echo -e "${C_CYAN}Full guide:${C_RESET} ${COMPILE_URL}"
fi
echo -e "${C_CYAN}========================================================${C_RESET}"