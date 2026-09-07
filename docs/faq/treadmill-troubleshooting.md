# Treadmill troubleshooting

## QZ is connected to my treadmill, but the session or distance is not updating. What should I check?

First check whether the QZ workout session itself is paused. If the **Play** button at the top of QZ is flashing, press it to resume/start the QZ session.

The QZ Play button controls the QZ workout session; it does not necessarily start the treadmill belt. Depending on the treadmill, you may still need to press Start on the treadmill itself.

If speed is updating but distance is still not behaving correctly, check **QZ Settings > Treadmill options > Treadmill Direct Distance**. Some treadmills do not provide a usable direct distance value. In that case, disable **Treadmill Direct Distance** so QZ calculates distance from speed over time instead.

In a confirmed support case, the session/distance started working after correcting these settings. QZ's current treadmill implementation also calculates distance from speed when `Treadmill Direct Distance` is disabled.

## Can Zwift automatically control both treadmill incline and speed through QZ?

QZ can use the Zwift integration for **automatic inclination**. Configure your Zwift credentials in QZ and enable the Zwift auto-inclination option.

Zwift does not provide QZ with a treadmill target speed in the same way, so Zwift cannot directly drive automatic treadmill speed through this integration.

If you want automatic speed changes for a structured workout, recreate the workout in the **QZ workout editor** and run it from QZ. QZ can then control the treadmill speed according to the workout steps while the Zwift integration supplies the inclination information.

## Can I use a Stryd footpod with a non-smart treadmill and send the data to Zwift through QZ?

Yes. If the treadmill itself has no FTMS or other usable fitness connection, QZ can connect directly to a **Stryd** sensor and use it as the treadmill data source, then expose the resulting treadmill data to Zwift.

A Garmin Virtual Run or Garmin Companion bridge is not required for this setup.

1. Pair the Stryd sensor directly with QZ.
2. Configure the external power/running sensor to be used as the treadmill source in QZ.
3. If the reported running speed needs calibration, use QZ's speed gain adjustment.
4. When the treadmill does not report inclination, enter the treadmill incline manually in QZ.
5. Pair Zwift with QZ as the treadmill source.

QZ's current Stryd implementation supports using the external sensor as a treadmill source, including the `power_sensor_as_treadmill` configuration.
