# YPOO M5 (YPTM-U0)

The M5 advertises FTMS telemetry but requires a vendor handshake before control.
QZ detects the `YPTM-U0` name prefix and uses the existing Horizon/FTMS driver.
No model selection or new setting is required. Other YPTM models retain their
existing behavior.

## Protocol verified on hardware

1. Subscribe to Treadmill Data (`1826/2ACD`) and Control Point indications
   (`1826/2AD9`).
2. Write `01 00 02 02 00 05 01 01`, without response, to
   `1826/d18d2c10-c44c-11e8-a355-529269fb1459`.
3. Request FTMS control (`00`) on `1826/2AD9` using a write with response.
4. Use standard FTMS control commands on `2AD9`:

| Action | Example | Hardware observation |
| --- | --- | --- |
| Start | `07` | Starts at 1.0 km/h after a three-second countdown |
| Speed | `02 78 00` | Changes to 1.2 km/h despite reply `80 02 04 2a 03` |
| Incline | `03 0a 00` | Changes to 1% during a run; telemetry confirms it |
| Flat | `03 00 00` | Returns to 0% during a run |
| Stop | `08 01` | Stops the belt; telemetry returns to zero |

Incline cannot change at rest. A stop sent during the startup countdown may be
rejected, so QZ cancels queued targets and retries stop without retrying start.
The belt can start briefly before the firmware accepts stop.
The unlock acknowledgement can repeat the previous FTMS opcode and is not used
as evidence that a motor command succeeded.

The advertised ranges are 1.0–18.8 km/h in 0.1 km/h steps and -6–15% in 1% steps.
Live testing used low speeds and 0–1% incline; the range endpoints were not
physically exercised.

## Control and app forwarding

QZ waits for fresh physical telemetry before applying queued speed/incline
targets. It confirms targets with telemetry, allows at most three target writes,
and prioritizes stop. Stop confirmation requires both a successful stop response
and a subsequent zero-speed sample. Stop/disconnect discard pending targets;
target changes never implicitly start the belt.
An unconfirmed stop disables new starts/targets until reconnection.

The Qt and opt-in native iOS virtual treadmill paths advertise speed/incline
control and the supported ranges. Native iOS commands are queued without
second-resolution timestamps; stop supersedes pending motion requests. Existing
QZ auto-control settings still apply. An app must send the relevant FTMS control
requests; application-specific workflows remain subject to end-to-end testing.

## Regression checks

- Google Test: `YpooM5ControlTest.*`, `FtmsTreadmillControlTest.*`, and Horizon
  device discovery cases in `tst/qdomyos-zwift-tests.pro`.
- Native iOS queue and bridge compilation on macOS:
  `python3 tools/test_ypoo_m5_ios_control.py`.
