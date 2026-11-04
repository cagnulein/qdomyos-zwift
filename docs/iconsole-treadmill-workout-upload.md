# Cardiostrong TX90 native iConsole session and workout upload

## Current supported behavior

For the captured name `iConsole+0014` (also accepting the `I-CONSOLE+0014` spelling, case-insensitively), the native driver connects to the iConsole service, initializes the session, reads metrics, and can upload the hardware-verified twenty-minute speed/inclination program and an experimental ten-minute duration test. It uses the ordinary parsed QZ `trainrow` input from XML, ZWO and the Workout Editor. Other iConsole names and AppGate treadmill profiles retain their existing behavior.

The encoder accepts **exactly twenty minutes of positive, whole-minute, non-ramp rows**, or the **experimental ten-minute format with rows in whole 30-second multiples**, with finite nonnegative encodable speeds. Both formats expand exactly into twenty slots (60 or 30 seconds respectively). Missing inclination is encoded as zero. Unsupported timelines are rejected rather than stretched, compressed or sampled across interval boundaries.

The twenty-minute format follows the successful captured program and the completed November 3 hardware test. A4 stays at `0x15` in both formats. Its meaning is still uncertain: the shorter experiment changes only A9/AA to distinguish duration-table behavior from A4. Thirty-second slots are not yet hardware-validated; this is a controlled test, not general support for arbitrary durations.

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
6. Earlier workout encoders guessed that twenty half-minute slots and a fixed A4 `15` represented ten minutes. The golden captured transaction proves a twenty-minute program; the twenty-minute path now follows that evidenced format. The new ten-minute experiment is explicitly awaiting hardware validation.
7. ACKs and telemetry require separate complete-frame extraction. The revised receive path handles fragmented/coalesced notifications, validates checksums and routes only B2 into metric decoding. Reconnection clears framing and acknowledged-profile state.

Completing the upload and readiness handshake addresses concrete software defects. The evidence does not independently prove the exact reason the user's physical Start button was blocked after the aborted A4 transaction, or whether a console reset is required after that older failure.

## Validation and remaining limits

- Actual driver and parser-test translation units compiled with Qt 5.15.13. One existing `PlayerStateWrapper::getCadence()` missing-return warning is outside this change.
- Prior revision: **39 focused tests passed**, linked with the actual driver object, including the full eight-request captured profile and recorded idle/ready/running/pause/resume B2 frames.
- Current duration change: **42 tests passed** in an isolated Qt 5.15.13 harness using the production static helper bodies verbatim and all checked-in parser/encoder tests. This checks the unchanged twenty-minute golden frames, exact thirty-second A9/AA frames and rejection of misaligned rows; it is not a complete application build. Both XML examples have twenty identical speed/incline slots, with totals of 1,200 and 600 seconds respectively.
- Same sixteen-byte B4 regression check against the old and revised compiled driver: old REJECTED, revised ACCEPTED.
- All 3,413 received notifications in the four available QZ logs reassemble without residual bytes. The original HCI contributes 1,049 treadmill notifications, including 1,036 B2 telemetry frames and thirteen ACKs.
- The November 3 TX90 hardware test completed the full twenty minutes: all eight upload requests were acknowledged, Start followed readiness, all twenty speed/incline slots matched, and the treadmill stopped at countdown zero before QZ Stop. The photo shows `00:09` remaining near the end; additional graph columns do not establish additional running time.
- The new thirty-second experiment and platform builds for this revision remain to be validated. Focused encoder tests do not simulate a GATT peripheral.
- Other total durations, rows not aligned to the selected slot length, and ramps remain unsupported. No timeline is silently changed.
- Live speed writes and a physical iConsole Stop command remain unimplemented; the capture contains neither. Use the treadmill's physical stop for hardware testing.
- QZ's general UI/workout scheduler is unchanged. Emitting `tapeStarted` after movement is the driver's confirmation; this is not a claim that every UI clock path waits for that signal, particularly when continuous-moving settings are enabled.

## Ten-minute controlled hardware test

Use `train-programs-examples/iconsole-tx90-10min-30sec.xml` with the updated build. It has the same twenty speed/incline slots as the verified twenty-minute example: 3/4/5/4/3 km/h and 0/0/1/1/0% repeated four times. Each slot lasts 30 seconds, total ten minutes.

A3/A4/A6/A7/A8/AF remain byte-identical between the two examples. A9 and AA contain ten `01 1f` duration pairs each (0 minutes + 1, 30 seconds + 1), replacing `02 01`; their checksums become `ad` and `ae` respectively.

1. Enable the QZ debug log, install the new build, reconnect, and load the shorter XML.
2. Press QZ Start as in the successful test; record the initial console countdown (expected approximately 9:59, but 19:59 would be informative too) and take a photo.
3. Verify transitions at 30 seconds (4 km/h, 0%), 60 seconds (5 km/h, 1%), and 90 seconds (4 km/h, 1%). Consecutive identical slots intentionally have no visible transition.
4. A two-minute run establishes the initial timing. If completing the test, check whether the treadmill stops by itself at ten minutes; report any mismatch between console countdown, QZ time and actual changes.
5. Use the physical treadmill Stop if ending early or if it continues beyond ten minutes. Do not rely on QZ Stop to stop the belt. Return the complete debug log and photos.

Do not change A4 in this experiment. If timing does not follow A9/AA, use that evidence to design a separate test changing A4 alone.
