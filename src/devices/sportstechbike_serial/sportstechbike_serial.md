# Sportstech v7.4.4 APK Reverse-Engineering Notes (Serial Path)

## Scope
This note summarizes what was inferred from the `sportstech_v744.apk` protocol behavior and integrated into `sportstechbike_serial`.

The focus is the **bike serial protocol on Android** (tablet-integrated setup), not BLE discovery logic.

## High-level findings
- The bike protocol uses short binary frames starting with `0xF2`.
- The same application-level packets used by the BLE Sportstech bike path are reusable over a serial transport.
- On Android, serial I/O can be bridged through the app JNI helper (`Usbserial`), which is already used by other serial devices in QZ.

## Transport assumptions
- Physical transport: USB serial (through Android host + JNI bridge).
- Byte stream framing is done in software.
- Serial port selection in current QZ Android bridge is global (`Usbserial.open(context)`), so the setting acts as an enable trigger.

## Frame format
Observed/used frame structure:
- Byte 0: `0xF2` (sync)
- Byte 1: command group / command id
- Byte 2: payload length `N`
- Bytes 3..(3+N-1): payload
- Last byte: checksum

Frame length rule:
- `frame_len = payload_len + 4`

Checksum rule:
- 8-bit sum of all bytes except checksum, modulo 256
- Expected: `sum(frame[0..len-2]) == frame[len-1]`

## Initialization sequence
Used init sequence (same logical protocol as BLE Sportstech implementation):
1. `f2 c0 00 b2`
2. `f2 c1 05 01 ff ff ff ff b5`
3. `f2 c4 0d 00 00 00 00 00 00 00 00 00 00 ff ff ff c0`
4. `f2 c3 07 01 00 00 00 00 ff ff bb`

## Poll / resistance write packet
Periodic packet template:
- `f2 c3 07 04 rr 00 00 00 ff ff cc`

Where:
- `rr` = requested resistance level
- `cc` = checksum byte updated after setting resistance

Resistance range used:
- Standard Sportstech profile: up to 24
- ESX500 profile: up to 10 (based on existing QZ setting logic)

## Telemetry packet parsing (20-byte frame)
Current decode offsets:
- Elapsed minutes: byte 3
- Elapsed seconds: byte 4
- Kcal: bytes 7..8 (big-endian)
- Watt: bytes 9..10 (big-endian)
- Heart rate: byte 11
- Speed: bytes 12..13 (big-endian, /10)
- Resistance: byte 15
- Cadence: byte 17

## Why a separate class (`sportstechbike_serial`)
- Keeps existing BLE class (`sportstechbike`) untouched.
- Isolates transport-specific behavior (serial read/write + stream framing).
- Reuses metric mapping and protocol semantics where possible.

## Confidence and caveats
- High confidence on packet format and command bytes (derived from existing app behavior and reused protocol path).
- Medium confidence on serial-device routing details, because Android bridge currently exposes a generic open/read/write API without explicit per-device path binding.
- If the vendor firmware changes packet layout, offsets may require adjustment.

## Practical troubleshooting
If no telemetry arrives:
- Ensure Android USB permission is granted.
- Confirm JNI `Usbserial` bridge is active.
- Enable debug logs and verify outgoing init packets and incoming `0xF2` frames.
- Check checksum mismatches in logs; persistent mismatch usually means framing/stream alignment issue.

## Related QZ files
- `src/devices/sportstechbike_serial/sportstechbike_serial.h`
- `src/devices/sportstechbike_serial/sportstechbike_serial.cpp`
- `src/devices/bluetooth.cpp` (device selection + instantiation)
- `src/qzsettings.h/.cpp` (setting key)
- `src/settings.qml` (UI setting)
