# FitPro BLE protocol foundation

The transport-independent C++11 implementation now has an opt-in BLE adapter.
Choose **ProForm/NordicTrack Generic (Experimental)** under Treadmill Options
→ Proform/Nordictrack Options → Specific Model, then restart QZ. The default
Other and all existing model profiles retain the previous behavior.

The generic adapter targets the device-04 treadmill family. Discovery,
limits and capture-verified polling must succeed before control. Explicit QZ
Start unlocks and arms; speed/incline writes are limited to workouts started
by QZ, and to the machine and QZ safety ceilings. It starts conservatively at
1–2 km/h (if allowed) and zero incline. Stop/pause use workout-mode writes;
no automatic legacy fallback, movement retries, or workout resume on reconnect.
Use the safety key and stand beside the belt when first testing. A communication
failure requires physical STOP; software cannot guarantee stopping a disconnected
machine. Unknown firmware, physical START takeover, and additional families
are not yet supported.

## Attribution

The unlock computation is adapted from [OpenFit UnlockCalculator.kt](https://github.com/BaconWappedBitcoin/openfit/blob/main/android/protocol/src/main/kotlin/com/ofit/protocol/UnlockCalculator.kt)
(BaconWappedBitcoin/openfit, GPL-3.0-or-later). Its [unit test](https://github.com/BaconWappedBitcoin/openfit/blob/main/android/protocol/src/test/kotlin/com/ofit/protocol/UnlockCalculatorTest.kt)
provides the T6.5 capture. See also [OpenFit protocol notes](https://github.com/BaconWappedBitcoin/openfit/blob/main/android/docs/protocol-notes.md).
The frame, checksum and notification padding rules are cross-checked against
QZ's existing ProForm treadmill driver and captures.

## Evidence

| Capture | Result |
| --- | --- |
| OpenFit NordicTrack T6.5, 2026-11-23 | Computed payload and all unlock BLE chunks match existing QZ T6.5S profiles |
| QZ T Series 8, 2026-11-12 | Computed payload and all chunks match the QZ capture; console replied to VerifySecurity with status 0x02 |
| QZ capture, 2026-10-30 | Candidate differs from legacy device-07 payload; no successful unlock established |
| QZ capture, 2026-10-08 | Candidate differs from legacy device-07 payload; no successful unlock established |

Only relevant protocol bytes are included, not full logs, serial-number
queries, Bluetooth addresses, or customer details. The latter two captures
are negative comparisons, not evidence that the new candidate works or that
the existing driver is wrong. The upstream model-specific correction remains
unverified independently. The tests are in the regular qdomyos-zwift-tests
target; no new workflow is needed.

## Next steps before wider rollout

1. Replay more complete discovery exchanges from known working legacy models;
   compare candidates against transmitted payloads AND accepted responses.
2. Investigate equipment-family differences (including 0x07), checksum variants
   and the upstream model-specific correction. Do not assume every console
   uses the same handshake or command family.
3. Validate the opt-in session on real treadmills: arming, start, stop,
   pause, readback, manual operation, and disconnect/reconnect. The adapter
   rejects unexpected status or telemetry lengths. Do not enable by default.
4. Add wider field-capability negotiation and hardware coverage before expanding
   to other equipment families.
