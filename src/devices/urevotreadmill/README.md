# UREVO URTM047 (experimental)

Dedicated Bluetooth LE driver for the tested URTM047. This intentionally
does not replace the existing Horizon path for other URTM devices.

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

- All motor control is native FFF2 only; FTMS is used for optional
  telemetry and never as a control fallback.
- A speed target alone never starts the motor. Start is triggered only by an
  explicit QZ Start request, after initialization and recent idle feedback.
- Command writes are serialized; a lost handshake or write response does not
  trigger unlimited retries.
- A Stop is prioritized over queued Start/target packets. BLE write
  acknowledgement does not prove that the belt physically stopped.
- Initialization and frame formats are verified against one URTM047
  capture. The fixed delays reproduce the observed order but are not proof
  that every firmware variant will behave identically.
- No physical treadmill test has been performed. Keep emergency stop/safety
  key in reach and do not merge without a supervised on-device validation.
