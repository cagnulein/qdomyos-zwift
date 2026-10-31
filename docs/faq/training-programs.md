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

## My custom QZ XML workout loads but does not run the programmed steps. What basic XML structure should I check?

For QZ's native XML training format, the workout must use a top-level `<rows>` element and each normal workout step must be a `<row>` element. Do not use a generic `<workout>` root or `<segment>` elements.

A minimal valid structure is:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<rows>
    <row duration="00:05:00" speed="5.0" inclination="1" forcespeed="1" zonehr="0"/>
    <row duration="00:10:00" speed="7.0" inclination="2" forcespeed="1" zonehr="0"/>
</rows>
```

If the file is accepted but the treadmill only starts at its minimum speed or the programmed steps do not execute, check these element names before troubleshooting the treadmill connection itself.

This exact correction was confirmed to restore a custom treadmill workout that initially used both `<segment>` and an incorrect top-level `<workout>` element.

For the complete native XML reference, including repeats, heart-rate zones, ramps, and supported row attributes, see [`train-programs-examples/README.md`](../../train-programs-examples/README.md).

## Can QZ control treadmill incline instead of speed to keep me in a target heart-rate zone?

Yes. QZ's HR PID controller can use treadmill **incline** as its actuator, so the treadmill speed can remain fixed while QZ raises or lowers incline according to your heart rate.

In **Training Program options**, set **PID HR actuator** to **Inclination**. When a workout-editor interval does not override the actuator, it uses this global selection. The workout editor can also select the PID actuator for an interval; an interval-specific **Inclination** selection overrides a global **Speed** selection for that interval.

When incline is the active actuator, QZ adjusts incline according to the HR PID target while leaving the requested workout speed unchanged. The configured maximum incline is still respected.

If you are testing this with a real treadmill, make sure **Fake Device** is disabled. Fake Device is intended for simulation/testing and can prevent QZ from connecting to the real treadmill.

Both the global Inclination actuator and the workout-editor Inclination override have been confirmed working with a real treadmill.


## How do I open an existing XML workout in QZ?

Use **Open Train Program** in QZ's main menu to load and run an existing training file. **Workout Editor** is a separate interface for creating and editing workouts, not the required import step.

On Android, use **Open Train Program > Other folders** to select an XML file in Download or another folder. A user confirmed that opening their XML workout through **Open Train Program** resolved confusion about the Workout Editor. Both menu entries and the file picker are present in current QZ code.

## How do I show the remaining time and next workout interval on the QZ dashboard?

In **QZ Settings > Tiles Options**, enable **Remaining Time/Row** and **Next Rows**. The first shows the time left in the current training step; the second previews the upcoming step and its available target and duration.

These tiles are implemented in QZ's current tile settings and training-program display code. The support user had not yet confirmed trying them.

## How often does a heart-rate-controlled XML workout adjust treadmill speed?

Set the `looptimehr` attribute, in seconds, on each HR-controlled XML row. For example, `looptimehr="3"` evaluates the heart-rate target about every three seconds, rather than every two seconds with `looptimehr="2"`. Less frequent updates can reduce competing commands but also slow the HR response; this setting does not change the speed step size.

The XML parser and [training-program authoring guide](../../train-programs-examples/README.md) verify this setting (default: 10 seconds). A support user asked about increasing it from two to three seconds; that adjustment was not separately confirmed to fix command overlap.

## My running ZWO workout imports into QZ but its interval graph is empty. What should I check?

For a **time-based running** ZWO workout, include both the running sport type and a time duration type near the beginning of the `.zwo` file, for example:

```xml
<sportType>run</sportType>
<durationType>time</durationType>
```

QZ's ZWO parser uses **sportType** together with **durationType** to distinguish timed running intervals from distance-based running intervals. Without `durationType=time`, a running workout may be interpreted incorrectly and fail to render its intended interval graph. A rider confirmed that adding the `durationType` element made the graph render.

For workouts that specify `pace=\"4\"`, QZ uses the **Marathon pace** in **Training Program settings** as the reference and scales it using the interval power percentages (for example, `OnPower` and `OffPower`). Match QZ's configured paces to your Zwift paces before comparing treadmill speed targets. Rendering the graph does **not** by itself confirm that treadmill speed control has been tested.

## Why does bike resistance fall when I pedal faster during a QZ workout?

If the workout has a **target power in watts (ERG mode)**, this is expected: the controller changes resistance to keep power near the target. At a higher cadence it generally needs less resistance; when you slow down it increases resistance. This is different from a workout that holds a specified **resistance level**.

To create intervals that follow resistance levels instead of a watt target, open **QZ > Workout Editor**, select **Bike**, and create a resistance-based workout with the desired resistance for each interval. Save and start that workout. A rider confirmed that switching to a resistance-based workout resolved the confusion after observing cadence-dependent resistance in ERG mode. Actual resistance control still requires a compatible controllable bike and the correct QZ device configuration.

## Can I load a GPX route and see my virtual position on a map while using a treadmill?

**Yes.** QZ supports importing a GPX route and displaying it on a map, including 2D/3D map views.

1. Create or export a **GPX** route using a mapping tool.
2. In QZ's main menu choose **Open GPX** and select the route file.
3. Start the treadmill session, then use the **map button** to display the route and follow your progress.
4. The position shown on the route is **virtual**, based on the workout's progress, not the iPhone's real-world GPS position.

The **Open GPX** menu and GPX/map support are present in QZ's current UI and README. A treadmill user explicitly confirmed that an externally created GPX route loaded and displayed correctly on the map during a workout.

## Why does my treadmill not stop automatically when a QZ interval program finishes?

By default, QZ **does not send an automatic Stop command** when the training program reaches its end. The program can finish its intervals without displaying an obvious completion message or stopping the treadmill motor. **Use the treadmill's physical controls when needed and press Stop in QZ** to finish and save the workout.

QZ's training-program code has a `trainprogram_stop_at_end` option, but its default is **false**; only when that option is enabled does the program-end path emit a device Stop request. Do not assume a particular treadmill accepts remote Stop commands. This is code-verified behavior, not a claim that automatic stopping was tested on every treadmill.

## My QZ XML resistance workout loads but stays on the first interval. How can I make it run?

Loading a QZ XML workout and starting the general activity timer are not always the same as **starting the training program**. If the file loads but the interval never advances, check that the training program itself has started before assuming that the bike is ignoring resistance commands.

In a confirmed **Domyos X12** case, the bike connected and the XML contained alternating resistance levels, but no programmed resistance command was sent because the program was still on its first row. The working sequence on that installation was:

1. Connect the bike and load the XML workout in **Open Train Program**.
2. If the workout remains on its first row, **stop and restart the QZ workout session** using QZ's controls after loading the file (do not power-cycle the bike).
3. Watch the interval timer advance and verify that the bike actually changes resistance at the next step.

The rider confirmed that stopping and restarting the QZ session was necessary each time on that build. A subsequent debug log showed QZ sending resistance **6** and then **9** about a minute apart, with the bike reporting those levels. This is a **confirmed workaround for an affected start/stop workflow**, not a requirement for every QZ workout or device.

## How can I align QZ's workout interval timing with a workout playing in Zwift or Peloton?

In **QZ Settings > Tiles Options**, enable **Peloton Offset** and **Remaining Time/Row**. During the workout, use the **Peloton Offset** tile to shift QZ's workout timeline:

- Press **+** if QZ's interval transitions are **behind** the external workout's callouts.
- Press **-** if QZ's transitions are **ahead**.

A treadmill user explicitly confirmed that the **Peloton Offset** tile aligned the transitions of a QZ workout with a workout in Zwift. The tile adjusts QZ's interval timing; it does **not** eliminate Bluetooth/network latency between Zwift, QZ and the treadmill, and it does not change the external workout itself.
