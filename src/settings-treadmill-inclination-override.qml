import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0

ScrollView {
    id: settingsInclinationPane
    contentWidth: -1
    focus: true
    anchors.fill: parent

    Settings {
        id: settings
        property double treadmill_inclination_override_0: 0.0
        property double treadmill_inclination_override_05: 0.5
        property double treadmill_inclination_override_10: 1.0
        property double treadmill_inclination_override_15: 1.5
        property double treadmill_inclination_override_20: 2.0
        property double treadmill_inclination_override_25: 2.5
        property double treadmill_inclination_override_30: 3.0
        property double treadmill_inclination_override_35: 3.5
        property double treadmill_inclination_override_40: 4.0
        property double treadmill_inclination_override_45: 4.5
        property double treadmill_inclination_override_50: 5.0
        property double treadmill_inclination_override_55: 5.5
        property double treadmill_inclination_override_60: 6.0
        property double treadmill_inclination_override_65: 6.5
        property double treadmill_inclination_override_70: 7.0
        property double treadmill_inclination_override_75: 7.5
        property double treadmill_inclination_override_80: 8.0
        property double treadmill_inclination_override_85: 8.5
        property double treadmill_inclination_override_90: 9.0
        property double treadmill_inclination_override_95: 9.5
        property double treadmill_inclination_override_100: 10.0
        property double treadmill_inclination_override_105: 10.5
        property double treadmill_inclination_override_110: 11.0
        property double treadmill_inclination_override_115: 11.5
        property double treadmill_inclination_override_120: 12.0
        property double treadmill_inclination_override_125: 12.5
        property double treadmill_inclination_override_130: 13.0
        property double treadmill_inclination_override_135: 13.5
        property double treadmill_inclination_override_140: 14.0
        property double treadmill_inclination_override_145: 14.5
        property double treadmill_inclination_override_150: 15.0

        property double treadmill_inclination_override_device_command_0: 0.0
        property double treadmill_inclination_override_device_command_05: 0.5
        property double treadmill_inclination_override_device_command_10: 1.0
        property double treadmill_inclination_override_device_command_15: 1.5
        property double treadmill_inclination_override_device_command_20: 2.0
        property double treadmill_inclination_override_device_command_25: 2.5
        property double treadmill_inclination_override_device_command_30: 3.0
        property double treadmill_inclination_override_device_command_35: 3.5
        property double treadmill_inclination_override_device_command_40: 4.0
        property double treadmill_inclination_override_device_command_45: 4.5
        property double treadmill_inclination_override_device_command_50: 5.0
        property double treadmill_inclination_override_device_command_55: 5.5
        property double treadmill_inclination_override_device_command_60: 6.0
        property double treadmill_inclination_override_device_command_65: 6.5
        property double treadmill_inclination_override_device_command_70: 7.0
        property double treadmill_inclination_override_device_command_75: 7.5
        property double treadmill_inclination_override_device_command_80: 8.0
        property double treadmill_inclination_override_device_command_85: 8.5
        property double treadmill_inclination_override_device_command_90: 9.0
        property double treadmill_inclination_override_device_command_95: 9.5
        property double treadmill_inclination_override_device_command_100: 10.0
        property double treadmill_inclination_override_device_command_105: 10.5
        property double treadmill_inclination_override_device_command_110: 11.0
        property double treadmill_inclination_override_device_command_115: 11.5
        property double treadmill_inclination_override_device_command_120: 12.0
        property double treadmill_inclination_override_device_command_125: 12.5
        property double treadmill_inclination_override_device_command_130: 13.0
        property double treadmill_inclination_override_device_command_135: 13.5
        property double treadmill_inclination_override_device_command_140: 14.0
        property double treadmill_inclination_override_device_command_145: 14.5
        property double treadmill_inclination_override_device_command_150: 15.0

        property double treadmill_inclination_ovveride_gain: 1.0
        property double treadmill_inclination_ovveride_offset: 0.0
    }

    ColumnLayout {
        width: settingsInclinationPane.availableWidth
        spacing: 8

        Label {
            text: qsTr("Treadmill Inclination Overrides")
            color: Material.color(Material.Red)
            Layout.fillWidth: true
        }

        Label {
            text: qsTr("Each raw/input row maps treadmill feedback to the Logical Inclination used throughout QZ. Device Command controls the value sent when reverse lookup selects that row. Leave Device Command at its row value to preserve legacy behavior.")
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            Label { text: qsTr("Gain"); Layout.fillWidth: true }
            SettingsNumberField {
                id: gainField
                Accessible.name: qsTr("Gain")
                signed: true
                text: settings.treadmill_inclination_ovveride_gain
                horizontalAlignment: Text.AlignRight
                Layout.preferredWidth: 90
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_ovveride_gain = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("Save")
                enabled: gainField.valid
                onClicked: { settings.treadmill_inclination_ovveride_gain = gainField.value; toast.show(qsTr("Setting saved!")); }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Label { text: qsTr("Offset"); Layout.fillWidth: true }
            SettingsNumberField {
                id: offsetField
                Accessible.name: qsTr("Offset")
                signed: true
                text: settings.treadmill_inclination_ovveride_offset
                horizontalAlignment: Text.AlignRight
                Layout.preferredWidth: 90
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_ovveride_offset = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("Save")
                enabled: offsetField.valid
                onClicked: { settings.treadmill_inclination_ovveride_offset = offsetField.value; toast.show(qsTr("Setting saved!")); }
            }
        }

        Repeater {
            model: [
            { raw: "0.0", suffix: "0" },
            { raw: "0.5", suffix: "05" },
            { raw: "1.0", suffix: "10" },
            { raw: "1.5", suffix: "15" },
            { raw: "2.0", suffix: "20" },
            { raw: "2.5", suffix: "25" },
            { raw: "3.0", suffix: "30" },
            { raw: "3.5", suffix: "35" },
            { raw: "4.0", suffix: "40" },
            { raw: "4.5", suffix: "45" },
            { raw: "5.0", suffix: "50" },
            { raw: "5.5", suffix: "55" },
            { raw: "6.0", suffix: "60" },
            { raw: "6.5", suffix: "65" },
            { raw: "7.0", suffix: "70" },
            { raw: "7.5", suffix: "75" },
            { raw: "8.0", suffix: "80" },
            { raw: "8.5", suffix: "85" },
            { raw: "9.0", suffix: "90" },
            { raw: "9.5", suffix: "95" },
            { raw: "10.0", suffix: "100" },
            { raw: "10.5", suffix: "105" },
            { raw: "11.0", suffix: "110" },
            { raw: "11.5", suffix: "115" },
            { raw: "12.0", suffix: "120" },
            { raw: "12.5", suffix: "125" },
            { raw: "13.0", suffix: "130" },
            { raw: "13.5", suffix: "135" },
            { raw: "14.0", suffix: "140" },
            { raw: "14.5", suffix: "145" },
            { raw: "15.0", suffix: "150" }
            ]

            delegate: Frame {
                Layout.fillWidth: true
                ColumnLayout {
                    anchors.fill: parent
                    Label {
                        text: qsTr("Raw/Input %1%").arg(modelData.raw)
                        font.bold: true
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: qsTr("Device Command"); Layout.fillWidth: true }
                        SettingsNumberField {
                            id: commandField
                            Accessible.name: qsTr("Device Command")
                            signed: true
                            text: settings["treadmill_inclination_override_device_command_" + modelData.suffix]
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            Layout.preferredWidth: 90
                            onAccepted: settings["treadmill_inclination_override_device_command_" + modelData.suffix] = value
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            text: qsTr("OK")
                            enabled: commandField.valid
                            onClicked: settings["treadmill_inclination_override_device_command_" + modelData.suffix] = commandField.value
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: qsTr("Logical Inclination"); Layout.fillWidth: true }
                        SettingsNumberField {
                            id: logicalField
                            Accessible.name: qsTr("Logical Inclination")
                            signed: true
                            text: settings["treadmill_inclination_override_" + modelData.suffix]
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            Layout.preferredWidth: 90
                            onAccepted: settings["treadmill_inclination_override_" + modelData.suffix] = value
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            text: qsTr("OK")
                            enabled: logicalField.valid
                            onClicked: settings["treadmill_inclination_override_" + modelData.suffix] = logicalField.value
                        }
                    }
                    Button {
                        text: qsTr("Save")
                        enabled: commandField.valid && logicalField.valid
                        Layout.alignment: Qt.AlignRight
                        onClicked: {
                            settings["treadmill_inclination_override_device_command_" + modelData.suffix] = commandField.value
                            settings["treadmill_inclination_override_" + modelData.suffix] = logicalField.value
                            toast.show(qsTr("Settings saved!"))
                        }
                    }
                }
            }
        }
    }
}
