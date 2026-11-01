# iConsole treadmill workout upload

## Scope

QZ can prepare a first experimental speed profile for BLE treadmills exposed as `I-CONSOLE+` or `ICONSOLE+`. The profile is built from the same `trainrow` list used by imported XML, imported ZWO, and Workout Editor workouts.

This is not a live speed command. The available capture shows iConsole+ loading a speed table before the workout and the treadmill applying the table internally. QZ still does not claim that a separate immediate speed command has been identified for this device.

## Capture evidence

The reference capture from the Cardiostrong TX 90 contains these writes:

```text
F0 A6 01 D3 01 29 01 3D 01 51 02 01 02 15 02 29 02 15 02 01 01 51 01 3D 13
F0 A7 01 D3 01 29 01 3D 01 51 02 01 02 15 02 29 02 15 02 01 01 51 01 3D 14
```

The BLE transport splits each 25-byte logical frame into a 20-byte write followed by a 5-byte write. Both commands contain the same ten speed values:

```text
4, 6, 8, 10, 12, 14, 12, 10, 8, 6 km/h
```

The last byte is the sum of all preceding bytes modulo 256. The observed profile changes approximately once per minute. The supported input is exactly ten minutes, with whole-minute fixed-speed rows. QZ maps each input minute to two 30-second slots. Other durations and fractional-minute boundaries are rejected rather than rescaled, so the treadmill profile and QZ workout clock describe the same timeline.

The capture also records `F0 A3`, `F0 A4`, `F0 A8`, `F0 A9`, `F0 AA`, and `F0 AF` around the speed frames. The candidate Android app's handlers associate these with workout mode, parameters, inclination, duration, and key-lock. The captured parameter frame uses `0x15`; the Android builders encode each data byte with `+1` for `A8/A9/AA`. QZ sends those profile frames in the observed order and checks the corresponding `B3/B4/B6/B7/B8/B9/BA/BF` response frame, including its length and checksum. The APK analyzed is a mirror candidate and has not been proven to match the captured app build. Their complete firmware semantics have not been confirmed on a physical TX 90. `F0 A5` remains a separate workout-control command and is sent only for the explicit Start Workout action, after the profile upload has been acknowledged.

## QZ behavior

When a treadmill workout is associated with a treadmill device, QZ calls the treadmill program hook with the parsed `QList<trainrow>`.

For an iConsole treadmill:

1. QZ requires a non-empty workout totaling exactly ten minutes, with positive whole-minute, non-ramp rows and finite, nonnegative encodable speeds.
2. QZ reads each of the ten input minutes at its midpoint. Each minute contributes two 30-second speed and inclination slots; missing inclination is encoded as zero.
3. Speed uses the captured representation: `deci-km/h = round(speed * 10)`, then high and low bytes are each incremented by one. `A8/A9/AA` data bytes are incremented by one as in the candidate Android builders; `A4` uses the captured `0x15` parameter value.
4. Before the profile, QZ sends the captured `A0 00 00`, `A0 01 D3`, and `A1 01 D3` initialization frames and waits for valid `B0/B1` responses. If initialization fails, QZ retries the sequence after one second, with at most three attempts total; retries stop if the BLE connection is no longer discovered.
5. QZ creates the ordered profile frames `F0 A3`, `F0 A4`, `F0 A6`, `F0 A7`, `F0 A8`, `F0 A9`, `F0 AA`, and `F0 AF`.
6. After initialization, each frame is written in chunks of at most 20 bytes, with no polling between fragments. QZ waits for the matching complete response frame and checksum after each command; a missing or invalid response stops the active batch. If a newer workout request arrives during transfer, QZ stops the active batch and proceeds with the newest valid queued profile; after an ACK failure it reinitializes before retrying that profile. A rejected latest request keeps Start blocked. A Start received while the profile is queued is retained until upload succeeds, while Stop cancels a deferred Start.
7. After the transaction, the captured iConsole poll `F0 A2 01 D3 66` resumes. iConsole telemetry does not trigger Start. The separate `F0 A5` Start command is sent only when the user requests Start Workout, and only after successful profile acknowledgements; QZ waits for `B5` before starting its workout clock.

Other AppGate treadmill variants keep their existing behavior and do not receive these frames.

## Current limitations

- The experimental device profile supports exactly ten minutes with whole-minute rows. Unsupported durations and interval boundaries are rejected; QZ never stretches or compresses the requested timeline.
- Ramp rows, zero/negative-duration rows, and non-finite speeds are rejected.
- The speed, inclination, and duration field mapping follows the candidate Android app and capture, but has not yet been confirmed by a physical TX 90 test.
- The live speed function remains unimplemented because the available evidence does not prove a live command.
- The implementation has not yet been validated on physical TX 90 hardware. Matthias reported that the treadmill moved only after pressing Start on the console; upload acknowledgements do not prove belt movement.

A first hardware test should use a conservative profile with clearly different speeds, for example 4, 6, 8, 6, 4, 6, 8, 6, 4, 6 km/h, and should record the QZ debug log and the treadmill response.

## Verification

The driver unit tests verify the recorded speed frames, the eight-frame profile sequence, supported timeline mapping, rejection of unsupported durations and invalid rows, the actual sixteen-byte TX90 B4 response, checksum rejection, and fragmented/coalesced response reassembly.

## TX90 regression investigation (2026-11-01)

### Evidence reviewed in this investigation

The email conversation and PR history were reviewed. Four original QZ attachments were downloaded in full: `debug-Mo__Okt__26_18_02_08_2026.log`, `debug-Mo__Okt__26_18_51_51_2026.log`, `debug-Sat_Oct_31_13_58_06_2026.log`, and `debug-Sat_Oct_31_22_00_49_2026.log`. The nRF screenshots identify `iConsole+0014` advertising FFF0 and FTMS; advertising FTMS does not establish that every FTMS control operation succeeds.

The original `20261021_iConsole_Prtogram-Test.pklg` and BTSnoop export `20261021_iConsole_Prtogram-Test - (null).log` could not be downloaded through the email connector: their MIME type is marked unsupported. Three QZ logs from October 21 and two from October 29 have the same access limitation. The captured-frame examples above are inherited PR evidence, not a new independent decode of those unavailable files. The candidate APK is likewise not independently revalidated here. Full protocol reconstruction remains blocked on access to these originals.

### Connection and metrics

- The October 26 18:02 session contains thousands of B2 notifications and movement at 0.8 km/h. Speed-profile exchanges alone did not establish program execution. The smaller 18:51 session contains no workout exchange.
- The October 31 13:58 Android session logs `iConsole initialization will be sent with the workout profile`, but the transmitted profile begins at A3 without the A0/A1 handshake. There are no received `<<` notifications in that log, including after the user reports starting the belt manually. The missing metrics are therefore not evidence of a speed-decoding arithmetic error. The later branch revision restores the A0/A0/A1 handshake.
- In the October 31 22:00 session that handshake is acknowledged by two B0 responses and `F0 B1 01 D3 01 15 02 01 10 9E`. Regular checksum-valid B2 telemetry is received before and after the attempted upload. All those B2 frames report zero speed. The link is working; the log does not show belt motion or prove the reason the console Start button is blocked.
- The revised iConsole receive path separates complete checksum-valid response frames from B2 telemetry. ACKs never enter the telemetry accumulator. Fragmented frames and multiple frames in one notification are handled without changing other AppGate variants' framing.

### Start and upload

At 22:01:04.411 a workout is queued while a poll is awaiting its response. Start is requested during that nested event loop. At 22:01:04.655 the old branch consumes and drops Start because the upload has not completed yet. The subsequent update begins the upload at 22:01:04.740. Retaining Start while upload is pending fixes this ordering defect; failed or invalid uploads still block Start, and Stop cancels it.

A3 receives its valid B3 reply. At 22:01:04.920 QZ sends:

```text
F0 A4 01 D3 15 01 01 01 01 01 01 01 01 01 01 87
```

The TX90 responds:

```text
F0 B4 01 D3 15 01 01 01 01 01 01 01 01 01 01 97
```

The reply is **16 bytes**, with correct checksum `97`. The old implementation expects 17 bytes, times out, and aborts at 22:01:06.425. It never sends A6, A7, A8, A9, AA, AF, or A5 in that attempt. The fix uses the observed sixteen-byte response length. This explains the aborted upload, but it does not independently establish the firmware semantics of selecting a program or locking/unlocking the physical keys.

### Remaining verification

Do not interpret these software corrections as hardware confirmation. Before claiming the TX90 is fixed, decode the original HCI files and compare the complete successful iConsole session: handshake, mode/parameters, both speed tables, inclination table, both duration tables, key control, Start, polling, and Stop. Confirm actual belt motion, console buttons, and profile execution on the TX90. Live speed remains unimplemented, and the existing iConsole Stop path does not currently transmit a physical stop command.

### Software validation for this revision

- Compiled the actual driver translation unit and its parser test translation unit with Qt 5.15.13; one pre-existing `PlayerStateWrapper::getCadence()` missing-return warning remains outside this change.
- Linked the actual driver object into the parser test executable: **36 tests passed**.
- Compiled the previous branch driver and the revised driver against the same standalone sixteen-byte B4 check: previous **REJECTED**, revised **ACCEPTED**.
- Replayed every received notification from the four available original QZ logs through the revised frame extractor: 3,273 notifications in the October 26 18:02 session (3,268 telemetry frames, five ACKs), 140 in the October 31 22:00 session (135 telemetry frames, five ACKs), and zero in the other two sessions. All replay buffers ended empty.
- `git diff --check` passed. These focused checks do not simulate the GATT device, execute the Start/Stop race on hardware, or establish physical workout execution. The full application/platform builds remain the responsibility of CI.
