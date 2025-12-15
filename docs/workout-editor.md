# Workout Editor

The Workout Editor lets you create multi-device training sessions without leaving QZ.

## Open the Editor
- Drawer → Workout Editor
- Select the target device profile (treadmill, bike, elliptical, rower).

## Build Intervals
- Every interval exposes the parameters supported by the selected device.
- Use **Add Interval**, **Copy**, **Up/Down**, or **Del** to manage the timeline.
- Select a block of consecutive intervals and hit **Repeat Selection** to clone it quickly (perfect for repeat sets like work/rest pairs).
- Toggle **Show advanced parameters** to edit cadence targets, Peloton levels, heart-rate limits, GPS metadata, etc.
- The Chart.js preview updates automatically while you edit.

## Load or Save Programs
- **Load** imports any `.xml` plan from `training/`.
- **Save** writes the XML back into the same folder (name is sanitised automatically).
- **Save & Start** persists the file and immediately queues it for playback.
- Existing files trigger an overwrite confirmation.

## Tips
- Durations must follow `hh:mm:ss` format.
- Speed/incline units follow the global miles setting.
- Saved workouts appear inside the regular “Open Train Program” list.
