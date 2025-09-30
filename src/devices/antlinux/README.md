# ANT+ Virtual Footpod Broadcaster for Linux (Raspberry Pi)

This guide will help you transform your Raspberry Pi running QDomyos-Zwift (QZ) into an ANT+ bridge that broadcasts treadmill data as a virtual footpod, enabling Garmin watches and other ANT+ devices to display real-time pace and distance.

## Prerequisites

This guide assumes you are comfortable using Linux and already have a working QDomyos-Zwift source installation on a Raspberry Pi. This will not work with pre-built QZ packages - you need the source code to compile with ANT+ support.

### Tested Environment

This guide has been tested on:
- Hardware: Raspberry Pi Zero 2 W
- Operating System: Raspberry Pi OS (64-bit, Bookworm)
- QZ Installation: Source compilation in `$HOME/qdomyos-zwift/src/`
- ANT+ Compatible Device: Garmin Forerunner 245

### ANT+ Requirements

To add ANT+ support, you will need an ANT+ USB dongle. Common models include:
- Garmin ANT+ USB2 (0fcf:1008)
- Garmin ANT+ USB-m (0fcf:1009)
- Wahoo ANT+ USB (11fd:0001)

An ANT+ compatible device like a Garmin fitness watch or other compatible device.

### Backup Recommendation

Consider backing up your system before proceeding.

## Installation

### Step 1 - Install System Dependencies

Ensure your QZ source is in `$HOME/qdomyos-zwift/`

Start by updating your system and installing the required packages:

```bash
# Move into the QZ source directory
cd $HOME/qdomyos-zwift/src/
```

```bash
# Pull the latest changes from the repository
git pull
```

```bash
# Ensure qdomyos-zwift.pro includes the ANT+ module.
grep -q "include(devices/antlinux/antlinux.pri)" qdomyos-zwift.pro || echo -e 'include(devices/antlinux/antlinux.pri)' >> qdomyos-zwift.pro
```

```bash
# Update package lists and install essential libraries for ANT+ support
sudo apt-get update
sudo apt-get install -y libusb-1.0-0-dev libudev-dev python3-pip python3-venv
```

Optionally, install ccache to speed up repeated compilations:
```bash
# Install compiler cache to significantly reduce rebuild times (highly recommended)
sudo apt-get install -y ccache
```

### Step 2 - Create Python Virtual Environment

This isolates the required Python libraries from system packages, which is mandatory on modern Linux distributions (Debian Bookworm+).

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

This step creates a "bridge" file that tells the QZ build system where to find your Python environment.

```bash
# Navigate to the QDomyos-Zwift source directory
cd $HOME/qdomyos-zwift/src
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

These rules allow non-root users (and the application when running as root) to reliably access the ANT+ USB dongle.

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
sudo usermod -aG plugdev $USER```
```

```bash
# A reboot is the most reliable way to apply group membership changes
sudo reboot
```

### Step 5 - Verify Setup

After rebooting, run the included diagnostic script to verify that your entire environment is correctly configured before you attempt to build.

```bash
# Run the automated checklist from the correct directory
bash $HOME/qdomyos-zwift/src/devices/antlinux/build_check_list.sh
```

If the checklist reports any critical failures, resolve them before proceeding.

### Step 6 - Test ANT+ Broadcasting

This standalone test validates that your Python environment and hardware are working correctly, independent of the main QZ application.

```bash
# Run the ANT+ test script with sudo privileges
sudo $HOME/ant_venv/bin/python3 $HOME/qdomyos-zwift/src/devices/antlinux/ant_test_broadcaster.py
```

You should see output simulating a run with speed and pace data. While the test is running, pair your ANT+ device:
*   **Garmin watches:** Menu > Sensors & Accessories > Add New > Foot Pod
*   The watch should find the footpod. Start a "Treadmill" or "Run Indoor" activity, and pace data should appear.

Press `Ctrl+C` once to stop the test. If this test fails, the full build will not work.

### Step 7 - Build QZ with ANT+ Support

Clean any previous build artifacts and run qmake to generate the Makefile with ANT+ support.

```bash
# Return to the QZ source directory
cd $HOME/qdomyos-zwift/src/
```

```bash
# Clean previous build and generate a new Makefile
make clean && qmake qdomyos-zwift.pro
```

During the `qmake` step, look for this message to confirm ANT+ support is detected:

`Project MESSAGE: >>> ANT+ ENABLED for build <<<`

If this message is missing, return to the verification steps.

Now, build the application. This may take a long time on a Raspberry Pi.
```bash
# Compile QDomyos-Zwift with ANT+ footpod support
make
```

### Step 8 - Test QZ with ANT+

After successfully building, test QZ with ANT+ broadcasting:

```bash
# Run QZ in no-GUI mode with ANT+ footpod broadcasting enabled
# The `sudo` command is required.
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

Ensure the `[Service]` section looks like this. The key changes are running as `root` and adding the ANT+ command-line flag.

```ini
[Unit]
Description=qdomyos-zwift service
After=multi-user.target

[Service]
# Run as root to handle Bluetooth permissions and allow QSettings to work correctly
User=root
Group=plugdev

# Set the working directory to where the binary is located
WorkingDirectory=/home/pi/qdomyos-zwift/src

# The command to execute (replace 'pi' with your actual username)
# the various options are dependent on your desired setup
ExecStart=/home/pi/qdomyos-zwift/src/qdomyos-zwift -no-gui -no-log -ant-footpod

# Ensure graceful shutdown on stop
KillSignal=SIGINT

[Install]
WantedBy=multi-user.target
```

Apply the changes:
```bash
# Reload systemd configuration and restart the QZ service
sudo systemctl daemon-reload
sudo systemctl restart qz
sudo systemctl status qz
```

## Troubleshooting

| Symptom / Error Message | Solution |
| --- | --- |
| **`qmake` fails with `Unknown module(s)`** | A required Qt development package is missing. Re-run the `apt-get install` command in **Step 1**. |
| **`make` fails with `Python.h: No such file or directory`** | The Python development headers are missing. Re-run the `apt-get install "python${PYVER}-dev"` command in **Step 3**. |
| **Out of memory during compilation** | Increase swap space. Edit `/etc/dphys-swapfile` and set `CONF_SWAPSIZE=2048`, then restart the service. |
| **`ant_test_broadcaster.py` hangs or fails** | 1. Ensure you ran the script with `sudo`. <br> 2. Verify you have rebooted after **Step 4**. <br> 3. Unplug and replug the ANT+ dongle. |
| **Watch connects, but pace is always --:--** | The application is not processing treadmill data. This means the model-specific flag is not set. You must edit the **root user's** config file (`sudo nano /root/.config/Roberto\ Viola/qDomyos-Zwift.conf`) and set the correct flag (e.g., `proform_treadmill_705_cst=true`). |
| **`systemctl stop qz` command hangs** | The `KillSignal=SIGINT` line is missing from your `qz.service` file. This is essential for graceful shutdown.