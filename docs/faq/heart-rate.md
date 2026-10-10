# Heart rate

## My external or companion heart rate is available, but QZ keeps using the heart rate from the fitness machine. What should I do?

If the bike, treadmill, rower, or other fitness machine reports its own heart-rate value, QZ can prefer that built-in value over an external source. This can make a heart-rate source connected through QZ Companion or another supported external HR source appear not to update QZ correctly.

In **Heart Rate Options**, enable **Disable HRM from the machinery**. This tells QZ to ignore the heart-rate value supplied by the fitness machine so that the configured external heart-rate source can be used instead.

This setting is especially useful when the machine reports a stale, zero, or otherwise unwanted heart-rate value while the external source is working correctly.

In a confirmed support case, heart rate from QZ Companion was working but was not being reflected correctly in QZ while using a bike. Enabling **Disable HRM from the machinery** resolved the issue.

## My Garmin watch stops broadcasting heart rate when QZ connects. What should I check?

If heart-rate broadcasting works before QZ connects but stops when QZ establishes its connection, inspect the watch's saved sensor connections. In **Garmin Sensors & Accessories**, look for a previously paired QZ/virtual fitness sensor and remove that old pairing if it is no longer needed. Then restart heart-rate broadcasting and reconnect QZ.

A confirmed support case recovered continuous heart-rate broadcasting after an obsolete QZ sensor pairing was removed from the Garmin watch. This is a troubleshooting step for an existing unwanted sensor pairing, not a requirement to delete every Garmin sensor or to disable heart-rate broadcasting.
