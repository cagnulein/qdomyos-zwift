# ANT+ Streamlined Test Plan

## Quick Test Strategy

Focus on the **3 critical scenarios** that cover all major functionality:

1. **Fresh Install** - Missing dependencies detection
2. **GUI Mode** - Visual feedback and device pairing
3. **Headless Mode** - ANT+ broadcasting with no GUI

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

Run guided setup: `sudo ./setup.sh --guided`

See README.md Step 2 for details.

**Expected output (all tests passing):**
```
═══════════════════════════════════════════════════════════
QDomyos-Zwift ANT+ Quick Validation
═══════════════════════════════════════════════════════════

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

───────────────────────────────────────────────────────────
Test Summary: 12 passed, 1 warning, 0 failed
───────────────────────────────────────────────────────────

System Status: READY
All tests passed - system is ready for ANT+ operation
```

**Expected output (with issues):**
```
═══════════════════════════════════════════════════════════
QDomyos-Zwift ANT+ Quick Validation
═══════════════════════════════════════════════════════════

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

───────────────────────────────────────────────────────────
Test Summary: 0 passed, 1 warning, 9 failed
───────────────────────────────────────────────────────────

System Status: FAILED
To fix issues, run:
  sudo ./setup.sh --guided
```

---

## Setup Tool Modes

The `setup.sh` script provides multiple modes for different use cases:

### **Testing and Development Modes:**

```bash
# Reset to clean state (useful before each test run)
sudo ./setup.sh --reset

# Test ANT+ broadcasting independently
sudo ./setup.sh --test
```

### **Setup and Validation Modes:**

```bash
# Quick validation (no changes made)
./setup.sh --quick

# Guided setup (recommended - explains everything)
sudo ./setup.sh --guided
```

### **What `--reset` does:**
- ✓ Removes user from plugdev group
- ✓ Removes ANT+ udev rules
- ✓ Optionally removes Python venv and packages (with confirmation)
- ✓ Returns system to pre-setup state
- ✗ Does NOT uninstall packages (Qt5, libusb, bluez remain)

**Use case:** Complete clean slate before running test scenarios

### **What `--test` does:**
- ✓ Runs standalone ANT+ broadcasting test
- ✓ Simulates treadmill data (pace, cadence, distance)
- ✓ Verifies dongle and watch pairing
- ✓ Isolates ANT+ issues from QDomyos-Zwift app

**Use case:** Verify ANT+ works before testing full application

---

## Manual Test Cases (4 Core Scenarios)

### T1: Fresh Install - Dependency Detection

**Time:** 5 minutes  
**Purpose:** Verify error messages are clear and helpful

**Setup:**
```bash
# Start with clean state
sudo ./setup.sh --reset
```

**Steps:**
1. Download and extract binary (README Step 1)
2. Run: `./setup.sh --quick`

**Expected:**
- Multiple `[FAIL]` messages with clear instructions
- Each failure shows how to fix it
- Suggested command: `sudo ./setup.sh --guided`

**Pass:** Clear, actionable error messages shown

---

### T2: GUI Mode - Visual Feedback

**Time:** 10 minutes  
**Purpose:** Verify GUI launches and shows device status

**Prerequisites:**
```bash
# Install dependencies using guided setup
cd ~/qdomyos-zwift-x86-64-ant
sudo ./setup.sh --guided

# Verify installation
./setup.sh --quick
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
1. Test ANT+ independently: `sudo ./setup.sh --test`
2. Verify watch pairs and shows data
3. Stop test (Ctrl+C)
4. Configure for headless: Create config file (see README 4.4)
5. Run: `sudo ./qdomyos-zwift -no-gui -ant-footpod`
6. On Garmin watch: Add Foot Pod sensor
7. Start treadmill
8. Control speed and observe watch for 5 minutes

**Expected from `--test`:**
```
═══════════════════════════════════════════════════════════
QDomyos-Zwift ANT+ Broadcasting Test
═══════════════════════════════════════════════════════════

This test simulates treadmill data broadcasting via ANT+.
Your Garmin watch should pair as a Foot Pod within 5-10 seconds.

Expected readings:
  - Pace: ~7:00 min/km (varying)
  - Cadence: ~166 SPM
  - Distance: accumulating
```

**Expected from main app:**
```
QDomyos-Zwift starting in no-gui mode...
ANT+ Footpod broadcaster initialized
Device ID: 54321
Treadmill connected: <YOUR_TREADMILL>
Speed: 8.5 km/h → Pace: 7:04 min/km
Cadence: 166 SPM (running)
```

**Pass:** 
- Test mode works and watch pairs
- Watch pairs successfully in headless mode
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
- [ ] **`--reset` mode** successfully cleans configurations
- [ ] **`--test` mode** works independently of main app
- [ ] **Documentation** matches actual behavior
- [ ] **No crashes** in any scenario

**Total manual testing time:** ~35 minutes per platform

---

## Test Workflow Best Practices

### **Between Test Runs:**
```bash
# Clean state for fresh test
sudo ./setup.sh --reset
./setup.sh --quick  # Verify clean
```

### **Before Testing Main App:**
```bash
# Verify ANT+ works independently
sudo ./setup.sh --test
# If test passes, ANT+ stack is working
# If main app fails but test passes → app issue, not ANT+
```

### **CI/CD Integration:**
```bash
# Automated validation
./setup.sh --quick --json > validation.json
# Parse JSON for pass/fail status
```

---

## Troubleshooting Test Failures

### **Test Mode Issues:**

| Issue | Solution |
|-------|----------|
| `test_ant.py` not found | Ensure script is in same directory as setup.sh |
| Virtual environment missing | Run `sudo ./setup.sh --fix` first |
| Watch won't pair | Check dongle is plugged in, verify with `lsusb` |
| Test runs but no data | USB permissions issue - run `./setup.sh --quick` |

### **Reset Mode Issues:**

| Issue | Solution |
|-------|----------|
| Still shows as in plugdev group | Logout and login required |
| Quick check still shows configured | Some manual configs remain - expected behavior |
| System packages still installed | Reset only removes configs, not packages (by design) |

---

## Related Documentation

- **Installation Guide:** [README.md](README.md) - User installation instructions
- **Compilation Guide:** [COMPILE.md](COMPILE.md) - Building from source with Docker