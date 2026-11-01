# JK Fitness 10800 investigation — 2026-11-01

PR #5493, investigation of original head `d611a455decc792d9b53c4c46fe56e4258b7304a`. This document records the pre-fix evidence; see the driver README and regression tests for the subsequent correction.

## Result

The Classic transport works in the supplied logs. The driver never reaches protocol readiness and consequently never sends the requested controls. The RFCOMM startup/heartbeat in this PR differs materially from the original Run on Earth app. Missing native startup/heartbeat is the leading explanation for the missing replies, but requires a new hardware run to prove causality.

Do not fix this by treating every reply, or `55 0b 01 01` alone, as successful initialization and enabling movement.

## Reproducible evidence

Downloaded Run on Earth 1.6.0, package `com.pafers.runonearth3`, from APKPure:

https://apkpure.net/run-on-earth/com.pafers.runonearth3/download

Download endpoint: https://d.apkpure.net/b/APK/com.pafers.runonearth3?version=latest

SHA-256: `e92f39f8220642c87a4eae19e52b2e59ab11aec4ba7c122b1d4615a7edb38cf5`.
This matches the APK fingerprint recorded by the PR. Decompiled afresh with jadx 1.5.0, without running the app.

Local evidence directory: `/private/tmp/jk5493-evidence/`:

- `runonearth-original.apk`: original downloaded binary.
- `decompiled/sources/com/pafers/a/a/a/b.java`: RFCOMM transport.
- `decompiled/sources/com/pafers/a/a/b/a.java`: initialization, heartbeat, controls and RX handling.
- `QZ/debug-*.log`: only debug logs extracted from the supplied ZIP.
- `probe/`: small executable built against the unchanged PR codec to reproduce parser discrepancies.

Raw user logs and third-party APK/source are not included in git.

## What the user logs prove

Six logs were supplied; four contain JK connections:

| Log | TX frames | RX frames | Terminal result |
| --- | ---: | --- | --- |
| Thu Oct 29 21:23:55 | 10 | `55 0b 01 01` once | controls remain disabled |
| Sun Nov 1 11:33:03 | 10 | `55 0b 01 01` once | controls remain disabled |
| Sun Nov 1 11:35:10 | 10 | `55 0b 01 01` once | controls remain disabled |
| Sun Nov 1 11:45:19 | 10 | `55 0b 01 01` once | controls remain disabled |

The two Friday logs have no JK TX/RX exchange.

In the Nov 1 11:35 log:

- Line 1543: RFCOMM connected.
- Lines 1544–1569: the ten setup writes.
- Line 1552: `<< 55 0b 01 01`.
- Line 1574: probe ends without validated telemetry, controls disabled.

The delay between writes does not establish which request caused the single reply. The implementation logs parsed frames, not raw read chunks, so these counts cannot exclude discarded or incomplete incoming bytes.

`finishSetup()` requires `setupHadValidResponse`; `processFrame()` only sets it for selected console state or 13-byte telemetry. The observed `0x0b` packet satisfies neither condition. `update()` then returns immediately because `initDone` is false, disabling both control writes and its proposed keep-alive. This directly explains the reported lack of control response in QZ.

## APK versus PR

References below use jadx's `com/pafers/a/a/b/a.java`.

| Behavior | Original APK | PR |
| --- | --- | --- |
| Transport | Insecure RFCOMM, standard SPP UUID; transport `b.java` | Same transport family; confirmed working by log |
| Connection callback | Calls `u()` to schedule heartbeat, sleeps 700 ms, calls `r()` | Sends setup queue immediately |
| Heartbeat | `a(boolean)` sends `55 17 01 01`; runnable `p` starts immediately and repeats every 1000 ms | Never sends opcode `0x17`; sends an all-FF `0x0d` frame instead, only after readiness |
| Initialization | `r()` sends 27 query frames, then waits 2000 ms | Ten frames borrowed from QZ Pafers BLE, 300 ms advancement on timeout or any parsed reply |
| Protocol online | `55 bb 01 <type>` sets type and STANDBY, invokes connected callback; `55 bb 00` causes retry of type query | Neither reply is handled as protocol discovery |
| Telemetry request | `t()` sends `55 07 01 ff`; quick-start invokes it | Opcode `0x07` never sent |
| Observed `55 0b 01 01` | For type 2 (`h()`), transitions to STANDBY and calls stop callback with `true` | Ignored for state and readiness |
| Console events | `55090101` running, `55090102` stopped, `55090100` paused | Handles 01 and 00 as a boolean; drops 02 |
| Telemetry | `550d`: 10-byte payload for type 2, 13-byte payload for type 0/1; also `551d` branch | Accepts only 13 total bytes; does not check opcode/declared length in decoder |
| Speed 1.00 | Decodes as 1.00 | Converts it to zero without support in this APK |

Native `r()` query opcodes, in order, each with payload `01 ff`:

```text
0c bb 24 25 26 27 02 03 04 06 1f a0 b0 b2 b3 b4
b5 b6 b7 b8 b9 ba 0b 18 19 1a 1b
```

Native `55 bb 01` type values: 0/1 use extended telemetry; 2 uses shorter telemetry. The app route handler (`com/sixnology/runonearth/e/a.java`, `w()`) applies treadmill slope to type 2 and resistance levels 1–24 to types 0/1; the treadmill driver should not enable controls for types 0/1. The APK defaults to type 2 before receiving the query response. The single observed `0x0b` response does not independently prove the hardware type.

The PR's reset/profile/speed/incline/start-enable writes belong to a later session-control phase in the original app. They are not the original connection query sequence. In particular, the original quick-start uses waits of 600 ms, `55 08 01 01`, state handling, then `55 07 01 ff`; copying only control frame bytes misses that sequence.

For short `0x0d` telemetry, the existing speed offsets 9/10 and incline offset 11 do match the APK. The problem is not those offsets. Extended types also need type-appropriate handling and unit discovery via `550c01`; the APK conditionally converts speed units.

## Executed verification

Compiled and executed a standalone Qt 5.15.2 probe linked directly to the unchanged PR `jkfitness10800protocol.cpp`. Results:

```text
keepAlive=550d0affffffffffffffffffff (APK=55170101)
stop02 accepted=0 (APK=stop)
speed1 accepted=1 speed=0 (APK=1)
extended16 accepted=0 (APK=supported)
wrongOpcode accepted=1 (APK=not telemetry)
```

This verifies concrete codec differences; it does not simulate firmware acceptance. No physical treadmill was available. This initial investigation did not build the full application or change driver source; subsequent implementation validation is documented separately.

## Correction scope and hardware check

1. Use the original RFCOMM startup: immediate independent one-second `55170101` heartbeat, 700 ms settle, native query sequence. Heartbeat must run while discovering protocol state.
2. Parse `55bb01` type and `550c01` units, retry type query for `55bb00`; keep discovery separate from movement controls. Log raw RX chunks and socket write failures/short writes.
3. Preserve meaningful response/state matching; do not advance setup merely because an unrelated event arrived. Handle stop/pause distinctly and interpret `0x0b` using the negotiated type.
4. Implement the native telemetry request and type-specific parsing; require opcode and length, preserve real 1.00 speed, and apply discovered units.
5. Keep movement/session initialization behind an explicit user start. Do not send a fixed unrelated user profile during connection.
6. Rebuild the Android test APK and obtain a fresh log. First establish heartbeat/query replies, type/units and telemetry; then validate explicit controls on the physical machine.

Confirmed: the supplied run is blocked in QZ initialization. Strong source-backed suspect: missing native heartbeat and incorrect startup sequence. Unproven: which individual change is sufficient for this firmware, and physical control acceptance.

## Implementation validation

Implemented the correction in the PR branch after the investigation, with a Luna agent and parent review. The driver now uses the native heartbeat/query burst, waits for type 2 and units discovery, decodes both telemetry opcodes, and performs the native type-2 session sequence on explicit start. Hardware validation remains pending.

Executed checks: 10/10 focused protocol tests passed; the changed driver and codec compiled with Qt 5.15.2 as a small static library; `git diff --check` passed. This compilation checks the changed translation units, not a full application or Android APK build.
