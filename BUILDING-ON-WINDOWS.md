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
| `windows-exe-only` | mingw, **Qt 5.15.2** | the branch you dispatch - the stripped `.exe` alone |
| `windows-msvc2022-binary-no-python` | MSVC 2022, **Qt 6.8.2** | upstream nightly only, and from `refs/pull/1508/head` - a PR branch, not master |

**Use `windows-exe-only` for iterating.** The full zip is ~62 MB and has failed
mid-transfer; the exe on its own is a few MB compressed and downloads in seconds.
Drop it straight into an existing `C:\QZ\...` install, over the old `.exe`.

It is only a drop-in replacement while the Qt DLL set is unchanged. Any commit
touching the `QT +=` line in `src/qdomyos-zwift.pri`, the Qt version, or the
toolchain needs the full `windows-binary-no-python` instead. QZ logs its build on
every launch, which is how you check:

```
QZ build 6eb7428 Qt 5.15.2 on Windows 11 ...
```

If that Qt version does not match the DLLs in the install, take the full artifact.
The commit on the same line answers "am I actually running the new binary?" - which
used to be answered by grepping ASCII out of the `.exe`.

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

Left running: `window-build`, `android-build`, and `linux-x86-build`.

`linux-x86-build` is back on as a **tests-only** job. It is the only job in the
workflow that runs the gtest suite, and while it was off every test written had to
be run by hand in a build VM. It publishes no Linux binary and gates nothing, so it
runs beside the Windows build rather than adding its minutes to that critical path.

Disabled here: `ios-build`, the three `raspberry-pi-*` jobs, `android-emulator-test`,
the vendor APK flavours `nordictrack-build` and `fitpro-build`, and:

- `window-msvc2019-build` and `window-msvc2019-aiserver-build`, which carried no
  gate at all and so started three Windows runners on every push and every PR for
  builds this fork does not ship.
- `window-msvc2022-pr-build`, whose first step checks out `ref: qt6`. This fork has
  no `qt6` branch, so it failed on that step on every PR and reported a red X that
  had nothing to do with the PR.

Re-enabling one is a single word - change its `if: false` back.

The nightly `schedule:` trigger is gone. `window-msvc2022-build`,
`peloton-bike-build`, `peloton-bike-plus-build` and `upload_to_release` are all
gated on `github.event_name == 'schedule'` and so are now dormant; they are left
gated that way so restoring the cron restores them.

A push to a feature branch does not double up with its PR run - `on: push` is
already restricted to `master`. A manual `workflow_dispatch` on a branch that also
has a PR open *is* two runs; that is the dispatch, not the push.

A superseded run reports `cancelled`, not `failed`, because of
`concurrency: cancel-in-progress`. Anything polling a run ID must treat `cancelled`
as "find the newer run".

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
- **The bike must be paired in Windows Settings first.** Until it is bonded at the
  OS level, the connection succeeds and the services enumerate, but no
  notifications are ever delivered - `characteristicChanged` stays at 0 and every
  tile reads zero. After bonding it jumped straight to 158 on the same build.
- **`applewatch_fakedevice` hijacks the connection.** The `fake_bike` branch in
  `bluetooth::deviceDiscovered` is evaluated *before* the `ftmsbike` branch, so
  with that setting on, the real bike is never reached. Turn it off before
  concluding anything about a Windows connection failure.

- **Windows owns the GATT cache, and nothing invalidates it.** The
  `ATT_ATTRIBUTE_NOT_FOUND` warnings in the log are Windows serving attribute
  handles out of the bond record for characteristics the bike no longer exposes.
  Android's stack invalidates on a Service Changed indication and re-reads;
  `QLowEnergyControllerPrivateWin32` - which is the backend these builds use - does
  not handle Service Changed at all, so a stale cache stays stale.

  The handles seen going stale so far (`2a05`, `fff1`, `fff2`, `d18d2c10-...`) are
  not characteristics QZ uses, so this is currently cosmetic. It does mean the bond
  carries a stale database, though, and **if `2ad2` or `2ad9` ever land in that set
  the only remedy is to remove the device in Windows Settings and pair it again.**
  Reach for that before rebuilding anything.

An earlier version of this file blamed `ftmsbike::stateChanged()` for refusing to
subscribe until every service reaches `ServiceDiscovered`. That was wrong: a
Windows debug log from this bike shows `all services discovered!` firing normally.
The gate exists, but it was not what broke here, and the speculative patch written
against it was reverted before it shipped.

What *did* break, later and separately, was the interaction between that gate and
how services get built. `discoverDetails()` resolves synchronously on the Win32
backend, so creating and discovering each service in one loop drove the gate to
completion against a list holding only the services built so far. The log tell is
**more than one `all services discovered!` line per connection** - twelve of them,
in the case that cost two evenings. One line per connection is correct; several
means the list was still being built. `serviceScanDone()` now creates every service
object before discovering any of them, so the gate sees the whole list.

Enable the debug log from QZ's settings; it is written to QZ's writable app
directory as `debug-<timestamp>.log`.

## Rouvy on the same PC (DIRCON over mDNS)

QZ advertises `_wahoo-fitness-tnp._tcp.local` on port 36866 and Rouvy browses for
it. Two things had to be true before that worked, and each failed silently:

- **QZ must publish a real A record.** `ProviderPrivate::publish` calls
  `localipaddress::getIP(QHostAddress())` with a *null* argument, which skips the
  subnet-matching block; with only an Android JNI fallback behind it, every
  desktop platform returned a null address and advertised a service that resolved
  to nothing. Fixed by `bestLocalIPv4()` (commit `e390b331`), which enumerates
  interfaces and scores them so real Wi-Fi/Ethernet beats virtual adapters - which
  matters on any machine with VirtualBox's `192.168.56.1` in the list. Verify in
  QZ's log: `ProviderPrivate::publish QHostAddress("192.168.x.y")`, not `("")`.
- **Windows' own mDNS resolver must stay enabled.** Rouvy's
  `DnsZeroConfLib.dll` imports `DnsServiceBrowse` from `DNSAPI.dll`, i.e. the
  Dnscache mDNS client - *not* Apple Bonjour, which is unused (`mdnsNSP.dll` loads
  only as a Winsock namespace provider). Setting
  `HKLM:\SYSTEM\CurrentControlSet\Services\Dnscache\Parameters\EnableMDNS = 0`
  makes the browse fail and **hard-crashes Rouvy**: its failure callback re-enters
  `StopBrowsing` on a handle `StartScan` has not finished building. The
  crash is preceded by `ERROR WindowsBonjourBrowser - Network Service Discovery:
  scan failed`, and `NetworkCheckingManagerOnStatusChanged` re-triggers it on every
  address change, so a DHCP renewal becomes a crash loop.

- **Announcements must leave by the right interface.** qmdnsengine joins the
  multicast group on every interface, so queries are always *received* - but a
  socket bound to the any-address *sends* out only one interface, whichever the
  OS picks for `224.0.0.251`. Every virtual adapter on a typical dev box
  (VirtualBox host-only, Tailscale, Docker, Hyper-V) tends to outrank Wi-Fi on
  interface metric, so the answers go somewhere nobody is listening and the
  failure is completely silent. `Server::sendMessage` /
  `sendMessageToAll` now call `ServerPrivate::writeToAllInterfaces()`, which sets
  `setMulticastInterface()` and sends one copy per usable interface.

To see what the OS would pick, connect a UDP socket to the group and read back
the local address it chose:

```powershell
$u = New-Object System.Net.Sockets.UdpClient
$u.Client.Bind((New-Object System.Net.IPEndPoint([System.Net.IPAddress]::Any,0)))
$u.Connect((New-Object System.Net.IPEndPoint([System.Net.IPAddress]::Parse('224.0.0.251'),5353)))
$u.Client.LocalEndPoint.Address    # the interface announcements would go out
```

Changing `Set-NetIPInterface -InterfaceMetric` does *not* reliably move this:
Windows caches a best-interface for the group and kept using a VirtualBox adapter
demoted to metric 9000. Disabling the adapter does take effect immediately.
`netsh interface ip show joins` confirms group membership per interface, which is
a separate question from egress and will look healthy either way.

There is no port conflict between QZ and Dnscache: qmdnsengine binds 5353 with
`ShareAddress` (falling back to `ReuseAddressHint`), so the two coexist.
Do not try to make QZ the sole owner of 5353 - Rouvy's client *is* the Windows
resolver.

Rouvy's logs are worth reading directly:
`%USERPROFILE%\AppData\LocalLow\VirtualTraining\ROUVY\rouvy.log` and crash dumps
in `%LOCALAPPDATA%\Temp\VirtualTraining\ROUVY\Crashes\*\Player.log`.

`rouvy_compatibility` in QZ's settings shortens the mDNS rebroadcast interval from
30 minutes to 5 seconds (`src/qmdnsengine/src/src/hostname.cpp`), which is what
makes Rouvy notice QZ within a reasonable time.
