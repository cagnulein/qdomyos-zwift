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

## Bluetooth virtual trainer works, but Wahoo DIRCON / Wi-Fi pairing fails

### Provide first

- The physical trainer model, QZ platform/build, the receiving app and device, and whether both devices are on the **same local network**.
- Confirm that QZ already receives live power/cadence and that pairing its **Bluetooth** virtual trainer works, if tested.
- Screenshot **Experimental Settings > Wahoo DIRCON** and identify whether that option is enabled. Record the current **Virtual Device** settings as well.
- Screenshot the receiving app's Wi-Fi/network trainer list, including the advertised name and the exact stage of failure (not listed, listed but rejected, connected with no metrics, or disconnected later).

### If still unclear

- If Wahoo DIRCON was disabled, enable it, fully restart QZ, and repeat only the network discovery/pairing test. Note whether the receiving app now sees or accepts the trainer.
- Confirm the network path separately from Bluetooth pairing; check whether the receiving app has a remembered/old trainer entry before interpreting a rejected connection.
- Collect a QZ debug log spanning virtual-device startup and the network pairing attempt, with the approximate failure time and the receiving app's message. Avoid changing trainer resistance or gear settings during this comparison.

### Why it matters

- QZ's DIRCON manager is conditional on the **Wahoo DIRCON** setting; a working Bluetooth virtual trainer does not prove that the Wi-Fi trainer service is active.
- In a reviewed support log, the network trainer appeared in the receiving app but pairing failed while DIRCON was disabled, making the option value a high-value first check.
- Device discovery, pairing acceptance, and metric streaming are separate stages. See the [integration FAQ](../faq/integrations.md) for supported DIRCON use cases.

## Physical treadmill disconnects shortly after enabling QZ's virtual treadmill on iOS

### Provide first

- Exact treadmill and console model, advertised Bluetooth name, iPhone/iPad model and iOS version, and the **installed QZ build number** (not just the store version).
- Confirm whether QZ reads **speed and incline continuously with Virtual Device disabled**, then whether the physical connection drops when **Virtual Device** is enabled. Record the elapsed time and whether QZ's session timer or Start state also changes.
- Identify **all Bluetooth participants**: QZ phone/tablet, treadmill, receiving app (such as Runna), heart-rate strap/watch, remote control, and any manufacturer app. State which device connects to which, and the exact order in which they are started.
- State whether the treadmill itself automatically connects to nearby Bluetooth heart-rate sensors or remotes, even when QZ has not selected them. Report whether the failure changes when those accessories are already connected before QZ starts.

### If still unclear

- Run two short, comparable tests with the **same startup order** and receiving app closed: Virtual Device off, then on; fully restart QZ between tests. Record whether physical speed/incline continues updating for at least two minutes. Do not change Garmin Companion or other options during this comparison.
- If the failure occurs only with virtual advertising, capture screenshots of the relevant Experimental virtual-device settings and heart-rate/virtual Bluetooth options. Compare one additional setting at a time **only when supported by the installed build**, keeping the FTMS virtual profile requirements of the receiving app in mind.
- Supply QZ debug logs for both tests, including the approximate disconnection time and whether the physical BLE connection or virtual advertisement failed first. If the problem depends on an HR strap or remote, report the accessory connection order for each test.
- After the physical link is stable, test the receiving app separately and report whether it discovers the virtual treadmill as a **controllable treadmill**; stable physical telemetry alone does not establish that the virtual FTMS service remains available.

### Why it matters

- A treadmill may act as both a BLE peripheral for QZ telemetry and a BLE central for its own accessories. Knowing its automatic HR/remote pairing behavior helps distinguish competing Bluetooth connections from a generic iOS disconnection.
- In a reproduced comparison, the physical treadmill link was stable without virtual advertising and also stayed connected when its accessory connections were already occupied. This is useful evidence of an advertising/connection interaction, **not proof** that the treadmill connected to QZ's virtual heart-rate service.
- Some compatibility settings alter the **advertised virtual services**, not merely the physical link. A test that stabilizes the physical treadmill but removes virtual FTMS may prevent the receiving app from controlling it; evaluate those outcomes separately.
- Paired A/B logs and a single changed option distinguish a repeatable interaction from unrelated backgrounding, startup timing, or app pairing differences.

## Garmin watch sees QZ's virtual running sensor but pairing times out

### Provide first

- Garmin watch model, firmware version if known, QZ iPhone/iPad model and iOS version, and the installed QZ build.
- Confirm that treadmill speed updates **inside QZ** before attempting watch pairing. Identify the treadmill model and whether the watch is recording a **Treadmill** activity rather than using QZ's completed-workout upload.
- Screenshot QZ's **Virtual Device**, **Virtual Device Bluetooth**, and **Garmin Options > Garmin Bluetooth Sensor** selections. State whether QZ was fully restarted after changing them.
- Describe the watch's **Sensors & Accessories** search result: is the QZ running sensor absent, visible but timing out, paired but not supplying speed, or dropping later? Include the exact error or a screenshot.
- State whether the Garmin watch's **Phone Connection** to the same iPhone was enabled during the attempt. Record the result of one comparison with only that watch-phone connection temporarily disabled; keep the iPhone's Bluetooth enabled.

### If still unclear

- Record the exact pairing order: QZ startup, treadmill connection, watch sensor search, and start of the watch's Treadmill activity. Note any previously saved failed QZ sensor entry and whether it was removed before retrying.
- Check whether the watch actually receives running speed/distance during a short treadmill activity, not merely whether sensor pairing reports success. Restore the watch's phone connection after the test.
- If it still fails, provide a short QZ debug log spanning virtual-sensor startup and the pairing attempt, plus the watch's sensor screen. Change no other virtual profiles during the comparison.

### Why it matters

- Live treadmill telemetry, QZ's Bluetooth RSC advertisement, Garmin sensor pairing, and activity recording are separate stages. The first failed stage determines which evidence is useful next.
- A confirmed watch-pairing timeout was resolved by temporarily disconnecting the watch's phone link while leaving the iPhone's Bluetooth on. This comparison helps distinguish competing phone connectivity from a missing virtual RSC advertisement, without assuming the same cause for every Garmin model.
- Garmin's Bluetooth running sensor is different from its cycling power sensor and from uploading a completed FIT activity. See the [Garmin integration FAQ](../faq/integrations.md) for supported setup guidance.
