# QZ Custom Dashboard — Developer & LLM Guide

This document is written for an LLM (or human developer) who wants to create a custom web dashboard for QZ Fitness. Read it top-to-bottom before writing any code.

---

## What a custom dashboard is

QZ already runs a local HTTP + WebSocket server (the "inner QZWS" server, default port 6666). A custom dashboard is a folder of static web files (`index.html`, CSS, JS, images) that QZ serves through this existing server. When the user enables the feature in **Settings → General Options → Custom Dashboard**, QZ replaces the standard tile-based home screen with a full-screen `WebView` that loads your `index.html`. The top toolbar (Bluetooth status, start/stop/lap buttons) always stays visible above the dashboard.

---

## File layout

```
src/inner_templates/<your-dashboard-name>/
    index.html      ← required entry point
    style.css       ← optional, referenced from index.html
    app.js          ← optional, referenced from index.html
    ...             ← any other assets (images, fonts, etc.)
```

Built-in dashboards live in `src/inner_templates/` and are compiled into the Qt resource system (`src/qml.qrc`). User-supplied dashboards live in `<QZ app-data dir>/dashboards/<name>/` and are served directly from the filesystem — no recompile needed.

**For built-in dashboards** (shipping with QZ): add every file to `src/qml.qrc`:
```xml
<file>inner_templates/<your-dashboard-name>/index.html</file>
<file>inner_templates/<your-dashboard-name>/style.css</file>
<file>inner_templates/<your-dashboard-name>/app.js</file>
```

**For user/community dashboards**: just drop the folder into `<appdata>/dashboards/` — no code changes needed. The folder name becomes the selectable name in Settings.

---

## Accessing shared assets

The `chartjs/` folder is always available at the same origin as your dashboard. Reference Chart.js without bundling it:

```html
<script src="../chartjs/chartjs.3.4.1.min.js"></script>
```

Other available libraries in `../chartjs/`:
- `chartjs-adapter-moment.js`
- `chartjs-plugin-annotation.min.js`
- `moment.js`
- `jquery-3.6.0.min.js`

---

## WebSocket connection

QZ sends live metrics once per second over a WebSocket at:

```
ws://localhost:<port>/
```

The port is stored in QSettings under the key `template_inner_QZWS_port`. The easiest way to discover it at runtime is to read the page's own port (since the HTTP server and WS server share the same port), or probe common fallbacks:

```js
function connectWS() {
  const port = parseInt(location.port, 10) || 6666;
  const ws = new WebSocket(`ws://localhost:${port}/`);
  ws.onopen  = () => console.log('connected');
  ws.onclose = () => setTimeout(connectWS, 2000); // auto-reconnect
  ws.onmessage = (ev) => {
    const msg = JSON.parse(ev.data);
    if (msg.msg === 'workout') handleMetrics(msg.content);
  };
}
```

Always implement auto-reconnect with a 2-second delay — the server may not be ready when the page first loads.

---

## Message format

Every second QZ sends one JSON message:

```json
{
  "msg": "workout",
  "content": { ...all metric fields... }
}
```

Only process messages where `msg.msg === "workout"`. Other `msg` values are internal QZ protocol messages; ignore them unless you need specific features (see "Sending commands" below).

---

## Metric fields reference (`msg.content`)

### Universal (all device types)

| Field | Type | Description |
|---|---|---|
| `deviceType` | int | 0 = treadmill, 1 = bike, 2 = elliptical, 3 = rower |
| `deviceName` | string | Bluetooth device name |
| `devicePaused` | bool | `true` while the workout is paused |
| `elapsed_h` | int | Elapsed hours |
| `elapsed_m` | int | Elapsed minutes (0–59) |
| `elapsed_s` | int | Elapsed seconds (0–59) |
| `lapelapsed_h/m/s` | int | Elapsed time for current lap |
| `moving_h/m/s` | int | Moving time (excludes pauses) |
| `remaining_time_h/m/s` | int | Time remaining in training program |
| `row_remaining_time_h/m/s` | int | Time remaining in current training row |
| `speed` | float | Current speed (unit depends on user setting) |
| `speed_avg` | float | Session average speed |
| `speed_lapavg` | float | Lap average speed |
| `calories` | float | Total calories burned |
| `distance` | float | Total distance (odometer) |
| `heart` | float | Current heart rate (BPM) |
| `heart_avg` | float | Session average HR |
| `heart_max` | float | Session max HR |
| `watts` | float | Current power output (W) |
| `watts_avg` | float | Session average watts |
| `watts_max` | float | Session max watts |
| `kgwatts` | float | Power-to-weight (W/kg) |
| `jouls` | float | Total energy in joules |
| `elevation` | float | Total elevation gain |
| `difficult` | float | Current difficulty multiplier |
| `latitude` / `longitude` / `altitude` | float | GPS coordinates |
| `workoutName` | string | Name of the loaded training program |
| `workoutStartDate` | string | ISO date when workout started |
| `instructorName` | string | Instructor name (Peloton integration) |
| `nickName` | string | User nickname from Settings |
| `autoresistance` | bool | ERG/auto-resistance mode active |
| `nextrow` | int | Next row index in training program |
| `pace_s/m/h` | int | Current pace |
| `avgpace_s/m/h` | int | Average pace |
| `maxpace_s/m/h` | int | Max pace |

### Bike-only fields

| Field | Type | Description |
|---|---|---|
| `cadence` | float | Pedalling cadence (RPM) |
| `cadence_avg` | float | Session average cadence |
| `resistance` | float | Current resistance level |
| `inclination` | float | Simulated road grade (%) |
| `peloton_resistance` | float | Peloton-mapped resistance (0–100) |
| `power_zone` | float | Current power zone (1–7) |
| `target_power` | float | ERG target power (W) |
| `target_cadence` | float | Target cadence |
| `target_resistance` | float | Target resistance level |
| `target_power_zone` | float | Target power zone |
| `req_power` | float | Last requested power |
| `req_cadence` | float | Last requested cadence |
| `gears` | int | Virtual gear position |
| `cranks` | int | Cumulative crank revolutions |

### Treadmill-only fields

| Field | Type | Description |
|---|---|---|
| `cadence` | float | Step cadence (steps/min) |
| `inclination` | float | Current treadmill incline (%) |
| `inclination_avg` | float | Average incline |
| `target_speed` | float | Target speed |
| `target_inclination` | float | Target incline |
| `stridelength` | float | Stride length (cm) |
| `groundcontact` | float | Ground contact time (ms) |
| `verticaloscillation` | float | Vertical oscillation (mm) |

### Rower-only fields

| Field | Type | Description |
|---|---|---|
| `cadence` | float | Stroke rate (strokes/min) |
| `strokescount` | float | Total stroke count |
| `strokeslength` | float | Stroke length |
| `resistance` | float | Resistance level |
| `target_pace_s/m/h` | int | Target pace |

### Elliptical-only fields

| Field | Type | Description |
|---|---|---|
| `cadence` | float | Stride cadence (RPM) |
| `resistance` | float | Resistance level |
| `inclination` | float | Ramp angle (%) |

---

## Sending commands to QZ

Your dashboard can send JSON commands back over the same WebSocket connection to control the device. All commands follow the pattern `{msg: "<command>", ...params}`.

### Control commands

```js
// Set resistance (bike/rower/elliptical)
ws.send(JSON.stringify({ msg: "setresistance", peloton_resistance: 42 }));

// Set power target (ERG mode)
ws.send(JSON.stringify({ msg: "setpower", power: 250 }));

// Set cadence target
ws.send(JSON.stringify({ msg: "setcadence", cadence: 90 }));

// Set speed (treadmill)
ws.send(JSON.stringify({ msg: "setspeed", speed: 10.5 }));

// Set incline (treadmill)
// (uses inclination field — value in %)
ws.send(JSON.stringify({ msg: "setinclination", inclination: 5.0 }));

// Set difficulty multiplier
ws.send(JSON.stringify({ msg: "setdifficult", difficult: 1.2 }));

// Set fan speed
ws.send(JSON.stringify({ msg: "setfanspeed", fanspeed: 3 }));
```

### Data request commands

```js
// Request current settings object (response: msg = "R_getsettings")
ws.send(JSON.stringify({ msg: "getsettings" }));

// Request session history array (response: msg = "R_getsessionarray")
ws.send(JSON.stringify({ msg: "getsessionarray" }));

// Request training program list (response: msg = "R_loadtrainingprograms")
ws.send(JSON.stringify({ msg: "loadtrainingprograms" }));
```

Response messages arrive as `{msg: "R_<command>", content: ...}`.

---

## Power zones reference

QZ reports `power_zone` as a float (1.0–7.0). If you want to compute zones yourself from raw watts, use the standard 7-zone model relative to FTP:

| Zone | % of FTP | Name |
|---|---|---|
| 1 | < 55% | Recovery |
| 2 | 55–75% | Endurance |
| 3 | 75–90% | Tempo |
| 4 | 90–105% | Threshold |
| 5 | 105–120% | VO₂max |
| 6 | 120–150% | Anaerobic |
| 7 | > 150% | Sprint |

The user's FTP is stored in QSettings under the key `ftp` (default 200 W). You can retrieve it from the `settings` object sent by QZ in the `R_getsettings` response.

---

## Minimal working example

A complete dashboard in a single file, no dependencies:

```html
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
  <style>
    body { background: #111; color: #fff; font-family: system-ui; display: flex;
           flex-direction: column; align-items: center; justify-content: center;
           height: 100dvh; margin: 0; font-size: 24px; }
    #power { font-size: 96px; font-weight: 700; color: #ff9f0a; }
  </style>
</head>
<body>
  <div>Power</div>
  <div id="power">–</div>
  <div>watts</div>

  <script>
    function connect() {
      const ws = new WebSocket(`ws://localhost:${location.port || 6666}/`);
      ws.onclose = () => setTimeout(connect, 2000);
      ws.onmessage = (ev) => {
        const msg = JSON.parse(ev.data);
        if (msg.msg === 'workout')
          document.getElementById('power').textContent = Math.round(msg.content.watts) || '–';
      };
    }
    connect();
  </script>
</body>
</html>
```

---

## Checklist before shipping a dashboard

- [ ] `index.html` exists at the root of the dashboard folder
- [ ] WebSocket connects to `ws://localhost:${location.port}/` (no hardcoded port)
- [ ] Auto-reconnect implemented (server may not be up when page first loads)
- [ ] Only `msg.msg === "workout"` messages are processed for live metrics
- [ ] All values guarded against `undefined` / `null` / `0` (device may not report all fields)
- [ ] `user-scalable=no` in viewport meta (prevents unwanted pinch-zoom in WebView)
- [ ] No `overflow: auto` on `body` / `html` — use `overflow: hidden` to avoid scroll bouncing on iOS
- [ ] External CDN links avoided — QZ may run offline; bundle assets or use `../chartjs/` shared libs
- [ ] Tested with simulated data before connecting to a real device

---

## Distributing a community dashboard

A dashboard is a plain folder. To share:
1. Zip the folder: `zip -r my-dashboard.zip my-dashboard/`
2. The recipient unzips it into `<QZ app-data>/dashboards/`
3. The name appears automatically in **Settings → General Options → Custom Dashboard**

No restart required — the picker reads the filesystem at open time.
