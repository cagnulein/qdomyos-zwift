# Treadmill + Runn/Stryd: experimental closed-loop speed control

Reference: [issue #4167](https://github.com/cagnulein/qdomyos-zwift/issues/4167), [source separation fix #5274](https://github.com/cagnulein/qdomyos-zwift/pull/5274), and [Runn feedback discussion](https://github.com/cagnulein/qdomyos-zwift/issues/4167#issuecomment-6076216916).

## Why

FitShow previously used the Runn-versus-machine speed difference **at the moment a new speed was requested**. In the October 8 test log, a request for 10 km/h used Runn = 0.028 km/h and the treadmill = 0.7 km/h (both still accelerating) and mistakenly commanded approximately 10.7 km/h. At 12 km/h the initially applied correction was about +0.5 km/h. PR #5274 correctly separates machine telemetry from Runn telemetry but intentionally retains one-shot speed correction.

## Setup for testing

1. Install an Android test build from the pull request's Actions artifacts (the issue reporter uses Android).
2. Select the Runn sensor in **Settings > Accessories > Power Sensor**.
3. Enable **Use speed from the power sensor**.
4. Leave **Power Sensor as a Treadmill** disabled.
5. Enable **FitShow Runn closed-loop speed correction** (experimental, off by default).
6. Use a QZ-controlled workout; the feature does not take control of manual treadmill-console speed changes.
7. Collect a debug log if a correction is unexpectedly skipped or applied.

## Control policy

- Each speed request from QZ begins by commanding that requested speed, without applying the legacy instant Runn/machine delta.
- The shared treadmill controller runs only when the treadmill has a fresh independent machine-speed reading and is running, the external speed sensor is configured, automatic speed control is enabled, and the experimental option is on.
- Do not correct below 5 km/h (Runn sticker readings at low speeds are unreliable).
- Wait at least 8 seconds after each command and 4 seconds after the treadmill first reports reaching it.
- Keep recent, distinct Runn samples over a 6-second window; require at least 5 observations covering at least 4 seconds. Ignore sensor samples older than 2.5 seconds and machine telemetry older than 3 seconds.
- Remove extreme samples before averaging. Ignore large within-window fluctuations (>2 km/h).
- Apply no correction within +/-0.15 km/h of the target. Otherwise apply a fraction of the error, at most 0.2 km/h per correction. Quantize to 0.1 km/h command resolution.
- Wait for the machine to settle again before any subsequent adjustment. Keep the commanded speed within both the treadmill hardware limits and the user's existing **Power sensor speed correction threshold**, capped at +/-1.5 km/h.
- Give priority to pending BLE commands. A QZ speed target change starts a fresh controller session and drops previous feedback history.
- Abort on stop, pause, unsafe treadmill state, setting disable or sustained speed changes made using physical treadmill buttons. On stale sensor data, hold the current speed; never generate a new correction.

## Physical test matrix

- 6:00 min/km (10 km/h), then 5:00 min/km (12 km/h), with the user running normally: compare Runn speed with target and commanded machine speed over at least two minutes.
- Ensure the next workout interval immediately takes precedence, with no correction carried over from the previous interval.
- Start from rest and test speeds below 5 km/h: no early sensor-based correction.
- Stop/pause from the console and change speed with the physical buttons: no automatic fight-back.
- Disconnect Runn during a run: motor speed remains unchanged, without runaway compensation.
- Repeat with the option disabled: previous behavior is preserved.

The controller's pure C++ policy has automated regression scenarios in `tst/ToolTests/treadmillrunnclosedlooptests.cpp`. **Hardware validation is still required before merging.**
