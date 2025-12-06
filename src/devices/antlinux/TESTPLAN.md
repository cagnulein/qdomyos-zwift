# ANT+ Streamlined Test Plan

## Quick Test Strategy

Focus on the **4 critical scenarios** that cover all major functionality:

1. **Fresh Install** - Missing dependencies detection
2. **GUI Mode** - Visual feedback and device pairing
3. **Headless Mode** - ANT+ broadcasting with no GUI
4. **Automated Tests** - Rapid regression checking

---

## Automated Test Scripts

Run automated tests to quickly validate core functionality:

```bash
# Run validation from your installation directory
cd ~/qdomyos-zwift-x86-64-ant  # or wherever you extracted the binary
./setup.sh --quick

# Or with JSON output for CI/CD integration
./setup.sh --quick --json
```

**What gets tested automatically:**
- ✓ Python 3.11 library detection (system and pyenv)
- ✓ Virtual environment validation
- ✓ Python packages (openant, pyusb, pybind11)
- ✓ Qt5 library availability
- ✓ USB permissions (plugdev group, udev rules)
- ✓ ANT+ dongle detection
- ✓ Bluetooth service status
- ✓ Binary execution permissions
- ✓ Wrapper script validation

**If tests fail:**

For quick automatic fixes, run: `sudo ./setup.sh --fix`

For interactive guided setup with explanations, run: `sudo ./setup.sh --interactive`

See README.md section 1.5 for details.

**Expected output:**
```
=== QDomyos-Zwift ANT+ Automated Test Suite ===
[PASS] Python 3.11 library found
[PASS] Virtual environment exists
[PASS] Required Python packages installed
[PASS] Qt5 libraries available
[WARN] ANT+ USB dongle not detected
[PASS] USB permissions configured
[PASS] Bluetooth service running
[PASS] Binary is executable

Test Summary: 7 passed, 1 warning, 0 failed
System Status: READY (ANT+ will work when dongle plugged in)
```

---

## Manual Test Cases (4 Core Scenarios)

### T1: Fresh Install - Dependency Detection

**Time:** 5 minutes  
**Purpose:** Verify error messages are clear and helpful

**Steps:**
1. Fresh OS installation (or use cleanup script)
2. Download binary
3. Run: `./qdomyos-zwift-wrapper.sh -no-gui -ant-footpod`

**Expected:**
```
WARNING: ANT+ functionality may not work:
  Python 3.11 library (libpython3.11.so) not found!
    Install via: sudo apt install libpython3.11
  ANT+ virtual environment not found at: /home/user/ant_venv
    Create with: python3.11 -m venv ~/ant_venv
```

**Pass:** Clear, actionable error messages shown

---

### T2: GUI Mode - Visual Feedback

**Time:** 10 minutes  
**Purpose:** Verify GUI launches and shows device status

**Prerequisites:**
```bash
# Install dependencies
sudo apt-get install -y libqt5bluetooth5 libqt5charts5 libqt5multimedia5 \
  libqt5networkauth5 libqt5positioning5 libqt5sql5 libqt5texttospeech5 \
  libqt5websockets5 libqt5xml5 libusb-1.0-0 bluez python3.11

# Setup Python environment
python3.11 -m venv ~/ant_venv
~/ant_venv/bin/pip install openant pyusb pybind11
```

**Steps:**
1. Connect ANT+ dongle
2. Run: `sudo ./qdomyos-zwift` (GUI mode)
3. Enable "ANT+ Footpod" option in settings
4. Pair Garmin watch (Menu > Sensors > Add > Foot Pod)
5. Start treadmill
6. Observe GUI for 2 minutes

**Expected:**
- GUI launches without errors
- Settings show ANT+ option
- Device status shows treadmill connected
- No crashes or freezes

**Pass:** GUI functional, no crashes, device connects

---

### T3: Headless Mode - ANT+ Broadcasting

**Time:** 10 minutes  
**Purpose:** Verify ANT+ works without GUI

**Prerequisites:** Same as T2

**Steps:**
1. Configure for headless: Create config file (see README 3.4)
2. Run: `sudo ./qdomyos-zwift -no-gui -ant-footpod`
3. On Garmin watch: Add Foot Pod sensor
4. Start treadmill
5. Control speed and observe watch for 5 minutes

**Expected:**
```
QDomyos-Zwift starting in no-gui mode...
ANT+ Footpod broadcaster initialized
Device ID: 54321
Treadmill connected: <YOUR_TREADMILL>
Speed: 8.5 km/h → Pace: 7:04 min/km
Cadence: 166 SPM (running)
```

**Pass:** 
- Watch pairs successfully
- Pace updates smoothly
- Cadence shows 90-140 SPM (walking) or 160-200 SPM (running)
- No disconnections or errors

---

### T4: Systemd Service - Auto-start

**Time:** 10 minutes  
**Purpose:** Verify reliable automatic startup

**Prerequisites:** T3 successful

**Steps:**
1. Create systemd service (see README 3.3)
2. `sudo systemctl enable qz && sudo systemctl start qz`
3. Check: `sudo systemctl status qz`
4. Pair watch
5. Reboot system
6. After boot, verify: `sudo systemctl status qz`
7. Re-pair watch and test for 2 minutes

**Expected:**
- Service starts successfully
- Status shows "active (running)"
- Watch pairs after reboot
- Data flows correctly

**Pass:** Reliable auto-start, no issues after reboot

---

## Quick Acceptance Checklist

Before release, verify:

- [ ] **Automated tests** pass on both x86-64 and ARM64
- [ ] **T1 (Fresh Install)** shows helpful error messages
- [ ] **T2 (GUI)** launches and connects to devices
- [ ] **T3 (Headless)** broadcasts ANT+ for 5+ minutes
- [ ] **T4 (Systemd)** starts reliably after reboot
- [ ] **Wrapper script** detects missing dependencies
- [ ] **Documentation** matches actual behavior
- [ ] **No crashes** in any scenario

**Total manual testing time:** ~35 minutes per platform

---

## Automated Test Script Implementation

The setup script (`setup.sh`) is included in the binary distribution and provides three modes:

**1. Quick Validation Mode (`--quick`)**
- No sudo required
- Shows all issues at once
- Fast execution (~10 seconds)
- Exit codes: 0 (ready), 1 (failed), 2 (warnings)
- JSON output available for CI/CD

**2. Automatic Fix Mode (`--fix`)**
- Requires sudo
- Automatically installs fixable components
- Installs: Qt5, libusb, configures USB permissions, udev rules, Bluetooth
- Cannot install Python 3.11 (requires pyenv if unavailable via apt)

**3. Interactive Mode (`--interactive`)**
- Requires sudo
- Step-by-step guided setup
- Explains each requirement before installation
- Prompts for confirmation at each step
- Best for first-time users or learning

The script is included in your binary distribution ZIP file.

---

## Related Documentation

- **Installation Guide:** [README.md](README.md) - User installation instructions
- **Compilation Guide:** [COMPILE.md](COMPILE.md) - Building from source with Docker
