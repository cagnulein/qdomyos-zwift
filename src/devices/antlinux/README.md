# ANT+ Virtual Footpod Broadcaster for Linux

Transform your Linux system into an ANT+ bridge that broadcasts **treadmill** data as a virtual footpod. Your Garmin watch or other ANT+ device will display real-time pace and cadence, just like running outdoors.

---

## 🚀 Quick Start (Recommended)

**Most users follow this simple path:**

1. **Download** the pre-built package for your system
2. **Run** `sudo ./setup-dashboard.sh` 
3. **Follow** the interactive prompts - the dashboard handles everything
4. **Test** with your watch

**Total time:** 15-30 minutes including installations

**Already know what you're doing?** Jump to [Installation](#installation)

---

## What You'll Achieve

- ✓ Broadcast treadmill pace, distance, and cadence in real-time
- ✓ Pair your Garmin watch as a Foot Pod for accurate indoor run tracking
- ✓ Automatically log runs to your fitness ecosystem
- ✓ No manual data entry needed

---

## Prerequisites

### What You'll Need

**Hardware (Required):**
- ✓ ANT+ USB dongle (Garmin USB2/USB-m/USB3/mini, or Suunto)
- ✓ Garmin watch or compatible ANT+ device
- ✓ Raspberry Pi (Zero 2 W, 3, 4, 5) **OR** x86-64 PC/Laptop

**Software (Recommended):**
- ✓ Debian/Ubuntu-based Linux (Debian Bookworm or Ubuntu 22.04+)
- ✓ Basic terminal skills (navigating directories, running commands)

**Don't have Python 3.11?** No problem - the setup dashboard installs it automatically.

---

### Verified Working Setups

This guide is tested on:
- **Raspberry Pi:** Zero 2 W (Raspberry Pi OS Bookworm)
- **Desktop:** Ubuntu 24.04 LTS (x86-64)
- **Treadmill:** Proform 705 CST
- **Watch:** Garmin Forerunner 245

**Using different hardware?** It should still work - these are tested examples.

---

### Device Type Guidance

**This guide focuses on treadmills.** The ANT+ footpod broadcasting feature is designed specifically for treadmill users.

**Using bikes/ellipticals/rowers?** You can still use the main application - just omit the `-ant-footpod` flag from all commands.

> **Tip:** If using this on your main desktop, back up your system first.

---

## Installation

### Step 1: Download and Extract

**Progress:** Step 1 of 4

1. Visit the **[Releases page](https://github.com/cagnulein/qdomyos-zwift/releases)**

2. Download the correct package:
   - **Desktop Linux:** `linux-binary-x86-64-ant.zip` 
   - **Raspberry Pi:** `linux-binary-arm64-ant.zip` 

3. Extract to your home directory:

```bash
cd ~
ARCH=$(uname -m | sed -e 's/aarch64/arm64/' -e 's/x86_64/x86-64/')
unzip linux-binary-$ARCH-ant.zip
cd qdomyos-zwift-$ARCH-ant
```

**What you get:**
- `qdomyos-zwift` - Main application wrapper (always use this)
- `qdomyos-zwift-bin` - Application binary (don't run directly)
- `setup-dashboard.sh` - Interactive setup and configuration dashboard
- `test_ant.py` - ANT+ hardware test
- `devices.ini` - Treadmill device mappings
- `devices_optimized.json` - Device menu data
- `ant_broadcaster.py` - ANT+ module

---

✓ **Done!** Package extracted.

**Next:** Run the setup dashboard → [Step 2](#step-2-run-setup-dashboard)

---

### Step 2: Run Setup Dashboard

**Progress:** Step 2 of 4

**One command handles everything:**

```bash
sudo ./setup-dashboard.sh
```

**What it does automatically:**
1. ✓ Checks what's installed (Python 3.11, Qt5, libraries)
2. ✓ Shows what's needed in clear status display
3. ✓ Guides installations step-by-step
4. ✓ Configures USB permissions and udev rules
5. ✓ Sets up virtual environment and packages
6. ✓ Validates everything works

**Expected time:** 5-30 minutes (depending on what needs installing)

**The dashboard is interactive:**
- Explains each step clearly
- Asks for confirmation before making changes
- Shows real-time progress
- Tests components as they're installed
- Provides specific error messages if issues occur

**What gets installed (if needed):**
- Python 3.11 (via apt or pyenv automatically)
- Qt5 libraries and QML modules
- Virtual environment with ANT+ packages
- USB permissions (plugdev group, udev rules)
- Bluetooth service configuration

---

**Advanced users wanting full control:** See [Appendix A: Manual Installation](#appendix-a-manual-installation)

---

✓ **Done when:** Dashboard shows "All checks passed"

**Next:** Configure your treadmill → [Step 3](#step-3-configure-your-treadmill)

**Having issues?** The dashboard provides specific guidance for each problem.

---

### Step 3: Configure Your Treadmill

**Progress:** Step 3 of 4

You need to configure:
1. Your profile (age, weight, units)
2. Your treadmill model
3. Bluetooth pairing with treadmill

Choose the method that fits your setup:

---

#### Option A: Desktop/Laptop (With Display)

**Easiest - just run the GUI:**

```bash
sudo ./qdomyos-zwift -ant-footpod
```

The GUI opens where you can:
- Set your profile details (Settings → User)
- Select your treadmill model (Settings → Devices)
- Pair via Bluetooth (connects automatically when treadmill is on)

Settings save automatically. Done!

---

#### Option B: Headless System (Raspberry Pi/Server)

**Use the setup dashboard:**

```bash
sudo ./setup-dashboard.sh
```

**Configure step-by-step:**

**1. User Profile:**
- Select "User Profile" from menu
- Enter age, weight, height
- Choose units (metric/imperial)
- Changes save automatically

**2. Treadmill Model:**
- Select "Equipment" from menu
- Browse by manufacturer or search
- Select your specific model
- Configuration saved

**3. Bluetooth Pairing:**
- Power on your treadmill
- Put it in pairing/discoverable mode (check treadmill manual)
- Select "Bluetooth Scan" from dashboard menu
- Choose your treadmill from the list
- Pairing saves - you won't need to pair again

---

**Alternative method:** Configure on another computer with a display, then transfer the config file. See [Appendix B: Transfer Config File](#appendix-b-transfer-config-file)

---

✓ **Done when:** Profile configured, treadmill model selected, Bluetooth paired

**Next:** Test everything works → [Step 4](#step-4-test-it-works)

---

### Step 4: Test It Works

**Progress:** Step 4 of 4

#### Test 1: ANT+ Hardware Test

**Verify ANT+ dongle and watch pairing work:**

```bash
sudo ./setup-dashboard.sh
```

Select "ANT+ Test" from the menu.

**What happens:**
- Simulates a treadmill running at different speeds
- Your watch should:
  - Pair as a foot pod within 5-10 seconds
  - Show pace updating in real-time
  - Show cadence changing (walk vs run)
  - Display distance accumulating

**This confirms:** ANT+ dongle → watch communication works ✓

If test fails, the dashboard shows specific troubleshooting steps.

---

#### Test 2: Full Application Test

**Test with your actual treadmill:**

1. **Make sure you're in the install directory:**
   ```bash
   cd ~/qdomyos-zwift-*-ant
   ```

2. **Power on your treadmill and ensure Bluetooth is connected**

3. **Run the application:**
   ```bash
   sudo ./qdomyos-zwift -no-gui -ant-footpod
   ```

4. **Start walking/running on your treadmill**

**Your watch should show:**
- Pace matching treadmill speed
- Cadence matching your steps
- Distance accumulating
- Activity can be saved to your fitness platform

**Success!** Your setup is working.

---

**Optional command flags:**
```bash
# Custom ANT+ device ID (useful if you have multiple devices)
sudo ./qdomyos-zwift -no-gui -ant-footpod -ant-device 12345

# Verbose logging (creates large logs, use only for debugging)
sudo ./qdomyos-zwift -no-gui -ant-footpod -ant-verbose
```

---

**See errors or warnings?**
1. Read the error message - it provides specific guidance
2. Run `sudo ./setup-dashboard.sh` to diagnose and fix
3. Test again

The application won't work properly until all dependencies are resolved.

---

✓ **Done!** Everything is working.

**Optional next steps:**
- [Set up auto-start on boot](#optional-auto-start-on-boot)
- [Pair your watch (first time)](#pairing-your-watch)
- [Learn about command options](#advanced-command-options)

---

## Using Your ANT+ Footpod

### Pairing Your Watch (First Time Only)

**On your Garmin watch:**
1. Go to: Menu → Sensors & Accessories → Add New → Foot Pod
2. Watch searches and finds device automatically (5-10 seconds)
3. Device shows as "Foot Pod" with ID number
4. Select to pair

**That's it!** The watch remembers the pairing.

**Note:** Make sure the application is running (`sudo ./qdomyos-zwift -no-gui -ant-footpod`) when pairing.

---

### Starting an Activity

**Each time you run:**
1. Start the application (if not auto-starting)
2. Power on your treadmill
3. Start a "Treadmill" or "Run Indoor" activity on your watch
4. Watch connects to foot pod automatically
5. Begin your workout - data appears in real-time

**Tip:** Add cadence to your data screens:
- Settings → Activities & Apps → [Your Activity] → Data Screens
- Customize → Add "Cadence" field

---

### Understanding Cadence

The system calculates cadence based on biomechanics research. It automatically switches between walking and running cadence at **7.0 km/h** (the walk-run transition zone):

| Speed | Cadence Type | Typical Range | Example |
|-------|--------------|---------------|---------|
| < 7.0 km/h | Walking | 90-140 SPM | 6.0 km/h ≈ 128 SPM |
| ≥ 7.0 km/h | Running | 160-200 SPM | 8.5 km/h ≈ 166 SPM |

The transition at 7.0 km/h matches the natural walk-run transition zone identified in biomechanics research.

> See [Further Reading](#credits--further-reading) for research references.

---

### Optional: Auto-Start on Boot

Want the application to start automatically when your system boots?

**Easiest method - use the dashboard:**

```bash
sudo ./setup-dashboard.sh
```

Select "Service Configuration" → "Generate Service"

The dashboard:
- Creates the systemd service file at the correct location
- Configures it for your system (user, paths, flags)
- Enables auto-start on boot
- Starts the service immediately

**Check it's running:**
```bash
sudo systemctl status qz
```

**Manage the service:**
```bash
sudo systemctl start qz    # Start manually
sudo systemctl stop qz     # Stop
sudo systemctl restart qz  # Restart
sudo systemctl disable qz  # Disable auto-start
```

**View logs:**
```bash
journalctl -u qz -f        # Live log view
journalctl -u qz -n 50     # Last 50 lines
```

---

**Manual setup:** See [Appendix C: Manual Service Configuration](#appendix-c-manual-service-configuration)

---

## Troubleshooting

### Quick Diagnostic

**First step for any issue:**

```bash
sudo ./setup-dashboard.sh
```

The dashboard:
- Shows current system status with clear pass/fail indicators
- Identifies missing or misconfigured dependencies
- Provides specific guidance for each issue
- Can re-run installations or tests

---

### Common Issues

#### ANT+ Dongle Not Detected

**Check if dongle is recognized:**
```bash
lsusb
```

Look for: Garmin, Suunto, or Dynastream device

**If not found:**
- [ ] Unplug and replug dongle
- [ ] Try a different USB port
- [ ] Reboot system
- [ ] Check dongle LED (should light up)
- [ ] Try dongle on another computer to verify it works

**If found but still not working:**
- [ ] Check USB permissions in dashboard
- [ ] Verify you're in plugdev group: `groups`
- [ ] Reboot after permission changes
- [ ] Check udev rules: `ls -la /etc/udev/rules.d/99-ant*`

---

#### Watch Won't Pair

**Checklist:**
- [ ] Running command with `sudo`?
- [ ] ANT+ dongle plugged in and recognized?
- [ ] Watch in "Add Sensor" mode?
- [ ] Watch within 3 meters of dongle?
- [ ] Application is running?

**Try these steps:**
1. Run ANT+ test from dashboard to verify hardware
2. Unplug and replug dongle
3. Restart the application
4. Delete any existing foot pod from watch and re-pair
5. Check logs for errors: `journalctl -u qz -n 50`

---

#### Treadmill Not Connecting via Bluetooth

**Checklist:**
- [ ] Treadmill powered on?
- [ ] Treadmill in pairing/discoverable mode?
- [ ] Bluetooth enabled on system: `sudo systemctl status bluetooth`
- [ ] Within 10 meters of treadmill?
- [ ] No other devices connected to treadmill?

**Try these steps:**
1. Dashboard → "Bluetooth Scan" → Rescan for devices
2. Restart Bluetooth service: `sudo systemctl restart bluetooth`
3. Check treadmill manual for pairing mode instructions
4. Try GUI pairing: `sudo ./qdomyos-zwift -ant-footpod`
5. Forget device and re-pair

---

#### Watch Pairs But Shows No Data (`--:--`)

**Most common cause:** Treadmill not configured or not connected

**Solutions:**

**1. Verify treadmill model is configured:**
```bash
sudo ./setup-dashboard.sh
```
Select "Equipment" → Choose your treadmill model

**2. Check treadmill is connected:**
- Verify Bluetooth connection established
- Look for "Connected" message in application output
- Try reconnecting via dashboard Bluetooth scan

**3. Verify configuration file:**
```bash
# Check if config exists
ls -la ~/.config/"Roberto Viola"/qDomyos-Zwift.conf
# or for root
sudo ls -la /root/.config/"Roberto Viola"/qDomyos-Zwift.conf
```

---

#### Python Library Error

**Error message:**
```
error while loading shared libraries: libpython3.11.so.1.0
```

**This means:** Python 3.11 shared library not found

**Solution:**
```bash
sudo ./setup-dashboard.sh
```

The dashboard will:
- Detect the missing library
- Show installation options (apt or pyenv)
- Guide you through installation
- Verify the library is found

**Manual check:**
```bash
ldconfig -p | grep libpython3.11
```

Should show the library location.

---

#### Service Won't Stop or Restart

**Symptom:** `systemctl stop qz` hangs or takes >90 seconds

**Cause:** Service file missing proper signal handling

**Solution:** Edit service file:
```bash
sudo nano /etc/systemd/system/qz.service
# or on some systems:
sudo nano /lib/systemd/system/qz.service
```

Add under `[Service]` section:
```ini
KillSignal=SIGINT
SuccessExitStatus=130
```

Then reload and test:
```bash
sudo systemctl daemon-reload
sudo systemctl restart qz
```

---

#### Commands Not Working (Permission Denied)

**Common mistakes:**
- Forgetting `sudo` before commands
- Running as wrong user

**Solution:** Always use `sudo` for:
- `sudo ./setup-dashboard.sh`
- `sudo ./qdomyos-zwift -no-gui -ant-footpod`
- `sudo ~/ant_venv/bin/python3 ./test_ant.py`

---

#### Wrong Package Downloaded

**Symptom:** `cannot execute binary file: Exec format error`

**Cause:** Wrong architecture package

**Solution:** Download correct package:
- **Raspberry Pi:** `linux-binary-arm64-ant.zip`
- **Desktop Linux:** `linux-binary-x86-64-ant.zip`

**Check your architecture:**
```bash
uname -m
# x86_64 or amd64 → Download x86-64 package
# aarch64 or arm64 → Download arm64 package
```

---

## Advanced Command Options

The wrapper script (`qdomyos-zwift`) accepts various flags:

### ANT+ Options
```bash
-ant-footpod              # Enable ANT+ footpod broadcasting (required for treadmills)
-ant-device 12345         # Custom ANT+ device ID (1-65535, default: 54321)
-ant-verbose              # Verbose ANT+ logging (creates large logs)
```

### Display Options
```bash
-no-gui                   # Run without GUI (headless mode)
-no-log                   # Disable logging to file
-no-console               # Suppress console output
```

### Device Options
```bash
-poll-device-time 200     # Bluetooth polling interval (milliseconds, default: 200)
-name "My Device"         # Custom device name
```

### Example Combinations
```bash
# Headless with logging
sudo ./qdomyos-zwift -no-gui -ant-footpod -log

# Headless without logging (quieter)
sudo ./qdomyos-zwift -no-gui -no-log -ant-footpod

# Custom device ID with verbose logging (debugging)
sudo ./qdomyos-zwift -no-gui -ant-footpod -ant-device 11111 -ant-verbose
```

---

## Appendices

### Appendix A: Manual Installation

For advanced users who want full control instead of using the setup dashboard.

#### System Dependencies

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

#### Python 3.11 Installation

**The pre-compiled binaries require Python 3.11 specifically** (not 3.10, not 3.12+).

**Check if available:**
```bash
python3.11 --version
```

**Option 1: Install via apt (if available)**
```bash
sudo apt-get install -y python3.11 python3.11-venv libpython3.11
```

**Option 2: Install via pyenv (if Python 3.11 not available)**

Choose the method for your system:

##### Standard Systems (2GB+ RAM)

**Install pyenv dependencies:**
```bash
sudo apt-get update
sudo apt-get install -y \
    git curl build-essential libssl-dev zlib1g-dev \
    libbz2-dev libreadline-dev libsqlite3-dev wget \
    llvm libncurses5-dev libncursesw5-dev xz-utils \
    tk-dev libffi-dev liblzma-dev

# Note: Ubuntu 24.04+ should use libncurses-dev instead of libncurses5-dev
```

**Install pyenv:**
```bash
curl https://pyenv.run | bash

# Add to shell config (if not already present)
if ! grep -q "PYENV_ROOT" ~/.bashrc; then
    cat >> ~/.bashrc << 'EOF'

# pyenv configuration
export PYENV_ROOT="$HOME/.pyenv"
export PATH="$PYENV_ROOT/bin:$PATH"
eval "$(pyenv init -)"
EOF
fi

source ~/.bashrc
```

**Install Python 3.11:**
```bash
pyenv install 3.11.9
pyenv global 3.11.9
python --version  # Should show Python 3.11.9
```

##### Low-Memory Systems (512MB-1GB RAM)

**For Pi Zero, Pi Zero 2 W, Pi 3:**

**Enable zram swap:**
```bash
sudo apt-get install -y zram-tools
echo -e 'ALGO=lz4\nPERCENT=50\nPRIORITY=100' | sudo tee /etc/default/zramswap
sudo systemctl restart zramswap
swapon -s | grep zram  # Verify
```

**Install build dependencies:**
```bash
sudo apt-get update
sudo apt-get install -y \
    git curl build-essential libssl-dev zlib1g-dev \
    libbz2-dev libreadline-dev libsqlite3-dev wget \
    llvm libncurses5-dev libncursesw5-dev xz-utils \
    tk-dev libffi-dev liblzma-dev ccache
```

**Install and configure pyenv:**
```bash
curl https://pyenv.run | bash

if ! grep -q "PYENV_ROOT" ~/.bashrc; then
    cat >> ~/.bashrc << 'EOF'

# pyenv configuration
export PYENV_ROOT="$HOME/.pyenv"
export PATH="$PYENV_ROOT/bin:$PATH"
eval "$(pyenv init -)"
EOF
fi

source ~/.bashrc
```

**Build Python 3.11 with optimization:**
```bash
export PYTHON_CONFIGURE_OPTS="--enable-shared --enable-optimizations"
export PYTHON_CFLAGS="-march=native -O2"
export MAKE_OPTS="-j1"

# Takes 60-90 minutes on Pi Zero 2 W
pyenv install 3.11.9
pyenv global 3.11.9
python --version
```

#### Virtual Environment and Packages

```bash
cd ~/qdomyos-zwift-*-ant
python3 -m venv ant_venv
source ant_venv/bin/activate
pip install --upgrade pip
pip install openant usb pybind11 bleak
deactivate
```

#### USB Permissions

```bash
# Add user to plugdev group
sudo usermod -aG plugdev $USER

# Create udev rules
sudo tee /etc/udev/rules.d/99-ant-usb.rules > /dev/null << 'EOF'
# Garmin ANT+ USB Sticks
SUBSYSTEM=="usb", ATTRS{idVendor}=="0fcf", ATTRS{idProduct}=="1008", MODE="0666", GROUP="plugdev"
SUBSYSTEM=="usb", ATTRS{idVendor}=="0fcf", ATTRS{idProduct}=="1009", MODE="0666", GROUP="plugdev"
# Suunto ANT+ USB
SUBSYSTEM=="usb", ATTRS{idVendor}=="0fcf", ATTRS{idProduct}=="0004", MODE="0666", GROUP="plugdev"
# Generic Dynastream
SUBSYSTEM=="usb", ATTRS{idVendor}=="0fcf", ATTRS{idProduct}=="000a", MODE="0666", GROUP="plugdev"
SUBSYSTEM=="usb", ATTRS{idVendor}=="0fcf", ATTRS{idProduct}=="81a9", MODE="0666", GROUP="plugdev"
# N.B. ANT+ Engineering board
SUBSYSTEM=="usb", ATTRS{idVendor}=="11fd", ATTRS{idProduct}=="0001", MODE="0666", GROUP="plugdev"
EOF

# Apply changes
sudo udevadm control --reload-rules && sudo udevadm trigger
```

**Reboot** for group membership to take effect.

#### Enable Bluetooth

```bash
sudo systemctl start bluetooth
sudo systemctl enable bluetooth
```

#### Verify Installation

```bash
cd ~/qdomyos-zwift-*-ant
sudo ./setup-dashboard.sh
```

All checks should pass.

---

### Appendix B: Transfer Config File

If you have access to another computer with a display, you can configure there and transfer the config file.

#### On System With Display

1. **Run GUI and configure:**
   ```bash
   sudo ./qdomyos-zwift -ant-footpod
   ```

2. **Configure all settings:**
   - User profile (age, weight, units)
   - Treadmill model
   - Any other preferences

3. **Copy config file:**
   ```bash
   # If you ran without sudo:
   cp ~/.config/"Roberto Viola"/qDomyos-Zwift.conf ~/qz-config.conf
   
   # If you ran with sudo:
   sudo cp /root/.config/"Roberto Viola"/qDomyos-Zwift.conf ~/qz-config.conf
   ```

#### Transfer to Headless System

**Using scp (from display system):**
```bash
scp ~/qz-config.conf user@headless-pi:~/
```

**Or using USB drive:**
```bash
cp ~/qz-config.conf /media/usb/
# Transfer USB to headless system
```

#### On Headless System

```bash
# Create config directory
mkdir -p ~/.config/"Roberto Viola"

# Copy config file
cp ~/qz-config.conf ~/.config/"Roberto Viola"/qDomyos-Zwift.conf

# If you'll run with sudo, also copy for root:
sudo mkdir -p /root/.config/"Roberto Viola"
sudo cp ~/qz-config.conf /root/.config/"Roberto Viola"/qDomyos-Zwift.conf
```

Done! Your headless system now has the same configuration.

---

### Appendix C: Manual Service Configuration

For users who want to manually create the systemd service instead of using the dashboard.

#### Platform-Specific Paths

| Platform | Service Location | Default Username | Install Path |
|----------|-----------------|------------------|--------------|
| Raspberry Pi | `/etc/systemd/system/qz.service` | `pi` | `/home/pi/qdomyos-zwift-arm64-ant` |
| Desktop Linux | `/lib/systemd/system/qz.service` | YOUR_USERNAME | `/home/YOUR_USERNAME/qdomyos-zwift-x86-64-ant` |

#### Create Service File

```bash
sudo nano /etc/systemd/system/qz.service  # or /lib/systemd/system/qz.service
```

**Service file content:**
```ini
[Unit]
Description=qdomyos-zwift service
After=multi-user.target

[Service]
User=root
Group=plugdev
Environment="QZ_USER=YOUR_USERNAME"
WorkingDirectory=/path/to/qdomyos-zwift-ARCH-ant
Environment="LD_LIBRARY_PATH=/usr/local/lib:/usr/lib"
ExecStart=/bin/bash -c '/path/to/qdomyos-zwift-ARCH-ant/qdomyos-zwift -no-gui -no-log -no-console -ant-footpod -ant-device 54321 -poll-device-time 200 -name YOUR_BLE_NAME'
KillSignal=SIGINT
SuccessExitStatus=130
TimeoutStopSec=10

[Install]
WantedBy=multi-user.target
```

**Replace:**
- `YOUR_USERNAME` with your actual username (e.g., `pi` or your user)
- `/path/to/qdomyos-zwift-ARCH-ant` with full path to install directory
- `YOUR_BLE_NAME` with your treadmill's Bluetooth name (e.g., `I_TL`)
- `-ant-device 54321` with your preferred ANT+ device ID (any 5-digit number)

> **Note on PYTHONHOME:** The `qdomyos-zwift` wrapper script automatically determines the correct `PYTHONHOME` at runtime by querying `sys.base_prefix` from your Python installation. You do not need to set it manually in the service file. If you ever see `failed to get the Python codec of the filesystem encoding`, regenerate the service via the setup dashboard which will include the correct `PYTHONHOME` as a fallback hint.

> **pyenv users:** Add `Environment="LD_LIBRARY_PATH=/home/YOUR_USERNAME/.pyenv/versions/3.11.9/lib:/usr/local/lib:/usr/lib"` — the wrapper detects pyenv automatically.

**Example for Raspberry Pi (system Python):**
```ini
[Unit]
Description=qdomyos-zwift service
After=multi-user.target

[Service]
User=root
Group=plugdev
Environment="QZ_USER=pi"
WorkingDirectory=/home/pi/qdomyos-zwift-arm64-ant
Environment="LD_LIBRARY_PATH=/usr/local/lib:/usr/lib:/usr/lib/aarch64-linux-gnu"
ExecStart=/bin/bash -c '/home/pi/qdomyos-zwift-arm64-ant/qdomyos-zwift -no-gui -no-log -no-console -ant-footpod -ant-device 54321 -poll-device-time 200 -name I_TL'
KillSignal=SIGINT
SuccessExitStatus=130
TimeoutStopSec=10

[Install]
WantedBy=multi-user.target
```

#### Enable and Start

```bash
sudo systemctl daemon-reload
sudo systemctl enable qz
sudo systemctl start qz
sudo systemctl status qz
```

---

## Credits & Further Reading

### Project & Development

- **Main project:** [github.com/cagnulein/qdomyos-zwift](https://github.com/cagnulein/qdomyos-zwift)
- **ANT+ Linux footpod implementation:** bassai-sho
- **Setup dashboard and documentation:** Community contributors

### Technology

- **ANT+** is a wireless protocol managed by Garmin Canada Inc.
- This is an independent open-source implementation for interoperability
- Uses the openant Python library for ANT+ communication

### Research References

The cadence calculations are based on biomechanics research:

**Preferred transition speed between walking and running**  
[pubmed.ncbi.nlm.nih.gov/16286854](https://pubmed.ncbi.nlm.nih.gov/16286854/)  
*Explains the biomechanical walk-run transition zone around 7.0-7.4 km/h*

**Biomechanics of Gait Transition**  
[scholarworks.boisestate.edu](https://scholarworks.boisestate.edu/cgi/viewcontent.cgi?article=1178&context=td)  
*Detailed analysis of cadence patterns during gait transitions*

---

## Related Documentation

- **[COMPILE.md](COMPILE.md)** - Building from source
- **[TESTPLAN.md](TESTPLAN.md)** - Testing procedures for contributors