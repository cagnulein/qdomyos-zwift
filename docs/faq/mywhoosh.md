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

**For the usual QZ virtual-trainer bridge, use two devices.** Running QZ and MyWhoosh on one phone/tablet is not a supported or reliable way for MyWhoosh to discover and connect to QZ's own virtual Bluetooth trainer. QZ connecting to the *physical* trainer does not mean MyWhoosh on that same device can discover QZ as another trainer.

1. Run **QZ** on a phone/tablet and let it connect to the physical bike or trainer. Confirm that live power and cadence appear in QZ.
2. Enable QZ's normal virtual-device/virtual Bluetooth connection. Restart QZ if the settings change requires it.
3. Run **MyWhoosh on a different device** (another phone/tablet or a PC) and select QZ's advertised virtual trainer, often named **Wahoo KICKR 0000**, rather than the physical trainer, as the power source and controllable trainer.
4. Keep the devices on the same local network if using QZ's Wi-Fi/DIRCON or MyWhoosh Link/OpenBikeControl features. For Bluetooth virtual-trainer pairing, use the receiving device's Bluetooth scan.

A support user running both apps on **one iPad** could connect QZ to a KICKR CORE but MyWhoosh only found the physical trainer, not QZ. The **separate-iPhone-and-iPad** setup worked for the same user, including virtual shifting. A separate Android-phone support conversation also established that the same-phone MyWhoosh bridge was not available for that workflow.

This limitation is specific to **MyWhoosh discovering QZ on the same device**; it does not mean QZ and every other training app have identical same-device capabilities.

## MyWhoosh ERG mode changes effort when I shift gears through QZ and a Wahoo KICKR. What should I check?

If **QZ is the bridge to your Wahoo KICKR** and changing QZ's virtual gears unexpectedly changes the effort during a MyWhoosh ERG interval, check for a second, direct connection to the physical KICKR in MyWhoosh.

1. Keep the physical **KICKR connected to QZ** as usual.
2. In **MyWhoosh's sensor pairing**, remove the **physical KICKR** if it is also selected separately as a **Power Source**.
3. Pair **QZ's virtual trainer** as the power source and, where MyWhoosh offers the choice, as the controllable trainer. Do not mix QZ control with a separate direct KICKR power connection for this test.
4. Start a short ERG interval, shift QZ's virtual gears and check whether the ERG target/effort remains stable.

**Confirmed workaround:** A rider found that gear changes interfered with ERG mode while the KICKR was also paired directly as a power sensor in MyWhoosh. After removing that separate power-sensor pairing, the rider explicitly confirmed that gear changes no longer affected ERG mode. This does not establish that duplicate pairing is the cause of every ERG issue.
