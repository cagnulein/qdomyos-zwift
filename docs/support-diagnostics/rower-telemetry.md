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


## QZ shows rowing data, but the receiving app has no live watts

### Provide first

- Exact rower model and console, the rower's advertised Bluetooth name, QZ version/build and platform, and the receiving app and its version.
- State which devices run QZ and the receiving app, and whether the receiving app connects to QZ's advertised rower or directly to the physical machine.
- In QZ, report **live watts and average watts separately**, plus stroke rate, during a short steady row. Include a screenshot of QZ and the receiving app's pairing screen; note whether power is available there and whether it appears during the activity.
- State whether the same symptom occurs on every QZ platform/build in the setup or only one. Identify the exact build used for each comparison.

### If still unclear

- Compare one short QZ-only row with a separate run in which the receiving app is paired. If possible, run the receiving app on a second device so the QZ data path is explicit.
- Record the **Virtual Rower** and **PM5 compatibility** settings for each run. If testing a setting change, change only one option between otherwise comparable runs.
- Provide a separate complete QZ debug log for each run that shows the missing live value, with approximate start/stop times. Include the QZ dashboard values and receiving-app pairing/telemetry screen from the same run.

### Why it matters

- Average watts and live watts can be derived or exposed through different telemetry paths; seeing one does not prove that the live value is available to a receiving app.
- Comparing QZ's own live metrics with the receiving app's pairing screen and in-activity values helps distinguish missing source telemetry from a virtual-device or app-interoperability problem.
- Platform/build and device topology matter when the same rower behaves differently across a native QZ app and a separate bridge host. Recording the exact PM5 mode per run prevents a setting change from being mistaken for a code or platform difference.

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

## Rower power is present but seems too low, or pace and speed disagree

### Provide first

- Exact rower model, QZ build and platform, and whether resistance is manually adjusted or transmitted electronically. Report whether QZ displays measured or estimated watts, and whether an external power source or HR-based estimation is configured (if available for this build and device).
- During a steady comfortable effort, report live watts, stroke rate, **Pace Last 500m**, and speed in QZ, with the units and the corresponding rower-console values if shown. Identify separately instantaneous, average and other summary fields in the receiving app.
- State whether a direct connection to a known working app displays similarly low power, or whether the discrepancy appears only with QZ. Do not connect two apps to the physical rower simultaneously for this comparison.

### If still unclear

- Enable the Pace Last 500m tile if not visible, and provide time-matched screenshots or readings from QZ and the receiving app; identify whether the receiving app uses the QZ virtual PM5 or another profile.
- Send a short QZ debug log of the steady effort. If HR-based power is explicitly being tested on a build that supports it, provide the configured calibration pairs, live HR and displayed estimated watts; keep that separate from the baseline raw-power test.

### Why it matters

- A low watt value reported by both a direct reference app and QZ can originate upstream at the rower, rather than from QZ's virtual-device path. A reviewed case used this comparison to establish similarly low readings.
- Pace in minutes per 500 m and speed in km/h may describe the same quantity (for example, 1:30/500 m is 20 km/h); comparing like-for-like values prevents a unit conversion from being mistaken for a decoding problem. Different summary or instantaneous fields need not agree, and the receiving app's calculation may be unknown.
- Estimated power is not measured power. Check the installed build and selected input path before treating a calibration as evidence of a telemetry defect.
