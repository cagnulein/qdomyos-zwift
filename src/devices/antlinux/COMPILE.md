# Compiling QDomyos-Zwift with ANT+ Support

This guide is for advanced users who need to compile QDomyos-Zwift from source. For majority of users it is recommended to follow the simpler path using [pre-compiled binaries](README.md).

**Return to main guide:** [README.md](README.md)

## When to Compile from Source

- Custom code modifications
- Feature development
- Unsupported Linux distribution
- Pre-compiled binaries don't work

## Prerequisites

Complete **Step 1: System Preparation** from the [main README](README.md) first:
- System dependencies installed
- Python virtual environment created
- USB permissions configured

**Important:** When compiling from source, you can use any Python 3.x version. However, if cross-compiling (building on one machine to run on another), ensure both machines use the same Python version to avoid library compatibility issues.

**Hardware requirements:** Raspberry Pi 4/5 or desktop Linux with 4GB+ RAM recommended. Pi Zero 2 W (512MB RAM) may encounter out-of-memory errors.

---

## Step 1: Install Build Tools

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
```

---

## Step 2: Ensure Python 3.x

### Check Python Availability

```bash
python3 --version
```

You can use any Python 3.x version for compilation. Common versions include Python 3.8, 3.9, 3.10, 3.11, or 3.12.

**Note:** If you need Python 3.11 specifically (e.g., to match the pre-compiled binary), or if you want a version not available in your system repositories, install it using pyenv:

### Install Python 3.11 via pyenv

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

# Add pyenv to shell (append to ~/.bashrc)
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

### Install Python 3.x Development Headers

```bash
# If Python 3.11 is from system packages
sudo apt-get install -y python3.11-dev

# Or for other Python versions, use the appropriate package
# Example: sudo apt-get install -y python3.10-dev

# If Python is from pyenv, dev headers are already included
```

### Create Virtual Environment

If not already created from README Step 1.2:

```bash
# Use whatever Python 3.x version you have available
python3 -m venv ~/ant_venv
~/ant_venv/bin/pip install --upgrade pip
~/ant_venv/bin/pip install openant pyusb pybind11
```

---

## Step 3: Clone Repository

```bash
cd ~
git clone https://github.com/cagnulein/qdomyos-zwift.git
cd qdomyos-zwift
git submodule update --init src/smtpclient/
git submodule update --init src/qmdnsengine/
git submodule update --init tst/googletest/
```

---

## Step 4: Configure for ANT+

```bash
cd ~/qdomyos-zwift/src

# Enable ANT+ support
grep -q "antlinux.pri" qdomyos-zwift.pro || echo 'include(devices/antlinux/antlinux.pri)' >> qdomyos-zwift.pro

# Link Python environment
echo "$HOME/ant_venv/bin/python3" > .ant_venv_path
```

---

## Step 5: Verify Setup (Optional)

```bash
sudo bash ~/qdomyos-zwift/src/devices/antlinux/build_check_list.sh
```

Address any critical failures before proceeding.

---

## Step 6: Test ANT+ Broadcasting (Optional)

Validates Python environment and hardware before full build:

```bash
sudo ~/ant_venv/bin/python3 ~/qdomyos-zwift/src/devices/antlinux/ant_test_broadcaster.py
```

**Expected:** Watch connects in 5-10 seconds, displays stable pace/cadence. Press `Ctrl+C` to stop.

See [main README pairing instructions](README.md#31-pairing-your-device) if needed.

---

## Step 7: Build

```bash
cd ~/qdomyos-zwift/src/
qmake qdomyos-zwift.pro
```

Verify output shows: `Project MESSAGE: >>> ANT+ ENABLED for build <<<`

If missing, return to Step 5.

```bash
make
```

Binary location: `~/qdomyos-zwift/src/qdomyos-zwift`

---

## Step 8: Test

### Copy to Standard Location

```bash
cp ~/qdomyos-zwift/src/qdomyos-zwift ~/qdomyos-zwift
```

### Run Application

```bash
cd ~
sudo ./qdomyos-zwift -no-gui -ant-footpod

# Optional flags:
# -ant-device 12345    # Custom device ID (1-65535)
# -ant-verbose         # Debug logging (large logs)
```

Your watch should pair as a Foot Pod. See [main README pairing instructions](README.md#31-pairing-your-device) for details.

---

## Step 9: Systemd Service (Optional)

Follow [Step 3.3 in main README](README.md#33-configure-automatic-startup-optional) to set up the systemd service.

---

## Cross-Compilation Notes

When compiling on one machine for another (e.g., compile on Pi 4, run on Pi Zero 2 W):

1. **Build machine:** Follow all steps in this guide with your chosen Python 3.x version
2. **Target machine:** Complete Step 1 from main README with the **same Python 3.x version**
3. **Critical:** Both machines must use the same Python version (e.g., both 3.11, or both 3.10)
4. Transfer compiled binary to target machine: `~/qdomyos-zwift`

**Verify Python version match:**
```bash
# On both machines, check Python version
~/ant_venv/bin/python3 --version
```

Both should show the same Python version (e.g., `Python 3.10.12` on both).

**Example scenarios:**
- Build on Pi 4 with Python 3.11 → Target Pi Zero 2 W must have Python 3.11
- Build on Ubuntu 24.04 with Python 3.12 → Target Ubuntu 24.04 must have Python 3.12
- Build on Bookworm with Python 3.11 → Target Bookworm with Python 3.11

---

## Success Indicators

✓ `qmake` shows "ANT+ ENABLED"  
✓ Test script connects to watch in <10 seconds  
✓ Watch displays stable pace/cadence  
✓ Treadmill data broadcasts via ANT+

---

## Troubleshooting

### Build Issues

| Issue | Solution |
|-------|----------|
| `qmake` fails "Unknown module" | Re-run Step 1 apt-get install |
| `make` fails "Python.h missing" | Install python3.11-dev or use pyenv (includes headers) |
| Build killed/out of memory | Use system with more RAM |
| Python version mismatch | Ensure both build/target use Python 3.11 |
| `error while loading shared libraries: libpython3.11.so.1.0` | Python 3.11 not on target system - install via apt or pyenv |

### pyenv Issues

| Issue | Solution |
|-------|----------|
| `pyenv: command not found` | Reload shell: `source ~/.bashrc` |
| pyenv build fails | Install all dependencies from Step 2, check disk space |
| Python 3.11.9 not found | Use different version: `pyenv install 3.11.7` |

See [main README troubleshooting](README.md#troubleshooting) for additional issues.

---

**Return to [main README](README.md)**