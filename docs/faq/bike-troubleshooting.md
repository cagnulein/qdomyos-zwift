# Bike and trainer troubleshooting

## QZ detects my bike, but the bike metrics stay at zero. What should I try?

If QZ recognizes the bike but cadence, power, speed, or other bike data are not updating while other sources such as heart rate still work:

1. Open **QZ Settings > Garmin options**.
2. Disable **ANT Bike Garmin**.
3. Fully close and restart QZ.
4. Reconnect the bike and start pedaling.

This can restore the normal bike data stream when the Garmin ANT bike option interferes with the connection.

### Why can incline still show zero?

Bike resistance and incline are different values. A Yesoul bike may report resistance without reporting a real incline value. QZ's incline tile can instead be populated by an external source such as Zwift or a GPX route. A resistance-to-incline conversion requires a specific mapping and should not be assumed from the resistance percentage alone.

## Zwift auto-resistance is working, but flat resistance or gradient changes feel wrong. Which settings should I adjust?

Use **Zwift Resistance Offset** to set the baseline and **Zwift Resistance Gain** to change how strongly resistance reacts to Zwift gradients. Do not use **Min. Resistance** as the flat-road baseline.

For example, if a bike feels right around resistance 50 on flat ground:

1. Set **Zwift Resistance Offset** to the desired flat-road baseline, such as 50.
2. Leave **Min. Resistance** at 0 unless you intentionally want to prevent QZ from ever requesting resistance below a specific value.
3. If climbs change resistance too little, increase **Zwift Resistance Gain**. If they change it too aggressively, reduce the gain.
4. Leave **Zwift Workout/Erg Mode** disabled for normal free rides; enable it when using a Zwift workout that requires ERG behavior.
5. Restart QZ after changing the configuration and test again.

QZ applies a gain and offset to the grade received from the client application, so the offset changes the baseline while the gain scales the response to gradient changes. In a confirmed support case, separating the minimum-resistance limit from the Zwift offset restored the target resistance, and increasing Zwift Resistance Gain then produced the desired stronger response to climbs.

## QZ cannot connect reliably to my bike even though the bike is supported. What else should I check?

Check whether another nearby phone, tablet, watch, training app, or companion app is already connecting to the bike over Bluetooth.

Many fitness devices accept only one active Bluetooth connection for a given service. A device you are not actively using may reconnect automatically in the background and prevent QZ from taking the connection.

To test this:

1. Fully close other fitness apps that know the bike.
2. Temporarily disable Bluetooth on nearby phones or tablets that have previously paired with or used the bike.
3. Power-cycle the bike if necessary.
4. Start QZ first and let it connect before opening the training app that will connect to QZ.

In a confirmed support case, an unnoticed second device was connecting to the bike; removing that competing connection restored QZ connectivity.

## My bike loses resistance control or virtual shifting after a minute, but power and cadence still work. What should I check?

A bike may initially work in QZ and a training app (such as MyWhoosh or Zwift), then stop responding to gradient/resistance or virtual-gear changes after a short time **even while watts and cadence continue updating**. This is not necessarily a complete Bluetooth disconnection. One possible cause is a competing or unwanted Bluetooth heart-rate connection.

To isolate this kind of problem:

1. **Test QZ without the training app.** Connect QZ to the physical bike, leave MyWhoosh/Zwift/other training apps closed or disconnected, and check that QZ can still change the bike's resistance after several minutes. If QZ alone is stable, focus next on how the training app pairs with QZ; if it fails even alone, investigate the direct bike-to-QZ connection instead.
2. In **QZ Settings > Heart Rate Options**, enable **Disable HRM from the machinery** while testing. This prevents QZ from using heart-rate readings supplied by the bike.
3. Keep **Heart rate service outside FTMS** disabled in QZ for the test, if that option is available.
4. In the training app's device-pairing screen, **disconnect or unpair the heart-rate sensor** associated with QZ/the bike. Keep QZ paired as the **power/cadence and controllable trainer** so that resistance control continues to use QZ. If you need heart rate, use an independent HR source supported by your setup instead of adding back the suspected conflicting pairing during the test.
5. Reconnect, ride beyond the time when control usually fails, and verify **both** that live metrics still update and that virtual shifting/gradient resistance still work.

**Confirmed short test:** In one two-Android-device setup with a Domyos Training Bike 900 and MyWhoosh, resistance control and virtual shifting stopped after roughly one minute while power and cadence still arrived. QZ controlling the bike alone remained stable. With the HR-related settings and MyWhoosh heart-rate pairing changed as above, the rider reported normal control after approximately ten minutes. A full-length workout had **not yet been confirmed**, and several settings were changed together, so this does **not** prove a single root cause or guarantee that the workaround applies to every bike.

If control still drops, note whether the **QZ-to-bike** connection or only the **training-app-to-QZ control path** is affected, check for other devices/apps connecting to the bike, and collect a QZ debug log covering the failure.

## QZ keeps finding or searching for the wrong nearby fitness device. How can I force the intended trainer or bike?

If several supported Bluetooth fitness devices are nearby and QZ selects or keeps searching for the wrong one, use the **Manual Device** setting to restrict discovery to the device you actually want to use.

1. Open **QZ Settings > Advanced Settings**.
2. Set **Manual Device** to the Bluetooth name of the intended bike or trainer.
3. Fully close and restart QZ, then let it reconnect.

QZ stores this selection as its device filter and uses it instead of unrestricted automatic discovery. To return to automatic device selection later, set **Manual Device** back to **Disabled**.

In a confirmed support case, QZ was searching for a different nearby fitness machine instead of the intended smart trainer. Selecting the trainer under **Manual Device** restored the expected connection.

## MyWhoosh connects to QZ for my Echelon bike, but power stays at 0 W. What should I check?

If the Echelon bike itself is already working normally in QZ and MyWhoosh can see/connect to the QZ device but the workout data remains at zero, check whether **Virtual Echelon** is still enabled from a previous setup or unlock attempt.

1. Open **QZ Settings > Experimental settings**.
2. Disable **Virtual Echelon**.
3. Fully close and restart QZ.
4. Reconnect the Echelon bike, then pair QZ again in MyWhoosh.

**Virtual Echelon** is intended for the Echelon initialization/unlock workflow. It is not normally needed once the bike can connect and operate directly through QZ. In a confirmed support case, disabling this setting and restarting QZ immediately restored data transmission to MyWhoosh.

If the bike itself is locked and QZ explicitly asks you to enable Virtual Echelon for initialization, follow the on-screen unlock instructions first; this troubleshooting step applies when the bike is already operating normally in QZ but the downstream training app receives no metrics.

## My Echelon bike is locked and QZ cannot start it directly. Can I unlock it with a second device?

Yes. For Echelon bikes that require the manufacturer's initialization before they will expose normal workout data, a practical workaround is to use a second phone or tablet only to unlock the bike, then let QZ take over.

1. Keep QZ ready on the primary device, but do not let another training app connect directly to the bike.
2. On a second phone or tablet, use an Echelon-compatible unlock app to connect to and initialize the bike.
3. Once the bike is unlocked, disconnect or close the unlock app so it releases Bluetooth.
4. Start or reconnect QZ on the primary device and verify that cadence, resistance, and power are updating.
5. Connect Zwift, MyWhoosh, ROUVY, or another training app to QZ's virtual device rather than directly to the physical bike.

The unlock step may need to be repeated after the bike is power-cycled. In a confirmed support case with an Echelon EX-7s, this two-device sequence reliably unlocked the bike and QZ then operated normally.

If QZ already connects to and initializes your bike directly, this workaround is unnecessary.

## My ProForm TDF bike used to connect over Wi-Fi, but QZ suddenly stays on Connecting. What should I check?

QZ's Wi-Fi connection to a ProForm TDF bike uses the bike's configured IP address. If the router assigns the bike a different address through DHCP, QZ can keep trying the old address and no longer connect even though nothing changed in QZ.

1. Check the bike's current IP address on the bike or in your router's connected-device list.
2. Compare it with the **TDF IP** value configured in QZ and update QZ if the address has changed.
3. Restart QZ and reconnect.
4. To prevent the problem from recurring, reserve a fixed DHCP address for the bike in your router, or otherwise configure the router so the bike always receives the same IP.

In a confirmed support case, the bike's IP address had changed; updating the address restored the QZ connection immediately.


## My bike does not have native ERG mode. How can I improve QZ's ERG emulation?

For supported bikes where QZ has to emulate ERG by selecting resistance levels, QZ learns the relationship between **cadence, power, and resistance** and uses that data to choose a resistance for the requested target power.

If ERG is working but the resistance changes are too coarse or QZ jumps between unexpected levels, spend some time building a better power curve:

1. Use a normal free ride with QZ running.
2. Change resistance manually from the bike's own controls/display.
3. Pedal through the resistance levels you normally use for ERG workouts.
4. Stay on each resistance level for roughly 30 seconds at a cadence similar to the cadence you normally use in ERG mode.
5. Repeat across a useful range of resistance levels and power outputs.

You do not need Zwift open during this learning phase; QZ is the component collecting the cadence/power/resistance relationship. You also do not need to run Zwift's trainer calibration for this QZ-emulated ERG setup. If Zwift's calibration screen waits for the device, it can be ignored for this workflow.

The more representative data QZ has around your normal cadence and power range, the better it can estimate which resistance level should produce a requested wattage. On bikes with discrete resistance levels, some jumps can still be unavoidable because an exact target power may fall between two available levels.

In a confirmed support case, QZ's ERG emulation was working on a bike without native ERG; the recommended learning procedure was then used to improve the resistance selection for ramp workouts.

## MyWhoosh cannot discover QZ's virtual trainer on an Echelon console. Which virtual-device options should I check?

If QZ is already connected to an unlocked Echelon bike and its own metrics work, but MyWhoosh on another device cannot see QZ as a trainer, check for special-purpose virtual-device options left enabled in **Experimental Settings**.

1. Keep the normal **Virtual Bluetooth Device** enabled.
2. Disable **Virtual iFit**, **Virtual Echelon**, and **Virtual Only Heart Rate** if they were enabled for previous experiments or setup procedures.
3. Fully close and restart QZ, reconnect it to the physical bike, and then rescan for QZ's virtual trainer from MyWhoosh.
4. Pair MyWhoosh with the QZ virtual trainer rather than directly with the Echelon bike.

In a confirmed Echelon EX-4S+ support case, disabling Virtual iFit and Virtual Echelon alone did not restore discovery. After **Virtual Only Heart Rate** was also disabled, MyWhoosh immediately recognized QZ. This does not isolate a single setting as the cause; the working configuration had all three special-purpose modes disabled.

**Exception:** If your bike still requires the **Virtual Echelon** unlock handshake, complete that separate unlock procedure first. These steps apply when the bike is already unlocked and QZ needs to advertise a normal trainer to MyWhoosh.

## QZ connects to my devices, but the dashboard tiles have disappeared. How do I restore them?

If QZ opens to its main screen without the expected speed, cadence, power, or gear tiles, try running QZ's initial **setup wizard** again to restore the dashboard layout. After completing the wizard, return to the main screen and check that the tiles have reappeared before investigating the bike connection or virtual gearing.

A support user explicitly confirmed that rerunning the wizard restored missing tiles while QZ was still detecting the connected Wahoo devices. This is a dashboard-layout issue, not evidence that the trainer itself has lost Bluetooth connectivity.

## My Wahoo KICKR is detected but will not connect or provide metrics. Could an old Bluetooth pairing be involved?

An old system-level Bluetooth pairing can leave stale service information on a phone. In addition to checking for competing fitness apps or nearby phones:

1. Close QZ and other training apps.
2. In the phone's Bluetooth settings, find the trainer's saved pairing, if present, and choose **Forget/Unpair**.
3. Power-cycle the trainer and reopen QZ to let it discover and connect again.
4. If the issue persists, try QZ with the trainer on a second phone to distinguish phone-specific Bluetooth cache behavior from a trainer problem.

A KICKR CORE support user confirmed that connectivity worked again after the pairing/cache troubleshooting. The reply did not isolate which check was decisive, so this is a diagnostic workaround, not a guaranteed fix.

## My Thinkrider X5 shows virtual gear changes in Zwift but the trainer does not change resistance. What should I check?

First confirm QZ receives the trainer's metrics and that Zwift pairs **QZ** as the controllable/resistance device, not just as a power source.

In a confirmed Android support case, the successful troubleshooting instructions were to update QZ to a current **Play Store beta** build, set **FTMS Bike** under **QZ Settings > Bike Options** to **Disabled** if a device was manually forced there, restart QZ, and test again. The rider confirmed resistance control worked afterward. Since the two changes were made together, the thread does not isolate which one fixed the problem. Do not disable FTMS indiscriminately on unrelated trainers.

## Why can Schwinn IC4 resistance in QZ differ from the console?

The Schwinn IC4 is supported by QZ. Its QZ driver receives cadence and power over Bluetooth and uses those readings to calculate a resistance estimate. That estimate can vary with pedaling effort even when the physical resistance knob has not moved.

Under **Bike Options**, QZ offers Schwinn resistance conversion variants and a resistance smoothing setting. Compare the QZ and console values at a steady cadence before adjusting the mapping. These settings change the reported resistance representation, not the physical knob calibration.

**Watt Gain** and **Watt Offset** can adjust power reported by QZ but cannot calibrate the bike's own power sensor. The **speed calculated from power** setting can also make QZ speed and distance differ from the console. These behaviors are verified in QZ's Schwinn IC4 driver and metric settings; no customer-specific calibration fix was confirmed.

## My ERG target power keeps falling by about 5 W while I pedal. Could Automatic Virtual Shifting be responsible?

Yes. **Automatic Virtual Shifting** can change QZ's virtual gear when cadence crosses its configured thresholds. In a standalone ERG/power-target setup, repeated downshifts can also reduce the **target power** in small steps. This is different from normal instantaneous power fluctuations while pedalling.

1. In **QZ Settings**, find **Automatic Virtual Shifting** and temporarily disable it. Restart QZ if needed.
2. Enable the **Target Watt / Target Power** dashboard tile under **Tiles Options**. Set a target (for example, 100 W), then compare the displayed target while stopped and while pedalling.
3. If the target previously fell repeatedly during pedalling but now stays at the requested value, review the automatic-shifting cadence thresholds before turning the feature back on.
4. Test third-party ERG or terrain control separately; fixing local automatic shifts does not prove that MyWhoosh or another app is sending power/resistance commands.

**Confirmed result:** One rider's low cadence (about 40–55 RPM) stayed below an automatic downshift threshold of 65 RPM, triggering repeated downshifts and roughly 5 W reductions until the target reached zero. After correcting the unwanted Automatic Virtual Shifting configuration, the rider reported QZ working. The cadence thresholds and feature enablement are also present in QZ's settings code.
