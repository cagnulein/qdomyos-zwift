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
