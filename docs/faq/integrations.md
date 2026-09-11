# Integrations and authentication

## Can I use QZ to get treadmill data into Garmin without Zwift or another training app?

Yes. There are two different approaches depending on whether you want the Garmin watch to receive the treadmill metrics live or you only want the completed activity in Garmin Connect.

### Option 1: send speed and distance live to a Garmin watch over ANT+

On Android, QZ can connect to a supported treadmill over Bluetooth/FTMS and retransmit treadmill speed and distance using the **ANT+ SDM/footpod profile**.

Your Garmin watch can then pair with QZ as a footpod or speed/distance sensor and record the treadmill activity directly. Zwift, Kinomap, and similar training apps are not required for this setup.

The Android device running QZ must be able to **transmit ANT+**. If the phone or tablet does not have usable native ANT+ support, a compatible USB ANT+ dongle with the appropriate USB/OTG adapter can be used. Make sure the dongle supports transmission, not only reception.

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
3. Confirm that power and the available sensor metrics are updating on the QZ main screen before opening the downstream training app.
4. Then pair the virtual QZ bike/trainer in the training app instead of pairing the physical power sensor directly there.

The `power_sensor_as_bike` option is part of QZ's current device-discovery configuration and changes how a selected power sensor is treated by QZ. This is useful for setups where the physical device provides usable cycling power data but is not otherwise selected as QZ's primary bike.

In a confirmed support case, QZ could see the physical trainer/power source but was not exposing the expected bike data path until **Power Sensor as a Bike** was enabled; after enabling it, the setup began working.

## Can I use Zwift Ride virtual gears with Kinomap through QZ?

Yes. QZ can use the Zwift Ride controllers for shifting while Kinomap connects to QZ as the virtual trainer.

For this setup:

1. Enable **Zwift Play** in **QZ Settings > Accessories**.
2. Wake the Zwift Ride controllers before starting QZ so QZ can discover them.
3. In **Bike Options**, keep **FTMS Bike** set to **Disabled** unless you specifically need to force a particular FTMS bike implementation.
4. Start QZ and verify that the controllers are connected, then pair the QZ virtual trainer in Kinomap.

If the controllers appear connected but gear changes do not work in Kinomap, check **FTMS Bike** first. Selecting a trainer model there can make QZ use the wrong bike path for this setup. In a confirmed support case, changing **FTMS Bike** from a KICKR model back to **Disabled** restored virtual shifting immediately.

## Can I use QZ virtual gears with Rouvy?

Yes. QZ can manage virtual gear changes while Rouvy controls the trainer through QZ. The current QZ gear is handled by QZ itself, so you should not expect Rouvy to display QZ's virtual gear number on its ride screen.

If using the phone's volume buttons to shift is inconvenient, enable QZ's **volume buttons change gears** option and use a Bluetooth media/volume remote. QZ handles volume-up and volume-down key events as gear controls when this option is enabled, so a small handlebar-mounted Bluetooth remote that sends the same keys can be used for shifting.

On Android, the system volume overlay may still briefly appear when those keys are pressed; that does not prevent QZ from using the key presses for gear changes.

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
