# Treadmill troubleshooting

## QZ is connected to my treadmill, but the session or distance is not updating. What should I check?

First check whether the QZ workout session itself is paused. If the **Play** button at the top of QZ is flashing, press it to resume/start the QZ session.

The QZ Play button controls the QZ workout session; it does not necessarily start the treadmill belt. Depending on the treadmill, you may still need to press Start on the treadmill itself.

If speed is updating but distance is still not behaving correctly, check **QZ Settings > Treadmill options > Treadmill Direct Distance**. Some treadmills do not provide a usable direct distance value. In that case, disable **Treadmill Direct Distance** so QZ calculates distance from speed over time instead.

In a confirmed support case, the session/distance started working after correcting these settings. QZ's current treadmill implementation also calculates distance from speed when `Treadmill Direct Distance` is disabled.

## My Horizon treadmill connects to QZ but behaves incorrectly or does not expose all expected data/control. What should I try?

Some Horizon treadmills expose both Horizon's proprietary Bluetooth service and the standard FTMS service. If QZ connects but the treadmill behaves incorrectly, try forcing QZ to use FTMS instead of the proprietary Horizon protocol.

1. Open **QZ Settings > Treadmill Options > Horizon Treadmill options**.
2. Enable **Force Using FTMS**.
3. Restart QZ and reconnect the treadmill.

This setting is especially relevant when the treadmill advertises a usable FTMS service. QZ's Horizon implementation explicitly detects this situation and can prompt the user to enable **Force Using FTMS**. In a confirmed Horizon 7.4AT support case, enabling it resolved most of the connection/control issues.

Note that some treadmill consoles disable their physical controls while an app has an active Bluetooth control connection. That behavior can be firmware-specific and is separate from QZ's protocol selection.

## My Horizon 7.4AT is set to km/h, but QZ shows a value about 1.609 times higher. How can I keep Peloton automatic speed control working?

On some Horizon 7.4AT setups, the proprietary Horizon protocol can report a speed value that QZ interprets as miles per hour even though the treadmill itself is configured for km/h. A typical symptom is that the treadmill shows **0.8** while QZ shows about **1.3 km/h**.

If enabling **Force Using FTMS** fixes the displayed speed but causes Peloton automatic treadmill speed control to stop working, keep the Horizon proprietary protocol and correct the conversion with the speed gain instead:

1. Keep the treadmill itself configured for **km/h**.
2. Set QZ to **km/h**.
3. Open **QZ Settings > Treadmill Options > Horizon Treadmill options** and leave **Force Using FTMS** disabled.
4. Set **Speed Gain** to **0.621371**. If the field does not accept that many digits, **0.6214** is fine.
5. Set **Speed Offset** to **0**.
6. Restart QZ and reconnect the treadmill.

`0.621371` is the inverse of the miles-to-kilometers conversion factor, so it cancels the unwanted `x 1.60934` conversion in this case.

This setup was confirmed on a Horizon 7.4AT to correct the displayed km/h value while preserving Peloton automatic speed control through QZ.

Because **Speed Gain** also participates in QZ's treadmill speed-control path, verify both directions after changing it: manually set a simple treadmill speed and confirm QZ displays the same value, then start a Peloton treadmill workout and confirm an automatically requested speed produces the expected treadmill speed.

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
