#!/usr/bin/env python3
"""Introduce a renderer-independent settings behavior controller in settings.qml.

The migration intentionally changes only the two generic catalog/search mutation paths.
Legacy hand-written controls remain untouched. The controller defaults restartRequired to
true, preserving the current generic-search behavior unless catalog metadata explicitly
opts out.
"""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
QML = ROOT / "src/settings.qml"

MARKER = "// SETTINGS_BEHAVIOR_CONTROLLER_V1"

CONTROLLER = r'''
        // SETTINGS_BEHAVIOR_CONTROLLER_V1
        QtObject {
            id: settingsBehavior

            function restartRequired(entry) {
                return entry.restartRequired === undefined ? true : !!entry.restartRequired
            }

            function afterGenericWrite(entry) {
                if (restartRequired(entry))
                    window.settings_restart_to_apply = true
                toast.show("Setting saved!")
            }

            function setSettingValue(entry, value) {
                if (entry.type === "boolean") {
                    settings[entry.key] = !!value
                } else if (entry.type === "integer") {
                    settings[entry.key] = parseInt(value)
                } else if (entry.type === "number") {
                    settings[entry.key] = parseFloat(value)
                } else {
                    settings[entry.key] = value
                }
                afterGenericWrite(entry)
            }

            function setVirtualSelection(entry, index) {
                if (!entry.options)
                    return

                for (var i = 0; i < entry.options.length; i++) {
                    if (entry.options[i].sets)
                        settings[entry.options[i].sets] = false
                }

                if (entry.options[index] && entry.options[index].sets)
                    settings[entry.options[index].sets] = true

                afterGenericWrite(entry)
            }
        }

'''


def function_span(text: str, name: str):
    needle = "function " + name + "("
    start = text.find(needle)
    if start < 0:
        raise RuntimeError(f"Cannot find {needle}")
    brace = text.find("{", start)
    if brace < 0:
        raise RuntimeError(f"Cannot find opening brace for {name}")
    depth = 0
    in_single = in_double = escaped = False
    i = brace
    while i < len(text):
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
                    return start, i + 1
        i += 1
    raise RuntimeError(f"Unbalanced function {name}")


def replace_function(text: str, name: str, replacement: str) -> str:
    start, end = function_span(text, name)
    indent_start = text.rfind("\n", 0, start) + 1
    indent = text[indent_start:start]
    replacement = replacement.replace("\n", "\n" + indent).rstrip()
    return text[:start] + replacement + text[end:]


def main() -> int:
    text = QML.read_text(encoding="utf-8")
    if MARKER not in text:
        anchor = text.find("function settingValue(")
        if anchor < 0:
            raise SystemExit("Cannot locate catalog helper functions")
        line_start = text.rfind("\n", 0, anchor) + 1
        text = text[:line_start] + CONTROLLER + text[line_start:]

    text = replace_function(
        text,
        "setSettingValue",
        "function setSettingValue(entry, value) {\n    settingsBehavior.setSettingValue(entry, value)\n}",
    )
    text = replace_function(
        text,
        "setVirtualSelection",
        "function setVirtualSelection(entry, index) {\n    settingsBehavior.setVirtualSelection(entry, index)\n}",
    )

    QML.write_text(text, encoding="utf-8")
    print("Shared settings behavior controller installed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
