# Heart rate source diagnostics

## Heart rate is missing, wrong, or coming from the wrong source

### Provide first

- Identify the intended heart-rate source: chest/arm belt, watch/companion app, or the fitness machine's built-in HR.
- State where that HR source is connected during the test: directly to QZ, to the treadmill/bike, or to another app/device.
- Report the exact **Heart Rate Belt Name** configured in QZ, or confirm that it is empty.
- Report whether **Disable HRM from the machinery** is enabled.
- If the HR sensor can connect to the fitness machine itself, say whether it is currently connected there.

### If still unclear

- Temporarily test with only one HR path active at a time. For example, if the belt is connected to the treadmill and the treadmill forwards HR to QZ, leave **Heart Rate Belt Name** empty so QZ does not also try to connect directly to the belt.
- If QZ is expected to connect directly to the sensor, confirm that the sensor is advertising and is not already occupied by another device or app.
- Provide screenshots of the relevant QZ Heart Rate Options.
- Enable QZ Debug Log, reproduce a short session while wearing/using the intended HR source, then provide the generated log and say what HR value/source you expected QZ to use.

### Why it matters

- A heart-rate sensor may reach QZ through more than one path. Knowing whether QZ should read it directly or receive it through the fitness machine prevents troubleshooting the wrong connection.
- A configured **Heart Rate Belt Name** tells QZ to look for that external sensor; this can be inappropriate when the sensor is already connected to the machine and the machine is forwarding HR.
- **Disable HRM from the machinery** determines whether HR reported by the fitness machine is allowed to be used. This is especially important when an external HR source is expected to override built-in or machine-forwarded HR.
- Testing one HR path at a time distinguishes sensor-discovery problems from source-selection conflicts before a full debug log is needed.
