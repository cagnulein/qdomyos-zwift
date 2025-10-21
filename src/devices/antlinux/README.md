# ANT+ Virtual Footpod Broadcaster for Linux (Raspberry Pi)

This guide will help you configure your Raspberry Pi running QDomyos-Zwift (QZ) into an ANT+ bridge that broadcasts treadmill data as a virtual footpod, enabling Garmin watches and other ANT+ devices to display real-time data.

## What You'll Achieve
- Broadcast treadmill pace, distance, and estimated cadence in real-time
- Pair your Garmin watch or other ANT+ device as a Foot Pod
- Log accurate run data directly to your fitness ecosystem

## Prerequisites

### Quick Prerequisites Check

**All users need:**
- ANT+ USB dongle (Garmin USB2 0fcf:1008 or USB-m 0fcf:1009 recommended)
- Garmin watch or compatible ANT+ device
- Raspberry Pi running OS based on Debian Bookworm
- Familiarity with terminal commands, editing text files, and basic Linux navigation

**Method 1 (Pre-compiled Binary):**
- Raspberry Pi Zero 2 W or newer (any model)
- No existing QDomyos-Zwift installation needed

**Method 2 (Compile from Source):**
- Raspberry Pi 4/5 OR desktop Linux for compilation
- Requires QZ source code: https://github.com/cagnulein/qdomyos-zwift/blob/master/docs/10_Installation.md

### Tested Configuration
- Hardware: Raspberry Pi Zero 2 W (Raspberry Pi OS Bookworm)
- Treadmill: Proform 705 CST
- Watch: Garmin Forerunner 245

### Backup Recommendation

Consider backing up your system before proceeding.

---

## Installation Overview

The installation process consists of three parts:

**Part A: System Setup** - One-time configuration (runtime dependencies, Python environment, USB permissions)

**Part B: Choose Your Method** - Download pre-compiled binary OR compile from source

**Part C: Testing and Automation** - Run the application and configure automatic startup

---

## Part A: System Setup (Required for All Users)

Complete these steps regardless of your chosen method. These prepare your Raspberry Pi with the necessary runtime environment.

### Step A1: Install System Dependencies

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
	python3.11 \
	python3.11-venv \
	python3-pip
```

### Step A2: Create Python Virtual Environment

The QZ application looks for an environment named `ant_venv` in your home directory. **This is mandatory even for the pre-compiled binary.**

```bash
# Create the virtual environment
python3.11 -m venv ~/ant_venv

# Install required Python libraries (do not use sudo)
~/ant_venv/bin/pip install --upgrade pip
~/ant_venv/bin/pip install openant pyusb pybind11
```

### Step A3: Configure USB Permissions

```bash
# Create udev rule for ANT+ dongles
sudo tee /etc/udev/rules.d/99-ant-usb.rules > /dev/null << 'EOF'
SUBSYSTEM=="usb", ATTRS{idVendor}=="0fcf", ATTRS{idProduct}=="100?", MODE="0666", GROUP="plugdev"
SUBSYSTEM=="usb", ATTRS{idVendor}=="11fd", ATTRS{idProduct}=="0001", MODE="0666", GROUP="plugdev"
EOF

# Apply permissions
sudo udevadm control --reload-rules && sudo udevadm trigger
sudo usermod -aG plugdev $USER

# Reboot to apply changes
sudo reboot
```

After the reboot, proceed to Part B.

---

## Part B: Choose Your Installation Method

### Method 1: Download Pre-Compiled Binary (Recommended)

Fastest method - skip compilation entirely.

#### Step B1.1: Download from GitHub Actions

1. Navigate to the **[Actions tab](https://github.com/cagnulein/qdomyos-zwift/actions)**
2. Find the latest successful workflow run on `master` branch (green checkmark)
3. Scroll to **"Artifacts"** section
4. Download **`raspberry-pi-binary-64bit-ant`**

#### Step B1.2: Install Binary

1. Unzip the downloaded file to find `qdomyos-zwift-64bit-ant`
2. Transfer to your Raspberry Pi's home directory (`/home/pi/`)
3. Rename and make executable:

```bash
cd /home/pi/
mv qdomyos-zwift-64bit-ant qdomyos-zwift
chmod +x qdomyos-zwift
```

Proceed to **Part C: Testing and Automation**.

---

### Method 2: Compile from Source (Advanced)

For developers or users needing custom modifications.

#### Compilation Hardware Requirements

Use Raspberry Pi 4 or 5 for compilation. Pi Zero 2 W has insufficient RAM (512MB) and will cause out-of-memory errors.

**Cross-compilation setup:**
- Build machine: Follow all Method 2 steps
- Target machine: Follow Part A, transfer binary, then Part C
- **Critical:** Both machines must have the same Python version (see troubleshooting)

#### Step B2.1: Install Development Dependencies

```bash
sudo apt-get install -y \
	git g++ make \
	qtbase5-dev qtbase5-private-dev \
	qtconnectivity5-dev qtpositioning5-dev \
	libqt5charts5-dev libqt5networkauth5-dev \
	libqt5websockets5-dev qtmultimedia5-dev \
	libqt5multimediawidgets5 libqt5multimedia5-plugins \
	qtlocation5-dev qtquickcontrols2-5-dev \
	libqt5texttospeech5-dev libqt5sql5-mysql \
	libqt5sql5-psql libusb-1.0-0-dev \
	libudev-dev qt5-assistant

# Install Python development headers
PYVER=$(~/ant_venv/bin/python3 -c 'import sys; print(f"{sys.version_info.major}.{sys.version_info.minor}")')
sudo apt-get install -y "python${PYVER}-dev"
```

#### Step B2.2: Clone Repository

```bash
cd $HOME
git clone https://github.com/cagnulein/qdomyos-zwift.git
cd qdomyos-zwift

# Update submodules
git submodule update --init src/smtpclient/
git submodule update --init src/qmdnsengine/
git submodule update --init tst/googletest/
```

#### Step B2.3: Configure Build

```bash
cd $HOME/qdomyos-zwift/src
git pull

# Enable ANT+ support
grep -q "antlinux.pri" qdomyos-zwift.pro || echo 'include(devices/antlinux/antlinux.pri)' >> qdomyos-zwift.pro

# Link Python environment
echo "$HOME/ant_venv/bin/python3" > .ant_venv_path
```

#### Step B2.4: Verify Setup (Optional)

```bash
sudo bash $HOME/qdomyos-zwift/src/devices/antlinux/build_check_list.sh
```

Resolve any critical failures before proceeding.

#### Step B2.5: Test ANT+ Broadcasting (Optional)

Validates Python environment and hardware before full build:

See *Step C1* for Pairing Your Device

```bash
sudo $HOME/ant_venv/bin/python3 $HOME/qdomyos-zwift/src/devices/antlinux/ant_test_broadcaster.py
```

**Expected behavior:**
- Watch connects within 5-10 seconds
- Displays stable pace (e.g., 5:35 min/km) and cadence (e.g., 150 SPM)
- Press `Ctrl+C` to stop

If this fails, the full build is unlikely to work.

#### Step B2.6: Build Application

```bash
cd $HOME/qdomyos-zwift/src/
qmake qdomyos-zwift.pro
```

**Verify ANT+ is enabled** - look for this message:
`Project MESSAGE: >>> ANT+ ENABLED for build <<<`

If missing, return to Step B2.4.

```bash
make
```

Binary will be at `$HOME/qdomyos-zwift/src/qdomyos-zwift`.

Proceed to **Part C: Testing and Automation**.

---

## Part C: Testing and Automation (Required for All Users)

### Step C1: Pairing Your Device

**On Garmin watches:**
1. Menu > Sensors & Accessories > Add New > Foot Pod
2. Watch detects and pairs within 5-10 seconds
3. Start "Treadmill" or "Run Indoor" activity
4. Add Cadence field: Settings > Activities & Apps > [Activity] > Data Screens > customize

**Cadence behavior:**
The system switches between walking and running cadence at 7.0 km/h (based on biomechanics research showing the walk-run transition occurs around 7.2-7.4 km/h):
- Below 7.0 km/h: Walking cadence (90-140 SPM)
- At/above 7.0 km/h: Running cadence (160-200 SPM)
- Example: 6.0 km/h walk ≈ 128 SPM, 8.5 km/h jog ≈ 166 SPM

See "Further reading" section for research details.

### Step C2: Test the Application

Connect your ANT+ dongle and start your treadmill:

```bash
cd /home/pi/
sudo ./qdomyos-zwift -no-gui -ant-footpod
```

*Note: If you compiled from source (Method 2), the binary is in `$HOME/qdomyos-zwift/src/`*

Your watch should now pair as a Foot Pod (see Step C1).

**Optional parameters:**

```bash
# Custom device ID (useful for conflicts)
sudo ./qdomyos-zwift -no-gui -ant-footpod -ant-device 12345

# Verbose logging (generates large logs - debug only)
sudo ./qdomyos-zwift -no-gui -ant-footpod -ant-verbose
```

### Step C3: Configure Automatic Startup (Optional)

```bash
sudo nano /lib/systemd/system/qz.service
```

Configuration:

```ini
[Unit]
Description=qdomyos-zwift service
After=multi-user.target

[Service]
User=root
Group=plugdev
Environment="QZ_USER=pi"
WorkingDirectory=/home/pi
ExecStart=/home/pi/qdomyos-zwift -no-gui -log -ant-footpod
KillSignal=SIGINT

[Install]
WantedBy=multi-user.target
```

*Note: If compiled from source, update `WorkingDirectory` to `/home/pi/qdomyos-zwift/src` and `ExecStart` to `/home/pi/qdomyos-zwift/src/qdomyos-zwift`*

Apply changes:

```bash
sudo systemctl daemon-reload
sudo systemctl enable qz
sudo systemctl start qz
sudo systemctl status qz
```

---

## Success Indicators

✓ (Method 2 only) `qmake` shows "ANT+ ENABLED" message  
✓ (Method 2 only) Test script connects to watch within 10 seconds  
✓ Watch displays stable pace and cadence  
✓ QZ broadcasts data from your treadmill  

---

## Troubleshooting

| Issue | Solution |
| --- | --- |
| `qmake` fails with "Unknown module(s)" | Re-run apt-get install from Step B2.1 |
| `make` fails with "Python.h: No such file" | Install Python dev headers (Step B2.1) |
| `g++: fatal error: Killed signal terminated` | Out of memory on Pi Zero 2 W - compile on Pi 4/5 instead |
| `error while loading shared libraries: libpython3.XX.so.1.0` | Python version mismatch - recompile on matching version OR `sudo apt-get install python3.11` |
| Test script hangs/fails | Run with sudo, reboot after Step A3, or unplug/replug dongle |
| Watch connects but pace shows --:-- | Treadmill model not set: `sudo nano /root/.config/Roberto\ Viola/qDomyos-Zwift.conf` add your model flag (e.g., `proform_treadmill_705_cst=true`) |
| App works but watch won't connect | Unplug/replug dongle, verify Step A3 + reboot, check device ID (default 54321), ensure running as root, check logs |
| `systemctl stop qz` hangs | Add `KillSignal=SIGINT` to [Service] section in qz.service |

---

## Credits & Acknowledgments

- Main project: https://github.com/cagnulein/qdomyos-zwift
- ANT+ Linux footpod implementation: bassai-sho
- Documentation assisted by Claude and Gemini

**Further reading:**

[Preferred transition speed between walking and running](https://pubmed.ncbi.nlm.nih.gov/16286854/)  
[Biomechanics of Gait Transition](https://scholarworks.boisestate.edu/cgi/viewcontent.cgi?article=1178&context=td)