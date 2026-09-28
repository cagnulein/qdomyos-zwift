# Integrations and authentication

## Can I use QZ to get treadmill data into Garmin without Zwift or another training app?

Yes. There are two different approaches depending on whether you want the Garmin watch to receive the treadmill metrics live or you only want the completed activity in Garmin Connect.

### Option 1: send speed and distance live to a Garmin watch over ANT+

On Android, QZ can connect to a supported treadmill over Bluetooth/FTMS and retransmit treadmill speed and distance using the **ANT+ SDM/footpod profile**.

Your Garmin watch can then pair with QZ as a footpod or speed/distance sensor and record the treadmill activity directly. Zwift, Kinomap, and similar training apps are not required for this setup.

The Android device running QZ must be able to **transmit ANT+**. If the phone or tablet does not have usable native ANT+ support, a compatible USB ANT+ dongle with the appropriate USB/OTG adapter can be used. Make sure the dongle supports transmission, not only reception.

On Android devices that rely on an external USB ANT+ stick, the ANT stack may also require the **ANT Radio Service** and **ANT USB Service** components. If Android does not recognize the stick for ANT+ use, install those two ANT service components, reconnect the stick through USB/OTG, and restart QZ. In a confirmed support case, an Android phone could then use QZ to broadcast trainer/power/speed data that became visible on a Garmin device.

### Option 2: upload the completed QZ workout to Garmin Connect

If you do not need the Garmin watch to receive the treadmill data live, QZ can record the workout itself and upload the generated **FIT activity** to Garmin Connect.

In this setup the watch does not need to be paired to QZ as an ANT+ sensor. Configure the Garmin Connect integration in QZ and use QZ to record the session; the completed activity will be pushed automatically to Garmin Connect and it will count as training effect and Vo2Max.

Choose **ANT+ SDM/footpod** when you want the watch itself to see and record the treadmill speed/distance during the workout. Choose **Garmin Connect FIT upload** when your main goal is simply to have the finished activity and its metrics in Garmin Connect.

## Can I use a basic indoor bike with a Bluetooth cadence sensor in MyWhoosh through QZ?

Yes. QZ can use a compatible Bluetooth cadence sensor as the bike data source and expose a virtual bike/trainer that MyWhoosh can connect to.

1. Open **QZ Settings > Accessories** and select the Bluetooth sensor under **Cadence sensor**, then confirm the selection.
2. Enable the option that uses the **cadence sensor as a bike**.
3. Confirm that cadence is updating in QZ before opening MyWhoosh.
4. Run MyWhoosh on a separate device and pair the virtual QZ trainer.

When MyWhoosh lists the virtual QZ devices, select **Wahoo KICK 0000** as the **Power Source**. If MyWhoosh also shows a separate **Controllable/Trainer** field, select **Wahoo KICK 0000** there as well.

**Wahoo HRM** is only QZ's virtual heart-rate device, so select it only in MyWhoosh's heart-rate field if you also want QZ to forward heart rate.

In a confirmed support case, selecting the external cadence sensor in QZ restored cadence immediately; the `cadence_sensor_as_bike` setting is also part of QZ's current device-discovery configuration.

## Can I use a Bluetooth power meter or trainer power service as the bike source in QZ?

Yes. If the Bluetooth device is fundamentally being detected as an external power sensor rather than as a complete bike, QZ can promote that sensor to the bike data source.

1. Select the device as the **Power Sensor** in QZ.
2. Enable **Power Sensor as a Bike**.
3. If the same physical device is also selected under **Bike Options > FTMS Bike**, set **FTMS Bike** to **Disabled** so QZ does not also initialize it through the FTMS-bike path.
4. Confirm that power and the available sensor metrics are updating on the QZ main screen before opening the downstream training app.
5. Then pair the virtual QZ bike/trainer in the training app instead of pairing the physical power sensor directly there.

The `power_sensor_as_bike` option is part of QZ's current device-discovery configuration and changes how a selected power sensor is treated by QZ. This is useful for setups where the physical device provides usable cycling power data but is not otherwise selected as QZ's primary bike.

If a trainer or smart roller suddenly applies unexpectedly high physical resistance as soon as QZ connects, check this configuration before changing resistance mappings. Do not configure the same device simultaneously as an FTMS Bike and as a Power Sensor used as a bike. Keep it selected as the **Power Sensor**, enable **Power Sensor as a Bike**, disable **FTMS Bike**, and restart QZ. In a confirmed support case with smart rollers, this removed the unexpected resistance applied at connection time.

In another confirmed support case, QZ could see the physical trainer/power source but was not exposing the expected bike data path until **Power Sensor as a Bike** was enabled; after enabling it, the setup began working.

## Can I use an Apple Watch for treadmill cadence while taking heart rate from a chest strap?

Yes. QZ can receive cadence from the QZ Apple Watch app while the watch supplies heart rate from a Bluetooth chest strap paired directly to the Apple Watch.

For example, with a smart treadmill, Apple Watch, and a Bluetooth heart-rate strap:

1. Pair the heart-rate strap with the **Apple Watch** in the watch Bluetooth settings.
2. Run the QZ app on the iPhone and the QZ companion app on the Apple Watch during the workout.
3. The Apple Watch can provide treadmill cadence to QZ, while the heart-rate value used by the watch comes from the paired external strap.
4. QZ can then record the treadmill metrics together with the cadence and heart-rate data.

You do not need to pair the chest strap separately to QZ for this setup. QZ's current treadmill code includes Apple Watch cadence handling, and its external heart-rate path explicitly supports Apple Watch heart-rate data.

## Can QZ bridge a trainer and heart rate to Zwift on Apple TV over Wi-Fi so Bluetooth remains available for Zwift Click?

Yes. QZ supports the Wahoo DIRCON-compatible Wi-Fi virtual-trainer path. This can be useful on Apple TV when you want Zwift to receive the trainer data and heart rate through QZ over the local network while keeping Apple TV's Bluetooth connection available for a Zwift Click controller.

A typical setup is:

1. Connect the physical trainer and heart-rate sensor to QZ.
2. Enable QZ's **Wahoo DIRCON / Wi-Fi** compatibility option.
3. Make sure the device running QZ and the Apple TV are on the same local network.
4. In Zwift on Apple TV, select the virtual QZ/Wahoo trainer exposed over Wi-Fi for the trainer connection and use the heart-rate data forwarded by QZ.
5. Pair the Zwift Click controller directly to Zwift on the Apple TV over Bluetooth.

This avoids using separate Apple TV Bluetooth connections for both the physical trainer and heart-rate sensor. QZ's current virtual-bike implementation includes DIRCON support, and the project documents DIRCON as a Wi-Fi-only path for sending QZ data to Zwift.

## Can I use Zwift Ride virtual gears with Kinomap through QZ?

Yes. QZ can use the Zwift Ride controllers for shifting while Kinomap connects to QZ as the virtual trainer.

For this setup:

1. Enable **Zwift Play** in **QZ Settings > Accessories**.
2. Wake the Zwift Ride controllers before starting QZ so QZ can discover them.
3. In **Bike Options**, keep **FTMS Bike** set to **Disabled** unless you specifically need to force a particular FTMS bike implementation.
4. Start QZ and verify that the controllers are connected, then pair the QZ virtual trainer in Kinomap.

If the controllers appear connected but gear changes do not work in Kinomap, check **FTMS Bike** first. Selecting a trainer model there can make QZ use the wrong bike path for this setup. In a confirmed support case, changing **FTMS Bike** from a KICKR model back to **Disabled** restored virtual shifting immediately.

## Can I use QZ virtual gears with Rouvy?

Yes. QZ can manage virtual gear changes while ROUVY controls the bike or trainer through QZ. ROUVY continues sending terrain/grade changes, while QZ applies the current virtual gear on top of the resistance requested for the route.

You can change QZ virtual gears using several controls, depending on your setup:

- the **Gear - / Gear +** buttons in QZ;
- a **CYCPLUS BC2** handlebar controller;
- **Zwift Click**, **Zwift Play**, or **Zwift Ride** controllers;
- a Bluetooth media/volume remote when **volume buttons change gears** is enabled;
- on a supported RENPHO bike, the physical resistance knob when the RENPHO knob-to-gears option is enabled.

The current virtual gear is managed by QZ. ROUVY does not currently provide a field where QZ can inject its gear number, so you should not expect the QZ gear to appear natively in the ROUVY ride screen. Keep the QZ **Gear** tile visible if you want to see the current gear while riding.

### Confirmed RENPHO R-Q002 + ROUVY + CYCPLUS BC2 setup

A real-world setup has been tested successfully with:

- **RENPHO R-Q002 / R-Q002 E** smart bike;
- QZ running on an iPhone;
- ROUVY running on a MacBook Pro;
- **ROUVY Compatibility** enabled in QZ, with ROUVY connected to QZ over the Wi-Fi/DIRCON virtual-trainer path;
- **Bike Resistance Offset = 8** for this rider;
- a **24-gear Custom Gear Table**;
- a **CYCPLUS BC2** on the handlebars for Gear - / Gear +.

In this configuration the route still changes resistance automatically with the terrain, while the BC2 changes the QZ virtual gear. This is especially useful on descents: instead of spinning out when the route reduces the base resistance, the rider can shift into a harder virtual gear and keep useful pedal resistance.

The RENPHO physical knob can also be used first to tune and test the virtual gear range. Once the preferred range is established, the BC2 can take over handlebar shifting without changing the underlying ROUVY terrain-response setup.

`Bike Resistance Offset = 8` and the exact 24-gear table are values from this confirmed setup, not universal defaults. Other riders may prefer a different baseline or gear table.

![ROUVY ride with the QZ Gear tile visible through iPhone Mirroring and a 24-gear guide](images/rouvy-renpho-bc2-gears.webp)

![Longer ROUVY ride showing QZ virtual gearing with the RENPHO R-Q002 setup](images/rouvy-renpho-bc2-ride.webp)

*Screenshots courtesy of Colby Brannon, shared with permission.*

If using a Bluetooth media/volume remote instead, QZ handles volume-up and volume-down key events as gear controls when the corresponding option is enabled. On Android, the system volume overlay may still briefly appear when those keys are pressed; that does not prevent QZ from using the key presses as gear changes.

## How do I follow Peloton resistance targets when my bike uses a different resistance scale?

Do not try to make the bike's native resistance scale match Peloton numerically. QZ calculates a separate **Peloton Resistance** value for supported bikes so you can follow the resistance target shown by a Peloton workout while the physical bike continues to use its own native levels.

If you only see the bike's normal resistance value:

1. Open **QZ Settings > Tiles**.
2. Find **Peloton Resistance** and make sure the tile is enabled.
3. Return to the main QZ screen and follow the **Peloton Resistance** tile during the workout rather than the normal **Resistance** tile.

For example, a bike whose native resistance stops at level 32 does not need to be remapped so that its normal tile reaches 40 or another Peloton number. The Peloton Resistance tile is the converted value intended for that purpose.

In a confirmed support case, enabling the Peloton Resistance tile immediately exposed the converted Peloton-scale value while the bike's normal resistance remained on its native scale.

## Peloton login from QZ stays on a spinning screen on an Echelon console. What should I try?

On some Echelon consoles, the built-in **Lightning** browser may no longer complete the Peloton authentication flow correctly. QZ can appear to remain on the spinning login screen even though the problem is actually the browser on the console.

If this happens:

1. Sideload a modern browser such as **Google Chrome** onto the Echelon console.
2. Set Chrome as the default browser.
3. Retry the Peloton login from QZ.

In a confirmed support case, replacing the built-in Lightning browser with Chrome immediately allowed the Peloton authentication flow to complete normally.

If changing the browser does not help, try opening the Peloton website directly in the console browser. If it also fails there, the issue is likely outside QZ itself.

## QZ says the Peloton login is successful, but it no longer detects my active workout. What should I try?

If QZ still reports a successful Peloton login but stops recognizing an active Peloton workout, changing the Peloton account password can refresh the authentication state even when simply signing out and back in does not help.

1. Change or reset the password for the Peloton account.
2. Update the saved Peloton password in QZ.
3. Restart QZ, confirm that the Peloton login succeeds, and open the workout again.

In a confirmed support case, signing out and back in did not restore workout detection, while changing the Peloton password did and the integration immediately started working again.

## Kinomap keeps reconnecting to an old device or does not use QZ correctly. What should I check?

When QZ is acting as the bridge between a physical bike and Kinomap, Kinomap should connect to the **virtual device exposed by QZ**, not directly to the physical bike.

If Kinomap has previously been paired with the bike or with older QZ virtual devices, remove those saved connections in Kinomap and set up the connection again. When Kinomap asks for the equipment type/brand, use the **FTMS** option and select the virtual QZ device.

Also avoid pairing the physical bike to the phone/tablet through the operating system Bluetooth settings. Let QZ establish the connection to the physical bike, then let Kinomap connect to QZ.

A useful two-device setup is:

1. Run QZ on the device that connects to the physical bike.
2. Run Kinomap on a second phone/tablet.
3. Remove stale/saved bike connections from Kinomap if it keeps selecting the wrong device.
4. In Kinomap, add the equipment again using **FTMS** and select QZ's virtual device.

In a confirmed support case, Kinomap had retained old connections and was interfering with the intended QZ bridge. Cleaning up those saved connections and using the FTMS path restored the expected connection and resistance behavior.
