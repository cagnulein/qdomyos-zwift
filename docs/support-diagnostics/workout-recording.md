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
- If Garmin Connect's cadence **graph** looks correct but its **average/maximum summary** is doubled, include both views and the exact QZ build; these are different FIT fields.
- Confirm how the activity reached Garmin Connect: **QZ Garmin Connect upload**, an intermediary such as Strava, or a separate watch recording. Include a screenshot of the Garmin activity showing its recording device/source.
- Report the value of **Half Cadence on Strava** under QZ's power-sensor options, even when the destination is Garmin Connect. State whether a *new* workout was recorded after changing it.

### If still unclear

- Record one short new treadmill workout with a stable cadence. Send its **original QZ-generated FIT file**, the QZ debug log from the same session, and the Garmin Connect cadence graph/source screenshot.
- Compare the cadence stored in the FIT records with QZ's live value and Garmin Connect's displayed value **before** changing additional cadence settings. Do not rely on an already-uploaded activity to test a setting change.

### Why it matters

- QZ's FIT exporter applies the **Half Cadence on Strava** option to the cadence field in the exported FIT records, regardless of the upload destination.
- FIT record samples and FIT session summary cadence are stored separately. Current QZ code applies the conversion to both; a graph/summary mismatch on an older build needs the original FIT file and version before assuming a sensor problem.
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

## Garmin Connect imports a QZ workout with Training Effect but no VO2max update

### Provide first

- QZ version and platform, activity type (indoor cycling, treadmill, etc.), and whether the workout was uploaded by **QZ to Garmin Connect** or recorded directly on a Garmin watch through ANT+.
- Screenshot the relevant QZ Garmin/FIT export settings, especially **Training Effect**, the selected **Garmin device/product**, and whether its device ID/serial is the default or customized. Mask any personal device identifiers in screenshots.
- Screenshot the resulting Garmin Connect activity showing which Training Effect or VO2max fields appear, and specify whether the concern is a missing activity metric or no change to the account's fitness estimate.
- State whether heart rate and power were present in the workout and give its approximate duration and intensity.

### If still unclear

- Provide the original QZ-generated FIT file for that workout and a screenshot of the Garmin Connect activity details. Check the file's device identity and session metrics before changing multiple settings.
- If support asks for a comparison, record a short new workout with the requested Garmin device selection and note the exact settings and result. Do not assume an existing activity will be recalculated after settings change.
- Request a QZ debug log only if the upload itself fails or there is evidence that expected data was not exported; a missing VO2max estimate alone is not a Bluetooth diagnostic.

### Why it matters

- QZ's FIT exporter writes manufacturer, product and serial/device identity according to its Garmin Training Effect and device-selection settings; an unexpected selection can affect how the activity is presented.
- Uploading a FIT activity to Garmin Connect and broadcasting live data to a Garmin watch are different paths. A successful upload or Training Effect value does **not** by itself prove Garmin will update VO2max for that workout.
- The [Garmin integration FAQ](../faq/integrations.md) describes the two paths; this intake distinguishes file metadata, missing sensor data and Garmin-side eligibility before recommending a configuration change.
