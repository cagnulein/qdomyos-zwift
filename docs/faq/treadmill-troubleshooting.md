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
