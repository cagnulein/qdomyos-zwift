# Treadmill control diagnostics

## QZ reads treadmill metrics, but some commands do not work

### Provide first

- The full treadmill model number, advertised Bluetooth name, QZ platform/version, and selected Specific Model option.
- Which actions work from the treadmill's physical controls, from QZ, and from the third-party training app? Check start, stop, speed, and incline separately.
- One concrete expected-versus-observed example for a failing command, including any delay.
- Confirm that QZ manual control was tested before troubleshooting commands from a training app.

### If still unclear

- Record a short QZ debug log with one clearly identified control action at a time and note the approximate time.
- For a proprietary treadmill model whose control commands remain unknown, support may request a separate Bluetooth capture from the manufacturer's app. Keep QZ closed during that capture and perform distinct start, stop, speed, and incline actions.
- On iOS, a Bluetooth logging profile and a sysdiagnose may be required for the manufacturer-app capture. Collect this only when requested; diagnostic archives can contain sensitive device data.

### Why it matters

- Receiving speed and incline telemetry does not prove that QZ has the correct command sequence for that exact treadmill revision.
- A physical start or stop button is not equivalent to a remote command from QZ or the manufacturer's app.
- Separating individual actions and comparing QZ logs with native-app captures helps identify missing model-specific commands without confusing the data path with the control path.
