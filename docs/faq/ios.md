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
