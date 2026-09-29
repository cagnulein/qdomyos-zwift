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
