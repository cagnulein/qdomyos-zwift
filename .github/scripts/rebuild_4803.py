from pathlib import Path
import json
import re

KEY = "power_sensor_speed_correction_threshold"


def patch_qzsettings_cpp():
    p = Path("src/qzsettings.cpp")
    s = p.read_text()
    if f'QZSettings::{KEY} = QStringLiteral("{KEY}")' in s:
        return

    anchor = 'const QString QZSettings::power_sensor_as_treadmill = QStringLiteral("power_sensor_as_treadmill");\n'
    assert anchor in s
    s = s.replace(anchor, anchor + f'const QString QZSettings::{KEY} = QStringLiteral("{KEY}");\n', 1)

    m = re.search(r'const uint32_t allSettingsCount = (\d+);', s)
    assert m
    s = s[:m.start()] + f'const uint32_t allSettingsCount = {int(m.group(1)) + 1};' + s[m.end():]

    anchor = '    {QZSettings::power_sensor_as_treadmill, QZSettings::default_power_sensor_as_treadmill},\n'
    assert anchor in s
    s = s.replace(anchor, anchor + f'    {{QZSettings::{KEY}, QZSettings::default_{KEY}}},\n', 1)
    p.write_text(s)


def matching_object_end(text, start):
    depth = 0
    in_string = False
    escaped = False
    for i in range(start, len(text)):
        ch = text[i]
        if in_string:
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif ch == '"':
                in_string = False
            continue
        if ch == '"':
            in_string = True
        elif ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return i + 1
    raise AssertionError("unterminated JSON object")


def patch_catalog():
    p = Path("src/settings-catalog.json")
    s = p.read_text()
    if f'"key": "{KEY}"' in s:
        return

    m = re.search(r'"settingCount"\s*:\s*(\d+)', s)
    assert m
    s = s[:m.start(1)] + str(int(m.group(1)) + 1) + s[m.end(1):]

    pos = s.index('"key": "power_sensor_as_treadmill"')
    start = s.rfind("{", 0, pos)
    assert start >= 0
    end = matching_object_end(s, start)
    line_start = s.rfind("\n", 0, start) + 1
    indent = s[line_start:start]

    entry = (
        ",\n" + indent + "{\n"
        + indent + f'  "key": "{KEY}",\n'
        + indent + '  "name": "Power sensor speed correction threshold (%)",\n'
        + indent + '  "description": "Maximum allowed difference between power sensor speed and treadmill speed before QZ stops applying the relative correction. Default: 20%.",\n'
        + indent + '  "parent": "Power Sensor Options",\n'
        + indent + '  "type": "number",\n'
        + indent + '  "qmlType": "real",\n'
        + indent + '  "control": "text",\n'
        + indent + '  "visible": true,\n'
        + indent + '  "defaultValue": 20.0,\n'
        + indent + '  "defaultExpression": "20.0",\n'
        + indent + '  "options": null\n'
        + indent + "}"
    )

    j = end
    while j < len(s) and s[j] in " \t\r\n":
        j += 1
    assert j < len(s) and s[j] == ","
    s = s[:end] + entry + s[j + 1:]
    json.loads(s)
    p.write_text(s)


def patch_settings_qml():
    p = Path("src/settings.qml")
    s = p.read_text()

    if f"property real {KEY}:" not in s:
        function_pos = s.index("        function paddingZeros")
        before = s[:function_pos]
        props = list(re.finditer(r"^\s+property\s+[^\n]+$", before, re.MULTILINE))
        assert props
        last = props[-1]
        s = s[:last.end()] + f"\n            property real {KEY}: 20.0" + s[last.end():]

    if "id: powerSensorSpeedCorrectionThresholdTextField" not in s:
        label_pos = s.index('text: qsTr("Use inclination from the power sensor")')
        insert_at = s.rfind("                            IndicatorOnlySwitch {", 0, label_pos)
        assert insert_at >= 0
        block = '''                            Label {
                                text: qsTr("Power sensor speed correction threshold (%):")
                                Layout.fillWidth: true
                            }
                            RowLayout {
                                spacing: 10
                                TextField {
                                    id: powerSensorSpeedCorrectionThresholdTextField
                                    text: settings.power_sensor_speed_correction_threshold
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                                    validator: DoubleValidator { bottom: 0; top: 100 }
                                }
                                Button {
                                    text: qsTr("OK")
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: {
                                        var value = Number(powerSensorSpeedCorrectionThresholdTextField.text)
                                        if (isNaN(value)) value = 20
                                        settings.power_sensor_speed_correction_threshold = Math.max(0, Math.min(100, value))
                                        powerSensorSpeedCorrectionThresholdTextField.text = settings.power_sensor_speed_correction_threshold
                                        toast.show(qsTr("Setting saved!"))
                                    }
                                }
                            }

                            Label {
                                text: qsTr("Maximum allowed difference between power sensor speed and treadmill speed before QZ stops applying the relative correction. Default: 20%.")
                                font.bold: true
                                font.italic: true
                                font.pixelSize: Qt.application.font.pixelSize - 2
                                textFormat: Text.PlainText
                                wrapMode: Text.WordWrap
                                verticalAlignment: Text.AlignVCenter
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                color: Material.color(Material.Lime)
                            }

'''
        s = s[:insert_at] + block + s[insert_at:]

    p.write_text(s)


patch_qzsettings_cpp()
patch_catalog()
patch_settings_qml()

# One-shot helpers must not survive in the final branch.
Path(".github/workflows/rebuild-4803-sync.yml").unlink(missing_ok=True)
Path(".github/scripts/rebuild_4803.py").unlink(missing_ok=True)
