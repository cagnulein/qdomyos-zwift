# Virtual device settings diagnostics

## Training application cannot discover QZ

### Provide first

- Confirm that live machine metrics update in QZ.
- Identify the device running QZ and the device running the training application.
- Share a screenshot of QZ Experimental Settings showing whether **Virtual Device** and **Virtual Device Bluetooth** are enabled, along with any other enabled virtual modes.
- State which pairing role the receiving app is scanning (**Power Source / FTMS trainer**, Speed Sensor, or another role), and whether it sees the **physical sensor** but not QZ's virtual trainer.
- Check whether the expected QZ virtual trainer advertisement (which may use a trainer name rather than the phone name) appears in a Bluetooth scan from the receiving device.

### If still unclear

- If the virtual Bluetooth device was disabled, enable it, fully restart QZ, and repeat the receiving app's **Power Source / trainer** scan. Record whether the virtual trainer appears while QZ still receives physical sensor data.
- If advertising still fails, compare the scan before and after disabling unused virtual modes, one option at a time.
- If no QZ advertisement appears, provide a short debug log from a session with the machine connected.
- On Android, check the existing FAQ about device name length and Bluetooth advertising.

### Why it matters

- A working physical machine or cadence-sensor connection does not establish that **Virtual Device Bluetooth** is enabled or that the correct virtual profile is being advertised. In a confirmed two-device setup, enabling the virtual device restored discovery in MyWhoosh.
- The receiving app seeing the physical sensor under Speed Sensor is not evidence that it can discover QZ's virtual trainer under Power Source.
- The selected virtual modes determine which protocol the receiving application can discover.
- A receiving-device scan separates Bluetooth advertising issues from the training application's device filters.

## Physical bike disconnects only while QZ advertises a virtual Bluetooth device

### Provide first

- Bike model, QZ device/Android version, QZ version, and whether the physical connection and metrics recover automatically.
- Whether Virtual Bluetooth is enabled and whether the receiving training app is running during the disconnect.
- Time from initial bike connection to disconnection, and whether the problem repeats at a similar interval.

### If still unclear

- Capture two short comparable QZ debug logs: Virtual Bluetooth enabled and disabled, with the same physical bike and no other settings changed.
- Record the approximate disconnect timestamp and whether power/cadence/resistance resume afterward.
- If Android battery optimization is suspected, report the current battery restriction setting; change it separately from the virtual-device comparison.

### Why it matters

- A repeatable disconnection only while virtual advertising is enabled distinguishes an interaction with the Bluetooth stack from a failed bike authentication or a general sensor problem.
- In a confirmed comparison on an older Android tablet, physical-bike connectivity remained stable with virtual advertising disabled, while direct bike authentication worked in both configurations.

## Rower metrics work in QZ but Zwift reports no signal

### Provide first

- Rower and console model, QZ version, and the devices running QZ and Zwift.
- Confirm whether QZ displays live rowing metrics before Zwift starts.
- Screenshot the virtual-device options, especially **Virtual Rower** under Experimental Settings, and the device or pairing role visible in Zwift.

### If still unclear

- Check whether Zwift sees QZ's virtual trainer advertisement rather than the physical rower or only the phone name.
- If the intended Zwift pairing mode uses QZ's virtual bike, disable **Virtual Rower**, restart QZ, and retry the same pairing sequence.
- If the device is still not usable, provide a short QZ debug log from the failed pairing and the exact Zwift message.

### Why it matters

- The selected virtual profile determines what Zwift can pair with, even when QZ receives correct data from the rower.
- In a confirmed rower setup, disabling Virtual Rower and restarting QZ resolved Zwift's no-signal pairing problem.

## Echelon bike needs the manufacturer app to start, or loses resistance control after reconnecting

### Provide first

- Exact Echelon bike/console model, advertised Bluetooth name, QZ platform and **installed build**.
- Whether QZ itself shows changing cadence/resistance and can change the bike's **physical resistance** before Peloton, Zwift, or another app is opened.
- Screenshot of **Experimental Settings > Virtual Echelon**, plus the devices running QZ, the Echelon app, and the receiving training app.
- Describe the **startup and reconnect order**, including whether the bike was power-cycled and whether opening the Echelon app on another device was necessary to unlock resistance.

### If still unclear

- Check whether the installed QZ build supports direct initialization for that bike; do not assume all Echelon models behave alike.
- With no training app connected, compare one short QZ-only restart with the existing **Virtual Echelon** setting and, where direct initialization is supported, another with it disabled. Record whether QZ gets live metrics **and** physical resistance control after each restart.
- If neither sequence works, request a QZ debug log spanning bike discovery, initialization, and one **small, safe** resistance change; note the time of any manufacturer-app connection.

### Why it matters

- The manufacturer-app unlock path and QZ's direct bike connection are different workflows. A successful unlock once does not prove resistance will initialize after the next power cycle.
- In a confirmed case, a newer QZ build with **Virtual Echelon** disabled restored standalone bike operation without first opening the manufacturer's app. The setting and build are therefore important evidence, not universal instructions.
- Testing physical resistance in QZ before the receiving app separates bike initialization from downstream virtual-device pairing. See the [Echelon troubleshooting FAQ](../faq/bike-troubleshooting.md) for the applicable setup paths.
