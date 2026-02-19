# ANT+ Feature Test Plan

This document provides comprehensive testing procedures for the ANT+ footpod broadcasting feature. Use this as a reference for:
- Validating changes and pull requests
- Verifying fresh installations
- Release testing and QA
- Troubleshooting issues

---

## Quick Reference

**Test Flow:** System Validation → ANT+ Hardware Test → Application Test

**Estimated Time:** 15-30 minutes for full test suite

**Requirements:**
- Completed installation (see [README.md](README.md))
- ANT+ USB dongle
- Garmin watch (or compatible ANT+ device)
- Treadmill (optional for hardware test, required for full test)

---

## Test Overview

### Test Levels

**Level 1: System Validation**
- Purpose: Verify all dependencies installed correctly
- Duration: 2-5 minutes
- Result: PASS/FAIL for each component
- Run when: After installation, before any other tests

**Level 2: ANT+ Hardware Test**
- Purpose: Verify ANT+ dongle → watch communication
- Duration: 2-3 minutes
- Result: Watch pairs and receives simulated data
- Run when: After system validation passes

**Level 3: Application Test**
- Purpose: Verify full treadmill → watch workflow
- Duration: 5-10 minutes  
- Result: Watch receives actual treadmill data
- Run when: After hardware test passes

---

## Pre-Test Checklist

Before starting tests, verify:

- [ ] Installation complete (package extracted)
- [ ] Setup dashboard run at least once
- [ ] ANT+ dongle plugged into USB port
- [ ] Watch powered on and charged
- [ ] Watch within 3 meters of dongle
- [ ] Running commands with `sudo`

---

## Test 1: System Validation

### Purpose
Verify all dependencies and environment components are correctly installed and configured.

### Procedure

**Run the dashboard:**
```bash
cd ~/qdomyos-zwift-*-ant
sudo ./setup-dashboard.sh
```

**Check status display** for all components:

### Expected Results

All items should show **✓ PASS**:

| Component | Status | What It Checks |
|-----------|--------|----------------|
| Python 3.11 Library | ✓ | Python 3.11 installed and shared library available |
| Virtual Environment | ✓ | ant_venv exists |
| Python Packages | ✓ | openant, usb, pybind11, bleak installed |
| Qt5 Libraries | ✓ | 17 Qt5 libraries present |
| QML Modules | ✓ | 9 QML modules available |
| USB Permissions | ✓ | plugdev group membership and udev rules |
| ANT+ Dongle | ✓ | Dongle detected via lsusb |
| Bluetooth Service | ✓ | Bluetooth daemon running |

**Overall system status:** READY

### Failure Handling

**If any component shows ! FAIL:**
1. Dashboard provides specific error message
2. Use dashboard interactive menus to fix
3. Re-run test after fixes
4. All must pass before continuing

**Common failures:**
- Python 3.11: Use dashboard to install
- Packages: Use dashboard to reinstall venv
- Permissions: Reboot after dashboard fixes permissions
- Dongle: Unplug/replug, try different port

---

## Test 2: ANT+ Hardware Test

### Purpose
Verify ANT+ dongle and watch communication works independently of treadmill. This isolates ANT+ hardware issues from treadmill connection issues.

### Prerequisites
- System validation passed
- ANT+ dongle connected
- Watch in "Add Sensor" mode

### Procedure - Method 1 (Recommended)

**Using the dashboard:**
```bash
sudo ./setup-dashboard.sh
```

1. Select "ANT+ Test" from main menu
2. Watch displays in dashboard interface
3. Put watch in "Add Sensor → Foot Pod" mode
4. Observe test progress

**Test stages (automatic):**
- Stage 1: Warmup (4 km/h walking - 30 seconds)
- Stage 2: Easy jog (8 km/h running - 30 seconds)
- Stage 3: Moderate run (10 km/h - 30 seconds)
- Stage 4: Fast run (12 km/h - 30 seconds)
- Stage 5: Sprint (15 km/h - 30 seconds)

### Procedure - Method 2 (Advanced)

**Direct script execution:**
```bash
cd ~/qdomyos-zwift-*-ant
sudo ~/ant_venv/bin/python3 ./test_ant.py
```

Same test stages as Method 1.

### Expected Results

**Watch behavior:**
- [ ] **Pairing (0-10 seconds):** Watch finds and pairs with "Foot Pod"
- [ ] **Stage 1 (warmup):** Pace shows ~15:00 min/km, cadence ~110 SPM
- [ ] **Stage 2 (jog):** Pace shows ~7:30 min/km, cadence ~165 SPM
- [ ] **Stage 3 (moderate):** Pace shows ~6:00 min/km, cadence ~175 SPM
- [ ] **Stage 4 (fast):** Pace shows ~5:00 min/km, cadence ~185 SPM
- [ ] **Stage 5 (sprint):** Pace shows ~4:00 min/km, cadence ~195 SPM
- [ ] **Distance:** Accumulates throughout test (~400-500 meters total)
- [ ] **Stability:** No disconnections or pauses

**Terminal/Dashboard output:**
- [ ] "ANT+ channel opened successfully"
- [ ] "Broadcasting footpod data..."
- [ ] Stage transitions shown clearly
- [ ] No USB errors
- [ ] No Python errors

### Pass Criteria

**PASS if:**
- Watch pairs within 10 seconds ✓
- Pace updates in real-time for all stages ✓
- Cadence changes appropriately (walk vs run) ✓
- Distance accumulates smoothly ✓
- No errors or disconnections ✓

**FAIL if:**
- Watch doesn't pair (see troubleshooting)
- Pace shows `--:--` (communication failure)
- USB errors appear
- Test crashes or hangs

### Troubleshooting

| Problem | Solution |
|---------|----------|
| Watch won't pair | Unplug/replug dongle, restart test, check watch is in pairing mode |
| USB error: resource busy | Stop any running qdomyos-zwift process, replug dongle |
| Permission denied | Verify running with sudo, check plugdev group membership |
| Python import error | Re-run dashboard, verify venv installation |

---

## Test 3: Full Application Test

### Purpose
Verify complete workflow: treadmill → application → ANT+ → watch

### Prerequisites
- System validation passed
- ANT+ hardware test passed
- Treadmill powered on
- Treadmill configured in dashboard or config file

### Test 3A: Desktop Mode (GUI)

**For systems with display:**

**Procedure:**
```bash
cd ~/qdomyos-zwift-*-ant
sudo ./qdomyos-zwift -ant-footpod
```

**Expected results:**
1. [ ] GUI opens successfully
2. [ ] Treadmill connects via Bluetooth automatically
3. [ ] Watch pairs (if not already paired)
4. [ ] GUI shows treadmill connection status
5. [ ] Start walking/running on treadmill
6. [ ] GUI displays speed, pace, cadence
7. [ ] Watch displays matching data
8. [ ] Distance accumulates on both

### Test 3B: Server Mode (No GUI)

**For headless systems:**

**Procedure:**
```bash
cd ~/qdomyos-zwift-*-ant
sudo ./qdomyos-zwift -no-gui -ant-footpod
```

**Expected results:**
1. [ ] Application starts successfully
2. [ ] Terminal shows "Waiting for Bluetooth connection..."
3. [ ] Treadmill connects (shows connection message)
4. [ ] Watch pairs (if not already paired)
5. [ ] Terminal shows speed/pace updates
6. [ ] Start walking/running on treadmill
7. [ ] Watch displays data matching treadmill
8. [ ] No crashes or disconnections

### Test Scenarios

**Scenario 1: Walking Speed (4-6 km/h)**
- [ ] Watch shows walking pace
- [ ] Cadence in walking range (90-140 SPM)
- [ ] Data updates every 1-2 seconds

**Scenario 2: Running Speed (8-12 km/h)**
- [ ] Watch shows running pace
- [ ] Cadence in running range (160-200 SPM)
- [ ] Pace matches treadmill speed

**Scenario 3: Speed Changes**
- [ ] Increase treadmill speed: Watch pace increases
- [ ] Decrease treadmill speed: Watch pace decreases
- [ ] Walk → Run transition (~7 km/h): Cadence switches from walking to running formula

**Scenario 4: Stop and Start**
- [ ] Stop treadmill: Pace shows stopped (0 or --:--)
- [ ] Restart treadmill: Data resumes immediately
- [ ] Distance continues accumulating

**Scenario 5: Extended Run (10+ minutes)**
- [ ] No disconnections
- [ ] Data remains accurate
- [ ] No memory leaks or performance degradation
- [ ] Distance accumulates correctly

### Expected Terminal Output (Server Mode)

**Startup:**
```
QDomyos-Zwift starting...
Platform: ARM64 / x86-64
Loading configuration...
Waiting for Bluetooth connection...
```

**Treadmill connected:**
```
Bluetooth: Connected to [Treadmill Name]
Device: Proform 705 CST (or your model)
ANT+ channel opened successfully
Broadcasting footpod data...
```

**During run:**
```
Speed: 8.0 km/h | Pace: 7:30 min/km | Cadence: 165 SPM | Distance: 0.52 km
Speed: 8.0 km/h | Pace: 7:30 min/km | Cadence: 165 SPM | Distance: 0.55 km
...
```

### Pass Criteria

**PASS if:**
- Application starts without errors ✓
- Treadmill connects via Bluetooth ✓
- Watch pairs and receives data ✓
- Pace matches treadmill speed (±5%) ✓
- Cadence appropriate for speed ✓
- Distance accumulates correctly ✓
- No disconnections during 10+ minute test ✓

**FAIL if:**
- Application crashes on startup
- Treadmill won't connect
- Watch shows no data or `--:--`
- Pace significantly wrong (>10% error)
- Frequent disconnections
- Data stops updating

### Troubleshooting

| Problem | Solution |
|---------|----------|
| Treadmill won't connect | Dashboard → Bluetooth Scan → Re-pair treadmill |
| Watch paired but no data | Verify treadmill model configured correctly |
| Pace shows `--:--` | Check treadmill is sending speed data, verify config |
| App crashes on start | Check logs: `journalctl -u qz -n 50`, verify dependencies |
| Data mismatch | Verify correct treadmill model selected, check calibration |

---

## Test 4: Service Mode (Optional)

### Purpose
Verify automatic startup service works correctly.

### Prerequisites
- Application tests passed
- Service configured via dashboard

### Procedure

**Check service status:**
```bash
sudo systemctl status qz
```

**Expected:** Active (running)

**Restart service:**
```bash
sudo systemctl restart qz
```

**Expected:** Restarts cleanly within 5 seconds

**Stop service:**
```bash
sudo systemctl stop qz
```

**Expected:** Stops cleanly within 5 seconds

**View logs:**
```bash
journalctl -u qz -n 50
```

**Expected:** No errors, shows normal startup sequence

### Pass Criteria

**PASS if:**
- Service starts automatically on boot ✓
- Service can be stopped/started/restarted ✓
- Logs show no errors ✓
- Application functions normally in service mode ✓

---

## Validation Checklist

### Pre-Release Checklist

Before marking a release as validated:

- [ ] System validation passes on all supported platforms
- [ ] ANT+ hardware test passes with multiple watch models
- [ ] Application test passes in both GUI and server modes
- [ ] Extended run test (30+ minutes) shows stability
- [ ] Service mode works correctly
- [ ] Documentation updated for any changes
- [ ] No critical bugs in issue tracker
- [ ] Tested on fresh installation (not just upgrade)

### Pull Request Validation

Before approving a pull request:

- [ ] System validation passes
- [ ] ANT+ hardware test passes
- [ ] Application test passes for affected functionality
- [ ] No new errors or warnings
- [ ] Code changes reviewed
- [ ] Tests run on both x86-64 and ARM64 (if applicable)

---

## Related Documentation

- **[README.md](README.md)** - Installation and setup procedures
- **[COMPILE.md](COMPILE.md)** - Building from source
- **Troubleshooting** - See README.md#troubleshooting for detailed solutions