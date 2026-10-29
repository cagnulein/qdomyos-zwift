# Heart rate source diagnostics

## Heart rate is missing, wrong, or coming from the wrong source

### Provide first

- Identify the intended heart-rate source: chest/arm belt, watch/companion app, or the fitness machine's built-in HR.
- State where that HR source is connected during the test: directly to QZ, to the treadmill/bike, or to another app/device.
- Report the exact **Heart Rate Belt Name** configured in QZ, or confirm that it is empty.
- Report whether **Disable HRM from the machinery** is enabled.
- If the HR sensor can connect to the fitness machine itself, say whether it is currently connected there.

### If still unclear

- Temporarily test with only one HR path active at a time. For example, if the belt is connected to the treadmill and the treadmill forwards HR to QZ, leave **Heart Rate Belt Name** empty so QZ does not also try to connect directly to the belt.
- If QZ is expected to connect directly to the sensor, confirm that the sensor is advertising and is not already occupied by another device or app.
- Provide screenshots of the relevant QZ Heart Rate Options.
- Enable QZ Debug Log, reproduce a short session while wearing/using the intended HR source, then provide the generated log and say what HR value/source you expected QZ to use.

### Why it matters

- A heart-rate sensor may reach QZ through more than one path. Knowing whether QZ should read it directly or receive it through the fitness machine prevents troubleshooting the wrong connection.
- A configured **Heart Rate Belt Name** tells QZ to look for that external sensor; this can be inappropriate when the sensor is already connected to the machine and the machine is forwarding HR.
- **Disable HRM from the machinery** determines whether HR reported by the fitness machine is allowed to be used. This is especially important when an external HR source is expected to override built-in or machine-forwarded HR.
- Testing one HR path at a time distinguishes sensor-discovery problems from source-selection conflicts before a full debug log is needed.

## Wear OS watch shows heart rate, but QZ on the phone stays at zero

### Provide first

- Phone model, Android version, watch model, Wear OS version, and QZ versions on both devices.
- Confirm the phone and watch are paired and connected, and whether the QZ watch app displays a changing heart-rate value during a started session.
- State whether the phone QZ app receives any heart-rate value while the watch session is running, and whether other machine metrics continue updating.

### If still unclear

- Restart both devices and repeat a short test, noting whether the watch session starts and remains active.
- Provide a short QZ debug log from the phone, the approximate test time, and screenshots showing the watch reading and phone's missing value.
- State whether the behavior began after an Android, Wear OS, or QZ update; test a newer build only if support requests it.

### Why it matters

- A live reading on the watch proves local measurement, not delivery to the phone.
- Distinguishing watch-session startup, watch-to-phone connectivity, and phone-side reception helps isolate the failed stage without confusing it with the bike connection.
- Exact OS and QZ versions are important when the watch and phone communication path changes between platform releases.

## Apple Watch shows heart rate but the iPhone app does not

### Provide first

- iPhone and watch models, operating system versions, and QZ build numbers.
- Confirm that heart rate changes on the watch while the iPhone QZ display remains at zero.
- Confirm that other machine metrics update in QZ before starting the watch session.
- Screenshot the selected heart-rate source in QZ.

### If still unclear

- Run a short test with QZ open on iPhone before starting the watch app.
- Note the time of the test and whether any heart-rate value reaches the phone.
- Share a fresh QZ debug log and screenshots showing both readings.

### Why it matters

- A live watch reading does not confirm delivery to the phone.
- Working machine metrics distinguish watch communication from machine discovery.
- Exact installed builds help determine whether a newer test version needs verification.

## A selected Bluetooth heart-rate strap does not supply live readings

### Provide first

- Strap model, phone operating system, and the exact **Heart Rate Belt Name** selected in QZ.
- Confirm that the strap is being worn and its electrodes are moist or otherwise activated before QZ starts.
- State whether the strap appears only in the settings list or whether QZ displays a changing heart-rate value during exercise.

### If still unclear

- Scan with nRF Connect while the strap is worn and awake; share a screenshot with the advertised device name.
- Check whether the strap broadcasts consistently and whether another app can read live heart rate. Note recent battery replacement if relevant.
- If advertising is confirmed but QZ still receives no readings, collect a QZ debug log from a fresh connection attempt.

### Why it matters

- A previously discovered name in QZ's selection list does not prove that the sensor is currently advertising.
- Checking current advertising before collecting logs distinguishes a sleeping or intermittent strap from a QZ pairing or source-selection problem.

## Apple Watch heart rate reaches QZ on iPhone but not a second QZ on iPad

### Provide first

- Identify the Apple Watch's paired iPhone, the iPad running QZ with the fitness machine, and the QZ/iOS/watchOS versions on each device.
- Confirm where heart rate last appears: on the Watch, in QZ on the paired iPhone, and in QZ's Heart tile on the iPad. Test **before** opening Zwift, MyWhoosh, or another receiving app.
- State whether both iOS devices use the same local Wi-Fi rather than cellular data or an isolated guest network, and whether **Local Network** permission is enabled for QZ on **both** devices.

### If still unclear

- With the fitness machine connected only to the iPad, keep both QZ instances open and repeat a short Watch HR test. Report which of the three displays changes and when.
- Screenshot QZ's Heart Rate Options on the iPad, especially any machine-provided HR or alternate belt source, and the iOS Local Network permission screen on each device.
- Only if the watch-to-phone stage works and the LAN permissions are correct, collect time-aligned QZ debug logs from the iPhone and iPad for the same short test.

### Why it matters

- The Watch sends HR to its paired iPhone; the second QZ instance depends on local-network communication, not direct Watch-to-iPad Bluetooth pairing.
- In a confirmed multi-device case, enabling missing Local Network access for one QZ instance restored the HR relay. This should be checked before investigating trainer Bluetooth or a receiving training app.
- A machine-provided HR value on the iPad can hide a correctly received external source. See the [Apple Watch/iPad FAQ](../faq/heart-rate.md) for setup guidance; this intake isolates the failing stage.
