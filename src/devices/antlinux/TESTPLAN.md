# ANT+ Feature Test Plan

Validation guide to verify ANT+ functionality before submitting for review.

---

## Prerequisites

**Hardware Requirements:**
- ANT+ USB dongle (Garmin USB2/USB-m/USB3/mini, or Suunto)
- Garmin watch or compatible ANT+ device
- Raspberry Pi (Zero 2 W, 3, 4, or 5) OR x86-64 PC/Laptop

**Software Requirements:**
- Linux distribution: Debian/Ubuntu-based (Bookworm or newer recommended)
- Pre-compiled binary package from GitHub Releases

**Time Required:** 15-20 minutes for complete testing

---

## Test Flow

### 1. Quick Validation (2 minutes)

Verify all dependencies are installed and configured.

**Raspberry Pi:**
```bash
cd ~/qdomyos-zwift-arm64-ant
./setup.sh --quick
```

**Desktop Linux:**
```bash
cd ~/qdomyos-zwift-x86-64-ant
./setup.sh --quick
```

**Expected Output:**
```
[PASS] Python 3.11 library found
[PASS] Virtual environment exists
[PASS] Python packages installed (openant, pyusb, pybind11)
[PASS] Qt5 runtime libraries available (12 libraries checked)
[PASS] QML modules available (9 modules checked)
[PASS] Bluetooth service available
[PASS] User in 'plugdev' group
[PASS] ANT+ udev rules configured
[PASS] ANT+ USB dongle detected
[PASS] Bluetooth service running

Test Summary: 12 passed, 0 warnings, 0 failed
System Status: READY
```

**If tests fail:** Run `sudo ./setup.sh --guided` to install missing dependencies.

---

### 2. Standalone ANT+ Test (3 minutes)

Test ANT+ broadcasting independently from the main application.

```bash
sudo ./setup.sh --test
```

**What to verify:**
1. Test starts without errors
2. On your Garmin watch: Menu > Sensors > Add > Foot Pod
3. Watch pairs within 10 seconds
4. Watch displays:
   - Pace: ~7:00 min/km (varies through test stages)
   - Cadence: 114-183 SPM
   - Distance: accumulating

**Expected Output:**
```
Starting ANT+ broadcaster with device ID 54321...
Broadcaster started successfully.

==================================================
 STAGE 1/6: Warm-up Walk
--------------------------------------------------
  TARGET SPEED:   5.0 km/h
  EXPECTED PACE:  ~12:00-12:05
  EXPECTED CADENCE: 114 SPM
==================================================
```

**Pass Criteria:**
- [ ] Watch pairs successfully
- [ ] Pace updates in real-time
- [ ] Cadence shows correctly
- [ ] Distance increments
- [ ] No USB errors

**Stop test:** Press Ctrl+C

---

### 3. GUI Mode with Treadmill (5 minutes)

Test full application with real treadmill.

**Prerequisites:**
- Bluetooth treadmill paired and connected
- ANT+ dongle plugged in

**Steps:**
```bash
sudo ./qdomyos-zwift -ant-footpod
```

1. Application launches and connects to treadmill
2. On Garmin watch: Add Foot Pod sensor
3. Watch pairs (shows "Searching" then "Connected")
4. Walk/run on treadmill for 3 minutes
5. Verify watch data matches treadmill display

**Pass Criteria:**
- [ ] GUI launches without errors
- [ ] Treadmill connects via Bluetooth
- [ ] Watch pairs within 10 seconds
- [ ] Pace matches treadmill speed (±0:05 min/km tolerance)
- [ ] Cadence shows realistic values (90-200 SPM range)
- [ ] No disconnections during test
- [ ] No crashes or freezes

---

### 4. Headless Mode (5 minutes)

Test ANT+ without GUI (server/headless deployment).

```bash
sudo ./qdomyos-zwift -no-gui -ant-footpod
```

**What to verify:**
1. Application starts in console mode
2. Treadmill connects (check console output)
3. Watch pairs
4. Run for 3 minutes
5. Data flows correctly

**Expected Console Output:**
```
OK, you are root.
Bluetooth: Scanning for devices...
Bluetooth: Connected to <TREADMILL_NAME>
[ANT+] Broadcaster started (Device ID 54321)
Speed: 8.5 km/h → Pace: 7:04 min/km, Cadence: 166 SPM
```

**Pass Criteria:**
- [ ] No GUI displayed
- [ ] Treadmill connects via Bluetooth
- [ ] Watch receives ANT+ data
- [ ] Runs stably for 5+ minutes
- [ ] Can stop cleanly with Ctrl+C

---

### 5. Reset and Reinstall (Optional - 5 minutes)

Verify cleanup and reinstall works correctly.

```bash
# Clean everything
sudo ./setup.sh --reset

# Verify cleaned
./setup.sh --quick
# Should show: "Virtual environment not found", "User not in plugdev", etc.

# Reinstall
sudo ./setup.sh --guided

# Verify fixed
./setup.sh --quick
# Should show: All tests passing
```

**Pass Criteria:**
- [ ] Reset removes configurations
- [ ] Quick check detects missing items
- [ ] Guided setup reinstalls successfully
- [ ] All tests pass after reinstall

**Note:** On desktop systems with graphical environments (e.g., ubuntu-desktop installed), system packages like Qt5 and libusb are protected from removal during reset. Only ANT+ specific configurations (udev rules, plugdev group membership, Python virtual environment) are removed.

---

## Pre-Review Checklist

Before submitting PR, verify:

- [ ] `./setup.sh --quick` passes all 12 tests
- [ ] `sudo ./setup.sh --test` pairs with watch and broadcasts data
- [ ] GUI mode (`sudo ./qdomyos-zwift -ant-footpod`) works with treadmill
- [ ] Headless mode (`-no-gui -ant-footpod`) runs stably
- [ ] Watch data matches treadmill (pace ±0:05 min/km, cadence realistic)
- [ ] No crashes during 5+ minute runs
- [ ] Setup/reset/reinstall cycle works
- [ ] README.md instructions are accurate
- [ ] No `qml-module-qtmultimedia` missing error

**Estimated total testing time:** 15-20 minutes

---

## Common Issues

### Test Mode Fails

**Symptom:** `./setup.sh --test` shows USB errors or "Resource busy"

**Solutions:**
1. Check if qdomyos-zwift is already running: `sudo pkill -f qdomyos-zwift`
2. Verify dongle detected: `lsusb | grep -i dynastream`
3. Check permissions and configuration: `./setup.sh --quick`
4. Reboot system after USB permissions setup
5. Try unplugging and replugging the dongle

### GUI Won't Start

**Symptom:** Application exits immediately with no error message

**Solutions:**
1. Check for missing QML module: `./setup.sh --quick` (validates 9 QML modules)
2. Install missing package if needed: `sudo apt install qml-module-qtmultimedia`
3. Verify Qt5 libraries: `./setup.sh --quick` (validates 12 Qt5 libraries)
4. Check binary architecture matches system:
   - Raspberry Pi requires ARM64 binary (`linux-binary-arm64-ant.zip`)
   - Desktop Linux requires x86-64 binary (`linux-binary-x86-64-ant.zip`)

### Watch Won't Pair

**Symptom:** Watch shows "Searching..." but never connects to foot pod

**Solutions:**
1. Verify ANT+ test works independently: `sudo ./setup.sh --test`
2. Remove old foot pod pairing from watch memory
3. Restart watch completely
4. Ensure dongle is plugged directly into USB port (not USB hub)
5. Check dongle is detected: `lsusb | grep -i dynastream`

### Data Doesn't Match Treadmill

**Symptom:** Watch pace is significantly different from treadmill speed

**Solutions:**
1. Verify treadmill calibration is correct
2. Check ANT+ test shows correct data progression through stages
3. Watch stride length may need manual calibration
4. ANT+ broadcasts actual speed - watch calculates pace from speed
5. Allow 5-10 seconds for watch to stabilize initial reading

### Python Library Error

**Symptom:** `error while loading shared libraries: libpython3.11.so.1.0`

**Solutions:**
1. Run quick validation: `./setup.sh --quick`
2. If Python 3.11 missing, install via apt: `sudo apt install python3.11`
3. If not available in apt, install via pyenv: `sudo ./setup.sh --guided`
4. After pyenv installation, reload shell: `source ~/.bashrc`

### Treadmill Not Configured

**Symptom:** Watch pairs but pace shows `--:--` or no treadmill connection

**Solutions:**
1. Ensure treadmill is powered on and in Bluetooth pairing mode
2. Check treadmill configuration in settings file
3. Edit configuration: `/root/.config/Roberto\ Viola/qDomyos-Zwift.conf`
4. Add your treadmill model (e.g., `proform_treadmill_705_cst=true`)
5. Verify treadmill is supported: check README.md or main project documentation

---

## Related Documentation

- **User Setup:** [README.md](README.md)
- **Build Instructions:** [COMPILE.md](COMPILE.md)
- **Architecture:** See source comments in `AntManager.cpp` and `AntWorker.cpp`
