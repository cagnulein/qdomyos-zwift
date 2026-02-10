import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0

ScrollView {
    contentWidth: -1
    focus: true
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.fill: parent
    id: settingsShortcutsPane
    objectName: "settingsShortcutsPane"

    Settings {
        id: settings
        property bool shortcuts_enabled: false
        property string shortcut_speed_plus: ""
        property string shortcut_speed_minus: ""
        property string shortcut_inclination_plus: ""
        property string shortcut_inclination_minus: ""
        property string shortcut_resistance_plus: ""
        property string shortcut_resistance_minus: ""
        property string shortcut_peloton_resistance_plus: ""
        property string shortcut_peloton_resistance_minus: ""
        property string shortcut_target_resistance_plus: ""
        property string shortcut_target_resistance_minus: ""
        property string shortcut_target_power_plus: ""
        property string shortcut_target_power_minus: ""
        property string shortcut_target_zone_plus: ""
        property string shortcut_target_zone_minus: ""
        property string shortcut_target_speed_plus: ""
        property string shortcut_target_speed_minus: ""
        property string shortcut_target_incline_plus: ""
        property string shortcut_target_incline_minus: ""
        property string shortcut_fan_plus: ""
        property string shortcut_fan_minus: ""
        property string shortcut_peloton_offset_plus: ""
        property string shortcut_peloton_offset_minus: ""
        property string shortcut_peloton_remaining_plus: ""
        property string shortcut_peloton_remaining_minus: ""
        property string shortcut_remaining_time_plus: ""
        property string shortcut_remaining_time_minus: ""
        property string shortcut_gears_plus: ""
        property string shortcut_gears_minus: ""
        property string shortcut_pid_hr_plus: ""
        property string shortcut_pid_hr_minus: ""
        property string shortcut_ext_incline_plus: ""
        property string shortcut_ext_incline_minus: ""
        property string shortcut_biggears_plus: ""
        property string shortcut_biggears_minus: ""
        property string shortcut_avs_cruise: ""
        property string shortcut_avs_climb: ""
        property string shortcut_avs_sprint: ""
        property string shortcut_power_avg: ""
        property string shortcut_erg_mode: ""
        property string shortcut_preset_resistance_1: ""
        property string shortcut_preset_resistance_2: ""
        property string shortcut_preset_resistance_3: ""
        property string shortcut_preset_resistance_4: ""
        property string shortcut_preset_resistance_5: ""
        property string shortcut_preset_speed_1: ""
        property string shortcut_preset_speed_2: ""
        property string shortcut_preset_speed_3: ""
        property string shortcut_preset_speed_4: ""
        property string shortcut_preset_speed_5: ""
        property string shortcut_preset_inclination_1: ""
        property string shortcut_preset_inclination_2: ""
        property string shortcut_preset_inclination_3: ""
        property string shortcut_preset_inclination_4: ""
        property string shortcut_preset_inclination_5: ""
        property string shortcut_preset_powerzone_1: ""
        property string shortcut_preset_powerzone_2: ""
        property string shortcut_preset_powerzone_3: ""
        property string shortcut_preset_powerzone_4: ""
        property string shortcut_preset_powerzone_5: ""
        property string shortcut_preset_powerzone_6: ""
        property string shortcut_preset_powerzone_7: ""
        property string shortcut_auto_resistance: ""
        property string shortcut_lap: ""
        property string shortcut_start_stop: ""
    }

    ColumnLayout {
        id: mainLayout
        spacing: 10
        anchors.fill: parent
        anchors.margins: 10

        Label {
            text: qsTr("Keyboard Shortcuts")
            font.pixelSize: 24
            font.bold: true
            color: Material.color(Material.Red)
        }

        SwitchDelegate {
            id: shortcutsEnabledDelegate
            text: qsTr("Enable Keyboard Shortcuts")
            checked: settings.shortcuts_enabled
            Layout.fillWidth: true
            onClicked: settings.shortcuts_enabled = checked
        }

        Label {
            text: qsTr("Click on a field and press a key to assign a shortcut. Press Backspace to clear.")
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        component ShortcutField : TextField {
            property string settingName: ""
            placeholderText: qsTr("None")
            horizontalAlignment: Text.AlignRight
            Layout.preferredWidth: 100
            readOnly: true
            selectByMouse: false

            MouseArea {
                anchors.fill: parent
                onClicked: parent.forceActiveFocus()
            }

            onActiveFocusChanged: {
                if (activeFocus) {
                    cursorPosition = text.length
                }
            }
            Keys.onPressed: (event) => {
                if (event.key === Qt.Key_Backspace || event.key === Qt.Key_Delete) {
                    settings[settingName] = ""
                } else if (event.text !== "") {
                    settings[settingName] = event.text.toUpperCase()
                }
                event.accepted = true
            }
        }

        GroupBox {
            title: qsTr("General Controls")
            Layout.fillWidth: true
            ColumnLayout {
                anchors.fill: parent
                RowLayout {
                    Label { text: qsTr("Start / Stop"); Layout.fillWidth: true }
                    ShortcutField { text: settings.shortcut_start_stop; settingName: "shortcut_start_stop" }
                }
                RowLayout {
                    Label { text: qsTr("Lap"); Layout.fillWidth: true }
                    ShortcutField { text: settings.shortcut_lap; settingName: "shortcut_lap" }
                }
            }
        }

        GroupBox {
            title: qsTr("Main Metrics")
            Layout.fillWidth: true
            ColumnLayout {
                anchors.fill: parent
                RowLayout {
                    Label { text: qsTr("Speed + / -"); Layout.fillWidth: true }
                    ShortcutField { text: settings.shortcut_speed_plus; settingName: "shortcut_speed_plus" }
                    ShortcutField { text: settings.shortcut_speed_minus; settingName: "shortcut_speed_minus" }
                }
                RowLayout {
                    Label { text: qsTr("Inclination + / -"); Layout.fillWidth: true }
                    ShortcutField { text: settings.shortcut_inclination_plus; settingName: "shortcut_inclination_plus" }
                    ShortcutField { text: settings.shortcut_inclination_minus; settingName: "shortcut_inclination_minus" }
                }
                RowLayout {
                    Label { text: qsTr("Resistance + / -"); Layout.fillWidth: true }
                    ShortcutField { text: settings.shortcut_resistance_plus; settingName: "shortcut_resistance_plus" }
                    ShortcutField { text: settings.shortcut_resistance_minus; settingName: "shortcut_resistance_minus" }
                }
                RowLayout {
                    Label { text: qsTr("Gears + / -"); Layout.fillWidth: true }
                    ShortcutField { text: settings.shortcut_gears_plus; settingName: "shortcut_gears_plus" }
                    ShortcutField { text: settings.shortcut_gears_minus; settingName: "shortcut_gears_minus" }
                }
                RowLayout {
                    Label { text: qsTr("Gears Big Buttons + / -"); Layout.fillWidth: true }
                    ShortcutField { text: settings.shortcut_biggears_plus; settingName: "shortcut_biggears_plus" }
                    ShortcutField { text: settings.shortcut_biggears_minus; settingName: "shortcut_biggears_minus" }
                }
            }
        }

        GroupBox {
            title: qsTr("Target Controls")
            Layout.fillWidth: true
            ColumnLayout {
                anchors.fill: parent
                RowLayout {
                    Label { text: qsTr("Target Resistance + / -"); Layout.fillWidth: true }
                    ShortcutField { text: settings.shortcut_target_resistance_plus; settingName: "shortcut_target_resistance_plus" }
                    ShortcutField { text: settings.shortcut_target_resistance_minus; settingName: "shortcut_target_resistance_minus" }
                }
                RowLayout {
                    Label { text: qsTr("Target Power + / -"); Layout.fillWidth: true }
                    ShortcutField { text: settings.shortcut_target_power_plus; settingName: "shortcut_target_power_plus" }
                    ShortcutField { text: settings.shortcut_target_power_minus; settingName: "shortcut_target_power_minus" }
                }
                RowLayout {
                    Label { text: qsTr("Target Zone + / -"); Layout.fillWidth: true }
                    ShortcutField { text: settings.shortcut_target_zone_plus; settingName: "shortcut_target_zone_plus" }
                    ShortcutField { text: settings.shortcut_target_zone_minus; settingName: "shortcut_target_zone_minus" }
                }
                RowLayout {
                    Label { text: qsTr("Target Speed + / -"); Layout.fillWidth: true }
                    ShortcutField { text: settings.shortcut_target_speed_plus; settingName: "shortcut_target_speed_plus" }
                    ShortcutField { text: settings.shortcut_target_speed_minus; settingName: "shortcut_target_speed_minus" }
                }
                RowLayout {
                    Label { text: qsTr("Target Incline + / -"); Layout.fillWidth: true }
                    ShortcutField { text: settings.shortcut_target_incline_plus; settingName: "shortcut_target_incline_plus" }
                    ShortcutField { text: settings.shortcut_target_incline_minus; settingName: "shortcut_target_incline_minus" }
                }
            }
        }

        GroupBox {
            title: qsTr("Peloton & Others")
            Layout.fillWidth: true
            ColumnLayout {
                anchors.fill: parent
                RowLayout {
                    Label { text: qsTr("Peloton Resistance + / -"); Layout.fillWidth: true }
                    ShortcutField { text: settings.shortcut_peloton_resistance_plus; settingName: "shortcut_peloton_resistance_plus" }
                    ShortcutField { text: settings.shortcut_peloton_resistance_minus; settingName: "shortcut_peloton_resistance_minus" }
                }
                RowLayout {
                    Label { text: qsTr("Peloton Offset + / -"); Layout.fillWidth: true }
                    ShortcutField { text: settings.shortcut_peloton_offset_plus; settingName: "shortcut_peloton_offset_plus" }
                    ShortcutField { text: settings.shortcut_peloton_offset_minus; settingName: "shortcut_peloton_offset_minus" }
                }
                RowLayout {
                    Label { text: qsTr("Peloton Remaining + / -"); Layout.fillWidth: true }
                    ShortcutField { text: settings.shortcut_peloton_remaining_plus; settingName: "shortcut_peloton_remaining_plus" }
                    ShortcutField { text: settings.shortcut_peloton_remaining_minus; settingName: "shortcut_peloton_remaining_minus" }
                }
                RowLayout {
                    Label { text: qsTr("Time to Next + / -"); Layout.fillWidth: true }
                    ShortcutField { text: settings.shortcut_remaining_time_plus; settingName: "shortcut_remaining_time_plus" }
                    ShortcutField { text: settings.shortcut_remaining_time_minus; settingName: "shortcut_remaining_time_minus" }
                }
                RowLayout {
                    Label { text: qsTr("Fan Speed + / -"); Layout.fillWidth: true }
                    ShortcutField { text: settings.shortcut_fan_plus; settingName: "shortcut_fan_plus" }
                    ShortcutField { text: settings.shortcut_fan_minus; settingName: "shortcut_fan_minus" }
                }
                RowLayout {
                    Label { text: qsTr("PID Heart Rate + / -"); Layout.fillWidth: true }
                    ShortcutField { text: settings.shortcut_pid_hr_plus; settingName: "shortcut_pid_hr_plus" }
                    ShortcutField { text: settings.shortcut_pid_hr_minus; settingName: "shortcut_pid_hr_minus" }
                }
                RowLayout {
                    Label { text: qsTr("Ext. Inclination + / -"); Layout.fillWidth: true }
                    ShortcutField { text: settings.shortcut_ext_incline_plus; settingName: "shortcut_ext_incline_plus" }
                    ShortcutField { text: settings.shortcut_ext_incline_minus; settingName: "shortcut_ext_incline_minus" }
                }
                RowLayout {
                    Label { text: qsTr("ERG Mode Toggle"); Layout.fillWidth: true }
                    ShortcutField { text: settings.shortcut_erg_mode; settingName: "shortcut_erg_mode" }
                }
                RowLayout {
                    Label { text: qsTr("Power Avg Toggle"); Layout.fillWidth: true }
                    ShortcutField { text: settings.shortcut_power_avg; settingName: "shortcut_power_avg" }
                }
                RowLayout {
                    Label { text: qsTr("Auto-Resistance Toggle"); Layout.fillWidth: true }
                    ShortcutField { text: settings.shortcut_auto_resistance; settingName: "shortcut_auto_resistance" }
                }
                RowLayout {
                    Label { text: qsTr("AVS Cruise / Climb / Sprint"); Layout.fillWidth: true }
                    ShortcutField { text: settings.shortcut_avs_cruise; settingName: "shortcut_avs_cruise" }
                    ShortcutField { text: settings.shortcut_avs_climb; settingName: "shortcut_avs_climb" }
                    ShortcutField { text: settings.shortcut_avs_sprint; settingName: "shortcut_avs_sprint" }
                }
            }
        }

        GroupBox {
            title: qsTr("Preset Resistance")
            Layout.fillWidth: true
            RowLayout {
                anchors.fill: parent
                ShortcutField { text: settings.shortcut_preset_resistance_1; settingName: "shortcut_preset_resistance_1" }
                ShortcutField { text: settings.shortcut_preset_resistance_2; settingName: "shortcut_preset_resistance_2" }
                ShortcutField { text: settings.shortcut_preset_resistance_3; settingName: "shortcut_preset_resistance_3" }
                ShortcutField { text: settings.shortcut_preset_resistance_4; settingName: "shortcut_preset_resistance_4" }
                ShortcutField { text: settings.shortcut_preset_resistance_5; settingName: "shortcut_preset_resistance_5" }
            }
        }

        GroupBox {
            title: qsTr("Preset Speed")
            Layout.fillWidth: true
            RowLayout {
                anchors.fill: parent
                ShortcutField { text: settings.shortcut_preset_speed_1; settingName: "shortcut_preset_speed_1" }
                ShortcutField { text: settings.shortcut_preset_speed_2; settingName: "shortcut_preset_speed_2" }
                ShortcutField { text: settings.shortcut_preset_speed_3; settingName: "shortcut_preset_speed_3" }
                ShortcutField { text: settings.shortcut_preset_speed_4; settingName: "shortcut_preset_speed_4" }
                ShortcutField { text: settings.shortcut_preset_speed_5; settingName: "shortcut_preset_speed_5" }
            }
        }

        GroupBox {
            title: qsTr("Preset Inclination")
            Layout.fillWidth: true
            RowLayout {
                anchors.fill: parent
                ShortcutField { text: settings.shortcut_preset_inclination_1; settingName: "shortcut_preset_inclination_1" }
                ShortcutField { text: settings.shortcut_preset_inclination_2; settingName: "shortcut_preset_inclination_2" }
                ShortcutField { text: settings.shortcut_preset_inclination_3; settingName: "shortcut_preset_inclination_3" }
                ShortcutField { text: settings.shortcut_preset_inclination_4; settingName: "shortcut_preset_inclination_4" }
                ShortcutField { text: settings.shortcut_preset_inclination_5; settingName: "shortcut_preset_inclination_5" }
            }
        }

        GroupBox {
            title: qsTr("Preset Power Zone")
            Layout.fillWidth: true
            RowLayout {
                anchors.fill: parent
                ShortcutField { text: settings.shortcut_preset_powerzone_1; settingName: "shortcut_preset_powerzone_1" }
                ShortcutField { text: settings.shortcut_preset_powerzone_2; settingName: "shortcut_preset_powerzone_2" }
                ShortcutField { text: settings.shortcut_preset_powerzone_3; settingName: "shortcut_preset_powerzone_3" }
                ShortcutField { text: settings.shortcut_preset_powerzone_4; settingName: "shortcut_preset_powerzone_4" }
                ShortcutField { text: settings.shortcut_preset_powerzone_5; settingName: "shortcut_preset_powerzone_5" }
                ShortcutField { text: settings.shortcut_preset_powerzone_6; settingName: "shortcut_preset_powerzone_6" }
                ShortcutField { text: settings.shortcut_preset_powerzone_7; settingName: "shortcut_preset_powerzone_7" }
            }
        }
    }
}
