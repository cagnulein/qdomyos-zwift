#!/usr/bin/env python3
"""Regression tests for Smooth Chart Lines (chart_smooth_lines) on the Charts page.

src/inner_templates/chartjs/dochart.js draws the Charts page and the charts saved for the
workout email. tests/chart_smoothing_harness.js runs it in Node with stubs and returns what it
gives to Chart.js (the points of every line) and the texts of the summary.

- Off (and a setting not saved yet) must give exactly what master gives: the fingerprint in
  tests/fixtures/chart_smoothing_master.json, made from master's dochart.js.
- On, only the power, cadence and speed lines change; heart rate, resistance, inclination,
  the targets, the power distribution and every number of the summary stay the same.
- The smoothed lines are the centered moving average of the window, Auto picks its window.

Run: python3 -m unittest tests/test_chart_smoothing.py
New fingerprint, only after a change of dochart.js that is meant to change the charts with
the smoothing Off (in the same commit, and say in the PR which chart changes and why: the
fixture keeps only hashes and point counts). The fingerprint is drawn with the setting not
saved, i.e. Off:
python3 tests/test_chart_smoothing.py --update src/inner_templates/chartjs/dochart.js
"""

import bisect
import hashlib
import json
import shutil
import subprocess
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DOCHART = ROOT / "src/inner_templates/chartjs/dochart.js"
HARNESS = ROOT / "tests/chart_smoothing_harness.js"
UPDATE_HINT = ("Off no longer draws what tests/fixtures/chart_smoothing_master.json holds. If the "
               "change of the charts is meant, write the new fingerprint in the same commit and say in "
               "the PR which chart changes and why: python3 tests/test_chart_smoothing.py --update "
               "src/inner_templates/chartjs/dochart.js")
FINGERPRINT = ROOT / "tests/fixtures/chart_smoothing_master.json"

# the index of chart_smooth_lines (settings.qml): Off, Auto, 3, 5, 10, 15, 30 s
OFF, AUTO = 0, 1
FIXED = {2: 3, 3: 5, 4: 10, 5: 15, 6: 30}

# the smoothed lines: (canvas, index of the line in the chart)
SMOOTHED = {("canvas", 0), ("canvasCadence", 0), ("canvasSpeedInclination", 0)}
# their rounding in dochart.js: speed to 0.1, power and cadence to whole numbers
DIGITS = {"canvas": 0, "canvasCadence": 0, "canvasSpeedInclination": 1}


class Noise:
    """The same numbers on every run and every Python (a linear congruential generator)."""

    def __init__(self, seed):
        self.state = seed

    def next(self):
        self.state = (self.state * 1103515245 + 12345) % 2147483648
        return self.state / 2147483648


def session(seconds, device_type, pause_at, pause_length, seed):
    """A synthetic workout of one point a second. During the pause the elapsed time stands still
    and the power, cadence and speed are 0 (a harder case for the smoothing than the app's
    session, which gets no points while paused)."""
    noise = Noise(seed)
    points = []
    elapsed = 0
    for i in range(seconds):
        paused = pause_at <= i < pause_at + pause_length
        if not paused and i > 0:
            elapsed += 1
        sprint = 600 <= elapsed < 615
        watts = 0 if paused else round(140 + 40 * noise.next() + (250 if sprint else 0))
        cadence = 0 if paused else round(80 + 12 * noise.next() + (20 if sprint else 0))
        speed = 0 if paused else round(9 + 2 * noise.next() + (4 if sprint else 0), 2)
        points.append({
            "watts": watts,
            "req_power": 150 if (elapsed // 300) % 2 == 0 else 200,
            "elapsed_s": elapsed % 60,
            "elapsed_m": (elapsed // 60) % 60,
            "elapsed_h": elapsed // 3600,
            "heart": round(110 + 40 * noise.next()),
            "resistance": 8 + (elapsed // 600) % 5,
            "req_resistance": 10,
            "cadence": cadence,
            "req_cadence": 90,
            "speed": speed,
            "inclination": (elapsed // 900) % 4,
            "peloton_resistance": 30 + (elapsed // 600) % 5,
            "peloton_req_resistance": 35,
            "workoutName": "Test ride",
            "workoutStartDate": "2026-11-27",
            "instructorName": "",
            "watts_avg": 155,
            "watts_max": 412,
            "heart_avg": 130,
            "heart_max": 151,
            "jouls": 123456,
            "calories": 456,
            "distance": 12.3,
            "cadence_avg": 86,
            "peloton_resistance_avg": 32,
            "resistance_avg": 10,
            "deviceType": device_type,
        })
    return points


SESSIONS = {
    # 2 h 1 min: longer than the chart keeps (the lines are thinned to the width of the screen),
    # Auto takes 11 s after Stop
    "bike_2h": session(7300, 2, 3000, 90, 7),
    # 40 min: not thinned, Auto takes 5 s after Stop
    "treadmill_40min": session(2400, 1, 1200, 30, 11),
}

SETTINGS = {
    "ftp": 200,
    "miles_unit": False,
    "age": 40,
    "heart_rate_zone1": 70,
    "heart_rate_zone2": 80,
    "heart_rate_zone3": 90,
    "heart_rate_zone4": 100,
    "heart_max_override_enable": False,
    "heart_max_override_value": 195,
}


def harness(request):
    node = shutil.which("node")
    if node is None:
        raise RuntimeError("Node.js is needed to run dochart.js (tests/chart_smoothing_harness.js)")
    done = subprocess.run([node, str(HARNESS)], input=json.dumps(request), capture_output=True,
                          text=True, encoding="utf-8", check=False)
    if done.returncode != 0:
        raise RuntimeError("the harness failed: " + done.stderr)
    out = json.loads(done.stdout)
    if out["errors"]:
        raise AssertionError("dochart.js logged errors: " + "; ".join(out["errors"]))
    return out


def request_for(smooth, source, points):
    settings = dict(SETTINGS)
    settings["chart_smooth_lines"] = smooth
    return {"source": str(source), "settings": settings, "running": False, "session": points}


def run_page(points, smooth, source=DOCHART, live_points=None):
    """What the page gives to Chart.js: the renders of the harness. smooth – the value of
    chart_smooth_lines as the app sends it (None: not saved yet)."""
    request = request_for(smooth, source, points)
    if live_points is not None:
        request["liveSession"] = live_points
    return harness(request)["renders"]


def windows(smooth, seconds, running):
    """smoothWindow() of dochart.js for workouts of these lengths."""
    request = request_for(smooth, DOCHART, SESSIONS["treadmill_40min"][:10])
    request["windowsFor"] = {"seconds": seconds, "running": running}
    return harness(request)["windows"]


def digest(data):
    return hashlib.sha256(json.dumps(data, separators=(",", ":")).encode("utf-8")).hexdigest()


def fingerprint(renders):
    """Every line by its points (a hash and the number of points) and the summary texts."""
    result = []
    for render in renders:
        charts = []
        for chart in render["charts"]:
            charts.append({
                "canvas": chart["canvas"],
                "type": chart["type"],
                "lines": [{
                    "label": line["label"],
                    "cubicInterpolationMode": line["cubicInterpolationMode"],
                    "points": len(line["data"]),
                    "sha256": digest(line["data"]),
                } for line in chart["lines"]],
            })
        result.append({"charts": charts, "texts": render["texts"]})
    return result


def lines(render):
    """(canvas, index) -> the line of one render."""
    found = {}
    for chart in render["charts"]:
        for index, line in enumerate(chart["lines"]):
            found[(chart["canvas"], index)] = line
    return found


def moving_average(points, window, digits):
    """The reference: the mean of the points with a value whose time is in
    [x - back, x + ahead], back + ahead + 1 = window, rounded as dochart.js does."""
    back = (window - 1) // 2
    ahead = window - 1 - back
    scale = 10 ** digits
    times = [p["x"] for p in points]  # the elapsed time only grows
    result = []
    for point in points:
        y = point["y"]
        if not isinstance(y, (int, float)):
            result.append(point)
            continue
        inside = points[bisect.bisect_left(times, point["x"] - back):bisect.bisect_right(times, point["x"] + ahead)]
        values = [p["y"] for p in inside if isinstance(p["y"], (int, float))]
        total = 0
        for value in values:  # in order, as JavaScript adds (sum() of Python 3.12 compensates)
            total += value
        mean = total / len(values)
        result.append({"x": point["x"], "y": js_round(mean * scale) / scale})
    return result


def js_round(value):
    """Math.round: the nearest integer, halves towards +infinity."""
    lower = int(value // 1)
    return lower + 1 if value - lower >= 0.5 else lower


def same_number(a, b):
    return abs(a - b) < 1e-9


class ChartSmoothingTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.expected = json.loads(FINGERPRINT.read_text(encoding="utf-8"))
        cls.off = {name: run_page(points, None) for name, points in SESSIONS.items()}

    def test_off_is_master(self):
        """Off, not saved yet, or an unknown value: the charts and texts of master."""
        for name, points in SESSIONS.items():
            self.assertEqual(fingerprint(self.off[name]), self.expected[name], name + ", not saved. " + UPDATE_HINT)
            for value in (OFF, "0", 99, "x"):
                with self.subTest(session=name, value=value):
                    self.assertEqual(fingerprint(run_page(points, value)), self.expected[name], UPDATE_HINT)

    def test_on_changes_only_power_cadence_speed(self):
        for name, points in SESSIONS.items():
            off = lines(self.off[name][0])
            for value in [AUTO] + list(FIXED):
                with self.subTest(session=name, value=value):
                    render = run_page(points, value)[0]
                    self.assertEqual(render["texts"], self.off[name][0]["texts"], "the summary must stay raw")
                    on = lines(render)
                    self.assertEqual(set(on), set(off))
                    for key, line in on.items():
                        if key in SMOOTHED:
                            self.assertEqual(line["cubicInterpolationMode"], "monotone", key)
                        else:
                            self.assertEqual(line, off[key], key)

    def test_fixed_windows_are_the_moving_average(self):
        # 40 min: the page draws every point (not thinned), so the points are comparable
        points = SESSIONS["treadmill_40min"]
        raw = lines(self.off["treadmill_40min"][0])
        for value, window in FIXED.items():
            on = lines(run_page(points, value)[0])
            for key in SMOOTHED:
                with self.subTest(window=window, line=key):
                    expected = moving_average(raw[key]["data"], window, DIGITS[key[0]])
                    got = on[key]["data"]
                    self.assertEqual(len(got), len(expected))
                    for e, g in zip(expected, got):
                        self.assertEqual(e["x"], g["x"])
                        self.assertTrue(same_number(e["y"], g["y"]), (e, g))

    def test_auto_after_stop_by_duration(self):
        # 40 min after Stop: 3 s + 2 s for the full 30 minutes = 5 s
        points = SESSIONS["treadmill_40min"]
        self.assertEqual(lines(run_page(points, AUTO)[0]), lines(run_page(points, 3)[0]))

    def test_auto_window_by_duration(self):
        # after Stop: 3 s and 2 s more for every full 30 minutes, up to 15 s; while the workout
        # runs: 3 s
        cases = {0: 3, 1799: 3, 1800: 5, 2400: 5, 3600: 7, 7300: 11, 10799: 13, 10800: 15, 36000: 15}
        seconds = list(cases)
        self.assertEqual(windows(AUTO, seconds, False), [cases[s] for s in seconds])
        self.assertEqual(windows(AUTO, seconds, True), [3] * len(seconds))
        for value, window in FIXED.items():
            for sent in (value, str(value)):  # a number, or a string read back from the ini file
                with self.subTest(value=sent):
                    self.assertEqual(windows(sent, seconds, False), [window] * len(seconds))
                    self.assertEqual(windows(sent, seconds, True), [window] * len(seconds))
        self.assertEqual(windows(str(AUTO), seconds, False), [cases[s] for s in seconds])
        for value in (None, OFF, 99):
            self.assertEqual(windows(value, seconds, False), [0] * len(seconds))

    def test_auto_during_the_workout_is_3s(self):
        points = SESSIONS["treadmill_40min"]
        live = run_page(points, AUTO, live_points=points)[-1]
        fixed = run_page(points, 2)[0]
        for key in SMOOTHED:
            with self.subTest(line=key):
                self.assertEqual(lines(live)[key]["data"], lines(fixed)[key]["data"])


    def test_stop_redraws_only_for_auto(self):
        """The page open while the workout runs, then Stop, the session no longer growing. Off
        and a fixed window: no drawing after the first one, as on master, so the screenshots
        are not taken again. Auto: one drawing more, with the window by the length and without
        the animation (no screenshots: they are taken by the page the app opens after Stop)."""
        for name, points in SESSIONS.items():
            for value in (None, OFF) + tuple(FIXED) + (AUTO,):
                with self.subTest(session=name, value=value):
                    out = run_open_then_stop(points, value)
                    self.assertEqual(out["stopAt"], 1, "drawn again while the workout runs")
                    self.assertTrue(all(c["animation"] for c in out["renders"][0]["charts"]))
                    if value != AUTO:
                        self.assertEqual(len(out["renders"]), 1, "drawn again after Stop")
                        continue
                    self.assertEqual(len(out["renders"]), 2)
                    self.assertFalse(any(c["animation"] for c in out["renders"][1]["charts"]))
                    if name == "treadmill_40min":  # 3 s while running, 5 s after Stop
                        for render, fixed in ((0, 2), (1, 3)):
                            got = lines(out["renders"][render])
                            want = lines(run_page(points, fixed)[0])
                            for key in SMOOTHED:
                                self.assertEqual(got[key]["data"], want[key]["data"], (render, key))


def run_open_then_stop(points, smooth):
    request = request_for(smooth, DOCHART, points)
    request["running"] = True
    request["stopLater"] = True
    return harness(request)


def update(master_source):
    data = {name: fingerprint(run_page(points, None, source=Path(master_source)))
            for name, points in SESSIONS.items()}
    FINGERPRINT.parent.mkdir(parents=True, exist_ok=True)
    FINGERPRINT.write_text(json.dumps(data, indent=1) + "\n", encoding="utf-8")
    print("written", FINGERPRINT)


if __name__ == "__main__":
    if len(sys.argv) == 3 and sys.argv[1] == "--update":
        update(sys.argv[2])
    else:
        unittest.main()
