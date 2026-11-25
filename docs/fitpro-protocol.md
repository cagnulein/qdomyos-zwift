# FitPro BLE protocol foundation

This is an offline, transport-independent C++11 implementation. No production
driver calls it yet. No settings, device routing, BLE writes, polling, or
existing treadmill behavior change.

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

## Next steps before any runtime integration

1. Replay more complete discovery exchanges from known working legacy models;
   compare candidates against transmitted payloads AND accepted responses.
2. Investigate equipment-family differences (including 0x07), checksum variants
   and the upstream model-specific correction. Do not assume every console
   uses the same handshake or command family.
3. Add a streaming notification assembler with timeouts and response matching;
   collect field capabilities and machine limits; validate arming and actual
   speed/incline readback. Keep existing QZ safety limits.
4. Only after offline validation, consider an opt-in experimental mode,
   disabled by default. Preserve legacy routing. Never switch or retry control
   strategies automatically while a belt is moving; hardware-test start,
   stop, disconnect and physical controls before wider rollout.
