# Windows BLE hardening and CI turnaround

**Baseline:** `dircon-server-refactor` @ `e4866361`

> **Author's note:** every `file:line` below is as of `e4866361` and approximate — locate by
> symbol name, not by number. Workflow job identifiers are from observed run output and must be
> confirmed against the YAML before editing.

---

## Part I — Why "works great on Android, flaky on Windows"

Several real differences stack up here, and one of them may be the actual answer.

### The one to check first: the tablet is stealing the bike

The tablet grabbed the bike the instant it powered on, two hours after being left alone. Most FTMS
bike consoles accept exactly **one** BLE central connection. If that tablet's QZ is still running
and in range, it owns the bike, and the Windows box is contending for a slot that's already taken.

That also retro-explains the countdown. The earlier reasoning — "the 3-2-1 can't be QZ, because
every `START_RESUME` write failed to queue" — only covers the *Windows* QZ. The tablet's QZ sending
`REQUEST_CONTROL` + `START_RESUME` on its own reconnect would produce exactly that countdown, at
exactly that moment, with the Windows instance showing nothing.

**Kill QZ on the tablet (or take it out of range) before the next Windows test**, otherwise you're
debugging a two-central race.

### The structural differences, if that's not it

**Bonding is mandatory on Windows, optional on Android.** Qt's Android backend talks to
`BluetoothGatt` directly and will happily discover services and write CCCDs on an unbonded device.
The Windows backend goes through the OS device stack, which only exposes GATT for devices paired in
Settings. On Windows, pairing isn't an optimization — it's the precondition, and anything that
disturbs the bond takes the whole connection with it.

**Windows owns the GATT cache; Android re-discovers.** The `ATT_ATTRIBUTE_NOT_FOUND` warnings in
the log are Windows serving attribute handles from the bond record for characteristics the bike no
longer exposes. Android's stack invalidates on a Service Changed indication and re-reads. The Win32
backend (`QLowEnergyControllerPrivateWin32`, which is what the log shows QZ using — not the WinRT
one) doesn't handle Service Changed at all, so a stale cache stays stale until you delete and
re-pair the device.

**There's no `autoConnect` on Windows.** On Android, a bonded peripheral coming back into range
gets picked up by the stack — precisely the "tablet reconnected on its own" behaviour observed.
Windows has no equivalent: `connectToDevice()` is an active attempt, and QZ's reconnect loop
(`ftmsbike.cpp:2517`) does the retrying by hand. That works, but it's polling where Android gets an
event.

**Connection parameters are Android-only.** `requestConnectionUpdate()` in `serviceScanDone` is
inside `#ifdef Q_OS_ANDROID`. On Android QZ asks for a 24–40 interval with a 420 supervision
timeout; on Windows it takes whatever the OS picks, which is more conservative and drops
notifications more readily under load.

### And the one that actually bit

Service detail discovery resolves **synchronously** on Windows inside `serviceScanDone()`'s
creation loop, so `stateChanged` fires while `gattCommunicationChannelService` is still being
filled. On Android it's asynchronous and the list is complete by the time the slot runs. That
asymmetry is the entire reason the `servicesSubscribed` one-shot guard was harmless on Android and
catastrophic on Windows — the Android build never sees a partial list.

So this isn't bad luck. Qt's Windows BLE backend is the weakest of the three, and QZ's device code
was written and tested against the Android one.

---

## Part II — The work

### Why this exists

Two evenings were lost to a one-line logic error whose entire cost was in the feedback loop: 50
minutes of CI, a 62 MB download that failed once mid-transfer, and no way to tell from a running
binary which commit it was built from. The engineering fixes below matter, but **§3 is what makes
the rest cheap**, so it is sequenced first.

The underlying defect class: QZ's device layer was written and tested against Qt's Android
Bluetooth backend. The Win32 backend differs in ways the code does not account for — synchronous
service-detail discovery, mandatory OS bonding, an OS-owned GATT cache with no Service Changed
handling, and no `autoConnect` equivalent. `8f198c55` was a fix that was correct on Android and
catastrophic on Windows, and nothing in the pipeline could have caught it.

### Recommended execution order

| Order | Item | Rationale |
| --- | --- | --- |
| 1 | §3 Feedback loop | Force multiplier; every later verification is cheaper |
| 2 | §1 Two-pass discovery | Removes the root cause of the `8f198c55` class of bug |
| 3 | §4 Tests + linux job | Locks §1 in; catches the next one before CI |
| 4 | §2 Reconnect hygiene | Real but lower-severity; benefits from §4 being in place |

---

## §1 — Fix the discovery asymmetry at its root

### Problem

`ftmsbike::serviceScanDone()` in [src/devices/ftmsbike/ftmsbike.cpp](src/devices/ftmsbike/ftmsbike.cpp)
interleaves object creation with detail discovery in a single loop:

```cpp
for (const QBluetoothUuid &s : qAsConst(services_list)) {
    QLowEnergyService *service = m_control->createServiceObject(s);
    gattCommunicationChannelService.append(service);
    connect(service, &QLowEnergyService::stateChanged, this, &ftmsbike::stateChanged);
    service->discoverDetails();          // <-- resolves synchronously on Win32
}
```

On Win32, `discoverDetails()` drives `stateChanged` to completion before the loop reaches the next
UUID. The slot therefore runs with `gattCommunicationChannelService` holding only the services built
so far, and the "all services discovered" gate in `ftmsbike::stateChanged()` is trivially true of a
list of one. On Android the call is asynchronous and the list is always complete, which is why the
same code is sound there.

Confirmed in `debug-qua_ago_12_21_52_25_2026.log`: twelve `all services discovered!` lines, the list
growing by exactly one service each time.

### Change

Split into two passes. Collect into a local list so the change is independent of §2's decision about
clearing the member list:

```cpp
QList<QLowEnergyService *> fresh;

// Pass 1: create and register every service object. No discovery yet, so
// stateChanged cannot fire against a half-built list.
for (const QBluetoothUuid &s : qAsConst(services_list)) {
    if (JK_fitness_577 && s != ftmsService) continue;
    QLowEnergyService *service = m_control->createServiceObject(s);
    if (!service) { qWarning() << "createServiceObject returned null for" << s; continue; }
    gattCommunicationChannelService.append(service);
    fresh.append(service);
    connect(service, &QLowEnergyService::stateChanged, this, &ftmsbike::stateChanged);
    if (s == QBluetoothUuid(QStringLiteral("b4cc1223-bc02-4cae-adb9-1217ad2860d1")) && !SS2K) {
        WATTBIKE = true;
        setGears(6);
    }
}

// Pass 2: now the list is complete, so the gate in stateChanged() means what
// its name says on every platform.
for (QLowEnergyService *service : qAsConst(fresh))
    service->discoverDetails();
```

`subscribedServices.clear()` must stay above pass 1, where it is now.

### Risk this introduces, and the mitigation

Today's partial-list behaviour accidentally guarantees forward progress: a pass runs early no matter
what. After the split, the gate at the top of `stateChanged()` only opens when every service reaches
`ServiceDiscovered` or `InvalidService`. A service stuck in `DiscoveryRequired` — a discovery request
dropped by the OS stack, plausible given the `ATT_ATTRIBUTE_NOT_FOUND` cache staleness already
observed — blocks the subscription of every other service indefinitely.

**Add a watchdog.** In `serviceScanDone()`, after pass 2, start a single-shot timer (10 s is
generous; Win32 discovery of six services completed in ~1 s in the logs). On timeout, log the
offending services and their states, then invoke the subscription pass anyway. The per-service
`QSet` makes forcing the pass safe — anything already subscribed is skipped, so a forced pass is
idempotent.

```cpp
// A service whose discovery never resolves must not hold the rest hostage.
serviceDiscoveryWatchdog.setSingleShot(true);
serviceDiscoveryWatchdog.start(10000);
```

…with the timeout slot logging each service's `serviceUuid()` and `state()` before calling the
subscription pass. Log loudly — a firing watchdog is a bug report, not routine operation.

### Deliberately out of scope

Decoupling `init()` (≈`ftmsbike.cpp:1992`) and the virtual-device creation from the all-discovered
gate is a genuine further simplification — `init()` only needs `gattFTMSService` and a valid
control-point characteristic, both assigned during `0x1826`'s own subscription. It is **not** part of
this change: it alters the ordering relative to virtual-device setup and DIRCON binding, which is the
machinery that took the whole DIRCON refactor to stabilise. Revisit only with §4's tests in place.

### Verification

New log, first launch:

- `{00001826-0000-1000-8000-00805f9b34fb}` present with its characteristics enumerated
- exactly **one** `all services discovered!`
- `FTMS service and Control Point found`
- indication subscribed on `2ad9`, `descriptorWritten "\x02\x00"`
- control granted and start acknowledged, and `degraded` absent
- no `already subscribed, skipping it` on the first connection (they should appear only after a
  reconnect)

Then quit and relaunch **without touching the bike**, and confirm the same set. That second run is
the original battery-only regression test.

---

## §2 — Reconnect hygiene

Three independent defects in the same area. They can land as separate commits.

### 2a. Backoff on reconnect

`ftmsbike::controllerStateChanged()` (≈`:2515`) calls `connectToDevice()` immediately on every
`UnconnectedState`. With the bike off, or owned by another central, this is a tight retry loop that
floods the log and the radio.

Replace the direct call with a single-shot timer: 1 s, doubling to a 30 s cap, counter reset in the
controller's connected lambda (≈`:2468`). Keep `initDone = false`, `initHandshake.reset()` and
`gearInclinationSent = false` where they are — they must fire on disconnect, not on the retry.

### 2b. Service object ownership

`gattCommunicationChannelService` is appended to in `serviceScanDone()` and never cleared.
`QLowEnergyController::createServiceObject()` returns an object the caller owns when no parent is
passed, so every reconnect leaks a full set of service objects, and the list grows without bound
across a long session with a flaky bike.

Today this is survivable — the stale objects go `InvalidService`, the gate tolerates that, and the
subscription loop skips them. It is still wrong, and it is directly under §1's gate.

In `serviceScanDone()`, before pass 1:

```cpp
for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService))
    if (s) s->deleteLater();
gattCommunicationChannelService.clear();
subscribedServices.clear();
```

`deleteLater()`, not `delete` — this runs from a signal handler and the objects may be on the stack.
Clearing both together is what keeps a recycled `QLowEnergyService*` from reading as
already-subscribed.

> **Audit before landing this.** Grep every use of `gattCommunicationChannelService` in
> `ftmsbike.cpp`; anything holding a raw pointer into it across a reconnect (in particular
> `gattFTMSService`, `zwiftPlayService`, `gattMokFitnessService`) becomes dangling. Those members
> must be nulled in the same place. **This is the highest-risk change in the document** — it trades a
> bounded leak for a potential use-after-free, so it needs the audit, not just the edit.

### 2c. Surface the two-central case

A second QZ instance on the tablet held the bike during the last debugging round and produced a
phantom 3-2-1 countdown that was attributed to the Windows build. Most FTMS consoles accept one
central; nothing in QZ says so.

Reliable detection is not available — the OS does not report why a connection attempt failed, and
"already connected elsewhere" is not distinguishable from "out of range" at the Qt API level. Do the
pragmatic thing: count consecutive failed attempts in 2a's backoff and, at five, raise a toast via
`homeform::singleton()->setToastRequested(...)` reading roughly *"Cannot connect to the bike. Another
device may be connected to it — check other QZ instances."* Fire **once per session**, not per
attempt.

Undiagnosable-but-common beats silent.

---

## §3 — Collapse the feedback loop

**Do this first.** It pays for itself within one iteration.

### 3a. Build identity in the log header

Every debugging round so far has included some version of "am I running the new binary?", answered by
grepping ASCII out of the `.exe`. Fix permanently.

In [src/qdomyos-zwift.pri](src/qdomyos-zwift.pri):

```qmake
QZ_GIT_SHA = $$system(git rev-parse --short HEAD)
isEmpty(QZ_GIT_SHA): QZ_GIT_SHA = unknown
DEFINES += QZ_GIT_SHA=\\\"$$QZ_GIT_SHA\\\"
```

In `main.cpp`, immediately after the log file handler is installed (before that, the line goes
nowhere):

```cpp
qDebug() << "QZ build" << QZ_GIT_SHA << "Qt" << qVersion() << "on" << QSysInfo::prettyProductName();
```

The Qt runtime version earns its place: §3b ships an exe without DLLs, and a mismatch needs to be
diagnosable from the log alone.

> Note the quoting is qmake-escaped and platform-fragile — verify on the Windows runner, not only
> locally. Detached-HEAD checkouts in Actions still resolve `rev-parse` correctly.

### 3b. Exe-only artifact

The Qt DLL set changes only when `qdomyos-zwift.pri`'s `QT +=` line changes. Everything else is a
drop-in replacement into an existing `C:\QZ\...` install.

Add a second `actions/upload-artifact` step to the Windows job:

```yaml
name: windows-exe-only
path: just output/qdomyos-zwift.exe
compression-level: 9
retention-days: 7
```

~20 MB stripped, well under 10 MB compressed. Download drops from ~50 minutes to seconds.

> **Caveat to write into the workflow as a comment:** any commit touching the `QT +=` line, the Qt
> version, or the toolchain requires the full `windows-binary-no-python` artifact. The Qt version
> line from 3a is the check — if the log's Qt version does not match the install's DLLs, take the
> full artifact.

### 3c. Pipeline trim

Target job set on the fork:

| Job | State | Notes |
| --- | --- | --- |
| `linux-x86-build` → tests | **ON** | Currently disabled by `753ff0e5`. See §4 |
| `window-build` (no-python) | **ON** | Plus the new exe-only artifact |
| Android build | **ON** | Daily driver; keep |
| `window-msvc2022-pr-build` | OFF | Fails at *Checkout MSVC2022 branch*; that branch does not exist on this fork. Its failure has already caused one false "CI failed" report |
| macOS / iOS / any remaining | OFF | Confirm `753ff0e5` covered these |
| `schedule:` trigger | OFF | Upstream inheritance; burns fork minutes on nothing |

Two further changes:

1. **Deduplicate push/PR runs.** With a PR open on `dircon-server-refactor`, a single push produced
   two runs (`31649196766` dispatch + `31649199723` PR). Restrict `on: push` to `master`, or add
   `if: github.event_name != 'push' || github.ref == 'refs/heads/master'`. This halves runner usage
   immediately.
2. **Do not gate the Windows build on tests.** Run them in parallel. The tests finish in minutes and
   surface a failure early; gating would add their duration to the 50-minute critical path for no
   benefit.

Leave `concurrency: cancel-in-progress` as it is — but note it has already cancelled two runs
mid-session (`31648622476`, `31648626748`) and confused the watcher. Any tooling that polls a run ID
must treat `cancelled` as "superseded, find the newer run", not as failure.

---

## §4 — Make the logic testable

### The pattern already exists in this repo

`67fdf241` extracted the FTMS control-point handshake into
[src/devices/ftmsbike/ftmscontrolpointhandshake.h](src/devices/ftmsbike/ftmscontrolpointhandshake.h):
header-only, no Qt objects, no clock (the caller supplies `now`), fully reachable from tests, 14 of
them in `tst/Devices/TestFtmsControlPointHandshake.{h,cpp}`. That design is why the handshake has
been reliable while the code driving it has not.

The subscription bookkeeping is the same shape and deserves the same treatment.

### What to extract

Not the `QSet` — that is trivial. The thing worth testing is *the decision*: given a list of services
with states and a record of what has been done, which need subscribing now, and is the gate open?

`src/devices/ftmsbike/servicesubscriptionplan.h`, header-only, no Qt types:

```cpp
enum class ServiceDiscoveryState { Required, Discovering, Discovered, Invalid };

struct ServiceView {
    quint64 id;                  // opaque handle; the caller maps QLowEnergyService* to it
    ServiceDiscoveryState state;
};

class ServiceSubscriptionPlan {
  public:
    bool allResolved(const std::vector<ServiceView> &services) const;   // gate
    std::vector<quint64> pending(const std::vector<ServiceView> &) const;
    void markSubscribed(quint64 id);
    void reset();                // called where subscribedServices.clear() is now
    int subscribedCount() const;
};
```

`ftmsbike` keeps the `QSet<QLowEnergyService *>` as the pointer→id mapping and delegates the
decision. The class never sees a Qt type, so it builds and runs anywhere.

### Tests that must exist

The first one is the bug that cost the evening — **write it first and watch it fail** against
`8f198c55`'s logic:

1. **Partial list growth.** Present one service, discovered; then two; then six. Every service must
   end up subscribed exactly once. *(This is the `8f198c55` regression.)*
2. **Repeat firings.** The same complete list presented five times yields subscriptions on the first
   only. *(This is the original battery-only bug — four CCCD writes per session.)*
3. **Gate correctness.** `allResolved` is false while any service is `Required` or `Discovering`;
   true when all are `Discovered` or `Invalid`; true for a mix of the two.
4. **Invalid services.** Counted as resolved by the gate, never subscribed.
5. **Reset.** After `reset()`, everything is pending again.
6. **Id reuse after reset.** The same id presented after a reset is treated as new — the
   recycled-pointer case §2b makes real.
7. **Empty list.** `allResolved` on an empty list must not open the gate onto a subscription pass
   over nothing.

Register in `tst/qdomyos-zwift-tests.pro` alongside the handshake suite.

### Re-enable the linux job

`linux-x86-build` is the only job that runs the test suite at all, including the ERG and slew suites,
and `753ff0e5` disabled it when the fork was cut down to Android + Windows. Every test written since
has been run by hand in the build VM. Re-enable it as a **tests-only** job — it does not need to
produce a shippable Linux binary:

- checkout with submodules (googletest is vendored — see `e1fc3219`, which made
  `build-and-run-tests.sh` build from it rather than a system copy)
- install Qt via the pinned `aqtinstall 3.3.0` (`a29d74f5`; do not float this)
- run `build-and-run-tests.sh`
- upload the test log on failure only

Expected suites green at baseline: handshake 14, slew 14, ERG table 13, plus the new subscription
plan.

---

## Definition of done

- [ ] Log header prints commit SHA and Qt version on every launch
- [ ] `windows-exe-only` artifact published and confirmed to drop into an existing install
- [ ] `window-msvc2022-pr-build`, scheduled runs, and duplicate push/PR runs gone
- [ ] `linux-x86-build` green and running the full suite on every push
- [ ] `serviceScanDone()` split into two passes, with a discovery watchdog
- [ ] Exactly one `all services discovered!` per connection in a real log
- [ ] Bike streams `2ad2` on a cold launch and on a relaunch with no bike interaction
- [ ] `ServiceSubscriptionPlan` extracted with all seven tests passing
- [ ] Reconnect backoff, service-object ownership (post-audit), multi-central toast

---

## Open questions for whoever picks this up

- **Win32 vs WinRT backend.** The log shows `QLowEnergyControllerPrivateWin32`. The WinRT backend
  handles Service Changed, supports connection-parameter requests, and does not discover
  synchronously — it would address several root causes at once. It is a Qt build-configuration
  change with its own risk surface, so it belongs in a separate experiment branch, not here.
- **Windows GATT cache staleness.** `ATT_ATTRIBUTE_NOT_FOUND` on `2a05`, `fff1`, `fff2`,
  `d18d2c10-…` are Windows serving handles from the bond record. None are characteristics QZ uses,
  so this is currently cosmetic — but it means the bond carries a stale database, and if
  `2ad2`/`2ad9` ever land in that set the only remedy is unpair and re-pair. Worth a note in
  `BUILDING-ON-WINDOWS.md`.
- **Stale documentation.** `tools/qz-rouvy-rtss/README.md` still asserts that QZ and Rouvy cannot
  work on a single Windows box. That was disproven; the file will mislead the next reader.
