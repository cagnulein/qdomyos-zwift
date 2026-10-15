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
