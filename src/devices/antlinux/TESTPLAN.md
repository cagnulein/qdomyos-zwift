# ANT+ Pre-compiled Binary Test Plan

## Overview

This document provides comprehensive testing procedures for the QDomyos-Zwift ANT+ Virtual Footpod pre-compiled binaries. It's designed for developers and testers.

**Related Documentation:**
- [Main Installation Guide (README.md)](README.md) - For standard users installing the application
- [Compilation Guide (COMPILE.md)](COMPILE.md) - For building from source

## Objective

Validate that the pre-compiled binaries for Raspberry Pi (ARM64) and Desktop Linux (x86-64) work correctly on fresh installations and handle missing dependencies gracefully with clear error messages.

## Test Environment Setup

### Test Environment 1: Raspberry Pi
- **Hardware:** Raspberry Pi Zero 2 W, 3, 4, or 5
- **OS:** Fresh Raspberry Pi OS Bookworm installation (minimal or desktop)
- **Initial State:** No additional packages installed beyond base OS

### Test Environment 2: Desktop Linux
- **Hardware:** x86-64 PC/Laptop
- **OS:** Fresh Ubuntu 24.04 LTS installation (or Debian Bookworm)
- **Initial State:** No additional packages installed beyond base OS

### Required Test Hardware
- **ANT+ USB dongle:** Garmin USB2 (0fcf:1008) or USB-m (0fcf:1009)
- **Garmin watch:** For pairing tests
- **Treadmill:** Physical Bluetooth-enabled treadmill (see [compatibility list](https://github.com/cagnulein/qdomyos-zwift#compatible-devices)) OR virtual treadmill setup (see TC7)

---

## Pre-Test Setup: Establishing Baseline Environment

**For Testers and Developers:**

Before each test iteration, reset your system to a clean baseline using the automated cleanup script.

### Download and Run Cleanup Script

```bash
# Download the cleanup script
curl -o ~/testplan_cleanup.sh https://raw.githubusercontent.com/cagnulein/qdomyos-zwift/master/src/devices/antlinux/testplan_cleanup.sh
chmod +x ~/testplan_cleanup.sh

# Run the cleanup script
~/testplan_cleanup.sh
```

**What the script does:**
1. Stops and disables qz.service
2. Removes binary and scripts
3. Removes Python virtual environment
4. Optionally removes pyenv installation
5. Removes USB permissions and udev rules
6. Removes user from plugdev group
7. Backs up and removes root configuration
8. Shows optional system package removal commands

**After cleanup:**
- Log out and log back in (or reboot) for group changes to take effect
- Run verification: `su - $USER` then check `groups | grep plugdev` (should be empty)
- System is now ready for fresh test iteration

---

## Test Preparation

Before starting test cases, download the runtime check script:

```bash
curl -o ~/runtime_check.sh https://raw.githubusercontent.com/cagnulein/qdomyos-zwift/master/src/devices/antlinux/runtime_check.sh
chmod +x ~/runtime_check.sh
```

**Note:** The runtime check script must be run with sudo: `sudo ~/runtime_check.sh`

This script validates the system state throughout testing.

---

## Test Cases

### TC1: Fresh System - Missing All Dependencies

**Purpose:** Verify the application exits gracefully when all dependencies are missing and provides clear error messages.

**Initial Setup:**
- Fresh OS installation
- No Python 3.11, no Qt5 libraries, no virtual environment
- Run cleanup script if needed to ensure baseline state

**Test Steps:**
1. Run runtime check: `sudo ~/runtime_check.sh`
   - **Expected:** Fails on all 6 checks (Python 3.11, venv, Qt5, USB permissions, ANT+ dongle, Bluetooth)
   - **Verify:** Provides installation commands for each failed check
   - **Verify:** Shows both apt-get and pyenv methods for Python 3.11 installation
   - **Verify:** Script detects whether Python 3.11 is available via apt-get
   - **Verify:** Notes that Python 3.11 may be unavailable on older (e.g., Ubuntu 20.04) or newer (e.g., Debian Trixie) distributions
   - **Verify:** For Bluetooth check, shows `sudo apt-get install bluez` if service is missing
2. Download appropriate binary from GitHub Releases:
   - **Raspberry Pi:** `qdomyos-zwift-arm64-ant`
   - **Desktop Linux:** `qdomyos-zwift-x86-64-ant`
3. Transfer to device home directory as `~/qdomyos-zwift`
4. Make executable: `chmod +x ~/qdomyos-zwift`
5. Run: `sudo ./qdomyos-zwift -no-gui -ant-footpod`

**Expected Results:**
- Runtime check clearly identifies all missing components
- Application fails to start immediately
- Error message: "error while loading shared libraries: libQt5Bluetooth.so.5" (or similar Qt5 library)
- Application exits without crash
- No log file created (application can't start)

**Pass Criteria:**
- Runtime check identifies all issues correctly
- Runtime check shows appropriate Python 3.11 installation method based on distribution
- Runtime check notes that Python 3.11 may be unavailable on older or newer distributions
- Application error message names the specific missing library
- No segmentation faults or unhandled exceptions

**Cleanup for Next Test:**
- None needed (nothing was installed)

---

### TC2: Python 3.11 via pyenv - Missing Qt5 Libraries

**Purpose:** Verify error handling when Python environment exists but Qt5 libraries are missing. Use pyenv to ensure consistent Python version testing across all distributions.

**Note:** This test case uses pyenv regardless of whether Python 3.11 is available via system packages. This allows testing Python version management and ensures consistent test execution across distributions (older distributions with Python 3.9, current distributions with Python 3.11, or newer distributions with Python 3.12+).

**Setup Steps:**
1. Starting from TC1 state
2. Install pyenv and Python 3.11:
   ```bash
   # Install pyenv dependencies (including git and curl required by pyenv installer)
   sudo apt-get install -y git curl build-essential libssl-dev zlib1g-dev libbz2-dev \
     libreadline-dev libsqlite3-dev wget llvm libncurses5-dev \
     libncursesw5-dev xz-utils tk-dev libffi-dev liblzma-dev
   
   # Install pyenv
   curl https://pyenv.run | bash
   
   # Add to shell
   echo 'export PYENV_ROOT="$HOME/.pyenv"' >> ~/.bashrc
   echo 'command -v pyenv >/dev/null || export PATH="$PYENV_ROOT/bin:$PATH"' >> ~/.bashrc
   echo 'eval "$(pyenv init -)"' >> ~/.bashrc
   source ~/.bashrc
   
   # Install Python 3.11
   pyenv install 3.11.9
   pyenv global 3.11.9
   ```
3. Verify: `python --version` (should show Python 3.11.9)
4. Create venv: `python3.11 -m venv ~/ant_venv`
5. Install Python packages: `~/ant_venv/bin/pip install --upgrade pip && ~/ant_venv/bin/pip install openant pyusb pybind11`

**Test Steps:**
1. Run runtime check: `sudo ~/runtime_check.sh`
   - **Expected:** Passes Python 3.11 and venv checks
   - **Expected:** Fails on Qt5 libraries, USB permissions, ANT+ dongle, Bluetooth
   - **Verify:** Provides correct apt-get command for Qt5 libraries
2. Run: `sudo ./qdomyos-zwift -no-gui -ant-footpod`

**Expected Results:**
- Runtime check correctly identifies Python is OK but Qt5 is missing
- Application fails to start immediately
- Error message indicates missing Qt5 libraries
- Application exits without crash

**Pass Criteria:**
- Runtime check passes Python/venv checks, fails Qt5
- Clear indication of which Qt5 libraries are missing
- No crash or hanging

**Cleanup for Next Test:**
- Keep Python 3.11 (pyenv) and venv installed
- Proceed to TC3

---

### TC3: All Dependencies Installed - Missing Virtual Environment

**Purpose:** Verify application behavior when system dependencies exist but the required venv is missing.

**Setup Steps:**
1. Starting from TC2 state
2. Install all system dependencies: 
   ```bash
   sudo apt-get install -y libqt5bluetooth5 libqt5charts5 libqt5multimedia5 \
   libqt5networkauth5 libqt5positioning5 libqt5sql5 libqt5texttospeech5 \
   libqt5websockets5 libqt5xml5 libusb-1.0-0 usbutils bluez
   ```
3. Remove the venv: `rm -rf ~/ant_venv`

**Test Steps:**
1. Run runtime check: `sudo ~/runtime_check.sh`
   - **Expected:** Passes Python 3.11 and Qt5 checks
   - **Expected:** Fails on venv (not found at ~/ant_venv), USB permissions, ANT+ dongle, Bluetooth
   - **Verify:** Provides venv creation commands
2. Run: `sudo ./qdomyos-zwift -no-gui -ant-footpod`

**Expected Results:**
- Runtime check identifies missing venv at expected location
- Application starts (Qt5 libraries are present)
- ANT+ feature fails to initialize
- Error mentions the expected path: `~/ant_venv`

**Pass Criteria:**
- Runtime check accurately identifies missing venv
- Clear error message about missing venv
- Application doesn't crash

**Cleanup for Next Test:**
- Recreate venv: `python3.11 -m venv ~/ant_venv`
- Proceed to TC4

---

### TC4: Virtual Environment Exists - Missing Python Packages

**Purpose:** Verify error handling when venv exists but required Python packages are not installed.

**Setup Steps:**
1. Starting from TC3 state (venv recreated but empty)
2. Do NOT install Python packages

**Test Steps:**
1. Run runtime check: `sudo ~/runtime_check.sh`
   - **Expected:** Passes Python 3.11, Qt5, and venv existence checks
   - **Expected:** Fails on Python packages (identifies specific missing packages), USB permissions, ANT+ dongle, Bluetooth
   - **Verify:** Provides correct pip install command
2. Run: `sudo ./qdomyos-zwift -no-gui -ant-footpod`

**Expected Results:**
- Runtime check identifies specific missing packages (openant, pyusb, pybind11)
- Application starts
- ANT+ feature attempts to initialize but fails
- Python error: "ModuleNotFoundError: No module named 'openant'" or similar
- Application handles the error gracefully

**Pass Criteria:**
- Runtime check identifies all missing packages
- Specific missing Python package named in application error
- No crash or segmentation fault

**Cleanup for Next Test:**
- Install packages: `~/ant_venv/bin/pip install openant pyusb pybind11`
- Proceed to TC5

---

### TC5: All Dependencies Present - Missing USB Permissions

**Purpose:** Verify behavior when all software is installed but USB permissions aren't configured.

**Setup Steps:**
1. Starting from TC4 state (all dependencies installed)
2. Skip USB permissions configuration (no udev rules)
3. Ensure user is NOT in plugdev group
4. Connect ANT+ dongle

**Test Steps:**
1. Run runtime check: `sudo ~/runtime_check.sh`
   - **Expected:** Passes Python, Qt5, venv, and packages checks
   - **Expected:** Fails/warns on USB permissions (no udev rule, not in plugdev), ANT+ dongle detection, Bluetooth
   - **Verify:** Provides udev rule creation commands (complete heredoc block)
2. Run: `sudo ./qdomyos-zwift -no-gui -ant-footpod`

**Expected Results:**
- Runtime check identifies USB permission issues
- Application starts successfully
- Since running with `sudo`, USB access may work despite missing udev rules
- Application remains stable

**Pass Criteria:**
- Runtime check identifies USB permission configuration gaps
- Runtime check provides complete, copy-pasteable udev rule block
- Application doesn't crash
- Any USB-related errors are logged clearly

**Cleanup for Next Test:**
- Configure USB permissions as per runtime check output
- Add user to plugdev group: `sudo usermod -aG plugdev $USER`
- Create udev rules and reboot
- Proceed to TC6

---

### TC6: Wrong Python Version via pyenv

**Purpose:** Verify error handling when venv is created with wrong Python version. Use pyenv to install and test with Python 3.10.

**Setup Steps:**
1. Starting from TC5 state
2. Uninstall Python 3.11 and install Python 3.10 via pyenv:
   ```bash
   # Uninstall Python 3.11
   pyenv uninstall 3.11.9
   
   # Install Python 3.10
   pyenv install 3.10.14
   pyenv global 3.10.14
   
   # Verify
   python --version  # Should show Python 3.10.14
   ```
3. Remove existing venv: `rm -rf ~/ant_venv`
4. Create venv with Python 3.10: `python3.10 -m venv ~/ant_venv`
5. Install packages: `~/ant_venv/bin/pip install openant pyusb pybind11`

**Test Steps:**
1. Run runtime check: `sudo ~/runtime_check.sh`
   - **Expected:** Fails on Python 3.11 check
   - **Expected:** Detects Python 3.10 is installed but 3.11 is required
   - **Verify:** Shows pyenv installation commands for Python 3.11
2. Run: `sudo ./qdomyos-zwift -no-gui -ant-footpod`

**Expected Results:**
- Runtime check identifies Python 3.11 is not installed
- Application fails to start
- Error message: "error while loading shared libraries: libpython3.11.so.1.0: cannot open shared object file"
- Clear indication that Python 3.11 specifically is required

**Pass Criteria:**
- Runtime check correctly identifies Python 3.11 absence
- Runtime check suggests pyenv installation method
- Application error clearly mentions libpython3.11.so.1.0
- No crash or hanging

**Cleanup for Next Test:**
- Remove wrong venv: `rm -rf ~/ant_venv`
- Reinstall Python 3.11 via pyenv:
  ```bash
  pyenv uninstall 3.10.14
  pyenv install 3.11.9
  pyenv global 3.11.9
  python --version  # Verify Python 3.11.9
  ```
- Recreate venv with Python 3.11: `python3.11 -m venv ~/ant_venv`
- Install packages: `~/ant_venv/bin/pip install openant pyusb pybind11`
- Proceed to TC7

---

### TC7: Complete Correct Setup - Success Case

**Purpose:** Verify full functionality when all prerequisites are correctly configured.

**Setup Steps:**
1. Complete all README steps exactly (Steps 1.1, 1.2, 1.3, 1.4 including reboot)
2. Connect ANT+ dongle
3. Have Garmin watch ready for pairing
4. Set up treadmill (physical or virtual - see below)

#### Option A: Physical Treadmill

Use a Bluetooth-enabled treadmill compatible with QDomyos-Zwift (see [compatibility list](https://github.com/cagnulein/qdomyos-zwift#compatible-devices)).

#### Option B: Virtual Treadmill Setup

If you don't have access to a compatible physical treadmill, use QDomyos-Zwift's built-in virtual/fake treadmill feature.

**Device Roles:**

| Device | Role | Runs | Purpose |
|--------|------|------|---------|
| **Test Device** | ANT+ broadcaster | `sudo ./qdomyos-zwift -no-gui -ant-footpod` | Creates fake treadmill, broadcasts ANT+ data |
| **Remote Control Device** | Speed controller | QZ mobile/desktop app | Connects to "KICKR RUN", controls speed |
| **Garmin Watch** | Data receiver | Fitness activity | Pairs as footpod, displays pace/cadence |

**Configuration Steps:**

1. **On Test Device** - Create configuration file:
   ```bash
   sudo mkdir -p "/root/.config/Roberto Viola"
   sudo tee "/root/.config/Roberto Viola/qDomyos-Zwift.conf" > /dev/null << 'EOF'
   [General]
   fakedevice_treadmill=true
   treadmill_force_speed=true
   virtual_device_bluetooth=true
   virtualtreadmill=true
   EOF
   ```

2. **On Remote Control Device** - Download QDomyos-Zwift:
   - **Android:** [QDomyos-Zwift on Google Play](https://play.google.com/store/apps/details?id=org.cagnulen.qdomyoszwift)
   - **iOS:** [QDomyos-Zwift on App Store](https://apps.apple.com/app/qdomyos-zwift/id1543684991)
   - **Desktop:** Use another Linux/Windows/Mac computer with QZ installed

**Test Steps:**
1. Run runtime check: `sudo ~/runtime_check.sh`
   - **Expected:** All 6 checks pass (✓ green)
   - **Expected:** Final message: "All checks passed! Your runtime environment is ready for ANT+."

2. **For Physical Treadmill:**
   - Start test device: `sudo ./qdomyos-zwift -no-gui -ant-footpod`
   - Start your physical treadmill
   
3. **For Virtual Treadmill:**
   - Start test device: `sudo ./qdomyos-zwift -no-gui -ant-footpod` (or without `-no-gui` if using GUI)
   - On remote control device: Run setup wizard for a Treadmill
   - Connect to "KICKR RUN" (test device's fake treadmill)
   - Complete all wizard steps
   - Open Settings → Enable **"Horizon Treadmill Force FTMS"**
   - Save and return to main screen

4. On Garmin watch: Menu > Sensors & Accessories > Add New > Foot Pod
5. Wait for pairing
6. Start "Treadmill" or "Run Indoor" activity on watch
7. Control treadmill speed (physical controls or remote device speed controls)
8. Observe watch display for 5+ minutes
9. Vary speed to test cadence switching (below/above 7.0 km/h)

**Expected Results:**
- Runtime check shows all green checkmarks
- Application starts successfully without errors
- Console shows: "ANT+ Footpod broadcaster initialized" or similar
- **For Virtual Treadmill:** Remote device connects to "KICKR RUN", speed changes show relay messages in test device console
- **For Physical Treadmill:** Application connects to treadmill via Bluetooth
- Watch detects and pairs within a few seconds
- Watch displays pace data that matches treadmill speed
- Watch displays cadence data (90-140 SPM walking, 160-200 SPM running)
- Cadence switches appropriately at ~7.0 km/h threshold
- No errors or warnings in logs
- Application runs stably for 5+ minutes

**Pass Criteria:**
- Runtime check passes all 6 checks
- Clean startup with no errors
- Treadmill connection successful (remote control for virtual, Bluetooth for physical)
- Watch pairs successfully
- Pace data is accurate and updates smoothly
- Cadence data displays correctly
- Application runs stably without crashes

**Cleanup for Next Test:**
- Leave system in this working state
- Proceed to TC8

---

### TC8: Systemd Service Configuration

**Purpose:** Verify the application runs correctly as a systemd service and starts automatically on boot.

**Setup Steps:**
1. Starting from TC7 state (successful manual run)
2. Configure systemd service as per README Step 3.3
3. Ensure username is correctly replaced in service file

**Test Steps:**
1. Run final runtime check: `sudo ~/runtime_check.sh`
   - **Expected:** All checks pass
2. Create service file: `sudo nano /etc/systemd/system/qz.service`
3. Add configuration with correct paths and username
4. Run:
   ```bash
   sudo systemctl daemon-reload
   sudo systemctl enable qz
   sudo systemctl start qz
   ```
5. Check status: `sudo systemctl status qz`
6. **For Virtual Treadmill:** Connect remote control device to "KICKR RUN"
7. **For Physical Treadmill:** Start your physical treadmill
8. Verify watch can pair
9. Reboot system: `sudo reboot`
10. After boot, check status: `sudo systemctl status qz`
11. **For Virtual Treadmill:** Verify remote can reconnect
12. Verify watch can still pair and receive data
13. Test graceful shutdown: `sudo systemctl stop qz`

**Expected Results:**
- Runtime check confirms system is ready
- Service starts successfully
- Status shows "active (running)"
- Treadmill connection works (remote control for virtual, Bluetooth for physical)
- Watch can pair and receive data
- Service starts automatically after reboot
- Treadmill reconnects after reboot
- Service stops gracefully within 5 seconds

**Pass Criteria:**
- Service runs without errors
- Automatic startup works reliably
- Watch functionality identical to manual run
- Clean shutdown without hanging

---

## Acceptance Checklist

Before releasing a new binary build, verify:

- [ ] TC1-TC6 (error cases) produce clear, helpful error messages
- [ ] TC6 specifically validates wrong Python version handling (3.10 vs required 3.11)
- [ ] Runtime check provides accurate diagnostics in all states
- [ ] Runtime check detects distribution-specific Python 3.11 availability (older and newer distributions)
- [ ] Runtime check shows appropriate installation method (apt-get vs pyenv)
- [ ] Runtime check provides complete, copy-pasteable udev rule block
- [ ] Runtime check suggestions are actionable with working commands
- [ ] Runtime check completes in under 10 seconds in all states
- [ ] Runtime check must be run with sudo (script validates this)
- [ ] All 6 runtime checks must pass for success in TC7
- [ ] TC7 (success case) works on both Raspberry Pi and Desktop Linux
- [ ] TC7 runs stably for 5+ minutes with accurate data
- [ ] TC7 runtime check shows all 6 green passes
- [ ] TC8 (systemd) starts reliably on boot on both platforms
- [ ] TC8 stops gracefully without hanging
- [ ] All error messages reviewed for clarity and helpfulness
- [ ] Documentation matches actual application behavior
- [ ] No crashes, hangs, or segfaults observed in any test case
- [ ] Log files contain useful diagnostic information (when logging enabled)
- [ ] Both ARM64 and x86-64 binaries tested on appropriate hardware
- [ ] Virtual treadmill testing (remote control method) validated
- [ ] Physical treadmill testing (if available) validated
- [ ] Cleanup script (testplan_cleanup.sh) successfully resets system to baseline

---

## Quick Reference

**Documentation:**
- [Installation Guide (README.md)](README.md)
- [Compilation Guide (COMPILE.md)](COMPILE.md)

**Scripts:**
- **Runtime Check:** `sudo ~/runtime_check.sh` - Validates system state (must run as sudo)
- **Cleanup:** `~/testplan_cleanup.sh` - Resets system to baseline

**Treadmill Options for Testing:**
- **Physical Treadmill:** Any Bluetooth-enabled treadmill from compatibility list
- **Virtual Treadmill:** Uses fake "KICKR RUN" treadmill with remote control device