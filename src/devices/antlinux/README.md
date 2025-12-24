# ANT+ Virtual Footpod Broadcaster for Linux

Transform your Linux system into an ANT+ bridge that broadcasts treadmill data as a virtual footpod. Your Garmin watch or other ANT+ device will display real-time pace, distance, and cadence—just like running outdoors.

## Device Type Guidance

This guide provides instructions for different device types. Select the correct flag for your equipment:

- **Treadmills:** Use the `-ant-footpod` flag as shown in the examples below.
- **Bikes, Rowers, or Other Devices:** Omit the `-ant-footpod` flag. Use the standard commands without it.

Using the appropriate flag ensures your device works as expected. If you are unsure, refer to the Interactive Setup for further guidance. 

**Summary:**
- For **treadmills**, always add the `-ant-footpod` flag to your command.
- For **bikes, rowers, or other devices**, do **not** use the `-ant-footpod` flag.

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
- Basic Linux terminal skills (navigating directories, editing files)

**Tested Configurations:**
- **Raspberry Pi:** Zero 2 W (Raspberry Pi OS Bookworm)
- **Desktop Linux:** Ubuntu 24.04 LTS (x86-64)
- **Treadmill:** Proform 705 CST
- **Watch:** Garmin Forerunner 245

> **Tip:** It's a good idea to back up your system before making changes, but this is optional for most users.

---


## Table of Contents


If you are new to this guide, start with the [Quick Start Guide](#quick-start-guide).
If you encounter issues, see [Troubleshooting](#troubleshooting).

### Main Guide
1. [What You'll Achieve](#what-youll-achieve)
2. [Prerequisites](#prerequisites)
3. [Quick Start Guide](#quick-start-guide)
	- Download & Extract
	- System Check
	- Interactive Setup
	- Configuration & Testing
4. [Automatic Startup](#automatic-startup)
5. [Troubleshooting](#troubleshooting)

### Appendices
- [Compilation Guide](COMPILE.md)

## Quick Start Guide

This guide uses **pre-compiled binaries** from GitHub Releases—the easiest method for most users.

> **Developers:** Need to compile from source? See the [Compilation Guide](COMPILE.md).



### Verifying Your System Architecture

Before proceeding, you can check your system architecture to ensure you have downloaded the correct binary:

```bash
uname -m
```

- If the output is `x86_64`, use the x86-64 binary.
- If the output is `aarch64`, use the arm64 binary.

If you have the wrong binary, please download the appropriate version from the releases page.


### Installation Steps

1. **Download Binary** - Get the pre-compiled package for your platform
2. **Run the System Check** - See what (if anything) you actually need:

	```bash
	./setup-dashboard.sh
	```

	- If all tests pass, you can proceed directly to running and configuring the application.
	- If there are warnings or failures, follow the guidance provided by the System Check or continue with the Interactive Setup.

3. **Install Dependencies** - Only if the System Check indicates they are needed
4. **Run & Automate** - Test and configure automatic startup

**Note:** For safety, reset/uninstall functionality is not provided. If you need to remove QDomyos-Zwift or its components, please do so manually.

---

## Step 1: Download and Extract

### Download from GitHub Releases

1. Visit the **[Releases page](https://github.com/cagnulein/qdomyos-zwift/releases)**
2. Download the package for your platform:
   - **Raspberry Pi (ARM64):** `linux-binary-arm64-ant.zip`
   - **Desktop Linux (x86-64):** `linux-binary-x86-64-ant.zip`

### Extract to Your Home Directory

**Raspberry Pi:**
```bash
cd ~
unzip linux-binary-arm64-ant.zip
cd qdomyos-zwift-arm64-ant
```

**Desktop Linux:**
```bash
cd ~
unzip linux-binary-x86-64-ant.zip
cd qdomyos-zwift-x86-64-ant
```

**Package Contents:**
- `qdomyos-zwift` - Smart wrapper script (use this, not the binary directly)
- `qdomyos-zwift-bin` - Application binary
- `setup-dashboard.sh` - Interactive setup and validation dashboard
- `test_ant.py` - Standalone ANT+ test for troubleshooting
- `ant_broadcaster.py` - ANT+ module used by test script

---

## Step 2: Check Your System

Before installing anything, see what's needed:

```bash
./setup-dashboard.sh
```

This validates your entire setup in seconds:
- Python 3.11 availability
- Virtual environment
- Python packages (openant, pyusb, pybind11)
- Qt5 libraries
- USB permissions (plugdev group, udev rules)
- ANT+ dongle detection
- Bluetooth service

**All tests pass?** Skip to Step 4!

**Issues found?** Continue to Step 3 for automatic fixes. Most issues can be resolved with the Interactive Setup or by following the provided instructions.

---

## Step 3: Install Dependencies

### Automatic Setup (Recommended)

Let the Interactive Setup guide you through installation:

Run the interactive dashboard which adapts to GUI or headless environments automatically:

```bash
sudo ./setup-dashboard.sh
```

The dashboard explains each step and asks for confirmation before making changes.

### Other Useful Commands

**Test ANT+ independently (via dashboard):**

Run `sudo ./setup-dashboard.sh` and choose "Test ANT+ Hardware" from the success menu. The dashboard will simulate treadmill data and display user-friendly output.

Advanced: to run the standalone test script directly (advanced users):

```bash
sudo ~/ant_venv/bin/python3 ./test_ant.py
```
This runs the same simulation but without the dashboard UI.


---

### Manual Installation (Advanced Users)

Prefer full control? Follow these steps to install each component yourself.

#### Install System Dependencies

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

#### Install Python 3.11

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

#### Create Python Virtual Environment

QZ looks for `ant_venv` in your home directory:

```bash
# Create virtual environment
python3.11 -m venv ~/ant_venv

# Install required packages (do NOT use sudo)
~/ant_venv/bin/pip install --upgrade pip
~/ant_venv/bin/pip install openant pyusb pybind11
```

#### Configure USB Permissions

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

# Reboot to apply group membership
sudo reboot
```

#### Enable Bluetooth Service

```bash
# Ensure Bluetooth is running (required for treadmill connection)
sudo systemctl start bluetooth
sudo systemctl enable bluetooth
```

#### Verify Manual Installation

After installation, verify everything works:

```bash
cd ~/qdomyos-zwift-x86-64-ant  # or qdomyos-zwift-arm64-ant
./setup-dashboard.sh
```

All tests should pass. If issues remain, the tool will provide specific guidance.

---

## Step 4: Run and Configure

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

Create a systemd service for automatic startup on boot:

**Raspberry Pi configuration:**
```bash
sudo nano /lib/systemd/system/qz.service
```
```ini
[Unit]
Description=qdomyos-zwift service
After=multi-user.target

[Service]
User=root
Group=plugdev
Environment="QZ_USER=pi"
WorkingDirectory=/home/pi/qdomyos-zwift-arm64-ant
ExecStart=/home/pi/qdomyos-zwift-arm64-ant/qdomyos-zwift -no-gui -log -ant-footpod
KillSignal=SIGINT

[Install]
WantedBy=multi-user.target
```

**Desktop Linux configuration:**
```bash
sudo nano /etc/systemd/system/qz.service
```
```ini
[Unit]
Description=qdomyos-zwift service
After=multi-user.target

[Service]
User=root
Group=plugdev
Environment="QZ_USER=YOUR_USERNAME"
WorkingDirectory=/home/YOUR_USERNAME/qdomyos-zwift-x86-64-ant
ExecStart=/home/YOUR_USERNAME/qdomyos-zwift-x86-64-ant/qdomyos-zwift -no-gui -log -ant-footpod
KillSignal=SIGINT

[Install]
WantedBy=multi-user.target
```

> **Important:** Replace `YOUR_USERNAME` with your actual username.

**Enable and start the service:**

```bash
sudo systemctl daemon-reload
sudo systemctl enable qz
sudo systemctl start qz
sudo systemctl status qz  # Check it's running
```


## Configuration

### For GUI Systems (Desktop/Laptop)
1. Run the application: `sudo ./qdomyos-zwift -ant-footpod` (for treadmills) or `sudo ./qdomyos-zwift` (for bikes/rowers/other)
2. The graphical interface will open—select your device type and configure preferences.
3. Settings are saved automatically to: `~/.config/Roberto Viola/qDomyos-Zwift.conf`
4. You can run the application anytime with the same command.

### For Headless Systems (Raspberry Pi/Server)

There are two main options:

**Option 1: Configure on a System with a Display (Recommended)**
1. On any system with a display:
	- For treadmills: run `sudo ./qdomyos-zwift -ant-footpod`
	- For bikes, rowers, or other devices: run `sudo ./qdomyos-zwift`
	- Configure your device and preferences

2. Transfer the configuration file to your headless system:
   
	The configuration file could be located at either of the following paths, depending on how you ran the application:
	- `/root/.config/Roberto Viola/qDomyos-Zwift.conf`
	- `~/.config/Roberto Viola/qDomyos-Zwift.conf`

	```bash
	# On the GUI system
	# Copy from the location where your configuration was saved
	sudo cp "/root/.config/Roberto Viola/qDomyos-Zwift.conf" ~/qz-config.conf
	# or
	cp ~/.config/Roberto\ Viola/qDomyos-Zwift.conf ~/qz-config.conf

	# Transfer qz-config.conf to the headless system (scp/sftp/usb)
	# On the headless system, copy into the intended service user's config directory
	TARGET_USER=pi  # or your service username
	sudo mkdir -p "/home/$TARGET_USER/.config/Roberto Viola"
	sudo cp ~/qz-config.conf "/home/$TARGET_USER/.config/Roberto Viola/qDomyos-Zwift.conf"
	sudo chown -R "$TARGET_USER:$TARGET_USER" "/home/$TARGET_USER/.config/Roberto Viola"
	```


**Option 2: Manual Configuration File Creation**
- Advanced users can create or edit the configuration file directly.

This approach ensures your headless system uses the same settings as a GUI-configured system.

---

## Success Indicators

- Watch displays stable pace and cadence
- QZ receives and broadcasts treadmill data  
- Distance accumulates accurately
- Service starts automatically on boot (if configured)

---


## Troubleshooting

### Quick Reference Checklist (First-Time Users)

If you run into problems, check these common issues first:

- **Is your ANT+ dongle plugged in and detected?**
	- Run `lsusb` and look for Garmin, Suunto, or Dynastream device.
- **Are you running commands with `sudo` when required?**
- **Did you run `./setup.sh --check` and follow all recommendations?**
- **Is your configuration file present and correct?**
	- See above for possible locations and transfer instructions.
- **Is your treadmill powered on and in Bluetooth pairing mode?**
- **Have you rebooted after changing USB permissions or user groups?**
- **Are you using the correct device flag for your equipment?**
	- Treadmills: add `-ant-footpod` flag. Bikes/rowers: omit this flag.

If you are still stuck, review the detailed troubleshooting table below or open an issue on GitHub with your log output.

### Quick System Check

Run the diagnostic dashboard anytime:
```bash
./setup-dashboard.sh
```

- ### Test ANT+ Without the Main App

Verify ANT+ works independently before troubleshooting the full application:

**Option 1: Dashboard (recommended):**

Run `sudo ./setup-dashboard.sh` and select "Test ANT+ Hardware" from the menu.

**Option 2: Direct test script:**

```bash
sudo ~/ant_venv/bin/python3 ./test_ant.py
```

Both simulate a running treadmill. Your watch should show:
- Pace: ~7:00 min/km (varying)
- Cadence: ~166 SPM
- Distance accumulating

**Test works but app doesn't?** This usually means the treadmill Bluetooth pairing or configuration needs adjustment, not the ANT+ setup.

**Test fails?** Check USB permissions, dongle connection, or Python environment. Most problems can be resolved by reviewing the troubleshooting table below.

Press `Ctrl+C` to stop.

---

### Common Issues

| Problem | Solution |
|---------|----------|
| `error while loading shared libraries: libpython3.11.so.1.0` | Python 3.11 missing. Run `./setup-dashboard.sh` to diagnose, then install via apt or pyenv |
| Wrapper shows dependency warnings | Follow provided instructions. Run `./setup-dashboard.sh` for diagnosis or run `sudo ./setup-dashboard.sh` for guided fixes |
| Test fails / watch won't pair | Ensure running with `sudo`. Reboot after USB permissions setup. Try unplug/replug dongle |
| Watch pairs but pace shows `--:--` | Treadmill not configured. Edit your configuration file (see above for possible locations) and add your model (e.g., `proform_treadmill_705_cst=true`) |
| App works but no watch connection | Unplug/replug dongle. Verify USB permissions + reboot. Check device ID (default 54321). Ensure running as root. Check logs |
| `systemctl stop qz` hangs | Add `KillSignal=SIGINT` to service file `[Service]` section |
| Binary won't run: "cannot execute binary file" | Wrong architecture downloaded. Get correct package: arm64 for Pi, x86-64 for desktop |
| `pyenv: command not found` | Reload shell: `source ~/.bashrc` or open new terminal |
| Setup validation fails multiple checks | Use the Interactive Setup: `sudo ./setup-dashboard.sh` for step-by-step fixes |
| Test script works but app doesn't connect to treadmill | Treadmill Bluetooth issue. Ensure treadmill is powered on and discoverable |

---

## Credits & Further Reading

**Project & Development:**
- Main project: https://github.com/cagnulein/qdomyos-zwift
- ANT+ Linux footpod implementation: bassai-sho
- Development assisted by AI analysis tools

**Research References:**

[Preferred transition speed between walking and running](https://pubmed.ncbi.nlm.nih.gov/16286854/)  
*Explains the biomechanical walk-run transition zone around 7.0-7.4 km/h*

[Biomechanics of Gait Transition](https://scholarworks.boisestate.edu/cgi/viewcontent.cgi?article=1178&context=td)  
*Detailed analysis of cadence patterns during gait transitions*