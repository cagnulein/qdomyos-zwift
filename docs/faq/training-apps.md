# Training app connections

## Kinomap keeps reconnecting to an old device or does not use QZ correctly. What should I check?

When QZ is acting as the bridge between a physical bike and Kinomap, Kinomap should connect to the **virtual device exposed by QZ**, not directly to the physical bike.

If Kinomap has previously been paired with the bike or with older QZ virtual devices, remove those saved connections in Kinomap and set up the connection again. When Kinomap asks for the equipment type/brand, use the **FTMS** option and select the virtual QZ device.

Also avoid pairing the physical bike to the phone/tablet through the operating system Bluetooth settings. Let QZ establish the connection to the physical bike, then let Kinomap connect to QZ.

A useful two-device setup is:

1. Run QZ on the device that connects to the physical bike.
2. Run Kinomap on a second phone/tablet.
3. Remove stale/saved bike connections from Kinomap if it keeps selecting the wrong device.
4. In Kinomap, add the equipment again using **FTMS** and select QZ's virtual device.

In a confirmed support case, Kinomap had retained old connections and was interfering with the intended QZ bridge. Cleaning up those saved connections and using the FTMS path restored the expected connection and resistance behavior.
