# MyWhoosh

## Zwift Click / Click V2 is connected to QZ, but it does not change gears in MyWhoosh. What should I check?

For MyWhoosh, the controller should stay connected to **QZ**. QZ can forward the controller button events to MyWhoosh through its **OpenBikeControl / MyWhoosh Link** integration.

Use this setup:

1. In QZ, enable **Zwift Click** so QZ connects to the Click / Click V2 controller.
2. Disable **Get gears from Zwift** when using MyWhoosh. That option is for the Zwift workflow, where Zwift owns the virtual gearing.
3. Enable **MyWhoosh Link / OpenBikeControl** in QZ.
4. Enable the MyWhoosh option that **overrides local gear changes**. With this enabled, Click `+` / `-` events are sent to MyWhoosh instead of only changing QZ's own local virtual gear.
5. Restart QZ after changing the MyWhoosh Link setting.
6. If QZ and MyWhoosh run on different devices, keep both devices on the same local Wi-Fi/LAN so MyWhoosh can discover QZ's OpenBikeControl service.
7. Start QZ first and confirm the Click is connected, then start MyWhoosh.

The trainer/data connection and OpenBikeControl have different jobs. MyWhoosh can still receive power/cadence/resistance from QZ through the normal virtual trainer connection, while OpenBikeControl is used for controller actions such as virtual gear changes and supported steering/button commands.

If the Click changes resistance in QZ but the gear shown inside MyWhoosh does not change, this usually means the Click is still operating QZ's **local virtual gear** instead of sending the gear command to MyWhoosh. Re-check **MyWhoosh Link / OpenBikeControl** and the **override local gear changes** option.

If MyWhoosh does not discover the OpenBikeControl service at all, verify local-network permissions, that both devices are on the same network, and then restart both QZ and MyWhoosh before collecting a QZ debug log.

## Can I use QZ's own virtual gears with Zwift Ride and KICKR CORE in MyWhoosh, instead of MyWhoosh's MyShift?

Yes. This is a different configuration from the OpenBikeControl setup above: QZ owns the gear number and applies the resistance change to the physical trainer.

1. Connect the physical KICKR CORE to QZ. Enable **Zwift Play** under **Accessories**, and leave **Get Gears from Zwift** disabled.
2. In **Bike Options > Wahoo Options**, select **Reality Bender (24 even spaced)** if you want 24 virtual gears instead of the 13-gear Explorer preset. Set the physical chainring and cog sizes to match the actual drivetrain; see the [Wahoo gearing FAQ](zwift.md).
3. Enable **Buttons debouncing** if one press skips multiple gears. Disable **MyWhoosh Link / OpenBikeControl** for this QZ-local shifting configuration, restart QZ with the handlebars awake, and enable the **Gears** tile.
4. On a second device, pair MyWhoosh to QZ's virtual trainer as power, cadence and controllable trainer, rather than pairing the physical KICKR directly.

A rider confirmed single-step shifting after configuring the controller and later confirmed that physical effort changed while shifting on a nearly flat MyWhoosh route. In this mode, read the current gear in QZ; MyWhoosh's own gear display is not necessarily synchronized.

## My Elite Rizer connects to QZ but does not follow hills in MyWhoosh. What should I check?

**Check MyWhoosh's own Gradient / trainer difficulty setting before changing QZ's incline mapping.** If that setting is at **0%**, MyWhoosh may not send the route's actual grade changes to QZ, even though the course visibly climbs and descends.

1. Confirm that the **Elite Rizer is connected to QZ** and that QZ is receiving live bike/trainer data.
2. In MyWhoosh's device pairing, choose **QZ's virtual trainer as the controllable trainer**, not just as the power source.
3. Find MyWhoosh's **Gradient** setting. If it is **0%**, raise it to the desired level; **100%** sends the full selected gradient effect.
4. In QZ's **Elite Rizer Options**, set **Elite Rizer Gain** to its normal default of **1.0** while testing. Do not try to compensate for absent MyWhoosh grades by setting the Rizer gain very high.
5. Test a short route with modest uphill and downhill changes. Compare the route's grade, QZ's incline tile, and the Rizer's response.

**Confirmed result:** In one support case, QZ connected to the Rizer and could send incline commands, but the MyWhoosh log contained only nearly flat requests (0%, +0.2%, 0%) during much steeper route sections. The user found MyWhoosh **Gradient = 0%**, changed it to **100%**, and explicitly confirmed that QZ began following the course and the Rizer responded correctly.

QZ's current Elite Rizer driver multiplies the requested incline by **Elite Rizer Gain**, whose default is **1.0**; it cannot reconstruct grades that MyWhoosh never sends. **Downhill range and steering** are separate features and were not validated by this fix.

## Can QZ and MyWhoosh run on the same phone or tablet?

**Yes, on iPhone and iPad with the newer QZ iOS beta.** In **QZ Settings > Experimental Settings**, enable **iOS Background Keep Alive** so QZ can continue running while MyWhoosh is in the foreground **on the same iOS device**. See the [iOS same-device setup FAQ](ios.md).

1. Install a QZ iOS beta that includes **iOS Background Keep Alive**, then enable it.
2. Connect the physical bike/trainer to QZ and check its live metrics. Configure QZ's virtual trainer as usual.
3. Open MyWhoosh on the **same iPhone or iPad** and pair with **QZ's virtual trainer**, not the physical trainer.
4. Test power/cadence, resistance control and virtual shifting while QZ runs in the background.

**Older builds and other platforms:** A previous test on an older iPad build found that MyWhoosh discovered the physical KICKR CORE but not QZ's virtual trainer; a separate iPhone-plus-iPad setup worked. An Android same-phone support case also could not use that same-device bridge. If your installed build does not support the new iOS background workflow, use **QZ on one device and MyWhoosh on another**. Enable the appropriate QZ virtual-device transport and keep the devices on the same local network when using Wi-Fi/DIRCON or OpenBikeControl. Do not assume initial virtual-trainer discovery works identically to an already-established session.

## MyWhoosh ERG mode changes effort when I shift gears through QZ and a Wahoo KICKR. What should I check?

If **QZ is the bridge to your Wahoo KICKR** and changing QZ's virtual gears unexpectedly changes the effort during a MyWhoosh ERG interval, check for a second, direct connection to the physical KICKR in MyWhoosh.

1. Keep the physical **KICKR connected to QZ** as usual.
2. In **MyWhoosh's sensor pairing**, remove the **physical KICKR** if it is also selected separately as a **Power Source**.
3. Pair **QZ's virtual trainer** as the power source and, where MyWhoosh offers the choice, as the controllable trainer. Do not mix QZ control with a separate direct KICKR power connection for this test.
4. Start a short ERG interval, shift QZ's virtual gears and check whether the ERG target/effort remains stable.

**Confirmed workaround:** A rider found that gear changes interfered with ERG mode while the KICKR was also paired directly as a power sensor in MyWhoosh. After removing that separate power-sensor pairing, the rider explicitly confirmed that gear changes no longer affected ERG mode. This does not establish that duplicate pairing is the cause of every ERG issue.
