# NordicTrack / ProForm iFIT consoles

## Can I run QZ directly on a NordicTrack or ProForm console without QZ Companion?

Yes. On supported Android-based NordicTrack and ProForm consoles, QZ can be installed directly on the machine. QZ Companion is not required.

There are two different QZ builds for this. Choose the build according to the iFIT interface installed on the console and whether iFIT will remain running.

| Method | When to use it | iFIT state | How QZ talks to the machine |
|---|---|---|---|
| **gRPC** | Newer consoles with the new iFIT **3D interface** | **iFIT must be installed, signed in, and running** | QZ talks to iFIT over gRPC, and iFIT talks to the machine |
| **FitPro USB** | Old or new iFIT versions, or when direct hardware access is preferred | **iFIT must be completely stopped/disabled** | QZ talks directly to the machine hardware over USB-HID |

Do not use the two methods at the same time.

### Which APK should I install?

Download the appropriate APK from the latest QZ nightly release:

<https://github.com/cagnulein/qdomyos-zwift/releases/latest>

#### gRPC builds

These builds work **only with the newer iFIT 3D interface** and require iFIT to remain active.

- Treadmill: `android-debug-nordictrack-treadmill.apk`
- Bike: `android-debug-nordictrack-bike.apk`
- Rower: `android-debug-nordictrack-rower.apk`

There is currently no NordicTrack gRPC elliptical build.

#### FitPro USB builds

These builds access the exercise machine hardware directly over USB-HID and do not depend on the iFIT software interface. They can therefore be used with both older and newer iFIT versions.

**iFIT must not be running while using a FitPro USB build.** If necessary, force-stop or disable iFIT before starting QZ so that iFIT does not keep control of the hardware.

- Treadmill: `android-debug-fitpro-treadmill.apk`
- Bike: `android-debug-fitpro-bike.apk`
- Elliptical: `android-debug-fitpro-elliptical.apk`
- Rower: `android-debug-fitpro-rower.apk`

## How do I install QZ directly on the console?

The procedure is similar to installing QZ Companion: you first need access to the Android system on the NordicTrack / ProForm console, then install the appropriate QZ APK.

### Method 1: install from the console browser

This is usually the simplest method.

1. Enter **Privileged Mode** / the Android desktop on the console.
   - On many iFIT consoles this is done from the main iFIT screen by tapping an unused area 10 times, waiting about 7 seconds, and tapping the same area another 10 times.
   - If the console asks for a privileged-mode response code, follow the procedure supported by that console/iFIT version.
2. Open the Android **Browser** on the console.
3. Go to the QZ releases page:
   <https://github.com/cagnulein/qdomyos-zwift/releases/latest>
4. Under **Assets**, download the APK that matches your machine and the communication method described above.
5. Open the downloaded APK.
6. If Android blocks the installation, open the requested Settings page and enable **Allow from this source** for the browser/file manager, then start the APK installation again.
7. Launch QZ from the Android app list.

For screenshots and examples of entering the Android environment and installing an APK on an iFIT console, the older QZ Companion installation guide is also a useful reference:

<https://github.com/cagnulein/QZCompanionNordictrackTreadmill>

A factory reset is not normally required just to install the direct QZ APK.

### Method 2: install over ADB from another computer

If USB/Wi-Fi debugging is available on the console, the APK can also be installed with ADB.

1. Enter the Android settings on the console.
2. Enable **Developer options** and **USB debugging** / ADB access.
3. Note the console IP address.
4. From a computer with ADB installed, connect to the console, for example:

```text
adb connect CONSOLE_IP:5555
```

5. Install the APK you downloaded from the latest QZ release:

```text
adb install -r android-debug-nordictrack-treadmill.apk
```

Replace the APK filename with the gRPC or FitPro build appropriate for your machine.

## What do I do after installation?

### If you installed a gRPC build

1. Keep **iFIT installed and enabled**.
2. Sign in to iFIT.
3. Start iFIT and leave it running. The gRPC build depends on the active iFIT application to communicate with the machine.
4. Start QZ on the console and configure QZ for the app or virtual device you want to use.
5. iFIT can remain in the foreground while QZ runs in the background.

If your console does not have the newer iFIT 3D interface, the gRPC build will not work. Use the FitPro USB build instead.

### If you installed a FitPro USB build

1. Make sure **iFIT is completely stopped**. Force-stop or disable it if necessary.
2. Start the FitPro version of QZ.
3. QZ will access the machine hardware directly over USB-HID, without iFIT or QZ Companion in the middle.

If iFIT is running at the same time, it may keep control of the USB hardware and prevent QZ from communicating correctly.

## Do I still need QZ Companion or a second device?

QZ Companion is not required with either of these direct-console methods. QZ itself runs on the NordicTrack / ProForm Android console and communicates with the exercise machine using either gRPC through iFIT or direct FitPro USB-HID access.

If QZ Companion was previously installed, **removing it is recommended but not strictly required**. The direct-console QZ builds do not use it anymore, so keeping it installed provides no benefit. Uninstalling it also keeps the setup simpler and avoids accidentally starting an obsolete bridge that is no longer needed.

You may still use another device for Zwift, Rouvy, or another training application, depending on your setup.

## Can QZ FitPro USB control speed and incline on a NordicTrack T9 without running iFIT?

**Yes, manual speed and incline control on a NordicTrack T9 has been explicitly confirmed** using QZ FitPro USB running directly on the treadmill's Android console.

1. Use the **FitPro USB treadmill** build (`android-debug-fitpro-treadmill.apk`) on the console. Keep **iFIT fully stopped** so it does not compete for the USB-HID hardware; an active iFIT subscription is not needed for this direct-hardware method.
2. Start QZ on the treadmill console, confirm that treadmill telemetry updates, and test a **small manual speed and incline adjustment** while remaining able to stop the treadmill safely.
3. To create an interval session on the same console, open **QZ → Workout Editor**, choose **Treadmill**, and enter the desired speed and incline steps. QZ's editor supports treadmill intervals and **Save & Start**; see the [Workout Editor guide](../workout-editor.md).

The **confirmed T9 test covers manual speed and incline commands**, not yet a complete automatically controlled interval workout. Verify short, gentle intervals on the specific console before relying on programmed transitions.

A separate iPhone running QZ is **not required** for this local-console workflow. Remote creation/start of FitPro workouts **from another QZ instance over Wi-Fi** is a different workflow and should not be assumed to work without a separately tested client connection. The gRPC build is likewise a different mode: it needs a newer iFIT 3D interface and iFIT running, whereas FitPro USB does not.

## How do I use QZ with a ProForm Smart Power 10.0C bike with an older iFIT console?

For the **ProForm Smart Power 10.0C (PFEX69420C.1)**, a confirmed setup runs QZ **directly on the bike's Android console**, rather than connecting QZ on a phone using only the bike's IP address.

1. Enter the console's **Privileged Mode** and open its Android browser (see the console installation instructions above).
2. Download and install the **FitPro USB bike** build, `android-debug-fitpro-bike.apk`, from the [QZ nightly releases](https://github.com/cagnulein/qdomyos-zwift/releases). Do not use the standard phone APK or the gRPC build for this older iFIT interface.
3. **Completely stop iFIT** before starting QZ so it does not retain control of the bike's USB hardware.
4. In QZ on the console, set **TDF Companion IP** under **NordicTrack bike options** to `localhost`, and **Max Resistance** under **Bike Options** to **22** (the bike's resistance-level count). Enable **Virtual Bluetooth Device**.
5. Pedal and verify cadence, power, and resistance in QZ. For Zwift on a **separate** device, pair the QZ virtual trainer (often **Wahoo KICKR 0000**) as the power source and controllable trainer.

A rider explicitly confirmed that the direct-console setup worked. **Automatic terrain-driven resistance from Zwift was not yet confirmed in that case** and should be tested separately; seeing a target resistance in QZ does not prove that the bike applied it. This setup does not run Zwift on the bike's iFIT display.
