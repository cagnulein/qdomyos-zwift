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
