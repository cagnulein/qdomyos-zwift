
# ANT+ Feature Test Plan

This document provides concise testing procedures for the ANT+ footpod broadcasting feature. It is intended for contributors, end users, and QA. Use as a reference for validating changes, verifying installations, or release testing.

---

## Test Flow Overview

1. **System Validation**: Check all dependencies and environment.
2. **ANT+ Hardware Test**: Verify dongle and watch pairing.
3. **Application Test**: Test with treadmill in desktop or server mode.

---

## 1. System Validation

Run:
```bash
./setup.sh --check
```
Success Criteria:
- All checks pass (Python, Qt5, QML, plugdev, udev, dongle, Bluetooth)
- System status: READY

If any check fails, run:
```bash
sudo ./setup.sh --gui
```

---

## 2. ANT+ Hardware Test

Run:
```bash
sudo ./setup.sh --test
```
Success Criteria:
- Watch pairs as footpod within 10 seconds
- Pace and cadence update in real time
- Distance accumulates
- No USB errors

---

## 3. Application Test

**Desktop Mode:**
```bash
sudo ./qdomyos-zwift -ant-footpod
```
**Server Mode:**
```bash
sudo ./qdomyos-zwift -no-gui -ant-footpod
```
Success Criteria:
- Application starts and connects to treadmill
- Watch pairs and receives data
- Pace and cadence match treadmill
- No disconnections or crashes

---


## Troubleshooting

Refer to the troubleshooting section in the [README.md](README.md#troubleshooting) for common issues and solutions, including:
- USB errors or resource busy
- GUI not starting
- Watch not pairing
- Data mismatch
- Python library errors
- Treadmill not configured

---

## Validation Checklist

- [ ] System validation passes
- [ ] ANT+ hardware test passes
- [ ] Application test (desktop or server) passes
- [ ] No critical errors or crashes
- [ ] Watch data matches treadmill

---

## Related Documentation

- [User Setup](README.md)
- [Build Instructions](COMPILE.md)
- Architecture: See `AntManager.cpp` and `AntWorker.cpp`
