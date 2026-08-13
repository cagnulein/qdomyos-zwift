# Moving the Windows BLE stack to WinRT

**Baseline:** `claude/windows-ble-hardening-review-b52ai6` @ `42290fc`

> **Author's note:** every claim below is marked **[verified]**, **[inferred]** or **[unverified]**.
> Verified means it was checked against this working tree or a CI run in this session. Inferred
> means it follows from verified facts but was not directly observed. Unverified means it comes from
> general knowledge of Qt and must be confirmed before anyone commits work to it. The distinction
> matters more than usual here, because the headline finding contradicts what the hardening document
> assumed.

---

## Part I — The premise is wrong, and that is good news

`WINDOWS-BLE-HARDENING.md` lists this under open questions as *"a Qt build-configuration change with
its own risk surface"*, implying WinRT is somewhere QZ has never been. That is not the case.

**This repository already ships a patched WinRT Qt Bluetooth build.** [verified]

`qt-patches/windows/5.15.2/` contains three patched backend sources and two sets of prebuilt DLLs:

| Path | Size | What it is |
|---|---|---|
| `qlowenergycontroller_win.cpp` | 51 KB | Win32 backend, QZ-patched |
| `qlowenergycontroller_winrt.cpp` | 55 KB | WinRT backend, QZ-patched |
| `qlowenergycontroller_winrt_new.cpp` | 87 KB | WinRT "new" backend, QZ-patched |
| `binary/mingw64/Qt5Bluetooth.dll` | 30 MB | debug |
| `binary/msvc2019/Qt5Bluetooth{,d}.dll` | 2.0 / 2.5 MB | release + debug, plus `ucrtbased.dll` |

Which backend is in which DLL is not documented anywhere, so it was measured directly by counting
backend-specific symbol strings in each binary: [verified]

| DLL | WinRT markers | Win32 markers | Backend |
|---|---|---|---|
| `mingw64/Qt5Bluetooth.dll` | 2 | **380** | **Win32** |
| `msvc2019/Qt5Bluetoothd.dll` | **259** | 0 | **WinRT** |
| `msvc2019/Qt5Bluetooth.dll` | 0 | 0 | stripped — inconclusive |

So the split is along the **toolchain**, not along anything QZ chose per se: the official Qt 5.15.2
`win64_mingw81` package carries the Win32 backend, and the official `win64_msvc2019_64` package
carries WinRT. [verified for these two DLLs; [inferred] that this reflects stock Qt packaging rather
than something QZ configured]

**The consequence: "switch to WinRT" is mostly "switch the Windows build from mingw to MSVC", and
that build already exists in CI.** `window-msvc2019-build` is wired end to end — Qt 5.15.2
`win64_msvc2019_64`, MSVC compiler install, vcpkg, the patched-DLL copy step, artifact upload.
[verified]

> **Note for whoever picks this up:** I disabled `window-msvc2019-build` in the §3c pipeline trim,
> because it carried no `if:` gate and was burning two Windows runners on every push for a build the
> fork does not ship. That decision was right for the fork as it stood, and it is the *first thing to
> reverse* if this work goes ahead. One line.

### The patches are load-bearing, and they are not the same patch

All three backends carry a `/* QZ rviola` edit, and both are of the form "do not let a failed read
abort the flow" — but they fix different things in different places. [verified]

**Win32** (`qlowenergycontroller_win.cpp:951`) comments out the error path after a descriptor read
during service discovery. Stock Qt, on a descriptor read failure, does:

```cpp
servicePrivate->setError(QLowEnergyService::DescriptorReadError);
servicePrivate->setState(QLowEnergyService::DiscoveryRequired);
return;
```

This is directly connected to the hardening work. The `ATT_ATTRIBUTE_NOT_FOUND` warnings in the
Windows logs are exactly this failure, and they are survivable **only because QZ patched them out**.
On a stock mingw Qt, a stale GATT cache entry would drop the service back to `DiscoveryRequired` and
leave it there — which is precisely the "service stuck in `DiscoveryRequired` blocks every other
service" scenario the §1 discovery watchdog was added to survive. The watchdog and this patch defend
the same failure from opposite ends.

**WinRT** (`qlowenergycontroller_winrt_new.cpp:1670`) comments out a blocking reachability probe
during connect — `ReadValueWithCacheModeAsync` followed by
`QWinRTFunctions::await(op, ..., 500)` — and declares the device connected without it.

These patches must be carried forward. Neither is upstream; both are edits to Qt's own source,
rebuilt into the shipped DLLs.

---

## Part II — What WinRT actually buys

Mapped against the four root causes in `WINDOWS-BLE-HARDENING.md` Part I:

> **Phase 0 ran. Three of these four rows are refuted.** The table below is left as written so the
> reasoning stays legible; the Confidence column now carries the result. Full evidence, with file and
> line references, is in **`WINDOWS-WINRT-PHASE0.md`**.

| Root cause | WinRT effect | Confidence |
|---|---|---|
| Synchronous service-detail discovery | Async — removes the asymmetry that caused the `8f198c55` bug class outright | **[verified]** — true, though the path is a hybrid: included-service enumeration still blocks |
| No Service Changed handling → stale GATT cache | WinRT stack observes Service Changed and re-reads | **[refuted]** — no Service Changed handling in any backend, and every *enumeration* call uses the `Cached` overload |
| No connection-parameter control | `requestConnectionUpdate()` is supported, so the Android-only block in `serviceScanDone()` could apply on Windows too | **[refuted]** — `Q_UNIMPLEMENTED()` in all three backends |
| No `autoConnect` equivalent | WinRT's association model gets closer, but the manual reconnect loop stays | **[refuted]** — no `GattSession` / `MaintainConnection` in Qt 5.15 at all |

The whole case for this work rested on these rows. One survives, and it is the one row that is a
means rather than an end — the §1 two-pass split and the discovery watchdog already defend that
failure mode from the QZ side, and both stay necessary while any mingw build ships.

The strategic argument is stronger than any individual row: **Qt 6 removed the Win32 Bluetooth
backend entirely and uses WinRT exclusively on Windows** — **[verified]**: `qtconnectivity/6.8`
has no `qlowenergycontroller_win.cpp` and no `native-win32-bluetooth` feature. The Win32 backend is
a dead end and this migration is a question of when, not whether. `window-msvc2022-build` already
targets Qt 6.8.2, so the fork has a foot in that door already. [verified] But this now points at
**Route C**, not Route A: under Qt 6, WinRT is not chosen, it is the only option.

---

## Part III — What it costs

The toolchain switch is the real payload, and it is not small. [verified from the workflow and
`src/qdomyos-zwift.pri` unless noted]

**Protobuf via vcpkg.** MSVC builds compile `zwift-api/zwift_messages.pb.cc`, which is inside an
`msvc { }` block (`qdomyos-zwift.pri:373`) and does not exist on the mingw path at all. That drags
in `-llibprotobuf -llibprotoc -labseil_dll -llibprotobuf-lite -ldbghelp` (`:34`) and a vcpkg install
of `protobuf`, `protobuf-c`, `abseil` pinned to baseline `8c2fcace`. The msvc2019 job has four cache
steps and a bootstrap just for this. **Zwift Play gear support is on the MSVC path but built
differently** — worth confirming it behaves identically, since it is not merely a recompile.

**A different C runtime.** The msvc2019 patch directory ships `ucrtbased.dll`, i.e. the *debug* UCRT,
because the CI artifact is a debug build out of `src/debug/output`. Users need the matching VC++
runtime where mingw needed only `libwinpthread-1.dll`, `libgcc_s_seh-1.dll`, `libstdc++-6.dll`.
The install layout changes, so **the first MSVC drop cannot be an exe-only swap into an existing
mingw install** — §3b's `windows-exe-only` artifact is invalid across this boundary and the full zip
must be used for the transition. The build header from §3a is what makes that detectable.

**Compiler flags already diverge.** `_ITERATOR_DEBUG_LEVEL=0` (`:33`), `/RTC1` on debug (`:50`), and
mingw's `-Wa,-mbig-obj` (`:49`) which MSVC does not need. The `optimize_full ltcg` config will behave
differently under MSVC.

**Revalidation of the device zoo.** This is the cost nobody budgets for. Every Windows workaround in
the device layer was written and tuned against Win32 backend timing, error codes and ordering. The
hardening document's own thesis — code written against one backend breaks on another — applies with
full force in this direction too. The §4 test suite does not help here: it covers the subscription
decision, not any Qt path.

---

## Part IV — Three routes

| | **A. MSVC 2019 + Qt 5.15.2** | **B. mingw + `winrt_bt`** | **C. MSVC 2022 + Qt 6.8.2** |
|---|---|---|---|
| WinRT backend | Yes [verified in the DLL] | Only if configurable | Yes, and the only option |
| CI job exists | **Yes**, needs `if: false` removed | No | Yes, schedule-gated, Qt 6 |
| Keeps mingw toolchain | No | **Yes** | No |
| Qt version change | No | No | **Yes — 5.15 → 6.8** |
| Patched DLLs in tree | **Yes, already** | Would need building | No — 5.15.2 patches do not apply |
| Effort | **Low** | Unknown, possibly zero, possibly impossible | High |
| Risk | Medium | Low if it works | High |

**Route B was the highest-value question to answer first**, and the cheapest. **Phase 0 answered it:
Route B is dead**, but not for the reason guessed here. The configure condition is
`config.win32 && !features.native-win32-bluetooth && tests.winrt_bt` — no `config.msvc`, and since
`native-win32-bluetooth` is `autoDetect: false`, WinRT is the *default* on any win32 build whose
compile test passes. What kills mingw is `qbluetoothutils_win.cpp`, the desktop shim the `winrt_bt`
branch pulls in: it redefines a member of the `Q_CORE_EXPORT`ed `QEventDispatcherWinRT` and silences
the resulting MSVC C4273, which GCC rejects outright. Enabling it for mingw means a third
qtconnectivity patch, more invasive than the two QZ already carries. See `WINDOWS-WINRT-PHASE0.md`.

**Route A is the recommendation** if B is dead: it reuses a CI job that already works, and the
patched WinRT DLL is already committed.

**Route C is not this change.** It bundles a Qt major-version upgrade with a backend change, so a
regression cannot be attributed to either. Its one merit is that it is where things end up anyway.

---

## Part V — Phased plan

Each phase ends somewhere it is safe to stop.

**Phase 0 — Answer the questions, build nothing. (~half a day) — DONE. Gate not passed.**
Read `qlowenergycontroller_winrt_new.cpp` and confirm the four Part II rows against actual code:
is detail discovery async, is Service Changed handled, is `requestConnectionUpdate` implemented?
Check the `winrt_bt` configure condition for Route B. Confirm whether Qt 6 dropped the Win32 backend.
**If the Part II rows do not hold, stop here** — the entire justification is those four rows, and
nothing has been spent.

> **Result: three of the four rows are refuted; stop here.** Full findings in
> **`WINDOWS-WINRT-PHASE0.md`**. Phases 1–3 below are not started, and on this evidence should not
> be — the recommendation is to fold WinRT into a Qt 6 move (Route C) rather than pay the Route A
> toolchain switch to buy one row out of four. `window-msvc2019-build` stays disabled.

**Phase 1 — Get a WinRT binary in hand.**
Re-enable `window-msvc2019-build` (revert the `if: false` from §3c) and add the `windows-exe-only`
equivalent for it. Confirm the patched WinRT DLL is what the artifact actually loads. The §3a build
header now prints the Qt version at launch, which is how you verify the right DLL is in play.
**Deliverable: a runnable WinRT build**, with the mingw build still the default. Nothing is switched.

**Phase 2 — Measure it against the same bike.**
Run the `WINDOWS-BLE-HARDENING.md` §1 verification list against the WinRT build: exactly one
`all services discovered!`, `2ad9` indication subscribed, control granted, `2ad2` streaming on cold
launch and on relaunch untouched. Then specifically:
- Do the `ATT_ATTRIBUTE_NOT_FOUND` warnings disappear? That is the Service Changed claim, tested.
- Does the §1 discovery watchdog ever fire? It should not.
- Does `requestConnectionUpdate()` succeed if the `#ifdef Q_OS_ANDROID` is widened?

**Phase 3 — Decide, on evidence.**
Only now is there a basis for switching the default. If WinRT wins, `window-build` becomes the MSVC
job, `BUILDING-ON-WINDOWS.md` is rewritten around the new install layout and CRT, and the exe-only
caveat is updated for the toolchain boundary. If it does not win, Phases 1–2 cost a few days and the
fork keeps a WinRT build available for the next time the question comes up.

---

## Risks

| Risk | Severity | Mitigation |
|---|---|---|
| The Part II benefits do not hold | **Fatal to the rationale** | Phase 0 confirms before anything is spent |
| Qt 5.15 WinRT backend has its own bugs — QZ already had to patch it | High | The patch is in-tree; budget for finding more, since it is less battle-tested here than Win32 |
| `runOnXamlThread` — WinRT needs a XAML-thread dispatcher; a classic source of hangs | High | Watch for deadlocks under load in Phase 2, not just happy-path connect |
| WinRT BLE APIs may need packaged app identity; the code has a "manifest capabilities" error path | Medium | Test an unpackaged run early in Phase 1 — `AppxManifest.xml` exists but the exe is normally run loose |
| Device zoo regressions from timing/ordering changes | Medium, wide | Phase 2 on the FTMS bike first; stage the rest |
| Users need a different C runtime; exe-only swap breaks | Medium | Full zip for the transition; §3a header makes a mismatch diagnosable |
| Zwift Play behaves differently — protobuf path differs on MSVC | Medium | Explicitly test gear control, not just the bike |
| vcpkg baseline drift breaks CI later | Low | Baseline is pinned; keep it pinned, as with aqtinstall |

---

## Definition of done

- [x] Part II's four rows confirmed or refuted against real Qt source — 1 confirmed, 3 refuted
- [x] Route B answered: can mingw enable `winrt_bt`? — not without a third qtconnectivity patch
- [ ] A WinRT artifact exists that a person can download and run
- [ ] The §1 verification list passes on it, on a cold launch and a relaunch
- [ ] `ATT_ATTRIBUTE_NOT_FOUND` gone, or explained
- [ ] The discovery watchdog does not fire
- [ ] Zwift Play gear control verified on the MSVC build
- [ ] A written recommendation with logs from both backends side by side

---

## Open questions

- **Where did the patched DLLs come from?** ~~No recorded provenance.~~ **Partly answered by Phase
  0:** `binary/msvc2019/Qt5Bluetooth.prl` records `QMAKE_PRL_BUILD_DIR =
  C:/qt-everywhere-src-5.15.2/qtconnectivity/src/bluetooth` and a `win32 msvc ... shared release`
  config — so the MSVC pair was built from a full `qt-everywhere-src-5.15.2` tree. Still not a
  reproducible procedure, and the mingw DLL ships no `.prl` and remains undocumented. Anyone changing
  Qt versions inherits this, and it is arguably worth fixing before a migration rather than during.
- **Is `qlowenergycontroller_winrt.cpp` or `..._winrt_new.cpp` the one in use?** **Answered:
  `_new`.** Both compile into the DLL (257 `...PrivateWinRT` strings, 135 of them `WinRTNew`);
  selection is at *runtime*, not build time — `supportsNewLEApi()` picks `_new` whenever
  `UniversalApiContract` ≥ 4 is present, i.e. on every supported Windows 10/11. Note also that the
  `/* QZ rviola` patch is in **both** WinRT backends (`_winrt.cpp:363` and `_winrt_new.cpp:1670`),
  not only `_new` as Part I states.
- **Does the §1 two-pass split stay necessary under WinRT?** If discovery is genuinely async there,
  the split is a no-op on that path — but it stays correct, and it stays necessary for as long as any
  mingw build ships. Do not revert it as part of this work.
- **Does the Win32 descriptor patch have a WinRT equivalent need?** **Answered, but not the hoped
  answer.** There is no equivalent error path in WinRT's discovery to patch out — the descriptor read
  happens on the worker thread and failures surface as `handleServiceHandlerError` — but the
  stale-cache *tolerance* it buys is still needed, because caches are not self-correcting on either
  backend. Service Changed is unhandled and enumeration is served from the Windows cache on both.
