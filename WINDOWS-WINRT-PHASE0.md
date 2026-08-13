# Phase 0 — Answering the questions

**Scope:** the Phase 0 gate in `WINDOWS-WINRT-BACKEND.md` Part V. Read code, build nothing.
**Sources:** the patched backends in `qt-patches/windows/5.15.2/`, upstream `qt/qtconnectivity`
`v5.15.2` and `6.8`, upstream `qt/qtbase` `v5.15.2`, and the DLLs committed under
`qt-patches/windows/5.15.2/binary/`.

**Verdict: the gate does not pass.** Three of the four Part II rows are refuted against real Qt
source. The one that survives is a means, not an end. Phase 0's own stopping condition —
*"If the Part II rows do not hold, stop here"* — is met.

---

## The four rows

| # | Part II claim | Result |
|---|---|---|
| 1 | Detail discovery is async | **Confirmed**, with a caveat |
| 2 | WinRT observes Service Changed and re-reads | **Refuted** |
| 3 | `requestConnectionUpdate()` is supported | **Refuted** |
| 4 | The association model gets closer to `autoConnect` | **Refuted** |

### Row 1 — async detail discovery: confirmed [verified]

The asymmetry is real and it is the one thing WinRT genuinely changes.

Win32 (`qlowenergycontroller_win.cpp:828`) does everything inline on the calling thread and reaches
`servicePrivate->setState(QLowEnergyService::ServiceDiscovered)` at `:975` before
`discoverServiceDetails()` returns.

WinRT-new does not:

- `discoverServices()` (`qlowenergycontroller_winrt_new.cpp:811`) calls `GetGattServicesAsync` and
  registers `onServiceDiscoveryFinished` via `put_Completed` on the XAML thread.
- `discoverServiceDetails()` (`:833`) hands the characteristic and descriptor walk to a
  `QWinRTLowEnergyServiceHandlerNew` worker on its own `QThread` (`:921–961`) and returns.
  `ServiceDiscovered` is set later, from the `charListObtained` handler at `:958`.

**Caveat — it is a hybrid, not an async path.** Before spawning that worker,
`discoverServiceDetails()` enumerates included services with a blocking
`QWinRTFunctions::await(op, result.GetAddressOf())` at `:873` — no timeout argument, so it defaults
to `0`, i.e. no timeout at all. And the worker itself awaits with `ProcessMainThreadEvents, 5000`
(`:167`) from a non-main thread. So the caller is not blocked for the expensive part, but the claim
"async" should not be read as "nothing blocks".

This does dissolve the `8f198c55` bug class at the source. It does not make the §1 two-pass split or
the discovery watchdog removable, both of which stay necessary for as long as any mingw build ships.

### Row 2 — Service Changed: refuted, twice over [verified]

**There is no Service Changed handling in any of the three backends.** Searching all of
`qlowenergycontroller_{win,winrt,winrt_new}.cpp` for `ServiceChanged`, `GattServicesChanged`, and
the `0x2A05` characteristic returns nothing. Nothing subscribes to it; nothing reacts to it.

The second half is worse, and it is the part that matters for `ATT_ATTRIBUTE_NOT_FOUND`. WinRT
exposes both a cached and an uncached form of every GATT accessor. The backend splits them like
this:

| Call | Cache mode | Sites |
|---|---|---|
| `GetGattServicesAsync` | **default → `Cached`** | `:819`, `:1590`, `:1715` |
| `GetCharacteristicsAsync` | **default → `Cached`** | `:165`, `:1632` |
| `GetDescriptorsAsync` | **default → `Cached`** | `:208` |
| `GetIncludedServicesAsync` | **default → `Cached`** | `:688`, `:867` |
| `ReadValueWithCacheModeAsync` | `Uncached` (explicit) | `:260`, `:363`, `:1017`, `:1187`, `:1671` |

Every *value* read is explicitly uncached. Every *enumeration* call uses the overload with no cache
mode, which defaults to `BluetoothCacheMode_Cached`. Enumeration is exactly where a stale GATT cache
produces a handle that no longer exists — the `ATT_ATTRIBUTE_NOT_FOUND` failure. WinRT serves that
from the same Windows cache Win32 does.

**Consequence for the plan:** Phase 2's headline test — *"Do the `ATT_ATTRIBUTE_NOT_FOUND` warnings
disappear? That is the Service Changed claim, tested"* — would be run against a claim that is
already false on inspection. Expect them to survive.

### Row 3 — `requestConnectionUpdate()`: refuted [verified]

```
qlowenergycontroller_winrt_new.cpp:978   requestConnectionUpdate(...)  { Q_UNIMPLEMENTED(); }
qlowenergycontroller_winrt.cpp:681       requestConnectionUpdate(...)  { Q_UNIMPLEMENTED(); }
qlowenergycontroller_win.cpp:988         requestConnectionUpdate(...)  { Q_UNIMPLEMENTED(); }
```

All three. The parameter is not even named. Widening the `#ifdef Q_OS_ANDROID` block in
`serviceScanDone()` (`cscbike.cpp:639`, `ftmsrower.cpp:764`, `npecablebike.cpp:574`, and the rest of
the same pattern) would compile and do nothing but print a warning.

### Row 4 — `autoConnect` equivalent: refuted [verified]

There is no `GattSession` and no `MaintainConnection` anywhere in the Qt 5.15 WinRT backends — those
are the WinRT primitives that would make an association model meaningful, and Qt only picked them up
in Qt 6. `connectToDevice()` (`:456`) resolves the device with `FromBluetoothAddressAsync`, awaits it
with a 5 s timeout, checks `get_ConnectionStatus`, then branches to `connectToPairedDevice()` or
`connectToUnpairedDevice()`. That is not closer to `autoConnect` than Win32 is. The manual reconnect
loop stays exactly as it is.

---

## Route B — can mingw enable `winrt_bt`?

**The configure condition is not the obstacle, and the plan's guess about it was wrong.** [verified]

`qtconnectivity/v5.15.2/src/bluetooth/configure.json`:

```json
"winrt_bt": {
    "condition": "config.win32 && !features.native-win32-bluetooth && tests.winrt_bt"
}
"native-win32-bluetooth": { "autoDetect": false }
```

`config.win32`, not `config.msvc`. And since `native-win32-bluetooth` is `autoDetect: false`, **WinRT
is the default on any win32 build where the `winrt_bt` compile test passes** — Win32 is the opt-in.
So the stock mingw package ships Win32 because the test *fails* under mingw, not because Qt gates the
feature on a toolchain or because anyone chose it.

**Why it fails is in the source, not the config.** [verified reading, [inferred] that it fails to
compile — not attempted here] The `winrt_bt` branch of `bluetooth.pro` pulls in a desktop shim:

```qmake
} else: qtConfig(winrt_bt) {
    !winrt {
        SOURCES += qbluetoothutils_win.cpp
        DEFINES += CLASSIC_APP_BUILD
        LIBS += runtimeobject.lib user32.lib
    }
```

`qbluetoothutils_win.cpp` provides a desktop stand-in for the XAML dispatcher by **redefining a
member of an exported class**:

```cpp
#define Q_OS_WINRT                       // lie to qfunctions_winrt.h
#include <QtCore/qfunctions_winrt.h>
#pragma warning (push)
#pragma warning (disable: 4273)          // MSVC: inconsistent dll linkage
HRESULT QEventDispatcherWinRT::runOnXamlThread(const std::function<HRESULT()> &delegate, bool)
{ return delegate(); }
#pragma warning (pop)
```

`QEventDispatcherWinRT` is declared `class Q_CORE_EXPORT` in
`qtbase/v5.15.2/src/corelib/kernel/qeventdispatcher_winrt_p.h:68`, which resolves to
`__declspec(dllimport)` when consuming a shared QtCore. Defining a member of a dllimport class is a
warning under MSVC — C4273, which this file explicitly suppresses — and a hard error under GCC and
therefore mingw. The `LIBS += runtimeobject.lib user32.lib` line in the same branch is MSVC link
syntax besides.

`QWinRTFunctions::await` is not a problem: it is an inline template in `qfunctions_winrt.h`, so it
needs no library. `qfunctions_winrt.cpp` and `qeventdispatcher_winrt.cpp` are only compiled under the
real `winrt` platform (`qtbase/.../kernel/kernel.pri:87–105`), which is why the shim exists at all.

**Answer: Route B is dead as it stands**, not by policy but by code. Making it live means a third
patch to qtconnectivity — one that is more invasive than the two QZ already carries, since it means
supplying `runOnXamlThread` some other way. Not worth it to avoid a toolchain switch that already
has a working CI job.

---

## Qt 6 — did it drop the Win32 backend?

**Yes.** [verified] `qtconnectivity/6.8/src/bluetooth/CMakeLists.txt` contains no
`qlowenergycontroller_win.cpp`, no Win32 branch, and no `native-win32-bluetooth` feature anywhere.
Windows resolves to `elseif(QT_FEATURE_winrt_bt)` or falls through to the dummy backend. The Qt 6
condition (`configure.cmake:56`) is `WIN32 AND TEST_winrt_bt` — also toolchain-agnostic, and with the
Win32 escape hatch removed entirely.

`winrt_btle_no_pairing` is gone too: Qt 6 has a single `qlowenergycontroller_winrt.cpp`, the
descendant of 5.15's `_new`.

So the strategic argument in Part II holds: Win32 is a dead end and this is a question of when. But
it points at **Route C**, not Route A — under Qt 6 WinRT is not a thing you choose, it is the only
thing there is.

---

## Two open questions closed on the way

**Which WinRT backend is actually in use — `_winrt.cpp` or `_winrt_new.cpp`?** `_new`. [verified]
Both compile into the shipped DLL: `msvc2019/Qt5Bluetoothd.dll` carries 257 strings matching
`QLowEnergyControllerPrivateWinRT`, 135 of them `WinRTNew`. Selection is at runtime, not build time —
`supportsNewLEApi()` (`qbluetoothutils_winrt.cpp:57`) returns true when
`Windows.Foundation.UniversalApiContract` major ≥ 4 is present, which is every supported Windows 10
and 11. So the file QZ patched at `:1670` is the live one, and the read of it above is the read that
matters.

**Where did the patched DLLs come from?** Partly recoverable. [verified]
`binary/msvc2019/Qt5Bluetooth.prl` records:

```
QMAKE_PRL_BUILD_DIR = C:/qt-everywhere-src-5.15.2/qtconnectivity/src/bluetooth
QMAKE_PRL_CONFIG = ... windows ... shared release ... win32 msvc ... c++1z ...
QMAKE_PRL_VERSION = 5.15.2
```

Built from a full `qt-everywhere-src-5.15.2` tree on Windows with MSVC, shared, release. Not a
reproducible procedure, but it names the source tree and the toolchain, which is more than the plan
assumed was available. The mingw DLL ships no `.prl` and remains undocumented.

**One correction to the plan.** Part I attributes the WinRT patch to `_winrt_new.cpp:1670` only. The
same `/* QZ rviola` edit is in **both** WinRT backends — `_winrt.cpp:363` and `_winrt_new.cpp:1670`.
Only the live one (`_new`) matters in practice, but both would need carrying forward.

---

## Recommendation

**Stop at Phase 0, as the plan provides for.** Do not spend Phase 1.

The case for this work was four benefits. One survives, and the two QZ-side defences already built
against that failure mode — the §1 two-pass split and the discovery watchdog — have to stay anyway
while mingw ships. Against that, the Part III cost is unchanged: vcpkg and protobuf, a different C
runtime, a broken exe-only upgrade path, and revalidation of the whole device zoo against new backend
timing.

The one claim that got *stronger* is the strategic one. Qt 6 has no Win32 backend at all, so the
migration is eventually forced. That is an argument for planning Route C on its own schedule — where
WinRT comes along for free and the Qt upgrade is the thing being tested — not for paying the Route A
toolchain switch now to buy one row out of four.

Worth keeping regardless of what happens next:

- `window-msvc2019-build` should stay disabled. Re-enabling it was Phase 1's first step; there is no
  Phase 1.
- The `ATT_ATTRIBUTE_NOT_FOUND` warnings are a *cached enumeration* problem, not a Service Changed
  problem. Whoever picks this up next should start from `BluetoothCacheMode` and the fact that Qt
  never asks for the uncached enumeration overload — that is a smaller and better-aimed question
  than a backend migration, and it applies to Qt 6 too.
- The Win32 descriptor patch has **no** WinRT equivalent need, but not for the hoped reason: WinRT
  simply has no descriptor-read error path in discovery to patch out, because the read happens on the
  worker thread and failures there surface as `handleServiceHandlerError`. The stale-cache tolerance
  it buys is still needed, because caches are not self-correcting on either backend.

## Definition of done — Phase 0 items

- [x] Part II's four rows confirmed or refuted against real Qt source — 1 confirmed, 3 refuted
- [x] Route B answered: can mingw enable `winrt_bt`? — not without a third qtconnectivity patch
- [x] Qt 6 Win32 backend removal confirmed
- [x] Which of `_winrt.cpp` / `_winrt_new.cpp` is in use — `_new`, selected at runtime
