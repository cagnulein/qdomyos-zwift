# ANT+ Virtual Footpod Broadcaster for Linux

This guide will help you configure your Linux system running QDomyos-Zwift (QZ) into an ANT+ bridge that broadcasts treadmill data as a virtual footpod, enabling Garmin watches and other ANT+ devices to display real-time data.

## What You'll Achieve
- Broadcast treadmill pace, distance, and estimated cadence in real-time
- Pair your Garmin watch or other ANT+ device as a Foot Pod
- Log accurate run data directly to your fitness ecosystem

## Prerequisites

**All users need:**
- ANT+ USB dongle (Garmin USB2 0fcf:1008 or USB-m 0fcf:1009 recommended)
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

This guide covers installation using **pre-compiled binaries** from GitHub Actions. This is the recommended method for most users.

**Advanced users:** If you need to compile from source (for development or custom modifications), see the [Compilation Guide](COMPILE.md).

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
	python3.11 \
	python3.11-venv \
	python3-pip
```

### 1.2 Create Python Virtual Environment

The QZ application looks for an environment named `ant_venv` in your home directory. **This is mandatory** due to the application's design and modern Linux package policies (PEP 668).

**Important:** You must use Python 3.11 as the pre-compiled binaries are built with this version.

```bash
# Create the virtual environment
python3.11 -m venv ~/ant_venv

# Install required Python libraries (do not use sudo)
~/ant_venv/bin/pip install --upgrade pip
~/ant_venv/bin/pip install openant pyusb pybind11
```

### 1.3 Configure USB Permissions

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

After the reboot, proceed to Step 2.

---

## Step 2: Download and Install Binary

### 2.1 Download from GitHub Actions

1. Navigate to the **[Actions tab](https://github.com/cagnulein/qdomyos-zwift/actions)**
2. Find the latest successful workflow run on `master` branch (green checkmark)
3. Scroll to **"Artifacts"** section
4. Download the correct artifact for your platform:
   - **Raspberry Pi (ARM64):** `raspberry-pi-binary-64bit-ant`
   - **Desktop Linux (x86-64):** `linux-binary-x86-64-ant`

### 2.2 Install Binary

1. Unzip the downloaded file
2. Transfer the extracted binary to your home directory
3. Rename and make executable:

**For Raspberry Pi:**
```bash
cd ~
mv qdomyos-zwift-64bit-ant qdomyos-zwift
chmod +x qdomyos-zwift
```

**For Desktop Linux:**
```bash
cd ~
mv qdomyos-zwift-x86-64-ant qdomyos-zwift
chmod +x qdomyos-zwift
```

---

## Step 3: Run and Automate

### 3.1 Pairing Your Device

Before running the application, understand how to pair your device:

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

### 3.2 Test the Application

Connect your ANT+ dongle and start your treadmill. Your watch should pair as a Foot Pod.

```bash
cd ~
sudo ./qdomyos-zwift -no-gui -ant-footpod

# Optional: Custom device ID (useful for conflicts, 1-65535)
sudo ./qdomyos-zwift -no-gui -ant-footpod -ant-device 12345

# Optional: Verbose logging (generates large logs - debug only)
sudo ./qdomyos-zwift -no-gui -ant-footpod -ant-verbose
```

### 3.3 Configure Automatic Startup (Optional)

Create a systemd service to start QZ automatically on boot:

```bash
sudo nano /etc/systemd/system/qz.service
```

Add the following configuration:

```ini
[Unit]
Description=qdomyos-zwift service
After=multi-user.target

[Service]
User=root
Group=plugdev
Environment="QZ_USER=%USERNAME%"
WorkingDirectory=/home/%USERNAME%
ExecStart=/home/%USERNAME%/qdomyos-zwift -no-gui -log -ant-footpod
KillSignal=SIGINT

[Install]
WantedBy=multi-user.target
```

*Replace `%USERNAME%` with your actual username (typically `pi` on Raspberry Pi).*

Apply changes:

```bash
sudo systemctl daemon-reload
sudo systemctl enable qz
sudo systemctl start qz
sudo systemctl status qz
```

---

## Success Indicators

✓ Watch displays stable pace and cadence  
✓ QZ broadcasts data from your treadmill  
✓ Systemd service starts automatically on boot (if configured)

---

## Troubleshooting

| Issue | Solution |
| --- | --- |
| `error while loading shared libraries: libpython3.11.so.1.0` | Python 3.11 not installed - run `sudo apt-get install python3.11` |
| Test fails or watch won't connect | Run with sudo, reboot after Step 1.3, or unplug/replug dongle |
| Watch connects but pace shows --:-- | Treadmill model not set: `sudo nano /root/.config/Roberto\ Viola/qDomyos-Zwift.conf` add your model flag (e.g., `proform_treadmill_705_cst=true`) |
| App works but watch won't connect | Unplug/replug dongle, verify Step 1.3 + reboot, check device ID (default 54321), ensure running as root, check logs |
| `systemctl stop qz` hangs | Add `KillSignal=SIGINT` to [Service] section in qz.service |
| Binary won't run: "cannot execute binary file" | Wrong architecture - ensure you downloaded the correct artifact for your platform |

---

## Credits & Acknowledgments

- Main project: https://github.com/cagnulein/qdomyos-zwift
- ANT+ Linux footpod implementation: bassai-sho
- Documentation assisted by Claude and Gemini

**Further reading:**

[Preferred transition speed between walking and running](https://pubmed.ncbi.nlm.nih.gov/16286854/)  
[Biomechanics of Gait Transition](https://scholarworks.boisestate.edu/cgi/viewcontent.cgi?article=1178&context=td)