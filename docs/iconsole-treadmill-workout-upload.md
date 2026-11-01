# Cardiostrong TX90 native iConsole session and workout upload

## Current supported behavior

For the captured name `iConsole+0014` (also accepting the `I-CONSOLE+0014` spelling, case-insensitively), the native driver connects to the iConsole service, initializes the session, reads metrics, and can upload a fixed twenty-minute speed/inclination program. It uses the ordinary parsed QZ `trainrow` input from XML, ZWO and the Workout Editor. Other iConsole names and AppGate treadmill profiles retain their existing behavior.

The experimental encoder accepts **exactly twenty minutes of positive, whole-minute, non-ramp rows**, with finite nonnegative encodable speeds. A twenty-row workout with one-minute rows or ten rows of two minutes is represented exactly. Missing inclination is encoded as zero. Unsupported timelines are rejected rather than stretched, compressed or sampled across interval boundaries.

This restriction follows the only successful captured program: twenty one-minute slots, an A4 parameter of `0x15`, and an initial console countdown of 19:59. One recording cannot distinguish whether the A4 field is a duration, slot count or related program parameter. Changing it or combining it with thirty-second slots is not yet validated. This supersedes earlier ten-minute/half-minute experimental mappings in this PR.

A conservative input example is available at `train-programs-examples/iconsole-tx90-20min.xml`. Physical validation of the revised QZ build remains necessary.

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
| A4 | Parameter payload `15` followed by ten `01` bytes; exact broader field semantics remain uncertain |
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
6. Earlier workout encoders guessed that twenty half-minute slots and a fixed A4 `15` represented ten minutes. The golden captured transaction proves a twenty-minute program; the revised encoder stays within that evidenced format.
7. ACKs and telemetry require separate complete-frame extraction. The revised receive path handles fragmented/coalesced notifications, validates checksums and routes only B2 into metric decoding. Reconnection clears framing and acknowledged-profile state.

Completing the upload and readiness handshake addresses concrete software defects. The evidence does not independently prove the exact reason the user's physical Start button was blocked after the aborted A4 transaction, or whether a console reset is required after that older failure.

## Validation and remaining limits

- Actual driver and parser-test translation units compiled with Qt 5.15.13. One existing `PlayerStateWrapper::getCadence()` missing-return warning is outside this change.
- **39 focused tests pass**, linked with the actual driver object, including the full eight-request captured profile and recorded idle/ready/running/pause/resume B2 frames.
- Same sixteen-byte B4 regression check against the old and revised compiled driver: old REJECTED, revised ACCEPTED.
- All 3,413 received notifications in the four available QZ logs reassemble without residual bytes. The original HCI contributes 1,049 treadmill notifications, including 1,036 B2 telemetry frames and thirteen ACKs.
- Full platform builds and a physical TX90 test remain necessary. The focused tests do not simulate a GATT peripheral or verify a moving belt.
- Workouts with other total durations, fractional-minute rows or ramps remain unsupported. No timeline is silently changed.
- Live speed writes and a physical iConsole Stop command remain unimplemented; the capture contains neither. Use the treadmill's physical stop for hardware testing.
- QZ's general UI/workout scheduler is unchanged. Emitting `tapeStarted` after movement is the driver's confirmation; this is not a claim that every UI clock path waits for that signal, particularly when continuous-moving settings are enabled.
