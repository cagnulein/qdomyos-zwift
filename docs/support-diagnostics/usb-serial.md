# USB and serial device diagnostics

## A USB-connected ergometer shows no metrics in QZ on Android

### Provide first

- Exact ergometer model and how it connects: a USB cable through an Android OTG adapter, an internal serial port, or Bluetooth. State the phone/tablet model and Android version.
- Whether Android detects the USB peripheral and asks to grant QZ access. Report the USB connection mode shown by Android (for example, charging only versus a data-capable connection), with a screenshot if possible.
- The current QZ build and the selected device-specific USB/serial settings. For a Kettler USB bike, include the **Baudrate** under **Settings > Bike Options > Kettler USB Bike Options**.
- State whether speed, cadence, or watts change in QZ while using the machine **before** opening Zwift or another training app.

### If still unclear

- Confirm the cable and OTG adapter support data transfer, and report whether the Android USB permission prompt appears. Check the Android USB connection/default-USB configuration where available; do not assume that a physically connected cable provides a data path.
- For Kettler USB, record the baud rate used in each test. QZ defaults to 9600; another confirmed setup required 57600. Test only plausible device-specific rates, restarting QZ between tests, and record which rate produces changing metrics.
- Provide a QZ debug log from a fresh USB connection attempt, including startup and a short period of pedaling. If the OS never enumerates the USB device, report that separately rather than sending only a downstream Zwift pairing log.
- Once metrics work in QZ, test the virtual trainer connection in the receiving app as a separate step.

### Why it matters

- USB enumeration/permission, serial framing/baud rate, QZ metric decoding, and virtual Bluetooth advertising are separate failure points. A charging-only USB connection cannot be fixed by changing a Zwift pairing role.
- The Kettler USB driver uses Android USB serial access and a configurable baud rate. Identifying the actual transport and setting is more useful than a Bluetooth scan for a wired machine.
- A confirmed USB ergometer case recovered only after establishing a data-capable Android USB connection and selecting the correct serial baud rate. See the [device-compatibility FAQ](../faq/device-compatibility.md#my-older-kettler-usb-ergometer-shows-no-data-through-qz-on-android-what-should-i-check) for the resolved setup, not as a substitute for these intake checks.
