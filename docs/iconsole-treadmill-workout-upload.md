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

The last byte is the sum of all preceding bytes modulo 256. The observed profile changes approximately once per minute. This implementation accepts exactly ten one-minute, non-ramp rows and maps each row to two 30-second slots. The profile is split into two ten-slot speed frames.

The capture also records `F0 A3`, `F0 A4`, `F0 A8`, `F0 A9`, `F0 AA`, and `F0 AF` around the speed frames. The candidate Android app's handlers associate these with workout mode, parameters, inclination, duration, and key-lock. The APK analyzed is a mirror candidate and has not been proven to match the captured app build. QZ now sends those profile frames in the observed order. Their complete firmware semantics have not been confirmed on a physical TX 90. `F0 A5` remains a separate workout-control command and is not part of the profile upload.

## QZ behavior

When a treadmill workout is associated with a treadmill device, QZ calls the treadmill program hook with the parsed `QList<trainrow>`.

For an iConsole treadmill:

1. QZ validates that the workout has exactly ten one-minute, non-ramp rows with finite speeds.
2. Each row contributes two 30-second speed and inclination slots. Missing inclination is encoded as zero.
3. Each speed is encoded using the captured representation:
   `deci-km/h = round(speed * 10)` followed by `deci-km/h / 100 + 1` and `deci-km/h % 100 + 1`.
4. QZ creates the ordered profile frames `F0 A3`, `F0 A4`, `F0 A6`, `F0 A7`, `F0 A8`, `F0 A9`, `F0 AA`, and `F0 AF`.
5. If BLE initialization is not complete, the frames remain queued.
6. After initialization, each frame is written in chunks of at most 20 bytes, with a 100 ms delay between frames. The 25-byte speed frames are therefore split into 20-byte and 5-byte writes.
7. The normal telemetry polling remains enabled.

Other AppGate treadmill variants keep their existing behavior and do not receive these frames.

## Current limitations

- Only ten one-minute, non-ramp rows are accepted; other workout shapes are rejected.
- The speed, inclination, and duration field mapping follows the candidate Android app and capture, but has not yet been confirmed by a physical TX 90 test.
- The live speed function remains unimplemented because the available evidence does not prove a live command.
- The implementation has not yet been validated on physical TX 90 hardware. Matthias reported that the treadmill moved only after pressing Start on the console; upload acknowledgements do not prove belt movement.

A first hardware test should use a conservative profile with clearly different speeds, for example 4, 6, 8, 6, 4, 6, 8, 6, 4, 6 km/h, and should record the QZ debug log and the treadmill response.

## Verification

The driver unit tests verify the captured speed frames, the complete eight-frame profile sequence, speed-slot mapping, and rejection of workouts that do not contain exactly ten valid rows.
