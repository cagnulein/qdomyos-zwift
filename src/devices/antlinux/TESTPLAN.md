# **ANT+ Pre-Compiled Binary Validation Test Plan**

## 1. Overview

This plan verifies the **pre-compiled `qdomyos-zwift` binary** across both x86_64 and ARM64 platforms.
It ensures that environment setup, dependency management, ANT+ communication (including Garmin pairing), and systemd service operation all work correctly, both in GUI and headless (`-no-gui -ant-footpod`) modes.

---

## 2. Objective

Validate that the binary:

1. Runs successfully across supported Python versions.
2. Handles dependency and permission errors gracefully.
3. Correctly communicates with ANT+ devices (Garmin watch / footpod).
4. Operates in both **GUI** and **headless modes**.
5. Functions under `systemd` for unattended start-up.

---

## 3. Scope

**In scope:**

* `pyenv`-based Python installation and switching.
* Clean baseline re-creation.
* GUI + headless operation.
* Fake treadmill simulation.
* Garmin ANT+ pairing.
* Systemd service verification.

**Out of scope:**

* UI layout testing.
* Non-Ubuntu distributions.

---

## 4. Test Preparation

### 4.1 Why Use `pyenv`

All platforms (x86 + ARM) will use **`pyenv`** for Python management because it:

* Allows deterministic installation of required Python 3.11.x.
* Avoids OS package conflicts.
* Enables seamless version switching during tests.
* Provides identical behavior on both architectures.

---

### 4.2 Clean Baseline Preparation

Remove all Python versions and cached environments:

```bash
sudo apt remove --purge -y python3 python3.* python-is-python3
sudo rm -rf ~/.pyenv ~/.local/lib/python* /usr/local/lib/python* /usr/lib/python*
sudo apt autoremove -y
sudo apt clean
hash -r
sudo reboot
```

---

### 4.3 Install Prerequisites

```bash
sudo apt update && sudo apt install -y \
  git curl build-essential libssl-dev zlib1g-dev \
  libbz2-dev libreadline-dev libsqlite3-dev wget llvm \
  libncurses5-dev libncursesw5-dev xz-utils tk-dev
```

---

### 4.4 Install `pyenv` and Required Python Versions

```bash
curl https://pyenv.run | bash
```

Add to shell profile:

```bash
echo 'export PYENV_ROOT="$HOME/.pyenv"' >> ~/.bashrc
echo 'command -v pyenv >/dev/null || export PATH="$PYENV_ROOT/bin:$PATH"' >> ~/.bashrc
echo 'eval "$(pyenv init -)"' >> ~/.bashrc
source ~/.bashrc
```

Install Python versions for testing:

```bash
pyenv install 3.10.14
pyenv install 3.11.9
pyenv install 3.12.6
pyenv global 3.11.9
```

Verify:

```bash
pyenv versions
```

---

### 4.5 Download and Prepare Binary Package

```bash
mkdir ~/qdomyos-test && cd ~/qdomyos-test
wget <binary_package_url> -O qdomyos-bundle.tar.gz
tar -xvzf qdomyos-bundle.tar.gz
cd qdomyos-zwift
chmod +x qdomyos-zwift
```

---

### 4.6 Setting Up a Fake Treadmill

If no physical treadmill is available, simulate one:

```bash
# Run once to create the initial profile
sudo ./qdomyos-zwift
# Stop the program after it starts
Ctrl+C
```

Then edit the config (root-owned):

```bash
sudo nano /root/.config/Roberto\ Viola/qDomyos-Zwift.conf
```

Enable simulation:

```ini
[General]
fakedevice_treadmill=true
virtual_device_bluetooth=true
```

Save and exit.

---

### 4.7 Prepare Runtime Check Script

Ensure executable:

```bash
chmod +x ./runtime_check.sh
```

---

## 5. Test Execution

Each test starts from a prepared environment with the binary unpacked and Python 3.11.9 active.

---

### TC1 – Baseline Verification (Missing Dependencies)

**Purpose:** Confirm correct error handling with incomplete setup.

```bash
pyenv global 3.11.9
rm -rf venv
./runtime_check.sh
```

**Expected:** Script reports missing Python modules; no crash.

---

### TC2 – Dependency Installation

```bash
python -m venv venv
source venv/bin/activate
pip install pyqt5 pyqt5-tools hidapi ant
./runtime_check.sh
```

**Expected:** All dependency checks pass ✅

---

### TC3 – Launch in GUI Mode

```bash
sudo ./qdomyos-zwift
```

**Expected:**

* GUI opens with no errors.
* Treadmill (fake or real) visible in device list.
* ANT+ dongle activity light flashes.
* Garmin watch can pair.

---

### TC4 – Launch in Headless Mode (No GUI, ANT+ Enabled)

```bash
sudo ./qdomyos-zwift -no-gui -ant-footpod
```

**Purpose:** Validate headless operation and ANT+ footpod broadcast.
**Expected:**

* Log output confirms “ANT+ footpod initialized”.
* Garmin watch pairs and displays speed/cadence.
* Continuous console output shows telemetry.

---

### TC5 – Switch Python Versions

Sequentially test compatibility:

```bash
pyenv global 3.10.14
./runtime_check.sh
sudo ./qdomyos-zwift -no-gui -ant-footpod

pyenv global 3.12.6
./runtime_check.sh
sudo ./qdomyos-zwift -no-gui -ant-footpod

pyenv global 3.11.9
./runtime_check.sh
sudo ./qdomyos-zwift -no-gui -ant-footpod
```

**Expected:**

* Only 3.11.9 fully functional.
* Other versions show version mismatch but no crash.

---

### TC6 – USB Permission Validation

```bash
sudo usermod -aG plugdev $USER
sudo reboot
```

After reboot:

```bash
./runtime_check.sh
sudo ./qdomyos-zwift -no-gui -ant-footpod
```

**Expected:** No `/dev/ttyUSB*` permission errors.

---

### TC7 – Systemd Service Verification

Create and enable:

```bash
sudo cp qdomyos-zwift.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable qdomyos-zwift
sudo systemctl start qdomyos-zwift
```

**Expected:**

* Service runs in headless mode automatically.
* `systemctl status qdomyos-zwift` → `active (running)`.
* ANT+ pairing persists.

---

### TC8 – Binary Architecture Compatibility

Attempt to run a binary from the opposite architecture:

```bash
sudo ./qdomyos-zwift -no-gui -ant-footpod
```

**Expected:**

* Immediate “Exec format error”.
* Runtime script logs “Architecture mismatch”.

---

## 6. ANT+ Functional Verification

### 6.1 Garmin Watch Pairing

1. Set Garmin watch to **Indoor Run** or **Footpod Mode**.

2. Run application in headless mode:

   ```bash
   sudo ./qdomyos-zwift -no-gui -ant-footpod
   ```

3. Confirm pairing via:

   * Watch shows treadmill metrics.
   * Console logs show “ANT+ footpod connected”.

**Pass Criteria:** Bidirectional data visible both on watch and in console output.

---

## 7. Success Criteria

Test suite passes when:

* Runtime script reports all green.
* `qdomyos-zwift` runs cleanly under Python 3.11.9.
* Garmin pairing succeeds in both GUI and headless modes.
* Fake treadmill telemetry transmits correctly.
* Systemd service starts automatically.
* No permission or version errors remain.

---

## 8. Acceptance Checklist

| Item                                            | Status |
| ----------------------------------------------- | ------ |
| Clean environment set up via pyenv              | ☐      |
| Python 3.11.9 verified                          | ☐      |
| Dependencies installed                          | ☐      |
| Fake treadmill configured                       | ☐      |
| ANT+ pairing verified                           | ☐      |
| Headless `-no-gui -ant-footpod` mode functional | ☐      |
| Systemd service running                         | ☐      |
| Cross-version tests complete                    | ☐      |

---

## Appendix A – Reference Commands

```bash
# Switch Python version
pyenv global <version>

# Verify Python version
python --version

# Reset to system default
pyenv global system

# Run headless ANT+ mode
sudo ./qdomyos-zwift -no-gui -ant-footpod
```