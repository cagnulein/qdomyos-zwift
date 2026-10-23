# YPOO treadmills (YPTM-U0 and YP-M1552)

QZ uses the `ypootreadmill` driver for device names that start with `YPTM-U0` or `YP-M1552-`.
You do not need to select a model or change a setting.
Other YPTM models and YPOO Mini Pro use the Horizon driver.

The M5 sends FTMS telemetry, but control requires a vendor unlock sequence.
Telemetry is the data that the treadmill sends to QZ.
A target is a requested speed or incline.

YP-M1552 support comes from upstream PR #5408.
This model uses levels 0–21 for an incline range of 0–12%.
The firmware removes the fractional part of the level in its telemetry.
For example, a 2% target sends level 3.5.
The firmware reports level 3, which is approximately 1.71%.
QZ uses this conversion to confirm YP-M1552 incline targets.

The M5 uses signed percentages directly, including −6% decline.
QZ does not convert percentages to levels for `YPTM-U0`.
The hardware results below apply to the M5 unless the text specifies another model.

## Protocol verified on hardware

1. Subscribe to Control Point indications at `1826/2AD9`.
2. Subscribe to Treadmill Data notifications at `1826/2ACD`.
3. Write `01 00 02 02 00 05 01 01` to `1826/d18d2c10-c44c-11e8-a355-529269fb1459` without response.
4. Request FTMS control with `00` at `1826/2AD9` with response.
   Send this request only once for each unlock attempt.
5. Use the standard FTMS control commands at `2AD9`.

The Mac QZ log shows success (`80 00 01`) for the first control request.
A second control request in the same session returned control-not-permitted (`80 00 05`).

| Command | Example | Hardware result |
| --- | --- | --- |
| Start | `07` | The belt starts at 1.0 km/h after a three-second countdown. |
| Speed | `02 78 00` | Speed changes to 1.2 km/h, although the reply is `80 02 04 2a 03`. |
| Incline | `03 0a 00` | Incline changes to 1% during a run. Telemetry confirms the change. |
| Flat | `03 00 00` | Incline returns to 0% during a run. |
| Stop | `08 01` | The belt stops. Telemetry reports zero speed. |

The treadmill cannot change incline while the belt is stopped.
The firmware can reject Stop during the countdown.
QZ then removes pending targets and retries Stop.
QZ does not retry Start.
The belt can move briefly before the firmware accepts Stop.

The unlock reply can contain the previous FTMS opcode.
QZ does not use that reply to confirm a motor command.

## Incline targets

The M5 advertises these ranges:

| Target | Minimum | Maximum | Increment |
| --- | --- | --- | --- |
| Speed | 1.0 km/h | 18.8 km/h | 0.1 km/h |
| Incline | −6% | 15% | 1% |

Runna sends half-percentage incline targets, although the advertised increment is 1%.
Hardware tests gave these results:

| Request | Reply | Reported incline |
| --- | --- | --- |
| `03 05 00` (0.5%) | `80 03 01 00 00` | 0% |
| `03 0f 00` (1.5%) | `80 03 01 0a 00` | 1% |

The M5 cannot reach half-percentage targets.
Rounding 2.5% to 3% can prevent Runna's decrease button from changing the incline.
QZ selects the whole percentage in the requested direction:

| Current incline | Requested incline | Target sent by QZ |
| --- | --- | --- |
| 3% | 2.5% | 2% |
| 2% | 2.5% | 3% |

Until telemetry confirms the target, duplicate requests keep the same target.
After confirmation, a new request can change the incline in either direction.
QZ rounds other fractional targets to the nearest whole percentage.
Whole-percentage requests keep their values.
Telemetry reports the actual whole percentage.

The owner confirmed that Runna's increase and decrease buttons work with this conversion.
After confirmation, an identical request can mean an app retry or a new opposite button press.
QZ cannot distinguish these two cases, so the request can reverse the change.
Automatic workouts with repeated half-percentage targets remain unverified.

Initial command-line tests used speeds of 1.0–1.2 km/h and inclines of 0–1%.
The owner also tested QZ controls, Kinomap incline and decline, and Runna controls.
Tests did not independently measure the advertised range limits.

## Control and app forwarding

### Initialization

QZ discovers only service `1826` for this driver.
It subscribes to Control Point indications before Treadmill Data notifications.
This order matches the hardware tests.

Initialization requires a control grant and live telemetry.
QZ makes a maximum of three unlock/control attempts.
It does not send motor commands during initialization.
It removes Start and target requests received before initialization completes.
Wait for the "YPOO M5 ready" message before you press Start.

### Command confirmation

QZ waits for fresh telemetry before it sends pending speed or incline targets.
It uses telemetry to confirm each target.
It sends a target a maximum of three times.
Stop has priority over Start and target changes.

After the belt has moved, QZ requires both conditions to confirm Stop:

- The treadmill returns a successful Stop response.
- A subsequent telemetry sample reports zero speed.

QZ sends Stop a maximum of six times.
If the treadmill acknowledged Stop and telemetry stays fresh, QZ permits time for the belt to decelerate.
This period ends 15 seconds after the final Stop write.
QZ sends no additional Stop writes during this period.
It does not restart the belt automatically.

The captured Runna stop took 7.44 seconds to reach zero speed.
This duration exceeded the original retry period for six Stop writes.
If telemetry stops or deceleration exceeds the permitted period, QZ blocks new starts until reconnection.

Telemetry confirmation means that the treadmill reports the requested value.
It does not independently measure belt speed or deck position.
A virtual FTMS success response confirms that QZ accepted the request.
QZ then uses subsequent telemetry to confirm the physical response.
If QZ reaches the retry limit without confirmation, it writes a failure to the debug log.

### Stop during the countdown

The firmware can reject every Stop request if the belt does not move during a canceled countdown.
QZ can confirm this cancellation when all these conditions apply:

- The ten-second startup period has ended.
- Live telemetry reports continuous zero speed for at least two seconds.
- No telemetry gap during that observation exceeds two seconds.

A new start requires an explicit request.
Stop and physical disconnection remove pending targets.
Target changes do not start the belt.
An unconfirmed Stop blocks new starts and targets until reconnection.

### UI Stop

The UI keeps its conditions for an already-stopped workout or zero speed with zero elapsed time.
The `stopWhenIdle()` method requests a physical YPOO Stop before the UI returns.
The UI does not end or save the workout again.

### Recording and distance

Fresh belt motion resumes QZ recording after a start from the console or an app.
QZ does not send another motor Start.
Pending Stop, Pause, or zero-speed requests prevent this automatic recording change during deceleration.
An unconfirmed Stop also prevents the change.

QZ calculates M5 software distance once for each valid telemetry sample.
It uses a separate monotonic clock and the previous speed value.
It does not use the generic timer and notification distance calculations for this model.
During a pause, samples update the time reference without adding distance.
After reconnection, QZ resets the time reference.
It does not add estimated distance for telemetry gaps longer than two seconds.

In one captured run, the console started the belt but QZ recording stayed stopped.
Two distance calculations shared a timestamp.
One calculation skipped distance but updated the timestamp.
The other continued to add distance.
QZ recorded 5.331 km, although the logged speeds gave approximately 6.659 km.
The firmware distance counter reached 6.610 km.

The regression fixture contains all 5,867 speed and timestamp samples from that run.
It contains no identifiers.
Replay tests verify the full 6.658622 km speed integral.
They also verify the change from stopped recording to active recording with an independent one-second UI poll.

### Virtual treadmill

The Qt and optional native iOS interfaces advertise speed and incline control with the supported ranges.
The driver supplies these values through `virtualTreadmillControl()`.
Shared Qt and iOS code does not identify YPOO models.

Native iOS stores commands in a queue.
It does not use whole-second timestamps to separate requests.
Stop removes pending requests for motion.
QZ automatic-control settings still apply.
The training app must send the applicable FTMS requests.
Each app workflow requires an end-to-end test.

## macOS Bluetooth

For this driver, QZ does not add the Device Information service (`180A`) on macOS.
A native CoreBluetooth test confirmed that macOS reserves this UUID and rejects its registration.
This rejection prevents Bluetooth advertising.
The FTMS (`1826`) and Heart Rate (`180D`) services work.

When a training app reconnects, QZ reuses the local services and handles that Qt retained.
It does not register duplicate services.
Other drivers and platform paths keep their existing behavior.

## Regression checks

- Google Test: `YpooM5ControlTest.*`, `YpooM5DistanceTest.*`,
  `YpooM5DistanceClockTest.*`, `YpooTreadmillTest.*`, and `FtmsTreadmillControlTest.*`.
- YPOO and Horizon device discovery cases in `tst/qdomyos-zwift-tests.pro`.
- Native iOS command queue tests and bridge compilation on macOS:
  `python3 tools/test_ypoo_m5_ios_control.py`.
