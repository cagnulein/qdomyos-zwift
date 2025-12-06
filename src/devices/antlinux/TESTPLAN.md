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
- ✓ Qt5 library availability (Bluetooth, Charts, Multimedia)
- ✓ USB permissions (plugdev group membership)
- ✓ udev rules configuration
- ✓ ANT+ dongle detection
- ✓ Bluetooth service status
- ✓ libusb-1.0 library availability

**If tests fail:**

For quick automatic fixes, run: `sudo ./setup.sh --fix`

For interactive guided setup with explanations, run: `sudo ./setup.sh --interactive`

**To validate and test ANT+ hardware in one command:**

```bash
sudo ./setup.sh --test
```

This runs validation checks and automatically launches the ANT+ hardware test if all checks pass.

See README.md Step 2 for details.

**Expected output (all tests passing):**
```
=== QDomyos-Zwift ANT+ Quick Validation ===

[PASS] Python 3.11 library found
[PASS] Virtual environment exists at ~/ant_venv
[PASS] Python package 'openant' installed
[PASS] Python package 'pyusb' installed
[PASS] Python package 'pybind11' installed
[PASS] Qt5 Bluetooth library available
[PASS] Qt5 Charts library available
[PASS] Qt5 Multimedia library available
[PASS] User 'username' is in 'plugdev' group
[PASS] ANT+ udev rules configured
[WARN] ANT+ USB dongle not detected (plug in Garmin/Suunto ANT+ stick)
[PASS] Bluetooth service running
[PASS] libusb-1.0 library available

Test Summary: 12 passed, 1 warning, 0 failed

System Status: READY
All tests passed - system is ready for ANT+ operation
```

**Expected output (with issues):**
```
=== QDomyos-Zwift ANT+ Quick Validation ===

[FAIL] Python 3.11 library not found (run: sudo ./setup.sh --interactive)
[FAIL] Virtual environment not found (run: sudo ./setup.sh --interactive)
[FAIL] Qt5 Bluetooth missing (run: sudo ./setup.sh --fix)
[FAIL] Qt5 Charts missing (run: sudo ./setup.sh --fix)
[FAIL] Qt5 Multimedia missing (run: sudo ./setup.sh --fix)
[FAIL] User not in 'plugdev' group (run: sudo ./setup.sh --fix)
[FAIL] ANT+ udev rules not found (run: sudo ./setup.sh --fix)
[WARN] ANT+ USB dongle not detected (plug in Garmin/Suunto ANT+ stick)
[FAIL] Bluetooth service not running (run: sudo ./setup.sh --fix)
[FAIL] libusb-1.0 missing (run: sudo ./setup.sh --fix)

Test Summary: 0 passed, 1 warning, 9 failed

System Status: FAILED
To fix issues, run: sudo ./setup.sh --interactive (guided)
            or run: sudo ./setup.sh --fix (automatic)
```

---

## Manual Test Cases (4 Core Scenarios)

### T1: Fresh Install - Dependency Detection

**Time:** 5 minutes  
**Purpose:** Verify error messages are clear and helpful

**Steps:**
1. Fresh OS installation
2. Download and extract binary (README Step 1)
3. Run: `./setup.sh --quick`

**Expected:**
- Multiple `[FAIL]` messages with clear instructions
- Each failure shows how to fix it
- Suggested commands: `sudo ./setup.sh --interactive` or `sudo ./setup.sh --fix`

**Pass:** Clear, actionable error messages shown

---

### T2: GUI Mode - Visual Feedback

**Time:** 10 minutes  
**Purpose:** Verify GUI launches and shows device status

**Prerequisites:**
```bash
# Install dependencies using automated tool
cd ~/qdomyos-zwift-x86-64-ant
sudo ./setup.sh --fix

# Verify installation and test hardware
sudo ./setup.sh --test
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

**Prerequisites:** T2 successful

**Steps:**
1. Configure for headless: Create config file (see README 4.4)
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
1. Create systemd service (see README 4.3)
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