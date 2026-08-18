#!/usr/bin/env python3
from pathlib import Path
import json
import re

ROOT = Path(__file__).resolve().parents[1]
QML = ROOT / "src/settings.qml"
CATALOG = ROOT / "src/settings-catalog.json"
MARKER = "// SETTINGS_BEHAVIOR_PARITY_V2"


def balanced_span(text, brace):
    depth = 0
    in_single = in_double = escaped = False
    for i in range(brace, len(text)):
        ch = text[i]
        if escaped:
            escaped = False
        elif ch == "\\" and (in_single or in_double):
            escaped = True
        elif ch == "'" and not in_double:
            in_single = not in_single
        elif ch == '"' and not in_single:
            in_double = not in_double
        elif not in_single and not in_double:
            if ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return i + 1
    raise RuntimeError("unbalanced QML block")


def handler_blocks(text):
    for m in re.finditer(r"\bon[A-Z][A-Za-z0-9_]*\s*:\s*", text):
        start = m.end()
        while start < len(text) and text[start].isspace() and text[start] != "\n":
            start += 1
        if start < len(text) and text[start] == "{":
            end = balanced_span(text, start)
            yield text[m.start():end]
        else:
            end = text.find("\n", start)
            if end < 0:
                end = len(text)
            yield text[m.start():end]


def add_restart_metadata(qml):
    restart_keys = set()
    assign_re = re.compile(r"settings\.([A-Za-z_][A-Za-z0-9_]*)\s*=(?!=)")
    for block in handler_blocks(qml):
        if "settings_restart_to_apply = true" not in block:
            continue
        restart_keys.update(assign_re.findall(block))

    catalog = json.loads(CATALOG.read_text(encoding="utf-8"))
    by_key = {entry["key"]: entry for entry in catalog.get("settings", [])}
    for key, entry in by_key.items():
        entry["restartRequired"] = key in restart_keys

    for entry in catalog.get("virtualSettings", []):
        members = [option.get("sets") for option in entry.get("options", []) if option.get("sets")]
        entry["restartRequired"] = any(member in restart_keys for member in members)

    CATALOG.write_text(json.dumps(catalog, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


DISPLAY_AND_SETTERS = r'''
            // SETTINGS_BEHAVIOR_PARITY_V2
            function rawValue(entry) {
                var value = settings[entry.key]
                return value === undefined ? entry.defaultValue : value
            }

            function twoDigits(value) {
                var rounded = Math.round(value)
                return rounded < 10 ? "0" + rounded : "" + rounded
            }

            function durationText(seconds) {
                var total = Math.max(0, Math.round(seconds))
                var hours = Math.floor(total / 3600)
                var minutes = Math.floor((total % 3600) / 60)
                var secs = total % 60
                return twoDigits(hours) + ":" + twoDigits(minutes) + ":" + twoDigits(secs)
            }

            function paceDistance(key) {
                if (key === "pacef_1mile") return 1.60934
                if (key === "pacef_5km") return 5
                if (key === "pacef_10km") return 10
                if (key === "pacef_halfmarathon") return 21
                if (key === "pacef_marathon") return 42
                return 0
            }

            function displayValue(entry) {
                var value = rawValue(entry)
                var key = entry.key

                if ((key === "weight" || key === "bike_weight") && settings.miles_unit && !settings.weight_kg_unit)
                    return value * 2.20462

                if (key === "height" && settings.miles_unit) {
                    var feet = Math.floor(value / 30.48)
                    var inches = Math.round((value % 30.48) / 2.54)
                    return feet + "'" + inches + '"'
                }

                if ((key === "autolap_distance" || key === "treadmill_speed_min" ||
                     key === "treadmill_speed_max" || key === "treadmill_step_speed" ||
                     key === "peloton_treadmill_running_min_speed" || key === "peloton_treadmill_walking_min_speed") && settings.miles_unit)
                    return value * 0.621371

                var distance = paceDistance(key)
                if (distance > 0)
                    return durationText(value * distance)

                return value
            }

            function setExclusive(keys, selectedKey, checked) {
                for (var i = 0; i < keys.length; i++)
                    settings[keys[i]] = keys[i] === selectedKey ? !!checked : false
            }

            function parseDuration(value) {
                var parts = ("" + value).split(":")
                if (parts.length !== 3)
                    return -1
                var h = parseInt(parts[0])
                var m = parseInt(parts[1])
                var s = parseInt(parts[2])
                if (isNaN(h) || isNaN(m) || isNaN(s) || m < 0 || m > 59 || s < 0 || s > 59)
                    return -1
                return h * 3600 + m * 60 + s
            }

            function setSettingValue(entry, value) {
                var key = entry.key

                if (key === "weight" || key === "bike_weight") {
                    var weightValue = parseFloat(value)
                    if (settings.miles_unit && !settings.weight_kg_unit)
                        weightValue = weightValue / 2.20462
                    settings[key] = weightValue
                    afterGenericWrite(entry)
                    return
                }

                if (key === "height") {
                    if (settings.miles_unit) {
                        var match = ("" + value).match(/(\d+)[\s''\u2018\u2019]*(\d+)/)
                        if (!match) {
                            toast.show(qsTr("Invalid format! Use feet'inches (e.g., 6'2\")"))
                            return
                        }
                        settings.height = parseInt(match[1]) * 30.48 + parseInt(match[2]) * 2.54
                    } else {
                        settings.height = parseFloat(value)
                    }
                    afterGenericWrite(entry)
                    return
                }

                if (key === "autolap_distance" || key === "treadmill_speed_min" || key === "treadmill_speed_max" || key === "treadmill_step_speed") {
                    var metricValue = parseFloat(value)
                    if (settings.miles_unit)
                        metricValue = metricValue * 1.60934
                    settings[key] = metricValue
                    afterGenericWrite(entry)
                    return
                }

                if (key === "peloton_treadmill_running_min_speed" || key === "peloton_treadmill_walking_min_speed") {
                    var pelotonSpeed = parseFloat(value)
                    if (settings.miles_unit)
                        pelotonSpeed = pelotonSpeed / 0.621371
                    settings[key] = pelotonSpeed
                    afterGenericWrite(entry)
                    return
                }

                var distance = paceDistance(key)
                if (distance > 0) {
                    var seconds = parseDuration(value)
                    if (seconds < 0) {
                        toast.show(qsTr("Invalid time format! Use hh:mm:ss"))
                        return
                    }
                    settings[key] = seconds / distance
                    afterGenericWrite(entry)
                    return
                }

                if (key === "domyos_bike_500_profile_v1" || key === "domyos_bike_500_profile_v2") {
                    setExclusive(["domyos_bike_500_profile_v1", "domyos_bike_500_profile_v2"], key, value)
                    afterGenericWrite(entry)
                    return
                }

                if (key === "kingsmith_encrypt_v2" || key === "kingsmith_encrypt_v3" || key === "kingsmith_encrypt_v4" ||
                    key === "kingsmith_encrypt_v5" || key === "kingsmith_encrypt_g1_walking_pad") {
                    setExclusive(["kingsmith_encrypt_v2", "kingsmith_encrypt_v3", "kingsmith_encrypt_v4", "kingsmith_encrypt_v5", "kingsmith_encrypt_g1_walking_pad"], key, value)
                    afterGenericWrite(entry)
                    return
                }

                if (key === "peloton_auto_start_with_intro" || key === "peloton_auto_start_without_intro") {
                    settings[key] = !!value
                    if (value)
                        settings[key === "peloton_auto_start_with_intro" ? "peloton_auto_start_without_intro" : "peloton_auto_start_with_intro"] = false
                    afterGenericWrite(entry)
                    return
                }

                if (key === "zwift_ocr" || key === "zwift_ocr_climb_portal" || key === "zwift_workout_ocr") {
                    setExclusive(["zwift_ocr", "zwift_ocr_climb_portal", "zwift_workout_ocr"], key, value)
                    settings.android_notification = true
                    afterGenericWrite(entry)
                    return
                }

                if (key === "zwift_play_emulator") {
                    if (!!value && !settings.zwift_play_emulator) {
                        if (settings.zwift_play || settings.zwift_click)
                            zwiftPlaySettingsDialog.visible = true
                        settings.watt_bike_emulator = false
                    }
                    settings.zwift_play_emulator = !!value
                    afterGenericWrite(entry)
                    return
                }

                if (key === "watt_bike_emulator") {
                    settings.watt_bike_emulator = !!value
                    if (value)
                        settings.zwift_play_emulator = false
                    afterGenericWrite(entry)
                    return
                }

                if (entry.type === "boolean") {
                    settings[key] = !!value
                } else if (entry.type === "integer") {
                    settings[key] = parseInt(value)
                } else if (entry.type === "number") {
                    settings[key] = parseFloat(value)
                } else {
                    settings[key] = value
                }

                if (key === "watt_offset" || key === "watt_gain" || key === "power_sensor_name") {
                    settings.treadmillDataPoints = ""
                    settings.ergDataPoints = ""
                }

                afterGenericWrite(entry)
            }
'''


def migrate_qml(text):
    if MARKER in text:
        return text

    old = '''            function setSettingValue(entry, value) {\n                if (entry.type === "boolean") {\n                    settings[entry.key] = !!value\n                } else if (entry.type === "integer") {\n                    settings[entry.key] = parseInt(value)\n                } else if (entry.type === "number") {\n                    settings[entry.key] = parseFloat(value)\n                } else {\n                    settings[entry.key] = value\n                }\n                afterGenericWrite(entry)\n            }'''
    if old not in text:
        raise SystemExit("Cannot locate shared setSettingValue controller")
    text = text.replace(old, DISPLAY_AND_SETTERS.rstrip(), 1)

    outer = '''        function settingValue(entry) {\n            var value = settings[entry.key]\n            return value === undefined ? entry.defaultValue : value\n        }'''
    replacement = outer + '''\n\n        function displaySettingValue(entry) {\n            return settingsBehavior.displayValue(entry)\n        }'''
    if outer not in text:
        raise SystemExit("Cannot locate settingValue wrapper")
    text = text.replace(outer, replacement, 1)

    text = text.replace('text: visible ? settingsPane.settingValue(entry) : ""',
                        'text: visible ? settingsPane.displaySettingValue(entry) : ""')

    completed = '''        Component.onCompleted: {\n            window.settings_restart_to_apply = false;\n        }'''
    promoted = '''        Component.onCompleted: {\n            window.settings_restart_to_apply = false;\n            Qt.callLater(function() { settingsPane.openModernSettingsPreview() })\n        }'''
    if completed not in text:
        raise SystemExit("Cannot locate settings Component.onCompleted")
    text = text.replace(completed, promoted, 1)
    return text


def main():
    qml = QML.read_text(encoding="utf-8")
    add_restart_metadata(qml)
    migrated = migrate_qml(qml)
    QML.write_text(migrated, encoding="utf-8")
    print("Phase 4 behavior parity and modern-default migration applied")


if __name__ == "__main__":
    main()
