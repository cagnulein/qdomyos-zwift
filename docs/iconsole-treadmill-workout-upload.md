# Cardiostrong TX90 native iConsole session and workout upload

## Current supported behavior

For the captured name `iConsole+0014` (also accepting `I-CONSOLE+0014`, case-insensitively), the native driver initializes the session, reads metrics, and uploads speed/incline programs from parsed QZ `trainrow` input. Other iConsole/AppGate names retain their existing path.

Hardware has verified complete twenty-minute and ten-minute runs, including countdown and automatic stop. The encoder still accepts only those total durations. With exactly twenty positive, non-ramp rows it now maps each row directly to a protocol slot and preserves its duration in seconds. This variable-duration path awaits hardware validation. Other row counts retain the previous exact expansion into 60-second slots (twenty minutes) or 30-second slots (ten minutes), requiring each row to align to that slot length. Speeds must be finite, nonnegative and encodable; missing incline becomes zero. Unsupported input is rejected without rescaling or sampling.

November 9 established that A9/AA controlled the 30-second interval changes while A4=`0x15` still kept the total countdown at twenty minutes. On November 12, changing only the ten-minute A4 parameter to `0x0b` produced a 9:59 countdown and automatic stop after ten minutes. A4 therefore behaves as total minutes + 1 for the two verified totals; arbitrary durations remain untested.

Examples: `train-programs-examples/iconsole-tx90-20min.xml` and `iconsole-tx90-10min-30sec.xml` are hardware-verified. The new `iconsole-tx90-10min-variable.xml` alternates 20/40-second intervals, preserving the same twenty speed/incline slots and verified ten-minute A4.

## Evidence inspected

Both original HCI files supplied by Matthias were parsed completely:

- `20261021_iConsole_Prtogram-Test.pklg`: 7,650 records, including metadata and HCI commands/events.
- `20261021_iConsole_Prtogram-Test - (null).log`: BTSnoop version 1, link type 1001, 7,623 records.

The two formats contain **the same 4,398 ACL packets in the same order and direction**. The extra PKLG records are metadata. ACL/L2CAP reassembly yields 4,397 ATT packets across the connections present in the recording. The treadmill's connection is distinguished from the pre-existing other connection; its application notifications are on ATT handle 0x0012, not the other connection's handle 0x0032.

Four complete QZ debug logs were also inspected: October 26 at 18:02 and 18:51, and October 31 at 13:58 and 22:00. Available QZ/nRF/Zwift screenshots were inspected. Three earlier QZ logs from October 21 and two from October 29 remain unavailable through the email connector. The supplied PDF contains no Bluetooth protocol data.

No private full HCI recording is committed. The tests contain only the protocol frames needed to reproduce the behavior. The earlier candidate-APK interpretation is not required for the exact captured program below.

## Complete successful iConsole flow

### Transport and initialization

The captured device exposes service FFF0. Its writable characteristic is FFF2 at ATT handle 0x0010, and its notification characteristic is FFF1 at handle 0x0012; CCCD is handle 0x0013. Writes use ATT Write Request, followed by ATT Write Response. Application acknowledgements are separate F0 Bx notification packets.

After enabling notifications, the app sends these initialization requests and receives their corresponding replies:

| Request | Reply |
| --- | --- |
| `F0 A0 00 00 90` | `F0 B0 01 D3 74` |
| `F0 A0 01 D3 64` | `F0 B0 01 D3 74` |
| `F0 A1 01 D3 65` | `F0 B1 01 D3 01 15 02 01 10 9E` |

It then polls with `F0 A2 01 D3 66`, approximately once per 500 ms. B2 replies are 19 bytes with an additive checksum. QZ now matches this polling cadence for native iConsole devices.

### Program upload

The ordered upload is A3, A4, A6, A7, A8, A9, AA, AF. Each request receives its matching B3/B4/B6/B7/B8/B9/BA/BF application acknowledgement before the next request. Twenty-five-byte requests are split into twenty bytes followed by five bytes, with ATT write completion between fragments and no poll interleaving.

| Command | Captured meaning established by the complete flow |
| --- | --- |
| A3 | Program selection payload `02 01 01 01 01 01 01` |
| A4 | Hardware-verified 20-minute payload: `15` followed by ten `01` bytes. New **unverified** 10-minute probe: `0b` followed by the same ten `01` bytes (possible total-minutes-plus-one field) |
| A6 | First ten speed slots |
| A7 | Second ten speed slots; the recorded second half repeats the first half |
| A8 | Twenty inclination values |
| A9 | First ten duration slots, each encoded `02 01` (one minute) |
| AA | Second ten duration slots, also one minute each |
| AF | Final control payload `02`; its receipt alone does not prove console readiness or movement |

Speeds use `(deci-km/h / 100) + 1` and `(deci-km/h % 100) + 1`; inclination uses value + 1. The final byte of every logical frame is the sum of preceding bytes modulo 256. The golden test reproduces **all eight full upload requests byte for byte**, including their checksums, from the observed twenty-slot input.

The first ten captured speeds are 4, 6, 8, 10, 12, 14, 12, 10, 8, 6 km/h. The first twenty inclination values are 2, 2, 9, 9, 9, 9, 12, 12, 12, 12, 12, 5, 5, 5, 5, 5, 5, 0, 0, 0 percent.

### Readiness, Start, running and pause

B2 bytes 15–17 distinguish these observed states. The tuples are empirical; the recording does not establish a complete firmware state dictionary.

| Observation | B2 bytes 15–17 | Speed |
| --- | --- | --- |
| Initial idle after connection | `04 04 02` | 0 |
| Console ready before Start/resume | `03 02 02` | 0 |
| Running after A5 | `03 04 03` | Positive |
| Paused/stopped belt | `04 04 03` | 0 |

After AF is acknowledged, iConsole resumes polling. Approximately 5.6 seconds later the console reports `03 02 02`. The app sends `F0 A5 01 D3 02 6B` approximately three seconds after that observation. Its B5 response is `F0 B5 01 D3 02 7B`. The next B2 reports 4 km/h, 2% incline and a countdown of 19:59.

QZ retains an explicit Start request through upload and subsequent readiness polling. It sends A5 only when the latest profile is acknowledged and the console is ready. It emits `tapeStarted` only after running B2 telemetry with positive speed, rather than treating B5 as proof that the belt moved. Actual metric assignment precedes that signal. Stop cancels a deferred Start. Without an uploaded program, the legacy handshake for the console's own Start-ready transition is preserved. Loading a program by itself does not manufacture a QZ Start request.

The capture later reports 6, 8, 10, 12 and 14 km/h at approximately one-minute boundaries; inclination changes follow the A8 table. There are **no live speed writes** between those transitions: A2 polling continues while the console executes the uploaded program.

The belt then pauses, with a frozen countdown of 14:35, later reports the ready tuple, receives another A5 `02`, and resumes at 14 km/h with 14:34 remaining. The final pause leaves 13:40 remaining. No application Stop command is present in this recording, so it cannot establish the bytes for a reliable remote Stop.

## What failed in QZ

1. The October 31 13:58 Android build logged that initialization would accompany the profile, but began at A3 without A0/A1. No incoming notifications appear in that entire session, even when the user reports manually running the belt. The later branch restores the captured initialization.
2. In the 22:00 session initialization succeeds, followed by regular B2 replies with zero speed. The Bluetooth link is communicating; there is no observed movement in that log.
3. At 22:01:04.411 a workout is queued inside a polling response wait. At 22:01:04.655 the old code consumes Start before the deferred upload begins. Start must remain queued until upload and console readiness complete.
4. The TX90 responds to A4 with `F0 B4 01 D3 15 01 01 01 01 01 01 01 01 01 01 97`: **16 bytes**, with a valid checksum. The old branch expected 17, timed out, and stopped before sending A6/A7/A8/A9/AA/AF/A5. The response size is corrected.
5. The previous implementation sent Start immediately after profile acknowledgement instead of waiting for the ready tuple. The revised sequence follows the successful capture's state order; it does not invent a fixed delay or infer readiness from an ACK.
6. The November 9 trial with 20 half-minute slots and A4=`15` succeeded in changing programmed speed/incline every 30 seconds, but the console's total countdown still started at 19:59 and did not stop at ten minutes. The verified twenty-minute encoder is unchanged. The November 12 follow-up changed only A4's first parameter (plus checksum), and verified the ten-minute countdown and automatic stop.
7. ACKs and telemetry require separate complete-frame extraction. The revised receive path handles fragmented/coalesced notifications, validates checksums and routes only B2 into metric decoding. Reconnection clears framing and acknowledged-profile state.

Completing the upload and readiness handshake addresses concrete software defects. The evidence does not independently prove the exact reason the user's physical Start button was blocked after the aborted A4 transaction, or whether a console reset is required after that older failure.

## Validation and remaining limits

- Actual driver and parser-test translation units compiled with Qt 5.15.13. One existing `PlayerStateWrapper::getCadence()` missing-return warning is outside this change.
- Prior revision: **39 focused tests passed**, linked with the actual driver object, including the full eight-request captured profile and recorded idle/ready/running/pause/resume B2 frames.
- Current revision: **46 parser/encoder tests passed** in an isolated Qt 5.15.13 harness using production static helper bodies verbatim. Checks include the twenty-minute golden upload, ten-minute A4/B4, variable 20/40-second A9/AA bytes, minute/second encoding and rejection of nonrepresentable row counts. A pre-existing constant-ten-minute test fixture was corrected to contain all ten duration pairs. This is not a complete application build; platform CI is separate.
- Same sixteen-byte B4 regression check against the old and revised compiled driver: old REJECTED, revised ACCEPTED.
- All 3,413 received notifications in the four available QZ logs reassemble without residual bytes. The original HCI contributes 1,049 treadmill notifications, including 1,036 B2 telemetry frames and thirteen ACKs.
- The November 3 TX90 hardware test completed the full twenty minutes: all eight upload requests were acknowledged, Start followed readiness, all twenty speed/incline slots matched, and the treadmill stopped at countdown zero before QZ Stop. The photo shows `00:09` remaining near the end; additional graph columns do not establish additional running time.
- November 12: all 1,517 received frames have valid checksums (1,505 B2 telemetry records, initialization ACKs, eight upload ACKs and B5). A4=`0x0b` is echoed by B4; readiness precedes A5. The first populated countdown is 599 seconds. All twenty interval midpoint speed/incline values match. Positive-speed telemetry runs from 13:52:50 to 14:02:51 (600.039 seconds between first/last positive samples), followed by zero speed and end state `02 04 02`. No remote Stop write occurs. The user's email independently confirms automatic termination.
- The new variable-duration test and pause/resume/reload checks remain pending. Other total durations and ramps remain unsupported. No timeline is silently changed.
- Live speed writes and a physical iConsole Stop command remain unimplemented; the capture contains neither. Use the treadmill's physical stop for hardware testing.
- QZ's general UI/workout scheduler is unchanged. Emitting `tapeStarted` after movement is the driver's confirmation; this is not a claim that every UI clock path waits for that signal, particularly when continuous-moving settings are enabled.

## Next hardware test: variable interval durations

Use the updated build and `train-programs-examples/iconsole-tx90-10min-variable.xml`: twenty rows alternating 20 and 40 seconds, ten minutes total. Speeds remain 3/4/5/4/3 km/h and inclines 0/0/1/1/0%, repeated four times. Against the verified uniform ten-minute example, only A9/AA duration tables change; A3/A4/A6/A7/A8/AF remain identical. Each ten-slot table contains five `01 15 01 29` groups, with checksums `ad` (A9) and `ae` (AA).

1. Install the new variable-duration build, enable debug logging and open the XML through **Open Train Program**. The prior build rejects the variable rows.
2. Start normally and check countdown near 9:59. From belt movement, the first changes should occur at 20 seconds (4 km/h, 0%), 60 seconds (5 km/h, 1%), 80 seconds (4 km/h, 1%) and 120 seconds (3 km/h, 0%). Some adjacent rows have identical targets.
3. A two-minute run verifies the first boundaries. If completing the run, verify automatic stop at ten minutes. Use the physical treadmill Stop if ending early or if behavior differs; QZ remote Stop is unverified.
4. After physical stop and return to idle, load the already verified uniform ten-minute XML again without restarting QZ or reconnecting. A short start confirms that a second upload works in the same connection. End that check with physical Stop.
5. Separately, with the verified uniform XML, test the console's physical Pause and normal Resume/Start controls and note which controls were used. Check whether the remaining countdown is preserved. Do not assume that QZ remote Pause/Stop works. Resume behavior on the revised QZ session is still pending hardware validation.
6. Return the complete log. The November 12 email mentions two videos, but its MIME attachments contain only the debug log; request the videos again before investigating the console graph. Extra graph columns alone do not establish extra execution time.
