# ANT+ Virtual Footpod Broadcaster for Linux (Raspberry Pi)

This guide will help you configure your Raspberry Pi running QDomyos-Zwift (QZ) into an ANT+ bridge that broadcasts treadmill data as a virtual footpod, enabling Garmin watches and other ANT+ devices to display real-time data.

## What You'll Achieve
- Broadcast treadmill pace, distance, and estimated cadence in real-time
- Pair your Garmin watch or other ANT+ device as a Foot Pod
- Log accurate run data directly to your fitness ecosystem

## Prerequisites

This feature requires the QZ source code and cannot be used with pre-built QZ packages.

### Quick Prerequisites Check
Before you begin, please verify the following:

- Raspberry Pi 4/5 OR desktop Linux for compilation
- ANT+ USB dongle (Garmin or compatible)
- Garmin watch or compatible ANT+ device
- Existing, working QDomyos-Zwift source installation on a Raspberry Pi
- Familiarity with running terminal commands, editing text files, and basic Linux file system navigation

This guide assumes you are comfortable using Linux and already have a working QDomyos-Zwift source installation on a Raspberry Pi. If not, first read these instructions: https://github.com/cagnulein/qdomyos-zwift/blob/master/docs/10_Installation.md

## Verified Hardware & Software
This guide has been tested using:
- qdomyos-zwift binary running on Raspberry Pi Zero 2 W (Raspberry Pi OS Bookworm & Trixie) 
- Dongles: Garmin ANT+ USB-m (0fcf:1009) & Garmin ANT+ USB2 (0fcf:1008) 
- Watch: Garmin Forerunner 245 Watch 
- Treadmill: Proform 705 CST treadmill 

### Compilation Hardware

Compile the application on a Raspberry Pi 4 or 5. These models have sufficient RAM and processing power for C++ compilation. Compiling directly on a Pi Zero 2 W is not recommended due to insufficient RAM (512MB) - this will likely result in out-of-memory errors or system freezing.

If you are compiling on a Pi 4/5 to run on a different Pi (such as Pi Zero 2 W):
- On the build machine: Follow all steps (Steps 1-9)
- On the target machine: Follow all steps with the exception of Step 7 (Build QZ with ANT+ Support) - simply use the binary you compiled on the build machine

**Important:** Ensure both machines have the same Python version. If they differ, you will experience library loading errors when running the application (see troubleshooting: "error while loading shared libraries: libpython3.XX.so.1.0").

## ANT+ Requirements

You will need an ANT+ USB dongle, such as the tested Garmin ANT+ USB2 (0fcf:1008) or Garmin ANT+ USB-m (0fcf:1009). A compatible fitness device like a Garmin watch is also necessary.

## Backup Recommendation

Consider backing up your system before proceeding.

## Installation

This guide assumes your QZ source code is located at `$HOME/qdomyos-zwift/`

### Step 1 - Install System Dependencies

(Note: Skip this step if you already have a working QZ source installation)

Ensure your QZ source is in `$HOME/qdomyos-zwift/`

Start by updating your system and installing the required packages:

```bash
# These are the steps from the main guide and included here for reference. Only follow if starting from scratch.
sudo apt-get update
sudo apt-get upgrade

sudo apt-get install git libqt5bluetooth5 libqt5widgets5 libqt5positioning5 libqt5xml5 qtconnectivity5-dev qtbase5-private-dev qtpositioning5-dev libqt5charts5-dev libqt5charts5 qt5-assistant libqt5networkauth5-dev libqt5websockets5-dev qtmultimedia5-dev libqt5multimediawidgets5 libqt5multimedia5-plugins libqt5multimedia5 qtlocation5-dev qtquickcontrols2-5-dev libqt5texttospeech5-dev libqt5texttospeech5 g++ make qtbase5-dev libqt5sql5 libqt5sql5-mysql libqt5sql5-psql

git clone https://github.com/cagnulein/qdomyos-zwift.git

# Move into the QZ directory
cd $HOME/qdomyos-zwift

# Update the submodules
git submodule update --init src/smtpclient/
git submodule update --init src/qmdnsengine/
git submodule update --init tst/googletest/
```

### Step 2 - Create Python Virtual Environment

This environment isolates the required Python libraries, which is recommended for modern Linux distributions (Debian Bookworm+) to avoid conflicts with system packages.

```bash
# Update package lists and install essential libraries for ANT+ support
sudo apt-get update
sudo apt-get install -y libusb-1.0-0-dev libudev-dev python3-pip python3-venv
```

```bash
# Create a Python virtual environment in your home directory
python3 -m venv $HOME/ant_venv
```

```bash
# Upgrade pip to the latest version within the virtual environment
$HOME/ant_venv/bin/python3 -m pip install --upgrade pip
```

```bash
# Install the required Python packages for ANT+ communication
$HOME/ant_venv/bin/python3 -m pip install pybind11 pyusb openant
```

### Step 3 - Configure Build Environment

This step prepares the QZ source code and links it to your new Python environment.

```bash
# Navigate to the QDomyos-Zwift source subdirectory
cd $HOME/qdomyos-zwift/src
```

```bash
# Pull the latest changes from the repository
git pull
```

```bash
# This command checks if ANT+ linux support is already enabled in qdomyos-zwift.pro and adds it if missing
grep -q "antlinux.pri" qdomyos-zwift.pro || echo 'include(devices/antlinux/antlinux.pri)' >> qdomyos-zwift.pro
```

```bash
# Store the Python executable path for the qmake build system to reference
echo "$HOME/ant_venv/bin/python3" > .ant_venv_path
```

Install Python development headers for your specific Python version:
```bash
# Detect the Python version from your venv and install the matching development headers
PYVER=$($HOME/ant_venv/bin/python3 -c 'import sys; print(f"{sys.version_info.major}.{sys.version_info.minor}")')
sudo apt-get install -y "python${PYVER}-dev"
```

### Step 4 - Configure USB Permissions

These rules enable non-root access to the ANT+ USB dongle.

```bash
# Create a udev rule file to automatically set correct permissions for ANT+ USB dongles
sudo tee /etc/udev/rules.d/99-ant-usb.rules > /dev/null << 'EOF'
SUBSYSTEM=="usb", ATTRS{idVendor}=="0fcf", ATTRS{idProduct}=="100?", MODE="0666", GROUP="plugdev"
SUBSYSTEM=="usb", ATTRS{idVendor}=="11fd", ATTRS{idProduct}=="0001", MODE="0666", GROUP="plugdev"
EOF
```

```bash
# Reload udev rules and trigger re-evaluation for all connected devices
sudo udevadm control --reload-rules && sudo udevadm trigger
```

```bash
# Add your user to the plugdev group for direct USB device access
sudo usermod -aG plugdev $USER
```

```bash
# Reboot system to apply settings
sudo reboot
```

### Step 5 - Verify Setup

After rebooting, run the included diagnostic script to check the environment.

```bash
# Run the automated checklist
sudo bash $HOME/qdomyos-zwift/src/devices/antlinux/build_check_list.sh
```

If the checklist reports any critical failures, resolve them before proceeding.

### Step 6 - Test ANT+ Broadcasting

This standalone test validates that your Python environment and hardware are working correctly before the full QZ build.

```bash
# Run the ANT+ test script with sudo privileges
sudo $HOME/ant_venv/bin/python3 $HOME/qdomyos-zwift/src/devices/antlinux/ant_test_broadcaster.py
```

**What to expect:**

The test script simulates a running session and should connect to your watch within 5-10 seconds. You'll see output showing speed and pace data being broadcast.

**Pairing your device:**

1. On Garmin watches: Navigate to Menu > Sensors & Accessories > Add New > Foot Pod
2. Your watch should detect and pair with the virtual footpod within 5-10 seconds
3. Start a "Treadmill" or "Run Indoor" activity to see pace and cadence data
4. To view cadence on your watch: Go to Settings > Activities & Apps > select your activity (e.g., Treadmill Run) > Data Screens > customize the screen and add Cadence as a field

**If successful:** Your watch will display stable pace (e.g., 5:35 min/km) and cadence (e.g., 150 SPM) values.

Press `Ctrl+C` once to stop the test. If this test fails, the full QZ build is unlikely to work correctly.

**Understanding the cadence values:**

The system automatically switches between walking and running cadence at 7.0 km/h. Biomechanics research shows this transition typically occurs around 7.2-7.4 km/h, where running becomes more energy-efficient than fast walking. The model uses 7.0 km/h as a compromise value that provides smoother data on Garmin watches while accounting for individual variation. Below 7.0 km/h you'll see walking cadence (90-140 steps per minute). At or above 7.0 km/h you'll see running cadence (160-200 steps per minute). For example, a 6.0 km/h walk shows around 128 SPM, while an 8.5 km/h jog shows around 166 SPM.

For more information on the walk-run transition, see the "Further reading" section at the end of this guide.

### Step 7 - Build QZ with ANT+ Support

Run qmake to generate the Makefile with ANT+ support.

```bash
# Return to the QZ source subdirectory
cd $HOME/qdomyos-zwift/src/
```

```bash
# Generate a new Makefile
qmake qdomyos-zwift.pro
```

During the `qmake` step, look for this message to confirm ANT+ support is detected:

`Project MESSAGE: >>> ANT+ ENABLED for build <<<`

If this message is missing, return to Step 5 to run the diagnostic script and resolve any failures before continuing.

Build the application:

```bash
# Compile QDomyos-Zwift with ANT+ footpod support
make
```

### Step 8 - Test QZ with ANT+

After successfully building, test QZ with ANT+ broadcasting:

```bash
# Run QZ in no-GUI mode with ANT+ footpod broadcasting enabled
sudo ./qdomyos-zwift -no-gui -ant-footpod
```

Optional parameters:

`-ant-device <id>`: Use a custom ANT+ device ID. This is useful if you have multiple devices or are experiencing conflicts. The ID can be any number from 1 to 65535.

```bash
sudo ./qdomyos-zwift -no-gui -ant-footpod -ant-device 12345
```

`-ant-verbose`: Enable detailed DEBUG level logging from the internal Python ANT+ script. The default logging level is INFO. This verbose mode will log every ANT+ payload being sent. Use this only when debugging, as it can generate very large log files.

```bash
sudo ./qdomyos-zwift -no-gui -ant-footpod -ant-verbose
```

### Step 9 - Configure Automatic Startup (systemd)

To have QZ start automatically with ANT+ support, modify your systemd service file.

```bash
# Edit the QZ systemd service file
sudo nano /lib/systemd/system/qz.service
```

Ensure the `[Service]` section looks like this. The key changes are running as `root`, replacing 'pi' with your actual username if different, and adding the ANT+ command-line flag.

```ini
[Unit]
Description=qdomyos-zwift service
After=multi-user.target

[Service]
# Run as root to handle Bluetooth permissions and allow QSettings to work correctly
User=root
Group=plugdev

# Replace 'pi' with username used for virtual Python environment 
Environment="QZ_USER=pi"

# Set working directory where log file will be written
WorkingDirectory=/home/pi/qdomyos-zwift/src

# The QZ command to execute, amend the path location and flags as needed
ExecStart=/home/pi/qdomyos-zwift/src/qdomyos-zwift -no-gui -log -ant-footpod

# Ensure graceful shutdown on stop
KillSignal=SIGINT

[Install]
WantedBy=multi-user.target
```

Apply the changes:
```bash
# Reload systemd configuration and start the QZ service
sudo systemctl daemon-reload
sudo systemctl start qz
sudo systemctl status qz
```

## Success Indicators
How to know everything is working correctly:

✓ qmake shows the "ANT+ ENABLED" message  
✓ The standalone test script (Step 6) connects to your watch within 10 seconds  
✓ Your watch displays a stable pace (e.g., 5:35 min/km) and cadence (e.g., 150 SPM)  
✓ The full QZ application (Step 8) successfully pairs and broadcasts data from your treadmill  

## Troubleshooting

| Issue | Solution |
| --- | --- |
| `qmake` fails with "Unknown module(s)" | Re-run the apt-get install command from Step 1. |
| `make` fails with "Python.h: No such file or directory" | Install Python development headers as shown in Step 3. |
| `g++: fatal error: Killed signal terminated program cc1plus` | Out-of-memory error on Pi Zero 2 W. Compile on Raspberry Pi 4/5 or Linux desktop instead. Increasing swap space is not effective. |
| `error while loading shared libraries: libpython3.XX.so.1.0` | Python version mismatch. Best solution: Re-compile on a machine with the same Python version. Alternative: Install the required Python version: `sudo apt-get install python3.11` |
| `ant_test_broadcaster.py` hangs or fails | Run with sudo, reboot after Step 4, or unplug/replug the ANT+ dongle. |
| Watch connects but pace shows --:-- | Treadmill model flag not set. Edit root config: `sudo nano /root/.config/Roberto\ Viola/qDomyos-Zwift.conf` and add your model flag (e.g., `proform_treadmill_705_cst=true`). |
| Test script works but watch doesn't connect | Unplug/replug dongle, verify Step 4 completion with reboot, check device ID (default 54321), ensure QZ runs as root (Step 9), check log for errors. |
| `systemctl stop qz` hangs | Add `KillSignal=SIGINT` to the [Service] section in qz.service file. |

---

## Credits & Acknowledgments

- Main project: https://github.com/cagnulein/qdomyos-zwift
- ANT+ Linux footpod implementation: bassai-sho
- Documentation assisted by Claude and Gemini

**Further reading:**

[Preferred transition speed between walking and running](https://pubmed.ncbi.nlm.nih.gov/16286854/)  
[Biomechanics of Gait Transition](https://scholarworks.boisestate.edu/cgi/viewcontent.cgi?article=1178&context=td)