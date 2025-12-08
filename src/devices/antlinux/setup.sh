#!/bin/bash
################################################################################
# setup.sh - QDomyos-Zwift ANT+ Setup and Validation Tool
#
# Purpose:
#   Unified tool for validating and setting up ANT+ prerequisites.
#   Provides quick validation, guided setup, testing, and reset capabilities.
#
# Usage:
#   ./setup.sh --quick              # Fast validation check (no sudo)
#   sudo ./setup.sh --guided        # Guided setup with prompts
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
MODE_QUICK=0
MODE_GUIDED=0
MODE_RESET=0
MODE_TEST=0

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
    ./setup.sh --quick              # Fast validation (no sudo required)
    sudo ./setup.sh --guided        # Interactive guided setup
    sudo ./setup.sh --reset         # Remove configurations (undo setup)
    sudo ./setup.sh --test          # Test ANT+ broadcasting
    ./setup.sh --help               # Show this help

MODES:
    --quick        Quick validation showing all issues at once
                   - No sudo required
                   - Fast execution (~10 seconds)
                   - Exit codes: 0 (ready), 1 (failed), 2 (warnings)

    --guided       Interactive guided setup with explanations
                   - Requires sudo
                   - Step-by-step with prompts
                   - Explains each requirement before installation
                   - Recommended for all users

    --reset        Remove QDomyos-Zwift configurations (complete undo)
                   - Requires sudo
                   - Removes user from plugdev group
                   - Removes udev rules
                   - Optionally removes Python venv and packages
                   - Does NOT uninstall system packages (Qt5, libusb)
                   - Useful for testing or returning to clean state

    --test         Test ANT+ broadcasting without QDomyos-Zwift
                   - Requires sudo
                   - Simulates treadmill data
                   - Verifies ANT+ dongle and watch pairing
                   - Useful for isolating ANT+ issues

    --help         Show this help message

EXAMPLES:
    # Check what's needed
    ./setup.sh --quick

    # Guided setup (recommended)
    sudo ./setup.sh --guided

    # Test ANT+ broadcasting
    sudo ./setup.sh --test

    # Reset to clean state (for testing)
    sudo ./setup.sh --reset

EXIT CODES (--quick mode):
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
    echo "  --quick        Quick validation check"
    echo "  --guided       Interactive guided setup"
    echo "  --reset        Remove configurations"
    echo "  --test         Test ANT+ broadcasting"
    echo "  --help         Show detailed help"
    echo ""
    exit 1
fi

for arg in "$@"; do
    case $arg in
        --quick)
            MODE_QUICK=1
            shift
            ;;
        --guided)
            MODE_GUIDED=1
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
if [ $MODE_QUICK -eq 1 ] && [ $MODE_GUIDED -eq 1 ]; then
    echo "Error: Cannot use --quick and --guided together"
    exit 1
fi

if [ $MODE_TEST -eq 1 ] && [ $MODE_QUICK -eq 1 ]; then
    echo "Error: Cannot use --test and --quick together"
    exit 1
fi

if [ $MODE_RESET -eq 1 ] && [ $MODE_GUIDED -eq 1 ]; then
    echo "Error: Cannot use --reset and --guided together"
    exit 1
fi

# Check sudo requirement for guided/reset/test modes
if [ $MODE_GUIDED -eq 1 ] || [ $MODE_RESET -eq 1 ] || [ $MODE_TEST -eq 1 ]; then
    if [ $EUID -ne 0 ]; then
        echo -e "${RED}ERROR: --guided, --reset, and --test modes require sudo${NC}"
        echo "Please run: sudo $0 $@"
        exit 1
    fi
fi

# Helper function for quick mode
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
        read -p "$prompt [y/n]: " response
        case "$response" in
            [Yy]* ) return 0;;
            [Nn]* ) return 1;;
            * ) echo "Please answer y or n.";;
        esac
    done
}

# ============================================================================
# QUICK MODE - Fast validation showing all issues
# ============================================================================

run_quick_mode() {
    echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
    echo -e "${BLUE}QDomyos-Zwift ANT+ Quick Validation${NC}"
    echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
    echo ""
    
    # Test 1: Python 3.11 library (check system and pyenv)
    test_check \
        "python311_library" \
        "ldconfig -p | grep 'libpython3.11.so' || ls ~/.pyenv/versions/3.11.*/lib/libpython3.11.so* &>/dev/null" \
        "Python 3.11 library found" \
        "Python 3.11 library not found (run: sudo ./setup.sh --guided)" \
        "true"
    
    # Test 2: Virtual environment
    test_check \
        "ant_venv" \
        "[ -d \"$TARGET_HOME/ant_venv\" ]" \
        "Virtual environment exists at ~/ant_venv" \
        "Virtual environment not found (run: sudo ./setup.sh --guided)" \
        "false"
    
    # Test 3-5: Python packages (only if venv exists)
    if [ -d "$TARGET_HOME/ant_venv" ]; then
        test_check \
            "python_package_openant" \
            "$TARGET_HOME/ant_venv/bin/python -c 'import openant' 2>/dev/null" \
            "Python package 'openant' installed" \
            "Python package 'openant' missing (run: sudo ./setup.sh --guided)" \
            "true"
        
        test_check \
            "python_package_pyusb" \
            "$TARGET_HOME/ant_venv/bin/python -c 'import usb' 2>/dev/null" \
            "Python package 'pyusb' installed" \
            "Python package 'pyusb' missing (run: sudo ./setup.sh --guided)" \
            "true"
        
        test_check \
            "python_package_pybind11" \
            "$TARGET_HOME/ant_venv/bin/python -c 'import pybind11' 2>/dev/null" \
            "Python package 'pybind11' installed" \
            "Python package 'pybind11' missing (run: sudo ./setup.sh --guided)" \
            "true"
    fi
    
    # Test 6-8: Qt5 libraries
    test_check \
        "qt5_bluetooth" \
        "ldconfig -p | grep 'libQt5Bluetooth.so'" \
        "Qt5 Bluetooth library available" \
        "Qt5 Bluetooth missing (run: sudo ./setup.sh --guided)" \
        "true"
    
    test_check \
        "qt5_charts" \
        "ldconfig -p | grep 'libQt5Charts.so'" \
        "Qt5 Charts library available" \
        "Qt5 Charts missing (run: sudo ./setup.sh --guided)" \
        "true"
    
    test_check \
        "qt5_multimedia" \
        "ldconfig -p | grep 'libQt5Multimedia.so'" \
        "Qt5 Multimedia library available" \
        "Qt5 Multimedia missing (run: sudo ./setup.sh --guided)" \
        "true"
    
    # Test 9: plugdev group
    test_check \
        "plugdev_group" \
        "groups $TARGET_USER | grep plugdev" \
        "User '$TARGET_USER' is in 'plugdev' group" \
        "User not in 'plugdev' group (run: sudo ./setup.sh --guided)" \
        "true"
    
    # Test 10: udev rules
    test_check \
        "udev_rules" \
        "[ -f /etc/udev/rules.d/99-garmin-ant.rules ] || [ -f /etc/udev/rules.d/51-garmin-ant.rules ] || [ -f /etc/udev/rules.d/99-ant-usb.rules ]" \
        "ANT+ udev rules configured" \
        "ANT+ udev rules not found (run: sudo ./setup.sh --guided)" \
        "true"
    
    # Test 11: ANT+ dongle (non-critical)
    test_check \
        "ant_dongle" \
        "lsusb | grep -E '0fcf:1009|0fcf:1008|0fcf:100c|0fcf:100e|0fcf:88a4|0fcf:1004|11fd:0001'" \
        "ANT+ USB dongle detected" \
        "ANT+ USB dongle not detected (plug in Garmin/Suunto ANT+ stick)" \
        "false"
    
    # Test 12: Bluetooth service
    test_check \
        "bluetooth_service" \
        "systemctl is-active --quiet bluetooth" \
        "Bluetooth service running" \
        "Bluetooth service not running (run: sudo ./setup.sh --guided)" \
        "true"
    
    # Test 13: libusb
    test_check \
        "libusb" \
        "ldconfig -p | grep 'libusb-1.0.so'" \
        "libusb-1.0 library available" \
        "libusb-1.0 missing (run: sudo ./setup.sh --guided)" \
        "true"
    
    # Output results
    echo ""
    echo -e "${BLUE}───────────────────────────────────────────────────────────${NC}"
    echo -e "${BLUE}Test Summary:${NC} ${GREEN}$PASS passed${NC}, ${YELLOW}$WARN warnings${NC}, ${RED}$FAIL failed${NC}"
    echo -e "${BLUE}───────────────────────────────────────────────────────────${NC}"
    echo ""
    
    if [ $FAIL -gt 0 ]; then
        echo -e "${RED}System Status: FAILED${NC}"
        echo ""
        echo -e "${CYAN}To fix issues, run:${NC}"
        echo -e "  ${YELLOW}sudo ./setup.sh --guided${NC}"
        exit 1
    elif [ $WARN -gt 0 ]; then
        echo -e "${YELLOW}System Status: WARNING${NC}"
        echo "Non-critical warnings present - some functionality may be limited"
        exit 2
    else
        echo -e "${GREEN}System Status: READY${NC}"
        echo "All tests passed - system is ready for ANT+ operation"
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
    echo -e "${YELLOW}WARNING: This will remove QDomyos-Zwift configurations${NC}"
    echo ""
    echo "This will:"
    echo "  - Remove user from 'plugdev' group"
    echo "  - Remove ANT+ udev rules"
    echo ""
    echo "This will NOT:"
    echo "  - Uninstall system packages (Qt5, libusb, bluez)"
    echo ""
    echo -e "${CYAN}Use this to return to a clean state for testing${NC}"
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
    if groups "$TARGET_USER" | grep -q plugdev; then
        echo -e "${YELLOW}Removing $TARGET_USER from plugdev group...${NC}"
        if gpasswd -d "$TARGET_USER" plugdev 2>/dev/null; then
            echo -e "${GREEN}✓ User removed from plugdev group${NC}"
            echo -e "${YELLOW}⚠ You must logout and login for changes to take effect${NC}"
            ((reset_count++))
        else
            echo -e "${RED}✗ Failed to remove user from plugdev group${NC}"
            ((failed_count++))
        fi
        echo ""
    fi
    
    # Reset 2: Remove udev rules
    local rules_removed=0
    for rules_file in \
        /etc/udev/rules.d/99-ant-usb.rules \
        /etc/udev/rules.d/99-garmin-ant.rules \
        /etc/udev/rules.d/51-garmin-ant.rules; do
        
        if [ -f "$rules_file" ]; then
            echo -e "${YELLOW}Removing udev rules: $rules_file${NC}"
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
        echo ""
    fi
    
    # Reset 3: Optionally remove Python virtual environment
    if [ -d "$TARGET_HOME/ant_venv" ]; then
        echo -e "${YELLOW}Python virtual environment found at: $TARGET_HOME/ant_venv${NC}"
        echo ""
        read -p "Remove Python virtual environment and packages? [y/N]: " venv_response
        case "$venv_response" in
            [Yy]* )
                echo -e "${YELLOW}Removing virtual environment...${NC}"
                if rm -rf "$TARGET_HOME/ant_venv"; then
                    echo -e "${GREEN}✓ Virtual environment removed${NC}"
                    ((reset_count++))
                else
                    echo -e "${RED}✗ Failed to remove virtual environment${NC}"
                    ((failed_count++))
                fi
                ;;
            * )
                echo "Keeping virtual environment"
                ;;
        esac
        echo ""
    fi
    
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
    echo -e "Note: System packages (Qt5, libusb, bluez) remain installed"
    echo -e "Run '${YELLOW}./setup.sh --quick${NC}' to verify clean state"
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
    
    if ldconfig -p | grep -q 'libpython3.11.so' || ls "$TARGET_HOME/.pyenv/versions/3.11".*/lib/libpython3.11.so* &>/dev/null; then
        echo -e "${GREEN}✓ Python 3.11 is already installed${NC}"
    else
        echo -e "${YELLOW}✗ Python 3.11 not found${NC}"
        echo ""
        echo "Python 3.11 can be installed via:"
        echo "  A) System package manager (apt) - Simpler, if available"
        echo "  B) pyenv - More flexible, works on any distribution"
        echo ""
        
        if apt-cache show python3.11 >/dev/null 2>&1; then
            echo "Your system provides Python 3.11 via apt (recommended)."
            if prompt_yes_no "Install Python 3.11 via apt?"; then
                apt-get update
                apt-get install -y python3.11 python3.11-venv
                echo -e "${GREEN}✓ Python 3.11 installed${NC}"
            fi
        else
            echo "Your system doesn't provide Python 3.11 via apt."
            echo "You'll need to install it via pyenv manually (requires logout/login)."
            echo ""
            echo "Steps:"
            echo "  1. Install prerequisites: sudo apt-get install -y build-essential libssl-dev zlib1g-dev..."
            echo "  2. Install pyenv: curl https://pyenv.run | bash"
            echo "  3. Configure shell (.bashrc)"
            echo "  4. Logout and login"
            echo "  5. Install Python: pyenv install 3.11.9"
            echo ""
            echo "For detailed instructions, see: README.md section 3.1.2"
            echo -e "${YELLOW}⚠ Skipping Python installation - please install manually${NC}"
        fi
    fi
    echo ""
    
    # Step 2: Create virtual environment
    echo -e "${BLUE}[Step 2/6] Setting up virtual environment...${NC}"
    
    if [ -d "$TARGET_HOME/ant_venv" ]; then
        echo -e "${GREEN}✓ Virtual environment already exists${NC}"
    else
        echo "A Python virtual environment isolates ANT+ dependencies."
        if command -v python3.11 >/dev/null 2>&1; then
            if prompt_yes_no "Create virtual environment at ~/ant_venv?"; then
                sudo -u "$TARGET_USER" python3.11 -m venv "$TARGET_HOME/ant_venv"
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
            if prompt_yes_no "Install missing Python packages?"; then
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
    
    # Check critical libraries
    local libs_missing=0
    ldconfig -p | grep 'libQt5Bluetooth.so' >/dev/null 2>&1 || ((libs_missing++))
    ldconfig -p | grep 'libQt5Charts.so' >/dev/null 2>&1 || ((libs_missing++))
    ldconfig -p | grep 'libusb-1.0.so' >/dev/null 2>&1 || ((libs_missing++))
    systemctl list-unit-files | grep '^bluetooth.service' >/dev/null 2>&1 || ((libs_missing++))
    
    if [ $libs_missing -eq 0 ]; then
        echo -e "${GREEN}✓ All system libraries already installed${NC}"
    else
        echo -e "${YELLOW}Some system libraries are missing${NC}"
        if prompt_yes_no "Install Qt5 and USB libraries?"; then
            apt-get update
            apt-get install -y \
                libqt5bluetooth5 libqt5charts5 libqt5multimedia5 \
                libqt5networkauth5 libqt5positioning5 libqt5sql5 \
                libqt5texttospeech5 libqt5websockets5 libqt5xml5 \
                libusb-1.0-0 bluez
            echo -e "${GREEN}✓ System libraries installed${NC}"
        fi
    fi
    echo ""
    
    # Step 5: Configure USB permissions
    echo -e "${BLUE}[Step 5/6] Configuring USB permissions...${NC}"
    
    if ! groups "$TARGET_USER" | grep -q plugdev; then
        if prompt_yes_no "Add $TARGET_USER to 'plugdev' group?"; then
            usermod -aG plugdev "$TARGET_USER"
            echo -e "${GREEN}✓ User added to plugdev group${NC}"
            echo -e "${YELLOW}⚠ You must logout and login for changes to take effect${NC}"
        fi
    else
        echo -e "${GREEN}✓ User already in plugdev group${NC}"
    fi
    
    if [ ! -f /etc/udev/rules.d/99-ant-usb.rules ]; then
        if prompt_yes_no "Create ANT+ USB udev rules?"; then
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
    echo -e "${BLUE}[Step 6/6] Enabling Bluetooth service...${NC}"
    
    if ! systemctl is-active --quiet bluetooth; then
        if prompt_yes_no "Start and enable Bluetooth service?"; then
            systemctl start bluetooth
            systemctl enable bluetooth
            echo -e "${GREEN}✓ Bluetooth service started${NC}"
        fi
    else
        echo -e "${GREEN}✓ Bluetooth service already running${NC}"
    fi
    echo ""
    
    # Final summary
    echo -e "${CYAN}═══════════════════════════════════════════════════════════${NC}"
    echo -e "${GREEN}✓ Setup wizard complete!${NC}"
    echo -e "${CYAN}═══════════════════════════════════════════════════════════${NC}"
    echo ""
    echo "Next steps:"
    echo "  1. If you were added to 'plugdev' group, logout and login"
    echo -e "  2. Run validation: ${YELLOW}./setup.sh --quick${NC}"
    echo "  3. If all tests pass, you're ready to use ANT+!"
    echo ""
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
        echo "  2. Check USB permissions: ./setup.sh --quick"
        echo "  3. Verify Python packages: ./setup.sh --quick"
        exit 1
    fi
}

# ============================================================================
# MAIN - Route to appropriate mode
# ============================================================================

if [ $MODE_QUICK -eq 1 ]; then
    run_quick_mode
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