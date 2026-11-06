#!/usr/bin/env python3
"""Android emulator E2E test of the *shipped* SupportEmail.java Java implementation.

Invokes the debug-only SupportEmailTestActivity inside the actual QZ APK. The
real Java method launches ACTION_CHOOSER, and this script selects a separate
fake mail app. The fake app reads the attachment via Android's granted
content:// URI and verifies To, subject, body, MIME type and gzip bytes.

Runs on each of the 10 existing android-emulator-test matrix entries.
Nothing is emailed or uploaded; failures are never silently skipped.
"""
import os
import re
import subprocess
import sys
import time
import xml.etree.ElementTree as ET
from pathlib import Path

QZ = "org.cagnulen.qdomyoszwift"
PROBE = "org.qz.supportmailprobe"
SCENARIOS = ("with_log", "without_log", "missing_log")
EVIDENCE = Path("support_email_evidence")
EVIDENCE.mkdir(exist_ok=True)


def adb(*args, timeout=35, check=True):
    result = subprocess.run(["adb", *args], capture_output=True, text=True,
                            encoding="utf-8", errors="replace", timeout=timeout)
    if check and result.returncode:
        raise RuntimeError(f"adb {args} failed ({result.returncode}): {result.stdout} {result.stderr}")
    return result.stdout.strip()


def output_result():
    return adb("shell", "run-as", PROBE, "cat", "files/result.txt", check=False)


def hierarchy(scenario):
    # The Android sharesheet/chooser UI varies across APIs. Read the platform
    # accessibility hierarchy instead of hard-coding screen coordinates.
    adb("shell", "uiautomator", "dump", "/sdcard/qz-mail-chooser.xml", timeout=35,
        check=False)
    xml = adb("exec-out", "cat", "/sdcard/qz-mail-chooser.xml", check=False)
    if not xml.startswith("<?xml"):
        return None, xml
    (EVIDENCE / f"{scenario}_chooser.xml").write_text(xml, encoding="utf-8")
    try:
        return ET.fromstring(xml), xml
    except ET.ParseError:
        return None, xml


def choose_mail_probe(scenario):
    deadline = time.monotonic() + 105
    presses = 0
    chooser_captured = False
    last_xml = ""
    while time.monotonic() < deadline:
        result = output_result()
        if result.startswith("PASS ") or result.startswith("FAIL "):
            if not chooser_captured:
                if presses:
                    raise RuntimeError("Email receiver launched, but chooser screenshot failed")
                # Some older Android system images auto-open the only ACTION_SEND
                # handler. That is valid: the receiver checked the real Intent,
                # but Android never displayed a chooser to photograph.
                marker = EVIDENCE / f"{scenario}_01_android_direct_launch.txt"
                marker.write_text(
                    "Android opened QZ Mail Probe directly: no chooser was displayed.\\n"
                    "The receiver still validates ACTION_SEND, recipient, subject,\\n"
                    "body and cross-UID attachment read permission.\\n",
                    encoding="utf-8")
                print(f"  Direct mail-app launch (no chooser): {marker}", flush=True)
            return result
        tree, last_xml = hierarchy(scenario)
        if tree is not None:
            nodes = [node for node in tree.iter("node")
                     # A directly opened receiver labels itself "QZ Mail Probe"
                     # too. Never mistake its own screen for Android's chooser.
                     if node.get("package") != PROBE and
                     "QZ Mail Probe" in ((node.get("text") or "") + " " +
                                         (node.get("content-desc") or ""))]
            if nodes:
                # Capture the actual Android share chooser BEFORE selecting our
                # fake mail app; otherwise the screenshot shows only the receiver.
                if not chooser_captured:
                    chooser_captured = screenshot(scenario, "01_android_chooser")
                # Tap the target label; chooser selections might need a second
                # tap/confirmation on older Android images.
                bounds = nodes[-1].get("bounds") or ""
                coords = [int(n) for n in re.findall(r"\d+", bounds)]
                if len(coords) == 4:
                    x = (coords[0] + coords[2]) // 2
                    y = (coords[1] + coords[3]) // 2
                    print(f"  Choosing QZ Mail Probe at {x},{y}")
                    adb("shell", "input", "tap", str(x), str(y))
                    presses += 1
                    time.sleep(2.0)
                    continue
            # For an Android resolver with Always/Just once secondary action.
            if presses:
                for node in tree.iter("node"):
                    if (node.get("text") or "").strip().lower() in ("just once", "only once"):
                        coords = [int(n) for n in re.findall(r"\d+", node.get("bounds") or "")]
                        if len(coords) == 4:
                            adb("shell", "input", "tap",
                                str((coords[0] + coords[2]) // 2),
                                str((coords[1] + coords[3]) // 2))
                            time.sleep(2)
                            break
        time.sleep(3.0)
    raise RuntimeError("Chooser did not deliver mail to QZ Mail Probe "
                       f"(taps={presses}; last UI={last_xml[:1200]!r})")


def screenshot(scenario, stage):
    """Save real emulator pixels, not an artificial mockup or synthesized image."""
    try:
        data = subprocess.run(["adb", "exec-out", "screencap", "-p"],
                              capture_output=True, timeout=20, check=True).stdout
        if not data.startswith(b"\x89PNG\r\n\x1a\n"):
            raise RuntimeError("adb screencap did not return a PNG")
        path = EVIDENCE / f"{scenario}_{stage}.png"
        path.write_bytes(data)
        print(f"  Captured {path}", flush=True)
        return True
    except Exception as error:
        print(f"Could not capture {scenario}/{stage}: {error}", flush=True)
        return False


def main():
    api = adb("shell", "getprop", "ro.build.version.sdk")
    print(f"Testing native Android support-mail handoff on API {api}")
    # Only our private debug activity can execute SupportEmail.compose(); a
    # release APK lacking that activity causes an immediate, explicit failure.
    installed = adb("shell", "pm", "path", PROBE, check=False)
    if not installed:
        raise RuntimeError("QZ Mail Probe is not installed")
    lines = [f"API={api}"]
    try:
        for scenario in SCENARIOS:
            print(f"Scenario: {scenario}", flush=True)
            adb("shell", "am", "force-stop", PROBE)
            adb("shell", "pm", "clear", PROBE)
            adb("shell", "am", "force-stop", QZ)
            start = adb("shell", "am", "start", "-W",
                        "-n", QZ + "/.SupportEmailTestActivity",
                        "--es", "scenario", scenario, timeout=55)
            if "Error" in start or "Exception" in start:
                raise RuntimeError(f"Could not launch QZ test activity: {start}")
            try:
                result = choose_mail_probe(scenario)
                chooser = EVIDENCE / f"{scenario}_01_android_chooser.png"
                direct_launch = EVIDENCE / f"{scenario}_01_android_direct_launch.txt"
                if not (chooser.is_file() or direct_launch.is_file()):
                    raise RuntimeError("No chooser screenshot or verified direct-launch marker")
                # The receiver writes result.txt only after setContentView().
                # Give Android time to render the actual email fields.
                time.sleep(1.5)
                if not screenshot(scenario, "02_email_compose_preview"):
                    raise RuntimeError("No Android email preview screenshot captured")
            except Exception:
                screenshot(scenario, "99_failure")
                raise
            lines.append(f"{scenario}: {result}")
            print(f"  {result}", flush=True)
            if result != f"PASS {scenario} API={api}":
                raise RuntimeError(f"{scenario} failed: {result}")
    finally:
        (EVIDENCE / "results.txt").write_text("\n".join(lines) + "\n",
                                                encoding="utf-8")
        # Do not hide the actual test failure if the API 24 emulator loses
        # ADB: logcat collection is best-effort and must preserve the original
        # chooser/recipient exception and the already-written results.txt.
        try:
            logcat = adb("logcat", "-d", "-s", "QZSupportMailTest:I",
                         "QZSupportEmail:E", "QZMailProbe:I", "AndroidRuntime:E",
                         "*:S", timeout=12, check=False)
        except (subprocess.TimeoutExpired, OSError) as error:
            logcat = "Could not collect logcat (emulator/ADB unavailable): " + str(error)
        (EVIDENCE / "logcat.txt").write_text(logcat, encoding="utf-8")
    print("ALL ANDROID SUPPORT EMAIL CASES PASSED")


if __name__ == "__main__":
    try:
        main()
    except Exception as error:
        print(f"FAIL: {error}", file=sys.stderr)
        sys.exit(1)
