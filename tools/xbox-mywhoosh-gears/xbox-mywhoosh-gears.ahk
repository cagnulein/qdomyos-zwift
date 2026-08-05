#Requires AutoHotkey v2.0
#SingleInstance Force

; ---------------------------------------------------------------------------
; Wired Xbox controller -> MyWhoosh virtual gears (MyShift), via keystrokes.
;
; MyWhoosh shifts gears with the "I" (up) and "K" (down) keys. This script
; polls an XInput pad and forwards the shoulder buttons as those keystrokes,
; so no companion app (BikeControl) is needed on the PC.
;
; See README.md for the alternative routes (QZ WebSocket / MQTT bridge).
; ---------------------------------------------------------------------------

; ------------------------------- config ------------------------------------

; Only send keys while this window is active, so the pad doesn't type into
; other apps. Set to "" to send regardless of which window has focus.
TARGET_WINDOW := "MyWhoosh"

KEY_GEAR_UP   := "i"
KEY_GEAR_DOWN := "k"

; XInput button masks (see BTN map below) bound to each action.
; Defaults mirror a road bike: right shifter = harder, left shifter = easier.
BTN_GEAR_UP   := 0x0200  ; RB / right shoulder
BTN_GEAR_DOWN := 0x0100  ; LB / left shoulder

POLL_MS := 15    ; controller poll interval

; Hold-to-repeat. Set REPEAT_FIRST_MS := 0 to shift only on the initial press.
REPEAT_FIRST_MS := 400   ; delay before a held button starts repeating
REPEAT_EVERY_MS := 180   ; interval between repeats while held

; Suspend/resume hotkey.
TOGGLE_HOTKEY := "^!p"

; ----------------------------- XInput setup --------------------------------

global XInputGetStateAddr := 0
for dll in ["xinput1_4", "xinput1_3", "xinput9_1_0"] {
    if (hModule := DllCall("LoadLibrary", "Str", dll ".dll", "Ptr")) {
        XInputGetStateAddr := DllCall("GetProcAddress", "Ptr", hModule, "AStr", "XInputGetState", "Ptr")
        break
    }
}
if !XInputGetStateAddr {
    MsgBox "XInput not found. Plug in the controller and make sure the Xbox driver is installed."
    ExitApp
}

; XINPUT_STATE: DWORD dwPacketNumber; XINPUT_GAMEPAD Gamepad
;   Gamepad: WORD wButtons (offset 4), BYTE bLeftTrigger (6), bRightTrigger (7),
;            SHORT sThumbLX (8), sThumbLY (10), sThumbRX (12), sThumbRY (14)
global XInputState := Buffer(16, 0)

XInputButtons(padIndex) {
    if DllCall(XInputGetStateAddr, "UInt", padIndex, "Ptr", XInputState, "UInt") != 0
        return -1  ; ERROR_DEVICE_NOT_CONNECTED
    return NumGet(XInputState, 4, "UShort")
}

; Button masks, for reference when rebinding above:
;   DPAD_UP 0x0001  DPAD_DOWN 0x0002  DPAD_LEFT 0x0004  DPAD_RIGHT 0x0008
;   START   0x0010  BACK      0x0020  LTHUMB    0x0040  RTHUMB     0x0080
;   LB      0x0100  RB        0x0200  A         0x1000  B          0x2000
;   X       0x4000  Y         0x8000
; (The analog triggers are not buttons; they live at offsets 6 and 7.)

; ------------------------------ main loop ----------------------------------

global heldSince := Map(BTN_GEAR_UP, 0, BTN_GEAR_DOWN, 0)
global nextRepeat := Map(BTN_GEAR_UP, 0, BTN_GEAR_DOWN, 0)
global prevButtons := 0
global activePad := -1

Poll(*) {
    global prevButtons, activePad

    buttons := -1
    if (activePad >= 0) {
        buttons := XInputButtons(activePad)
        if (buttons = -1)
            activePad := -1          ; pad went away, rescan next tick
    }
    if (activePad < 0) {
        loop 4 {
            if (XInputButtons(A_Index - 1) != -1) {
                activePad := A_Index - 1
                buttons := XInputButtons(activePad)
                break
            }
        }
    }
    if (buttons = -1) {
        prevButtons := 0
        return
    }

    if (TARGET_WINDOW != "" && !WinActive(TARGET_WINDOW)) {
        prevButtons := buttons       ; track state, but stay quiet
        return
    }

    HandleButton(buttons, BTN_GEAR_UP, KEY_GEAR_UP)
    HandleButton(buttons, BTN_GEAR_DOWN, KEY_GEAR_DOWN)
    prevButtons := buttons
}

HandleButton(buttons, mask, key) {
    global prevButtons, heldSince, nextRepeat

    down := (buttons & mask) != 0
    wasDown := (prevButtons & mask) != 0
    now := A_TickCount

    if (down && !wasDown) {
        Shift(key)
        heldSince[mask] := now
        nextRepeat[mask] := now + REPEAT_FIRST_MS
    } else if (down && REPEAT_FIRST_MS > 0 && now >= nextRepeat[mask]) {
        Shift(key)
        nextRepeat[mask] := now + REPEAT_EVERY_MS
    } else if (!down) {
        heldSince[mask] := 0
    }
}

Shift(key) {
    SendInput "{" key "}"
}

SetTimer Poll, POLL_MS

; ------------------------------ tray / hotkey ------------------------------

A_IconTip := "Xbox -> MyWhoosh gears (" TOGGLE_HOTKEY " to pause)"

Hotkey TOGGLE_HOTKEY, TogglePaused

TogglePaused(*) {
    static paused := false
    paused := !paused
    SetTimer Poll, paused ? 0 : POLL_MS
    TrayTip paused ? "Paused" : "Running", "Xbox -> MyWhoosh gears", 1
}
