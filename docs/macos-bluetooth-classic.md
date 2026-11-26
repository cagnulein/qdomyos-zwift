# Native macOS Bluetooth Classic / RFCOMM validation

QZ builds a native desktop DMG for Apple Silicon (`QZ-macos-apple-silicon-trial.dmg`) using Qt 5.15.19.
It is not an iPad app. The desktop Mac build always has `LICENSE` defined in qmake and
the existing ten-minute trial / Patreon license handling remains active. CI checks that
`-DLICENSE` appears in the macOS compiler flags and fails the build if it is absent.

## Scope

On **macOS only**, QZ now requests every Bluetooth discovery method supported by the
installed Qt backend (Bluetooth Classic + BLE where both are available), even when no
specific Classic trainer option is enabled. The existing device-specific RFCOMM/SPP
drivers are unchanged. In particular, this is *discovery*, not a new generic RFCOMM
trainer protocol. Android, iOS, Linux and Windows discovery policy is unchanged.

Qt on macOS uses IOBluetooth for Classic and can require pairing/encrypted RFCOMM;
Android's `NoSecurity` socket workaround is not portable. Some devices also stop
being discoverable after pairing or when not in pairing mode. If a bonded device
does not appear in discovery, direct connection by saved address may require an
additional device-specific change.

## Apple Silicon test procedure (real Mac + trainer required)

1. Open the successful `macos-apple-silicon-trial` GitHub Actions artifact and install
   `QZ-macos-apple-silicon-trial.dmg`. Verify Activity Monitor shows an Apple native
   app, and allow Bluetooth access in macOS System Settings.
2. Start QZ with Bluetooth enabled, with no special Classic setting. Check the QZ
   debug log for `macOS Bluetooth discovery methods: Classic=1 BLE=1` (or an explicit
   `Classic=0` when the Qt/macOS backend does not support it).
3. Put a Classic SPP fitness device in discovery/pairing mode; make sure its device name
   is listed, then enable its existing driver option if needed (for example:
   `bh_spada_2` for Kettler Racer S or Coach S; `trx_route_key` for iConsole).
4. Connect and check driver telemetry; for controllable devices test a manual
   resistance/speed request, then disconnect/reconnect. Check for any
   `macOS Bluetooth discovery error` and driver-specific RFCOMM errors in logs.
5. Turn Bluetooth off, then on; retry. Check an ordinary BLE FTMS device still
   discovers and connects as before.
6. Keep QZ running for over ten minutes without a valid supporter license and verify
   the existing trial expiry prompt is still enforced. With a valid supporter email,
   verify that the existing license check still approves it.

## CI limitations

The macOS DMG smoke job verifies app architecture, packaging, QML startup, and the
compile-time `LICENSE` definition. GitHub-hosted macOS runners have no paired fitness
hardware; successful CI does **not** prove Classic inquiry, SDP, RFCOMM pairing,
telemetry or control work end-to-end. Those need a physical Mac and device.

Do not merge on CI alone if the target device has not been tested on macOS.
