# Accessibility and screen-reader diagnostics

These checks identify whether a control is discoverable, can be activated through the screen reader, and actually reaches the fitness machine. Do not ask a user to turn off assistive technology just to make a report. Test treadmill controls only with the belt stopped and nobody standing on it.

## VoiceOver announces Start or Stop, but activating it does nothing

### Provide first

- QZ version/build, iPhone or iPad model, iOS version, and whether VoiceOver is enabled.
- The exact screen and control name VoiceOver announces. Can focus reach **Start** and **Stop**? What happens with a normal VoiceOver double-tap: no response, a QZ confirmation dialog, or a command without physical machine movement?
- Check separately whether QZ's speed/incline controls respond and whether the treadmill's **physical** Start/Stop controls work. Report each result; do not infer that a silent QZ button means Bluetooth is disconnected.

### If still unclear

- Report the exact gesture and outcome of a second activation attempt. In one confirmed test, a **double long-press** activated Start/Stop when a normal double-tap did not; this distinguishes gesture handling from missing treadmill control.
- If feasible, provide a short screen recording or a step-by-step description of VoiceOver focus, announcements, and whether a QZ confirmation prompt appears.
- Only request a QZ debug log if the button seems to activate but the machine fails to respond; a purely inaccessible gesture may leave no device-control event in the log.

### Why it matters

- QZ's Start and Stop are declared as accessible buttons in the QML interface, but being announced is not proof that the screen-reader activation triggers their click handlers.
- A working speed/incline command with nonfunctional Start/Stop activation points to a different layer than a complete loss of treadmill communication.

## VoiceOver focus moves through settings, but controls cannot be edited

### Provide first

- QZ version, iOS version, iPhone/iPad model, and the exact QZ settings page.
- Name the control (checkbox, text field, dropdown), what VoiceOver announces, and whether a normal double-tap changes its value or opens an editor.
- Say whether VoiceOver focus moves to an off-screen option while the **visible settings page fails to scroll**, or whether the page scrolls but the control still cannot be activated.

### If still unclear

- Provide a short sequence of gestures, the initial and final control value, and—if possible—a screen recording showing visible scrolling versus VoiceOver focus movement.
- Check whether the same type of control behaves differently on another settings page or the other device used for testing. Do not send credentials or personal data from text fields.
- Collect a QZ debug log only if a setting appears to save but the resulting machine behavior is wrong; UI focus/scrolling defects may not be represented in Bluetooth logs.

### Why it matters

- VoiceOver navigation, scrolling a QML settings view, and activating a checkbox/text field are distinct interactions.
- Identifying which step fails is more useful for reproducing an accessibility defect than collecting unrelated fitness-machine logs.

## Android settings switches appear disabled or do not respond in right-to-left layouts

### Provide first

- QZ build, Android version, device model, and the **system display language and layout direction** (left-to-right or right-to-left).
- Identify the exact settings page and switch that cannot be changed. Describe whether the switch itself is unresponsive or only the surrounding row/label does not respond; include a screenshot without account details.
- Check whether other switches on the same page respond, and whether the problem appeared after changing phones or language. Do not assume a purchase or license problem based on an icon beside a setting.

### If still unclear

- Without changing other QZ options, compare the same switch with Android's system language temporarily set to English (left-to-right), then restore the original language. Report whether tapping the **switch thumb/track itself** works in each layout.
- If the difference is repeatable, provide a short screen recording showing the exact tap location and resulting switch state in both layouts. Record QZ and Android versions; a Bluetooth debug log is not the first diagnostic for a UI hit-target issue.

### Why it matters

- Android right-to-left layout can mirror the position of a switch, so its visual position and touch target must be distinguished from an unavailable setting or failed license restoration.
- In a confirmed support comparison, a setting that seemed inaccessible after changing phones became usable when the system language was switched to English. That isolates a possible layout/touch interaction; it does **not** establish which QML component causes it.

## Android TalkBack or Voice Access cannot edit or search settings reliably

### Provide first

- Device and Android version, exact QZ build (including test APK if relevant), Classic or Modern interface if known, and the assistive technologies in use (TalkBack, Voice Access, or both).
- Name the exact field or screen (settings search, a text field, or a confirmation button). State the spoken label and announcement before and after activation with TalkBack's double-tap; distinguish silence from an announced editable field.
- Identify the input method: onscreen keyboard, Voice Access commands, or a physical Bluetooth keyboard. For a physical keyboard, report its model, language/layout and whether the field was activated before typing.
- Describe separately whether text enters correctly, search results update, editing can be ended, and Save/OK produces audible confirmation or a persistent setting change. Use harmless sample text rather than account details.

### If still unclear

- Compare the same input method in one other QZ field, if accessible, and report the exact action or spoken command and the observed result. Do not assume a keyboard failure in one field occurs in all fields.
- For unidentified elements, report their spoken label, role, and the preceding/following controls in TalkBack focus order. A step-by-step account is sufficient; do not require screenshots or require assistive technology to be disabled.
- If already back on a working build, identify the earlier test build and describe the behavior from memory rather than requiring reinstallation solely to answer intake questions.

### Why it matters

- Focus, editable-field accessibility semantics, physical keyboard input, Voice Access actions, search filtering and spoken save confirmation are separate stages. QZ's QML UI exposes accessible roles and actions, so the exact control and activation method matter more than a Bluetooth fitness-machine log.
- In a reviewed Android case, the search and text-field problems were reproduced by another tester; the physical keyboard issue still needed a separate comparison. The reported keyboard layout, interface and activation gesture made that investigation more specific without assuming a fix.
