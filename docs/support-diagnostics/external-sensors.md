# External speed and cadence sensors

## QZ discovers accessory sensors, but no workout tiles appear or the sensor data is not used

### Provide first

- Describe the physical setup: bike/trainer type and whether each accessory sensor is mounted for **speed** or **cadence**.
- Give the advertised Bluetooth name of each sensor and state which one is expected to start the QZ bike session.
- Provide a screenshot of **Settings > Accessories > Cadence Sensor** (or the relevant accessory sensor setting), including the selected device.
- State whether **Cadence Sensor as a Bike** is enabled when the cadence sensor is intended to act as the primary bike input.
- If **Advanced Settings > Manual Device** is configured, state which device is selected there as well.
- Confirm whether the sensors work when connected directly to another app, and whether QZ reports them as discovered even though the workout UI does not start.

### If still unclear

- Temporarily configure only the sensor that should act as the primary bike input, restart QZ, and report whether the workout tiles appear.
- Provide screenshots of the relevant sensor and manual-device settings so the selected Bluetooth names can be compared.
- Enable QZ Debug Log, restart QZ, reproduce the discovery/startup problem, stop the session, and provide the generated log.

### Why it matters

- Discovering a Bluetooth sensor is not the same as selecting it as the device that QZ should use to instantiate the bike session.
- Multi-purpose speed/cadence sensors can advertise similarly while being mounted in different modes, so the physical role of each sensor must be known before interpreting the settings.
- A stale or different **Manual Device** selection can conflict with the accessory selection and is easy to miss when only the normal sensor page is checked.
- Verifying that the sensor works directly in another app separates a basic sensor/Bluetooth problem from a QZ device-selection or startup problem.

## Cadence intermittently drops to zero while power keeps updating

### Provide first

- Name the bike/trainer and external power meter (including model), plus the QZ version and platform.
- State which device supplies **power** and which supplies **cadence** in QZ, and whether **Use cadence from the power sensor** is enabled.
- Confirm whether cadence drops to zero **inside QZ** as well as in the training app, while power continues updating normally. Give the approximate duration and frequency of the drops.

### If still unclear

- Record a short QZ debug log containing two or three drops during steady pedaling, with approximate timestamps. A brief screen recording of the QZ power and cadence tiles can help align the events.
- If the trainer also reports cadence, run a separate short comparison using the trainer's cadence source instead of the power meter; change only the cadence source between tests.

### Why it matters

- A zero already visible in QZ points to the sensor-to-QZ data path, not the receiving training application's pairing.
- A power meter can continue reporting watts while its crank-revolution/cadence information is absent or not advancing. Recording both metrics distinguishes a cadence-specific dropout from a complete Bluetooth disconnect.
- Comparing independent cadence sources identifies whether the issue follows the external power meter or QZ's broader device connection.

## External power-match correction appears ineffective during ERG workouts

### Provide first

- Trainer and external power-meter models, QZ build, receiving app, and whether the external meter is connected **to QZ**, not solely to the receiving app.
- Screenshot of QZ **Power Sensor Name** and live power values displayed in QZ before starting an ERG interval.
- One ERG target and the observed QZ power values during the interval.

### If still unclear

- Confirm the exact test build includes the intended power-match behavior, restart QZ with the external power sensor selected, and repeat a short controlled ERG interval.
- Send the QZ debug log and identify whether any separate trainer Bluetooth disconnects occurred.
- If power still diverges, compare the external meter reading, trainer-reported power, and ERG target at the same timestamps.

### Why it matters

- An external meter paired only to ROUVY or another receiving app does not establish that QZ can use its data for power matching.
- A reviewed test log showed **Power Sensor Name = Disabled**, meaning the intended correction path was not active; Bluetooth disconnects were a separate issue.
