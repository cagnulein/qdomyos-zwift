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
