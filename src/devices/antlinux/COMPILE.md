# Building QDomyos-Zwift from Source

This guide is for users who need to build from source, such as for custom modifications, development, or unsupported systems.

**Most users should use pre-compiled binaries** - see the [README](README.md) for installation instructions.

---

## Why Build from Source?

**Build from source when you:**
- Want to modify the code or contribute changes
- Need to build for an unsupported platform
- Want the absolute latest development version
- Are developing new features

**Use pre-built binaries when you:**
- Just want to use the application (95% of users)
- Are following the standard installation guide
- Want the easiest and fastest setup

---

## Why Use Docker?

Docker provides a consistent, isolated build environment with all required dependencies. This:
- ✓ Avoids host system configuration issues
- ✓ Ensures reproducible builds
- ✓ Works for both x86-64 and ARM64 targets
- ✓ Handles all build dependencies automatically
- ✓ Creates a clean package ready to deploy

**You don't need to know Docker** - the script handles everything.

---

## Prerequisites

**What you need:**
- Linux system (x86-64 or ARM64)
- Docker (version 20.10 or newer)
- Git
- ~10GB free disk space
- 30-60 minutes for build time

**Install Docker on Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y docker.io docker-buildx
sudo usermod -aG docker $USER
newgrp docker  # Or logout and login
```

**Verify Docker works:**
```bash
docker --version
docker run hello-world
```

---

## Quick Start

**Build in 3 steps:**

### Step 1: Clone Repository

```bash
git clone https://github.com/cagnulein/qdomyos-zwift.git
cd qdomyos-zwift
```

### Step 2: Build for Your Architecture

```bash
cd src/devices/antlinux

# For desktop Linux (x86-64):
./docker-build.sh --arch x86-64

# For Raspberry Pi (ARM64):
./docker-build.sh --arch arm64
```

**Build time:**
- x86-64: ~30-45 minutes
- ARM64: ~45-60 minutes (cross-compilation)

**Output:** `linux-binary-ARCH-ant.zip` in project root

### Step 3: Extract and Test

```bash
cd ../../..  # Back to project root
unzip linux-binary-*-ant.zip -d ~/
cd ~/qdomyos-zwift-*-ant
sudo ./setup-dashboard.sh
```

**Expected result:** All checks pass, ready to use!

---

## Detailed Build Process

### What the Build Script Does

The `docker-build.sh` script:
1. ✓ Creates a Docker container with all build dependencies
2. ✓ Compiles Qt application with ANT+ support
3. ✓ Bundles all required libraries and files
4. ✓ Creates deployment package (.zip)
5. ✓ Cleans up temporary files

### Build Options

```bash
# Basic build
./docker-build.sh --arch x86-64

# Clean build (removes cached layers)
./docker-build.sh --arch x86-64 --clean

# Build for ARM64 from x86-64 host (cross-compile)
./docker-build.sh --arch arm64
```

### Build Output Structure

The created package contains:
```
qdomyos-zwift-ARCH-ant/
├── qdomyos-zwift           # Wrapper script
├── qdomyos-zwift-bin       # Compiled binary
├── setup-dashboard.sh      # Setup dashboard
├── test_ant.py            # ANT+ test script
├── ant_broadcaster.py      # ANT+ module
├── devices.ini            # Device mappings
├── devices_optimized.json # Device menu data
└── .menu_cache/           # Menu cache directory
```

---

## Testing Your Build

### Step 1: System Validation

```bash
cd ~/qdomyos-zwift-*-ant
sudo ./setup-dashboard.sh
```

**Dashboard will:**
- Check all dependencies
- Guide installations if needed
- Validate everything works

**Expected:** All checks pass ✓

### Step 2: ANT+ Hardware Test

**Via dashboard (recommended):**
```bash
sudo ./setup-dashboard.sh
```
Select "ANT+ Test" from menu

**Direct test (advanced):**
```bash
sudo ~/ant_venv/bin/python3 ./test_ant.py
```

**Expected results:**
- Watch pairs as footpod within 10 seconds ✓
- Pace and cadence update in real-time ✓
- Distance accumulates ✓
- No USB errors ✓

### Step 3: Full Application Test

```bash
sudo ./qdomyos-zwift -no-gui -ant-footpod
```

**Expected results:**
- Application starts ✓
- Connects to treadmill ✓
- Watch pairs and receives data ✓
- Pace and cadence match treadmill ✓

**Success indicators:**
- Build completes without errors
- Dashboard passes all checks
- ANT+ test works
- Application detects ANT+ dongle
- Watch connects and shows data

---

## Troubleshooting Build Issues

### Docker Permission Denied

**Error:** `permission denied while trying to connect to Docker daemon`

**Solution:**
```bash
# Add user to docker group
sudo usermod -aG docker $USER

# Apply group membership
newgrp docker

# Or logout and login again
```

**Verify:**
```bash
docker ps  # Should work without sudo
```

---

### Out of Memory Error

**Error:** `c++: fatal error: Killed signal terminated program cc1plus`

**Cause:** Docker container ran out of memory

**Solution:**
```bash
# Increase Docker memory limit (4GB+ recommended)
# Docker Desktop: Settings → Resources → Memory → 4GB or more

# Or build with reduced parallelism:
export MAKE_OPTS="-j2"  # Use only 2 CPU cores
./docker-build.sh --arch x86-64
```

---

### Runtime Issues After Building

**If application builds but doesn't work:**

**See runtime troubleshooting in [README.md](README.md#troubleshooting):**
- USB permissions
- Python library issues
- Treadmill configuration
- ANT+ dongle detection

---

## Build Performance Tips

### Speed Up Builds

**Use build cache:**
```bash
# Don't use --clean unless necessary
./docker-build.sh --arch x86-64
```

**Increase CPU cores:**
```bash
# Use more parallel jobs (if you have RAM)
export MAKE_OPTS="-j4"
./docker-build.sh --arch x86-64
```

**Use fast disk:**
- Build on SSD instead of HDD
- Use local disk instead of network storage

### Typical Build Times

| System | Architecture | Time |
|--------|-------------|------|
| Desktop (8 cores, SSD) | x86-64 | 15-25 min |
| Desktop (8 cores, SSD) | arm64 cross | 25-40 min |
| Desktop (4 cores, HDD) | x86-64 | 30-45 min |
| Laptop (4 cores, SSD) | x86-64 | 25-35 min |
| Raspberry Pi 4 | arm64 native | 60-90 min |

---

## Related Documentation

- **[README.md](README.md)** - Setup and usage after building
- **[TESTPLAN.md](TESTPLAN.md)** - Testing procedures
- **[Contributing Guidelines](CONTRIBUTING.md)** - How to contribute (if available)