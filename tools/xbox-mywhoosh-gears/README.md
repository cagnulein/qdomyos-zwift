# Wired Xbox controller → MyWhoosh / Rouvy virtual gears

Shift virtual gears from a wired Xbox controller on Windows, without the
BikeControl or ROUVY Companion apps. (The directory keeps its original
MyWhoosh-only name; Rouvy support was added later.)

Two routes are provided, and they answer different questions:

| | Route A — keystrokes | Route B — QZ bridge |
|---|---|---|
| Script | `xbox-mywhoosh-gears.ahk` | `qz_gear_bridge.py` |
| Apps | MyWhoosh and Rouvy | MyWhoosh |
| Path | pad → AutoHotkey → shift keys → app | pad → Python → QZ WebSocket → OpenBikeControl → MyWhoosh |
| Who owns the gears | the app | QZ (gear table, gear→resistance mapping, gear tile) |
| Needs QZ running | no | yes, on a host the PC can reach |
| Needs the app focused | yes | no |

Start with Route A. Move to Route B only if you want QZ's gearing model rather
than MyWhoosh's. Route B is MyWhoosh-only — it works by speaking
OpenBikeControl, which Rouvy does not implement.

## Route A — AutoHotkey sends each app's own shift keys

The script polls the pad through XInput and sends the focused app's shortcuts.
It ships with one **profile** per app, and picks whichever profile's window is
active — so the same script covers both, with nothing to change between rides.
When neither app is in front, it stays completely silent.

1. Install [AutoHotkey v2](https://www.autohotkey.com/), e.g.
   `winget install --id AutoHotkey.AutoHotkey --exact`. A silent/user-scope
   install does not always register the `.ahk` file association; `run-gears.cmd`
   sidesteps that by calling `AutoHotkey64.exe` directly.
2. Plug in the controller and run `run-gears.cmd` (or `xbox-mywhoosh-gears.ahk`
   itself, if double-clicking scripts works on the machine).
3. `Ctrl+Alt+P` pauses the script; the tray icon exits it.

### Pad mapping — MyWhoosh

MyWhoosh's keyboard support is deliberately small — steering, shifting, seven
emotes and two UI toggles is the whole of it. All of it is bound:

| Pad | Key | Action |
|---|---|---|
| `RT` or `LT` | `I` | Shift up (harder) — hold to repeat |
| `RB` or `LB` | `K` | Shift down (easier) — hold to repeat |
| Left stick ←/→, or D-pad ←/→ | `←` / `→` | Steer, held for as long as the input is |
| `Y` | `1` | Peace |
| `B` | `2` | Wave |
| `X` | `3` | Fist bump |
| D-pad ↓ | `4` | Dab |
| `L3` | `5` | Elbow flick |
| D-pad ↑ | `6` | Toast |
| `A` | `7` | Thumbs up |
| `Back` | `U` | Toggle minimal UI |
| `Start` | `H` | Hide all controls (HD version only) |

Shifting is on both shoulder pairs, by pair rather than by side: **triggers
shift up, bumpers shift down**, left and right alike. Either hand therefore has
a complete shifter under it, so it does not matter which way the pad ends up
mounted on the bars. The same rule applies to Rouvy below.

`R3` is left free: `F11` (fullscreen) is the only shortcut still spare, and a
stray thumb-click dropping MyWhoosh out of fullscreen mid-ride is worse than not
having it.

### Pad mapping — Rouvy

Rouvy's shortcut surface is smaller still: 24 virtual gears, and the OmniMode
camera views. There is no steering and there are no emotes, so the sticks,
D-pad, `Back` and `Start` are left unbound rather than mapped to something
invented.

| Pad | Key | Action |
|---|---|---|
| `RT` or `LT` | `.` | Shift up — hold to repeat |
| `RB` or `LB` | `,` | Shift down — hold to repeat |
| `Y` | `F` | Front view |
| `B` | `B` | Look back |
| `X` | `P` | Panorama |

Rouvy accepts either `.` / `,` or `+` / `-` for shifting. The unshifted pair is
bound because `+` is `Shift`+`=` on most layouts (and on the ABNT2 layout this
machine uses), which is a messier event for a game engine to read than a single
unshifted key. Both forms were checked to send correctly; swap them in the
profile if your build disagrees.

The camera keys apply to OmniMode (360°) routes — on a normal route there is
nothing for them to switch.

### Rebinding

Each profile is an entry in the `PROFILES` table at the top of the script, with
a `window` to match on and a list of bindings. A binding takes an XInput mask
(the masks are listed just above the table), the key to send, and a mode:

- `tap` sends the key once per press, with optional hold-to-repeat. Correct for
  shifting, emotes and camera views.
- `hold` presses the key down with the button and releases it when the button
  comes up. Steering needs this; a tap would produce a twitch rather than a
  turn.

The first profile whose window is active wins, so order matters; a profile with
`window: ""` matches anything and is only useful as a last-entry catch-all.
Adding a third app is a new entry, not a code change.

Held keys are released whenever the profile changes, the app loses focus, the
pad disconnects, the script is paused, or it exits — so a steering key can never
get stuck down.

Windows are matched on the process rather than the title, so a mid-ride title
change cannot silently stop the shifting:

- MyWhoosh — the Microsoft Store build
  (`MyWhooshTechnologyService.644173E064ED2`) ships its binary at
  `MyWhoosh\Binaries\Win64\MyWhoosh.exe`.
- Rouvy — installs outside Program Files' usual layout, at
  `C:\Program Files\VirtualTraining\Rouvy\Rouvy.exe`.

If a build names its executable differently, check with AutoHotkey's bundled
WindowSpy.

Requirements and caveats:

- Virtual shifting must be active for the ride. On MyWhoosh that is MyShift,
  which needs the trainer paired in a mode MyWhoosh can control; its gear
  indicator sits at the bottom right and starts at 15. On Rouvy it is the
  virtual shifting mode, giving 24 gears.
- Because the app owns the gears here, do not run QZ's virtual gearing against
  the same ride as well, or you get two gearboxes fighting over resistance.
- The analog triggers are not buttons in XInput; they are separate 0–255 axes
  (offsets 6 and 7 of `XINPUT_STATE`), absent from the button word every other
  binding is matched against. `XInputButtons()` reads them and folds them in as
  two invented masks, `0x10000` for `LT` and `0x20000` for `RT`, deliberately
  above 16 bits so they can never collide with a real button. A trigger counts
  as pressed past `TRIGGER_THRESHOLD` (30, Microsoft's own value) — everything
  downstream then treats it as an ordinary button.

### When a button does nothing

Work down the chain rather than guessing — set `DEBUG := true` at the top of the
script and it logs every profile switch, button edge and keystroke to
`xbox-mywhoosh-gears.log` next to the script. That separates the failures that
look identical from the saddle:

1. **Nothing in the log at all** — the pad is not being read. An unplugged or
   asleep controller makes the script silently do nothing, by design.
2. **The log stops at `profile -> (none)`** — no profile's window is active.
   The script is deliberately quiet unless the app is in front, so alt-tabbing
   away stops it.
3. **The log shows the keystroke, but the app ignores it** — the usual cause is
   the app sampling input once per frame and missing a keystroke that went down
   and up inside that frame. Both are game engines (MyWhoosh is Unreal, Rouvy is
   Unity), so raise `KEY_HOLD_MS` (default 60 ms).
4. **The wrong thing happens** — the shortcut is bound the other way round in
   your build. Swap the two `key:` values in that profile.
5. **The button tests fine in Windows but does nothing here** — check which
   button it actually is. `R2` and `L2` are PlayStation names for the *triggers*,
   which Xbox calls `RT`/`LT`; the bumpers are `R1`/`L1` there and `RB`/`LB`
   here. A pad tester showing the button alive says nothing about whether this
   script has it bound. Both pairs shift now, so either naming reaches a
   shifter, but the tables above use Xbox names throughout — read `R2` as `RT`.

If the app is running elevated and AutoHotkey is not, Windows blocks the
keystrokes outright (UIPI) and nothing reaches the app whatever the log says;
run both the same way.

## Route B — bridge the pad into QZ, let QZ drive MyWhoosh

QZ already speaks OpenBikeControl, the protocol MyWhoosh implemented natively in
5.7.0 — that is the same protocol BikeControl uses, so QZ replaces BikeControl
here rather than sitting behind it. Every gear change QZ makes is forwarded to
MyWhoosh (`src/devices/bike.cpp:290`, `src/mywhooshlink.cpp`), whatever
triggered it: a Zwift Play/Click paddle, the gear tile, or a remote command.

`qz_gear_bridge.py` is that remote command: it reads the pad with XInput and
sends `{"msg":"gears_plus"}` / `{"msg":"gears_minus"}` over QZ's template
WebSocket, the same messages the QZ web UI sends
(`src/templateinfosenderbuilder.cpp:1590`). Standard library only — no pip
install.

QZ setup:

1. Settings → Template Settings → enable the **web server** (port 6666 by
   default, `src/webserverinfosender.cpp:94`).
2. Settings → **OpenBikeControl** → *Enable OpenBikeControl*, then **restart
   QZ**: the link is only constructed at startup (`src/main.cpp:892`), so the
   toggle does nothing until the app is relaunched. QZ then advertises
   `_openbikecontrol._tcp` on port **36867** and MyWhoosh discovers it. Enable
   *override local gears* if the shift should go only to MyWhoosh instead of
   also moving QZ's local gear.
3. Start the workout with auto resistance on — `homeform::gearUp()` is a no-op
   otherwise (`src/homeform.cpp:2073`).

Note: the settings screen still describes the OpenBikeControl server as being on
"port 21587" (`src/settings.qml:3481`). That label predates commit `e55e4b3`,
which moved the server to the OpenBikeControl port 36867
(`src/mywhooshlink.h:109`). Trust the code, not the label.

### Topology: QZ on a tablet, MyWhoosh on a Windows PC

This is the case the bridge is actually built for, and it works — but the two
connections MyWhoosh needs (trainer data, and the controller) take different
paths:

- **Trainer data** (power/cadence/controllable): enabling OpenBikeControl in QZ
  turns *Wahoo Direct Connect* off (`src/settings.qml:3472`), so the Wi-Fi
  pairing route disappears and MyWhoosh must pair with the QZ virtual bike over
  **Bluetooth**. The Windows PC therefore needs a working Bluetooth adapter. If
  you would rather keep the Wi-Fi (Direct Connect) pairing, stay on Route A —
  the two are mutually exclusive.
- **Controller**: MyWhoosh → Connection screen → the **OpenBikeControl icon in
  the top right** finds QZ over mDNS. Tablet and PC must be on the **same Wi-Fi
  SSID and subnet** — mDNS does not cross VLANs, guest networks, AP client
  isolation, or a USB/phone tether. Windows Firewall must let MyWhoosh receive
  mDNS (UDP 5353); the outbound TCP connection to the tablet on 36867 is not
  blocked by default.
- **The gear bridge** runs on the PC and connects the other way, to the tablet:
  `python qz_gear_bridge.py --host <tablet-ip>`. Nothing listens on the PC, so
  no firewall rule is needed for it.

To confirm QZ's side is up, check its debug log: the link prints
`MyWhooshLink(OpenBikeControl): ACTIVE on port 36867` every 10 seconds
(`src/mywhooshlink.cpp:600`), plus the interface list it bound to at startup.

Then, on the Windows PC:

```
python qz_gear_bridge.py --host 192.168.1.50          # IP of the QZ device
python qz_gear_bridge.py --host 192.168.1.50 --send up  # one-shot, for testing
```

`--up-button` / `--down-button` accept `rb`, `lb`, `dpad_up`, `a`, … and
`--repeat-delay 0` disables hold-to-repeat. `--send` needs no controller, so it
is the quickest way to confirm QZ is listening before a ride.

If QZ runs on a phone or tablet, point `--host` at that device — the WebSocket
is plain TCP on the LAN, and only the Python side needs Windows (for XInput).

### Route B variant: MQTT

If a broker is already in the setup, QZ subscribes to control topics and
`QZ/<nickname>/control/bike/gears_up` (and `…/gears_down`) calls the same gear
functions (`src/mqttpublisher.cpp:421`). Same result as the WebSocket bridge,
one more moving part.

## Why not QZ's built-in keyboard shortcuts

QZ has shortcut settings including *Gears + / -*
(`src/settings-shortcuts.qml:219`), but they are declared with
`context: Qt.WindowShortcut` (`src/main.qml:1546`) — they fire only while the QZ
window itself has focus. During a ride MyWhoosh has focus, so mapping the pad to
those shortcuts with AutoHotkey does not work. That is the reason Route B goes
through the WebSocket instead of synthesizing keys for QZ.

## Upstream research

Nobody upstream appears to have gotten an Xbox controller working for this; the
one direct request was declined, and the OpenBikeControl work that landed is
what makes Route B possible.

- [#2783 — *Create Qz compatible with Xbox Controller*](https://github.com/cagnulein/qdomyos-zwift/issues/2783)
  asked for exactly this, for Rouvy. Closed `enhancement` + `wontfix` — the
  Rouvy profile in Route A is what that request wanted, done outside QZ.
- [#4512 — *[REQ] Add openbikecontrol protocol support*](https://github.com/cagnulein/qdomyos-zwift/issues/4512)
  requested the protocol so QZ could drive MyWhoosh's shifting directly. The
  producer side shipped as #4504 (`MyWhooshLink`, mDNS + TCP/UDP on 36867).
- [#4790](https://github.com/cagnulein/qdomyos-zwift/issues/4790) /
  [#4791](https://github.com/cagnulein/qdomyos-zwift/issues/4791) — *qz as an
  OpenBikeControl listener*: the symmetric half, so any OBC controller
  (Zwift Play, SRAM AXS, an ESP32 button box) could feed QZ's button→action
  dispatch. Labelled `wontfix`. A native Xbox-pad mapping in QZ would most
  naturally hang off this work.
- [#4249 — *[REQ] "click" tiles with a keyboard*](https://github.com/cagnulein/qdomyos-zwift/issues/4249)
  is where the shortcut settings came from — an Arduino keyboard for tiles, the
  same idea as Route A but aimed at QZ's own UI.
- [#4608 — *Support Zwift Play buttons (A/B/X/Y) and directional controls*](https://github.com/cagnulein/qdomyos-zwift/issues/4608)
  is the mapping UI those OBC actions are configured with.

On the connection side, there is exactly one open report of MyWhoosh failing to
find QZ: [#4862 — *MyWhoosh app (Android tablet) can't pair with QZ app (Android
phone)*](https://github.com/cagnulein/qdomyos-zwift/issues/4862) (open since
2026-07-30). The reporter's phone and tablet were joined by USB tethering rather
than a shared Wi-Fi network, which is exactly the case mDNS discovery cannot
serve. Nothing else upstream reports OpenBikeControl discovery problems.

Non-QZ prior art for Route A: Paul Stallard's
[handlebar-mounted virtual shifters](https://paulstallard.com/posts/2026-01-06_picow_ble_keyboard/)
send MyWhoosh's `I`/`K` from a Pico W acting as a BLE keyboard — the same
keystroke path, in hardware.

## References

- [MyShift — MyWhoosh's Virtual Shifting Explained](https://mywhoosh.com/myshift-mywhooshs-virtual-shifting-explained/)
- [MyWhoosh keyboard shortcuts](https://mywhooshinfo.com/blog/mywhoosh-keyboard-shortcuts)
- [MyWhoosh now officially supports BikeControl / OpenBikeControl](https://bikecontrol.app/blog/mywhoosh-bikecontrol-partnership/)
- [OpenBikeControl protocol](https://github.com/OpenBikeControl/openbikecontrol-protocol)
- [ROUVY — Virtual Shifting](https://support.rouvy.com/hc/en-us/articles/32452137189393-Virtual-Shifting)
  (`.` / `,` or `+` / `-`, 24 gears)
- [ROUVY — virtual shifting overview](https://rouvy.com/virtual-shifting)
- [ROUVY — Display and Video Issues and Tips](https://support.rouvy.com/hc/en-us/articles/12988220814225-Display-and-Video-Issues-and-Tips)
  (camera hotkeys `B` / `P` / `F`)
- [QZ wiki — Virtual Gearing with Zwift, Rouvy, MyWhoosh and Training Peaks](https://github.com/cagnulein/qdomyos-zwift/wiki/Virtual-Gearing-on-QZ-with-Zwift,-Rouvy,-Mywhoosh-and-Traning-Peaks)
