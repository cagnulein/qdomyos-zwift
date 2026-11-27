# Crashes and unexpected exits

## QZ freezes, closes, or crashes during use

### Provide first

- Confirm explicitly **which app crashes**: QZ or the training app used with it.
- The device and operating system running QZ.
- The fitness machine/trainer model and any additional sensors connected to QZ, such as a heart-rate monitor.
- The exact trigger: for example, immediately after launch, when the machine connects, when pedaling starts, when a workout starts, or when a control command is received.
- Whether the crash also happens when QZ is used without the third-party training app.

### If still unclear

- Provide the relevant QZ settings or profile screenshots, especially virtual-device and integration settings.
- Disable unrelated integrations/connections and reproduce the smallest configuration that still crashes.
- Enable QZ Debug Log and reproduce the crash if possible. If the app exits before the normal log-email flow can complete, provide any available platform crash report together with the last actions performed.

### Why it matters

- In a multi-app setup, identifying the application that actually exits is the first branching point; otherwise diagnostics can target the wrong software.
- The exact trigger narrows the problem to discovery, device parsing, metric updates, virtual-device advertising, or control handling.
- Reproducing without the third-party app separates a QZ/device problem from an interaction between QZ and another application.
- Reducing the setup before collecting deeper logs makes the resulting evidence substantially easier to interpret.

## QZ closes during a workout, but the debug log shows only a normal shutdown

### Provide first

- Android device/model and OS version, QZ build, and whether the app closed by itself or after a Back/Close action.
- Workout type (including imported workout format), approximate elapsed time at the exit, and whether it repeats near the same duration.
- The exact heart-rate and power sources selected in QZ, especially when one accessory is configured for multiple roles.

### If still unclear

- Send the QZ debug log covering the actual reported failure, not only shorter sessions from the same day; identify the approximate start and exit times.
- If the QZ log ends with normal shutdown events but no exception, collect an Android bug report immediately after the next unexpected exit.
- Retest with one source-selection change at a time and record whether the failure still occurs.

### Why it matters

- A QZ log ending in a normal shutdown sequence does not establish whether the user closed the app or Android terminated it.
- Short logs that stop before the reported failure cannot identify a later crash.
- A platform bug report may capture a system-side process exit that QZ's own logger cannot record.

## iOS floating metrics (Picture in Picture) freezes or QZ exits when switching apps

### Provide first

- iPhone/iPad model, iOS/iPadOS version, exact QZ build, training app and whether both apps run on the same device. Report whether QZ's background option is enabled.
- Distinguish stale floating-window values from stale values in QZ itself, loss of training-app control/telemetry, and an actual QZ process exit. State which happens first.
- Record the exact steps and approximate time: starting the floating window, switching to the training app, changing a gear, returning to QZ, locking the screen, or other relevant action.

### If still unclear

- Compare two short otherwise identical tests: switch between QZ and the receiving app with the floating window closed, then repeat with it open. Check whether QZ metrics, floating metrics and receiving-app telemetry each continue updating. Do not change the background setting or pairing configuration during this comparison.
- If QZ exits, check iOS/iPadOS **Settings > Privacy & Security > Analytics & Improvements > Analytics Data** for a QZ/qdomyos-zwift crash report or a JetsamEvent around the reported time. Provide the relevant `.ips` report, the installed build number and the QZ debug log if available. Share these privately with support rather than posting unredacted system diagnostics publicly.

### Why it matters

- A visible but stale Picture in Picture window, a disconnected trainer, and a terminated app are distinct failure stages.
- A reviewed case supplied iOS crash reports after repeated app switching and allowed support to locate the failing processing queue. The reports did not prove that Picture in Picture or background Bluetooth was the cause.
- The exact build permits matching crash addresses to the corresponding debug symbols; the comparison with the floating window closed helps isolate whether its presence is required to reproduce the issue.
