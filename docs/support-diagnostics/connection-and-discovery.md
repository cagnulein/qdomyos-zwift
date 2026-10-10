# Connection and device discovery

## QZ does not find or connect to the fitness machine

### Provide first

- The exact machine make and model.
- A screenshot from **nRF Connect** showing the machine in the Bluetooth scan, including the advertised Bluetooth device name.
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
