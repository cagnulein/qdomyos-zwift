#!/usr/bin/env python3
"""Regression tests for Smooth Chart Lines (chart_smooth_lines) on the live charts.

The live charts are shown during a workout: the power line of chartlive.htm (dochartlive.js)
and the speed line of treadmillchartlive.htm (dotreadmillchartlive.js); the moving average is
in livesmooth.js. tests/live_chart_smoothing_harness.js runs them in Node with stubs and
returns the line as Chart.js gets it: when the chart is made (the session drawn at the page
opening) and after every 'workout' message.

- Off (and a setting not saved yet, or unknown) draws the raw points, as master does.
- On, the line is the trailing moving average over the window in seconds of the elapsed time:
  a point drawn once does not change when the next ones arrive. Auto is 3 s (a live chart is
  shown only while the workout runs).

Run: python3 -m unittest tests/test_live_chart_smoothing.py
"""

import json
import math
import shutil
import subprocess
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CHARTJS = ROOT / "src/inner_templates/chartjs"
HARNESS = ROOT / "tests/live_chart_smoothing_harness.js"

# the index of chart_smooth_lines (settings.qml): Off, Auto, 3, 5, 10, 15, 30 s
OFF, AUTO = 0, 1
WINDOWS = {AUTO: 3, 2: 3, 3: 5, 4: 10, 5: 15, 6: 30}

BIKE = {
    "htm": "chartlive.htm",
    "script": "dochartlive.js",
    "dataset": 0,  # Watts
    "field": "watts",
    "target": ("target_power", 150),
    "digits": 0,
    "settings": {
        "ftp": 200, "miles_unit": False, "age": 40, "heart_rate_zone1": 70,
        "heart_rate_zone2": 80, "heart_rate_zone3": 90, "heart_rate_zone4": 100,
        "heart_max_override_enable": False, "heart_max_override_value": 195,
    },
    "interpolation": lambda smoothed: "monotone",  # the power line is a curve on master too
}
TREADMILL = {
    "htm": "treadmillchartlive.htm",
    "script": "dotreadmillchartlive.js",
    "dataset": 2,  # Speed (actual)
    "field": "speed",
    "target": ("target_speed", 10),
    "digits": 1,
    "settings": {
        "miles_unit": False, "pacef_1mile": 300, "pacef_5km": 330, "pacef_10km": 360,
        "pacef_halfmarathon": 390, "pacef_marathon": 420,
    },
    "interpolation": lambda smoothed: "monotone" if smoothed else None,
}


class Noise:
    """The same numbers on every run and every Python (a linear congruential generator)."""

    def __init__(self, seed):
        self.state = seed

    def next(self):
        self.state = (self.state * 1103515245 + 12345) % 2147483648
        return self.state / 2147483648


def point(page, elapsed, value, target_value=None):
    """A row of the session or a 'workout' message; target_value: the target of the program
    (None: the target of the page)."""
    target, page_target = page["target"]
    return {
        page["field"]: value,
        target: page_target if target_value is None else target_value,
        "inclination": 1,
        "elapsed_s": elapsed % 60,
        "elapsed_m": (elapsed // 60) % 60,
        "elapsed_h": elapsed // 3600,
    }


def workout(page, history_seconds, live_messages, seed):
    """The session at the page opening (a point a second from 1 s on) and the 'workout'
    messages after it: every 1-2 s, as measured on a phone (T-101), with a sprint."""
    noise = Noise(seed)

    def value(elapsed):
        sprint = 40 <= elapsed < 48
        if page["field"] == "watts":
            return round(140 + 60 * noise.next() + (250 if sprint else 0))
        return round(9 + 2 * noise.next() + (4 if sprint else 0), 2)

    session = [point(page, s, value(s)) for s in range(1, history_seconds + 1)]
    live = []
    elapsed = history_seconds
    for i in range(live_messages):
        elapsed += 1 if i % 3 == 0 else 2
        live.append(point(page, elapsed, value(elapsed)))
    return session, live


def raw_line(page, points):
    """The raw points as the page draws them: x from the first point with a target."""
    first = None
    line = []
    for p in points:
        elapsed = p["elapsed_s"] + p["elapsed_m"] * 60 + p["elapsed_h"] * 3600
        if first is None:
            first = elapsed
        line.append({"x": elapsed - first, "y": p[page["field"]]})
    return line


def js_round(value, digits):
    """Math.round(v * k) / k of livesmooth.js (half up, unlike Python's round)."""
    k = 10 ** digits
    return math.floor(value * k + 0.5) / k


def trailing(line, seconds, digits):
    """The trailing average written once more, without livesmooth.js."""
    out = []
    for i, p in enumerate(line):
        if not isinstance(p["y"], (int, float)):  # no value: a gap, as on the Charts page
            out.append(p)
            continue
        # only the points after the last one with a later x: a new workout starts x from 0 again
        start = 0
        for k in range(i - 1, -1, -1):
            if line[k]["x"] > p["x"]:
                start = k + 1
                break
        last = {}  # the last value of every x so far: a pause repeats the x
        for q in line[start:i + 1]:
            last[q["x"]] = q["y"]
        values = [y for x, y in last.items() if x > p["x"] - seconds and isinstance(y, (int, float))]
        out.append({"x": p["x"], "y": js_round(sum(values) / len(values), digits)})
    return out


def htm_scripts(page):
    """The src of every <script src="..."> of the page, in the order the .htm loads them."""
    text = (CHARTJS / page["htm"]).read_text(encoding="utf-8")
    scripts = []
    for part in text.split("<script")[1:]:
        tag = part.split(">", 1)[0]
        if 'src="' in tag:
            scripts.append(tag.split('src="', 1)[1].split('"', 1)[0])
    return scripts


def page_sources(page):
    """The scripts the harness runs, in the order of the .htm: the moving average, the page
    script and the heart chart below it, which shares the globals of the page."""
    wanted = ("livesmooth.js", page["script"], "dochartliveheart.js")
    return [name for name in htm_scripts(page) if name in wanted]


def harness(page, smooth, session, live):
    node = shutil.which("node")
    if node is None:
        raise RuntimeError("Node.js is needed to run the live chart pages (tests/live_chart_smoothing_harness.js)")
    settings = dict(page["settings"])
    settings["chart_smooth_lines"] = smooth
    request = {
        "sources": [str(CHARTJS / name) for name in page_sources(page)],
        "inits": ["dochart_init", "dochartheart_init"],
        "canvas": "canvas",
        "dataset": page["dataset"],
        "settings": settings,
        "session": session,
        "workouts": live,
    }
    done = subprocess.run([node, str(HARNESS)], input=json.dumps(request), capture_output=True,
                          text=True, encoding="utf-8", check=False)
    if done.returncode != 0:
        raise RuntimeError("the harness failed: " + done.stderr)
    out = json.loads(done.stdout)
    if out["errors"]:
        raise AssertionError(page["script"] + " logged errors: " + "; ".join(out["errors"]))
    return out["lines"]


class LiveChartSmoothing(unittest.TestCase):
    def test_page_loads_the_moving_average(self):
        # the .htm loads livesmooth.js before the page script, and the app has it in its
        # resources: the page is served from qml.qrc
        qrc = (ROOT / "src/qml.qrc").read_text(encoding="utf-8")
        for page in (BIKE, TREADMILL):
            with self.subTest(page=page["htm"]):
                sources = page_sources(page)
                self.assertEqual(sources, ["livesmooth.js", page["script"], "dochartliveheart.js"])
                for name in [page["htm"]] + sources:
                    self.assertIn("<file>inner_templates/chartjs/%s</file>" % name, qrc)

    def check_page(self, page, smooth, seconds):
        session, live = workout(page, 60, 20, seed=7)
        lines = harness(page, smooth, session, live)
        # the chart made from the session, then one update per 'workout' message
        self.assertEqual(len(lines), 1 + len(live))
        raw = raw_line(page, session + live)
        expected = trailing(raw, seconds, page["digits"]) if seconds > 1 else raw
        for n, line in enumerate(lines):
            drawn = len(session) + n
            self.assertEqual(line["points"], expected[:drawn],
                             "%s, setting %r: the line after %d messages" % (page["script"], smooth, n))
            self.assertEqual(line["cubicInterpolationMode"], page["interpolation"](seconds > 1))
        return raw, lines

    def test_off_draws_the_raw_points(self):
        for page in (BIKE, TREADMILL):
            for smooth in (OFF, str(OFF), None, "7", "", "x"):  # not saved yet or unknown: Off
                with self.subTest(page=page["script"], smooth=smooth):
                    self.check_page(page, smooth, 0)

    def test_windows_are_trailing_averages(self):
        for page in (BIKE, TREADMILL):
            for index, seconds in WINDOWS.items():
                with self.subTest(page=page["script"], index=index):
                    # the app sends the value of the ini as a string
                    raw, lines = self.check_page(page, str(index), seconds)
                    self.assertNotEqual(lines[-1]["points"], raw)

    def test_drawn_points_do_not_change(self):
        # trailing, not centered: every update only adds a point at the end
        for page in (BIKE, TREADMILL):
            with self.subTest(page=page["script"]):
                session, live = workout(page, 30, 15, seed=3)
                lines = harness(page, "3", session, live)
                for before, after in zip(lines, lines[1:]):
                    self.assertEqual(after["points"][:len(before["points"])], before["points"])
                    self.assertEqual(len(after["points"]), len(before["points"]) + 1)

    def test_page_opened_with_no_history(self):
        # the page opened at the start of the workout: every point comes from a message
        for page in (BIKE, TREADMILL):
            with self.subTest(page=page["script"]):
                _, live = workout(page, 0, 12, seed=5)
                lines = harness(page, "3", [], live)
                raw = raw_line(page, live)
                self.assertEqual(lines[-1]["points"], trailing(raw, 5, page["digits"]))

    def test_points_before_the_first_target_are_dropped(self):
        # the page opened before the program has a target: the line starts again at the first
        # message with a target, x from it, as on master (off and on)
        no_target = {"watts": 0, "speed": -1}  # the app sends -1 for no target speed
        for page in (BIKE, TREADMILL):
            for smooth, seconds in ((str(OFF), 0), ("3", 5)):  # index 3: 5 s
                with self.subTest(page=page["script"], smooth=smooth):
                    _, live = workout(page, 10, 12, seed=13)
                    before = [point(page, s, 120 if page["field"] == "watts" else 8,
                                    no_target[page["field"]]) for s in range(1, 11)]
                    lines = harness(page, smooth, [], before + live)
                    raw = raw_line(page, live)
                    expected = trailing(raw, seconds, page["digits"]) if seconds > 1 else raw
                    self.assertEqual(lines[-1]["points"], expected)

    def test_new_workout_on_the_open_page(self):
        # Stop and Start again while the chart stays shown: the page is not reloaded and the x of
        # the new workout starts from 0 again; its average does not take the last workout in
        for page in (BIKE, TREADMILL):
            with self.subTest(page=page["script"]):
                session, live = workout(page, 30, 10, seed=17)
                busy = 300 if page["field"] == "watts" else 15
                again = [point(page, s, busy) for s in range(1, 13)]
                lines = harness(page, "4", session, live + again)  # 10 s
                drawn = lines[-1]["points"]
                self.assertEqual(drawn, trailing(raw_line(page, session + live + again), 10,
                                                 page["digits"]))
                # the new workout: its own points only, all at the same value
                self.assertEqual({p["y"] for p in drawn[-len(again):]}, {busy})

    def test_pause_does_not_weigh_in(self):
        # paused, the elapsed time stands still and the messages keep coming every second with
        # the same x; after the resume the line is the average of the seconds before, as if the
        # pause was one point
        for page in (BIKE, TREADMILL):
            with self.subTest(page=page["script"]):
                session, live = workout(page, 30, 0, seed=11)
                busy = 200 if page["field"] == "watts" else 10
                live = [point(page, 30, 0) for _ in range(40)]
                live += [point(page, 30 + s, busy) for s in range(1, 8)]
                # a message without the value: the power is drawn as it comes (a gap); the speed
                # page multiplies it by the unit, and null * 1 is 0 there, as on master
                bike = page["field"] == "watts"
                live.append(point(page, 38, None if bike else busy))
                live.append(point(page, 39, busy))
                lines = harness(page, "6", session, live)  # 30 s
                raw = raw_line(page, session + live)
                expected = trailing(raw, 30, page["digits"])
                self.assertEqual(lines[-1]["points"], expected)
                # the pause counts once: 7 s of busy in a 30 s window, not pulled to 0 by 40 zeros
                after = expected[len(session) + 40 + 6]["y"]
                self.assertGreater(after, busy * 7 / 30 * 0.8)
                if bike:
                    self.assertIsNone(lines[-1]["points"][-2]["y"])


if __name__ == "__main__":
    unittest.main()
