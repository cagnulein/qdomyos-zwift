# Third-party app integration

## QZ works, but Zwift, MyWhoosh, ROUVY, Kinomap, or another app does not behave as expected

### Provide first

- Confirm whether the expected live metrics already work correctly **inside QZ** before testing the third-party app. Specify which metrics work and which do not: speed, cadence, power, resistance, incline, heart rate, or gears.
- Name the third-party app and the devices on which QZ and that app are running.
- Describe exactly how the third-party app is paired to QZ, including the selected Power/Speed/Cadence/Controllable or equivalent sources.
- State whether the original fitness machine is also paired directly to the third-party app.
- For control problems, report whether manual changes made from QZ itself reach the machine correctly.

### If still unclear

- Provide screenshots of the third-party app's pairing/device screen and the relevant QZ settings.
- Reproduce the problem with only the required apps and Bluetooth connections active, then report whether the behavior changes.
- Enable QZ Debug Log, reproduce one short example of the failure, stop the session, and provide the generated log together with the exact action and expected result.

### Why it matters

- If a metric is already wrong or missing in QZ, troubleshooting the third-party pairing first adds another layer and hides the original problem.
- The device topology determines whether QZ is actually in the data/control path.
- A machine connected both directly and through QZ can create competing control paths or make it unclear which device is producing the observed behavior.
- Testing manual control in QZ separates machine-control problems from commands originating in the third-party application.

## Resistance or incline behaves unexpectedly only when a third-party app controls the machine

### Provide first

- Confirm that resistance/incline commands issued manually in QZ work as expected.
- Identify which device is selected as the third-party app's **Controllable/Resistance** device.
- Identify separately which device is selected for power, cadence, speed, and heart rate.
- Give one concrete example with expected and observed values, rather than only describing the behavior as too high, too low, or delayed.

### If still unclear

- Temporarily remove unnecessary direct machine pairings from the third-party app and retest with a single control path.
- Provide a QZ debug log covering a short reproduction, including the approximate time and the command that produced the unexpected result.

### Why it matters

- Data sources and control sources are independent in many training apps. Correct metrics do not prove that commands are being sent through QZ.
- A single numerical example makes scaling, rounding, lag, and conflicting-control problems much easier to distinguish in a log.
