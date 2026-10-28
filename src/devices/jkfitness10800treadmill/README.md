# JK Fitness 10800 treadmill

## Scope

This driver is the first hardware-validation implementation for the JK Fitness 10800 family reported with the Bluetooth name `JK-10800-076`. It intentionally keeps the protocol evidence and the hardware-validation boundary separate: a valid RFCOMM write or a changing notification is not proof that the belt accepted a control command.

## Evidence used

- A support thread reports a JK Fitness 10800 treadmill purchased in 2014 and the advertised name `JK-10800-076`.
- The legacy companion application is Run on Earth 1.6.0 (`com.pafers.runonearth3`). The APK was analyzed locally; SHA-256: `e92f39f8220642c87a4eae19e52b2e59ab11aec4ba7c122b1d4615a7edb38cf5`. The binary is not committed here.
- The companion app opens Bluetooth Classic RFCOMM/SPP using the standard Serial Port UUID:

  `00001101-0000-1000-8000-00805F9B34FB`

- The nRF Connect screenshots for the reported machine also show a separate custom BLE service:

  `11110001-877a-45e8-b807-29775c6a0bb3`

  No characteristic-level command capture was supplied for that BLE service. This driver therefore uses the companion-app transport (Classic RFCOMM) and does not send speculative BLE writes.

## QZ discovery prerequisite

Enable **Settings → BH SPADA Compatibility**, then restart QZ. This existing setting enables the combined `ClassicMethod | LowEnergyMethod` discovery path; without it, a Classic-only JK device may never reach this driver. The driver itself is selected only for a case-insensitive `JK-10800` name prefix.

Enable **Settings → Debug Log** and restart QZ before a hardware test. The resulting `debug-*.log` file contains the driver-level RFCOMM discovery, setup, TX, RX, and socket-error lines needed for the next protocol iteration.

## Transport and stream framing

The driver discovers the Serial Port service with `QBluetoothServiceDiscoveryAgent` and falls back to direct UUID `0x1101` connection. It uses `QBluetoothSocket::RfcommProtocol` and a stream buffer because RFCOMM can fragment or concatenate application frames.

Application frames have the source-derived form:

```text
55 <opcode> <payload length> <payload>
```

The codec uses the declared payload length to reassemble complete frames. Unknown or incomplete frames remain in the buffer; malformed length values are discarded one byte at a time.

## Source-derived frame table

| Purpose | Frame | Evidence/status |
| --- | --- | --- |
| Start | `55 0A 01 01` | RunOnEarth/QZ PAFERS source; hardware acceptance unverified |
| Stop | `55 0A 01 02` | RunOnEarth/QZ PAFERS source; hardware acceptance unverified |
| Pause/reset | `55 0A 01 00` | APK source; hardware acceptance unverified |
| Speed | `55 0F 02 <whole> <fraction>` | APK source exposes two speed bytes; QZ sends hundredths in the second byte |
| Incline | `55 11 01 <value>` | APK/QZ PAFERS source; unit and limit are not independently documented |
| Keep-alive/poll | `55 0D 0A FF FF FF FF FF FF FF FF FF FF` | Existing QZ PAFERS implementation |
| Setup | `55 BB 01 FF`, `55 0C 01 FF`, `55 1F 01 FF`, then reset/configuration frames | Existing QZ PAFERS implementation, ported to RFCOMM |

The implementation advances through setup frames after a response or a bounded 300 ms timeout. This is deliberately diagnostic-friendly, but it is not yet a proof that the native app uses the same response timing. A timeout-only probe never marks the device ready: QZ keeps controls and polling disabled until a recognized console-state or telemetry frame is received.

## Telemetry currently decoded

The known PAFERS telemetry shape is 13 bytes. The driver accepts a `0x55` frame of that length and decodes:

- speed: raw bytes 9–10, whole value plus hundredths;
- stopped-speed sentinel: raw speed `1.00` is treated as `0.0`, matching the existing PAFERS driver;
- incline: raw byte 11.

Short console state frames `55 09 01 01` and `55 09 01 00` are logged and exposed as hardware start/stop events. Other packets are preserved in the debug log but are not interpreted as metrics.

## Test boundary

Confirmed by source/static tests:

- RFCOMM/SPP transport and Serial Port UUID;
- frame construction and stream reassembly;
- source-derived control bytes;
- 13-byte parser offsets and console state markers;
- routing name and `BH SPADA Compatibility` gate.

Still unverified until a JK Fitness 10800 owner runs this build:

- whether the physical JK device accepts the RunOnEarth/Pafers initialization over RFCOMM;
- exact response matching and setup timing;
- telemetry units/limits on this model;
- physical start, stop, speed, and incline effects;
- whether the reported BLE custom service is the actual control endpoint instead of, or in addition to, the Classic endpoint.

When the first debug log is available, compare the complete ordered `>>`/`<<` exchange before changing commands or parser offsets. Do not use a successful socket connection alone as compatibility proof.
