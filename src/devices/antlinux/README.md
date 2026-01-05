# ANT+ Virtual Footpod Broadcaster for Linux

Transform your Linux system into an ANT+ bridge that broadcasts **treadmill** data as a virtual footpod. Your Garmin watch or other ANT+ device will display real-time pace and cadence, just like running outdoors.

## Device Type Guidance

**This guide is specifically for treadmill users.** The ANT+ footpod broadcasting feature is designed for treadmills only.

If you're using bikes, ellipticals, or rowers, you can still use the main application, but simply omit the `-ant-footpod` flag from all commands shown in this guide.

## What You'll Achieve
- Broadcast treadmill pace, distance, and cadence in real-time
- Pair your Garmin watch as a Foot Pod for accurate indoor run tracking
- Automatically log runs to your fitness ecosystem

## Prerequisites

**Hardware Requirements:**
- ANT+ USB dongle (Garmin USB2/USB-m/USB3/mini, or Suunto)
- Garmin watch or compatible ANT+ device
- Raspberry Pi (Zero 2 W, 3, 4, or 5) **OR** x86-64 PC/Laptop

**Software Requirements:**
- Linux distribution: Debian/Ubuntu-based (Bookworm or newer recommended)
- Linux terminal skills (navigating directories, editing files)

**Tested Configurations:**
- **Raspberry Pi:** Zero 2 W (Raspberry Pi OS Bookworm)
- **Desktop Linux:** Ubuntu 24.04 LTS (x86-64)
- **Treadmill:** Proform 705 CST
- **Watch:** Garmin Forerunner 245

> **Tip:** If you are using this on your main desktop, it's a good idea to back up your system before making changes.

---

## Installation

This guide uses **pre-compiled binaries** from GitHub Releases, the easiest method for most users.

## Step 1: Download and Extract

- Visit the **[Releases page](https://github.com/cagnulein/qdomyos-zwift/releases)**
- **Desktop Linux:** Download: `linux-binary-x86-64-ant.zip` 
- **Raspberry Pi:** Download: `linux-binary-arm64-ant.zip` 

### Extract to Your Home Directory

```bash
cd ~
ARCH=$(uname -m | sed -e 's/aarch64/arm64/' -e 's/x86_64/x86-64/')
unzip linux-binary-$ARCH-ant.zip
cd qdomyos-zwift-$ARCH-ant
```

### Package Contents
- `qdomyos-zwift` - Smart wrapper script (use this, not the binary directly)
- `qdomyos-zwift-bin` - Application binary
- `devices.ini` - Device mappings
- `setup-dashboard.sh` - Interactive setup and validation dashboard
- `test_ant.py` - Standalone ANT+ test (used by setup-dashboard.sh)
- `ant_broadcaster.py` - ANT+ module (used by test_ant.py)
- `devices_optimized.json` - Device menu JSON (used by setup-dashboard.sh)
- `.menu_cache/` - Per-section menu caches (used by setup-dashboard.sh)


## Step 2: Check Your System

Before installing anything, see what's needed by running the script.

```bash
sudo ./setup-dashboard.sh
```
This validates your setup:
- Python 3.11 availability
- Python Virtual environment
- Python packages (openant, pyusb, pybind11, bleak)
- Qt5 libraries
- USB permissions (plugdev group, udev rules)
- ANT+ dongle detection
- Bluetooth service

**All tests pass?** Skip to Step 4!

**Issues found?** Continue to Step 3 for guided setup.

---

## Step 3: Install Dependencies

### Guided Setup

Let the script guide you through the installation steps.

```bash
sudo ./setup-dashboard.sh
```

The dashboard explains each step and asks for confirmation before making changes.

---

### Manual Installation (Advanced Users)

Prefer full control? Follow these steps to install each component yourself. These are the same steps the guided setup performs.

#### 1. Install System Dependencies

```bash
sudo apt-get update
sudo apt-get install -y \
	libqt5core5a \
	libqt5qml5 \
	libqt5quick5 \
	libqt5quickwidgets5 \
	libqt5concurrent5 \
	libqt5bluetooth5 \
	libqt5charts5 \
	libqt5multimedia5 \
	libqt5multimediawidgets5 \
	libqt5multimedia5-plugins \
	libqt5networkauth5 \
	libqt5positioning5 \
	libqt5sql5 \
	libqt5texttospeech5 \
	libqt5websockets5 \
	libqt5widgets5 \
	libqt5xml5 \
	libqt5location5 \
	qtlocation5-dev \
	qml-module-qtlocation \
	qml-module-qtpositioning \
	qml-module-qtquick2 \
	qml-module-qtquick-controls \
	qml-module-qtquick-controls2 \
	qml-module-qtquick-dialogs \
	qml-module-qtquick-layouts \
	qml-module-qtquick-window2 \
	qml-module-qtmultimedia \
	libusb-1.0-0 \
	bluez \
	usbutils \
	python3-pip
```

#### 2. Install Python 3.11

The pre-compiled binaries require Python 3.11. Check availability:

```bash
python3.11 --version
```

**If Python 3.11 is available:**
```bash
sudo apt-get install -y python3.11 python3.11-venv
```

**If Python 3.11 is NOT available**, install via pyenv:
```bash
# Install pyenv dependencies
sudo apt-get install -y \
	git curl build-essential libssl-dev zlib1g-dev \
	libbz2-dev libreadline-dev libsqlite3-dev wget \
	llvm libncurses5-dev libncursesw5-dev xz-utils \
	tk-dev libffi-dev liblzma-dev

# Note: Ubuntu 24.04+ users should use libncurses-dev instead:
# sudo apt-get install -y libncurses-dev

# Install pyenv
curl https://pyenv.run | bash

# Add pyenv to shell
echo 'export PYENV_ROOT="$HOME/.pyenv"' >> ~/.bashrc
echo 'command -v pyenv >/dev/null || export PATH="$PYENV_ROOT/bin:$PATH"' >> ~/.bashrc
echo 'eval "$(pyenv init -)"' >> ~/.bashrc

# Reload shell configuration
source ~/.bashrc

# Install Python 3.11
pyenv install 3.11.9
pyenv global 3.11.9

# Verify installation
python --version  # Should show Python 3.11.9
```

#### 3. Create Python Virtual Environment with required packages

QZ looks for `ant_venv` in your home directory:

```bash
# Create virtual environment
python3.11 -m venv ~/ant_venv

# Install required packages (do NOT use sudo)
~/ant_venv/bin/pip install --upgrade pip
~/ant_venv/bin/pip install openant pyusb pybind11 bleak
```

#### 4. Configure USB Permissions

```bash
# Create udev rule for ANT+ dongles
sudo tee /etc/udev/rules.d/99-ant-usb.rules > /dev/null << 'EOF'
SUBSYSTEM=="usb", ATTRS{idVendor}=="0fcf", ATTRS{idProduct}=="100?", MODE="0666", GROUP="plugdev"
SUBSYSTEM=="usb", ATTRS{idVendor}=="0fcf", ATTRS{idProduct}=="88a4", MODE="0666", GROUP="plugdev"
SUBSYSTEM=="usb", ATTRS{idVendor}=="11fd", ATTRS{idProduct}=="0001", MODE="0666", GROUP="plugdev"
EOF

# Apply permissions
sudo udevadm control --reload-rules && sudo udevadm trigger
sudo usermod -aG plugdev $USER
```

#### 5. Enable Bluetooth Service

```bash
# Ensure Bluetooth is running (required for treadmill connection)
sudo systemctl start bluetooth
sudo systemctl enable bluetooth
```

#### 6. Verify Manual Installation

After installation, verify everything works:

```bash
cd ~/qdomyos-zwift-x86-64-ant  # or qdomyos-zwift-arm64-ant
sudo ./setup-dashboard.sh
```

All tests should pass. If issues remain, the tool will provide specific guidance.

---

### Other Useful Commands

**Test ANT+ independently (via dashboard):**

Run `sudo ./setup-dashboard.sh` and choose "ANT+ Test" from the menu. The dashboard will simulate treadmill data and display user-friendly output.

Advanced: to run the standalone test script directly (advanced users):

```bash
sudo ~/ant_venv/bin/python3 ./test_ant.py
```
This runs the same simulation but without the dashboard UI.

---

## Step 4: Configure and Run

### Configuration Settings

Configuration is required to set up your user profile (age, weight, units) and select your specific treadmill model.

### Does your system have a display?

**YES - I have a display (Desktop/Laptop):**

Simply run the application. The GUI will open where you can configure everything:

```bash
sudo ./qdomyos-zwift -ant-footpod  # for treadmills
sudo ./qdomyos-zwift                # for bikes/ellipticals/rowers
```

Your settings are saved automatically. You're done!

---

**NO - I'm running headless (Raspberry Pi/Server):**

You need to create a configuration file. Choose the method that works best for you:

**Method 1: Interactive Setup (Easiest)**

Run the dashboard:

```bash
sudo ./setup-dashboard.sh
```

- Select "User Profile" to enter your age, weight, and units.
- Select "Equipment" to choose your treadmill model.
- Select Bluetooth Scan to choose the device if switched on and near by
- The dashboard will create the configuration file for you.

**Method 2: Configure on Another System and Transfer**

If you have access to another computer with a display:

1. On the system with a display, run the GUI and configure your settings:
   ```bash
   sudo ./qdomyos-zwift -ant-footpod
   ```

2. Copy the configuration file:
   ```bash
   cp "~/.config/Roberto Viola/qDomyos-Zwift.conf" ~/qz-config.conf
   # or 
   sudo cp "/root/.config/Roberto Viola/qDomyos-Zwift.conf" ~/qz-config.conf
   ```

3. Transfer `qz-config.conf` to your headless system (via scp/sftp/USB)

4. On your headless system, place the file in the correct location:
   ```bash
   mkdir -p "~/.config/Roberto Viola"
   cp ~/qz-config.conf "~/.config/Roberto Viola/qDomyos-Zwift.conf"
   ```

---


### Understanding Device Pairing

**Pairing your Garmin watch:**
1. Menu → Sensors & Accessories → Add New → Foot Pod
2. Watch detects and pairs automatically (5-10 seconds)
3. Start "Treadmill" or "Run Indoor" activity
4. **Add Cadence field:** Settings → Activities & Apps → [Activity] → Data Screens → customize

**Cadence Calculation:**

The system uses biomechanics research to switch between walking and running cadence at **7.0 km/h** (the walk-run transition zone):

| Speed | Cadence Type | Typical Range | Example |
|-------|--------------|---------------|---------|
| < 7.0 km/h | Walking | 90-140 SPM | 6.0 km/h ≈ 128 SPM |
| ≥ 7.0 km/h | Running | 160-200 SPM | 8.5 km/h ≈ 166 SPM |

> See "Further Reading" section for research references.

### Test the Application

Connect your ANT+ dongle, start your treadmill, and run QZ:

```bash
cd ~/qdomyos-zwift-arm64-ant  # or ~/qdomyos-zwift-x86-64-ant
sudo ./qdomyos-zwift -no-gui -ant-footpod

# Optional: Custom device ID (useful for conflicts, range 1-65535)
sudo ./qdomyos-zwift -no-gui -ant-footpod -ant-device 12345

# Optional: Verbose logging (debug only - creates large logs)
sudo ./qdomyos-zwift -no-gui -ant-footpod -ant-verbose
```

**Note:** See dependency warnings? The wrapper provides specific fix instructions. The app may still run but ANT+ won't work until resolved.


### Configure Automatic Startup

The `setup-dashboard.sh` script can create and enable the systemd service file to start automatically for you at the correct location for your platform.

**To manually create or modify the service:**

**Platform-specific values:**

| Platform | Service Location | Username | Install Path |
|----------|-----------------|----------|--------------|
| Raspberry Pi | `/etc/systemd/system/qz.service` | `pi` | `/home/pi/qdomyos-zwift-arm64-ant` |
| Desktop Linux | `/lib/systemd/system/qz.service` | YOUR_USERNAME | `/home/YOUR_USERNAME/qdomyos-zwift-x86-64-ant` |

**Create the service file:**

```bash
sudo nano <SERVICE_LOCATION>
```

```ini
[Unit]
Description=qdomyos-zwift service
After=multi-user.target

[Service]
User=root
Group=plugdev
Environment="QZ_USER=<USERNAME>"
WorkingDirectory=<INSTALL_PATH>
ExecStart=<INSTALL_PATH>/qdomyos-zwift -no-gui -log -ant-footpod
KillSignal=SIGINT
SuccessExitStatus=130

[Install]
WantedBy=multi-user.target
```

Replace `<SERVICE_LOCATION>`, `<USERNAME>`, and `<INSTALL_PATH>` using the values from the table above for your platform.

**Enable and start the service:**

```bash
sudo systemctl daemon-reload
sudo systemctl enable qz
sudo systemctl start qz
sudo systemctl status qz  # Check it's running
```

## Troubleshooting

### Quick Diagnostics

**Run the System Check:**
```bash
sudo ./setup-dashboard.sh
```
This validates your setup and identifies any missing dependencies or configuration issues.

**Test ANT+ Independently:**

If you need to verify that ANT+ broadcasting works separately from the main application:

```bash
sudo ./setup-dashboard.sh
```
Select "ANT+ Test" from the menu. This simulates a running treadmill at various speeds. When paired, your watch will display the current test stage with varying pace and cadence.

**Advanced:** To run the test script directly:
```bash
sudo ~/ant_venv/bin/python3 ./test_ant.py
```

---

### Common Issues

| Problem | Solution |
|---------|----------|
| ANT+ dongle not detected | Run `lsusb` and look for Garmin, Suunto, or Dynastream device. Unplug/replug dongle. Verify USB permissions and reboot |
| `error while loading shared libraries: libpython3.11.so.1.0` | Python 3.11 missing. Run `sudo ./setup-dashboard.sh` to diagnose and install via apt or pyenv |
| Wrapper shows dependency warnings | Run `sudo ./setup-dashboard.sh` for diagnosis and guided fixes |
| Test fails / watch won't pair | Ensure running with `sudo`. Reboot after USB permissions setup. Try unplug/replug dongle |
| Watch pairs but pace shows `--:--` | Treadmill not configured. Edit your configuration file and add your model (e.g., `proform_treadmill_705_cst=true`) |
| Commands not running as root | Use `sudo` for setup-dashboard.sh, test_ant.py, and qdomyos-zwift |
| Configuration file missing | Check `/root/.config/Roberto Viola/qDomyos-Zwift.conf` or `~/.config/Roberto Viola/qDomyos-Zwift.conf` |
| Treadmill not pairing via Bluetooth | Ensure treadmill is powered on and in pairing mode |
| App works but no watch connection | Unplug/replug dongle. Verify USB permissions and reboot. Check device ID (default 54321). Check logs |
| Changes not taking effect | Reboot after changing USB permissions or user groups |
| Wrong device flag used | Treadmills: add `-ant-footpod` flag. Bikes/ellipticals/rowers: omit this flag |
| `systemctl stop qz` hangs | Add `KillSignal=SIGINT` to service file `[Service]` section |
| Binary won't run: "cannot execute binary file" | Wrong architecture. Download correct package: arm64 for Pi, x86-64 for desktop |
| `pyenv: command not found` | Reload shell: `source ~/.bashrc` or open new terminal |
| Multiple validation failures | Run `sudo ./setup-dashboard.sh` and use the Interactive Setup for step-by-step fixes |
| Test works but app doesn't connect | Treadmill Bluetooth pairing or configuration issue. Verify treadmill is powered and discoverable |

---

## Credits & Further Reading

**Project & Development:**
- Main project: https://github.com/cagnulein/qdomyos-zwift
- ANT+ Linux footpod implementation: bassai-sho

**Technology:**
- ANT+ is a wireless protocol managed by Garmin Canada Inc.
- This is an independent open-source implementation for interoperability

**Research References:**

[Preferred transition speed between walking and running](https://pubmed.ncbi.nlm.nih.gov/16286854/)  
*Explains the biomechanical walk-run transition zone around 7.0-7.4 km/h*

[Biomechanics of Gait Transition](https://scholarworks.boisestate.edu/cgi/viewcontent.cgi?article=1178&context=td)  
*Detailed analysis of cadence patterns during gait transitions*