# ANT+ Virtual Footpod Broadcaster for Linux

This guide will help you configure your Linux system running QDomyos-Zwift (QZ) into an ANT+ bridge that broadcasts treadmill data as a virtual footpod, enabling Garmin watches and other ANT+ devices to display real-time data.

## What You'll Achieve
- Broadcast treadmill pace, distance, and estimated cadence in real-time
- Pair your Garmin watch or other ANT+ device as a Foot Pod
- Log accurate run data directly to your fitness ecosystem

## Prerequisites

**All users need:**
- ANT+ USB dongle (Garmin USB2/USB-m/USB3/mini, or Suunto compatible)
- Garmin watch or compatible ANT+ device
- Linux system running Debian/Ubuntu-based distribution (Bookworm or newer recommended)
- Familiarity with terminal commands, editing text files, and basic Linux navigation

**Supported Platforms:**
- **Raspberry Pi:** Zero 2 W, 3, 4, or 5 (ARM64)
- **Desktop Linux:** x86-64 PC/Laptop running Ubuntu, Debian, or derivatives

### Tested Configuration
- **Raspberry Pi:** Zero 2 W (Raspberry Pi OS Bookworm)
- **Desktop Linux:** Ubuntu 24.04 LTS (x86-64)
- **Treadmill:** Proform 705 CST
- **Watch:** Garmin Forerunner 245

### Backup Recommendation

Consider backing up your system before proceeding.

---

## Installation Methods

This guide covers installation using **pre-compiled binaries** from GitHub Releases. This is the recommended method for most users.

**For developers or advanced users:** If you need to compile from source (for development or custom modifications), see the [Compilation Guide](COMPILE.md).

---

## Installation Overview

**Step 1:** System Preparation - Install dependencies and configure environment

**Step 2:** Download Binary - Get the correct pre-compiled binary for your platform

**Step 3:** Run and Automate - Test and configure automatic startup

---

## Step 1: System Preparation

### 1.1 Install System Dependencies

This command installs all necessary Qt runtime libraries, USB libraries, and Python packages.

```bash
sudo apt-get update
sudo apt-get install -y \
	libqt5bluetooth5 \
	libqt5charts5 \
	libqt5multimedia5 \
	libqt5networkauth5 \
	libqt5positioning5 \
	libqt5sql5 \
	libqt5texttospeech5 \
	libqt5websockets5 \
	libqt5xml5 \
	libusb-1.0-0 \
	bluez \
	python3-pip
```

### 1.2 Install Python 3.11

The pre-compiled binaries require Python 3.11. Check if it's available:

```bash
python3.11 --version
```

**If Python 3.11 is available:**
```bash
sudo apt-get install -y python3.11 python3.11-venv
```

**If Python 3.11 is not available** (some distributions may only provide older or newer Python versions), install via pyenv:

```bash
# Install pyenv dependencies
sudo apt-get install -y \
	git \
	curl \
	build-essential \
	libssl-dev \
	zlib1g-dev \
	libbz2-dev \
	libreadline-dev \
	libsqlite3-dev \
	wget \
	llvm \
	libncurses5-dev \
	libncursesw5-dev \
	xz-utils \
	tk-dev \
	libffi-dev \
	liblzma-dev

# Note: On Ubuntu 24.04+, use libncurses-dev instead of libncurses5-dev/libncursesw5-dev
# sudo apt-get install -y libncurses-dev

# Install pyenv
curl https://pyenv.run | bash

# Add pyenv to shell
echo 'export PYENV_ROOT="$HOME/.pyenv"' >> ~/.bashrc
echo 'command -v pyenv >/dev/null || export PATH="$PYENV_ROOT/bin:$PATH"' >> ~/.bashrc
echo 'eval "$(pyenv init -)"' >> ~/.bashrc

# Reload shell
source ~/.bashrc

# Install Python 3.11
pyenv install 3.11.9
pyenv global 3.11.9

# Verify
python --version  # Should show Python 3.11.9
```

### 1.3 Create Python Virtual Environment

The QZ application looks for an environment named `ant_venv` in your home directory.

```bash
# Create the virtual environment
python3.11 -m venv ~/ant_venv

# Install required Python libraries (do not use sudo)
~/ant_venv/bin/pip install --upgrade pip
~/ant_venv/bin/pip install openant pyusb pybind11
```

### 1.4 Configure USB Permissions

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

# Reboot to apply changes
sudo reboot
```

### 1.5 Verify Setup

After rebooting, verify your environment is ready using the included setup tool:

**Quick Validation (No sudo required)**

```bash
cd ~/qdomyos-zwift-x86-64-ant  # or qdomyos-zwift-arm64-ant
./setup.sh --quick
```

This performs all prerequisite checks and shows results immediately. If all tests pass, you're ready to use ANT+!

**If Issues Found - Choose Your Approach:**

**Option A: Automatic Fix (Fastest)**

```bash
sudo ./setup.sh --fix
```

Automatically installs/configures fixable components (Qt5, libusb, USB permissions, etc.).

**Option B: Interactive Guided Setup (Recommended for first-time users)**

```bash
sudo ./setup.sh --interactive
```

Step-by-step wizard that explains each requirement and prompts before making changes. Best for learning what's needed and why.

---

## Step 2: Download and Install Binary

### 2.1 Download from GitHub Releases

1. Navigate to the **[Releases page](https://github.com/cagnulein/qdomyos-zwift/releases)**
2. Download the correct pre-compiled package for your platform:
   - **Raspberry Pi (ARM64):** `linux-binary-arm64-ant.zip` (contains `qdomyos-zwift-arm64-ant/`)
   - **Desktop Linux (x86-64):** `linux-binary-x86-64-ant.zip` (contains `qdomyos-zwift-x86-64-ant/`)

### 2.2 Install Binary

1. Extract the downloaded package to your home directory:

**For Raspberry Pi:**
```bash
cd ~
unzip linux-binary-arm64-ant.zip
cd qdomyos-zwift-arm64-ant
```

**For Desktop Linux:**
```bash
cd ~
unzip linux-binary-x86-64-ant.zip
cd qdomyos-zwift-x86-64-ant
```

**Note:** The package includes five files:
- `qdomyos-zwift` - Intelligent wrapper script that checks runtime dependencies
- `qdomyos-zwift-bin` - The actual application binary
- `setup.sh` - Setup and validation tool (use `./setup.sh --quick` to verify installation)
- `test_ant.py` - Standalone ANT+ test script (for troubleshooting)
- `ant_broadcaster.py` - ANT+ broadcasting module (required by test_ant.py)

The wrapper script automatically:
- Detects Python 3.11 library location (pyenv or system)
- Checks for ANT+ virtual environment and required packages
- Verifies USB permissions (plugdev group and udev rules)
- Provides helpful error messages with fix instructions

You should always run `./qdomyos-zwift` (the wrapper), not the binary directly.

---

## Step 3: Run and Automate

### 3.1 Pairing Your Device

Before running the application, understand how to pair your device:

**On Garmin watches:**
1. Menu > Sensors & Accessories > Add New > Foot Pod
2. Watch detects and pairs within a few seconds
3. Start "Treadmill" or "Run Indoor" activity
4. Add Cadence field: Settings > Activities & Apps > [Activity] > Data Screens > customize

**Cadence behavior:**
The system switches between walking and running cadence at 7.0 km/h (based on biomechanics research showing the walk-run transition occurs around 7.2-7.4 km/h):
- Below 7.0 km/h: Walking cadence (90-140 SPM)
- At/above 7.0 km/h: Running cadence (160-200 SPM)
- Example: 6.0 km/h walk ≈ 128 SPM, 8.5 km/h jog ≈ 166 SPM

See "Further reading" section for research details.

### 3.2 Test the Application

Connect your ANT+ dongle and start your treadmill. Your watch should pair as a Foot Pod.

```bash
# Basic usage (run from the extracted directory)
cd ~/qdomyos-zwift-arm64-ant  # or ~/qdomyos-zwift-x86-64-ant
sudo ./qdomyos-zwift -no-gui -ant-footpod

# Optional: Custom device ID (useful for conflicts, 1-65535)
sudo ./qdomyos-zwift -no-gui -ant-footpod -ant-device 12345

# Optional: Verbose logging (generates large logs - debug only)
sudo ./qdomyos-zwift -no-gui -ant-footpod -ant-verbose
```

**If you see warnings about missing dependencies**, the wrapper script will provide specific instructions on how to fix them. Common warnings:
- Python 3.11 library not found → Install via apt or pyenv (see Step 1.2)
- ANT+ virtual environment not found → Create ~/ant_venv (see Step 1.3)
- USB permissions issues → Check plugdev group and udev rules (see Step 1.4)

The application will still attempt to run with warnings, but ANT+ functionality may not work until dependencies are resolved.

### 3.3 Configure Automatic Startup (Optional)

Create a systemd service to start QZ automatically on boot:

```bash
sudo nano /etc/systemd/system/qz.service
```

Add the following configuration (adjust paths based on your platform):

**For Raspberry Pi:**
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

**For Desktop Linux:**
```ini
[Unit]
Description=qdomyos-zwift service
After=multi-user.target

[Service]
User=root
Group=plugdev
Environment="QZ_USER=%USERNAME%"
WorkingDirectory=/home/%USERNAME%/qdomyos-zwift-x86-64-ant
ExecStart=/home/%USERNAME%/qdomyos-zwift-x86-64-ant/qdomyos-zwift -no-gui -log -ant-footpod
KillSignal=SIGINT

[Install]
WantedBy=multi-user.target
```

*Replace `%USERNAME%` with your actual username.*

Apply changes:

```bash
sudo systemctl daemon-reload
sudo systemctl enable qz
sudo systemctl start qz
sudo systemctl status qz
```

### 3.4 Configuration File for Headless Systems (Optional)

When running headless with `-no-gui`, QZ creates a default configuration file at `/root/.config/Roberto Viola/qDomyos-Zwift.conf` on first run. However, this is unlikely to work with your setup. The best way to configure for your specific treadmill and to set other options is as follows:

1. **On a system with GUI** (can be a different device):
   - Download and extract the appropriate package for that system
   - Run QZ with GUI: `sudo ./qdomyos-zwift` (from the extracted directory)
   - Configure all your desired settings through the GUI interface
   - The settings are saved to `/root/.config/Roberto Viola/qDomyos-Zwift.conf`

2. **Copy to your headless system:**
   ```bash
   # On the GUI system, copy the config file
   sudo cp "/root/.config/Roberto Viola/qDomyos-Zwift.conf" ~/qz-config.conf
   
   # Transfer to your headless system, then:
   sudo mkdir -p "/root/.config/Roberto Viola"
   sudo cp ~/qz-config.conf "/root/.config/Roberto Viola/qDomyos-Zwift.conf"
   ```

This ensures your headless system has the exact configuration you set up through the GUI, including treadmill model settings, Bluetooth options, and any other preferences.

---

## Success Indicators

✓ Watch displays stable pace and cadence  
✓ QZ broadcasts data from your treadmill  
✓ Systemd service starts automatically on boot (if configured)

---

## Troubleshooting

### Quick Diagnostics

Run the validation tool to check your setup:
```bash
./setup.sh --quick
```

### Test ANT+ Independently

To test ANT+ functionality **without the QDomyos-Zwift binary**, use the included test script:

```bash
cd ~/qdomyos-zwift-x86-64-ant  # or qdomyos-zwift-arm64-ant
sudo ~/ant_venv/bin/python3 ./test_ant.py
```

This simulates a running treadmill broadcasting data. Your watch should pair within 5-10 seconds and show:
- Pace: ~7:00 min/km (varying)
- Cadence: ~166 SPM
- Distance accumulating

**If this works but the main app doesn't**, the issue is with treadmill Bluetooth pairing or configuration, not ANT+.

**If this fails**, check:
- ANT+ dongle is plugged in
- USB permissions are correct (`./setup.sh --quick`)
- Python environment is set up (`~/ant_venv` exists with packages)

Press `Ctrl+C` to stop the test.

---

### Common Issues

| Issue | Solution |
| --- | --- |
| `error while loading shared libraries: libpython3.11.so.1.0` | Python 3.11 not installed - run `./setup.sh --quick` to diagnose. Install via apt or pyenv (Step 1.2) |
| Wrapper shows missing dependency warnings | Follow the specific instructions provided. Run `./setup.sh --quick` for full diagnosis or `sudo ./setup.sh --fix` to auto-fix |
| Test fails or watch won't connect | Run with sudo, reboot after Step 1.4, or unplug/replug dongle |
| Watch connects but pace shows --:-- | Treadmill model not set: `sudo nano /root/.config/Roberto\ Viola/qDomyos-Zwift.conf` add your model flag (e.g., `proform_treadmill_705_cst=true`) |
| App works but watch won't connect | Unplug/replug dongle, verify Step 1.4 + reboot, check device ID (default 54321), ensure running as root, check logs |
| `systemctl stop qz` hangs | Add `KillSignal=SIGINT` to [Service] section in qz.service |
| Binary won't run: "cannot execute binary file" | Wrong architecture - ensure you downloaded the correct package for your platform (arm64 vs x86-64) |
| `pyenv: command not found` | Reload shell: `source ~/.bashrc` or start new terminal session |
| Setup validation fails | Use interactive setup: `sudo ./setup.sh --interactive` for step-by-step guidance |
| Test script works but app doesn't | Treadmill Bluetooth pairing issue - check treadmill is on and discoverable |

---

## Credits & Acknowledgments

- Main project: https://github.com/cagnulein/qdomyos-zwift
- ANT+ Linux footpod implementation: bassai-sho
- Development assisted by AI analysis tools

**Further reading:**

[Preferred transition speed between walking and running](https://pubmed.ncbi.nlm.nih.gov/16286854/)  
[Biomechanics of Gait Transition](https://scholarworks.boisestate.edu/cgi/viewcontent.cgi?article=1178&context=td)