# ANT+ Pre-compiled Binary Test Plan

## Overview

This document provides comprehensive testing procedures for the QDomyos-Zwift ANT+ Virtual Footpod pre-compiled binaries. It's designed for developers and "KICKR RUN".

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
- **Treadmill:** One of the following options:
  - Bluetooth-enabled treadmill compatible with QDomyos-Zwift (see [compatibility list](https://github.com/cagnulein/qdomyos-zwift#compatible-devices))
  - **Virtual treadmill setup** (see below)

---

## Virtual Treadmill Setup for Testing

If you don't have easy access to a compatible physical treadmill, use QDomyos-Zwift's virtual fake treadmill feature.

### Create Configuration File

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

### Method 1: GUI Control (Desktop)

For devices with desktop environment:

```bash
sudo ./qdomyos-zwift -ant-footpod
```

Use GUI speed controls to adjust speed during testing.

### Method 2: Headless Control (Second qz as Remote)

For headless devices (Pi Zero 2 W, servers):

```bash
sudo ./qdomyos-zwift -no-gui -ant-footpod
```

**Second QZ GUI settings:**
- This can be QZ running on Mobile or Desktop
- Use wizard to connect to the Fake Treadmill "KICKR RUN"
- Set Horizon Treadmill Force FTMS to Enabled

**Connect:** Open QZ app → connect to "KICKR RUN" → use GUI speed controls

### Pair Garmin Watch

1. Watch: Menu > Sensors & Accessories > Add New > Foot Pod
2. Start "Treadmill" or "Run Indoor" activity
3. Adjust speed (GUI or Android)
4. Verify pace/cadence updates on watch

**Success indicators:**
- Console shows: `[ANT+] Relaying FTMS speed request (X.Xkm/h) to faketreadmill.`
- Watch displays stable pace and cadence (no flickering or `--:--`)

---

## Pre-Test Setup: Establishing Baseline Environment

**Important Notice for Testers and Developers:**

This section is is for developers, QA testers, and contributors who need to repeatedly test the installation process from a clean baseline state.

**Before proceeding with cleanup:**

1. **Backup your configuration files** if they contain important settings:
   ```bash
   # Backup root's QZ config (if it exists)
   sudo cp -r "/root/.config/Roberto Viola" "/root/.config/Roberto Viola.backup" 2>/dev/null
   
   # Backup user's QZ config (if it exists)
   cp -r "$HOME/.config/Roberto Viola" "$HOME/.config/Roberto Viola.backup" 2>/dev/null
   ```

2. **Take note of your treadmill model settings** - you may need to reconfigure these after testing

3. **Consider creating a system snapshot** if your platform supports it (e.g., SD card backup for Raspberry Pi)

### Remove Existing Installation

```bash
# Stop and disable any running service
sudo systemctl stop qz 2>/dev/null
sudo systemctl disable qz 2>/dev/null
sudo rm -f /etc/systemd/system/qz.service
sudo systemctl daemon-reload

# Remove the binary
rm -f ~/qdomyos-zwift

# Remove Python virtual environment
rm -rf ~/ant_venv

# Remove runtime check script
rm -f ~/runtime_check.sh
```

### Remove System Dependencies (Optional - for complete baseline)

**Warning:** Only remove these if they're not needed by other applications on your system.

```bash
# Remove Qt5 libraries
sudo apt-get remove -y \
	libqt5bluetooth5 \
	libqt5charts5 \
	libqt5multimedia5 \
	libqt5networkauth5 \
	libqt5positioning5 \
	libqt5sql5 \
	libqt5texttospeech5 \
	libqt5websockets5 \
	libqt5xml5

# Remove Python 3.11 (only if not needed by other software)
sudo apt-get remove -y python3.11 python3.11-venv

# Remove USB utilities
sudo apt-get remove -y libusb-1.0-0 usbutils

# Clean up
sudo apt-get autoremove -y
```

### Remove USB Permissions Configuration

```bash
# Remove udev rule
sudo rm -f /etc/udev/rules.d/99-ant-usb.rules
sudo udevadm control --reload-rules && sudo udevadm trigger

# Remove user from plugdev group (replace $USER with actual username if needed)
sudo gpasswd -d $USER plugdev
```

### Remove Root Configuration File

```bash
# Backup first (if you haven't already)
sudo cp -r "/root/.config/Roberto Viola" "/root/.config/Roberto Viola.backup" 2>/dev/null

# Remove QZ configuration
sudo rm -rf "/root/.config/Roberto Viola"
```

**Note:** To restore from backup after testing:
```bash
sudo cp -r "/root/.config/Roberto Viola.backup" "/root/.config/Roberto Viola" 2>/dev/null
```

### Verification

After cleanup, verify baseline state:

```bash
# Check binary is gone
ls ~/qdomyos-zwift 2>/dev/null && echo "Binary still present" || echo "Binary removed ✓"

# Check venv is gone
ls ~/ant_venv 2>/dev/null && echo "Venv still present" || echo "Venv removed ✓"

# Check Python 3.11
command -v python3.11 >/dev/null 2>&1 && echo "Python 3.11 present" || echo "Python 3.11 removed ✓"

# Check udev rule
[ -f /etc/udev/rules.d/99-ant-usb.rules ] && echo "Udev rule still present" || echo "Udev rule removed ✓"

# Check group membership
groups | grep -q plugdev && echo "Still in plugdev group" || echo "Removed from plugdev ✓"
```

You're now ready to begin testing from a clean baseline.

---

## Test Preparation

Before starting the test cases, download the runtime check script once:

```bash
curl -o ~/runtime_check.sh https://raw.githubusercontent.com/cagnulein/qdomyos-zwift/master/src/devices/antlinux/runtime_check.sh
chmod +x ~/runtime_check.sh
```

This script will be used throughout testing to validate the system state.

---

## Test Cases

### TC1: Fresh System - Missing All Dependencies

**Purpose:** Verify the application exits gracefully when Qt5 libraries are missing and provides clear error messages.

**Initial Setup:**
- Fresh OS installation
- No Python 3.11, no Qt5 libraries, no virtual environment

**Test Steps:**
1. Run runtime check: `sudo ~/runtime_check.sh`
   - **Expected:** Fails on Python 3.11, venv, Qt5 libraries, USB permissions, ANT+ dongle, Bluetooth
   - **Verify:** Provides installation commands for each failed check
   - **Verify:** Links to README sections are present and accurate
2. Download appropriate binary from GitHub Actions
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
- Application error message names the specific missing library
- No segmentation faults or unhandled exceptions

**Cleanup for Next Test:**
- None needed (nothing was installed)

---

### TC2: Python 3.11 Installed - Missing Qt5 Libraries

**Purpose:** Verify error handling when Python environment exists but Qt5 libraries are missing.

**Setup Steps:**
1. Starting from TC1 state
2. Install only Python 3.11: `sudo apt-get install python3.11 python3.11-venv python3-pip`
3. Create venv: `python3.11 -m venv ~/ant_venv`
4. Install Python packages: `~/ant_venv/bin/pip install --upgrade pip && ~/ant_venv/bin/pip install openant pyusb pybind11`

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
- Runtime check passes Python/venv, fails Qt5
- Clear indication of which Qt5 libraries are missing
- No crash or hanging

**Cleanup for Next Test:**
- Keep Python 3.11 and venv installed
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
   libqt5websockets5 libqt5xml5 libusb-1.0-0 usbutils
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
   - **Expected:** Fails/warns on USB permissions (no udev rule, not in plugdev), ANT+ dongle permissions, Bluetooth
   - **Verify:** Provides udev rule creation commands
2. Run: `sudo ./qdomyos-zwift -no-gui -ant-footpod`

**Expected Results:**
- Runtime check identifies USB permission issues
- Application starts successfully
- Since running with `sudo`, USB access may work despite missing udev rules
- Application remains stable

**Pass Criteria:**
- Runtime check identifies USB permission configuration gaps
- Application doesn't crash
- Any USB-related errors are logged clearly

**Cleanup for Next Test:**
- Configure USB permissions as per README Step 1.3
- Add user to plugdev group: `sudo usermod -aG plugdev $USER`
- Create udev rules and reboot
- Proceed to TC6

---

### TC6: Complete Setup - Wrong Python Version

**Purpose:** Verify error handling when wrong Python version is used for the venv.

**Setup Steps:**
1. Starting from fresh system or TC5 state
2. Install Python 3.9 or 3.10 (if available): `sudo apt-get install python3.9 python3.9-venv`
3. Install all system dependencies (Qt5 libraries)
4. Create venv with wrong Python version: `python3.9 -m venv ~/ant_venv`
5. Install packages: `~/ant_venv/bin/pip install openant pyusb pybind11`

**Test Steps:**
1. Run runtime check: `sudo ~/runtime_check.sh`
   - **Expected:** Fails on Python 3.11 (wrong version detected)
   - **Verify:** Suggests installing Python 3.11
2. Run: `sudo ./qdomyos-zwift -no-gui -ant-footpod`

**Expected Results:**
- Runtime check identifies Python version issue
- Application fails to start
- Error message: "error while loading shared libraries: libpython3.11.so.1.0"
- Clear indication that Python 3.11 is required

**Pass Criteria:**
- Runtime check identifies Python version mismatch
- Error clearly mentions Python 3.11
- No crash or hanging

**Cleanup for Next Test:**
- Remove wrong venv: `rm -rf ~/ant_venv`
- Install Python 3.11: `sudo apt-get install python3.11 python3.11-venv`
- Recreate venv: `python3.11 -m venv ~/ant_venv`
- Install packages: `~/ant_venv/bin/pip install openant pyusb pybind11`
- Proceed to TC7

---

### TC7: Complete Correct Setup - Success Case

**Purpose:** Verify full functionality when all prerequisites are correctly configured.

**Setup Steps:**
1. Complete all README steps exactly (Steps 1.1, 1.2, 1.3 including reboot)
2. Connect ANT+ dongle
3. Have Garmin watch ready for pairing
4. Set up virtual treadmill configuration (see Virtual Treadmill Setup section)

**Test Steps:**
1. Run runtime check: `sudo ~/runtime_check.sh`
   - **Expected:** All 6 checks pass (✓ green)
   - **Expected:** Final message: "All checks passed! Your runtime environment is ready for ANT+."
   - **Verify:** Links to next steps in README
2. Run: `sudo ./qdomyos-zwift -no-gui -ant-footpod` (or without `-no-gui` if using GUI control)
3. Wait for initialization messages
4. On Garmin watch: Menu > Sensors & Accessories > Add New > Foot Pod
5. Wait for pairing
6. Control speed via GUI or Android app
7. Observe watch display for 5+ minutes
8. Vary speed to test cadence switching (below/above 7.0 km/h)

**Expected Results:**
- Runtime check shows all green checkmarks
- Application starts successfully without errors
- Console/log shows: "ANT+ Footpod broadcaster initialized" or similar
- Watch detects and pairs within a few seconds
- Watch displays pace data that matches commanded speed
- Watch displays cadence data (90-140 SPM walking, 160-200 SPM running)
- Cadence switches appropriately at ~7.0 km/h threshold
- No errors or warnings in logs
- Application runs stably for 5+ minutes

**Pass Criteria:**
- Runtime check passes all 6 checks
- Clean startup with no errors
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
6. Verify watch can pair
7. Reboot system: `sudo reboot`
8. After boot, check status: `sudo systemctl status qz`
9. Verify watch can still pair and receive data
10. Test graceful shutdown: `sudo systemctl stop qz`

**Expected Results:**
- Runtime check confirms system is ready
- Service starts successfully
- Status shows "active (running)"
- Watch can pair and receive data
- Service starts automatically after reboot
- Service stops gracefully within 5 seconds

**Pass Criteria:**
- Service runs without errors
- Automatic startup works reliably
- Watch functionality identical to manual run
- Clean shutdown without hanging

**Cleanup for Next Test:**
- Leave service configured
- Proceed to TC9

---

### TC9: Wrong Architecture Binary

**Purpose:** Verify clear error message when user downloads wrong architecture binary.

**Setup Steps:**
1. On Raspberry Pi: download x86-64 binary
2. OR on Desktop Linux: download ARM64 binary
3. Transfer wrong binary to `~/qdomyos-zwift`
4. Make executable: `chmod +x ~/qdomyos-zwift`

**Test Steps:**
1. Run: `sudo ./qdomyos-zwift -no-gui -ant-footpod`

**Expected Results:**
- Immediate error message: "cannot execute binary file: Exec format error"
- Clear indication of architecture mismatch
- No system instability

**Pass Criteria:**
- Immediate, clear error about binary format
- System remains stable

**Cleanup:**
- Remove wrong binary
- Download correct architecture binary

---

## Acceptance Checklist

Before releasing a new binary build, verify:

- [ ] TC1-TC6 (error cases) produce clear, helpful error messages
- [ ] Runtime check integrated in TC1-TC8 provides accurate diagnostics
- [ ] Runtime check suggestions are actionable with working commands
- [ ] Runtime check completes in under 10 seconds in all states
- [ ] Runtime check README links are accurate and working
- [ ] All 6 runtime checks must pass for success
- [ ] TC7 (success case) works on both Raspberry Pi and Desktop Linux
- [ ] TC7 runs stably for 5+ minutes with accurate data
- [ ] TC7 runtime check shows all 6 green passes
- [ ] TC8 (systemd) starts reliably on boot on both platforms
- [ ] TC8 stops gracefully without hanging
- [ ] TC9 (wrong architecture) fails with clear architecture error
- [ ] All error messages reviewed for clarity and helpfulness
- [ ] Documentation matches actual application behavior
- [ ] No crashes, hangs, or segfaults observed in any test case
- [ ] Log files contain useful diagnostic information (when logging enabled)
- [ ] Both ARM64 and x86-64 binaries tested on appropriate hardware
- [ ] Virtual treadmill testing (GUI and headless control) validated

---

## Quick Reference

**Documentation:**
- [Installation Guide (README.md)](README.md)
- [Compilation Guide (COMPILE.md)](COMPILE.md)