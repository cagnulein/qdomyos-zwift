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

The codec uses the declared payload length to reassemble complete frames. Incomplete frames remain in the buffer; complete unknown frames are logged and ignored by the driver; malformed length values are discarded one byte at a time.

## Source-derived frame table

| Purpose | Frame | Evidence/status |
| --- | --- | --- |
| Start | `55 0A 01 01` | RunOnEarth/QZ PAFERS source; hardware acceptance unverified |
| Stop | `55 0A 01 02` | RunOnEarth/QZ PAFERS source; hardware acceptance unverified |
| Pause | `55 0A 01 00` | APK source; hardware acceptance unverified |
| Speed | `55 0F 02 <whole> <fraction>` | APK source exposes two speed bytes; QZ sends hundredths in the second byte |
| Incline | `55 11 01 <value>` | APK/QZ PAFERS source; unit and limit are not independently documented |
| Connection heartbeat | `55 17 01 01` immediately and every 1000 ms | Run on Earth 1.6.0 source |
| Telemetry request | `55 07 01 FF` | Run on Earth 1.6.0 source |
| Connection discovery | 27 queries, in source order, each `55 <opcode> 01 FF`: `0C BB 24 25 26 27 02 03 04 06 1F A0 B0 B2 B3 B4 B5 B6 B7 B8 B9 BA 0B 18 19 1A 1B` | Run on Earth 1.6.0 source |
| Explicit start, type 2 | reset `55 0A 01 02`; profile `55 01 06 <age> <sex> <weight> <height>`; option `55 15 01 00`; speed 1.0 and current incline; `55 08 01 01`; wait for running state; wait 3 s; `55 0A 01 01`; telemetry request | Native type-2 session sequence; profile values come from QZ age/sex/weight/height settings |

After RFCOMM connects, QZ sends the native heartbeat immediately, waits 700 ms, then sends the native 27-query burst. Heartbeat continues independently during discovery. A `55 BB 01 <type>` response negotiates the device type; only type 2 is treated as a treadmill. QZ waits for both type and `55 0C 01 <units>` before exposing controls. `55 BB 00` causes the native type query to be retried. Other events, including `55 0B 01 01` by itself, do not establish readiness. Connection setup sends no motion or profile writes. The native session sequence starts only after an explicit user start; speed and profile values are converted to the negotiated units.

## Telemetry currently decoded

The source handles telemetry opcode `0x0D` with type-specific frame sizes: 13 total bytes for type 2 and 16 total bytes for types 0/1. Opcode `0x1D` uses 13 total bytes for type 2 and 10 for types 0/1. The decoder requires the opcode and declared payload length to match and uses each type/opcode's source-derived offsets. It decodes:

- treadmill speed: raw bytes 9–10, whole value plus hundredths;
- speed `1.00` as the actual reported speed;
- treadmill incline: raw byte 11.

Speed units are converted from the device's non-metric mode to QZ's km/h convention. Console state `55 09 01 01` means running, `55 09 01 00` means paused, and `55 09 01 02` means stopped. The type-2 `55 0B 01 01` event reports standby/stop only after type 2 has been negotiated. Raw RX chunks and parsed frames, TX frames, and short socket writes are logged for diagnosis.

## Test boundary

Confirmed by source/static tests:

- RFCOMM/SPP transport and Serial Port UUID;
- native heartbeat, query burst, device type and units parsing, and stream reassembly;
- source-derived control bytes;
- type-specific telemetry frame validation, 1.00 speed preservation, and console state markers;
- routing name and `BH SPADA Compatibility` gate.

Still unverified until a JK Fitness 10800 owner runs this build:

- whether the physical JK device accepts the RunOnEarth/Pafers initialization over RFCOMM;
- whether the device returns the expected type and units discovery replies;
- exact telemetry units, limits, and physical response timing;
- whether the native type-2 start sequence is accepted by this model;
- physical start, stop, speed, and incline effects;
- whether the reported BLE custom service is the actual control endpoint instead of, or in addition to, the Classic endpoint.

When the first debug log is available, compare the complete ordered `>>`/`<<` exchange before changing commands or parser offsets. Do not use a successful socket connection alone as compatibility proof.
