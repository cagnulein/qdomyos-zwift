# Connection and device discovery

## QZ does not find or connect to the fitness machine

### Provide first

- The exact machine make and model.
- For a **BLE** machine, a screenshot from **nRF Connect** showing its advertised Bluetooth name. For older **Bluetooth Classic** equipment, state the module/model and whether the device appears in Android's Bluetooth device list instead: nRF Connect's BLE scan does not establish Classic discoverability.
- Confirm whether the machine is already powered on or awake and actively advertising over Bluetooth **before QZ is opened**. For intermittent cases, report whether the expected Bluetooth name is visible in nRF Connect immediately before launching QZ.
- The phone/tablet/computer running QZ and its operating system.
- Whether the manufacturer's app or another fitness app is currently open or connected to the machine.
- What QZ shows while searching: the expected device name, another device, or continuous scanning without a stable connection.

### If still unclear

- A screenshot of the relevant QZ device/settings page.
- Close the manufacturer's app and any other app that may be connected to the machine, restart QZ, and report whether discovery changes.
- Reproduce once with a controlled startup order: completely close QZ, power on or wake the machine first, wait until its Bluetooth name is visible in a scan, then open QZ. Report whether the machine's Bluetooth name appears in QZ.
- Enable QZ Debug Log, reproduce the connection attempt, stop the session, and provide the generated log.

### Why it matters

- The commercial model name alone is often insufficient to identify the Bluetooth protocol; the advertised device name helps map the hardware to QZ's supported device handlers.
- QZ cannot establish a Bluetooth connection to a machine that is not advertising at discovery time. Confirming advertising immediately before launch separates a machine wake/advertising problem from a QZ protocol, permission, or connection problem.
- Many fitness machines allow only one active Bluetooth client. Knowing which apps are open distinguishes QZ discovery problems from a connection already occupied by another app.
- Platform and OS information helps separate device/protocol problems from platform-specific Bluetooth behavior.
- A debug log is most useful after the basic topology and advertised device identity are known, so it should not be the first request in a simple discovery case.


## The machine is not visible in QZ on iPhone or iPad

### Provide first

- The exact machine make and model.
- A screenshot from **nRF Connect on the same iPhone or iPad** showing whether the machine appears in the scan and, if it does, its advertised name.
- Confirm whether the machine is visible to its original manufacturer application or only through the operating system Bluetooth settings.
- State whether an Android device is available for a comparison scan.

### If still unclear

- Repeat the scan with the machine awake and disconnected from other applications.
- If the machine is absent from the iOS nRF Connect scan, compare with nRF Connect on Android before collecting a QZ debug log.
- If Android sees the machine, provide its advertised name and the Android version/device used for the scan.

### Why it matters

- QZ relies on Bluetooth interfaces exposed to applications. A machine that never appears in an iOS BLE scanner may use a legacy Bluetooth path that third-party iOS applications cannot access.
- Comparing the same machine on iOS and Android separates a QZ discovery problem from a platform/protocol limitation before spending time on QZ settings or logs.
- The advertised name from Android can still help identify the protocol and the appropriate QZ device handler.

## Machine connects but QZ metrics remain zero

### Provide first

- Exact machine model, Bluetooth name, and firmware revision if available.
- QZ version and screenshots of relevant machine-specific and generic FTMS settings.
- Identify which QZ metrics update during a short workout.

### If still unclear

- Capture a short QZ debug log after restarting QZ.
- Identify which device handler QZ selected for the machine.
- Compare only one setting change at a time, restarting between tests.

### Why it matters

- A successful Bluetooth connection does not guarantee that the correct protocol handler was selected.
- Conflicting model-specific and generic settings can route a device to an unsuitable parser.

## Bike reports metrics but remote resistance support is uncertain

### Provide first

- Exact bike model, QZ version, and which metrics update in QZ.
- Whether the bike changes resistance using mechanical or electronic controls, and whether its manufacturer app can control resistance remotely.
- Whether manual resistance requests from QZ work before a third-party app is opened.

### If still unclear

- With other bike apps closed, provide an nRF Connect screenshot showing the bike's Bluetooth services and characteristics, including any FTMS control point.
- Provide a QZ debug log covering one manual resistance request and the bike's response.

### Why it matters

- Working telemetry does not prove remote resistance control is available.
- Electronic buttons and FTMS service discovery do not by themselves establish which remote commands are accepted.

## Trainer detected but workout metrics remain unavailable

### Provide first

- Trainer model, Bluetooth device name, QZ version, operating system, and whether workout tiles are absent or show zero.
- Screenshot of the FTMS Bike selection in Bike Options.
- Confirm whether live power and cadence appear in QZ before opening a receiving training app.

### If still unclear

- Select the trainer explicitly under FTMS Bike, restart QZ, and test QZ alone.
- Compare the behavior before and after connecting any external controllers.
- Provide a debug log beginning before device discovery if the dashboard still has no live values.

### Why it matters

- Bluetooth discovery does not establish that QZ selected a telemetry source.
- Testing the trainer without other apps distinguishes incoming metric failures from downstream virtual-device pairing.

## QZ discovers a different nearby fitness machine instead of the intended one

### Provide first

- Exact model and advertised Bluetooth name of the machine you intend to use, plus the other powered-on fitness devices nearby.
- State which machine QZ actually names or attempts to connect to, and whether the failure occurs during startup, discovery, or after a connection.
- Screenshot **Settings > Advanced Settings > Manual Device**, including the selected value, and report the QZ version.

### If still unclear

- For one short test, power off the unrelated fitness machine and select the intended device explicitly under **Manual Device**; restart QZ and confirm which machine supplies changing metrics.
- If discovery remains wrong, provide a QZ debug log from startup showing the device names found and the connection attempts.
- Re-enable other machines only after confirming that QZ reliably connects to the intended one.

### Why it matters

- A crash, empty dashboard, or endless search can originate from QZ selecting a different supported machine, not from the intended machine's protocol.
- The **Manual Device** filter isolates the intended device from other Bluetooth advertisements. In a confirmed case, selecting the intended bike instead of an adjacent elliptical restored operation.
- This is distinct from selecting an FTMS Bike profile or pairing a virtual trainer in a third-party app.

## Life Fitness treadmill is visible over Bluetooth but does not stream workout metrics

### Provide first

- Exact Life Fitness treadmill **and console** model, QZ platform and installed build, and the Bluetooth name advertised by the console.
- State whether the console has completed its workout/profile login, or is still at a Bluetooth/audio/heart-rate pairing screen. If possible, report whether the workout screen shows a logged-in state (for example, **Logout**).
- Report whether QZ displays any live treadmill speed during an active belt session and whether it stops receiving data after a short time.
- Screenshot **Settings > Treadmill Options > Life Fitness Options** showing the guest-login selection (if supported by that build). Do **not** share passwords, authentication tokens, or account details.

### If still unclear

- Verify the exact QZ build first, especially if support provided a TestFlight build; send a new log captured **after** installing that build.
- Capture a short debug log covering console discovery, the profile/login handshake, and the beginning of a workout. Report whether the console itself entered the logged-in workout state.
- Compare guest and account login only when the specific console and QZ build support those modes; change one mode at a time and restart QZ.

### Why it matters

- On the supported Life Fitness login-based treadmill path, Bluetooth discovery and audio/HR pairing do not establish an authenticated workout session. Telemetry may be unavailable until the console's profile exchange completes.
- Distinguishing an incomplete login handshake from a failed FTMS data parser prevents troubleshooting the wrong layer. A production-build log also cannot validate a handshake change present only in a test build.
- This is model-specific: see the [Life Fitness protocol scope](../../src/devices/lifefitnesstreadmill/LIFE_FITNESS_95T_PROTOCOL.md); do not assume every Life Fitness console requires the same exchange.
