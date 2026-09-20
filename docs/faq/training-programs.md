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
