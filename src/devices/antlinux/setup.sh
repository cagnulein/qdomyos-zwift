#!/bin/bash
################################################################################
# setup.sh - QDomyos-Zwift ANT+ Setup and Validation Tool
#
# Purpose:
#   Unified tool for validating and setting up ANT+ prerequisites.
#   Provides quick validation, guided setup, testing, and reset capabilities.
#
# Usage:
#   ./setup.sh --check              # Fast validation check (no sudo)
#   sudo ./setup.sh --guided        # Guided setup with prompts
#   sudo ./setup.sh --headless      # Generate systemd service file only
#   sudo ./setup.sh --reset         # Remove configurations
#   sudo ./setup.sh --test          # Test ANT+ broadcasting
#   ./setup.sh --help               # Show this help
#
# Platform: Linux x86-64 and ARM64
# Dependencies: bash, ldconfig, systemctl, lsusb
# Author: bassai-sho
# Development assisted by AI analysis tools
################################################################################

set -uo pipefail

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Mode flags
MODE_CHECK=0
MODE_GUIDED=0
MODE_RESET=0
MODE_TEST=0

# Guided mode option
GUIDED_WITH_SERVICE=0  # 1 for --headless, 0 for --gui

# Test counters
PASS=0
FAIL=0
WARN=0

# Test result storage
declare -a TEST_RESULTS=()

# User context
if [ -n "${SUDO_USER:-}" ]; then
    TARGET_USER="$SUDO_USER"
    TARGET_HOME=$(eval echo "~$SUDO_USER")
else
    TARGET_USER="$USER"
    TARGET_HOME="$HOME"
fi

# Function to show help
show_help() {
    cat << 'EOF'
QDomyos-Zwift ANT+ Setup and Validation Tool

DESCRIPTION:
    Validates and configures your system for ANT+ footpod broadcasting.
    Checks Python 3.11, virtual environment, Qt5 libraries, USB permissions,
    and more. Provides guided setup with explanations.

USAGE:
    ./setup.sh --check              # Validate prerequisites (no sudo required)
    sudo ./setup.sh --gui           # Interactive guided setup for GUI systems
    sudo ./setup.sh --headless      # Interactive guided setup for headless systems
    sudo ./setup.sh --reset         # Remove configurations (undo setup)
    sudo ./setup.sh --test          # Test ANT+ broadcasting
    ./setup.sh --help               # Show this help

MODES:
    --check        Validate all prerequisites showing issues at once
                   - No sudo required
                   - Fast execution (~10 seconds)
                   - Exit codes: 0 (ready), 1 (failed), 2 (warnings)

    --gui          Interactive guided setup for GUI systems
                   - Requires sudo
                   - Step-by-step with prompts
                   - Installs dependencies and configures system
                   - No systemd service file generated
    
    --headless     Interactive guided setup for headless systems
                   - Requires sudo
                   - Step-by-step with prompts
                   - Installs dependencies and configures system
                   - Generates systemd service file for automatic startup
                   - Recommended for all users

    --reset        Complete removal of ANT+ setup
                   - Requires sudo
                   - Removes Python 3.11 (system and pyenv)
                   - Removes Python virtual environment
                   - Removes Qt5 libraries
                   - Removes libusb-1.0
                   - Removes user from plugdev group
                   - Removes udev rules
                   - Keeps bluez (Bluetooth service)

    --test         Test ANT+ broadcasting without QDomyos-Zwift
                   - Requires sudo
                   - Simulates treadmill data
                   - Verifies ANT+ dongle and watch pairing
                   - Useful for isolating ANT+ issues

    --help         Show this help message

EXAMPLES:
    # Check what's needed
    ./setup.sh --check

    # Guided setup (recommended)
    sudo ./setup.sh --guided

    # Test ANT+ broadcasting
    sudo ./setup.sh --test

    # Reset to clean state (for testing)
    sudo ./setup.sh --reset

EXIT CODES (--check mode):
    0 - All tests passed, system ready
    1 - Critical failures detected
    2 - Non-critical warnings present
EOF
}

# Parse arguments
if [ $# -eq 0 ]; then
    echo "Error: No mode specified."
    echo ""
    echo "Usage: ./setup.sh [MODE]"
    echo ""
    echo "Available modes:"
    echo "  --check        Quick validation check"
    echo "  --guided       Interactive guided setup"
    echo "  --reset        Remove configurations"
    echo "  --test         Test ANT+ broadcasting"
    echo "  --help         Show detailed help"
    echo ""
    exit 1
fi

for arg in "$@"; do
    case $arg in
        --check)
            MODE_CHECK=1
            shift
            ;;
        --gui)
            MODE_GUIDED=1
            GUIDED_WITH_SERVICE=0
            shift
            ;;
        --headless)
            MODE_GUIDED=1
            GUIDED_WITH_SERVICE=1
            shift
            ;;
        --reset)
            MODE_RESET=1
            shift
            ;;
        --test)
            MODE_TEST=1
            shift
            ;;
        --help)
            show_help
            exit 0
            ;;
        *)
            echo "Unknown option: $arg"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Validate mode combination
active_modes=0
for mode in $MODE_CHECK $MODE_RESET $MODE_GUIDED $MODE_TEST; do
    active_modes=$((active_modes + mode))
done

if [ $active_modes -gt 1 ]; then
    echo "Error: Only one mode can be selected at a time"
    echo "Use --help for usage information"
    exit 1
fi

# Default to guided mode (GUI) if no mode selected
if [ $active_modes -eq 0 ]; then
    MODE_GUIDED=1
    GUIDED_WITH_SERVICE=0
fi

# Check sudo requirement for guided/reset/test modes
if [ $MODE_GUIDED -eq 1 ] || [ $MODE_RESET -eq 1 ] || [ $MODE_TEST -eq 1 ]; then
    if [ $EUID -ne 0 ]; then
        echo -e "${RED}ERROR: --gui, --headless, --reset, and --test modes require sudo${NC}"
        echo "Please run: sudo $0 $@"
        exit 1
    fi
fi

# Helper function for check mode
test_check() {
    local test_name="$1"
    local test_command="$2"
    local success_msg="$3"
    local failure_msg="$4"
    local is_critical="${5:-true}"
    
    if eval "$test_command" > /dev/null 2>&1; then
        echo -e "${GREEN}[PASS]${NC} $success_msg"
        ((PASS++))
        TEST_RESULTS+=("{\"test\":\"$test_name\",\"status\":\"pass\",\"message\":\"$success_msg\"}")
        return 0
    else
        if [ "$is_critical" = "true" ]; then
            echo -e "${RED}[FAIL]${NC} $failure_msg"
            ((FAIL++))
            TEST_RESULTS+=("{\"test\":\"$test_name\",\"status\":\"fail\",\"message\":\"$failure_msg\"}")
        else
            echo -e "${YELLOW}[WARN]${NC} $failure_msg"
            ((WARN++))
            TEST_RESULTS+=("{\"test\":\"$test_name\",\"status\":\"warn\",\"message\":\"$failure_msg\"}")
        fi
        return 1
    fi
}

# Helper function for prompting user
prompt_yes_no() {
    local prompt="$1"
    local response
    
    while true; do
        # Try to read from stdin first (for automation), fall back to /dev/tty if stdin is not a terminal
        if [ -t 0 ]; then
            # stdin is a terminal, read from it directly
            read -p "$prompt [y/n]: " response
        else
            # stdin is piped, try to read from it
            if read -t 0.1 response; then
                echo "$prompt [y/n]: $response"
            else
                # stdin exhausted, read from /dev/tty for interactive prompt
                read -p "$prompt [y/n]: " response </dev/tty
            fi
        fi
        
        case "$response" in
            [Yy]* ) return 0;;
            [Nn]* ) return 1;;
            * ) echo "Please answer y or n.";;
        esac
    done
}

# ============================================================================
# CHECK MODE - Validate all prerequisites
# ============================================================================

run_check_mode() {
    echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
    echo -e "${BLUE}QDomyos-Zwift ANT+ Prerequisites Check${NC}"
    echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
    echo ""

    # Detect headless (no X11/Wayland) and skip GUI/QML checks if true
    SKIP_GUI_CHECKS=false
    if [ -z "${DISPLAY:-}" ] && [ -z "${WAYLAND_DISPLAY:-}" ]; then
        SKIP_GUI_CHECKS=true
    fi
    
    # Test 1: Python 3.11 library (check system and pyenv)
    test_check \
        "python311_library" \
        "ldconfig -p | grep 'libpython3.11.so' || ls $TARGET_HOME/.pyenv/versions/3.11.*/lib/libpython3.11.so* &>/dev/null" \
        "Python 3.11 library found" \
        "Python 3.11 library not found" \
        "true"
    
    # Test 2: Virtual environment
    test_check \
        "ant_venv" \
        "[ -d \"$TARGET_HOME/ant_venv\" ]" \
        "Virtual environment exists at ~/ant_venv" \
        "Virtual environment not found" \
        "false"
    
    # Test 3-5: Python packages (only if venv exists)
    if [ -d "$TARGET_HOME/ant_venv" ]; then
        # Determine how to run python checks (with or without sudo -u)
        if [ -n "${SUDO_USER:-}" ]; then
            PYTHON_CMD="sudo -u $TARGET_USER $TARGET_HOME/ant_venv/bin/python"
        else
            PYTHON_CMD="$TARGET_HOME/ant_venv/bin/python"
        fi
        
        test_check \
            "python_package_openant" \
            "$PYTHON_CMD -c 'import openant' 2>/dev/null" \
            "Python package 'openant' installed" \
            "Python package 'openant' missing" \
            "true"
        
        test_check \
            "python_package_pyusb" \
            "$PYTHON_CMD -c 'import usb' 2>/dev/null" \
            "Python package 'pyusb' installed" \
            "Python package 'pyusb' missing" \
            "true"
        
        test_check \
            "python_package_pybind11" \
            "$PYTHON_CMD -c 'import pybind11' 2>/dev/null" \
            "Python package 'pybind11' installed" \
            "Python package 'pybind11' missing" \
            "true"
    fi
    
    # Test 6: Qt5 runtime libraries (check all required ones)
    # Comprehensive check prevents runtime failures from missing dependencies
    missing_libs=()
    required_libs=(
        "libQt5Core.so"
        "libQt5Qml.so"
        "libQt5Quick.so"
        "libQt5QuickWidgets.so"
        "libQt5Concurrent.so"
        "libQt5Bluetooth.so"
        "libQt5Charts.so"
        "libQt5Multimedia.so"
        "libQt5MultimediaWidgets.so"
        "libQt5NetworkAuth.so"
        "libQt5Positioning.so"
        "libQt5Location.so"
        "libQt5Sql.so"
        "libQt5TextToSpeech.so"
        "libQt5WebSockets.so"
        "libQt5Widgets.so"
        "libQt5Xml.so"
        "libusb-1.0.so.0"
    )
    
    for lib in "${required_libs[@]}"; do
        if ! ldconfig -p 2>/dev/null | grep >/dev/null 2>&1 "$lib"; then
            missing_libs+=("$lib")
        fi
    done
    
    if [ ${#missing_libs[@]} -eq 0 ]; then
        test_check \
            "qt5_libraries" \
            "true" \
            "Qt5 runtime libraries available (${#required_libs[@]} libraries checked)" \
            "" \
            "true"
    else
        test_check \
            "qt5_libraries" \
            "false" \
            "Qt5 runtime libraries available" \
            "Qt5 runtime libraries missing (${#missing_libs[@]} libraries)" \
            "true"
    fi
    
    # Test 7: QML modules (check all required ones)
    # Ensures all QML imports are available to prevent UI failures
    missing_qml=()
    required_qml_dirs=(
        "QtLocation"
        "QtMultimedia"
        "QtPositioning"
        "QtQuick.2"
        "QtQuick/Controls"
        "QtQuick/Controls.2"
        "QtQuick/Dialogs"
        "QtQuick/Layouts"
        "QtQuick/Window.2"
    )
    
    for qml_dir in "${required_qml_dirs[@]}"; do
        # Check common QML paths
        found=0
        for qml_path in /usr/lib/*/qt5/qml /usr/lib/qt5/qml /usr/lib/aarch64-linux-gnu/qt5/qml /usr/lib/x86_64-linux-gnu/qt5/qml; do
            if [ -d "$qml_path/$qml_dir" ]; then
                found=1
                break
            fi
        done
        if [ $found -eq 0 ]; then
            missing_qml+=("$qml_dir")
        fi
    done
    
    if [ ${#missing_qml[@]} -eq 0 ]; then
        test_check \
            "qml_modules" \
            "true" \
            "QML modules available (${#required_qml_dirs[@]} modules checked)" \
            "" \
            "true"
    else
        test_check \
            "qml_modules" \
            "false" \
            "QML modules available" \
            "QML modules missing (${#missing_qml[@]} modules)" \
            "true"
    fi
    
    # Test 8: Bluetooth service
    test_check \
        "bluetooth_service" \
        "systemctl list-unit-files | grep '^bluetooth.service'" \
        "Bluetooth service available" \
        "Bluetooth service not installed" \
        "true"
    
    # Test 9: plugdev group
    test_check \
        "plugdev_group" \
        "groups $TARGET_USER | grep plugdev" \
        "User '$TARGET_USER' is in 'plugdev' group" \
        "User not in 'plugdev' group" \
        "true"
    
    # Test 10: udev rules
    test_check \
        "udev_rules" \
        "[ -f /etc/udev/rules.d/99-garmin-ant.rules ] || [ -f /etc/udev/rules.d/51-garmin-ant.rules ] || [ -f /etc/udev/rules.d/99-ant-usb.rules ]" \
        "ANT+ udev rules configured" \
        "ANT+ udev rules not found" \
        "true"
    
    # Test 11: lsusb availability (required for dongle detection)
    test_check \
        "lsusb_available" \
        "command -v lsusb" \
        "lsusb command available" \
        "lsusb not found (usbutils package needed)" \
        "true"
    
    # Test 12: ANT+ dongle (only check if lsusb is available)
    if command -v lsusb >/dev/null 2>&1; then
        test_check \
            "ant_dongle" \
            "lsusb | grep -E '0fcf:1009|0fcf:1008|0fcf:100c|0fcf:100e|0fcf:88a4|0fcf:1004|11fd:0001'" \
            "ANT+ USB dongle detected" \
            "ANT+ USB dongle not detected (plug in Garmin/Suunto ANT+ stick)" \
            "false"
    fi
    
    # Test 13: Bluetooth service status
    test_check \
        "bluetooth_running" \
        "systemctl is-active --quiet bluetooth" \
        "Bluetooth service running" \
        "Bluetooth service not running" \
        "true"
    
    # Test 14: Configuration file exists
    CONFIG_FILE="${TARGET_HOME}/.config/Roberto Viola/qDomyos-Zwift.conf"
    if [ -f "$CONFIG_FILE" ]; then
        echo -e "${GREEN}[PASS]${NC} Configuration file exists"
    else
        echo -e "${YELLOW}[INFO]${NC} Configuration file not found at:"
        echo -e "    ${YELLOW}${CONFIG_FILE}${NC}"
        echo -e "    ${YELLOW}(Optional - app will use defaults)${NC}"
    fi
    
    # Test 15: Systemd service file exists
    SERVICE_FILE_SYSTEM="/etc/systemd/system/qz.service"
    SERVICE_FILE_LIB="/lib/systemd/system/qz.service"
    SERVICE_EXISTS=false
    SERVICE_LOCATION=""
    
    if [ -f "$SERVICE_FILE_SYSTEM" ]; then
        SERVICE_EXISTS=true
        SERVICE_LOCATION="$SERVICE_FILE_SYSTEM"
    elif [ -f "$SERVICE_FILE_LIB" ]; then
        SERVICE_EXISTS=true
        SERVICE_LOCATION="$SERVICE_FILE_LIB"
    fi
    
    if [ "$SERVICE_EXISTS" = true ]; then
        echo -e "${GREEN}[PASS]${NC} Systemd service file exists: ${SERVICE_LOCATION}"
        
        # Check if service is enabled
        if systemctl is-enabled qz.service >/dev/null 2>&1; then
            echo -e "${GREEN}[PASS]${NC} Service is enabled"
        else
            echo -e "${YELLOW}[INFO]${NC} Service exists but is not enabled. To enable:"
            echo -e "    ${CYAN}sudo systemctl enable qz.service${NC}"
        fi
        
        # Check if service is running
        if systemctl is-active --quiet qz.service; then
            echo -e "${GREEN}[PASS]${NC} Service is running"
        else
            echo -e "${YELLOW}[INFO]${NC} Service is not running. To start:"
            echo -e "    ${CYAN}sudo systemctl start qz.service${NC}"
            echo -e "${YELLOW}[INFO]${NC} To view service status:"
            echo -e "    ${CYAN}sudo systemctl status qz.service${NC}"
        fi
    else
        # Service file is optional - only relevant for headless systems
        # Don't display message in check mode to avoid confusion
        :
    fi
    
    # Output results
    echo ""
    
    if [ $FAIL -gt 0 ]; then
        echo -e "${RED}System Status: FAILED${NC}"
        echo ""
        echo -e "${CYAN}To fix issues, run:${NC}"
        echo -e "  ${YELLOW}sudo ./setup.sh --guided${NC}"
        exit 1
    elif [ $WARN -gt 0 ]; then
        echo -e "${YELLOW}System Status: WARNING${NC}"
        echo ""
        exit 2
    else
        echo -e "${GREEN}System Status: READY${NC}"
        echo ""
        echo -e "${CYAN}To run qdomyos-zwift with ANT+ support:${NC}"
        echo -e "  ${YELLOW}./qdomyos-zwift -no-gui -ant-footpod${NC}"
        echo ""
        echo -e "${CYAN}For automatic startup on headless systems:${NC}"
        echo -e "  Run: ${YELLOW}sudo ./setup.sh --headless${NC}"
        echo ""
        exit 0
    fi
}

# ============================================================================
# RESET MODE - Remove configurations (undo --guided)
# ============================================================================

run_reset_mode() {
    echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
    echo -e "${BLUE}QDomyos-Zwift ANT+ Configuration Reset${NC}"
    echo -e "${CYAN}Running for user: $TARGET_USER${NC}"
    echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
    echo ""
    echo -e "${YELLOW}WARNING: This will completely remove ANT+ setup${NC}"
    echo ""
    echo "This will remove:"
    echo "  - Python 3.11 (system and pyenv installations)"
    echo "  - Python virtual environment (~/ant_venv)"
    echo "  - Python packages (openant, pyusb, pybind11)"
    echo "  - Qt5 libraries (9 packages: charts, multimedia, networkauth, etc.)"
    echo "  - QML modules (8 packages: QtLocation, QtQuick, etc.)"
    echo "  - libusb-1.0"
    echo "  - User from 'plugdev' group"
    echo "  - ANT+ udev rules"
    echo ""
    echo "This will NOT remove:"
    echo "  - bluez (system Bluetooth service)"
    echo "  - libqt5bluetooth5, libqt5positioning5, libqt5sql5 (system packages)"
    echo ""
    echo -e "${YELLOW}Note: Qt5/libusb removal skipped on desktop systems to prevent breakage${NC}"
    echo ""
    echo -e "${CYAN}Completely removes what was installed by --guided mode${NC}"
    echo ""
    
    read -p "Continue with reset? [y/N]: " response
    case "$response" in
        [Yy]* ) ;;
        * ) 
            echo "Reset cancelled"
            exit 0
            ;;
    esac
    
    echo ""
    local reset_count=0
    local failed_count=0
    
    # Reset 1: Remove user from plugdev group
    echo -e "${YELLOW}[1/6] Removing $TARGET_USER from plugdev group...${NC}"
    if groups "$TARGET_USER" | grep >/dev/null 2>&1 plugdev; then
        if gpasswd -d "$TARGET_USER" plugdev 2>/dev/null; then
            echo -e "${GREEN}✓ User removed from plugdev group${NC}"
            echo -e "${YELLOW}⚠ You must logout and login for changes to take effect${NC}"
            ((reset_count++))
        else
            echo -e "${RED}✗ Failed to remove user from plugdev group${NC}"
            ((failed_count++))
        fi
    else
        echo -e "${GREEN}✓ User not in plugdev group${NC}"
    fi
    echo ""
    
    # Reset 2: Remove udev rules
    echo -e "${YELLOW}[2/6] Removing ANT+ udev rules...${NC}"
    local rules_removed=0
    for rules_file in \
        /etc/udev/rules.d/99-ant-usb.rules \
        /etc/udev/rules.d/99-garmin-ant.rules \
        /etc/udev/rules.d/51-garmin-ant.rules; do
        
        if [ -f "$rules_file" ]; then
            if rm -f "$rules_file"; then
                ((rules_removed++))
            else
                echo -e "${RED}✗ Failed to remove $rules_file${NC}"
                ((failed_count++))
            fi
        fi
    done
    
    if [ $rules_removed -gt 0 ]; then
        udevadm control --reload-rules
        udevadm trigger
        echo -e "${GREEN}✓ Removed $rules_removed udev rule(s)${NC}"
        ((reset_count++))
    else
        echo -e "${GREEN}✓ No udev rules to remove${NC}"
    fi
    echo ""
    
    # Reset 3: Remove Python virtual environment
    echo -e "${YELLOW}[3/6] Removing Python virtual environment...${NC}"
    if [ -d "$TARGET_HOME/ant_venv" ]; then
        if rm -rf "$TARGET_HOME/ant_venv"; then
            echo -e "${GREEN}✓ Virtual environment removed${NC}"
            ((reset_count++))
        else
            echo -e "${RED}✗ Failed to remove virtual environment${NC}"
            ((failed_count++))
        fi
    else
        echo -e "${GREEN}✓ No virtual environment found${NC}"
    fi
    echo ""
    
    # Reset 4: Remove Python 3.11
    echo -e "${YELLOW}[4/6] Removing Python 3.11...${NC}"
    local python_removed=0
    
    # Check if Python 3.11 is installed via apt
    if dpkg -l python3.11 2>/dev/null | grep >/dev/null 2>&1 "^ii"; then
        echo "Removing system Python 3.11 packages..."
        if apt-get remove -y python3.11 python3.11-venv python3.11-dev 2>/dev/null; then
            apt-get autoremove -y 2>/dev/null
            echo -e "${GREEN}✓ System Python 3.11 removed${NC}"
            ((reset_count++))
            ((python_removed++))
        else
            echo -e "${RED}✗ Failed to remove system Python 3.11${NC}"
            ((failed_count++))
        fi
    fi
    
    # Remove pyenv Python 3.11 installations
    if [ -d "$TARGET_HOME/.pyenv/versions" ]; then
        local pyenv_versions=$(ls -d "$TARGET_HOME/.pyenv/versions/3.11"* 2>/dev/null)
        if [ -n "$pyenv_versions" ]; then
            echo "Removing pyenv Python 3.11 installation(s)..."
            for version_dir in $pyenv_versions; do
                if rm -rf "$version_dir"; then
                    echo -e "${GREEN}✓ Removed pyenv version: $(basename $version_dir)${NC}"
                    ((python_removed++))
                else
                    echo -e "${RED}✗ Failed to remove: $(basename $version_dir)${NC}"
                    ((failed_count++))
                fi
            done
            ((reset_count++))
        fi
    fi
    
    if [ $python_removed -eq 0 ]; then
        echo -e "${GREEN}✓ No Python 3.11 installations found${NC}"
    fi
    echo ""
    
    # Check if running on a desktop environment
    # Use multiple detection methods to catch all desktop variants
    local is_desktop=false
    
    # Method 1: Check if X11 or Wayland display server is available
    if [ -n "${DISPLAY:-}" ] || [ -n "${WAYLAND_DISPLAY:-}" ]; then
        is_desktop=true
    # Method 2: Check systemd default target (graphical.target indicates desktop)
    elif systemctl get-default 2>/dev/null | grep -q "graphical.target"; then
        is_desktop=true
    # Method 3: Check for common desktop packages (fallback)
    elif dpkg -l 2>/dev/null | grep -qE "^ii  (ubuntu-desktop|lubuntu-desktop|kubuntu-desktop|xubuntu-desktop|ubuntu-mate-desktop|ubuntu-budgie-desktop|gnome-shell|kde-plasma-desktop|xfce4|lxde|mate-desktop)"; then
        is_desktop=true
    fi
    
    # Reset 5: Remove Qt5 libraries
    echo -e "${YELLOW}[5/6] Removing Qt5 libraries and QML modules...${NC}"
    
    if [ "$is_desktop" = true ]; then
        echo -e "${YELLOW}⚠ Desktop environment detected - skipping Qt5 removal to prevent system breakage${NC}"
        echo -e "${YELLOW}  Qt5 is a core dependency of your desktop environment${NC}"
        echo -e "${GREEN}✓ Skipped (desktop protection)${NC}"
    else
        local qt_packages=(
            "libqt5charts5"
            "libqt5multimedia5"
            "libqt5multimediawidgets5"
            "libqt5multimedia5-plugins"
            "libqt5networkauth5"
            "libqt5texttospeech5"
            "libqt5websockets5"
            "libqt5widgets5"
            "libqt5xml5"
            "qtlocation5-dev"
            "qml-module-qtlocation"
            "qml-module-qtpositioning"
            "qml-module-qtquick2"
            "qml-module-qtquick-controls"
            "qml-module-qtquick-controls2"
            "qml-module-qtquick-dialogs"
            "qml-module-qtquick-layouts"
            "qml-module-qtquick-window2"
            "qml-module-qtmultimedia"
        )
        
        # Note: Excluding system packages that may be auto-installed:
        # - libqt5bluetooth5, libqt5positioning5, libqt5sql5 (often system deps)
        
        local qt_found=0
        for pkg in "${qt_packages[@]}"; do
            if dpkg -l "$pkg" 2>/dev/null | grep >/dev/null 2>&1 "^ii"; then
                ((qt_found++))
            fi
        done
        
        if [ $qt_found -gt 0 ]; then
            echo "Removing $qt_found Qt5/QML/Bluetooth package(s)..."
            if apt-get remove -y "${qt_packages[@]}" 2>/dev/null; then
                apt-get autoremove -y 2>/dev/null
                echo -e "${GREEN}✓ Qt5 libraries and QML modules removed${NC}"
                ((reset_count++))
            else
                echo -e "${RED}✗ Failed to remove some Qt5/QML packages${NC}"
                ((failed_count++))
            fi
        else
            echo -e "${GREEN}✓ No Qt5/QML packages found${NC}"
        fi
    fi
    echo ""
    
    # Reset 6: Remove libusb
    echo -e "${YELLOW}[6/6] Removing libusb-1.0...${NC}"
    
    if [ "$is_desktop" = true ]; then
        echo -e "${YELLOW}⚠ Desktop environment detected - skipping libusb removal to prevent system breakage${NC}"
        echo -e "${YELLOW}  libusb is a core dependency of your desktop environment${NC}"
        echo -e "${GREEN}✓ Skipped (desktop protection)${NC}"
    else
        if dpkg -l libusb-1.0-0 2>/dev/null | grep >/dev/null 2>&1 "^ii"; then
            if apt-get remove -y libusb-1.0-0 2>/dev/null; then
                apt-get autoremove -y 2>/dev/null
                echo -e "${GREEN}✓ libusb-1.0 removed${NC}"
                ((reset_count++))
            else
                echo -e "${RED}✗ Failed to remove libusb-1.0${NC}"
                ((failed_count++))
            fi
        else
            echo -e "${GREEN}✓ No libusb-1.0 package found${NC}"
        fi
    fi
    echo ""
    
    # Summary
    echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
    if [ $reset_count -gt 0 ]; then
        echo -e "${GREEN}✓ Reset complete - removed $reset_count configuration(s)${NC}"
    fi
    if [ $failed_count -gt 0 ]; then
        echo -e "${RED}✗ Failed to reset $failed_count item(s)${NC}"
    fi
    if [ $reset_count -eq 0 ] && [ $failed_count -eq 0 ]; then
        echo -e "${GREEN}✓ No configurations to reset - system already clean${NC}"
    fi
    echo ""
    echo -e "Note: bluez (Bluetooth) remains installed as it may be used by other apps"
    echo -e "Run '${YELLOW}./setup.sh --check${NC}' to verify clean state"
}

# ============================================================================
# GUIDED MODE - Interactive step-by-step setup
# ============================================================================

run_guided_mode() {
    echo -e "${CYAN}═══════════════════════════════════════════════════════════${NC}"
    echo -e "${CYAN}QDomyos-Zwift ANT+ Setup Wizard${NC}"
    echo -e "${CYAN}Running for user: $TARGET_USER${NC}"
    echo -e "${CYAN}═══════════════════════════════════════════════════════════${NC}"
    echo ""
    echo "This wizard will guide you through setting up ANT+ prerequisites."
    echo "Each step will be explained before execution."
    echo ""
    
    # Step 1: Check Python 3.11
    echo -e "${BLUE}[Step 1/6] Checking Python 3.11...${NC}"
    
    if ldconfig -p | grep >/dev/null 2>&1 'libpython3.11.so' || ls "$TARGET_HOME"/.pyenv/versions/3.11.*/lib/libpython3.11.so* &>/dev/null; then
        echo -e "${GREEN}✓ Python 3.11 is already installed${NC}"
    else
        echo -e "${YELLOW}✗ Python 3.11 not found${NC}"
        echo ""
        
        if apt-cache show python3.11 >/dev/null 2>&1; then
            if prompt_yes_no "Install Python 3.11?"; then
                apt-get update
                apt-get install -y python3.11 python3.11-venv
                echo -e "${GREEN}✓ Python 3.11 installed${NC}"
            fi
        else
            echo "Python 3.11 needs to be installed via pyenv."
            echo ""
            if prompt_yes_no "Install Python 3.11 via pyenv?"; then
                echo ""
                echo "Installing pyenv prerequisites..."
                apt-get update
                apt-get install -y \
                    git curl build-essential libssl-dev zlib1g-dev \
                    libbz2-dev libreadline-dev libsqlite3-dev wget \
                    llvm libncurses-dev xz-utils tk-dev libffi-dev liblzma-dev
                
                echo ""
                echo "Installing pyenv for user $TARGET_USER..."
                if [ ! -d "$TARGET_HOME/.pyenv" ]; then
                    # Install pyenv as the target user
                    sudo -u "$TARGET_USER" bash -c 'curl https://pyenv.run | bash'
                    
                    # Add to shell configuration if not already there
                    if ! grep >/dev/null 2>&1 'PYENV_ROOT' "$TARGET_HOME/.bashrc" 2>/dev/null; then
                        sudo -u "$TARGET_USER" bash -c 'cat >> ~/.bashrc << '\''EOF'\''

# Pyenv configuration
export PYENV_ROOT="$HOME/.pyenv"
command -v pyenv >/dev/null || export PATH="$PYENV_ROOT/bin:$PATH"
eval "$(pyenv init -)"
EOF'
                    fi
                fi
                
                echo ""
                echo "Installing Python 3.11.9..."
                # Source the pyenv configuration and install Python
                sudo -u "$TARGET_USER" bash -c 'export PYENV_ROOT="$HOME/.pyenv"; export PATH="$PYENV_ROOT/bin:$PATH"; eval "$(pyenv init -)"; pyenv install -s 3.11.9 && pyenv global 3.11.9'
                
                if [ $? -eq 0 ]; then
                    echo -e "${GREEN}✓ Python 3.11 installed via pyenv${NC}"
                    # Reload shell configuration to make Python available
                    sudo -u "$TARGET_USER" bash -c 'source ~/.bashrc'
                else
                    echo -e "${RED}✗ Failed to install Python 3.11 via pyenv${NC}"
                    echo "You may need to logout/login and run this setup again."
                fi
            else
                echo ""
                echo -e "${YELLOW}Skipping Python installation.${NC}"
                echo ""
                echo "To install manually later:"
                echo "  1. See README.md section 'Install Python 3.11'"
                echo "  2. Run this guided setup again after installation"
            fi
        fi
    fi
    echo ""
    
    # Step 2: Create virtual environment
    echo -e "${BLUE}[Step 2/6] Setting up virtual environment...${NC}"
    
    if [ -d "$TARGET_HOME/ant_venv" ]; then
        echo -e "${GREEN}✓ Virtual environment already exists${NC}"
    else
        # Try to find python3.11 in multiple locations
        PYTHON311=""
        if command -v python3.11 >/dev/null 2>&1; then
            PYTHON311="python3.11"
        elif [ -f /usr/bin/python3.11 ]; then
            PYTHON311="/usr/bin/python3.11"
        elif ls "$TARGET_HOME"/.pyenv/versions/3.11.*/bin/python3.11 &>/dev/null; then
            PYTHON311=$(ls "$TARGET_HOME"/.pyenv/versions/3.11.*/bin/python3.11 2>/dev/null | head -1)
        fi
        
        if [ -n "$PYTHON311" ]; then
            if prompt_yes_no "Create Python virtual environment?"; then
                sudo -u "$TARGET_USER" "$PYTHON311" -m venv "$TARGET_HOME/ant_venv"
                echo -e "${GREEN}✓ Virtual environment created${NC}"
            fi
        else
            echo -e "${RED}✗ Python 3.11 not available - complete Step 1 first${NC}"
        fi
    fi
    echo ""
    
    # Step 3: Install Python packages
    echo -e "${BLUE}[Step 3/6] Installing Python packages...${NC}"
    
    if [ -d "$TARGET_HOME/ant_venv" ]; then
        # Check if packages are already installed
        local packages_missing=0
        local missing_names=""
        
        if ! sudo -u "$TARGET_USER" "$TARGET_HOME/ant_venv/bin/python" -c 'import openant' 2>/dev/null; then
            ((packages_missing++))
            missing_names="openant"
        fi
        if ! sudo -u "$TARGET_USER" "$TARGET_HOME/ant_venv/bin/python" -c 'import usb' 2>/dev/null; then
            ((packages_missing++))
            missing_names="$missing_names pyusb"
        fi
        if ! sudo -u "$TARGET_USER" "$TARGET_HOME/ant_venv/bin/python" -c 'import pybind11' 2>/dev/null; then
            ((packages_missing++))
            missing_names="$missing_names pybind11"
        fi
        
        if [ $packages_missing -eq 0 ]; then
            echo -e "${GREEN}✓ All Python packages already installed${NC}"
        else
            echo -e "${YELLOW}Missing packages:${missing_names}${NC}"
            if prompt_yes_no "Install Python packages?"; then
                # Run pip as target user, ensuring proper ownership
                sudo -u "$TARGET_USER" "$TARGET_HOME/ant_venv/bin/pip" install --upgrade pip
                sudo -u "$TARGET_USER" "$TARGET_HOME/ant_venv/bin/pip" install openant pyusb pybind11
                echo -e "${GREEN}✓ Python packages installed${NC}"
            fi
        fi
    else
        echo -e "${YELLOW}⚠ Skipping - virtual environment not created${NC}"
    fi
    echo ""
    
    # Step 4: Install system libraries
    echo -e "${BLUE}[Step 4/6] Installing system libraries...${NC}"
    
    # Use the same comprehensive check as validation
    missing_libs=()
    required_libs=(
        "libQt5Bluetooth.so"
        "libQt5Charts.so"
        "libQt5Multimedia.so"
        "libQt5MultimediaWidgets.so"
        "libQt5NetworkAuth.so"
        "libQt5Positioning.so"
        "libQt5Sql.so"
        "libQt5TextToSpeech.so"
        "libQt5WebSockets.so"
        "libQt5Widgets.so"
        "libQt5Xml.so"
        "libusb-1.0.so.0"
    )
    
    for lib in "${required_libs[@]}"; do
        if ! ldconfig -p | grep >/dev/null 2>&1 "$lib"; then
            missing_libs+=("$lib")
        fi
    done
    
    # Check QML modules
    missing_qml=()
    required_qml_dirs=(
        "QtLocation"
        "QtPositioning"
        "QtQuick.2"
        "QtQuick/Controls"
        "QtQuick/Controls.2"
        "QtQuick/Dialogs"
        "QtQuick/Layouts"
        "QtQuick/Window.2"
    )
    
    for qml_dir in "${required_qml_dirs[@]}"; do
        found=0
        for qml_path in /usr/lib/*/qt5/qml /usr/lib/qt5/qml /usr/lib/aarch64-linux-gnu/qt5/qml /usr/lib/x86_64-linux-gnu/qt5/qml; do
            if [ -d "$qml_path/$qml_dir" ]; then
                found=1
                break
            fi
        done
        if [ $found -eq 0 ]; then
            missing_qml+=("$qml_dir")
        fi
    done
    
    # Check Bluetooth service
    if ! systemctl list-unit-files | grep >/dev/null 2>&1 '^bluetooth.service'; then
        missing_libs+=("bluetooth")
    fi
    
    # Check for lsusb command (usbutils package)
    if ! command -v lsusb >/dev/null 2>&1; then
        missing_libs+=("lsusb")
    fi
    
    if [ ${#missing_libs[@]} -eq 0 ] && [ ${#missing_qml[@]} -eq 0 ]; then
        echo -e "${GREEN}✓ All system libraries already installed${NC}"
    else
        if [ ${#missing_libs[@]} -gt 0 ]; then
            echo -e "${YELLOW}Missing libraries: ${missing_libs[*]}${NC}"
        fi
        if [ ${#missing_qml[@]} -gt 0 ]; then
            echo -e "${YELLOW}Missing QML modules: ${missing_qml[*]}${NC}"
        fi
        if prompt_yes_no "Install system libraries and QML modules?"; then
            apt-get update
            apt-get install -y \
                libqt5core5a libqt5qml5 libqt5quick5 libqt5quickwidgets5 libqt5concurrent5 \
                libqt5bluetooth5 libqt5charts5 libqt5multimedia5 \
                libqt5multimediawidgets5 libqt5multimedia5-plugins \
                libqt5networkauth5 libqt5positioning5 libqt5sql5 \
                libqt5texttospeech5 libqt5websockets5 libqt5widgets5 \
                libqt5xml5 libqt5location5 qtlocation5-dev \
                qml-module-qtlocation qml-module-qtpositioning \
                qml-module-qtquick2 qml-module-qtquick-controls \
                qml-module-qtquick-controls2 qml-module-qtquick-dialogs \
                qml-module-qtquick-layouts qml-module-qtquick-window2 \
                qml-module-qtmultimedia \
                libusb-1.0-0 bluez usbutils python3-pip
            echo -e "${GREEN}✓ System libraries installed${NC}"
        fi
    fi
    echo ""
    
    # Step 5: Configure USB permissions
    echo -e "${BLUE}[Step 5/6] Configuring USB permissions...${NC}"
    
    if ! groups "$TARGET_USER" | grep >/dev/null 2>&1 plugdev; then
        if prompt_yes_no "Configure USB permissions?"; then
            usermod -aG plugdev "$TARGET_USER"
            echo -e "${GREEN}✓ User added to plugdev group${NC}"
        fi
    else
        echo -e "${GREEN}✓ User already in plugdev group${NC}"
    fi
    
    if [ ! -f /etc/udev/rules.d/99-ant-usb.rules ]; then
        if prompt_yes_no "Configure ANT+ USB access?"; then
            cat > /etc/udev/rules.d/99-ant-usb.rules <<'EOF'
SUBSYSTEM=="usb", ATTRS{idVendor}=="0fcf", ATTRS{idProduct}=="100?", MODE="0666", GROUP="plugdev"
SUBSYSTEM=="usb", ATTRS{idVendor}=="0fcf", ATTRS{idProduct}=="88a4", MODE="0666", GROUP="plugdev"
SUBSYSTEM=="usb", ATTRS{idVendor}=="11fd", ATTRS{idProduct}=="0001", MODE="0666", GROUP="plugdev"
EOF
            udevadm control --reload-rules
            udevadm trigger
            echo -e "${GREEN}✓ udev rules created and activated${NC}"
        fi
    else
        echo -e "${GREEN}✓ udev rules already configured${NC}"
    fi
    echo ""
    
    # Step 6: Enable Bluetooth
    echo -e "${BLUE}[Step 6/7] Enabling Bluetooth service...${NC}"
    
    if ! systemctl is-active --quiet bluetooth; then
        if prompt_yes_no "Enable Bluetooth service?"; then
            systemctl start bluetooth
            systemctl enable bluetooth
            echo -e "${GREEN}✓ Bluetooth service started${NC}"
        fi
    else
        echo -e "${GREEN}✓ Bluetooth service already running${NC}"
    fi
    echo ""
    
    # Step 7: Conditional service file generation based on mode
    if [ $GUIDED_WITH_SERVICE -eq 1 ]; then
        # Headless mode - generate systemd service file
        echo -e "${BLUE}[Step 7/7] Generate systemd service file for automatic startup${NC}"
        echo ""
        echo "This will create a systemd service that starts qdomyos-zwift automatically"
        echo "on boot in headless mode with ANT+ footpod support."
        echo ""
        
        # Determine service file location based on system
        if [ -d "/lib/systemd/system" ] && ! [ -f "/etc/systemd/system/qz.service" ]; then
            # Raspberry Pi - prefer /lib/systemd/system
            SERVICE_FILE="/lib/systemd/system/qz.service"
        else
            # Desktop Linux or already exists in /etc
            SERVICE_FILE="/etc/systemd/system/qz.service"
        fi
        
        if [ -f "$SERVICE_FILE" ]; then
            echo -e "${YELLOW}Service file already exists at: ${SERVICE_FILE}${NC}"
            if ! prompt_yes_no "Overwrite existing service file?"; then
                echo -e "${YELLOW}Skipping service file generation${NC}"
                echo ""
                GENERATE_SERVICE=false
            else
                GENERATE_SERVICE=true
            fi
        else
            GENERATE_SERVICE=true
        fi
        
        if [ "$GENERATE_SERVICE" = true ]; then
            # Get installation directory
            SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
            INSTALL_DIR="$(cd "$SCRIPT_DIR/../../.." && pwd)"
            
            # Detect architecture
            ARCH=$(uname -m)
            if [ "$ARCH" = "aarch64" ]; then
                BIN_DIR="$INSTALL_DIR/qdomyos-zwift-arm64-ant"
            else
                BIN_DIR="$INSTALL_DIR/qdomyos-zwift-x86-64-ant"
            fi
            
            # Create service file
            cat > "$SERVICE_FILE" <<EOF
[Unit]
Description=qdomyos-zwift service
After=multi-user.target

[Service]
User=root
Group=plugdev
Environment="QZ_USER=${TARGET_USER}"
WorkingDirectory=${BIN_DIR}
ExecStart=${BIN_DIR}/qdomyos-zwift -no-gui -log -ant-footpod
KillSignal=SIGINT

[Install]
WantedBy=multi-user.target
EOF
            
            echo -e "${GREEN}✓ Service file created: ${SERVICE_FILE}${NC}"
            echo ""
            echo -e "${CYAN}Service configuration:${NC}"
            echo -e "  Working Directory: ${BIN_DIR}"
            echo -e "  User: root (runs as ${TARGET_USER} via QZ_USER)"
            echo -e "  Startup: Automatic on boot"
            echo ""
            
            if prompt_yes_no "Enable service to start on boot?"; then
                systemctl daemon-reload
                systemctl enable qz.service
                echo -e "${GREEN}✓ Service enabled${NC}"
                echo ""
                
                if prompt_yes_no "Start service now?"; then
                    systemctl start qz.service
                    echo -e "${GREEN}✓ Service started${NC}"
                    echo ""
                    echo -e "${CYAN}To check service status:${NC}"
                    echo -e "  ${YELLOW}sudo systemctl status qz.service${NC}"
                else
                    echo -e "${YELLOW}To start service manually:${NC}"
                    echo -e "  ${YELLOW}sudo systemctl start qz.service${NC}"
                fi
            else
                echo -e "${YELLOW}Service created but not enabled.${NC}"
                echo -e "${YELLOW}To enable and start:${NC}"
                echo -e "  ${YELLOW}sudo systemctl enable qz.service${NC}"
                echo -e "  ${YELLOW}sudo systemctl start qz.service${NC}"
            fi
        fi
    else
        # GUI mode - skip service file generation
        echo -e "${BLUE}[Step 7/7] System Integration${NC}"
        echo -e "${GREEN}✓ Setup complete!${NC}"
        echo ""
    fi
    echo ""
    sleep 3
    run_check_mode
}

# ============================================================================
# TEST MODE - Run standalone ANT+ test
# ============================================================================

run_test_mode() {
    echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
    echo -e "${BLUE}QDomyos-Zwift ANT+ Broadcasting Test${NC}"
    echo -e "${CYAN}Running for user: $TARGET_USER${NC}"
    echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
    echo ""
    
    # Check if qdomyos-zwift is already running
    if pgrep -f "qdomyos-zwift-bin" >/dev/null 2>&1; then
        echo -e "${RED}✗ qdomyos-zwift is already running!${NC}"
        echo ""
        echo "The ANT+ USB dongle can only be used by one process at a time."
        echo ""
        echo "Please stop qdomyos-zwift first:"
        echo -e "  ${YELLOW}sudo pkill -f qdomyos-zwift-bin${NC}"
        echo ""
        exit 1
    fi
    
    # Check if test script exists
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    if [ ! -f "$SCRIPT_DIR/test_ant.py" ]; then
        echo -e "${RED}✗ Test script not found: $SCRIPT_DIR/test_ant.py${NC}"
        echo ""
        echo "The test script should be in the same directory as setup.sh"
        exit 1
    fi
    
    # Check if virtual environment exists
    if [ ! -d "$TARGET_HOME/ant_venv" ]; then
        echo -e "${RED}✗ Virtual environment not found at: $TARGET_HOME/ant_venv${NC}"
        echo ""
        echo "Run setup first: sudo ./setup.sh --guided"
        exit 1
    fi
    
    echo "This test simulates treadmill data broadcasting via ANT+."
    echo "Your Garmin watch should pair as a Foot Pod within 5-10 seconds."
    echo ""
    echo "Expected readings:"
    echo "  - Pace: ~7:00 min/km (varying)"
    echo "  - Cadence: ~166 SPM"
    echo "  - Distance: accumulating"
    echo ""
    echo "Press Ctrl+C to stop the test"
    echo ""
    echo -e "${CYAN}Starting ANT+ test...${NC}"
    echo ""
    
    # Run test as target user (not root) to use their venv
    if ! sudo -u "$TARGET_USER" "$TARGET_HOME/ant_venv/bin/python3" "$SCRIPT_DIR/test_ant.py"; then
        echo ""
        echo -e "${RED}✗ Test failed${NC}"
        echo ""
        echo "Troubleshooting:"
        echo "  1. Ensure ANT+ dongle is plugged in"
        echo "  2. Check USB permissions: ./setup.sh --check"
        echo "  3. Verify Python packages: ./setup.sh --check"
        exit 1
    fi
}

# ============================================================================
# MAIN - Route to appropriate mode
# ============================================================================

if [ $MODE_CHECK -eq 1 ]; then
    run_check_mode
elif [ $MODE_RESET -eq 1 ]; then
    run_reset_mode
elif [ $MODE_GUIDED -eq 1 ]; then
    run_guided_mode
elif [ $MODE_TEST -eq 1 ]; then
    run_test_mode
else
    echo "Error: No valid mode selected"
    exit 1
fi