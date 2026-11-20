# iOS

## QZ keeps scanning and does not find my Bluetooth fitness device on iPhone. What should I check first?

Make sure QZ has permission to use Bluetooth in iOS. QZ needs Bluetooth access to scan for and connect to supported bikes, treadmills, rowers, trainers, and sensors.

1. Open the iPhone **Settings** app.
2. Find **QZ Fitness** in the app settings and confirm that **Bluetooth** access is enabled. You can also check the Bluetooth permission under **Settings > Privacy & Security > Bluetooth**.
3. Make sure Bluetooth itself is turned on on the iPhone.
4. Fully close and reopen QZ, then try the scan again with the fitness device awake and ready to connect.

If QZ was previously denied Bluetooth permission, simply restarting the scan inside QZ is not enough until the iOS permission is restored.

In a confirmed support case, QZ could not connect because its iOS Bluetooth permission had not been granted. Enabling the permission and retrying made the connection work immediately.

## Why is my QZ workout saved to Apple Health as the wrong activity type?

If you use the QZ Apple Watch app, make sure the **workout type on the watch is set to the activity you are actually doing** before starting the workout. The selected sport determines the HealthKit workout activity type that QZ records.

For example, if you are using an elliptical but the watch is still configured for cycling, Apple Health can record the session as indoor cycling even though QZ is receiving elliptical data correctly.

Select the appropriate workout type in the QZ Apple Watch app, then start the workout again. QZ currently maps its watch workout types to HealthKit activities including walking, running, cycling, rowing, and elliptical.

This setting affects how the workout is classified in Apple Health; it is separate from whether QZ can connect to and receive metrics from the fitness machine.

## Why does Garmin Connect classify my QZ elliptical workout as Walking?

Disable **Strava Virtual Activity** in QZ before recording the elliptical session. When this option is enabled, QZ exports the FIT session as virtual Running or Walking. When disabled, it exports **Fitness Equipment / Elliptical**. This behavior is verified in QZ's FIT writer and was confirmed in support.

## Watt Gain resets to 1.00 after entering a decimal value

On older QZ iPhone builds, if **1.20** is not saved, try **1,20** instead, press **OK**, restart and verify the value. A user confirmed this workaround. Current QZ builds accept both decimal separators. Watt Gain scales reported power, not actual physical effort.

## Can I run QZ together with Zwift or MyWhoosh on the same iPhone or iPad?

**Yes, with the newer QZ iOS beta (TestFlight).** Enable **QZ Settings > Experimental Settings > iOS Background Keep Alive** to let QZ continue running when you switch to **Zwift or MyWhoosh on the same iPhone or iPad**. A separate iOS device is no longer required for this beta workflow.

1. Install a QZ iOS beta that includes **iOS Background Keep Alive**.
2. Enable that setting in **Experimental Settings**. Connect QZ to your physical bike/trainer and configure its virtual trainer as usual.
3. Open **Zwift or MyWhoosh** on that same iPhone/iPad and connect the training app to QZ's virtual trainer rather than directly to the physical trainer.
4. Verify that power, cadence, trainer control and (if used) virtual shifting continue updating while the training app is in the foreground.

**Important:** This is an **iOS beta feature**, not a guarantee for older stable versions or Android. iOS background execution and initial virtual-trainer discovery can behave differently from an established connection; test pairing and stability before a long workout. If Zwift cannot initially discover QZ when QZ is backgrounded, see the [Zwift background discovery FAQ](zwift.md).

## Can I also keep QZ on one iOS device and run MyWhoosh on another?

**Yes.** The two-device arrangement still works: for example, QZ on an **iPhone** connected to the physical trainer, and **MyWhoosh on an iPad** paired with QZ's virtual trainer.

A TestFlight background-operation support test confirmed that virtual shifting continued while QZ was in the background on the iPhone and other apps were being used. An earlier same-iPad MyWhoosh test had failed to discover QZ, which is why older guidance recommended two devices. **That earlier limitation should not be applied to the new same-device iOS beta workflow described above.**

## Apple Music or Siri audio stops when I open QZ on iOS. What can I try?

If the iPhone's Music app stops playing when QZ starts, or Siri begins a track but playback immediately stops, check whether QZ is configured to use the device's volume buttons for virtual gear changes.

1. In QZ settings, disable **Volume buttons change gears** (also labeled **Volume change gears** in some builds).
2. Fully close QZ, start a track in the iOS Music app, and reopen QZ.
3. Also test the opposite order: open QZ first, then start Music or ask Siri to play a track.
4. If you use **iOS Background Keep Alive**, you can test it both enabled and disabled; it is not necessary to disable background operation permanently just to try this workaround.

QZ's volume-button gear feature intercepts volume-button events and can interact with the iOS audio session. In a confirmed support case, disabling **Volume change gears** restored Music playback with **iOS Background Keep Alive** both on and off. In another confirmed iPhone/AirPods test, music continued playing in the QZ foreground **while a CYCPLUS BC2 Bluetooth shifter still changed the Gears tile**. Disabling volume-button shifting does not inherently disable native Bluetooth controller buttons. This is a targeted workaround for audio interruption, not a general requirement to disable background mode.


## Does starting or stopping a workout in QZ on Apple Watch also start or stop QZ on the iPhone?

**No.** The QZ Apple Watch app's Start/Stop controls manage the watch's **HealthKit workout** and enable live heart-rate sharing; they do not automatically start or stop the independent workout session in QZ on the iPhone.

For example, when using a rower with QZ on an iPhone, start the iPhone QZ session to record the machine's metrics and separately start the QZ workout on Apple Watch to share heart rate. Confirm that the Heart tile on the iPhone updates. When finished, stop the watch workout and the iPhone session separately.

A rower user confirmed that starting the QZ watch workout restored heart-rate sharing, and the separate Start/Stop behavior was explained in the same support conversation. This describes the current workflow; automatic synchronization of the two workout controls was not available in that setup.

## Can I use QZ on Apple Watch for live heart rate without saving a duplicate workout to Apple Health?

**Yes, on QZ Apple Watch builds that offer the stop confirmation.** Start the QZ workout on Apple Watch to share live heart rate with QZ on the iPhone and the connected training app. When ending the watch workout, choose **Don't Save** rather than **Save to Apple Health** in the watch's stop confirmation. A user confirmed that **Don't Save** prevented the QZ watch workout and its workout heart-rate recording from being saved to Apple Health.

**Limitation:** This does not disable every HealthKit write from QZ or other apps. Cycling speed, cadence, and cycling-performance entries may still appear in Apple Health, including entries written by QZ or Strava. Review the respective apps' Apple Health write permissions if you want to prevent those additional entries. This option is about saving the **watch workout**, not globally disabling HealthKit writes or heart-rate forwarding.
