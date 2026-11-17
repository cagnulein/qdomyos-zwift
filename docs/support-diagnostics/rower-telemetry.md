# Rower telemetry diagnostics

## Rower is discovered, but QZ shows zero or implausible rowing metrics

### Provide first

- Exact rower and console models, the advertised Bluetooth name, QZ build, and phone/tablet operating system.
- Screenshot of the relevant **Rower Options** (for example, **iConsole Rower** versus **iConsole Bike**) and QZ's connection status.
- Two short observations **in QZ before opening EXR, Kinomap, or another app**: what the stroke count, stroke rate/cadence, distance, speed/pace, and watts show **before the first stroke**, then while rowing steadily for 20–30 seconds. Compare with the rower's own display if available.
- State whether a manufacturer's app can read the same rower, and whether it was fully closed before QZ connected.

### If still unclear

- With the rower awake and disconnected from other apps, share a **BLE scanner screenshot** (nRF Connect or an equivalent) showing its advertised name and services. This helps identify which iConsole-compatible protocol variant is present.
- Enable QZ Debug Log and record **one QZ-only session** from discovery through idle, the first strokes, and a brief stop. Include the approximate times and a screenshot of the changing or implausible values.
- If QZ's rowing values are now plausible but the receiving app still shows zero or refuses to start, separately report the app, the advertised **virtual rower** name, and whether QZ's **Virtual Rower** and **PM5** compatibility modes are enabled. See the [rowing FAQ](../faq/rowing.md) for app-specific virtual-profile expectations.

### Why it matters

- A blue Bluetooth connection or a recognized rower name does not establish that the correct packet format is being decoded. The iConsole-compatible device handlers use protocol-specific services and packet layouts.
- Distance advancing while stationary, a frozen stroke counter, or implausible power during gentle strokes points to incoming telemetry interpretation; **correct QZ values but zero values in EXR** points instead to the virtual-device path.
- A confirmed rower comparison progressed from no metrics, to implausible values, to realistic metrics and a working EXR connection after a protocol-specific test build. Capturing the idle-to-rowing transition makes this distinction possible without requesting a long log first.

## Rower is detected but remains at zero, or Start flashes briefly

### Provide first

- Provide the rower's exact model, advertised BLE name, QZ version/platform, and a screenshot of **Rower Options** showing the physical **FTMS Rower** selection and any **Fake Rower** setting.
- Confirm whether QZ shows live stroke rate, distance, or power when rowing **without** a receiving app. Describe what happens when Start is pressed, including any brief button flash.
- State whether the rower is being used as a real Bluetooth machine or whether a simulated rower was intentionally selected.

### If still unclear

- Disable any unintended fake/simulated rower selection, restart QZ, and compare the QZ-only metrics with the intended physical FTMS rower setting. Record the exact setting values before and after.
- If telemetry is still absent, provide one short debug log from discovery through the first strokes and a BLE scanner screenshot showing the advertised device and services. Continue with the telemetry checks above.

### Why it matters

- A simulated rower profile can mask the intended physical-device selection. In one support log, the enabled fake rower option was identified after a recognized FTMS rower showed no values and Start briefly flashed; the configuration mismatch was confirmed, although a successful retest was not yet established.
- Verifying the physical-versus-simulated device choice before requesting protocol traces avoids investigating a BLE parser that QZ may not be using.

## Bluetooth Classic rower reports strokes but not watts or physical resistance changes

### Provide first

- Identify the exact rower/console model, Android version, QZ build, and whether the machine connects over Bluetooth Classic/SPP rather than BLE. Confirm that the manufacturer's app is closed during QZ testing.
- Report separately whether QZ receives **stroke count, stroke rate, distance, and live watts** while rowing, and whether watts return to zero after stopping. Compare a few live values with the console rather than relying on a workout average.
- Record whether **Ignore built-in watts** is enabled and whether QZ is using an external power source.
- For resistance, report the physical console's level range and a short, safe sequence of **nonconsecutive** levels (for example L2 → L4 → L3). For each, distinguish QZ's requested level, any reported level, and whether the actual rowing load changes.

### If still unclear

- Capture one short QZ debug log covering connection, idle, steady strokes, stopping, and two or three deliberate resistance changes. Include approximate timestamps and console readings; keep other training apps closed.
- If the manufacturer's app physically controls resistance but QZ only updates its requested level, obtain a **separate Android Bluetooth HCI snoop capture** of a short manufacturer-app session when support requests it. Close QZ for that capture and annotate the exact level sequence and whether the load changed. Bluetooth Classic RFCOMM traffic may be fragmented, so preserve the original complete capture rather than copied hex fragments.
- If a test build changes power decoding or command handling, compare the same short test with the previous build, recording any command timeouts and whether the machine acknowledges or reports each requested level. Only after physical telemetry and control work should the virtual rower and receiving-app pairing be tested separately.

### Why it matters

- Working stroke and distance telemetry does not establish that the same protocol supplies usable power or accepts resistance writes; QZ's requested resistance level is not evidence of physical movement.
- In a confirmed Bluetooth Classic rower investigation, a comparison of idle and rowing telemetry identified a usable live-power field, while repeated reads of unsupported functions timed out. A separate, time-annotated manufacturer-app HCI capture and an explicitly confirmed nonconsecutive level sequence distinguished actual resistance commands from other changing protocol fields.
- Checking **Ignore built-in watts** before interpreting a zero-watt tile avoids mistaking a source-selection setting for a decoding failure. Distinguishing physical resistance, device-reported acknowledgements, and virtual-app behavior prevents conflating three independent stages.
