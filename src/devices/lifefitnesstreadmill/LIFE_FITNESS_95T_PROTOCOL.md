# Life Fitness 95T/95Te protocol notes

## Scope

This document records the reverse-engineering and implementation findings for the
Life Fitness treadmill path currently handled by QZ's
`lifefitnesstreadmill` driver.

The evidence comes from:

- the Life Fitness Connect companion application and its protocol behaviour;
- sanitized BLE observations from one tested treadmill;
- the QZ implementation and focused parser/workout tests.

This is a protocol and implementation note, not a claim of support for every
Life Fitness model, console revision, or firmware.

## Confidentiality boundary

This document deliberately excludes:

- passwords, account email addresses, personal names, profile values, cookies,
  OAuth tokens, session identifiers, and device-installation identifiers;
- the vendor application-key value and any captured server-issued HMAC;
- raw captures, APK/XAPK files, decompiled application code, and private
  attachments.

`[REDACTED]` means that a value was intentionally omitted. UUIDs and standard
Bluetooth characteristic identifiers are retained where they describe the
public protocol shape; they are not credentials.

## Device discovery and transport

- The current QZ discovery branch accepts Bluetooth names beginning with `LF`
  when the observed name length is 15 or 18 characters.
- The device is routed to `lifefitnesstreadmill` before generic treadmill
  handling.
- The driver discovers all advertised services and attaches to the
  characteristics it recognizes by UUID.
- The treadmill exposes standard Fitness Machine data alongside Life Fitness
  custom characteristics. The implementation uses the FTMS treadmill-data
  characteristic (`0x2ACD`) when available and can also recognize the FTMS
  cross-trainer-data characteristic (`0x2ACE`).
- The standard FTMS training-status characteristic (`0x2AD3`) is subscribed for
  notification setup. The standard FTMS control-point characteristic (`0x2AD9`)
  is discovered when present, but QZ does not currently use it for treadmill
  control in this driver.

## Life Fitness custom characteristic map

| UUID | Role in the QZ path | Notes |
|---|---|---|
| `5da551de-9cb2-11e5-8994-feff819cdc9f` | Current State | Login state notifications, including profile request and login result. |
| `5da558dc-9cb2-11e5-8994-feff819cdc9f` | Login Command | `01` starts the profile exchange; `02` completes it. |
| `5da569e2-9cb2-11e5-8994-feff819cdc9f` | Workout summary/catalog | Carries the framed workout-library summary. |
| `5da56ab6-9cb2-11e5-8994-feff819cdc9f` | Workout detail | Carries the framed, Base64-wrapped workout XML. |
| `5da55aa8-9cb2-11e5-8994-feff819cdc9f` | Workout selection event | The console reports the selected library index here. |
| `5da552a6-9cb2-11e5-8994-feff819cdc9f` | Notification channel observed by the driver | Subscribed during initialization; the exact application role is not required by the current login path. |
| `5da54cf2-9cb2-11e5-8994-feff819cdc9f` | HMAC/profile authentication field | The account path receives the value from the service; the value itself is never documented here. |
| `ce78e85e-9cb6-11e5-8994-feff819cdc9f` | Metric-units/profile field | Used by the account path; deliberately omitted from the guest path. |

A legacy Life Fitness T5 workaround also recognizes
`c52d3161-d1a1-11e3-9c1a-0800200c9a66`. That path is kept separate from the
95T/95Te profile exchange.

For standard Bluetooth short UUIDs, the usual base UUID is:

```text
0000xxxx-0000-1000-8000-00805f9b34fb
```

## Account login path

The account path is enabled when the guest setting is disabled and account
credentials are available.

### Service exchange

The companion-app-derived service flow is:

```text
POST https://trainer.lifefitness.com/api/v3/signin
Content-Type: application/json
x-api-key: [REDACTED]
x-device-id: [per-installation UUID, not documented]
accept-language: [runtime language]
time-zone: [runtime IANA timezone]

{"username":"[REDACTED]","password":"[REDACTED]"}
```

QZ uses an in-memory cookie jar for the request sequence and validates the
result with:

```text
GET https://trainer.lifefitness.com/api/user
```

The service response contains the profile and authentication fields needed by
the BLE login. QZ keeps the sensitive values internal; debug messages report
only sanitized stages such as successful service login or failure status.

The application key is reconstructed internally from the inspected companion
application and must not be copied into documentation, logs, support emails,
or fixtures. The real server-issued HMAC and all OAuth/session values are also
omitted from this document.

### BLE account sequence

After service discovery and notification subscription, the driver:

1. writes `01` to Login Command;
2. waits for Current State `07` (profile requested);
3. writes the account profile fields in the captured order;
4. writes the service-returned HMAC unchanged to the HMAC characteristic;
5. writes the metric-units field for the account path;
6. sends either the configured nine-byte preset summary or the workout summary
   frames;
7. writes `02` to Login Command;
8. waits for Current State `04 01` (accepted). `04 00` is treated as rejection.

Profile values are derived from QZ settings or the service response. Account
credentials are not logged. If service login is unavailable, the existing
cached profile/HMAC path may be used; an account login failure is not silently
converted into guest mode.

## Explicit guest login path

Guest login is controlled by the persistent setting
`life_fitness_guest_login`, disabled by default.

In the QZ UI, the Life Fitness controls are grouped under:

```text
Settings > Treadmill Options > Life Fitness Options
```

Changing the Life Fitness username, password, or guest-login switch marks the
application for restart/reload. The account path and guest path remain explicit;
QZ does not automatically switch to guest mode after an account-login failure.

When enabled:

- no HTTP service login is attempted;
- no username or password is needed;
- the profile and synthetic authentication field exist only in memory;
- the account email field is not written;
- the metric-units characteristic is omitted;
- the guest language is `en`;
- the guest profile uses synthetic, non-personal values for name, age, gender,
  weight, and height;
- the synthetic HMAC is intentionally not reproduced in this document;
- cached account profile and HMAC values are ignored for that handshake.

The guest BLE sequence is otherwise the same notification-driven sequence as
the account path. Profile-field writes use acknowledged writes and are paced by
approximately 120 ms in the current implementation. A zero-length write to the
metric-units/profile characteristic produced a GATT error during investigation;
omitting that write succeeded, so the guest path omits it entirely.

The guest result was confirmed on one treadmill after a user-confirmed restart.
It does not establish compatibility with all 95T/95Te firmware versions or
other Life Fitness consoles. Bluetooth disconnection alone was not treated as
proof of logout.

## FTMS telemetry

The driver parses the little-endian FTMS treadmill-data payload with its flag
word and advances through only the fields indicated by the flags.

| Field | Encoding used by the driver |
|---|---|
| Instant speed | Unsigned 16-bit, `0.01 km/h` |
| Average speed | Unsigned 16-bit, `0.01 km/h` |
| Total distance | Unsigned 24-bit, metres |
| Inclination | Signed 16-bit, `0.1%`; `0x7FFF` means unavailable |
| Energy | Unsigned 16-bit; `0xFFFF` means unavailable |
| Heart rate | Unsigned 8-bit bpm |
| Elapsed time | Unsigned 16-bit seconds |

When direct treadmill distance is enabled, the reported distance is used. When
it is absent or disabled, QZ integrates the parsed speed over elapsed time.
Calories use the treadmill value when present and otherwise use QZ's existing
fallback calculation.

On Android, when the ANT+ heart-rate option is enabled, the ANT+ value takes
priority over the treadmill-reported heart rate. Otherwise the driver respects
the normal heart-rate-belt and ignore-built-in-heart-rate settings.

The focused sanitized fixture verifies speed, average speed, distance,
inclination, energy, and elapsed-time parsing, and rejects truncated payloads
and FTMS sentinel values.

## Workout-library upload

QZ can prepare one custom workout for the Life Fitness console and advertise it
during login. The console selects and starts the workout manually; QZ does not
remotely start the belt.

### Conversion and limits

- A program must contain at least one interval and no more than 100 intervals.
- Every interval must have a positive duration.
- Speed is validated from `0` through `22.5` in the QZ representation; `-1`
  means that the source interval has no speed target and is serialized as zero.
- Incline is validated from `0` through `15`; QZ's `-200` missing-incline marker
  is serialized as zero.
- The first positive speed must exist because it initializes the workout.
- The XML uses the Life Fitness workout schema version `2.0`, metric units, a
  workout-library program type, and comma-separated speed, incline, and time
  arrays.
- The XML contains only synthetic/default workout metadata. Personal account
  identifiers are not copied into this document.

### Transport framing

The summary JSON and detail XML are framed independently.

For each body, the transport frame is:

```text
byte 0   : reserved, zero
byte 1   : zero-based chunk index
byte 2   : reserved, zero
byte 3   : total chunk count
byte 4   : checksum = sum(body bytes in this chunk) modulo 256
byte 5.. : up to 15 body bytes
```

Limits and encoding:

- body chunks are 15 bytes;
- at most 255 chunks are accepted;
- detail XML is Base64 encoded before framing;
- Base64 is wrapped at 76 characters with a trailing newline;
- the summary is sent to the workout-summary characteristic;
- detail frames are sent to the workout-detail characteristic only after the
  console selects a known index.

The current implementation accepts selection event `0x15` and serves index `0`.
It rejects unknown indices and refuses detail transfer while the treadmill is
moving. Frames are dispatched sequentially with acknowledged writes, and a
second transfer is blocked while one is in progress.

## Control boundary

Telemetry and login are implemented; reliable treadmill control is not.

- `canStartStop()` returns false.
- `canHandleSpeedChange()` and `canHandleInclineChange()` return false.
- The FTMS control point may be discovered, but the driver does not send
  proven speed/incline/start commands through it.
- Current start/stop requests update QZ's local state/signals only; they are not
  evidence of physical belt control.
- No remote workout start is implemented.

Any future control change needs a same-device capture with the complete write,
response, timing, and owner-observed physical result. A transport write
acknowledgement or changing telemetry value alone is not enough.

## Evidence status and open validation

Confirmed by source/tests or the reported same-device investigation:

- `LF*` device discovery and Life Fitness driver routing;
- notification-driven account/guest profile exchange;
- guest login on one tested treadmill;
- FTMS telemetry parsing and sentinel handling;
- Android ANT+ heart-rate precedence;
- workout summary/detail framing and offline helper tests;
- manual console selection as the workout start point.

Not established across all devices or firmware:

- fresh server-returned HMAC acceptance on every supported console;
- guest workout transfer and guest telemetry on hardware as an independent test;
- universal compatibility across 95T/95Te revisions and other Life Fitness
  models;
- reliable speed, incline, start, stop, or remote workout control;
- a logout operation triggered solely by BLE disconnect;
- service token refresh, HMAC lifetime, or cross-account portability.

Keep future captures and logs sanitized. Do not commit raw BLE captures,
credentials, application keys, HMACs, OAuth/session values, cookies, or personal
profile data.

## Implementation references

- `lifefitnesstreadmill.cpp`: discovery, account/guest login, FTMS parsing,
  telemetry publication, and workout transfer state machine.
- `lifefitnesstreadmill.h`: public parser/profile/workout interfaces and driver
  state.
- `lifefitnessworkout.cpp/.h`: deterministic XML/JSON conversion and transport
  framing.
- `tst/Devices/TestLifeFitness95TParser.h`: sanitized FTMS and guest-profile
  fixtures.
- `tst/Devices/TestLifeFitnessWorkout.cpp`: workout XML, summary, and framing
  fixtures.

The document describes the PR implementation as pulled at the current branch
head; it does not replace hardware validation on the exact console and
firmware under test.
