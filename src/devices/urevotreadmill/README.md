# UREVO URTM family (experimental)

The dedicated Bluetooth LE driver is the **default for all devices whose
Bluetooth name starts with `URTM`**, case-insensitively (URTM047, URTM024,
etc.). It has precedence over the Horizon driver.

**Manual FTMS fallback:** Select the actual URTM device in the existing
**FTMS Treadmill** setting. When `ftms_treadmill` matches that device's full
Bluetooth name (case-insensitively), QZ bypasses `urevotreadmill` and
uses `horizontreadmill`. Unselecting that device restores native UREVO
routing. No automatic switch is attempted after a native connection error.

The proprietary handshake was captured only from URTM047; it is not yet
confirmed on every URTM firmware. The manual fallback makes it possible to
keep using Horizon/FTMS for incompatible models.

## Source trace

The reference is the original UREVO iOS application's 2026-11-06 PacketLogger
HCI captures (bluetoothd-hci-2026-11-06_20-29-27.pklg and subsequent
bluetoothd-hci-latest.pklg). The raw capture is not checked into the
repository because it includes other unrelated Bluetooth devices and private
traffic.

In this capture, ATT handles map to:

| Service | Characteristic | HCI handle | Direction |
| --- | --- | --- | --- |
| FFF0 | FFF1 | 0x001d | notify |
| FFF0 | FFF2 | 0x0020 | write-with-response |
| FEE0 | FEE1 | 0x0023 | notify |
| FEE0 | FEE2 | 0x0026 | write-with-response |

The captured first session included enabling FFF1 and FEE1 notifications,
02 51 0B 03 -> 02 51 00 02 09 03,
02 50 03 09 03 -> 02 50 03 5A 00 5B 52 03, and
5A A5 01 03 00 01 10 00 00 CC 76 -> FEE1 response.
Before the physical Start it repeated the same three-command handshake.
The Start was sent twice, with 02 40 02 18 03 between them and
02 50 02 08 03 after the second Start's acknowledgement. Movement
was confirmed only by subsequent full FFF1 telemetry.

The companion GTest suite tst/ToolTests/urevoprotocoltests.cpp replays those
exact characteristic payloads, including 2.0 mph and 4% incline, stop
preemption, invalid checksums, missing acknowledgements, and reconnect reset.

## Safety and test status

- Inside the native UREVO driver, motor control is FFF2 only; FTMS is
  optional read-only telemetry. The **user-selected FTMS Treadmill fallback**
  instead routes the whole device to the existing Horizon driver.
- A speed target alone never starts the motor. Start is triggered only by an
  explicit QZ Start request, after initialization and recent idle feedback.
- Command writes are serialized; a lost handshake or write response does not
  trigger unlimited retries.
- A Stop is prioritized over queued Start/target packets. BLE write
  acknowledgement does not prove that the belt physically stopped.
- Initialization and frame formats are verified against one URTM047
  capture. The fixed delays reproduce the observed order but are not proof
  that every firmware variant will behave identically.
- The discovery regression suite covers default native URTM routing and
  explicitly selected FTMS/Horizon fallback on multiple URTM names.
- No physical treadmill test has been performed. Keep emergency stop/safety
  key in reach and do not merge without a supervised on-device validation.
