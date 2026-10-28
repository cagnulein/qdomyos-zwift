# JK Fitness 10800 RFCOMM Implementation Plan

> **For Hermes:** implement task-by-task in the isolated `feat/jk-fitness-10800-bt` worktree.

**Goal:** Add first-pass QZ support for JK Fitness 10800 treadmills that expose the RunOnEarth/Pafers application protocol over Bluetooth Classic RFCOMM/SPP.

**Architecture:** Route the advertised `JK-10800` name to a dedicated treadmill driver only when the existing `BH SPADA Compatibility` setting is enabled. The setting enables QZ's combined `ClassicMethod | LowEnergyMethod` discovery. The driver discovers Serial Port Service `0x1101`, connects with `QBluetoothSocket::RfcommProtocol`, reassembles length-prefixed `0x55` frames, parses the source-derived 13-byte telemetry shape, and logs every raw frame. Control bytes are derived from the RunOnEarth APK and the existing PAFERS treadmill implementation; physical acceptance remains unverified until Marco tests the build.

**Tech Stack:** Qt 5 Bluetooth (`QBluetoothServiceDiscoveryAgent`, `QBluetoothSocket`), C++11, QTest/GoogleTest, QZ `treadmill` and `virtualtreadmill` classes.

---

## Evidence boundary

- Reported device name: `JK-10800-076`; model family: JK Fitness 10800.
- Companion APK analyzed: Run on Earth 1.6.0, package `com.pafers.runonearth3`.
- APK transport: Bluetooth Classic RFCOMM/SPP, UUID `00001101-0000-1000-8000-00805F9B34FB`.
- Source-derived application frame grammar: `55 <opcode> <payload_length> <payload>`.
- Source-derived control families: start/stop/pause `55 0A 01 xx`, speed `55 0F 02 <whole> <fraction>`, incline `55 11 01 <value>`; existing QZ PAFERS code supplies initialization and keep-alive frames.
- Reporter nRF Connect evidence: a separate BLE custom service `11110001-877a-45e8-b807-29775c6a0bb3`; no characteristic-level command capture is available. It is documented but not used to invent a BLE writer.
- Physical movement, command acknowledgement, and hardware compatibility are not proven by the APK or this source patch.

## Tasks

### Task 1: Add the pure JK/Pafers frame codec

**Files:**
- Create: `src/devices/jkfitness10800treadmill/jkfitness10800protocol.h`
- Create: `src/devices/jkfitness10800treadmill/jkfitness10800protocol.cpp`
- Test: `tst/Devices/TestJkFitness10800Protocol.cpp`
- Test: `tst/Devices/TestJkFitness10800Protocol.h`
- Modify: `tst/qdomyos-zwift-tests.pro`

Implement deterministic helpers for:

- `55` frame construction and stream reassembly using the third byte as payload length.
- Start, stop, pause, speed, incline, and keep-alive frames.
- Telemetry decoding from the source-derived 13-byte packet shape: speed at bytes 9/10, incline at byte 11, with the known stopped-speed sentinel handled conservatively.
- Console start/stop packet classification.

Verify with exact byte fixtures, fragmented/concatenated input, incomplete frames, invalid prefixes, fractional speed, clamping, and the stopped-speed sentinel.

### Task 2: Implement the RFCOMM treadmill driver

**Files:**
- Create: `src/devices/jkfitness10800treadmill/jkfitness10800treadmill.h`
- Create: `src/devices/jkfitness10800treadmill/jkfitness10800treadmill.cpp`
- Create: `src/devices/jkfitness10800treadmill/README.md`
- Modify: `src/qdomyos-zwift.pri`

Implement the driver using the existing Classic treadmill structure:

- Full service discovery with Serial Port Service fallback to UUID `0x1101`.
- Android no-security RFCOMM connection handling.
- Ordered source-derived initialization queue with bounded timeout advancement.
- Fragment-safe RX buffer and raw `>>`/`<<` debug logging.
- Telemetry parser, distance/metric updates, console start/stop events, optional virtual treadmill, start/stop/speed/incline control, and reconnect/disconnect cleanup.
- Do not claim protocol-level or physical acceptance in logs; label the setup and control writes clearly.

The README must record the thread-derived model/name, transport, frame table, the separate BLE service observation, the `BH SPADA Compatibility` prerequisite, source/test evidence, and unresolved hardware validation without personal email addresses or raw attachments.

### Task 3: Route the advertised device

**Files:**
- Modify: `src/devices/bluetooth.h`
- Modify: `src/devices/bluetooth.cpp`

Add a dedicated pointer/include and a first-match-safe branch for `JK-10800` before unrelated generic treadmill branches. Gate the branch on `bh_spada_2`, preserving the existing default behavior when the compatibility setting is off. Connect debug, connected, and disconnect/restart signals like the other Classic driver paths.

### Task 4: Add routing regression coverage

**Files:**
- Create: `tst/Devices/TestJkFitness10800Routing.cpp`
- Modify: `tst/qdomyos-zwift-tests.pro`

Verify that `JK-10800-076` selects `jkfitness10800treadmill` when `bh_spada_2=true`, and does not claim the device when the setting is false. Verify the name matcher is case-insensitive and does not broaden to unrelated `JK-` devices.

### Task 5: Verify locally and prepare the PR

Run:

```bash
git diff --check
qmake /root/work/qz-jk-fitness-10800/src/qdomyos-zwift.pro -o /root/work/qz-jk-fitness-10800-build/src/Makefile
make -C /root/work/qz-jk-fitness-10800-build/src -j1
```

Then build/run the focused protocol and routing tests from an out-of-tree test build with one job. Review the changed-file list, commit only the scoped files, push the branch, create the PR, and read back the PR metadata/body and CI state. A GitHub Actions APK artifact is a separate remote build result; do not call it available until the workflow exposes and verifies an artifact URL.

### Task 6: User test instructions after a verified artifact

Provide the PR/Actions/artifact links only after remote verification. The test instructions must say:

1. Install the debug APK from the GitHub Actions artifact.
2. Turn on `BH SPADA Compatibility` under Settings and restart QZ; this enables full Classic + BLE discovery.
3. Turn on `Debug Log` and restart QZ.
4. Power on the treadmill, leave the phone disconnected from RunOnEarth/other controller, and let QZ search for `JK-10800-076`.
5. Test connection/telemetry first; only then test start/stop/speed/incline while ready to stop the belt manually.
6. If it fails, reproduce once, stop, then send the newest `debug-*.log` from the QZ documents folder with the exact device name and the approximate test action/time. Do not send passwords or unrelated personal files.
