#!/bin/bash
# setup.sh - QDomyos-Zwift ANT+ Setup and Validation Tool
# 
# Purpose:
#   Unified tool for validating and setting up ANT+ prerequisites.
#   Combines quick validation, detailed diagnostics, and automated fixes.
#
# Features:
#   --quick       Quick validation (no sudo, shows all issues)
#   --interactive Interactive guided setup (with sudo, step-by-step)
#   --fix         Automated installation of fixable components
#   --json        JSON output for CI/CD integration
#   --help        Show usage information
#
# Usage:
#   ./setup.sh --quick              # Fast validation check
#   sudo ./setup.sh --test          # Validate + run ANT+ hardware test
#   sudo ./setup.sh --interactive   # Guided setup with prompts
#   sudo ./setup.sh --fix           # Auto-install what's possible
#
# Platform: Linux x86-64 and ARM64
# Dependencies: bash, ldconfig, systemctl, lsusb
# Author: bassai-sho
# Development assisted by AI analysis tools

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
MODE_INTERACTIVE=0
MODE_FIX=0
MODE_JSON=0
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

# Parse arguments
if [ $# -eq 0 ]; then
    echo "Error: No mode specified. Use --help for usage information."
    exit 1
fi

for arg in "$@"; do
    case $arg in
        --quick)
            MODE_QUICK=1
            shift
            ;;
        --interactive)
            MODE_INTERACTIVE=1
            shift
            ;;
        --fix)
            MODE_FIX=1
            shift
            ;;
        --json)
            MODE_JSON=1
            shift
            ;;
        --test)
            MODE_TEST=1
            shift
            ;;
        --help)
            grep '^#' "$0" | sed 's/^# //' | sed 's/^#//'
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
if [ $MODE_QUICK -eq 1 ] && [ $MODE_INTERACTIVE -eq 1 ]; then
    echo "Error: Cannot use --quick and --interactive together"
    exit 1
fi

if [ $MODE_QUICK -eq 1 ] && [ $MODE_FIX -eq 1 ]; then
    echo "Error: Cannot use --quick and --fix together (--fix requires sudo)"
    exit 1
fi

if [ $MODE_TEST -eq 1 ] && [ $MODE_INTERACTIVE -eq 1 ]; then
    echo "Error: Cannot use --test and --interactive together"
    exit 1
fi

if [ $MODE_TEST -eq 1 ] && [ $MODE_FIX -eq 1 ]; then
    echo "Error: Cannot use --test and --fix together"
    exit 1
fi

# Check sudo requirement for interactive/fix/test modes
if [ $MODE_INTERACTIVE -eq 1 ] || [ $MODE_FIX -eq 1 ] || [ $MODE_TEST -eq 1 ]; then
    if [ $EUID -ne 0 ]; then
        echo -e "${RED}ERROR: --interactive, --fix, and --test modes require sudo${NC}"
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
        if [ $MODE_JSON -eq 0 ]; then
            echo -e "${GREEN}[PASS]${NC} $success_msg"
        fi
        ((PASS++))
        TEST_RESULTS+=("{\"test\":\"$test_name\",\"status\":\"pass\",\"message\":\"$success_msg\"}")
        return 0
    else
        if [ "$is_critical" = "true" ]; then
            if [ $MODE_JSON -eq 0 ]; then
                echo -e "${RED}[FAIL]${NC} $failure_msg"
            fi
            ((FAIL++))
            TEST_RESULTS+=("{\"test\":\"$test_name\",\"status\":\"fail\",\"message\":\"$failure_msg\"}")
        else
            if [ $MODE_JSON -eq 0 ]; then
                echo -e "${YELLOW}[WARN]${NC} $failure_msg"
            fi
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
# QUICK MODE - Fast validation only
# ============================================================================

run_quick_mode() {
    local return_to_caller="${1:-false}"  # Allow test mode to continue after validation
    
    if [ $MODE_JSON -eq 0 ]; then
        echo -e "${BLUE}=== QDomyos-Zwift ANT+ Quick Validation ===${NC}"
        echo ""
    fi
    
    # Test 1: Python 3.11 library (check system and pyenv)
    test_check \
        "python311_library" \
        "ldconfig -p | grep 'libusb-1.0.so' || ls ~/.pyenv/versions/3.11.*/lib/libpython3.11.so* &>/dev/null" \
        "Python 3.11 library found" \
        "Python 3.11 library not found (run: sudo ./setup.sh --interactive)" \
        "true"
    
    # Test 2: Virtual environment
    test_check \
        "ant_venv" \
        "[ -d \"$TARGET_HOME/ant_venv\" ]" \
        "Virtual environment exists at ~/ant_venv" \
        "Virtual environment not found (run: sudo ./setup.sh --interactive)" \
        "false"
    
    # Test 3-5: Python packages (only if venv exists)
    if [ -d "$TARGET_HOME/ant_venv" ]; then
        test_check \
            "python_package_openant" \
            "$TARGET_HOME/ant_venv/bin/python -c 'import openant' 2>/dev/null" \
            "Python package 'openant' installed" \
            "Python package 'openant' missing (run: sudo ./setup.sh --fix)" \
            "true"
        
        test_check \
            "python_package_pyusb" \
            "$TARGET_HOME/ant_venv/bin/python -c 'import usb' 2>/dev/null" \
            "Python package 'pyusb' installed" \
            "Python package 'pyusb' missing (run: sudo ./setup.sh --fix)" \
            "true"
        
        test_check \
            "python_package_pybind11" \
            "$TARGET_HOME/ant_venv/bin/python -c 'import pybind11' 2>/dev/null" \
            "Python package 'pybind11' installed" \
            "Python package 'pybind11' missing (run: sudo ./setup.sh --fix)" \
            "true"
    fi
    
    # Test 6-8: Qt5 libraries
    test_check \
        "qt5_bluetooth" \
        "ldconfig -p | grep 'libQt5Bluetooth.so'" \
        "Qt5 Bluetooth library available" \
        "Qt5 Bluetooth missing (run: sudo ./setup.sh --fix)" \
        "true"
    
    test_check \
        "qt5_charts" \
        "ldconfig -p | grep 'libQt5Charts.so'" \
        "Qt5 Charts library available" \
        "Qt5 Charts missing (run: sudo ./setup.sh --fix)" \
        "true"
    
    test_check \
        "qt5_multimedia" \
        "ldconfig -p | grep 'libQt5Multimedia.so'" \
        "Qt5 Multimedia library available" \
        "Qt5 Multimedia missing (run: sudo ./setup.sh --fix)" \
        "true"
    
    # Test 9: plugdev group
    test_check \
        "plugdev_group" \
        "groups $TARGET_USER | grep plugdev" \
        "User '$TARGET_USER' is in 'plugdev' group" \
        "User not in 'plugdev' group (run: sudo ./setup.sh --fix)" \
        "true"
    
    # Test 10: udev rules
    test_check \
        "udev_rules" \
        "[ -f /etc/udev/rules.d/99-garmin-ant.rules ] || [ -f /etc/udev/rules.d/51-garmin-ant.rules ] || [ -f /etc/udev/rules.d/99-ant-usb.rules ]" \
        "ANT+ udev rules configured" \
        "ANT+ udev rules not found (run: sudo ./setup.sh --fix)" \
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
        "Bluetooth service not running (run: sudo ./setup.sh --fix)" \
        "true"
    
    # Test 13: libusb
    test_check \
        "libusb" \
        "ldconfig -p | grep 'libusb-1.0.so'" \
        "libusb-1.0 library available" \
        "libusb-1.0 missing (run: sudo ./setup.sh --fix)" \
        "true"
    
    # Output results
    echo ""
    
    if [ $MODE_JSON -eq 1 ]; then
        # JSON output
        echo "{"
        echo "  \"total_tests\": $((PASS + FAIL + WARN)),"
        echo "  \"passed\": $PASS,"
        echo "  \"failed\": $FAIL,"
        echo "  \"warnings\": $WARN,"
        echo "  \"results\": ["
        
        first=1
        for result in "${TEST_RESULTS[@]}"; do
            if [ $first -eq 0 ]; then
                echo ","
            fi
            echo "    $result"
            first=0
        done
        
        echo ""
        echo "  ],"
        
        if [ $FAIL -gt 0 ]; then
            echo "  \"status\": \"FAILED\","
            echo "  \"message\": \"Run 'sudo ./setup.sh --interactive' or 'sudo ./setup.sh --fix' to resolve issues\""
        elif [ $WARN -gt 0 ]; then
            echo "  \"status\": \"WARNING\","
            echo "  \"message\": \"Non-critical warnings present - functionality may be limited\""
        else
            echo "  \"status\": \"READY\","
            echo "  \"message\": \"All tests passed - system ready for ANT+ operation\""
        fi
        
        echo "}"
    else
        # Human-readable output
        echo -e "${BLUE}Test Summary:${NC} ${GREEN}$PASS passed${NC}, ${YELLOW}$WARN warnings${NC}, ${RED}$FAIL failed${NC}"
        echo ""
        
        if [ $FAIL -gt 0 ]; then
            echo -e "${RED}System Status: FAILED${NC}"
            echo "To fix issues, run: ${YELLOW}sudo ./setup.sh --interactive${NC} (guided)"
            echo "            or run: ${YELLOW}sudo ./setup.sh --fix${NC} (automatic)"
            [ "$return_to_caller" = "false" ] && exit 1
        elif [ $WARN -gt 0 ]; then
            echo -e "${YELLOW}System Status: WARNING${NC}"
            echo "Non-critical warnings present - some functionality may be limited"
            [ "$return_to_caller" = "false" ] && exit 2
        else
            echo -e "${GREEN}System Status: READY${NC}"
            echo "All tests passed - system is ready for ANT+ operation"
            [ "$return_to_caller" = "false" ] && exit 0
        fi
    fi
}

# ============================================================================
# FIX MODE - Automated installation
# ============================================================================

run_fix_mode() {
    echo -e "${BLUE}=== QDomyos-Zwift ANT+ Automated Fix ===${NC}"
    echo -e "${CYAN}Running for user: $TARGET_USER${NC}"
    echo ""
    
    local fixed=0
    local failed=0
    
    # Fix 1: Install Qt5 libraries
    if ! ldconfig -p | grep 'libQt5Bluetooth.so' >/dev/null 2>&1; then
        echo -e "${YELLOW}Installing Qt5 libraries...${NC}"
        if apt-get update && apt-get install -y \
            libqt5bluetooth5 libqt5charts5 libqt5multimedia5 \
            libqt5networkauth5 libqt5positioning5 libqt5sql5 \
            libqt5texttospeech5 libqt5websockets5 libqt5xml5; then
            echo -e "${GREEN}✓ Qt5 libraries installed${NC}"
            ((fixed++))
        else
            echo -e "${RED}✗ Failed to install Qt5 libraries${NC}"
            ((failed++))
        fi
        echo ""
    fi
    
    # Fix 2: Install libusb
    if ! ldconfig -p | grep 'libusb-1.0.so' >/dev/null 2>&1; then
        echo -e "${YELLOW}Installing libusb-1.0...${NC}"
        if apt-get install -y libusb-1.0-0; then
            echo -e "${GREEN}✓ libusb-1.0 installed${NC}"
            ((fixed++))
        else
            echo -e "${RED}✗ Failed to install libusb-1.0${NC}"
            ((failed++))
        fi
        echo ""
    fi
    
    # Fix 3: Add user to plugdev group
    if ! groups "$TARGET_USER" | grep -q plugdev; then
        echo -e "${YELLOW}Adding $TARGET_USER to plugdev group...${NC}"
        if usermod -aG plugdev "$TARGET_USER"; then
            echo -e "${GREEN}✓ User added to plugdev group${NC}"
            echo -e "${YELLOW}⚠ You must logout and login for group changes to take effect${NC}"
            ((fixed++))
        else
            echo -e "${RED}✗ Failed to add user to plugdev group${NC}"
            ((failed++))
        fi
        echo ""
    fi
    
    # Fix 4: Create udev rules
    if [ ! -f /etc/udev/rules.d/99-ant-usb.rules ]; then
        echo -e "${YELLOW}Creating ANT+ udev rules...${NC}"
        cat > /etc/udev/rules.d/99-ant-usb.rules <<'EOF'
SUBSYSTEM=="usb", ATTRS{idVendor}=="0fcf", ATTRS{idProduct}=="100?", MODE="0666", GROUP="plugdev"
SUBSYSTEM=="usb", ATTRS{idVendor}=="0fcf", ATTRS{idProduct}=="88a4", MODE="0666", GROUP="plugdev"
SUBSYSTEM=="usb", ATTRS{idVendor}=="11fd", ATTRS{idProduct}=="0001", MODE="0666", GROUP="plugdev"
EOF
        if [ -f /etc/udev/rules.d/99-ant-usb.rules ]; then
            udevadm control --reload-rules
            udevadm trigger
            echo -e "${GREEN}✓ ANT+ udev rules created${NC}"
            ((fixed++))
        else
            echo -e "${RED}✗ Failed to create udev rules${NC}"
            ((failed++))
        fi
        echo ""
    fi
    
    # Fix 5: Start Bluetooth service
    if ! systemctl is-active --quiet bluetooth; then
        echo -e "${YELLOW}Starting Bluetooth service...${NC}"
        if systemctl start bluetooth && systemctl enable bluetooth; then
            echo -e "${GREEN}✓ Bluetooth service started and enabled${NC}"
            ((fixed++))
        else
            echo -e "${RED}✗ Failed to start Bluetooth service${NC}"
            ((failed++))
        fi
        echo ""
    fi
    
    # Fix 6: Install Python packages in venv (if venv exists)
    if [ -d "$TARGET_HOME/ant_venv" ]; then
        local packages_missing=0
        
        if ! sudo -u "$TARGET_USER" "$TARGET_HOME/ant_venv/bin/python" -c 'import openant' 2>/dev/null; then
            ((packages_missing++))
        fi
        if ! sudo -u "$TARGET_USER" "$TARGET_HOME/ant_venv/bin/python" -c 'import usb' 2>/dev/null; then
            ((packages_missing++))
        fi
        if ! sudo -u "$TARGET_USER" "$TARGET_HOME/ant_venv/bin/python" -c 'import pybind11' 2>/dev/null; then
            ((packages_missing++))
        fi
        
        if [ $packages_missing -gt 0 ]; then
            echo -e "${YELLOW}Installing Python packages in virtual environment...${NC}"
            if sudo -u "$TARGET_USER" "$TARGET_HOME/ant_venv/bin/pip" install openant pyusb pybind11; then
                echo -e "${GREEN}✓ Python packages installed${NC}"
                ((fixed++))
            else
                echo -e "${RED}✗ Failed to install Python packages${NC}"
                ((failed++))
            fi
            echo ""
        fi
    fi
    
    # Summary
    echo -e "${BLUE}========================================${NC}"
    if [ $fixed -gt 0 ]; then
        echo -e "${GREEN}✓ Fixed $fixed issue(s)${NC}"
    fi
    if [ $failed -gt 0 ]; then
        echo -e "${RED}✗ Failed to fix $failed issue(s)${NC}"
    fi
    if [ $fixed -eq 0 ] && [ $failed -eq 0 ]; then
        echo -e "${GREEN}✓ No fixes needed - system already configured${NC}"
    fi
    echo ""
    echo -e "Run '${YELLOW}./setup.sh --quick${NC}' to verify all issues are resolved"
}

# ============================================================================
# INTERACTIVE MODE - Guided step-by-step setup
# ============================================================================

run_interactive_mode() {
    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN}QDomyos-Zwift ANT+ Setup Wizard${NC}"
    echo -e "${CYAN}Running for user: $TARGET_USER${NC}"
    echo -e "${CYAN}========================================${NC}"
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
            echo "For detailed instructions, see: README.md section 1.2"
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
    
    # Check critical libraries (match quick mode logic exactly)
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
    echo -e "${CYAN}========================================${NC}"
    echo -e "${GREEN}✓ Setup wizard complete!${NC}"
    echo -e "${CYAN}========================================${NC}"
    echo ""
    echo "Next steps:"
    echo "  1. If you were added to 'plugdev' group, logout and login"
    echo -e "  2. Run validation: ${YELLOW}./setup.sh --quick${NC}"
    echo "  3. If all tests pass, you're ready to use ANT+!"
    echo ""
}

# ============================================================================
# TEST MODE - Run validation + ANT+ hardware test
# ============================================================================

run_test_mode() {
    echo -e "${CYAN}=== QDomyos-Zwift ANT+ Validation and Test ===${NC}"
    echo ""
    
    # Run quick validation first (pass true to prevent exit)
    run_quick_mode true
    
    # Check if all tests passed
    if [ $FAIL -gt 0 ]; then
        echo ""
        echo -e "${RED}Cannot run ANT+ test - validation failures detected${NC}"
        echo "Fix issues with: ${YELLOW}sudo ./setup.sh --fix${NC} or ${YELLOW}sudo ./setup.sh --interactive${NC}"
        exit 1
    fi
    
    # Check if test_ant.py exists
    SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
    TEST_SCRIPT="$SCRIPT_DIR/test_ant.py"
    
    if [ ! -f "$TEST_SCRIPT" ]; then
        echo ""
        echo -e "${RED}Error: test_ant.py not found in $SCRIPT_DIR${NC}"
        echo "This script should be included in the binary distribution."
        exit 1
    fi
    
    # Run the ANT+ test
    echo ""
    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN}Starting ANT+ Hardware Test${NC}"
    echo -e "${CYAN}========================================${NC}"
    echo ""
    echo "This will simulate a running treadmill broadcasting ANT+ data."
    echo "Your watch should pair within 5-10 seconds and display:"
    echo "  - Pace: ~7:00 min/km (varying)"
    echo "  - Cadence: ~166 SPM"
    echo "  - Distance: accumulating"
    echo ""
    echo -e "${YELLOW}Press Ctrl+C to stop the test${NC}"
    echo ""
    
    # Run the test with the venv Python
    if [ -d "$TARGET_HOME/ant_venv" ]; then
        "$TARGET_HOME/ant_venv/bin/python3" "$TEST_SCRIPT"
    else
        echo -e "${RED}Error: Virtual environment not found at $TARGET_HOME/ant_venv${NC}"
        exit 1
    fi
}

# ============================================================================
# MAIN - Route to appropriate mode
# ============================================================================

if [ $MODE_QUICK -eq 1 ]; then
    run_quick_mode
elif [ $MODE_FIX -eq 1 ]; then
    run_fix_mode
elif [ $MODE_INTERACTIVE -eq 1 ]; then
    run_interactive_mode
elif [ $MODE_TEST -eq 1 ]; then
    run_test_mode
else
    echo "Error: No valid mode selected"
    exit 1
fi
