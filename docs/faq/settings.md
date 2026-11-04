# General settings and migration

## How can I transfer QZ settings to a new phone?

On the old phone, open **QZ Settings** and use the **down-arrow** button to save a configuration file. Copy the resulting `.qzs` file to the new phone before uninstalling QZ. Open **QZ Settings** there and use the **up-arrow** button to load the copied file, then restart QZ.

This transfers configuration, not the entire workout history or all separate profile files. Check the restored options and sign in to external services again if needed. The save/load flow is verified in current QZ code; the support user had not yet confirmed completing the transfer.

## Can I change QZ's interface language without changing my phone language?

Yes. Open **QZ Settings > General Options > App Language**, select German, Italian, or another available language, press **OK**, and restart QZ. Choose **Auto (System)** to follow your device's language.

This selector and restart requirement are verified in QZ's current settings UI. **Android:** if the installed Play Store version does not yet contain the Italian translation, join the [QZ Google Play beta](https://play.google.com/apps/testing/org.cagnulen.qdomyoszwift), then update QZ from the Play Store and reopen it. In a confirmed support case, the updated beta displayed Italian successfully. Availability of a particular translation may change as beta releases reach the stable channel.


## Can I use my QZ purchase on a replacement phone, or transfer it between iPhone and Android?

A QZ purchase can be used again on a replacement phone **within the same app store**, using the same Apple account for iPhone/iPad or the same Google Play account for Android. Install QZ from the store associated with the original purchase.

**Apple App Store and Google Play are separate marketplaces.** A purchase made on iPhone does **not** transfer to an Android phone (or vice versa), even if you use the same email address for both accounts; purchasing on the new platform is required.

The app purchase and your saved QZ configuration are separate. To move your settings between phones, use the [QZ settings export/import instructions](#how-can-i-transfer-qz-settings-to-a-new-phone).

## I subscribed to support QZ, but the app still stops after the trial period. What should I check?

If you are using QZ's **supporter/Patreon email-based unlock** on a compatible installation, open **Settings > General Options > Email** and enter the **same email address as the active subscription, entirely in lowercase**. Save the settings, fully close QZ, and reopen it so the license status can refresh.

In a confirmed case, the email had an uppercase first letter; changing it to lowercase stopped QZ from closing after ten minutes. This is a **supporter-email unlock** troubleshooting step, not a way to transfer an Apple App Store purchase to Google Play or vice versa. If it still times out, check that the subscription is active and contact QZ support with the email configured in QZ.

## QZ connects to a nearby trainer instead of the fitness machine I selected. How can I force the main device?

If QZ detects multiple supported Bluetooth fitness machines in the same room, selecting a specific **FTMS Treadmill** or **FTMS Bike** protocol alone may not prevent another supported machine from becoming QZ's main device.

1. Open **QZ Settings > Advanced Settings > Manual Device**.
2. Select the **exact Bluetooth name** of the machine you want as the main device.
3. Fully close and restart QZ with both devices powered on.
4. Confirm that the main QZ device is the intended machine and that its Start/Stop or other supported controls work.

QZ may still **discover** other Bluetooth devices in its debug log; discovery alone does not mean they were connected as the main fitness machine. If the target is unavailable, QZ may keep looking for the selected name. Remove or change the Manual Device filter when switching to a different main machine.

**Confirmed result:** A treadmill was previously displaced by a nearby Tacx Neo 2T bike trainer despite an explicit **FTMS Treadmill** selection. After setting **Manual Device** to the treadmill's exact advertised name, QZ still discovered the Tacx but selected the treadmill as its main device and successfully started and stopped the belt. The trainer no longer needed to be powered off.
