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

QZ waits for fresh treadmill-reported telemetry before applying queued speed/incline
targets. It confirms targets with telemetry, allows at most three target writes,
and prioritizes stop. Stop confirmation requires both a successful stop response
and a subsequent zero-speed sample. Stop/physical disconnect discard pending targets;
target changes never implicitly start the belt.
An unconfirmed stop disables new starts/targets until reconnection.
Confirmation means that the treadmill reports the requested value; it is not an
independent measurement of belt speed or deck position. The virtual FTMS success
response acknowledges acceptance by QZ, not completed physical actuation. Actual
application is checked asynchronously against treadmill telemetry; exhausted
retries are recorded in the QZ debug log.

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

## Live app acceptance test

Use QZ on one device and Kinomap on another. Close YPOOFIT, Kinni, and other BLE
test clients first. Connect QZ to the physical `YPTM-U0…` treadmill, then connect
Kinomap to QZ's virtual FTMS treadmill (`DomyosBridge` on macOS, `QZ` with the
native iOS bridge). Do not select the physical treadmill in Kinomap.
Keep QZ Auto Resistance enabled, virtual Bluetooth enabled, and bike/cadence-only
virtual modes disabled. Enable QZ debug logging before the test and restart QZ.

| Test | Expected result |
| --- | --- |
| Explicit start, followed immediately by 1.2 km/h and 1% targets | One start; targets wait through countdown, then telemetry and console reach both targets |
| During a run: 1.2 → 1.0 km/h and 1 → 0% incline | Both changes reach the console and QZ; the misleading speed failure reply does not cause an immediate retry loop |
| During a run: 0 → -1 → 0% incline | Deck, console, QZ, and virtual FTMS telemetry preserve decline without clamping to zero |
| Stop during a run | Successful stop reply followed by zero-speed telemetry; belt visibly stops |
| Stop during countdown with targets queued | Targets are discarded; only bounded stop retries follow; firmware may briefly start the belt |
| Change a target repeatedly | Latest target wins; earlier targets are not replayed later |
| Physical stop, then physical restart | Previous pending workout targets are not replayed |
| Stop, disconnect the physical treadmill, reconnect | No queued start or target is replayed; a new explicit start is required |

Record the app request, QZ outgoing command, control-point reply, subsequent
telemetry, and visual observation for each row. Mark an app control as untested
if the app does not send that FTMS request; test that control with QZ's own UI
separately. A Mac session exercises the Qt virtual peripheral path; repeat on
an iOS build to validate the native iOS forwarding path end to end.

### Building on macOS

Install Qt 5 and Apple Command Line Tools. From the repository root, with pinned
submodules initialized:

```sh
brew install qt@5
git submodule update --init src/smtpclient tst/googletest zwiftplay
/opt/homebrew/opt/qt@5/bin/lrelease src/qdomyos-zwift.pro
qz_source="$PWD"
mkdir -p /tmp/qz-macos-build
cd /tmp/qz-macos-build
/opt/homebrew/opt/qt@5/bin/qmake "$qz_source/src/qdomyos-zwift.pro" CONFIG-=ltcg CONFIG-=optimize_full
make -j6
open qdomyos-zwift.app --args -qml -name YPTM-U012A6B
```

The Qt path above is for Apple Silicon Homebrew. On Intel use the path reported
by `brew --prefix qt@5`. This runs the branch built from the checkout; an existing
released QZ app does not contain this change.
