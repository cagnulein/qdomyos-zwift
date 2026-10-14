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
