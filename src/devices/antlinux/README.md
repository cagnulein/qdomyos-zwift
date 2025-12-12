# ANT+ Virtual Footpod Broadcaster for Linux

Transform your Linux system into an ANT+ bridge that broadcasts treadmill data as a virtual footpod. Your Garmin watch or other ANT+ device will display real-time pace, distance, and cadence—just like running outdoors.

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

> **Note:** Consider backing up your system before proceeding.

---

## Quick Start Guide

This guide uses **pre-compiled binaries** from GitHub Releases—the easiest method for most users.

> **Developers:** Need to compile from source? See the [Compilation Guide](COMPILE.md).

### Installation Steps

1. **Download Binary** - Get the pre-compiled package for your platform
2. **Check System** - Verify what dependencies you need
3. **Install Dependencies** - Set up required libraries automatically
4. **Run & Automate** - Test and configure automatic startup

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
- `setup.sh` - Automated setup and validation tool
- `test_ant.py` - Standalone ANT+ test for troubleshooting
- `ant_broadcaster.py` - ANT+ module used by test script

---

## Step 2: Check Your System

Before installing anything, see what's needed:

```bash
./setup.sh --check
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

**Issues found?** Continue to Step 3 for automatic fixes.

---

## Step 3: Install Dependencies

### Automatic Setup (Recommended)

Let the wizard guide you through installation:

**For GUI/Desktop systems** (no systemd service):
```bash
sudo ./setup.sh --gui
```

**For headless/server systems** (with systemd service):
```bash
sudo ./setup.sh --headless
```

The guided setup explains each step and asks for confirmation before making changes.

**For automation/scripting** (unattended installation):
```bash
yes | sudo ./setup.sh --gui      # or --headless
```

### Other Useful Commands

**Test ANT+ independently:**
```bash
sudo ./setup.sh --test
```
Simulates treadmill data to verify ANT+ works before testing the full app. Your watch should pair within 5-10 seconds showing pace (~7:00 min/km) and cadence (~166 SPM).

**Reset configuration:**
```bash
sudo ./setup.sh --reset
```
Removes user from plugdev group and clears udev rules. On desktop systems, system packages (Qt5, libusb) are protected from removal. On headless systems, you'll be prompted to remove Python packages.

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

<details>
<summary>Click to expand pyenv installation steps</summary>

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
</details>

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
./setup.sh --check
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

### Headless Configuration (Optional)

Running without a GUI? You'll need to create a configuration file with your treadmill settings.

**Best approach - configure via GUI first:**

Running the app for the first time will automatically create the configuration file under the effective user's home:
- If the app is started as root (eg. using sudo or run directly as root) the config is created at:
  /root/.config/Roberto Viola/qDomyos-Zwift.conf
- If the app is started via the systemd service and you set Environment="QZ_USER=YOUR_USER" in the service file, the wrapper looks for and uses:
/home/YOUR_USER/.config/Roberto Viola/qDomyos-Zwift.conf

1. **On a system with display** (can be different hardware):
   - Run: `sudo ./qdomyos-zwift`
   - Configure your treadmill model and preferences
   - Settings save to `/root/.config/Roberto Viola/qDomyos-Zwift.conf`

2. **Copy to your headless system:**
   ```bash
   # On the GUI system, export the config to a transfer file
   sudo cp "/root/.config/Roberto Viola/qDomyos-Zwift.conf" ~/qz-config.conf

   # Transfer qz-config.conf to the headless system (scp/sftp/usb)
   # On the headless system, copy into the intended service user's config.
   # Replace TARGET_USER with the username used by the service (QZ_USER in systemd).
   TARGET_USER=pi

   sudo mkdir -p "/home/$TARGET_USER/.config/Roberto Viola"
   sudo cp ~/qz-config.conf "/home/$TARGET_USER/.config/Roberto Viola/qDomyos-Zwift.conf"
   sudo chown -R "$TARGET_USER:$TARGET_USER" "/home/$TARGET_USER/.config/Roberto Viola"
   ```

This ensures your headless system has the exact GUI-configured settings.

---

## Success Indicators

- Watch displays stable pace and cadence
- QZ receives and broadcasts treadmill data  
- Distance accumulates accurately
- Service starts automatically on boot (if configured)

---

## Troubleshooting

### Quick System Check

Run the diagnostic tool anytime:
```bash
./setup.sh --check
```

### Test ANT+ Without the Main App

Verify ANT+ works independently before troubleshooting the full application:

**Option 1: Setup tool (recommended):**
```bash
sudo ./setup.sh --test
```

**Option 2: Direct test script:**
```bash
sudo ~/ant_venv/bin/python3 ./test_ant.py
```

Both simulate a running treadmill. Your watch should show:
- Pace: ~7:00 min/km (varying)
- Cadence: ~166 SPM
- Distance accumulating

**Test works but app doesn't?** Issue is with treadmill Bluetooth pairing/configuration, not ANT+.

**Test fails?** Check USB permissions, dongle connection, or Python environment.

Press `Ctrl+C` to stop.

---

### Common Issues

| Problem | Solution |
|---------|----------|
| `error while loading shared libraries: libpython3.11.so.1.0` | Python 3.11 missing. Run `./setup.sh --check` to diagnose, then install via apt or pyenv |
| Wrapper shows dependency warnings | Follow provided instructions. Run `./setup.sh --check` for diagnosis or `sudo ./setup.sh --gui` for automatic fix |
| Test fails / watch won't pair | Ensure running with `sudo`. Reboot after USB permissions setup. Try unplug/replug dongle |
| Watch pairs but pace shows `--:--` | Treadmill not configured. Edit `/root/.config/Roberto\ Viola/qDomyos-Zwift.conf` and add your model (e.g., `proform_treadmill_705_cst=true`) |
| App works but no watch connection | Unplug/replug dongle. Verify USB permissions + reboot. Check device ID (default 54321). Ensure running as root. Check logs |
| `systemctl stop qz` hangs | Add `KillSignal=SIGINT` to service file `[Service]` section |
| Binary won't run: "cannot execute binary file" | Wrong architecture downloaded. Get correct package: arm64 for Pi, x86-64 for desktop |
| `pyenv: command not found` | Reload shell: `source ~/.bashrc` or open new terminal |
| Setup validation fails multiple checks | Use guided setup: `sudo ./setup.sh --gui` (or `--headless`) for step-by-step fixes |
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