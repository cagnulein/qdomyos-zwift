# Treadmill control diagnostics

## QZ reads treadmill metrics, but some commands do not work

### Provide first

- The full treadmill model number, advertised Bluetooth name, QZ platform/version, and selected Specific Model option. If the model has had multiple hardware revisions, also report its manufacture date or console firmware version when available.
- Which actions work from the treadmill's physical controls, from QZ, and from the third-party training app? Check start, stop, speed, and incline separately.
- One concrete expected-versus-observed example for a failing command, including any delay.
- Confirm that QZ manual control was tested before troubleshooting commands from a training app.

### If still unclear

- Record a short QZ debug log with one clearly identified control action at a time and note the approximate time.
- For an FTMS treadmill, distinguish a **successful Bluetooth write** to the Fitness Machine Control Point (`0x2AD9`) from an actual **Control Point response indication** and a physical speed/incline change. If the write is acknowledged but the treadmill does nothing, preserve the raw command and any indication/response in the log.
- If QZ control fails but another app controls the treadmill, record **which action** that app actually performs (start, speed, or incline), not just whether it connects. When requested by support, an isolated nRF Connect control-point test can help identify missing responses; use a stationary, unoccupied treadmill with its safety stop available. Do not assume a write acknowledgment means the command was accepted.
- For a proprietary treadmill model whose control commands remain unknown, support may request a separate Bluetooth capture from the manufacturer's app. Keep QZ closed during that capture and perform distinct start, stop, speed, and incline actions.
- On iOS, a Bluetooth logging profile and a sysdiagnose may be required for the manufacturer-app capture. Collect this only when requested; diagnostic archives can contain sensitive device data.

### Why it matters

- Receiving speed and incline telemetry does not prove that QZ has the correct command sequence for that exact treadmill revision.
- A physical start or stop button is not equivalent to a remote command from QZ or the manufacturer's app.
- Separating individual actions and comparing QZ logs with native-app captures helps identify missing model-specific commands without confusing the data path with the control path.
- A working manufacturer-app incline command alongside an ignored standard FTMS control-point write points to a possible protocol or initialization difference. Device revision and the actual response indications are needed before changing a model-specific workaround or guessing proprietary command bytes.

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

## Treadmill speed or distance appears in the wrong units in QZ

### Provide first

- Exact treadmill model, QZ version, and the unit displayed on the treadmill console versus QZ.
- One fixed-speed example with the numeric value and unit from the console, QZ speed tile, and, if relevant, the receiving fitness app.
- Screenshots of both the device-specific unit interpretation setting (where present) and the QZ user-interface unit preference.

### If still unclear

- Change only the device-specific incoming-unit setting, fully restart QZ, and repeat the same fixed-speed test; retain the preferred display unit.
- Compare both speed and accumulated distance in a short controlled session before investigating Apple Health or other exports.
- Provide a debug log if the reported device value or conversion remains inconsistent.

### Why it matters

- Incoming device units and preferred display units are separate decisions. A value interpreted as mph when the treadmill sends km/h can appear numerically correct but be labeled incorrectly and can also affect distance exports.
- Comparing the same physical speed before and after a single setting change isolates interpretation from downstream recording.

## Treadmill incline is much steeper than the grade requested by Zwift

### Provide first

- Treadmill model, QZ build, and a low, safe numerical example: Zwift route grade, QZ incline tile, and physical console incline at the same moment.
- Screenshots of **both** Zwift Options > Zwift Inclination Gain/Offset and Treadmill Inclination Overrides > Inclination Override Gain/Offset.
- Whether manual incline changes from QZ agree with the physical treadmill console.

### If still unclear

- Test only a gentle grade at a comfortable speed, with nobody relying on automatic incline for safety. Restore override gain/offset to neutral values before isolating Zwift-specific scaling.
- Change one gain or offset at a time and record the three readings again; do not attempt to calibrate using a steep hill.
- Supply a short debug log covering one small grade transition, including the time and requested versus observed values.

### Why it matters

- The Zwift incline gain and the treadmill override gain are distinct controls. A screenshot showing one set cannot establish how the other is configured.
- Comparing route, QZ, and console values identifies whether amplification occurs in the incoming grade interpretation or in the treadmill command path.

## Treadmill speed is readable, but the selected driver cannot control it

### Provide first

- Exact treadmill model/revision, QZ platform/build, and **advertised Bluetooth name** (not just the marketing model name).
- Confirm whether QZ's speed tile follows changes made with the **physical console**, and whether QZ's own Speed +/- buttons can change the belt speed. Test QZ before opening Zwift, Runna, or another training app.
- Share screenshots of **Treadmill Options > FTMS Treadmill**, any selected model-specific treadmill profile, and an **nRF Connect** service list for the treadmill. In particular, note whether the standard Fitness Machine service **0x1826** is advertised.

### If still unclear

- If the service list confirms FTMS and the current model-specific driver only reads data, support can compare a short test using **FTMS Treadmill** set to the *exact advertised device name*. Save and **fully restart QZ** before comparing. Do not assume every FTMS-advertising treadmill accepts remote FTMS control.
- Conversely, if forcing FTMS makes a model-specific treadmill stop or behave differently, record that selection and compare a support-recommended model profile in a separate restart/test. Change **one profile at a time**.
- For each test, start the belt from the console at a **safe low speed**, note the console and QZ speed, try only one small QZ speed change, and record whether the belt responds or unexpectedly stops. If inconclusive, provide a fresh debug log with the profile used and the approximate action times.

### Why it matters

- The same treadmill can expose standard FTMS telemetry while QZ's automatic name-based detection chooses a manufacturer-specific driver with different control support. Bluetooth discovery and readable speed do not establish that the chosen driver implements speed commands.
- A confirmed test on an FTMS-advertising treadmill restored QZ speed control by selecting its exact name under **FTMS Treadmill** instead of the automatically chosen manufacturer driver. A separate model-specific treadmill case required clearing the forced FTMS selection and choosing a matching profile. These outcomes show why the **advertised services and active QZ profile must be collected together**, not why FTMS should be forced universally.
- Comparing one safe command across explicitly recorded profiles separates **driver selection** from a missing control-point response or an underlying treadmill fault. For protocol-level failures, continue with the [general treadmill control intake](#qz-reads-treadmill-metrics-but-some-commands-do-not-work).

## FTMS treadmill advertises incline control but rejects remote commands

### Provide first

- Exact treadmill and console model, firmware version if available, and the Bluetooth name shown in nRF Connect.
- Confirm whether speed and incline values **read correctly** in QZ or the receiving app, separately from whether either app can **change** the treadmill's incline.
- State whether any manufacturer-supported app can actually change incline remotely, or only read it. Include the exact app name and the action tested.
- If already captured, share the nRF Connect **Fitness Machine Service (0x1826)** characteristics, especially **Fitness Machine Control Point (0x2AD9)**, **Supported Inclination Range (0x2AD5)**, and the complete response to a **Request Control** attempt. Do not infer control support from the presence of these characteristics alone.

### If still unclear

- If the manufacturer app genuinely controls incline, capture its app version and a short, isolated comparison of one small incline adjustment with QZ closed; only then consider a manufacturer-app Bluetooth trace with support guidance.
- If an FTMS Request Control response was captured, report the exact response bytes and decoded status. For example, **0x80 0x00 0x05** indicates a Request Control response with **Control Not Permitted**; distinguish this from a transport/write failure or a missing response.
- Capture a short QZ debug log with one safe, low-incline control attempt only when the command response or active driver remains unclear. Keep the safety key available; do not repeatedly force incline changes after an explicit rejection.

### Why it matters

- Advertising the FTMS control point and an inclination range does **not** guarantee that firmware grants external control. Successful telemetry can coexist with rejected commands.
- An explicit **Control Not Permitted** response narrows the issue to device-side permission or initialization rather than a missing incline range or an unrelated receiving-app pairing problem.
- A demonstrated manufacturer-app control path is stronger evidence for investigating a proprietary initialization sequence than a manufacturer's general claim of app connectivity. Without such evidence, do not assume a hidden unlock exists.

## FTMS command succeeds but only speed or incline responds physically

### Provide first

- Exact treadmill model, **advertised Bluetooth name**, QZ version, and whether other nearby FTMS machines are powered on.
- Confirm that speed, incline, and distance **read correctly in QZ** before testing controls. Report **start, speed, and incline separately**: for each, did QZ issue a command, did the treadmill beep or acknowledge it, and did the belt or deck actually move?
- Screenshot the selected **FTMS Treadmill** value and **Advanced Settings > Manual Device**. Record whether the intended treadmill or a different nearby fitness device connects at startup.
- Provide one safe, low-speed test of speed and one small incline adjustment from QZ alone, noting which works in the **same connection**, before involving Runna, Zwift, or Kinomap.

### If still unclear

- With other fitness devices powered off, compare **separate QZ restarts** using the explicitly selected FTMS treadmill profile and the supported model-specific profile (if available). Record which profile handles speed versus incline; do not assume they provide identical control capabilities.
- If nRF Connect data is already available, include the advertised **Fitness Machine Service (0x1826)**, **Control Point (0x2AD9)** and supported speed/incline ranges. A control-point success indication such as **80 02 01** or **80 03 01** should be recorded alongside the **actual physical response**, not treated as proof of movement.
- If the difference persists, provide **two short, separately labeled QZ debug logs** with the active profile, connection target, and approximate time of each safe command. Request a manufacturer-app Bluetooth trace only if these comparisons cannot identify the command path.

### Why it matters

- Some treadmills advertise standard FTMS alongside a manufacturer-specific control path. Reading metrics and receiving a successful FTMS response do not establish that **both** physical actuators can be controlled through the same selected protocol.
- Recording speed and incline independently, along with the selected profile, distinguishes **driver/protocol selection** from command rejection, an unexpected physical response, and a third-party app pairing issue.
- Nearby fitness machines can be discovered instead of the intended treadmill; the actual connected Bluetooth name must be confirmed before interpreting control results. See [connection and discovery](connection-and-discovery.md) for device selection intake.
