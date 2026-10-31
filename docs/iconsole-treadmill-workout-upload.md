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

The last byte is the sum of all preceding bytes modulo 256. The observed profile changes approximately once per minute. QZ samples the input timeline at the midpoint of each of ten output minutes, then maps every output minute to two 30-second slots. This approximates the input speed and inclination changes, while normalizing the programmed duration to ten minutes.

The capture also records `F0 A3`, `F0 A4`, `F0 A8`, `F0 A9`, `F0 AA`, and `F0 AF` around the speed frames. The candidate Android app's handlers associate these with workout mode, parameters, inclination, duration, and key-lock. The captured parameter frame uses `0x15`; the Android builders encode each data byte with `+1` for `A8/A9/AA`. QZ sends those profile frames in the observed order and checks the corresponding `B3/B4/B6/B7/B8/B9/BA/BF` response frame, including its length and checksum. The APK analyzed is a mirror candidate and has not been proven to match the captured app build. Their complete firmware semantics have not been confirmed on a physical TX 90. `F0 A5` remains a separate workout-control command and is sent only for the explicit Start Workout action, after the profile upload has been acknowledged.

## QZ behavior

When a treadmill workout is associated with a treadmill device, QZ calls the treadmill program hook with the parsed `QList<trainrow>`.

For an iConsole treadmill:

1. QZ requires a non-empty workout with positive-duration, non-ramp rows and finite speeds.
2. QZ samples the timeline at ten evenly spaced minute midpoints. Each sampled value contributes two 30-second speed and inclination slots; missing inclination is encoded as zero.
3. Speed uses the captured representation: `deci-km/h = round(speed * 10)`, then high and low bytes are each incremented by one. `A8/A9/AA` data bytes are incremented by one as in the candidate Android builders; `A4` uses the captured `0x15` parameter value.
4. Before the profile, QZ sends the captured `A0 00 00`, `A0 01 D3`, and `A1 01 D3` initialization frames and waits for valid `B0/B1` responses. If initialization fails, QZ retries the sequence after one second, with at most three attempts total; retries stop if the BLE connection is no longer discovered.
5. QZ creates the ordered profile frames `F0 A3`, `F0 A4`, `F0 A6`, `F0 A7`, `F0 A8`, `F0 A9`, `F0 AA`, and `F0 AF`.
6. After initialization, each frame is written in chunks of at most 20 bytes, with no polling between fragments. QZ waits for the matching complete response frame and checksum after each command; a missing or invalid response stops the active batch. If a newer workout request arrives during transfer, QZ stops the active batch and proceeds with the newest valid queued profile; after an ACK failure it reinitializes before retrying that profile. A rejected latest request keeps Start blocked.
7. After the transaction, the captured iConsole poll `F0 A2 01 D3 66` resumes. iConsole telemetry does not trigger Start. The separate `F0 A5` Start command is sent only when the user requests Start Workout, and only after successful profile acknowledgements; QZ waits for `B5` before starting its workout clock.

Other AppGate treadmill variants keep their existing behavior and do not receive these frames.

## Current limitations

- The device profile is fixed at ten minutes. Imported workout duration is not preserved: speed and inclination are sampled across the source timeline and stretched or compressed to ten minutes.
- Ramp rows, zero/negative-duration rows, and non-finite speeds are rejected.
- The speed, inclination, and duration field mapping follows the candidate Android app and capture, but has not yet been confirmed by a physical TX 90 test.
- The live speed function remains unimplemented because the available evidence does not prove a live command.
- The implementation has not yet been validated on physical TX 90 hardware. Matthias reported that the treadmill moved only after pressing Start on the console; upload acknowledgements do not prove belt movement.

A first hardware test should use a conservative profile with clearly different speeds, for example 4, 6, 8, 6, 4, 6, 8, 6, 4, 6 km/h, and should record the QZ debug log and the treadmill response.

## Verification

The driver unit tests verify the captured speed frames, the complete eight-frame profile sequence, speed-slot mapping for one- and two-row workouts, and rejection of invalid rows.
