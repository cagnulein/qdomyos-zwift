# Device compatibility

## Is the BH Fitness Nexor Dual / i-Nexor compatible with QZ?

Yes. QZ has current support for BH Fitness bikes that use the iConcept/BH DUALKIT Bluetooth protocol, including the Nexor-specific handling used by devices advertising with a `BH-` name.

A confirmed support case with a BH Fitness Nexor Dual connected successfully to QZ and allowed resistance control from QZ.

If the bike connects and responds correctly inside QZ but a downstream training app cannot see QZ, treat that as a separate virtual-device/advertising issue rather than a bike-compatibility problem.

## How do I connect a Life Fitness Discover SE4 bike?

QZ can connect to the Discover SE4 through its FTMS bike interface. The console must first be activated with its NFC tap so that it starts Bluetooth advertising. QZ does not currently reproduce the NFC/OOB activation itself.

1. Wake the Discover SE4 and perform the console's NFC tap/activation procedure.
2. Wait for the bike to start advertising over Bluetooth. Its name normally starts with `LF-`; the rest of the name can change between sessions.
3. In QZ, open **Settings > Bike Options > FTMS Bike** and select the exact `LF-*` device name that appeared after the NFC activation.
4. Fully close and restart QZ so the selected FTMS-bike route is used during discovery.
5. Reconnect and start pedaling. Cadence, speed, power, and the other available FTMS metrics should then update in QZ.

If the `LF-*` device does not appear, repeat the NFC activation and temporarily close other fitness or companion apps that may have taken the Bluetooth connection. Do not select the Life Fitness treadmill entry for this bike; select the detected `LF-*` name under **FTMS Bike**.

## Is the BKOOL Smart Bike v1 compatible with QZ?

Yes. QZ includes a dedicated BKOOL bike implementation, and a confirmed support case with a BKOOL Smart Bike v1 connected successfully and could then be used with MyWhoosh through QZ.

This confirms the basic bike-to-QZ and QZ-to-training-app path. It does not imply that MyWhoosh virtual gear difficulty will feel identical to the bike's previous native integration; gear-range or resistance-feel tuning should be treated separately if needed.

## Is a Bodytone DS60 advertising as SMB1 compatible with QZ and MyWhoosh?

Yes. QZ recognizes Bluetooth fitness bikes advertising with an **SMB1** name through its FTMS bike support. A confirmed Bodytone DS60/SMB1 setup connected to QZ and supported automatic resistance control from MyWhoosh through QZ.

Connect the bike to QZ first and confirm cadence/power data and resistance control in QZ. Then enable QZ's normal virtual Bluetooth bike and pair **MyWhoosh with QZ**, not directly with the physical bike. If the bike is not detected, check its advertised Bluetooth name and ensure no other app has taken its Bluetooth connection.
