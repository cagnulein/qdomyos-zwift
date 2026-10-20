# QZ FTMS treadmill to Garmin RSC bridge

This Arduino project turns an ESP32 into a Bluetooth bridge:

1. It scans for a treadmill advertising the standard Fitness Machine Service
   (`0x1826`).
2. It subscribes to the standard FTMS Treadmill Data characteristic (`0x2ACD`)
   and reads instantaneous speed in `0.01 km/h` units.
3. It advertises the standard Running Speed and Cadence service (`0x1814`) for a
   Garmin watch, including the RSC Measurement characteristic (`0x2A53`).

## Cadence calculation

Most FTMS treadmills expose speed but not running cadence. The sketch therefore
uses the same calculation as QZ:

```text
cadence (steps/min) = speed (m/s) / stride length (m) * 60
stride length = height (m) * 0.415 for male users
stride length = height (m) * 0.413 for female users
```

Change `USER_HEIGHT_CM` and `USER_IS_MALE` near the top of
`ftms_treadmill.ino` before uploading. The defaults are 170 cm and male.

## Arduino IDE

- Open `ftms_treadmill.ino` in Arduino IDE.
- Select the ESP32 board connected to the Mac and its serial port.
- Install/select the NimBLE-Arduino library. This project was written for
  NimBLE-Arduino 2.x.
- Upload at 115200 baud and open the serial monitor.
- Start the treadmill. The serial log should show the detected FTMS device,
  its speed, and the calculated cadence.
- On the Garmin, add a new Running Dynamics / running cadence sensor and select
  `QZ FTMS RSC`.

The ESP32 must remain powered while both the treadmill and Garmin are connected.
The bridge does not control the treadmill and does not expose FTMS itself; it
only reads FTMS and publishes RSC.
