# Bike and trainer troubleshooting

## QZ detects my bike, but the bike metrics stay at zero. What should I try?

If QZ recognizes the bike but cadence, power, speed, or other bike data are not updating while other sources such as heart rate still work:

1. Open **QZ Settings > Garmin options**.
2. Disable **ANT Bike Garmin**.
3. Fully close and restart QZ.
4. Reconnect the bike and start pedaling.

This can restore the normal bike data stream when the Garmin ANT bike option interferes with the connection.

### Why can incline still show zero?

Bike resistance and incline are different values. A Yesoul bike may report resistance without reporting a real incline value. QZ's incline tile can instead be populated by an external source such as Zwift or a GPX route. A resistance-to-incline conversion requires a specific mapping and should not be assumed from the resistance percentage alone.

## MyWhoosh connects to QZ for my Echelon bike, but power stays at 0 W. What should I check?

If the Echelon bike itself is already working normally in QZ and MyWhoosh can see/connect to the QZ device but the workout data remains at zero, check whether **Virtual Echelon** is still enabled from a previous setup or unlock attempt.

1. Open **QZ Settings > Experimental settings**.
2. Disable **Virtual Echelon**.
3. Fully close and restart QZ.
4. Reconnect the Echelon bike, then pair QZ again in MyWhoosh.

**Virtual Echelon** is intended for the Echelon initialization/unlock workflow. It is not normally needed once the bike can connect and operate directly through QZ. In a confirmed support case, disabling this setting and restarting QZ immediately restored data transmission to MyWhoosh.

If the bike itself is locked and QZ explicitly asks you to enable Virtual Echelon for initialization, follow the on-screen unlock instructions first; this troubleshooting step applies when the bike is already operating normally in QZ but the downstream training app receives no metrics.
