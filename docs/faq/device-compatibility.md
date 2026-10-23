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

1. Under **QZ Settings > Bike Options > Flywheel Bike Options**, enable **Life Fitness IC5** and restart QZ before connecting to the bike.
2. Power on the IC5 console, then scan and check whether QZ receives live bike metrics.
3. If the console is not discovered, scan it with **nRF Connect** and provide the advertised **Bluetooth name and services**, together with the IC5 console model/firmware. Different console versions can expose different Bluetooth behavior.

QZ's current driver sends the IC5-specific initialization packets when this option is enabled. This is **code-verified setup guidance**, not confirmation that every IC5 console variant supports all metrics or resistance commands. In particular, a console that advertises unexpected services needs verification before compatibility can be promised.

**Confirmed IC5 discovery workaround:** In one case the bike initially connected without RPM and then stopped connecting after a forced **FTMS Bike** selection was disabled. The console was advertising as `BIKE 76`. After shortening its advertised Bluetooth name to `BIKE 7` (while using the dedicated IC5/Flywheel setup), the user confirmed QZ worked. If your console supports changing its Bluetooth name, this is worth testing for a similarly named IC5, but it is **not a universal IC5 requirement**. The sequence involved more than one configuration change, so the precise cause was not isolated. Confirm the advertised name with nRF Connect before changing anything.

## Does QZ support the Dynamax Sprint Bike, and why might it be detected as a treadmill?

Yes. QZ supports a Dynamax Sprint Bike advertising a Bluetooth name beginning with **DYNAMAX** through its FTMS bike driver. Because some Dynamax treadmills use a similar name, **select the bike explicitly**:

1. Open **QZ Settings > Bike Options > FTMS Bike** and choose the exact Bluetooth name of the Dynamax bike.
2. Fully restart QZ and pedal to check whether power and cadence update.
3. If the readings remain wrong, capture a debug log and compare the Bluetooth name with the selected FTMS Bike.

QZ's current discovery tests explicitly distinguish a DYNAMAX FTMS bike selected under **FTMS Bike** from a DYNAMAX treadmill using the same name prefix. A support user subsequently reported that the bike appeared to be working. Earlier unusual speed/distance values were not independently confirmed fixed, so check those readings before relying on them.

## Does QZ recognize the Van Rysel D100 trainer and work with Zwift Click?

QZ has specific FTMS support for the Van Rysel D100 when it advertises as **VANRYSEL-HT**. Connect the trainer to QZ first and confirm that the live power reading changes while pedalling.

For **Zwift**, let Zwift pair directly with Click and enable **Get Gears from Zwift** in QZ; disable QZ's own **Zwift Click** and **Zwift Play** accessory connections. Pair QZ's virtual trainer in Zwift as Power Source and Controllable. For use with other training apps, QZ can instead own the Click controller: disable **Get Gears from Zwift** and enable **Zwift Click**. See [Zwift and virtual gearing](zwift.md) for the full pairing sequence and the Click V2 handshake procedure.

This is **code-verified setup guidance**: the current QZ FTMS driver explicitly recognizes `VANRYSEL-HT`.

**Confirmed D100 + Click V2 resistance fix:** If QZ shows Click V2 gear changes but the D100's **physical resistance does not change**, go to **QZ Settings > Bike Options > FTMS Bike** and select the trainer's **exact advertised `VANRYSEL-HT-...` name** instead of leaving that selector at **Disabled**. Fully restart QZ, reconnect the trainer and Click V2, and test QZ's **local shifting without Zwift running** first. One rider explicitly confirmed resistance control worked after this change. This confirms that particular local-shifting setup, not every possible Zwift-controlled pairing mode.

## Can I use a ProForm Carbon TLX treadmill with QZ and a training app?

QZ has dedicated **ProForm Carbon TLX** support, including a separate treadmill profile for firmware **84.314**. Select the profile that matches the treadmill's firmware under **QZ Settings > Treadmill Options > NordicTrack Treadmill Options**; use the **84.314** variant only if the installed firmware matches. Restart QZ and first confirm that the treadmill's live speed/incline values appear. Test any manual incline adjustment cautiously before relying on route-driven incline changes.

For a typical Zwift setup, run QZ on one phone/tablet and the training app on another device, and pair the app to QZ's virtual device. Zwift route-based incline also requires QZ's separate **Zwift auto-inclination** configuration.

A Carbon TLX owner reported that the setup was working after receiving these instructions, and QZ's treadmill driver contains both Carbon TLX profiles. The reply did **not** separately identify the firmware or prove that **automatic incline in both Zwift and Kinomap** worked, so do not assume those individual capabilities without testing.
