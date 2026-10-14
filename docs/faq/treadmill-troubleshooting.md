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

## My Adidas T-800 High Incline shows an incline 6% higher in QZ. How do I fix it?

The Adidas T-800 High Incline has a **-6% to +40%** incline range. QZ has dedicated handling for this range, so do not compensate for the +6% mismatch with the generic **Treadmill Inclination Offset** setting.

Instead:

1. Open **QZ Settings > Treadmill Options > FTMS Treadmill**.
2. Select the treadmill's exact Bluetooth name, for example `ADIDAS xxxx`.
3. Fully close QZ and restart it.
4. Test the incline again. A treadmill incline of 0% should now also appear as 0% in QZ.

You can also configure **Treadmill minimum incline = -6** and **Treadmill maximum incline = 40**. If you use Zwift automatic incline, set **Zwift Max Incline = 40** as well.

This setup was explicitly confirmed to resolve a +6% incline display mismatch on an Adidas T-800 High Incline. The current QZ implementation recognizes Adidas treadmills and applies dedicated minimum/maximum incline handling for the -6% to +40% range.

## Zwift incline through QZ feels about twice as steep as expected. What should I change?

When QZ is using Zwift auto-inclination, the inclination received from Zwift is transformed with QZ's **Zwift Inclination Gain** and **Zwift Inclination Offset** settings. The current calculation is:

`calculated inclination = raw Zwift inclination × gain + offset`

If the treadmill response feels roughly twice as strong as expected, use this as a starting point:

1. Set **Zwift Inclination Gain** to **0.5**.
2. Set **Zwift Inclination Offset** to **0**.
3. Test several climbs and descents and adjust only if your treadmill or personal preference requires it.

The gain changes the scale of every grade change, while the offset shifts the whole range up or down. Do not use the offset to correct a proportional mismatch.

This setup was confirmed in a Wahoo KICKR RUN support case where a gain of 0.5 produced the expected incline behavior. The same 0.5 starting value was also used successfully while validating automatic Zwift incline on another supported treadmill.

## Strava does not show the treadmill elevation gain recorded by QZ. What should I change?

If QZ records the incline/elevation correctly but Strava does not display the elevation gain, enable **Strava Virtual Activity** in QZ.

QZ writes treadmill activities differently depending on this setting:

- with **Strava Virtual Activity = ON**, the FIT activity is marked as a **Virtual Activity**;
- with **Strava Virtual Activity = OFF** and the treadmill tag enabled, the FIT activity is marked as **Treadmill/Indoor**.

Strava can hide the elevation for activities tagged as treadmill/indoor even when the FIT file contains the elevation data. In a confirmed support case, removing the indoor/treadmill classification made the elevation visible in Strava, while keeping the virtual classification still showed it correctly.

So, for treadmill workouts where you want Strava to display QZ's calculated elevation gain, use **Strava Virtual Activity = ON**.

## My treadmill only changes incline in 1% increments. Why do 0.5% commands not work as expected?

Set **Inclination Step** in QZ's advanced treadmill settings to match the smallest incline increment supported by the treadmill. For a treadmill that only accepts whole-percent changes, set **Inclination Step = 1**.

QZ uses this setting to round requested incline values to a supported step before sending them to the treadmill. For example, with a 1% step, half-percent targets are rounded to whole-percent commands instead of repeatedly requesting an unsupported intermediate value.

This was confirmed in a support case where treadmill incline control was working but 0.5% changes were not: the treadmill itself only adjusted in 1% intervals, so setting the QZ inclination step to 1 matched the hardware behavior.

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

## How can I temporarily hold my treadmill incline instead of following Zwift's automatic incline?

If QZ is following Zwift's automatic incline but you want to hold a manually selected incline (for example, during a cooldown), tap the **magnet / auto-resistance icon** at the top of QZ to pause automatic control. Set the desired incline using the treadmill's own controls and leave QZ connected to continue recording the treadmill's reported speed, incline and elevation gain.

The toggle pauses **automatic speed control as well as incline control**; it is not an incline-only lock. Tap it again to request automatic control when needed, but verify that incoming Zwift grade updates actually resume before relying on it for a subsequent climb.

In a confirmed treadmill support case, pausing automatic control allowed the user to hold a 15% incline while continuing to record the workout. The user also reported that automatic incline did not reliably resume afterward; that separate problem remains under investigation.
