# Workout recording and history

## Live metrics are visible but a workout is missing or incomplete afterward

### Provide first

- QZ version, operating system, and machine model.
- Whether QZ elapsed time and distance advanced during the activity.
- Whether the QZ Play and Stop buttons were used, or only the machine's physical controls.
- Whether the session is missing from QZ history, the external service, or both.
- Whether QZ remained in the foreground with the screen awake.

### If still unclear

- Run a short session, starting and stopping the workout inside QZ.
- Check QZ history before testing external uploads.
- Provide a QZ debug log with the approximate start and stop times.

### Why it matters

- Live telemetry is not proof that a workout was recorded and finalized.
- Distinguishing local history from external upload failures isolates the affected step.

## Running cadence looks correct in QZ but is doubled in Garmin Connect

### Provide first

- QZ version and platform, treadmill model, and the cadence source (watch, footpod, chest strap, or treadmill).
- One comparison from the **same completed running workout**: cadence shown in QZ while running versus cadence shown in Garmin Connect, with the values and units (steps/min or strides/min).
- Confirm how the activity reached Garmin Connect: **QZ Garmin Connect upload**, an intermediary such as Strava, or a separate watch recording. Include a screenshot of the Garmin activity showing its recording device/source.
- Report the value of **Half Cadence on Strava** under QZ's power-sensor options, even when the destination is Garmin Connect. State whether a *new* workout was recorded after changing it.

### If still unclear

- Record one short new treadmill workout with a stable cadence. Send its **original QZ-generated FIT file**, the QZ debug log from the same session, and the Garmin Connect cadence graph/source screenshot.
- Compare the cadence stored in the FIT records with QZ's live value and Garmin Connect's displayed value **before** changing additional cadence settings. Do not rely on an already-uploaded activity to test a setting change.

### Why it matters

- QZ's FIT exporter applies the **Half Cadence on Strava** option to the cadence field in the exported FIT records, regardless of the upload destination.
- Garmin's presentation of running cadence may differ from the underlying FIT cadence units. Comparing the original FIT and the actual upload route distinguishes export conversion from downstream interpretation or a second recording.

## Strava does not show treadmill elevation on the activity page

### Provide first

- QZ version, treadmill model, and whether QZ showed changing incline and accumulated elevation gain during the workout.
- A screenshot of the individual Strava activity and its activity type/virtual/indoor classification; state whether elevation appears in Strava's weekly totals or another activity viewer.
- State whether the workout was uploaded directly from QZ, via Garmin Connect, or through another service. If available, compare the **same workout** in Garmin Connect.

### If still unclear

- Provide the original QZ FIT file and one screenshot of the QZ workout summary. Compare the FIT session's **total ascent** with what the receiving service displays.
- Check the activity classification against the existing [treadmill/Strava FAQ](../faq/treadmill-troubleshooting.md) before requesting another workout or changing route settings.

### Why it matters

- QZ writes total ascent into the FIT session. A missing elevation field on an individual Strava page does not necessarily mean the workout file lacks elevation.
- Comparing the source FIT, an independent viewer, and the activity classification distinguishes a recording defect from an external display or classification choice.
