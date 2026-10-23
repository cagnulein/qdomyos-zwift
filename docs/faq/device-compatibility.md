# Device compatibility

## Is the BH Fitness Nexor Dual / i-Nexor compatible with QZ?

Yes. QZ has current support for BH Fitness bikes that use the iConcept/BH DUALKIT Bluetooth protocol, including the Nexor-specific handling used by devices advertising with a `BH-` name.

A confirmed support case with a BH Fitness Nexor Dual connected successfully to QZ and allowed resistance control from QZ.

If the bike connects and responds correctly inside QZ but a downstream training app cannot see QZ, treat that as a separate virtual-device/advertising issue rather than a bike-compatibility problem.

## Is the BKOOL Smart Bike v1 compatible with QZ?

Yes. QZ includes a dedicated BKOOL bike implementation, and a confirmed support case with a BKOOL Smart Bike v1 connected successfully and could then be used with MyWhoosh through QZ.

This confirms the basic bike-to-QZ and QZ-to-training-app path. It does not imply that MyWhoosh virtual gear difficulty will feel identical to the bike's previous native integration; gear-range or resistance-feel tuning should be treated separately if needed.

## My older Kettler bike is missing from nRF Connect or uses Bluetooth 2.0. How can QZ discover it?

Some older Kettler bikes, including **RACER S** models that connect through **MyHomeFIT**, communicate over **Bluetooth Classic (Bluetooth 2.0 / SPP)** rather than Bluetooth Low Energy (BLE). **nRF Connect for Mobile scans BLE devices**, so a Bluetooth Classic-only bike may not appear there. Not appearing in nRF Connect is a clue, **not proof**, that the bike uses Bluetooth Classic.

On **Android**, try the following:

1. Close **MyHomeFIT** and any other app or phone that might already be connected to the bike.
2. Open **QZ Settings > Bike Options > iConsole Bike Options** and enable **BH SPADA Compatibility**. This allows QZ to search for Bluetooth Classic devices as well as BLE devices on Android.
3. Fully close and restart QZ. Ensure the Kettler console is powered on and ready to pair.
4. Check whether QZ discovers the bike and whether **power (watts)** and **cadence** update while pedaling.

If QZ detects the Kettler but all metrics stay at zero, discovery alone is not enough: the bike may need a matching protocol driver. For older **RACER S** bikes using MyHomeFIT's Bluetooth Classic protocol, use a QZ build that includes the [Kettler Classic driver](https://github.com/cagnulein/qdomyos-zwift/pull/5147), and repeat the test with **BH SPADA Compatibility** enabled. If it still fails, share the exact Bluetooth device name, model/year, and a QZ debug log.

**Note:** The Bluetooth Classic workaround is different from support for newer Kettler models that use BLE. Do not assume every Kettler bike uses the same protocol.
