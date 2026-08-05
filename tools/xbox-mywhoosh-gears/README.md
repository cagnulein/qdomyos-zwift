# Wired Xbox controller → MyWhoosh virtual gears

Shift MyWhoosh's virtual gears from a wired Xbox controller on Windows, without
the BikeControl companion app.

Two routes are provided, and they answer different questions:

| | Route A — keystrokes | Route B — QZ bridge |
|---|---|---|
| Script | `xbox-mywhoosh-gears.ahk` | `qz_gear_bridge.py` |
| Path | pad → AutoHotkey → `I`/`K` → MyWhoosh | pad → Python → QZ WebSocket → OpenBikeControl → MyWhoosh |
| Who owns the gears | MyWhoosh (MyShift) | QZ (gear table, gear→resistance mapping, gear tile) |
| Needs QZ running | no | yes, on a host the PC can reach |
| Needs MyWhoosh focused | yes | no |

Start with Route A. Move to Route B only if you want QZ's gearing model rather
than MyWhoosh's.

## Route A — AutoHotkey sends MyWhoosh's own shift keys

MyWhoosh shifts with the `I` (up) and `K` (down) keys, the same keys the
handlebar-shifter DIY builds emulate with a BLE keyboard. The script polls the
pad through XInput and sends those keystrokes only while the MyWhoosh window is
active.

1. Install [AutoHotkey v2](https://www.autohotkey.com/).
2. Plug in the controller and run `xbox-mywhoosh-gears.ahk`.
3. `RB` shifts up (harder), `LB` shifts down (easier). Hold a button to repeat.
   `Ctrl+Alt+P` pauses the script.

Rebinding is at the top of the file: `BTN_GEAR_UP` / `BTN_GEAR_DOWN` take the
XInput masks listed in the comments (D-pad, A/B/X/Y, thumb clicks, …), and
`TARGET_WINDOW` restricts where keys are sent — set it to `""` to send
regardless of focus.

Requirements and caveats:

- MyWhoosh virtual shifting (MyShift) must be active for the ride — that means
  the trainer paired in a mode MyWhoosh can control. The gear indicator sits at
  the bottom right and starts at 15.
- Because MyWhoosh owns the gears here, do not run QZ's virtual gearing against
  the same ride as well, or you get two gearboxes fighting over resistance.
- The analog triggers are not buttons in XInput; they are separate 0–255 axes
  (offsets 6 and 7 of `XINPUT_STATE`). The script reads the button word only, so
  bind shoulder buttons, D-pad or face buttons.

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
2. Settings → **OpenBikeControl** → *Enable OpenBikeControl*. QZ then advertises
   `_openbikecontrol._tcp` on port 36867 and MyWhoosh discovers it. Enable
   *override local gears* if the shift should go only to MyWhoosh instead of
   also moving QZ's local gear.
3. Start the workout with auto resistance on — `homeform::gearUp()` is a no-op
   otherwise (`src/homeform.cpp:2073`).

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
  asked for exactly this (for Rouvy). Closed `enhancement` + `wontfix`.
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

Non-QZ prior art for Route A: Paul Stallard's
[handlebar-mounted virtual shifters](https://paulstallard.com/posts/2026-01-06_picow_ble_keyboard/)
send MyWhoosh's `I`/`K` from a Pico W acting as a BLE keyboard — the same
keystroke path, in hardware.

## References

- [MyShift — MyWhoosh's Virtual Shifting Explained](https://mywhoosh.com/myshift-mywhooshs-virtual-shifting-explained/)
- [MyWhoosh keyboard shortcuts](https://mywhooshinfo.com/blog/mywhoosh-keyboard-shortcuts)
- [MyWhoosh now officially supports BikeControl / OpenBikeControl](https://bikecontrol.app/blog/mywhoosh-bikecontrol-partnership/)
- [OpenBikeControl protocol](https://github.com/OpenBikeControl/openbikecontrol-protocol)
- [QZ wiki — Virtual Gearing with Zwift, Rouvy, MyWhoosh and Training Peaks](https://github.com/cagnulein/qdomyos-zwift/wiki/Virtual-Gearing-on-QZ-with-Zwift,-Rouvy,-Mywhoosh-and-Traning-Peaks)
