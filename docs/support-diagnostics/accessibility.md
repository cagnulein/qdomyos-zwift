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
