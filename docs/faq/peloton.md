# Peloton consoles

## QZ does not reach the metrics screen on an original Peloton Bike. What should I check?

On the original Peloton Bike (Bike V1), use the QZ build specifically prepared for that console, rather than the generic Android build or the Bike+ variant. The current nightly release contains a separate Peloton Bike V1 package.

If QZ stays on its opening screen with an older installation, update to the current Peloton Bike V1 release, restart the console, and launch QZ after the console has finished booting. The console integration reads its local bike service; it does not require selecting a Bluetooth fitness device, so **Fitness Device = Disabled** is expected.

In a confirmed support case, updating the original Bike from an outdated QZ console build restored normal startup on the first attempt. For installation conflicts, follow the [Android test-build installation FAQ](android.md) and back up local settings before removing an existing installation.
