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
    //anchors.bottom: footerSettings.top
    //anchors.bottomMargin: footerSettings.height + 10
    id: settingsTTLPane

    Settings {
        id: settings
        property bool tts_enabled: false
        property int tts_summary_sec: 120
        property bool tts_act_speed: false
        property bool tts_avg_speed: true
        property bool tts_max_speed: false
        property bool tts_act_inclination: false
        property bool tts_act_cadence: false
        property bool tts_avg_cadence: true
        property bool tts_max_cadence: false
        property bool tts_act_elevation: true
        property bool tts_act_calories: true
        property bool tts_act_odometer: true
        property bool tts_act_pace: false
        property bool tts_avg_pace: true
        property bool tts_max_pace: false
        property bool tts_act_resistance: true
        property bool tts_avg_resistance: true
        property bool tts_max_resistance: false
        property bool tts_act_watt: false
        property bool tts_avg_watt: true
        property bool tts_max_watt: true
        property bool tts_act_ftp: false
        property bool tts_avg_ftp: true
        property bool tts_max_ftp: false
        property bool tts_act_heart: true
        property bool tts_avg_heart: true
        property bool tts_max_heart: false
        property bool tts_act_jouls: true
        property bool tts_act_elapsed: true
        property bool tts_act_peloton_resistance: false
        property bool tts_avg_peloton_resistance: false
        property bool tts_max_peloton_resistance: false
        property bool tts_act_target_peloton_resistance: true
        property bool tts_act_target_cadence: true
        property bool tts_act_target_power: true
        property bool tts_act_target_zone: true
        property bool tts_act_target_speed: true
        property bool tts_act_target_incline: true
        property bool tts_act_watt_kg: false
        property bool tts_avg_watt_kg: false
        property bool tts_max_watt_kg: false
    }


    ColumnLayout {
        id: column1
        spacing: 0
        anchors.fill: parent

        Label {
            Layout.preferredWidth: parent.width
            id: ttsLabel
            text: qsTr("TTS (Text to Speech) Settings")
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            color: Material.color(Material.Red)
        }
        SwitchDelegate {
            id: ttsEnableDelegate
            text: qsTr("TTS Enabled")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_enabled
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_enabled = checked
        }
    }
}
