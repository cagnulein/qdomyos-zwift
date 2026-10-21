# YPOO M5 (YPTM-U0)

The M5 advertises FTMS telemetry but requires a vendor handshake before control.
QZ detects the `YPTM-U0` name prefix and uses the existing Horizon/FTMS driver.
No model selection or new setting is required. Other YPTM models retain their
existing behavior.

## Protocol verified on hardware

1. Subscribe to Control Point indications (`1826/2AD9`), then Treadmill Data
   (`1826/2ACD`).
2. Write `01 00 02 02 00 05 01 01`, without response, to
   `1826/d18d2c10-c44c-11e8-a355-529269fb1459`.
3. Request FTMS control (`00`) once per unlock attempt on `1826/2AD9` using a write with response.
   The Mac QZ log confirmed success (`80 00 01`) for the first request, but
   control-not-permitted (`80 00 05`) for a second request in the same session.
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
Runna was observed requesting 0.5% incline steps despite the advertised increment.
A direct hardware probe confirmed that `03 05 00` (0.5%) returns
`80 03 01 00 00` and reports 0%; `03 0f 00` (1.5%) returns
`80 03 01 0a 00` and reports 1%. Half-percent requests are not achievable targets.
Nearest-step rounding can leave Runna's decrease button unchanged (2.5% maps to
3%). QZ instead resolves halfway targets in the requested direction: 2.5% from
3% maps to 2%, while 2.5% from 2% maps to 3%. Duplicate requests retain the same
mapping while the movement is unconfirmed. After telemetry confirms the target,
a new request can move in either direction. Other fractions retain nearest-step
rounding, and telemetry always reports the actual whole percentage.
The owner confirmed that Runna's increase/decrease buttons work with this mapping.
A repeated half-step after confirmation is still indistinguishable from a new
opposite button press and can reverse the change; automatic half-step workout
targets are not verified. Whole-percentage requests retain their existing mapping.
CLI checks used 1.0–1.2 km/h and 0–1% incline. Further owner tests exercised QZ,
Kinomap incline/decline, and Runna controls. The advertised range endpoints have
not been independently measured.

## Control and app forwarding

QZ discovers only service `1826` for this model, subscribing to Control Point
indications before Treadmill Data, matching the verified BLE transport.
Initialization requires both a control grant and live telemetry, with at most
three non-moving unlock/control attempts. Early Start/target requests are
discarded; motor commands are never part of initialization. Wait for the
"YPOO M5 ready" toast before pressing Start.

QZ waits for fresh treadmill-reported telemetry before applying queued speed/incline
targets. It confirms targets with telemetry, allows at most three target writes,
and prioritizes stop. Once motion has been reported, stop confirmation requires
a successful stop response followed by a zero-speed sample. If startup is
cancelled before any motion, the firmware may reject every stop despite staying
at rest. That cancellation can instead complete after the ten-second startup
window expires and live telemetry confirms at least two seconds of continuous
zero speed without a telemetry gap longer than two seconds. A new start still
requires an explicit request. Stop/physical disconnect discard pending targets;
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

On macOS, the M5 virtual peripheral omits Device Information (`180A`): a native
CoreBluetooth probe confirmed that macOS reserves this UUID and rejects adding
it, preventing advertising from starting. FTMS (`1826`) and Heart Rate (`180D`)
are accepted. On client reconnect, the Mac peripheral reuses the local services
and handles retained by Qt instead of registering duplicates. These restrictions
do not change other device/platform paths.

## Live app acceptance test

Use QZ on one device and Kinomap or Runna on another. Close YPOOFIT, Kinni, and other BLE
test clients first. Connect QZ to the physical `YPTM-U0…` treadmill, then connect
the training app to QZ's virtual FTMS treadmill (macOS may show the computer's
Bluetooth name rather than `DomyosBridge`; the native iOS bridge uses `QZ`).
Do not select the physical treadmill in the training app.
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
| Runna incline +, wait for telemetry, then − | Half-step requests move to the next whole percentage in each direction; telemetry reports the actual grade |
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
