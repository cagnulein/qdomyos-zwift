#Requires AutoHotkey v2.0
#SingleInstance Force

; ---------------------------------------------------------------------------
; Pad probe - what does Windows actually see?
;
; xbox-mywhoosh-gears.ahk reads the pad through XInput, which only some pads
; speak. A pad in DirectInput mode (8BitDo's "D") is invisible to XInput, and
; a pad in keyboard mode ("K") is not a pad at all - it is a keyboard, and
; neither API will report it.
;
; This shows all three at once, live, so the answer takes one press per button
; rather than a guess. Run it, press every button you intend to bind, and read
; the numbers off the window. Edges are also appended to probe-pad.log.
;
; It binds nothing and sends no keystrokes - it is safe to leave running while
; an app is in front.
; ---------------------------------------------------------------------------

POLL_MS := 50
LOG := true          ; append every button edge to probe-pad.log

; ----------------------------- XInput setup --------------------------------

global XInputGetStateAddr := 0
for dll in ["xinput1_4", "xinput1_3", "xinput9_1_0"] {
    if (hModule := DllCall("LoadLibrary", "Str", dll ".dll", "Ptr")) {
        XInputGetStateAddr := DllCall("GetProcAddress", "Ptr", hModule, "AStr", "XInputGetState", "Ptr")
        break
    }
}

global XInputState := Buffer(16, 0)

; ------------------------------- the window --------------------------------

g := Gui("+AlwaysOnTop +Resize", "Pad probe")
g.SetFont("s10", "Consolas")
global out := g.AddText("w700 h460", "scanning...")
g.OnEvent("Close", (*) => ExitApp())
g.Show()

global prevDown := Map()     ; "src:id" -> true while that input reads pressed

SetTimer Update, POLL_MS

Update(*) {
    s := "=== XInput ===`n"
    s .= XInputReport()
    s .= "`n=== DirectInput ===`n"
    s .= DirectInputReport()
    s .= "`n"
    s .= "Nothing at all? The pad is probably in keyboard mode - it is a`n"
    s .= "keyboard then, so type into Notepad to see which keys it sends.`n"
    out.Value := s
}

XInputReport() {
    if !XInputGetStateAddr
        return "  xinput dll not found`n"

    s := ""
    found := false
    loop 4 {
        slot := A_Index - 1
        if DllCall(XInputGetStateAddr, "UInt", slot, "Ptr", XInputState, "UInt") != 0
            continue
        found := true
        buttons := NumGet(XInputState, 4, "UShort")
        lt := NumGet(XInputState, 6, "UChar")
        rt := NumGet(XInputState, 7, "UChar")
        lx := NumGet(XInputState, 8, "Short")
        ly := NumGet(XInputState, 10, "Short")
        s .= Format("  slot {1}  buttons 0x{2:04X}  LT {3:3}  RT {4:3}  LX {5:6}  LY {6:6}`n",
                    slot, buttons, lt, rt, lx, ly)
        s .= "    " (XInputNames(buttons) = "" ? "(no button down)" : XInputNames(buttons)) "`n"
        Edge("xinput" slot, buttons != 0 || lt > 30 || rt > 30,
             Format("buttons 0x{1:04X} LT {2} RT {3}", buttons, lt, rt))
    }
    return found ? s : "  no XInput pad in any of the 4 slots`n"
}

XInputNames(b) {
    static masks := [[0x0001, "DPAD_UP"], [0x0002, "DPAD_DOWN"], [0x0004, "DPAD_LEFT"],
                     [0x0008, "DPAD_RIGHT"], [0x0010, "START"], [0x0020, "BACK"],
                     [0x0040, "L3"], [0x0080, "R3"], [0x0100, "LB"], [0x0200, "RB"],
                     [0x1000, "A"], [0x2000, "B"], [0x4000, "X"], [0x8000, "Y"]]
    names := []
    for m in masks
        if (b & m[1])
            names.Push(m[2])
    return Join(names, " ")
}

DirectInputReport() {
    s := ""
    found := false
    loop 16 {
        id := A_Index
        name := GetKeyState(id "JoyName")
        if (name = "")
            continue
        found := true
        count := GetKeyState(id "JoyButtons")
        down := []
        loop count {
            if GetKeyState(id "Joy" A_Index)
                down.Push(A_Index)
        }
        pov := GetKeyState(id "JoyPOV")
        s .= "  joystick " id ": " name "  (" count " buttons)`n"
        s .= "    down: " (down.Length ? Join(down, " ") : "(none)")
             . "    POV: " (pov = -1 ? "centred" : pov / 100 " deg") "`n"
        s .= "    axes: " AxisReport(id) "`n"
        Edge("joy" id, down.Length > 0 || pov != -1,
             "buttons " Join(down, "+") " POV " pov)
    }
    return found ? s : "  no DirectInput joystick found`n"
}

; Axes read 50 when centred or when the stick does not exist, so an axis stuck
; at exactly 50 says nothing - move it and watch whether the number follows.
AxisReport(id) {
    parts := []
    for ax in ["X", "Y", "Z", "R", "U", "V"] {
        v := GetKeyState(id "Joy" ax)
        if (v != "")
            parts.Push(ax "=" Round(v))
    }
    return Join(parts, "  ")
}

; Logs only the transitions, so holding a button does not flood the file.
Edge(src, isDown, detail) {
    global prevDown
    was := prevDown.Has(src) && prevDown[src]
    if (isDown = was)
        return
    prevDown[src] := isDown
    if (isDown && LOG)
        FileAppend FormatTime(, "HH:mm:ss") " " src " " detail "`n", A_ScriptDir "\probe-pad.log"
}

Join(arr, sep) {
    s := ""
    for v in arr
        s .= (s = "" ? "" : sep) v
    return s
}
