# Virtual device settings diagnostics

## Training application cannot discover QZ

### Provide first

- Confirm that live machine metrics update in QZ.
- Identify the device running QZ and the device running the training application.
- Share a screenshot of QZ Experimental Settings showing all enabled virtual device options.
- Check whether the expected QZ advertisement appears in a Bluetooth scan from the receiving device.

### If still unclear

- Compare the scan before and after disabling unused virtual modes, one option at a time.
- If no QZ advertisement appears, provide a short debug log from a session with the machine connected.
- On Android, check the existing FAQ about device name length and Bluetooth advertising.

### Why it matters

- A working physical machine connection does not establish that the correct virtual Bluetooth profile is being advertised.
- The selected virtual modes determine which protocol the receiving application can discover.
- A receiving-device scan separates Bluetooth advertising issues from the training application's device filters.

## Physical bike disconnects only while QZ advertises a virtual Bluetooth device

### Provide first

- Bike model, QZ device/Android version, QZ version, and whether the physical connection and metrics recover automatically.
- Whether Virtual Bluetooth is enabled and whether the receiving training app is running during the disconnect.
- Time from initial bike connection to disconnection, and whether the problem repeats at a similar interval.

### If still unclear

- Capture two short comparable QZ debug logs: Virtual Bluetooth enabled and disabled, with the same physical bike and no other settings changed.
- Record the approximate disconnect timestamp and whether power/cadence/resistance resume afterward.
- If Android battery optimization is suspected, report the current battery restriction setting; change it separately from the virtual-device comparison.

### Why it matters

- A repeatable disconnection only while virtual advertising is enabled distinguishes an interaction with the Bluetooth stack from a failed bike authentication or a general sensor problem.
- In a confirmed comparison on an older Android tablet, physical-bike connectivity remained stable with virtual advertising disabled, while direct bike authentication worked in both configurations.
