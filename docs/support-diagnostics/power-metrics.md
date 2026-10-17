# Power and training metrics

## Power differs from the machine or training app

### Provide first

- Machine model and QZ version.
- One simultaneous power reading from the machine and QZ.
- The current Watt Gain and Watt Offset values.
- The configured FTP if the issue concerns power zones.
- Which power source is selected in the training app.

### If still unclear

- Record a short QZ debug log while pedaling steadily.
- Note the approximate time and expected versus observed watts.

### Why it matters

- QZ can scale received power using gain and offset.
- FTP affects power-zone classification separately from watts.

## Displayed resistance varies with cadence while the physical resistance setting is unchanged

### Provide first

- Bike model, QZ version, and whether the console and QZ show the same resistance value.
- Hold the bike's physical resistance setting constant and report cadence, watts, and displayed resistance at three comfortable pedaling rates.
- State whether an external resistance sensor or power meter is configured.

### If still unclear

- Repeat a short controlled session with the physical resistance unchanged and approximately 20 seconds at each cadence.
- Provide a QZ debug log from that session, not just the workout summary or a training-plan XML file.

### Why it matters

- Some QZ bike drivers estimate resistance from power and cadence when direct resistance data is unavailable.
- A fixed-knob cadence comparison distinguishes an estimated metric from an actual physical resistance change; raw logs are needed before attributing any difference to firmware or calibration.
