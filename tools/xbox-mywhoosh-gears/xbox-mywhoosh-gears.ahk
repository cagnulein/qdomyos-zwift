#Requires AutoHotkey v2.0
#SingleInstance Force

; ---------------------------------------------------------------------------
; Wired Xbox controller -> MyWhoosh / Rouvy, via keystrokes.
;
; Both apps expose a small fixed set of keyboard shortcuts (shifting, and
; whatever else each one supports). This script polls an XInput pad and
; forwards pad buttons as those keystrokes, so no companion app (BikeControl,
; ROUVY Companion) is needed on the PC.
;
; One profile per app, picked automatically from whichever window is active, so
; the same script covers both without editing anything between rides.
;
; See README.md for the alternative routes (QZ WebSocket / MQTT bridge).
; ---------------------------------------------------------------------------

; ------------------------------- config ------------------------------------

POLL_MS := 15    ; controller poll interval

; Both apps are game engines (MyWhoosh is Unreal, Rouvy is Unity) that sample
; input once per frame. A keystroke whose down and up both land inside one
; frame can be missed entirely, so every tap is held down for this long. Raise
; it if shifts still get dropped.
KEY_HOLD_MS := 60

; Hold-to-repeat, for the bindings that opt in via "repeat" below.
REPEAT_FIRST_MS := 400   ; delay before a held button starts repeating
REPEAT_EVERY_MS := 180   ; interval between repeats while held

; Left stick deflected past this counts as a D-pad left/right press, so the
; stick and the D-pad both steer. 7849 is Microsoft's own left-stick deadzone.
STICK_DEADZONE := 7849

; A trigger pulled past this counts as pressed. Triggers are analog (0-255);
; 30 is Microsoft's own XINPUT_GAMEPAD_TRIGGER_THRESHOLD.
TRIGGER_THRESHOLD := 30

TOGGLE_HOTKEY := "^!p"   ; suspend/resume

; Log every button edge and keystroke to xbox-mywhoosh-gears.log, next to this
; script. Turn on when a binding "does nothing" and you need to know which part
; of the chain failed: the pad, the window match, or the keystroke.
DEBUG := false

; ------------------------------ profiles -----------------------------------

; XInput button masks:
;   DPAD_UP 0x0001  DPAD_DOWN 0x0002  DPAD_LEFT 0x0004  DPAD_RIGHT 0x0008
;   START   0x0010  BACK      0x0020  LTHUMB    0x0040  RTHUMB     0x0080
;   LB      0x0100  RB        0x0200  A         0x1000  B          0x2000
;   X       0x4000  Y         0x8000
;
; The analog triggers are not part of that word - the hardware reports them as
; two bytes elsewhere in the state struct. XInputButtons() converts them into
; the two synthetic masks below so a trigger can be bound like any button.
; These sit above 16 bits precisely so they can never collide with a real one:
;   LT      0x10000 RT        0x20000
;
; mode "tap"  - press sends the key once, held down for KEY_HOLD_MS.
; mode "hold" - key goes down with the button and up when it is released, which
;               is what a continuous input like steering needs.
;
; The first profile whose window is active wins. A profile with window "" is a
; catch-all and matches anything, so it only makes sense as the last entry.
; Matching on the process rather than the title survives either app renaming
; its window mid-ride.
;
; Only shortcuts the apps actually implement are bound; see README.

PROFILES := [
{
    name: "MyWhoosh",
    window: "ahk_exe MyWhoosh.exe",
    bindings: [
        ; --- virtual shifting (MyShift) ----------------------------------
        ; Triggers shift up, bumpers shift down, on both sides - so either hand
        ; has a full shifter under it, the way a road bike's right lever works
        ; both ways. The rule is the pair you are touching, not which side.
        { mask: 0x20000, key: "i",    mode: "tap",  repeat: true,  name: "shift up"    },  ; RT
        { mask: 0x10000, key: "i",    mode: "tap",  repeat: true,  name: "shift up"    },  ; LT
        { mask: 0x0200, key: "k",     mode: "tap",  repeat: true,  name: "shift down"  },  ; RB
        { mask: 0x0100, key: "k",     mode: "tap",  repeat: true,  name: "shift down"  },  ; LB

        ; --- steering (left stick or D-pad) ------------------------------
        { mask: 0x0004, key: "Left",  mode: "hold", repeat: false, name: "steer left"  },
        { mask: 0x0008, key: "Right", mode: "hold", repeat: false, name: "steer right" },

        ; --- emotes -------------------------------------------------------
        { mask: 0x8000, key: "1",     mode: "tap",  repeat: false, name: "peace"       },  ; Y
        { mask: 0x2000, key: "2",     mode: "tap",  repeat: false, name: "wave"        },  ; B
        { mask: 0x4000, key: "3",     mode: "tap",  repeat: false, name: "fist bump"   },  ; X
        { mask: 0x0002, key: "4",     mode: "tap",  repeat: false, name: "dab"         },  ; D-pad down
        { mask: 0x0040, key: "5",     mode: "tap",  repeat: false, name: "elbow flick" },  ; L3
        { mask: 0x0001, key: "6",     mode: "tap",  repeat: false, name: "toast"       },  ; D-pad up
        { mask: 0x1000, key: "7",     mode: "tap",  repeat: false, name: "thumbs up"   },  ; A

        ; --- UI ------------------------------------------------------------
        { mask: 0x0020, key: "u",     mode: "tap",  repeat: false, name: "minimal UI"  },  ; Back
        { mask: 0x0010, key: "h",     mode: "tap",  repeat: false, name: "hide UI"     }   ; Start
    ]
},
{
    name: "Rouvy",
    window: "ahk_exe Rouvy.exe",
    bindings: [
        ; --- virtual shifting (24 gears) ----------------------------------
        ; Rouvy accepts either "." / "," or "+" / "-". The unshifted pair is
        ; used because "+" is Shift+= on most layouts, which is a messier key
        ; event for a game engine to read. Swap if your build disagrees.
        { mask: 0x20000, key: ".",    mode: "tap",  repeat: true,  name: "shift up"    },  ; RT
        { mask: 0x10000, key: ".",    mode: "tap",  repeat: true,  name: "shift up"    },  ; LT
        { mask: 0x0200, key: ",",     mode: "tap",  repeat: true,  name: "shift down"  },  ; RB
        { mask: 0x0100, key: ",",     mode: "tap",  repeat: true,  name: "shift down"  },  ; LB

        ; --- camera views (OmniMode routes) --------------------------------
        { mask: 0x8000, key: "f",     mode: "tap",  repeat: false, name: "front view"  },  ; Y
        { mask: 0x2000, key: "b",     mode: "tap",  repeat: false, name: "look back"   },  ; B
        { mask: 0x4000, key: "p",     mode: "tap",  repeat: false, name: "panorama"    }   ; X
    ]
}
]
; Rouvy has no steering and no emotes, so the sticks, D-pad, Back and Start are
; unbound there rather than mapped to something invented.

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

; Returns the button word with the left stick folded into the D-pad left/right
; bits and the two triggers folded in as 0x10000 / 0x20000, or -1 when that slot
; has no pad. Everything downstream then treats a trigger as just another mask.
XInputButtons(padIndex) {
    if DllCall(XInputGetStateAddr, "UInt", padIndex, "Ptr", XInputState, "UInt") != 0
        return -1  ; ERROR_DEVICE_NOT_CONNECTED
    buttons := NumGet(XInputState, 4, "UShort")
    lx := NumGet(XInputState, 8, "Short")
    if (lx <= -STICK_DEADZONE)
        buttons |= 0x0004
    else if (lx >= STICK_DEADZONE)
        buttons |= 0x0008
    if (NumGet(XInputState, 6, "UChar") >= TRIGGER_THRESHOLD)
        buttons |= 0x10000
    if (NumGet(XInputState, 7, "UChar") >= TRIGGER_THRESHOLD)
        buttons |= 0x20000
    return buttons
}

; ------------------------------ main loop ----------------------------------

global heldKeys := Map()      ; key name -> true while currently held down
global prevButtons := 0
global activePad := -1
global activeProfile := ""

; Repeat timing lives on the binding itself, so two profiles can use the same
; button without sharing state.
for p in PROFILES
    for b in p.bindings
        b.nextRepeat := 0

PickProfile() {
    for p in PROFILES
        if (p.window = "" || WinActive(p.window))
            return p
    return ""
}

Poll(*) {
    static busy := false
    if (busy)                     ; Tap() sleeps; never re-enter
        return
    busy := true
    try
        PollOnce()
    finally
        busy := false
}

PollOnce() {
    global prevButtons, activePad, activeProfile

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
        ReleaseAllHeld()
        prevButtons := 0
        return
    }

    profile := PickProfile()
    name := (profile = "") ? "" : profile.name
    if (name !== activeProfile) {    ; compare names, not object identity
        ReleaseAllHeld()             ; never leave a key stuck across a switch
        activeProfile := name
        prevButtons := buttons       ; a button held across the switch is not a press
        Log("profile -> " (name = "" ? "(none)" : name))
        return
    }
    if (profile = "") {
        prevButtons := buttons       ; track state, but stay quiet
        return
    }

    for b in profile.bindings
        HandleBinding(buttons, b)
    prevButtons := buttons
}

HandleBinding(buttons, b) {
    global prevButtons

    down := (buttons & b.mask) != 0
    wasDown := (prevButtons & b.mask) != 0
    now := A_TickCount

    if (b.mode = "hold") {
        if (down && !wasDown)
            KeyDown(b)
        else if (!down && wasDown)
            KeyUp(b)
        return
    }

    if (down && !wasDown) {
        Tap(b)
        b.nextRepeat := now + REPEAT_FIRST_MS
    } else if (down && b.repeat && REPEAT_FIRST_MS > 0 && now >= b.nextRepeat) {
        Tap(b)
        b.nextRepeat := now + REPEAT_EVERY_MS
    }
}

Tap(b) {
    SendInput "{" b.key " down}"
    Sleep KEY_HOLD_MS
    SendInput "{" b.key " up}"
    Log("tap " b.name " -> " b.key)
}

KeyDown(b) {
    global heldKeys
    SendInput "{" b.key " down}"
    heldKeys[b.key] := true
    Log("down " b.name " -> " b.key)
}

KeyUp(b) {
    global heldKeys
    SendInput "{" b.key " up}"
    heldKeys.Delete(b.key)
    Log("up " b.name " -> " b.key)
}

ReleaseAllHeld() {
    global heldKeys
    for key in heldKeys.Clone() {
        SendInput "{" key " up}"
        heldKeys.Delete(key)
        Log("release " key)
    }
}

Log(msg) {
    if (DEBUG)
        FileAppend FormatTime(, "HH:mm:ss") " " msg "`n", A_ScriptDir "\xbox-mywhoosh-gears.log"
}

SetTimer Poll, POLL_MS

; ------------------------------ tray / hotkey ------------------------------

A_IconTip := "Xbox -> MyWhoosh / Rouvy (" TOGGLE_HOTKEY " to pause)"

Hotkey TOGGLE_HOTKEY, TogglePaused

TogglePaused(*) {
    static paused := false
    paused := !paused
    SetTimer Poll, paused ? 0 : POLL_MS
    if (paused)
        ReleaseAllHeld()
    TrayTip paused ? "Paused" : "Running", "Xbox -> MyWhoosh / Rouvy", 1
}

OnExit((*) => ReleaseAllHeld())
