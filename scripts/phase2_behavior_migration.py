#!/usr/bin/env python3
"""Introduce a renderer-independent settings behavior controller in settings.qml.

Only the two generic catalog/search mutation paths are redirected. Legacy hand-written
controls stay untouched. The generic restart policy defaults to true so behavior is
preserved unless catalog metadata explicitly opts out.
"""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
QML = ROOT / "src/settings.qml"
MARKER = "// SETTINGS_BEHAVIOR_CONTROLLER_V1"
ANCHOR = "function settingValue(entry)"

CONTROLLER = r'''        // SETTINGS_BEHAVIOR_CONTROLLER_V1
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


def function_span(text: str, name: str, start_at: int = 0):
    needle = "function " + name + "("
    start = text.find(needle, start_at)
    if start < 0:
        raise RuntimeError(f"Cannot find {needle} after offset {start_at}")
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


def replace_function_after(text: str, name: str, start_at: int, replacement: str) -> str:
    start, end = function_span(text, name, start_at)
    indent_start = text.rfind("\n", 0, start) + 1
    indent = text[indent_start:start]
    replacement = replacement.replace("\n", "\n" + indent).rstrip()
    return text[:start] + replacement + text[end:]


def install_controller(text: str) -> str:
    anchor = text.find(ANCHOR)
    if anchor < 0:
        raise RuntimeError("Cannot locate settingValue anchor")
    anchor_line = text.rfind("\n", 0, anchor) + 1

    marker = text.find(MARKER)
    if marker >= 0:
        marker_line = text.rfind("\n", 0, marker) + 1
        # Everything from the marker through the line immediately before settingValue
        # is the generated controller and can be deterministically replaced.
        return text[:marker_line] + CONTROLLER + text[anchor_line:]
    return text[:anchor_line] + CONTROLLER + text[anchor_line:]


def main() -> int:
    text = QML.read_text(encoding="utf-8")
    text = install_controller(text)

    outer_anchor = text.find(ANCHOR)
    text = replace_function_after(
        text,
        "setSettingValue",
        outer_anchor,
        "function setSettingValue(entry, value) {\n    settingsBehavior.setSettingValue(entry, value)\n}",
    )
    # Recompute after the first replacement because offsets may move.
    outer_anchor = text.find(ANCHOR)
    text = replace_function_after(
        text,
        "setVirtualSelection",
        outer_anchor,
        "function setVirtualSelection(entry, index) {\n    settingsBehavior.setVirtualSelection(entry, index)\n}",
    )

    QML.write_text(text, encoding="utf-8")
    print("Shared settings behavior controller installed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
