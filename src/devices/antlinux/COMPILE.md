# Compiling QDomyos-Zwift with ANT+ Support

This guide is for advanced users who need to compile QDomyos-Zwift from source. Most users will find the [pre-compiled binary installation method](README.md) more straightforward.

**Return to the main installation guide:** [README.md](README.md)

## When to Compile from Source

- You need custom modifications to the code
- You're developing new features
- You're running an unsupported Linux distribution
- The pre-compiled binaries don't work on your system

## Prerequisites

Before starting, complete **Step 1: System Preparation** from the [main README](README.md) first. This includes:
- Installing system dependencies
- Creating the Python virtual environment (you can use any Python 3 version for compilation)
- Configuring USB permissions

**Note on Python versions:** Unlike the pre-compiled binary which requires Python 3.11, when compiling from source you can use any Python 3 version. Just ensure the Python version matches between your build machine and target machine if cross-compiling.

## Compilation Hardware Requirements

**Raspberry Pi:** We recommend using Pi 4 or 5 for compilation. Pi Zero 2 W has limited RAM (512MB) which may cause out-of-memory errors during compilation.

**Desktop Linux:** Any modern x86-64 system with adequate RAM (4GB+ recommended).

### Cross-Compilation Setup

If compiling on one machine to run on another (e.g., compile on Pi 4, run on Pi Zero 2 W):
- **Build machine:** Follow all steps in this guide
- **Target machine:** Complete Step 1 from the main README, then transfer the compiled binary
- **Recommendation:** Ensure both machines have matching Python versions to avoid library compatibility issues

---

## Step 1: Install Development Dependencies

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

# Install Python development headers matching your venv Python version
PYVER=$(~/ant_venv/bin/python3 -c 'import sys; print(f"{sys.version_info.major}.{sys.version_info.minor}")')
sudo apt-get install -y "python${PYVER}-dev"
```

---

## Step 2: Clone Repository

```bash
cd $HOME
git clone https://github.com/cagnulein/qdomyos-zwift.git
cd qdomyos-zwift

# Update submodules
git submodule update --init src/smtpclient/
git submodule update --init src/qmdnsengine/
git submodule update --init tst/googletest/
```

---

## Step 3: Configure Build

```bash
cd $HOME/qdomyos-zwift/src

# Enable ANT+ support
grep -q "antlinux.pri" qdomyos-zwift.pro || echo 'include(devices/antlinux/antlinux.pri)' >> qdomyos-zwift.pro

# Link Python environment
echo "$HOME/ant_venv/bin/python3" > .ant_venv_path
```

---

## Step 4: Verify Setup (Optional but Recommended)

Run the diagnostic script to check your environment:

```bash
sudo bash $HOME/qdomyos-zwift/src/devices/antlinux/build_check_list.sh
```

Review the output and address any issues flagged as critical failures before proceeding.

---

## Step 5: Test ANT+ Broadcasting (Optional but Recommended)

This validates your Python environment and hardware before the full build:

```bash
sudo $HOME/ant_venv/bin/python3 $HOME/qdomyos-zwift/src/devices/antlinux/ant_test_broadcaster.py
```

**Expected behavior:**
- Watch connects within 5-10 seconds
- Displays stable pace (e.g., 5:35 min/km) and cadence (e.g., 150 SPM)
- Press `Ctrl+C` to stop

For watch pairing instructions, see **Step 3.1: Pairing Your Device** in the [main README](README.md#31-pairing-your-device).

**If this test doesn't work**, it's worth troubleshooting before proceeding with the full build.

---

## Step 6: Build Application

```bash
cd $HOME/qdomyos-zwift/src/
qmake qdomyos-zwift.pro
```

**Verify ANT+ is enabled** - look for this message:
```
Project MESSAGE: >>> ANT+ ENABLED for build <<<
```

If this message is missing, return to Step 4 to diagnose the issue.

```bash
make
```

The compilation will take several minutes. The binary will be at `$HOME/qdomyos-zwift/src/qdomyos-zwift`.

---

## Step 7: Test and Configure

### 7.1 Copy Binary to Standard Location

To keep the configuration consistent with the main README, copy your compiled binary to your home directory:

```bash
cp $HOME/qdomyos-zwift/src/qdomyos-zwift ~/qdomyos-zwift
```

### 7.2 Test the Application

Connect your ANT+ dongle and start your treadmill:

```bash
cd ~
sudo ./qdomyos-zwift -no-gui -ant-footpod

# Optional: Custom device ID (useful for conflicts, 1-65535)
sudo ./qdomyos-zwift -no-gui -ant-footpod -ant-device 12345

# Optional: Verbose logging (generates large logs - debug only)
sudo ./qdomyos-zwift -no-gui -ant-footpod -ant-verbose
```

Your watch should pair as a Foot Pod. See the [main README pairing instructions](README.md#31-pairing-your-device) for details.

### 7.3 Configure Automatic Startup (Optional)

Follow **Step 3.3** in the [main README](README.md#33-configure-automatic-startup-optional) to set up the systemd service. Since you've copied the binary to `~/qdomyos-zwift`, the configuration is identical to the pre-compiled binary setup.

---

## Success Indicators

✓ `qmake` shows "ANT+ ENABLED" message  
✓ Test script (Step 5) connects to watch within 10 seconds  
✓ Watch displays stable pace and cadence  
✓ QZ broadcasts data from your treadmill  

---

## Troubleshooting

**Compilation-specific issues:**

| Issue | Solution |
| --- | --- |
| `qmake` fails with "Unknown module(s)" | Re-run apt-get install from Step 1 |
| `make` fails with "Python.h: No such file" | Install Python dev headers (Step 1) |
| `g++: fatal error: Killed signal terminated` | Out of memory - compile on system with more RAM (Pi 4/5 or desktop) |
| Python version mismatch errors | Ensure build and target machines have matching Python versions, or recompile on target machine |

**For all other issues**, see the [main README troubleshooting section](README.md#troubleshooting).

---

**For Developers and Testers:**

If you're interested in testing the pre-compiled binaries or contributing to quality assurance, see our [Test Plan](TEST_PLAN.md) which covers validation procedures for both Raspberry Pi and Desktop Linux platforms.

---

**Return to [main README](README.md)** for additional information and credits.