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

## Zwift automatic incline stops while local treadmill control still works

### Provide first

- Treadmill model, QZ platform/version, and whether manual incline changes from QZ still reach the treadmill.
- Whether incline is expected through QZ's Zwift account integration or a paired virtual controllable device.
- Whether it never worked or stopped after working, and whether Zwift website sign-in works. Do not share credentials or tokens.
- One comparison of Zwift route grade, QZ incline, and treadmill incline.

### If still unclear

- Capture a short QZ debug log spanning session start and one route-grade change; remove sensitive authentication information before sharing.
- If authentication fails, isolate account access before examining treadmill commands.
- If incoming grade is visible, report Zwift Inclination Gain and Offset and compare two grades.

### Why it matters

- Missing incoming grade updates and rejected treadmill commands are different failure points.
- Successful manual incline control narrows the fault to grade acquisition or translation.
