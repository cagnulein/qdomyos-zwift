# ANT+ Virtual Footpod Broadcaster for Linux

Transform your Linux system into an ANT+ bridge that broadcasts treadmill data as a virtual footpod. Your Garmin watch or ANT+ device displays real-time pace, distance and cadence, just like running outdoors.

## Device Type Guidance

**For treadmill users only.** This ANT+ footpod feature broadcasts treadmill data.

For bikes, ellipticals or rowers, use the main application without the `-ant-footpod` flag in all commands.

---

## What You'll Achieve

- Broadcast treadmill pace, distance and cadence in real-time
- Pair your Garmin watch as a footpod for accurate indoor run tracking
- Automatically log runs to your fitness ecosystem

---

## Prerequisites

### Hardware
- ANT+ USB dongle (Garmin USB2/USB-m/USB3/mini or Suunto)
- Garmin watch or compatible ANT+ device
- Raspberry Pi (Zero 2 W, 3, 4 or 5) **or** x86-64 PC/laptop

### Software
- Debian/Ubuntu-based Linux (Bookworm or newer recommended)
- Basic terminal skills (navigating directories, editing files)

### Tested Configurations
- **Raspberry Pi**: Zero 2 W (Raspberry Pi OS Bookworm)
- **Desktop Linux**: Ubuntu 24.04 LTS (x86-64)
- **Treadmill**: Proform 705 CST
- **Watch**: Garmin Forerunner 245

---

## Quick Start

### 1. Download & Extract

Identify your platform:
- **Raspberry Pi**: `arm64`
- **Desktop Linux**: `x86-64`

Verify your system architecture:
```bash
uname -m
```
Output `x86_64` = use x86-64 binary | Output `aarch64` = use arm64 binary

Download from the [Releases page](https://github.com/cagnulein/qdomyos-zwift/releases):
- File: `linux-binary-<PLATFORM>-ant.zip`

Extract:
```bash
cd ~
unzip linux-binary-<PLATFORM>-ant.zip
cd qdomyos-zwift-<PLATFORM>-ant
```

### 2. System Check

Run the validation dashboard:
```bash
sudo ./setup-dashboard.sh
```

The dashboard validates:
- Python 3.11 availability
- Virtual environment
- Python packages (openant, pyusb, pybind11)
- Qt5 libraries
- USB permissions
- ANT+ dongle detection
- Bluetooth service

**All tests pass?** Skip to Step 4.

**Issues found?** Continue to Step 3.

### 3. Install Dependencies

The interactive dashboard guides you through automatic fixes:
```bash
sudo ./setup-dashboard.sh
```

Select the relevant option from the menu. The dashboard explains each step and requests confirmation before making changes.

### 4. Configure & Run

#### Configure the Application

**You must configure your treadmill model and user profile before pairing your watch.** Without this, the application cannot connect to your treadmill or broadcast data.

**With Display (Desktop/Laptop):**

Run the GUI to configure:
```bash
cd ~/qdomyos-zwift-<PLATFORM>-ant
sudo ./qdomyos-zwift -ant-footpod
```
Select your treadmill model and enter your user profile (age, weight, units). Settings save automatically.

**Headless (Raspberry Pi/Server):**

Use the interactive dashboard:
```bash
sudo ./setup-dashboard.sh
```
Select 'User Profile' for age, weight and units. Select 'Equipment' for your treadmill model.

Alternatively, configure on another system and transfer the file. The configuration file location is `~/.config/Roberto Viola/qDomyos-Zwift.conf` for ANT+ builds.

To transfer from another system:
```bash
# On the system with display
cp ~/.config/Roberto\ Viola/qDomyos-Zwift.conf ~/qz-config.conf
scp ~/qz-config.conf pi@raspberrypi:~/

# On headless system
mkdir -p ~/.config/Roberto\ Viola
cp ~/qz-config.conf ~/.config/Roberto\ Viola/qDomyos-Zwift.conf
```

#### Start the Application

Connect your ANT+ dongle, start your treadmill and run:

```bash
cd ~/qdomyos-zwift-<PLATFORM>-ant
sudo ./qdomyos-zwift -no-gui -ant-footpod

# Optional: Custom device ID (range 1–65535)
sudo ./qdomyos-zwift -no-gui -ant-footpod -ant-device 12345

# Optional: Verbose logging (debug only)
sudo ./qdomyos-zwift -no-gui -ant-footpod -ant-verbose
```

The application will connect to your treadmill via Bluetooth and begin broadcasting via ANT+.

#### Pair Your Garmin Watch

Once the application is running and connected to your treadmill:

1. Menu → Sensors & Accessories → Add New → Foot Pod
2. Watch detects and pairs automatically (5–10 seconds)
3. Start 'Treadmill' or 'Run Indoor' activity
4. **Add cadence field**: Settings → Activities & Apps → [Activity] → Data Screens → customise

#### Understanding Cadence

The system switches between walking and running cadence at **7.0 km/h** based on biomechanics research:

| Speed | Cadence Type | Typical Range | Example |
|-------|--------------|---------------|---------|
| < 7.0 km/h | Walking | 90–140 SPM | 6.0 km/h ≈ 128 SPM |
| ≥ 7.0 km/h | Running | 160–200 SPM | 8.5 km/h ≈ 166 SPM |

---

## Automatic Startup

The dashboard creates the systemd service file during setup. Enable and start manually:

```bash
sudo systemctl daemon-reload
sudo systemctl enable qz
sudo systemctl start qz
sudo systemctl status qz
```

---

## Troubleshooting

### Quick Diagnostics

**Validate Setup:**
```bash
sudo ./setup-dashboard.sh
```

**Test ANT+ Independently:**

From the dashboard menu, select 'ANT+ Test'. This simulates a running treadmill at various speeds. Press `Ctrl+C` to stop.

Advanced: run the test directly:
```bash
sudo ~/ant_venv/bin/python3 ./test_ant.py
```

---

### Common Issues

| Problem | Solution |
|---------|----------|
| ANT+ dongle not detected | Run `lsusb`, look for Garmin/Suunto/Dynastream. Unplug/replug dongle. Verify USB permissions and reboot |
| `error while loading shared libraries: libpython3.11.so.1.0` | Python 3.11 missing. Run dashboard for guided installation |
| Wrapper shows dependency warnings | Run dashboard for diagnosis and fixes |
| Test fails or watch won't pair | Ensure using `sudo`. Reboot after USB permissions setup. Try unplug/replug dongle |
| Watch won't detect footpod | Configure application first (Step 4). Application must be running and connected to treadmill before pairing watch |
| Watch pairs but pace shows `--:--` | Treadmill not configured or not connected. Configure your treadmill model first (Step 4). Ensure treadmill is powered and paired via Bluetooth |
| Treadmill not pairing via Bluetooth | Ensure treadmill is powered and in pairing mode |
| App works but no watch connection | Unplug/replug dongle. Verify USB permissions and reboot. Check device ID (default 54321) |
| Changes not taking effect | Reboot after changing USB permissions or user groups |
| Wrong device flag used | Treadmills: add `-ant-footpod`. Bikes/ellipticals/rowers: omit flag |
| `systemctl stop qz` hangs | Add `KillSignal=SIGINT` to service file `[Service]` section |
| Binary won't run: 'cannot execute binary file' | Wrong architecture. Download correct package: arm64 for Pi, x86-64 for desktop |
| `pyenv: command not found` | Reload shell: `source ~/.bashrc` or open new terminal |
| Multiple validation failures | Run dashboard and use Interactive Setup for step-by-step fixes |
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
*Biomechanical walk-run transition zone around 7.0–7.4 km/h*

[Biomechanics of Gait Transition](https://scholarworks.boisestate.edu/cgi/viewcontent.cgi?article=1178&context=td)  
*Detailed analysis of cadence patterns during gait transitions*

---

## Appendices

- [Compilation Guide](COMPILE.md)
- [Test Plan](TESTPLAN.md)