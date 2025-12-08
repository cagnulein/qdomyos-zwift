# Compiling QDomyos-Zwift with ANT+ Support

This guide explains how to compile QDomyos-Zwift from source using Docker. **For most users, [pre-compiled binaries](README.md) are recommended.**

**📌 TL;DR:** Use `./docker-build.sh --arch x86-64` (or `arm64`) to build from source.

**Return to main guide:** [README.md](README.md)

---

## Docker Build Method

**This is the primary build method** used to create GitHub releases and provides the most reliable compilation experience.

The Docker build method:
- ✅ Works on any Linux system with Docker installed
- ✅ Zero build tools needed on host system  
- ✅ Produces identical binaries to GitHub releases
- ✅ Handles all dependencies automatically
- ✅ Supports both x86-64 and ARM64 (Raspberry Pi)

### Prerequisites

```bash
# Install Docker
sudo apt-get update
sudo apt-get install -y docker.io docker-buildx
sudo usermod -aG docker $USER
newgrp docker  # Or logout/login

# Clone repository
git clone https://github.com/cagnulein/qdomyos-zwift.git
cd qdomyos-zwift
```

### Build for Desktop Linux (x86-64)

```bash
cd src/devices/antlinux
./docker-build.sh --arch x86-64
```

Output: `linux-binary-x86-64-ant.zip` in project root

### Build for Raspberry Pi (ARM64)

```bash
cd src/devices/antlinux
./docker-build.sh --arch arm64
```

Output: `linux-binary-arm64-ant.zip` in project root

### Extract and Test

```bash
cd ../../..  # Back to project root
unzip linux-binary-x86-64-ant.zip -d ~/
cd ~/qdomyos-zwift-x86-64-ant

# Verify setup
./setup.sh --quick

# Test ANT+ hardware
sudo ./setup.sh --test

# Run application
./qdomyos-zwift
```

**That's it!** The Docker build handles all Qt, Python, and dependency complexities.

---

## Build Verification

### Success Indicators

✓ Build completes without errors  
✓ `setup.sh --quick` shows all tests passing  
✓ Application starts and detects ANT+ dongle  
✓ Watch connects and displays stable pace/cadence

### Troubleshooting

| Issue | Solution |
|-------|----------|
| Docker permission denied | Run `newgrp docker` or logout/login after adding user to docker group |
| Build fails with OOM | Increase Docker memory limit in Docker Desktop settings |
| Invalid architecture specified | Use `x86-64` or `arm64` only |
| Binary won't run on target | Ensure architecture matches (x86-64 vs ARM64) |

See [main README troubleshooting](README.md#troubleshooting) for runtime issues.

---

**Return to [main README](README.md)**