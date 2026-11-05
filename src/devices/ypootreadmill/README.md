# YPOO treadmill driver

The `ypootreadmill` driver connects YPOO treadmills to QZ through Bluetooth FTMS.
Device discovery selects the driver automatically from the Bluetooth name.

| Name prefix | Speed range | Incline range | Incline encoding |
| --- | --- | --- | --- |
| `YPTM-U0` | 1.0–18.8 km/h | −6–15% | Signed percentage in 0.1% units |
| `YP-M1552-` | 1.0–18.8 km/h | 0–12% | Levels 0–21 in 0.1-level units |

Both models use 0.1 km/h speed steps and 1% incline targets.
Other YPTM models and YPOO Mini Pro use the Horizon driver.

## Connection and handshake

The driver discovers only the Fitness Machine service (`1826`).
It uses these characteristics:

| Characteristic | UUID | Purpose |
| --- | --- | --- |
| Fitness Machine Control Point | `2AD9` | FTMS commands and response indications |
| Treadmill Data | `2ACD` | Speed, incline, distance and other telemetry |
| Vendor unlock | `d18d2c10-c44c-11e8-a355-529269fb1459` | Enable control before the FTMS control request |

Initialization follows this sequence:

1. Subscribe to `2AD9` indications.
2. Subscribe to `2ACD` notifications.
3. Write `01 00 02 02 00 05 01 01` to the vendor unlock characteristic.
4. Wait 500 ms after the unlock write procedure.
5. Send Request Control (`00`) to `2AD9` once for this unlock attempt.
6. Require a successful control response and fresh telemetry before enabling motor commands.

The driver selects the write mode from the characteristic properties.
It prefers Write Without Response when the characteristic supports it.
For Control Point procedures, it waits up to one second for a response with the matching opcode.
Telemetry and unrelated responses do not complete the procedure.
The unlock reply does not confirm a motor command.

Initialization has a maximum of three unlock/control attempts.
It sends no motor commands and discards early Start and target requests.
Successful initialization displays "YPOO M5 ready".
A physical disconnection clears pending commands, resets the controller and distance clock, and starts reconnection.
Each new physical connection requires the handshake again.

## Commands and telemetry

The driver uses standard FTMS commands on `2AD9`.
All multi-byte command values use little-endian byte order.

| Command | Packet | Value |
| --- | --- | --- |
| Start | `07` | Start or resume the belt |
| Speed | `02` + unsigned 16-bit value | Speed × 100; `02 78 00` requests 1.2 km/h |
| Incline | `03` + signed 16-bit value | Incline percentage or model-specific level × 10 |
| Stop | `08 01` | Stop the belt |

`YpooM5Control` serializes motor commands with at least 250 ms between writes.
Speed and incline targets require telemetry no more than two seconds old.
The controller holds targets during the startup countdown until telemetry reports motion.
Target changes alone do not start the belt.
Start is never automatically retried.
If Start produces no motion within ten seconds, the controller cancels targets and requests Stop.

Speed and incline confirmation uses telemetry because the firmware can apply a speed command while returning an FTMS failure.
Each target has a maximum of three writes, with at least 2.5 seconds between attempts.
If confirmation fails, QZ records the failure in its debug log.
A virtual FTMS success response means that QZ accepted the request; physical confirmation follows separately.

The telemetry parser requires instantaneous speed and incline, and validates the length of all declared fields.
It ignores RFU bits and additional vendor bytes.
Invalid or incomplete packets do not update the controller or metrics.

### Incline conversion

For `YPTM-U0`, the driver sends and reads signed incline percentages directly.
For `YP-M1552-`, it converts a percentage to a level as follows:

```text
command level = round(percent × 17.5) / 10
reported percent = reported level × 12 / 21
```

YP-M1552 confirmation allows for firmware that truncates the reported level to an integer.
It compares telemetry with the converted truncated level, rather than the original percentage target.

The controller rounds fractional incline requests to whole percentages.
For halfway values, it selects the whole percentage in the requested direction.
Thus, a 2.5% request from 3% becomes 2%; the same request from 2% becomes 3%.
Duplicate requests keep the same target until confirmation.
After confirmation, the controller treats an identical request as a new request.
An app retry and a new opposite button press are indistinguishable at that point.

### Stop and countdown cancellation

Stop takes priority over Start and target changes.
Pause and zero-speed requests also use the Stop command.
Stop clears pending targets and has a maximum of six writes, at least one second apart.

After motion, Stop confirmation requires a successful response followed by zero-speed telemetry.
If Stop was acknowledged and telemetry stays fresh, the controller permits deceleration until 15 seconds after the final write.
It sends no further Stop commands during this period.
Missing telemetry or an expired confirmation period puts the controller in a fault state.
New starts and targets then require reconnection.

For a canceled countdown with no motion, the controller also accepts continuous zero-speed telemetry after the ten-second startup period.
This requires at least two seconds of zero-speed samples, with no gap longer than two seconds.
A new start always requires an explicit request.

The UI retains its guards for an already-stopped workout or zero speed with zero elapsed time.
Before those guards return, `stopWhenIdle()` requests a physical Stop without repeating workout finalization or saving the workout again.

## Recording and distance

Fresh belt motion can resume QZ recording after a console or app start without sending another motor Start.
Pending Stop, Pause, zero-speed requests and a controller fault block this transition during deceleration.

By default, `YpooM5Distance` calculates distance once per valid telemetry sample:

```text
distance increment in km = previous speed in km/h × elapsed milliseconds / 3,600,000
```

The calculation uses a monotonic clock separate from the UI update timer.
It adds distance only while QZ recording is active and the sample interval is greater than zero and no more than two seconds.
The first sample establishes the time and speed reference.
Paused samples update that reference without adding distance.
Duplicate timestamps and longer telemetry gaps add no distance.
A physical disconnection resets the reference without resetting the workout distance.
The driver does not also add distance through the generic timer or notification calculations.

If `treadmill_direct_distance` is enabled, the driver instead uses the FTMS Total Distance field when present.
It reads that field as an unsigned 24-bit meter counter and converts it to kilometers.
Software distance increments do not contribute in this mode.

Enable `treadmill_direct_distance_increment` with `treadmill_direct_distance` to add only increases in the counter.
Counter resets preserve the accumulated QZ distance; paused samples update the reference without adding distance.
The first sample after connection establishes a reference, and implausible positive jumps are ignored.
Both options default to off; speed-based distance and the existing absolute-counter mode remain unchanged.

## Virtual treadmill

`virtualTreadmillControl()` supplies the forwarding capability and encoded speed/incline ranges to the shared Qt and native iOS interfaces.
Model detection and range selection stay in the physical driver.
Existing QZ automatic-control settings apply to incoming requests.

The native iOS interface uses a queue of up to 16 commands, without whole-second timestamps.
Stop and zero-speed requests replace pending motion commands.
A training-app disconnect clears pending motion while retaining an accepted Stop.
A physical disconnection clears the forwarding queue.

On macOS, this forwarding mode omits the reserved Device Information service (`180A`) so FTMS advertising can start.
When a training app reconnects, it reuses Qt's retained local services and handles instead of registering duplicates.
