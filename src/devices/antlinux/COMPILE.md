# Compiling QDomyos-Zwift with ANT+ Support

**For most users:** [Pre-compiled binaries](README.md) are the easiest option. This guide is for developers or users who need to build from source.

## Quick Start

```bash
cd src/devices/antlinux
./docker-build.sh --arch x86-64  # or --arch arm64 for Raspberry Pi
```

That's it! The script handles everything automatically.

**Need the main guide?** [README.md](README.md)

---

## Why Docker Build?

The Docker method is **the recommended way** to compile QDomyos-Zwift:

- **Works anywhere** - Any Linux system with Docker installed
- **Zero host dependencies** - No build tools needed on your system
- **Reproducible builds** - Same binaries as GitHub releases
- **Automatic dependencies** - Qt, Python, and libraries handled for you
- **Cross-platform** - Build x86-64 and ARM64 from the same machine

---

## Prerequisites

### Install Docker

```bash
sudo apt-get update
sudo apt-get install -y docker.io docker-buildx

# Add your user to docker group
sudo usermod -aG docker $USER

# Activate group (or logout/login)
newgrp docker
```

### Clone Repository

```bash
git clone https://github.com/cagnulein/qdomyos-zwift.git
cd qdomyos-zwift
```

---

## Building

### Desktop Linux (x86-64)

```bash
cd src/devices/antlinux
./docker-build.sh --arch x86-64
```

**Output:** `linux-binary-x86-64-ant.zip` in project root

### Raspberry Pi (ARM64)

```bash
cd src/devices/antlinux
./docker-build.sh --arch arm64
```

**Output:** `linux-binary-arm64-ant.zip` in project root

> **Tip:** You can build ARM64 binaries on an x86-64 machine! Docker handles cross-compilation automatically.

---

## Testing Your Build

### Extract and Run

```bash
# Return to project root
cd ../../..

# Extract the package
unzip linux-binary-x86-64-ant.zip -d ~/
cd ~/qdomyos-zwift-x86-64-ant
```

### Verify Everything Works

```bash
# Check system requirements
./setup.sh --quick

# Test ANT+ hardware independently
sudo ./setup.sh --test

# Launch the application
sudo ./qdomyos-zwift -no-gui -ant-footpod
```

---

## Build Verification

**Your build is successful when:**
- Build completes without errors
- `./setup.sh --quick` passes all tests
- Application detects your ANT+ dongle
- Watch connects and shows stable pace/cadence

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| `Docker permission denied` | Run `newgrp docker` or logout/login after adding user to docker group |
| Build fails with out-of-memory error | Increase Docker memory in Docker Desktop settings (4GB+ recommended) |
| `Invalid architecture specified` | Use `--arch x86-64` or `--arch arm64` only |
| Binary won't run on target system | Verify architecture matches your hardware (use `uname -m` to check) |
| Build succeeds but runtime issues | See [main README troubleshooting section](README.md#-troubleshooting) for setup and runtime problems |

---

## Next Steps

Now that you have a compiled binary:

1. **Follow the main guide:** [README.md](README.md) for complete setup instructions
2. **Install dependencies:** Run `sudo ./setup.sh --guided` for automatic setup
3. **Configure and run:** See Step 4 in the main README

---

**Return to [main README](README.md)**