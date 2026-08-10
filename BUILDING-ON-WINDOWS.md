# Getting a Windows build of this fork

Windows binaries for this fork come from **GitHub Actions on the fork**, not from a
local compiler. This note says why, how to get one, and what is already installed
should the local route ever become available.

## Why not build locally

Smart App Control is enforced on the development machine:

```
HKLM\SYSTEM\CurrentControlSet\Control\CI\Policy
VerifiedAndReputablePolicyState = 1        # 1 = enforced
```

It blocks binaries per-file by signature and reputation, and it blocks
`C:\msys64\mingw64\bin\g++.exe`:

```
Uma política de Controle de Aplicativo bloqueou este arquivo
```

so `qmake` stops with `Project ERROR: Cannot run compiler 'g++'` and nothing links.
The decision is not consistent across a single package - `gcc.exe` from the same
MSYS2 install runs fine - so there is no subset of the toolchain to retreat to. It
also blocks unsigned native Python wheels; `py7zr` fails to import its `pybcj`
extension, which is why `aqt` has to be pointed at a real 7-Zip with
`-E "C:\Program Files\7-Zip\7z.exe"`.

**Do not turn Smart App Control off to work around this.** Disabling it is
permanent: re-enabling requires reinstalling Windows. That is the machine owner's
call, and the CI route below costs nothing by comparison.

Note that enforcement is not a blanket ban on unsigned binaries - the released QZ
nightly `qdomyos-zwift.exe` runs on this machine untouched. A freshly built binary
carries no reputation, so whether it runs is worth confirming early rather than
after a long debugging session.

## Getting a build

1. Actions must be enabled once per fork - GitHub disables them on forks by
   default: <https://github.com/nickolas122/qz/actions>
2. Actions → **CI** → *Run workflow* → pick the branch.
3. When `window-build (false)` goes green, take the **`windows-binary-no-python`**
   artifact from the run summary, unzip, run `output\qdomyos-zwift.exe`.

`workflow_dispatch` runs the workflow as it exists **on the selected branch**, so a
change to `.github/workflows/main.yml` only affects dispatches started after it is
pushed. A run already in flight keeps the workflow and commit it started with.

The `Secrets` step is `if: github.ref == 'refs/heads/master'`, so on any other
branch `src/secret.h` is never written. That is harmless - every use of it is
guarded by `#if __has_include("secret.h")`; only the Strava, Peloton and SMTP keys
are lost.

### Which artifact

| Artifact | Toolchain | Built from |
|---|---|---|
| `windows-binary-no-python` | mingw, **Qt 5.15.2** | the branch you dispatch |
| `windows-msvc2019-binary-no-python` | MSVC 2019, Qt 5.15.2 | the branch you dispatch |
| `windows-msvc2022-binary-no-python` | MSVC 2022, **Qt 6.8.2** | upstream nightly only, and from `refs/pull/1508/head` - a PR branch, not master |

Prefer **mingw** when comparing against upstream. The upstream nightly publishes a
`windows-binary-no-python.zip` built from master with that same toolchain, so
testing a fork build against it changes one variable - the branch - instead of two.
The msvc2022 artifact is a poor baseline for two reasons at once: a different Qt
major version, and a source tree that is neither this fork nor upstream master.

The `-no-python` variants omit the bundled Python and PaddleOCR payload, which only
serves the Zwift-workout OCR and NordicTrack ADB paths. They are several hundred MB
smaller and fine for everything else.

All Windows nightlies are **debug** builds - CI archives `src/debug/output` - so the
Qt DLLs carry the `d` suffix and the binaries are large. Do not read performance
into them.

## Which jobs run

A dispatch used to fan out to 21 jobs. Everything except Android and Windows is now
gated off with a literal `if: false` carrying the marker comment
`# fork: disabled - only android-build and window-* build here`:

```sh
grep "fork: disabled" .github/workflows/main.yml
```

Left running: `window-build`, `window-msvc2019-build`,
`window-msvc2019-aiserver-build`, `android-build`.

Disabled here: `linux-x86-build`, `ios-build`, the three `raspberry-pi-*` jobs,
`android-emulator-test`, and the vendor APK flavours `nordictrack-build` and
`fitpro-build`. Re-enabling one is a single word - change its `if: false` back.

`window-msvc2022-build`, `window-msvc2022-pr-build`, `peloton-bike-build`,
`peloton-bike-plus-build` and `upload_to_release` are left untouched: their own
`schedule` / `pull_request` gates already make them unreachable from a manual
dispatch.

Gating rather than deleting keeps the diff against upstream to one line per job, so
a merge conflicts on a line instead of on a missing block.

## The local toolchain, if it is ever usable

Everything but the compiler policy is already in place, so if Smart App Control is
ever off this needs no further setup:

- **MSYS2** with `mingw-w64-x86_64-toolchain` and `mingw-w64-x86_64-qt5-webview`.
  The webview package matters: Qt's official mingw 5.15.2 build ships no WebView.
- **Qt 5.15.2** `win64_mingw81` plus `qtnetworkauth` and `qtcharts`, in `C:\Qt`,
  installed with `aqt` (`pip install aqtinstall`).
- **CMake** and **7-Zip**.
- Submodules - these are real submodules, so no manual cloning:
  ```sh
  git submodule update --init --recursive src/smtpclient tst/googletest src/qthttpserver
  ```
  `android_openssl` and `zwiftplay` are not needed for Windows; the build files do
  not reference `zwiftplay`, and upstream CI does not check either of them out for
  the Windows jobs.

Then, following the CI job:

```sh
cp qHttpServerBin/5.15.2/headers/* src/qthttpserver/src/3rdparty/http-parser/
cd src/qthttpserver && qmake && mingw32-make -j8 && mingw32-make install && cd ../..
lrelease src/qdomyos-zwift.pri
qmake && mingw32-make -j8
cd src/debug && mkdir output && cp qdomyos-zwift.* output/ && cd output
windeployqt --qmldir ../../ qdomyos-zwift.exe
# then libwinpthread-1.dll, libgcc_s_seh-1.dll, libstdc++-6.dll from C:/msys64/mingw64/bin
# and windows_openssl/*
```

## Testing the binary

The bike is Bluetooth LE, and **BLE does not work in a VM** - VirtualBox has no
virtual Bluetooth adapter, and the only route is USB passthrough of a physical
dongle, which needs the Extension Pack (not installed) and is unreliable for BLE
anyway. Whatever builds the binary, it has to be run on the machine holding the
Bluetooth adapter.

Two Windows-specific behaviours to keep in mind when a bike connects but no data
arrives:

- `bluetooth::finished` is not connected on Windows
  (`src/devices/bluetooth.cpp`, `#ifndef Q_OS_WIN`), and the rescan cycle lives in
  that handler. Discovery is effectively one-shot at launch, so the bike must be
  advertising before QZ starts. This also means the reconnection work on
  `kind-of-stable` has no effect on Windows.
- `ftmsbike::stateChanged()` refuses to subscribe to any characteristic until
  *every* discovered service reaches `ServiceDiscovered`. `cscbike` and
  `renphobike` bind a single service on Windows specifically to keep that gate
  passable; `ftmsbike` does not. If a Windows debug log shows repeated
  `not all services discovered`, that is the cause.

Enable the debug log from QZ's settings; it is written to QZ's writable app
directory as `debug-<timestamp>.log`.
