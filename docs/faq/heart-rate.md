# Heart rate

## My external or companion heart rate is available, but QZ keeps using the heart rate from the fitness machine. What should I do?

If the bike, treadmill, rower, or other fitness machine reports its own heart-rate value, QZ can prefer that built-in value over an external source. This can make a heart-rate source connected through QZ Companion or another supported external HR source appear not to update QZ correctly.

In **Heart Rate Options**, enable **Disable HRM from the machinery**. This tells QZ to ignore the heart-rate value supplied by the fitness machine so that the configured external heart-rate source can be used instead.

This setting is especially useful when the machine reports a stale, zero, or otherwise unwanted heart-rate value while the external source is working correctly.

In a confirmed support case, heart rate from QZ Companion was working but was not being reflected correctly in QZ while using a bike. Enabling **Disable HRM from the machinery** resolved the issue.

## My Garmin watch stops broadcasting heart rate when QZ connects. What should I check?

If heart-rate broadcasting works before QZ connects but stops when QZ establishes its connection, inspect the watch's saved sensor connections. In **Garmin Sensors & Accessories**, look for a previously paired QZ/virtual fitness sensor and remove that old pairing if it is no longer needed. Then restart heart-rate broadcasting and reconnect QZ.

A confirmed support case recovered continuous heart-rate broadcasting after an obsolete QZ sensor pairing was removed from the Garmin watch. This is a troubleshooting step for an existing unwanted sensor pairing, not a requirement to delete every Garmin sensor or to disable heart-rate broadcasting.

## Garmin Companion connects, but QZ still shows zero heart rate. What should I check?

When using QZ Garmin Companion on a Garmin watch with QZ on an iPhone, enable **Garmin Companion** in QZ and set **Heart Rate Belt Name** to **Disabled**. If the connected fitness machine reports zero or stale heart rate, enable **Heart Rate Options > Disable HRM from the machinery** so QZ uses the companion heart rate instead.

Start Garmin Companion on the watch, reconnect QZ and confirm the QZ Heart Rate tile updates before recording. This combination was confirmed working with a supported elliptical and Garmin watch: the watch's heart rate appeared in QZ and in the resulting Garmin Connect activity.

Garmin Companion supplies watch data such as heart rate to QZ; it does not mirror the fitness machine's workout metrics onto the watch screen. For heart rate alone, the watch's Bluetooth HR Broadcast feature is another option; do not enable both input paths at the same time.

## Can QZ on an iPhone receive heart rate from a Garmin watch without pairing QZ to the watch as a bike sensor?

**Yes.** Use the Garmin watch as a **Bluetooth heart-rate broadcaster**, rather than trying to pair QZ's virtual bike as a sensor on the watch.

1. On the Garmin watch, start **Broadcast Heart Rate** (often under **Settings > Health & Wellness > Wrist Heart Rate**; menus vary by model).
2. In **QZ Settings > Heart Rate Options > Heart Rate Belt Name**, select the watch's advertised Bluetooth name.
3. If QZ instead keeps using the machine's built-in heart-rate value, enable **Disable HRM from the machinery**.
4. Check that QZ's Heart Rate tile updates before starting the workout. You do not need to start a second workout recording on the watch just to broadcast heart rate.

This is **watch → QZ** heart-rate input, not live transmission of bike data from an iPhone to Garmin over ANT+ (which iOS does not support). For the completed workout, use QZ's **Garmin Connect integration** separately. A rider with a Garmin Enduro 3 and QZ on iPhone explicitly confirmed that heart rate appeared in QZ after following the broadcast setup.

