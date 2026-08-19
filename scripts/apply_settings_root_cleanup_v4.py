#!/usr/bin/env python3
from pathlib import Path

QML = Path("src/settings.qml")
HIER = Path("scripts/settings_hierarchy.py")


def replace_function(source: str, name: str, replacement: str) -> str:
    needle = "function " + name + "("
    start = source.find(needle)
    if start < 0:
        return source
    brace = source.find("{", start)
    if brace < 0:
        raise RuntimeError("missing opening brace for " + name)
    depth = 0
    single = double = escaped = False
    for i in range(brace, len(source)):
        ch = source[i]
        if escaped:
            escaped = False
            continue
        if ch == "\\" and (single or double):
            escaped = True
            continue
        if ch == "'" and not double:
            single = not single
            continue
        if ch == '"' and not single:
            double = not double
            continue
        if single or double:
            continue
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return source[:start] + replacement + source[i + 1:]
    raise RuntimeError("unterminated function " + name)


qml = QML.read_text(encoding="utf-8")

# Root icons are intentionally removed: Unicode emoji rendering is not consistent
# enough across iOS, Android, Windows and Linux to keep row text aligned.
qml = replace_function(qml, "legacyRootIconColor", "")
qml = replace_function(qml, "legacyRootIcon", "")

old_root_row = '''                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 16
                            anchors.rightMargin: 12

                            Rectangle {
                                Layout.preferredWidth: 34
                                Layout.preferredHeight: 34
                                radius: 8
                                color: settingsPane.legacyRootIconColor(modelData.name)
                                Label {
                                    anchors.centerIn: parent
                                    text: settingsPane.legacyRootIcon(modelData.name)
                                    color: "white"
                                    font.bold: true
                                    font.pixelSize: Qt.application.font.pixelSize + 2
                                }
                            }
                            Label {
                                Layout.fillWidth: true
                                text: modelData.name
                                font.pixelSize: Qt.application.font.pixelSize + 1
                                elide: Text.ElideRight
                            }
                            Label {
                                text: "›"
                                color: Material.color(Material.Grey)
                                font.pixelSize: Qt.application.font.pixelSize + 8
                            }
                        }
'''
new_root_row = '''                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 18
                            anchors.rightMargin: 14
                            spacing: 8

                            Label {
                                Layout.fillWidth: true
                                text: modelData.name
                                font.pixelSize: Qt.application.font.pixelSize + 1
                                verticalAlignment: Text.AlignVCenter
                                elide: Text.ElideRight
                            }
                            Label {
                                text: "›"
                                color: Material.color(Material.Grey)
                                font.pixelSize: Qt.application.font.pixelSize + 8
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
'''
if old_root_row in qml:
    qml = qml.replace(old_root_row, new_root_row, 1)
elif new_root_row not in qml:
    raise RuntimeError("modern root delegate row not found")

# Keep the legacy controls compiled for now, but remove them from the user
# experience while runtime validation of the modern renderer continues.
if "property bool legacySettingsUiEnabled" not in qml:
    marker = '        property bool settingsSearchPending: false\n'
    if marker not in qml:
        raise RuntimeError("settings state marker not found")
    qml = qml.replace(marker, marker + '        property bool legacySettingsUiEnabled: false\n', 1)

old_visible = '                visible: !settingsSearchActive\n                spacing: 0\n                Layout.fillWidth: true\n\n            AccordionElement {'
new_visible = '                // Legacy settings UX retained as compiled fallback during validation, hidden by default.\n                visible: legacySettingsUiEnabled && !settingsSearchActive\n                spacing: 0\n                Layout.fillWidth: true\n\n            AccordionElement {'
if old_visible in qml:
    qml = qml.replace(old_visible, new_visible, 1)
elif new_visible not in qml:
    raise RuntimeError("legacy settingsContent visibility marker not found")

if "// MODERN_SETTINGS_ROOT_TEXT_ONLY_V4" not in qml:
    marker = "        // MODERN_SETTINGS_IOS_GROUPED_V3\n"
    qml = qml.replace(marker, marker + "        // MODERN_SETTINGS_ROOT_TEXT_ONLY_V4\n", 1)

QML.write_text(qml, encoding="utf-8")

hier = HIER.read_text(encoding="utf-8")
old_decode = '                    node["name"] = bytes(m.group(1), "utf-8").decode("unicode_escape")'
json_decode = '                    node["name"] = json.loads(\'"\' + m.group(1) + \'"\')'
new_decode = '                    node["name"] = json.loads(\'"\' + m.group(1) + \'"\').strip()'
if old_decode in hier:
    hier = hier.replace(old_decode, new_decode, 1)
elif json_decode in hier:
    hier = hier.replace(json_decode, new_decode, 1)
elif new_decode not in hier:
    raise RuntimeError("hierarchy title decoder marker not found")
HIER.write_text(hier, encoding="utf-8")

print("Applied modern settings root cleanup v4")
