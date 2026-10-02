# Training programs

## Can I create an incline ramp in a QZ XML training program?

QZ XML training programs support a direct speed ramp with `speedfrom` and `speedto`, but there is currently no generic `inclinefrom` / `inclineto` attribute for an incline ramp. The `inclination` attribute sets the incline for a row.

To simulate a gradual incline change, split the ramp into consecutive short rows and increase or decrease `inclination` a little on each row. For example:

```xml
<row duration="00:00:30" speed="8.0" inclination="1" forcespeed="1" zonehr="0"/>
<row duration="00:00:30" speed="8.0" inclination="2" forcespeed="1" zonehr="0"/>
<row duration="00:00:30" speed="8.0" inclination="3" forcespeed="1" zonehr="0"/>
<row duration="00:00:30" speed="8.0" inclination="4" forcespeed="1" zonehr="0"/>
```

Use as many steps as needed for the smoothness you want and for what your treadmill can safely accept. This stepped-incline workaround, as well as normal `speedfrom` / `speedto` ramps, has been confirmed working in a real QZ training program.

For the full XML attribute reference and speed-ramp behavior, see [`train-programs-examples/README.md`](../../train-programs-examples/README.md).

## Can QZ control treadmill incline instead of speed to keep me in a target heart-rate zone?

Yes. QZ's HR PID controller can use treadmill **incline** as its actuator, so the treadmill speed can remain fixed while QZ raises or lowers incline according to your heart rate.

In **Training Program options**, set **PID HR actuator** to **Inclination**. When a workout-editor interval does not override the actuator, it uses this global selection. The workout editor can also select the PID actuator for an interval; an interval-specific **Inclination** selection overrides a global **Speed** selection for that interval.

When incline is the active actuator, QZ adjusts incline according to the HR PID target while leaving the requested workout speed unchanged. The configured maximum incline is still respected.

If you are testing this with a real treadmill, make sure **Fake Device** is disabled. Fake Device is intended for simulation/testing and can prevent QZ from connecting to the real treadmill.

Both the global Inclination actuator and the workout-editor Inclination override have been confirmed working with a real treadmill.