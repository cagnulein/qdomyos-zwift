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
