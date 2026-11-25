# Stryde Android 8.1: privileged QZ serial access (experimental)

The Stryde tablet's original `com.katanafit.beta` is a privileged system app. Its current update is installed under `/data/app`, but the original `/system/priv-app/stryde` package retains the **PRIVILEGED** status and `android.permission.SERIAL_PORT: granted=true`. A normal sideload of QZ does not have that status. This is why the first QZ Stryde UART build never reached telemetry.

QZ now requests `android.permission.SERIAL_PORT` on Android API 27 or earlier; **declaring the permission is not enough**. A system-authorized installation is also required.

## Safety and prerequisites

- Test only on a bike/tablet you own or have permission to modify.
- Preserve the original Stryde app, its system partition, and recovery options. Do not replace `com.katanafit.beta`, alter `/dev/ttyS*` modes, or disable SELinux.
- Privileged installation requires root/system-image modification or an OEM-supported system-app deployment path. Merely enabling USB debugging/ADB does not provide this.
- Bootloader unlocking and rooting can wipe data or leave the tablet unbootable; do **not** attempt either without identifying the exact board, firmware, and known-good recovery procedure.

## Stage 0: Verify MTKClient access (no flash changes)

The Stryde/Foenix documentation lists a MediaTek MT8167A. MTKClient has a configuration for HW code `0x8167`. **Hardware similarity does not prove that this particular OEM firmware is accessible.** The tablet currently has `ro.build.type=user`, `ro.debuggable=0`, no `su`, and verified boot reported as green.

Start with **detection only**. Do not unlock, erase, write, or patch any partitions. This first step needs a host computer with a data-capable USB cable; it is not run from ADB on the Android tablet.

Official instructions: [MTKClient](https://github.com/bkerler/mtkclient), [Windows setup](https://github.com/bkerler/mtkclient/blob/main/README-WINDOWS.md), [Linux/macOS setup](https://github.com/bkerler/mtkclient/blob/main/README-INSTALL.md).

On Windows 10/11, install Python, Git, and the official README's MediaTek port/UsbDk drivers on the **computer**. Then run:

```powershell
git clone https://github.com/bkerler/mtkclient
cd mtkclient
python -m pip install -r requirements.txt
python mtk.py gettargetconfig
```

On Linux, follow the official README for libusb, udev rules and Python dependencies; run the last command as `python3 mtk.py gettargetconfig`.

With the detection command already running, completely power down the tablet using its normal power-off procedure, disconnect the USB data cable if needed, and reconnect it to the computer. The official MTKClient procedure suggests holding Volume Up + Power or Volume Down + Power when the hardware offers those buttons; **do not assume the Foenix has such buttons**, and do not dismantle the bike or short any test points. For a tablet without volume buttons, first try reconnecting in its normal powered-off state. If normal power-off/restart is not straightforward, stop and report that instead.

Collect the complete terminal output, including any errors. If no target is found, note whether Windows Device Manager or Linux `lsusb` shows a transient MediaTek USB device and provide its VID:PID, along with the computer OS. Do not run undocumented `adb reboot edl`, crash/bootrom payloads, or driver-replacement utilities without a separate review.

If `gettargetconfig` reports the hardware code and security status, **stop there**. In the next controlled stage, `python mtk.py printgpt` can enumerate partitions, and `python mtk.py r boot boot.img` or `python mtk.py rl backup` can read data, but these can invoke a download agent/exploit in RAM. They should only be considered after reviewing the first output and confirming a reboot and recovery path. Backups may contain personal information and security material; do not share full flash images publicly.

The upstream MTKClient root workflow explicitly reports testing on Android 9–12. Do not treat its `seccfg unlock`, `vbmeta`, `w` or `e` commands as validated for the Foenix Android 8.1 firmware.

## Step 1: Read-only capability check over ADB

Run these commands and review/redact device identifiers before sharing output:

```sh
adb shell getprop ro.build.type
adb shell getprop ro.debuggable
adb shell getprop ro.build.fingerprint
adb shell getprop ro.boot.verifiedbootstate
adb shell id
adb shell command -v su
adb shell su -c id
```

The last command may display a root-authorization prompt or return `su: not found` / `permission denied`. The test does not require modifying the system. Do not assume that `adb shell` or `adb root` has root access on a production firmware.

Also verify the existing QZ and Stryde permission state:

```sh
adb shell dumpsys package com.katanafit.beta
adb shell dumpsys package org.cagnulen.qdomyoszwift
```

Look at `codePath`, `privateFlags`, requested permissions and `android.permission.SERIAL_PORT: granted=...`. The Stryde app already demonstrated `PRIVILEGED` and `granted=true`; the sideloaded QZ did not.

## Step 2: Privileged QZ deployment **only after root and recovery are confirmed**

The preferred reversible approach is a **systemless privileged-app module** compatible with the tablet's Android version, if a root solution is already present and supported. The equivalent direct system-image layout is:

```text
/system/priv-app/QZ/QZ.apk
/system/etc/permissions/privapp-permissions-qz.xml
```

For module-based deployment the paths above live under the module's `system/` directory and are mounted at boot, rather than replacing the real OEM partition. Use the same `org.cagnulen.qdomyoszwift` package as QZ; do not rename the package to the Stryde app.

The allowlist file content is:

```xml
<?xml version="1.0" encoding="utf-8"?>
<permissions>
    <privapp-permissions package="org.cagnulen.qdomyoszwift">
        <permission name="android.permission.SERIAL_PORT"/>
    </privapp-permissions>
</permissions>
```

When installing into a writable system image, directory/file permissions are typically `0755`/`0644`, with system ownership and correct SELinux labels. **Do not run remount, copy, chmod or reboot commands on an unknown device**: first confirm its filesystem layout and recovery plan. Whitelist behavior can vary on OEM firmware. Keep the module disabled until the matching APK and rollback path are validated.

If the tablet already has QZ installed under `/data/app`, verify that the system copy and subsequent updates are signed with a compatible certificate. A data-app update may keep the `/data/app` code path while inheriting privilege from the system copy, as demonstrated by Stryde, but a signing mismatch will prevent a normal update. Back up QZ settings before any reinstall.

## Step 3: Verify grant before exercising serial I/O

After a controlled privileged installation and reboot, collect:

```sh
adb shell dumpsys package org.cagnulen.qdomyoszwift
adb shell pm path org.cagnulen.qdomyoszwift
adb shell getenforce
```

Expected package state: `privateFlags` includes `PRIVILEGED`, and `install permissions` includes `android.permission.SERIAL_PORT: granted=true`. SELinux should remain `Enforcing`.

In QZ, enable **Stryde Bike Options**, fully close the original Stryde app, start QZ, pedal gently, and collect the debug log. The Java bridge logs `PERMISSION SERIAL_PORT GRANTED` before opening the OEM serial service. Only then examine actual port name, `OPEN`, `TX`, `RX`, and decoded cadence/resistance/power.

The observed `/dev/ttyS0` through `/dev/ttyS3` are `root:root` and mode `0600`. This does **not** block use of Android's privileged `SerialManager` service, which performs access on behalf of an authorized app. An empty `dumpsys serial` is not sufficient evidence that the service is absent.

## Failure interpretation

- `PERMISSION SERIAL_PORT DENIED`: QZ is not authorized as a privileged system app, or the permission is missing from the effective package state.
- `PERMISSION ... GRANTED` but no port / `openSerialPort` error: inspect the OEM serial-service port enumeration, path, and reflection/API compatibility.
- `OPEN` but no `RX`: inspect UART initialization/polling/protocol, not Android permission.
- Unexpected `RX` or wrong metrics: the five-byte framing and watt formula are still experimental; preserve raw logs and compare to the Stryde screen.

The initial goal is **read-only telemetry**. Do not enable write operations beyond the already implemented polling command or any resistance-control commands until the hardware protocol is validated.
