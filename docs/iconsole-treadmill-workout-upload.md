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

The last byte is the sum of all preceding bytes modulo 256. The observed profile changes approximately once per minute, so the first QZ implementation accepts exactly ten non-ramp rows of one minute each.

The capture also contains an `F0 A8` table. Its exact value layout and relationship to the ten workout rows are not sufficiently established yet, so this patch does not upload inclination values.

## QZ behavior

When a treadmill workout is associated with a treadmill device, QZ calls the treadmill program hook with the parsed `QList<trainrow>`.

For an iConsole treadmill:

1. QZ validates that the workout contains ten one-minute non-ramp rows.
2. Each speed is encoded using the captured representation:
   `deci-km/h = round(speed * 10)` followed by `deci-km/h / 100 + 1` and `deci-km/h % 100 + 1`.
3. QZ creates both `F0 A6` and `F0 A7` frames.
4. If BLE initialization is not complete, the frames remain queued.
5. After initialization, each frame is written as 20 bytes plus 5 bytes, matching the capture.
6. The normal telemetry polling remains enabled.

Other AppGate treadmill variants keep their existing behavior and do not receive these frames.

## Current limitations

- Only ten one-minute non-ramp rows are accepted.
- Only speed is uploaded.
- Inclination upload is intentionally deferred until the `F0 A8` layout is confirmed with another capture or a physical TX 90 test.
- The live speed function remains unimplemented because the available evidence does not prove a live command.
- The implementation has not yet been validated on physical TX 90 hardware.

A first hardware test should use a conservative profile with clearly different speeds, for example 4, 6, 8, 6, 4, 6, 8, 6, 4, 6 km/h, and should record the QZ debug log and the treadmill response.

## Verification

The driver unit tests verify the two speed frames against the captured byte sequences and verify rejection of a workout with the wrong number of rows.
