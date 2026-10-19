# Device compatibility

## Is the BH Fitness Nexor Dual / i-Nexor compatible with QZ?

Yes. QZ has current support for BH Fitness bikes that use the iConcept/BH DUALKIT Bluetooth protocol, including the Nexor-specific handling used by devices advertising with a `BH-` name.

A confirmed support case with a BH Fitness Nexor Dual connected successfully to QZ and allowed resistance control from QZ.

If the bike connects and responds correctly inside QZ but a downstream training app cannot see QZ, treat that as a separate virtual-device/advertising issue rather than a bike-compatibility problem.

## Is the BKOOL Smart Bike v1 compatible with QZ?

Yes. QZ includes a dedicated BKOOL bike implementation, and a confirmed support case with a BKOOL Smart Bike v1 connected successfully and could then be used with MyWhoosh through QZ.

This confirms the basic bike-to-QZ and QZ-to-training-app path. It does not imply that MyWhoosh virtual gear difficulty will feel identical to the bike's previous native integration; gear-range or resistance-feel tuning should be treated separately if needed.

## Is a Bodytone DS60 advertising as SMB1 compatible with QZ and MyWhoosh?

Yes. QZ recognizes Bluetooth fitness bikes advertising with an **SMB1** name through its FTMS bike support. A confirmed Bodytone DS60/SMB1 setup connected to QZ and supported automatic resistance control from MyWhoosh through QZ.

Connect the bike to QZ first and confirm cadence/power data and resistance control in QZ. Then enable QZ's normal virtual Bluetooth bike and pair **MyWhoosh with QZ**, not directly with the physical bike. If the bike is not detected, check its advertised Bluetooth name and ensure no other app has taken its Bluetooth connection.

## Can I connect a Life Fitness / ICG IC7 bike to QZ and use MyWhoosh instead of Zwift?

Yes. QZ includes a dedicated Flywheel bike driver for supported ICG/Life Fitness bikes. For an IC7, enable the relevant option under **QZ Settings > Bike Options > Flywheel Bike Options** and connect the bike to QZ first. Verify that the bike's live metrics update in QZ, then pair **MyWhoosh with QZ's virtual bike** as the power/cadence source.

Zwift is not required for this connection path. This is verified setup guidance based on QZ's Flywheel bike implementation; it does not imply that every IC7 console firmware or every control feature has been tested.

## Can KayakFirst connect to MyWhoosh through QZ?

Yes. A KayakFirst ergometer was confirmed working with QZ and MyWhoosh. Pair the machine with QZ first, then select the QZ virtual device in MyWhoosh. The [Rowing FAQ](rowing.md) explains the available virtual rower and PM5 compatibility settings.

**Watt Gain** changes the power value reported by QZ, not the physical effort. A gain of 2 doubles the reported watts. QZ may round a distance such as 317 metres to 0.3 km in its summary; this is a display rounding difference, not necessarily a tracking error.

## Does the Sunny Health & Fitness SF-T723030 treadmill work with QZ and Zwift automatic incline?

**Yes.** A support user confirmed that QZ connected to a **Sunny SF-T723030** and adjusted the treadmill's incline during hills in Zwift. The user saw QZ's virtual trainer in Zwift under a **Wahoo KICKR** name; that name is expected and does not mean the physical treadmill is a Wahoo device.

For the confirmed setup:

1. Power on the treadmill and connect it to QZ. Verify that QZ is receiving the treadmill's live metrics before starting Zwift.
2. Under **QZ Settings > Zwift Options**, enter the credentials of the Zwift account used for the workout and configure QZ's Zwift auto-inclination integration.
3. In Zwift, select the virtual trainer advertised by QZ rather than trying to pair the Sunny treadmill directly.
4. Test a few gentle Zwift climbs and compare the treadmill's displayed incline with the requested Zwift grades before relying on stronger hills.

**Incline calibration remains device-specific.** The confirmed SF-T723030 had **15 physical incline levels**, with an indicated range of approximately **1.8% to 15%**, so small or zero-grade Zwift requests may not map exactly to a physical incline. If the response is consistently too strong or offset, adjust **Zwift Inclination Gain** (scale) and **Zwift Inclination Offset** (baseline) only after comparing actual values. The user confirmed automatic incline **works**, but had not yet confirmed an exact grade-to-incline calibration.

## Does QZ have a dedicated setup for the Life Fitness / ICG IC5 bike?

**Yes.** QZ includes a **Life Fitness IC5** option in its bike settings and a dedicated initialization path for the IC5 in the Flywheel bike driver. This is distinct from the IC7/IC8 handling.

1. Under **QZ Settings > Bike Options**, enable **Life Fitness IC5** and restart QZ before connecting to the bike.
2. Power on the IC5 console, then scan and check whether QZ receives live bike metrics.
3. If the console is not discovered, scan it with **nRF Connect** and provide the advertised **Bluetooth name and services**, together with the IC5 console model/firmware. Different console versions can expose different Bluetooth behavior.

QZ's current driver sends the IC5-specific initialization packets when this option is enabled. This is **code-verified setup guidance**, not confirmation that every IC5 console variant supports all metrics or resistance commands. In particular, a console that advertises unexpected services needs verification before compatibility can be promised.
