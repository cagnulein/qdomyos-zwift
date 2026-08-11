# QZ gears on screen while riding Rouvy

Rouvy does not offer virtual shifting to the QZ virtual trainer, so the gears
have to be changed inside QZ. This directory answers the other half of that
problem: **seeing which gear QZ is in, without leaving Rouvy.**

It targets the split setup — QZ on a tablet, Rouvy on a Windows PC — which is
the arrangement that actually works. On a single Windows box QZ cannot reliably
advertise itself: `virtualbike` needs `QLowEnergyController::createPeripheral()`
(`src/virtualdevices/virtualbike.cpp`), which Qt does not support on Windows, so
only the DirCon/Wi-Fi path is left and it is flaky over loopback
(upstream issue #3231).

```
[trainer] --BLE--> [QZ on the tablet] --Wi-Fi/DirCon--> [Rouvy on the PC]
                          |                                    ^
                          +--------- gear readout -------------+
```

Shifting itself is not covered here — use a Bluetooth volume remote with
`volume_change_gears`, a Bluetooth keyboard with `shortcut_gears_plus` /
`shortcut_gears_minus` (Android only; `src/main.qml` disables shortcuts on iOS),
or a Zwift Click/Play.

## Two routes

| | Route A — RTSS OSD | Route B — HTML overlay |
|---|---|---|
| Script | `qz_rtss_bridge.py` | `qz-gear-overlay.html` |
| Draws | inside Rouvy's own frame, via the RTSS D3D hook | a separate always-on-top window |
| Fullscreen | works, including exclusive fullscreen | needs Rouvy windowed (`Alt+Enter`) |
| Needs | RivaTuner Statistics Server, Python | any browser |
| QZ port | 6666 (the default) is fine | must **not** be 6666 — see below |

Route A is the better one, and `Start-QzRouvy.ps1` automates it end to end.

## QZ setup (both routes)

* `Settings > Rouvy Options > Rouvy Compatibility` — this is what makes Rouvy
  find QZ over Wi-Fi. It drops the mDNS rebroadcast from 30 minutes to 5
  seconds (`src/qmdnsengine/src/src/hostname.cpp`).
* `Settings > Template Settings > user_QZWS` — set to Enabled. This is the
  WebSocket these scripts read; it is off by default.
* Pair QZ in Rouvy as Power + Cadence + Controllable.

## Route A — RTSS

```
pip install websocket-client
python qz_rtss_bridge.py --host 192.168.1.50:6666
```

Renders three lines into an OSD slot RTSS owns:

```
Gear: 12/24
ERG: OFF
Resistance: 18/32
```

`--console` prints to the terminal instead, which is the quickest way to check
the data before involving RTSS. `--max-gear` and `--max-resistance` set the
denominators; QZ does not publish either maximum.

### Where the three values come from

`gears` and `resistance` ride along in the periodic `workout` broadcast
(`templateinfosenderbuilder.cpp`). **ERG does not.** QZ's ERG state is the
`zwift_erg` setting — the value the ERG tile reads to colour itself green or red
(`homeform.cpp`) — and it never enters the broadcast. The bridge therefore polls
it over the `getsettings` channel every two seconds.

The `autoresistance` field that *is* in the broadcast is a different thing: it
is the master switch for automatic resistance control, not ERG mode.

The same poll asks for `gears_zwift_ratio` and `gears_custom_table_enabled`,
because when either is on `bike::gears()` clamps the gear to 1..24, which makes
`/24` a real maximum rather than an assumption.

### Launcher

`Pedalar.bat` (double-click) → `Start-QzRouvy.ps1`. It starts RTSS, starts the
bridge, waits for QZ to connect, and only then opens Rouvy; when Rouvy exits the
bridge is shut down and the OSD slot released. It refuses to open Rouvy if QZ
never answered — pass `-LaunchAnyway` to override.

Rouvy is located by, in order: a saved path, a Microsoft Store package
(`Get-AppxPackage`, launched through `shell:AppsFolder`), the uninstall
registry, the usual install folders, and the Start Menu shortcut. If all of that
misses, run it once with `-Rouvy "C:\...\Rouvy.exe"` and the path is remembered.

The shutdown is deliberate: killing the bridge would skip its cleanup and leave
a stale gear number frozen on screen. The launcher closes the child's stdin
instead and the bridge exits through `--stop-on-stdin-eof`, releasing its slot.

## Route B — HTML overlay

Open `qz-gear-overlay.html` and give it the tablet's address, or pass it
directly:

```
chrome --app="file:///C:/.../qz-gear-overlay.html?host=192.168.1.50:8080"
```

Then Rouvy windowed (`Alt+Enter`) and PowerToys Always on Top (`Win+Ctrl+T`).
`H` changes the address, `C` toggles a compact layout.

**This route cannot use port 6666.** Browsers refuse it as `ERR_UNSAFE_PORT`
(it is on Chromium's blocked-port list as an IRC port), and the page just never
connects. Change the `user_QZWS` port to 8080. The Python bridge is unaffected.

## Notes

* `qz_rtss_bridge.py` needs `websocket-client`, unlike its neighbour
  `../xbox-mywhoosh-gears/qz_gear_bridge.py`, which is standard library only.
  That bridge's WebSocket client only sends — it never reads frames — so it
  could not be reused for a script whose whole job is to consume broadcasts.
* The RTSS shared-memory layout is read out of the header (`dwOSDArrOffset`,
  `dwOSDEntrySize`) rather than hardcoded, so older versions without `szOSDEx`
  still work.
* The `.ps1` and `.bat` speak Portuguese, matching the rider they were written
  for.
