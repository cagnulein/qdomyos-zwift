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
- If a bike responds to QZ controls but not to external ERG/controllable commands, report whether QZ identifies it as a **bike or elliptical**, and screenshot **Bike Options > FTMS Bike** plus any competing model-specific selection.
- Identify which device is selected as the third-party app's **Controllable/Resistance** device.
- Identify separately which device is selected for power, cadence, speed, and heart rate.
- Give one concrete example with expected and observed values, rather than only describing the behavior as too high, too low, or delayed.

### If still unclear

- Temporarily remove unnecessary direct machine pairings from the third-party app and retest with a single control path.
- If the selected device class appears wrong, verify the advertised name and the actual QZ device handler before changing settings; for a confirmed FTMS bike, compare one restart with the correct FTMS Bike selection and record whether external ERG control changes.
- Provide a QZ debug log covering a short reproduction, including the approximate time and the command that produced the unexpected result.

### Why it matters

- Data sources and control sources are independent in many training apps. Correct metrics do not prove that commands are being sent through QZ.
- A bike misclassified as an elliptical can still respond to some local controls while external ERG control fails. A confirmed FTMS bike case recovered ERG response after selecting the bike profile; see the [device-compatibility FAQ](../faq/device-compatibility.md).
- A single numerical example makes scaling, rounding, lag, and conflicting-control problems much easier to distinguish in a log.

## A debug log does not show the reported control or pairing failure

### Provide first

- Confirm the exact machine model and the device running QZ.
- Confirm which app is expected to send the command and which QZ virtual device is selected in that app.
- State whether QZ is already showing live metrics from the machine before the test begins.
- Describe one short action that will reproduce the problem, for example starting an ERG interval, changing resistance, or attempting to pair a controllable device.

### Controlled reproduction

1. Fully close QZ and the third-party fitness app.
2. Wake the fitness machine or console first.
3. Make sure manufacturer apps and unrelated Bluetooth fitness apps are closed so they cannot take the connection.
4. Start QZ and wait until the machine is connected and live metrics are visible.
5. Start the third-party app and pair only the QZ virtual devices required for the test.
6. Perform one clearly identifiable control action and keep the test short.
7. Stop QZ and provide the complete debug log, together with the action performed and the approximate time it happened.

If a QZ option is suspected of changing the behavior, run separate short tests with the option enabled and disabled rather than changing several settings during one recording.

### Why it matters

- A log that ends before the machine connects or before the third-party app sends a command cannot show the failure being investigated.
- Starting from a known state makes the connection order visible and reduces ambiguity from stale Bluetooth sessions.
- One controlled action creates a clear point in the log where the expected command can be compared with what QZ actually received and sent.
- Changing one setting at a time makes A/B logs directly comparable and helps distinguish configuration effects from intermittent connection behavior.

## Resistance levels differ between the bike and QZ

### Provide first

- Bike model, QZ version, physical console resistance range, and the minimum/maximum **Resistance** reported by QZ.
- One simultaneous example of the **Gears** tile, **Resistance** tile, bike console level, and how the pedals actually feel. Do not assume virtual gears and physical resistance are the same scale.
- Whether changes originate from the bike, QZ, or the external training app, and whether the bike starts reporting metrics immediately or only after the first Gear+ / resistance command.
- Screenshot relevant FTMS Bike and model-specific resistance settings before changing any options.

### If still unclear

- Close training apps and turn off Auto Resistance. Compare a few **safe, comfortable** physical resistance levels with the values QZ displays; stop rather than forcing high resistance.
- If telemetry stays at zero until a control action, record one QZ-only startup test: pedal briefly without touching the gear buttons, then press Gear+ once. Note when metrics begin and collect a log covering both stages.
- If physical resistance stops increasing while Gears continues upward, capture a separate short log of two or three safe resistance requests. Where FTMS is used, compare the machine's advertised resistance range, the control-point responses, the reported Resistance tile, and actual physical response.
- Test automatic control from Zwift/MyWhoosh/ROUVY only after the QZ-only test, recording the app's Controllable pairing role separately.

### Why it matters

- A bike may advertise fewer FTMS resistance levels than its console provides; the Gears number is not proof of the Bluetooth-controlled physical level.
- A successful FTMS command response confirms protocol acceptance, not that the motor moved through the console's entire range.
- Metrics beginning only after the first control command can distinguish startup/control initialization timing from an incorrect resistance scale.
- Separating local QZ control from third-party commands distinguishes telemetry mapping, initialization, and downstream trainer-control problems.

## Virtual gear numbers update but trainer resistance does not follow

### Provide first

- Trainer model, QZ version, and devices running QZ and Zwift.
- Whether QZ receives power/cadence before Zwift starts, and which device Zwift selects as Controllable/Resistance.
- Whether Click/Play is paired to Zwift or QZ; report Get Gears from Zwift and Align Gear settings.
- One shift with the gear numbers shown in both apps and the physical resistance response.

### If still unclear

- For Wahoo trainers, capture the Without Wheel Diameter Protocol setting, virtual gearing configuration, and actual drivetrain tooth counts.
- Provide a short QZ debug log with two deliberate shifts on flat terrain. Stop if resistance becomes excessive.

### Why it matters

- Gear display synchronization alone does not prove a control command reached the trainer.
- Controllable pairing and drivetrain scaling can fail independently.

## Gradient-controlled accessory does not follow route grade

### Provide first

- Accessory model, receiving training app, QZ version, and device topology.
- Whether manual incline control from QZ works, and one comparison of route grade, QZ incline tile, and physical movement.
- Which virtual device is selected as Controllable, and any accessory-specific gain setting.
- Report the training app's own gradient intensity or realism setting (including whether it is 0%), separate from QZ's gain.

### If still unclear

- Capture a short QZ log across a clear grade transition, with approximate times and values.
- If the app's gradient setting was zero, repeat the same short route section with a nonzero setting before collecting a second log.
- Compare incoming simulation requests with outgoing accessory commands; vary one setting at a time.

### Why it matters

- Absent grade requests and incorrectly scaled commands look similar at the accessory but require different investigation.
- An app-level gradient setting of zero can suppress grade changes before QZ receives them. In a confirmed case, increasing the MyWhoosh gradient setting restored QZ incline updates and Elite Rizer movement.

## The training app sees QZ, but a pairing role is unavailable or the connection fails

### Provide first

- Name the receiving app and its version; identify the devices and operating systems running QZ and the receiving app. **Explicitly state whether both apps run on the same device.**
- Confirm which live metrics work inside QZ before opening the receiving app.
- Provide a screenshot of the receiving app's pairing screen, including which role (Power, Speed, Cadence, or Controllable) is greyed out, fails to connect, or is paired to a different device.
- Identify the exact advertised virtual trainer name, and whether the receiving app connects to QZ over Bluetooth or Wi-Fi.

### If still unclear

- On a two-device setup, check that QZ remains open and the receiving device is scanning for QZ's virtual trainer, not the physical machine. Record whether discovery succeeds but pairing fails.
- For a supported network connection, confirm that the devices share the same local network and report the QZ virtual-device/DIRCON settings; compare Bluetooth and network discovery separately.
- Remove stale pairing entries in the receiving app and retry one role at a time, without connecting the physical trainer directly to that app. Record which role becomes selectable.
- If QZ and the receiving app are on the **same phone or tablet**, establish whether that specific app supports QZ's same-device connection method before requesting Bluetooth logs or asking the user to change virtual profiles.
- Request a short QZ debug log only after the topology, transport, and failing pairing role are known.

### Why it matters

- Seeing a virtual trainer in a device list does not prove the receiving app can connect to it or expose it for the required sensor/control role.
- Same-device integration is app-specific; instructions intended for a second phone, PC, or tablet can be inapplicable when both apps run on one Android device.
- Separating discovery, pairing, and controllable-role selection avoids treating a transport limitation as a physical-bike or virtual-gearing failure.

## Virtual gears affect resistance only after the training app connects

### Provide first

- Trainer model, QZ build, receiving app, and whether the gear tile changes before and after pairing the receiving app.
- Exact startup sequence: controller unlock/pairing, QZ launch, receiving-app pairing, ride start, and the first Gear + / Gear - press.
- Whether the physical resistance changes on a flat route when shifting **only after** the receiving app has fully connected.

### If still unclear

- Repeat one short ride without pressing any gear buttons until the receiving app has connected to QZ and started the activity. Compare with a separate ride where shifting occurs before that connection.
- Record whether the receiving app needed a cached trainer entry removed and paired again, and whether the result survives the next launch.
- Capture a QZ debug log of each sequence with the first shift timestamp, and note whether the virtual gear tile changes while physical resistance stays constant.

### Why it matters

- A confirmed comparison restored physical gear response when shifts were delayed until after Kinomap had connected. This distinguishes controller input failure from a timing/state problem in the virtual-trainer control path.
- Cached trainer pairings and early gear changes can produce similar symptoms but should be tested independently.


## Zwift Click buttons appear connected but do not change gears or the local ERG target

### Provide first

- Identify the controller version (**Click v1 or Click v2**), trainer model, QZ device/OS, and the **installed QZ build** (release, beta, or test build).
- State **which app owns the Click connection**: QZ directly, or Zwift with QZ receiving gear changes. Include the values of **Accessories > Zwift Click** and **Zwift Options > Get Gears from Zwift**.
- Before opening QZ, wake the Click and report whether its LEDs blink; after QZ starts, report whether the LEDs become steady or continue blinking. Describe the LED behavior rather than assuming it proves button input.
- Press a button and state **which QZ value changes**, if any: the **Gears** tile, the **Target Power (T.Power)** tile, or neither. Distinguish an unchanged tile from a changing tile with no physical trainer response.
- For standalone ERG control, confirm that the target was set above zero in QZ and that no external training app was controlling the target.

### If still unclear

- For Click v2, identify whether the initial pairing/unlock was performed in the **full Zwift app** or only in Zwift Companion, and whether the problem occurs immediately or after about a minute. Refer to the [Zwift/Click FAQ](../faq/zwift.md) for the known handshake procedure; do not repeat the full procedure in intake requests.
- Repeat a short controlled test with only the intended controller owner connected. Note the LED state and QZ tile values before and after two deliberate presses.
- If button input still does not reach QZ, provide a debug log from startup through those presses, with approximate timestamps. If QZ changes the gear number but trainer resistance does not follow, continue with the existing **Virtual gear numbers update but trainer resistance does not follow** guide above.

### Why it matters

- The LED transition helps narrow down **discovery/pairing**, but only a changing QZ tile establishes that button input reached the intended control path.
- Click ownership differs between direct QZ control and **Get Gears from Zwift**; mixing them can obscure whether QZ should receive Bluetooth button events or virtual gear updates.
- In a confirmed Android test, standalone Click v2 ERG target changes worked in the beta but not in the previously installed release. Checking the exact running build avoids misdiagnosing a missing feature as a controller connection failure.

## A training app disconnects partway through a previously working session

### Provide first

- Fitness machine model, QZ version and platform, receiving app and version, and which device runs each app.
- At the moment of disconnection, do QZ's live speed/power/cadence tiles also freeze or fall to zero, or do they continue updating while only the receiving app loses the connection?
- Does the connection recover on its own, after re-pairing in the receiving app, or only after fully restarting QZ?
- Was QZ in the foreground with the screen awake, or in the background or with the screen locked?

### If still unclear

- Confirm that no manufacturer app or second phone is also connected directly to the physical machine. Record whether the receiving app uses QZ's virtual Bluetooth device or a network connection.
- Capture one QZ debug log that includes a disconnection, and provide the approximate elapsed time of the failure and whether the QZ metrics stopped. Keep the test configuration unchanged.
- If QZ continues to receive live metrics, capture the receiving app's pairing/disconnection screen and retry its virtual-device connection separately before changing machine-specific settings.

### Why it matters

- If QZ metrics stop, investigate the physical machine-to-QZ connection; if QZ metrics continue, investigate the QZ-to-training-app link instead.
- Foreground state, competing connections, recovery behavior, and a timestamped log distinguish intermittent Bluetooth loss from downstream virtual-device or app lifecycle problems.
- In a reported multi-minute session, resistance and incline worked until intermittent disconnects; checking QZ's own tiles established that the physical machine connection, not just the receiving app's pairing, had dropped.
