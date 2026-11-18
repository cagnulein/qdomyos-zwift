# JK Fitness 10800 RFCOMM treadmill

Experimental dedicated driver for `JK-10800` names, selected only with BH SPADA Compatibility enabled. This setting enables Classic Bluetooth discovery; PAFERS BLE overrides are not required.

## Native startup

Based on static analysis of RunOnEarth 1.6.0 (`com.pafers.runonearth3`, SHA-256 `e92f39f8220642c87a4eae19e52b2e59ab11aec4ba7c122b1d4615a7edb38cf5`):

- Connect insecure RFCOMM/SPP UUID `00001101-0000-1000-8000-00805F9B34FB`.
- Keep `55 17 01 01` running every second.
- After 700 ms, send the 27 native discovery queries in their original order without per-query ACK waits.
- Allow 2000 ms to settle, then expose controls for protocol type 2. Missing discovery replies retain the APK defaults: type 2 and units 0. Logs distinguish defaults from reported values.
- Valid type/unit replies override defaults. A reported non-treadmill type disables controls and cancels delayed start writes. Empty type replies request a bounded retry.

The APK defaults are source-backed behavior, not physical confirmation of this model's units. Hardware validation is still required. RX `0xC0` and `0x1E` have no demonstrated handler in the analyzed APK and are logged without invented acknowledgement writes.

## Explicit Start

Connection does not start the belt. Requests queued before readiness are discarded. A subsequent user Start sends the type-2 reset/profile/mode/speed/incline/enable sequence, spaced by 600 ms. QZ arms the running-state wait before writing `55 08 01 01`. A valid `55 09 01 01` continues the sequence after 3 seconds, then sends Start and a telemetry query.

Unlike the APK's infinite wait, QZ cancels after 10 seconds without running confirmation and sends Stop. Stop requests cancel delayed start work. Unknown packets cannot unlock this wait.

## Regression checks

Protocol tests cover source-derived byte frames, stream reassembly, type/unit decoding and telemetry shapes. Driver state tests cover default readiness after settling, ignored unknown/status packets, reported units, reset, incompatible reported types and start timeout cancellation. No physical-device success is claimed by these tests.

## Owner test

Enable Debug Log, restart QZ, connect with the belt stopped and wait for startup. Verify whether telemetry appears, then test explicit Start/Stop with the safety key available. Send the complete new debug log and the approximate button times. The console may disable its physical keys while in smartphone mode, so console-key tests after pairing are not required.
