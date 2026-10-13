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
