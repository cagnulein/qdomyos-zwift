# Compiling QDomyos-Zwift with ANT+ Support

This guide is for users who need to build from source, such as for custom modifications, development, or unsupported systems. Most users should use pre-compiled binaries—see the [README](README.md) for details.

**Note:** For safety, reset/uninstall functionality is not provided. If you need to remove QDomyos-Zwift or its components, please do so manually.

---

## Why Use Docker?

Docker provides a consistent, isolated build environment with all required dependencies. This avoids host system configuration issues and ensures reproducible builds for both x86-64 and ARM64 targets.

---

## Prerequisites

- Linux system (x86-64 or ARM64)
- Docker (version 20.10+)
- Git
- ~10GB free disk space

Install Docker (Ubuntu/Debian):
```bash
sudo apt-get update
sudo apt-get install -y docker.io docker-buildx
sudo usermod -aG docker $USER
newgrp docker  # Or logout/login
```

---

## Quick Start

1. Ensure Docker is installed and running.
2. Clone the repository:
	```bash
	git clone https://github.com/cagnulein/qdomyos-zwift.git
	cd qdomyos-zwift
	```
3. Build for your target architecture:
	```bash
	cd src/devices/antlinux
	./docker-build.sh --arch x86-64   # For desktop Linux
	./docker-build.sh --arch arm64    # For Raspberry Pi
	```
	The output will be a `.zip` file in the project root.

---

## Testing Your Build

Extract the package and verify:
```bash
cd ../../..  # From src/devices/antlinux
unzip linux-binary-x86-64-ant.zip -d ~/
cd ~/qdomyos-zwift-x86-64-ant
./setup.sh --check
sudo ./setup.sh --test
sudo ./qdomyos-zwift -no-gui -ant-footpod
```

Your build is successful if:
- Build completes without errors
- `./setup.sh --check` passes
- Application detects your ANT+ dongle
- Watch connects and shows data

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Docker permission denied | Run `newgrp docker` or logout/login after adding user to docker group |
| Out-of-memory error | Increase Docker memory (4GB+ recommended) |
| Invalid architecture | Use `--arch x86-64` or `--arch arm64` only |
| Binary won't run | Check architecture with `uname -m` |
| Runtime issues | See [README troubleshooting](README.md#troubleshooting) |

---

## Next Steps

Refer to the [README](README.md) for setup, configuration, and usage instructions after building.