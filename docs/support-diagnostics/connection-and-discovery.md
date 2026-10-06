# Connection and device discovery

## QZ does not find or connect to the fitness machine

### Provide first

- The exact machine make and model.
- A screenshot from **nRF Connect** showing the machine in the Bluetooth scan, including the advertised Bluetooth device name.
- The phone/tablet/computer running QZ and its operating system.
- Whether the manufacturer's app or another fitness app is currently open or connected to the machine.
- What QZ shows while searching: the expected device name, another device, or continuous scanning without a stable connection.

### If still unclear

- A screenshot of the relevant QZ device/settings page.
- Close the manufacturer's app and any other app that may be connected to the machine, restart QZ, and report whether discovery changes.
- Enable QZ Debug Log, reproduce the connection attempt, stop the session, and provide the generated log.

### Why it matters

- The commercial model name alone is often insufficient to identify the Bluetooth protocol; the advertised device name helps map the hardware to QZ's supported device handlers.
- Many fitness machines allow only one active Bluetooth client. Knowing which apps are open distinguishes QZ discovery problems from a connection already occupied by another app.
- Platform and OS information helps separate device/protocol problems from platform-specific Bluetooth behavior.
- A debug log is most useful after the basic topology and advertised device identity are known, so it should not be the first request in a simple discovery case.
