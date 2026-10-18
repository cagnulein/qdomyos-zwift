# Treadmill control diagnostics

## QZ reads treadmill metrics, but some commands do not work

### Provide first

- The full treadmill model number, advertised Bluetooth name, QZ platform/version, and selected Specific Model option.
- Which actions work from the treadmill's physical controls, from QZ, and from the third-party training app? Check start, stop, speed, and incline separately.
- One concrete expected-versus-observed example for a failing command, including any delay.
- Confirm that QZ manual control was tested before troubleshooting commands from a training app.

### Sportstech F75 and Sportstech/ZY treadmills with an Android console

Sportstech F75 units with an integrated Android panel may use the proprietary
ZY-Treadmill serial controller instead of FTMS Bluetooth. QZ must use the
model-specific **Sportstech F75 Android serial** profile; selecting a generic
FTMS treadmill profile is not equivalent.

#### Before testing

1. Close QZ.
2. Stop the manufacturer's app so it cannot keep the UART open. For the F75
   profile the verified package is `com.zhongyang.treadmill`:

   ```cmd
   adb shell am force-stop com.zhongyang.treadmill
   adb shell pm disable-user --user 0 com.zhongyang.treadmill
   adb shell pidof com.zhongyang.treadmill
   adb shell pidof com.zhongyang.treadmill:remote
   ```

   The last two commands must return no PID. `pm disable-user` alone does not
   prove that an already-running process has exited.
3. Open QZ and enable **Settings > Treadmill Options > Sportstech F75 Android
   serial**. The F75 driver uses its fixed Android UART; do not enter a
   guessed serial-port path.
4. Test one action at a time: Start, Pause/Resume, Stop, speed, and incline.
   Do not run QZ and the manufacturer's app concurrently.

#### Expected F75 behavior

- Physical Start toggles Start, Pause, and Resume through QZ.
- Physical Stop ends the QZ workout and the normal belt stop is sent as a
  gradual speed ramp rather than an immediate speed drop.
- The F75 emergency-stop event currently mapped to serial key `0x69` causes an
  immediate QZ stop and `FB 00` command. This interpretation comes from the
  available capture and should be confirmed with an isolated debug; always
  treat the treadmill's physical safety mechanism as authoritative.
- Incline+ changes the target by one supported step.
- The treadmill's safety/emergency stop remains the emergency mechanism; do
  not use a software ramp as a substitute for it.

QZ distinguishes commanded values from independent feedback. During a
profile that has no validated non-zero telemetry, a speed or incline visible in
QZ can be the last requested command rather than a measurement from the
console. A physical reaction proves that a command reached a controller; it
does not by itself prove that the displayed value is independent feedback. The
F75 profile integrates the displayed session distance from that commanded
speed because the serial protocol does not provide an independent odometer
frame, and caps the requested/serialized speed at 20 km/h.

#### Unlocking an experimental APK

If support grants access for a test build, enter the exact granted email under
**QZ > Settings > General > Email**, save it, and restart QZ. Install/update
the supplied APK, keep the manufacturer's app stopped, enable the F75 serial
profile, and then repeat the controlled test. An allowlist grant unlocks QZ
access to the test build; it does not create a Patreon subscription or unlock
another manufacturer's app.

#### Other Sportstech/ZY Android treadmills

The same ownership and testing rules apply to other Sportstech models with an
Android console, but the UART path, baud rate, frame layout, telemetry, button
codes, and vendor package may differ. Do not reuse the F75 profile merely
because the console looks similar. Collect the exact model number and vendor
package, keep the official app closed during QZ testing, and provide one
annotated log for each control action if the behavior is not supported.

For Windows captures, use `cmd.exe` from the folder containing `adb.exe`,
verify `adb devices` first, clear logcat before launching QZ, and record the
whole test from QZ launch through the single physical action. Include the QZ
version, selected profile, exact model, and whether the vendor process checks
returned empty.

### If still unclear

- Record a short QZ debug log with one clearly identified control action at a time and note the approximate time.
- For a proprietary treadmill model whose control commands remain unknown, support may request a separate Bluetooth capture from the manufacturer's app. Keep QZ closed during that capture and perform distinct start, stop, speed, and incline actions.
- On iOS, a Bluetooth logging profile and a sysdiagnose may be required for the manufacturer-app capture. Collect this only when requested; diagnostic archives can contain sensitive device data.

### Why it matters

- Receiving speed and incline telemetry does not prove that QZ has the correct command sequence for that exact treadmill revision.
- A physical start or stop button is not equivalent to a remote command from QZ or the manufacturer's app.
- Separating individual actions and comparing QZ logs with native-app captures helps identify missing model-specific commands without confusing the data path with the control path.
