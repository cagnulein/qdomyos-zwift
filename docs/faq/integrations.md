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

## Can I use QZ virtual gears with Rouvy?

Yes. QZ can manage virtual gear changes while Rouvy controls the trainer through QZ. The current QZ gear is handled by QZ itself, so you should not expect Rouvy to display QZ's virtual gear number on its ride screen.

If using the phone's volume buttons to shift is inconvenient, enable QZ's **volume buttons change gears** option and use a Bluetooth media/volume remote. QZ handles volume-up and volume-down key events as gear controls when this option is enabled, so a small handlebar-mounted Bluetooth remote that sends the same keys can be used for shifting.

On Android, the system volume overlay may still briefly appear when those keys are pressed; that does not prevent QZ from using the key presses for gear changes.

## Peloton login from QZ stays on a spinning screen on an Echelon console. What should I try?

On some Echelon consoles, the built-in **Lightning** browser may no longer complete the Peloton authentication flow correctly. QZ can appear to remain on the spinning login screen even though the problem is actually the browser on the console.

If this happens:

1. Sideload a modern browser such as **Google Chrome** onto the Echelon console.
2. Set Chrome as the default browser.
3. Retry the Peloton login from QZ.

In a confirmed support case, replacing the built-in Lightning browser with Chrome immediately allowed the Peloton authentication flow to complete normally.

If changing the browser does not help, try opening the Peloton website directly in the console browser. If it also fails there, the issue is likely outside QZ itself.
