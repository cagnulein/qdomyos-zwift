# QDomyos-Zwift Training Programmes — Authoring Guide

## XML Format Reference

```xml
<?xml version="1.0" encoding="UTF-8"?>
<rows>
    <row duration="HH:MM:SS" speed="7.0" inclination="1" forcespeed="1" zonehr="0"/>
    <row duration="00:10:00" zonehr="2" looptimehr="10" minspeed="7.0" maxspeed="10.0"/>
    <row duration="00:02:00" speedfrom="5.0" speedto="8.0" forcespeed="1" zonehr="0"/>
    <repeat times="6">
        <row duration="00:01:30" zonehr="4" minspeed="10.0" maxspeed="14.0"/>
    </repeat>
</rows>
```

### Row Attributes

| Attribute | Type | Description |
|-----------|------|-------------|
| `duration` | `HH:MM:SS` | **Required.** How long the row lasts. |
| `distance` | km | Row ends after this distance instead of `duration`. |
| `speed` | km/h | Fixed speed target. |
| `speedfrom` / `speedto` | km/h | Speed ramp — QZ auto-splits into 0.1 km/h, 1-second increments. |
| `inclination` | % | Treadmill incline. |
| `forcespeed` | `1` | Request the programmed treadmill speed when the row starts (without this, speed is advisory only). This does not lock out other automatic controllers. See §2 for HR PID settings. |
| `zonehr` | 0–5 | `0` explicitly disables HR zone control; `1`–`5` select an HR target zone. Omit the attribute to leave the current HR zone setting unchanged. Make sure you correctly set your max heart rate (Settings > Heart Rate Options > Heart Rate Zone Options > Heart Rate Max Override > Max Heart Rate). See §1 and §2. |
| `hrmin` / `hrmax` | bpm | Custom HR target range (alternative to `zonehr`, more precise). PID targets the midpoint. Avoid combining a fixed-speed target with HR-driven speed adjustments unless that is intentional. |
| `looptimehr` | seconds | How often the HR PID adjusts speed (default 10). Lower = more responsive. |
| `minspeed` / `maxspeed` | km/h | Speed bounds for HR PID adjustments. **Always set these on HR-zone-controlled speed rows (`zonehr` 1–5).** |
| `resistance` | 1–100 | Bike resistance. |
| `cadence` | RPM | Target cadence (bike/rower). |
| `power` | watts | Power target. |
| `maxresistance` | int | Max resistance the HR PID may set. |
| `fanspeed` | 0–100 | Fan speed percentage. |

### Text Events (on-screen messages)

```xml
<row duration="00:10:00" zonehr="2" minspeed="7.0" maxspeed="10.0">
    <textevent timeoffset="5" message="Zone 2 — keep your back straight"/>
</row>
```

`timeoffset` is seconds from the start of the enclosing row.

---

## Tips when using a heart rate sensor

### 1. HR Zone 1 Does Not Work with the PID Controller

The PID HR controller adjusts speed **up** when HR is below target and **down** when above. Zone 1 is so low that the controller keeps reducing speed toward zero and never meaningfully recovers. **Do not use `zonehr="1"` as a working control zone.** Use it only as a label/cosmetic marker if desired, but pair it with a fixed speed row instead:

```xml
<!-- WRONG — PID will drive speed to near-zero -->
<row duration="00:03:00" zonehr="1" minspeed="5.0" maxspeed="8.0"/>

<!-- RIGHT — use a fixed speed for warm-up/cool-down instead -->
<row duration="00:03:00" speed="5.5" inclination="0" forcespeed="1" zonehr="0"/>
```

### 2. Fixed Speed, HR Zones and the Workout Editor

There are three distinct ways to specify HR zone control:

- **Omit `zonehr`:** leave the current HR zone setting unchanged. For a speed/incline-only workout, this is sufficient if the HR zone PID is already disabled and global HR min/max control is also off.
- **`zonehr="0"`:** explicitly disable HR zone control. This is important when switching from an HR-zone interval to a fixed-speed recovery or cool-down; otherwise the preceding HR zone can remain active (see #4671 and #4672).
- **`zonehr="1"` through `"5"`:** select a heart-rate target. A forced speed can intentionally seed the HR controller, or be used with the Inclination HR PID actuator. It does not by itself prevent subsequent speed adjustments.

The built-in **Workout Editor** automatically sets `forcespeed="1"` when a treadmill speed/pace is enabled. It does not automatically add `zonehr="0"`. For a mixed workout, set **HR Zone** to 0 in Advanced options on intervals where HR zone control should be explicitly disabled. Leaving that field unset is not equivalent to setting it to zero.

**Important — unexpected speed changes on incline transitions:** Even with `zonehr="0"`, the current incline-related PID compensation can still issue speed commands when the incline changes (see [#5520](https://github.com/cagnulein/qdomyos-zwift/issues/5520)). Before running fixed-speed workouts that change incline, enable **Settings > Training Program Options > PID Ignore Inclination** and fully restart QZ. This is a verified workaround, not a change to the controller. It is separate from **Treadmill Auto-adjust speed by power**. Verify the workout at a safe low speed with the safety stop accessible before relying on automatic control.

```xml
<!-- Speed/incline-only workout: HR zone PID already disabled, HR min/max off -->
<row duration="00:03:00" speed="5.5" inclination="0" forcespeed="1"/>

<!-- Mixed workout: explicitly disable the previous HR zone for recovery -->
<row duration="00:05:00" zonehr="2" minspeed="6.0" maxspeed="8.0"/>
<row duration="00:03:00" speed="5.5" inclination="0" forcespeed="1" zonehr="0"/>
```

For fixed-speed intervals, also leave global HR min/max control disabled unless deliberately using HR control. `zonehr="0"` is a zone-control directive, not a general override for every automatic speed-control feature.

### 3. Seed Speed Before HR Zone Rows (Especially when there are large speed changes)

When transitioning to an HR zone row, the PID inherits whatever speed the treadmill is currently at. If the previous row was at a very different speed (e.g., 12 km/h sprint → Zone 2 recovery), the belt is still running fast when the PID takes over. The PID adjusts in small 0.2 km/h steps every `looptimehr` seconds, so it can take many cycles to bring speed down — meanwhile you're running much harder than intended.

**Insert a short `forcespeed` row (5 seconds) to set a sensible starting speed:**

```xml
<!-- After a high-intensity interval -->
<row duration="00:00:05" speed="8.0" inclination="0" forcespeed="1" zonehr="0"/>
<row duration="00:01:30" zonehr="2" looptimehr="5" minspeed="7.5" maxspeed="8.5"/>
```

This immediately brings the treadmill to a reasonable speed before the PID takes over. Without it, you could be running at sprint pace for 30+ seconds while the PID slowly ramps down.

For adjacent zone transitions (e.g., Zone 3 → Zone 2), a seed row is usually not needed — the PID will converge on its own since the speed difference is small. Only add seed rows when there's a large speed gap between the outgoing and incoming row.

### 4. Always Set `minspeed` and `maxspeed` on Zone Rows

Without bounds, the PID can adjust speed from 0 up to the treadmill's maximum. For example, if your HR stays below the Zone 3 target, the PID will keep increasing speed in 0.2 km/h steps indefinitely — potentially reaching speeds you can't safely run at. **Always constrain it:**

```xml
<!-- WRONG — PID can ramp speed from 0 to device max -->
<row duration="00:10:00" zonehr="3"/>

<!-- RIGHT -->
<row duration="00:10:00" zonehr="3" minspeed="9.0" maxspeed="12.0"/>
```

Choose bounds that reflect your fitness level. Tight bounds (e.g., ±1 km/h) give more predictable behaviour; wide bounds give the PID more room to chase the HR target.

### 5. Speed Ramps

Speed ramps (`speedfrom`/`speedto`) are expanded internally by QZ into 1-second micro-rows with 0.1 km/h increments, each with `forcespeed` set automatically. This means:

- You **must** include `forcespeed="1"` on the original ramp row in your XML — QZ copies this flag to the expanded micro-rows.
- **Ramp duration affects smoothness.** QZ splits ramps into 0.1 km/h steps. If the duration is long enough, each step gets multiple seconds (smooth). If the duration is short relative to the speed delta, multiple steps are packed per second (still works, just feels more abrupt).
- For fixed-speed ramps, avoid positive HR zone targets unless HR control is intentional. Use `zonehr="0"` when explicitly disabling a preceding HR zone (see §2).

### 6. Repeat Blocks

Use `<repeat times="N">` to loop interval sets without duplicating rows:

```xml
<repeat times="6">
    <row duration="00:00:05" speed="11.5" inclination="1" forcespeed="1" zonehr="0"/>
    <row duration="00:01:30" zonehr="4" looptimehr="5" minspeed="10.0" maxspeed="14.0"/>
    <row duration="00:00:05" speed="8.0" inclination="0" forcespeed="1" zonehr="0"/>
    <row duration="00:01:30" zonehr="2" looptimehr="5" minspeed="7.5" maxspeed="8.5"/>
</repeat>
```

Note the `forcespeed` seed rows before each zone row — this is critical in intervals.

---

## Recommended QZ Settings for Training Programmes

Before running a training programme, review these PID-related settings in **Settings → Training Program Options** (they appear in this order in the UI, after "Auto Lap on Segment"):

| Setting | Recommended | Default | Why |
|---------|-------------|---------|-----|
| **PID on HR min / PID on HR max** | Leave at 0 | 0 | Alternative to "PID on Heart Zone" for specifying a custom HR range globally. Same rule: leave disabled when using training programmes. |
| **PID 'Pushy'** | ✅ Enabled (default) | ✅ Enabled | When enabled, the PID targets the **midpoint** between lower and upper HR limit of the zone. See "How the PID Targets Heart Rate" below. |
| **PID Ignore Inclination** | ✅ Enabled for fixed-speed incline-changing workouts | ❌ Disabled | Prevents the incline-related PID speed compensation, including the known `zonehr="0"` issue (#5520). Enable it and restart QZ before using such workouts. Leave other intentional PID configurations unchanged unless needed. |

### How the PID Targets Heart Rate

With "PID Pushy" enabled (the default), the PID targets the **midpoint between the lower and upper HR limit** of the zone (Karvonen formula). If you use `hrmin`/`hrmax` instead of `zonehr`, it targets the midpoint of that custom range.

This means:
- For **Zone 2** (by default 60–70% HRR): the PID targets ~65% HRR
- For **Zone 3** (by default 70–80% HRR): the PID targets ~75% HRR
- With `hrmin="130" hrmax="145"`: the PID targets ~137–138 bpm

If you want the PID to settle to a precise sub-zone of an HR zone, use `hrmin`/`hrmax`:

```xml
<!-- Target low Zone 2 (~62% HRR) instead of mid Zone 2 (~65% HRR) -->
<row duration="00:10:00" hrmin="125" hrmax="132" looptimehr="10" minspeed="7.0" maxspeed="10.0"/>
```

---

## Recommended Programme Structure

The example below assumes a mixed HR/fixed-speed workout and the recommended settings above. For speed-only workouts with HR control already disabled, `zonehr` may be omitted.

```
1. Warm-up ramp         — speedfrom/speedto + `forcespeed="1"` + `zonehr="0"`
2. (Optional) Zone 2    — zonehr="2" with minspeed/maxspeed
3. Main block           — zones, intervals, or fixed speed
4. Seed rows            — 5s forcespeed seeds with `zonehr="0"` between intensity changes
5. Cool-down            — fixed speed + `forcespeed="1"` with `zonehr="0"`
```

## File Format

- Extension: `.xml` (native) or `.zwo` (Zwift format, also supported)
- Location: place files in the `training` folder on the device running QZ, where the other example trainings are located.
- Subdirectories are supported for organisation
