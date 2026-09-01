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

## Peloton login from QZ stays on a spinning screen on an Echelon console. What should I try?

On some Echelon consoles, the built-in **Lightning** browser may no longer complete the Peloton authentication flow correctly. QZ can appear to remain on the spinning login screen even though the problem is actually the browser on the console.

If this happens:

1. Sideload a modern browser such as **Google Chrome** onto the Echelon console.
2. Set Chrome as the default browser.
3. Retry the Peloton login from QZ.

In a confirmed support case, replacing the built-in Lightning browser with Chrome immediately allowed the Peloton authentication flow to complete normally.

If changing the browser does not help, try opening the Peloton website directly in the console browser. If it also fails there, the issue is likely outside QZ itself.
