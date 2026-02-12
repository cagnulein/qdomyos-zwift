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
        property bool tts_act_ftp: true
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
        property bool tts_description_enabled: true
        property bool tts_act_target_pace: false
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
        IndicatorOnlySwitch {
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
        RowLayout {
            spacing: 10
            Label {
                id: labelTTSSummarySec
                text: qsTr("Summary Each Seconds:")
                Layout.fillWidth: true
            }
            TextField {
                id: ttsSummarySecTextField
                text: settings.tts_summary_sec
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.tts_summary_sec = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                id: okTTSSummarySec
                text: "OK"
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: { settings.tts_summary_sec = ttsSummarySecTextField.text; toast.show("Setting saved!"); }
            }
        }
        IndicatorOnlySwitch {
            id: ttsDescriptionEnableDelegate
            text: qsTr("TTS Description Enabled")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_description_enabled
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_description_enabled = checked
        }
        IndicatorOnlySwitch {
            id: ttsActualSpeedDelegate
            text: qsTr("Actual Speed")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_act_speed
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_act_speed = checked
        }
        IndicatorOnlySwitch {
            id: ttsAverageSpeedDelegate
            text: qsTr("Average Speed")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_avg_speed
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_avg_speed = checked
        }
        IndicatorOnlySwitch {
            id: ttsMaxSpeedDelegate
            text: qsTr("Max Speed")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_max_speed
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_max_speed = checked
        }
        IndicatorOnlySwitch {
            id: ttsActualInclinationDelegate
            text: qsTr("Actual Inclination")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_act_inclination
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_act_inclination = checked
        }
        IndicatorOnlySwitch {
            id: ttsActualCadenceDelegate
            text: qsTr("Actual Cadence")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_act_cadence
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_act_cadence = checked
        }
        IndicatorOnlySwitch {
            id: ttsAverageCadenceDelegate
            text: qsTr("Average Cadence")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_avg_cadence
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_avg_cadence = checked
        }
        IndicatorOnlySwitch {
            id: ttsMaxCadenceDelegate
            text: qsTr("Max Cadence")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_max_cadence
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_max_cadence = checked
        }
        IndicatorOnlySwitch {
            id: ttsActualElevationDelegate
            text: qsTr("Actual Elevation")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_act_elevation
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_act_elevation = checked
        }
        IndicatorOnlySwitch {
            id: ttsActualCaloriesDelegate
            text: qsTr("Actual Calories")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_act_calories
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_act_calories = checked
        }
        IndicatorOnlySwitch {
            id: ttsActualOdometerDelegate
            text: qsTr("Actual Odometer")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_act_odometer
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_act_odometer = checked
        }
        IndicatorOnlySwitch {
            id: ttsActualpaceDelegate
            text: qsTr("Actual Pace")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_act_pace
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_act_pace = checked
        }
        IndicatorOnlySwitch {
            id: ttsAvgpaceDelegate
            text: qsTr("Average Pace")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_avg_pace
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_avg_pace = checked
        }
        IndicatorOnlySwitch {
            id: ttsMaxpaceDelegate
            text: qsTr("Max Pace")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_max_pace
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_max_pace = checked
        }
        IndicatorOnlySwitch {
            id: ttsActualresDelegate
            text: qsTr("Actual Resistance")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_act_resistance
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_act_resistance = checked
        }
        IndicatorOnlySwitch {
            id: ttsAvgResistanceDelegate
            text: qsTr("Average Resistance")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_avg_resistance
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_avg_resistance = checked
        }
        IndicatorOnlySwitch {
            id: ttsMaxResistanceDelegate
            text: qsTr("Max Resistance")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_max_resistance
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_max_resistance = checked
        }
        IndicatorOnlySwitch {
            id: ttsActualWattDelegate
            text: qsTr("Actual Watt")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_act_watt
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_act_watt = checked
        }
        IndicatorOnlySwitch {
            id: ttsAvgWattDelegate
            text: qsTr("Average Watt")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_avg_watt
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_avg_watt = checked
        }
        IndicatorOnlySwitch {
            id: ttsMaxWattDelegate
            text: qsTr("Max Watt")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_max_watt
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_max_watt = checked
        }
        IndicatorOnlySwitch {
            id: ttsActualFTPDelegate
            text: qsTr("Actual FTP")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_act_ftp
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_act_ftp = checked
        }/*
        IndicatorOnlySwitch {
            id: ttsAvgFTPDelegate
            text: qsTr("Average FTP")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_avg_ftp
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_avg_ftp = checked
        }
        IndicatorOnlySwitch {
            id: ttsMaxFTPDelegate
            text: qsTr("Max FTP")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_max_ftp
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_max_ftp = checked
        }*/
        IndicatorOnlySwitch {
            id: ttsActualHeartDelegate
            text: qsTr("Actual Heart")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_act_heart
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_act_heart = checked
        }
        IndicatorOnlySwitch {
            id: ttsAvgHeartDelegate
            text: qsTr("Average Heart")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_avg_heart
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_avg_heart = checked
        }
        IndicatorOnlySwitch {
            id: ttsMaxHeartDelegate
            text: qsTr("Max Heart")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_max_heart
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_max_heart = checked
        }
        IndicatorOnlySwitch {
            id: ttsActualJoulsDelegate
            text: qsTr("Actual Jouls")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_act_jouls
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_act_jouls = checked
        }
        IndicatorOnlySwitch {
            id: ttsActualElapsedDelegate
            text: qsTr("Actual Elapsed")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_act_elapsed
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_act_elapsed = checked
        }
        IndicatorOnlySwitch {
            id: ttsActualPelotonResistanceDelegate
            text: qsTr("Actual Peloton Resistance")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_act_peloton_resistance
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_act_peloton_resistance = checked
        }
        IndicatorOnlySwitch {
            id: ttsAvgPelotonResistanceDelegate
            text: qsTr("Average Peloton Resistance")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_avg_peloton_resistance
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_avg_peloton_resistance = checked
        }
        IndicatorOnlySwitch {
            id: ttsMaxPelotonResistanceDelegate
            text: qsTr("Max Peloton Resistance")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_max_peloton_resistance
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_max_peloton_resistance = checked
        }
        IndicatorOnlySwitch {
            id: ttsActualTargetPelotonResistanceDelegate
            text: qsTr("Actual Target Peloton Resistance")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_act_target_peloton_resistance
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_act_target_peloton_resistance = checked
        }
        IndicatorOnlySwitch {
            id: ttsActualTargetCadenceDelegate
            text: qsTr("Actual Target Cadence")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_act_target_cadence
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_act_target_cadence = checked
        }
        IndicatorOnlySwitch {
            id: ttsActualTargetPowerDelegate
            text: qsTr("Actual Target Power")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_act_target_power
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_act_target_power = checked
        }
        IndicatorOnlySwitch {
            id: ttsActualTargetZoneDelegate
            text: qsTr("Actual Target Zone")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_act_target_zone
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_act_target_zone = checked
        }
        IndicatorOnlySwitch {
            id: ttsActualTargetSpeedDelegate
            text: qsTr("Actual Target Speed")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_act_target_speed
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_act_target_speed = checked
        }
        IndicatorOnlySwitch {
            id: ttsActualTargetPaceDelegate
            text: qsTr("Actual Target Pace")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_act_target_pace
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_act_target_pace = checked
        }
        IndicatorOnlySwitch {
            id: ttsActualTargetInclineDelegate
            text: qsTr("Actual Target Incline")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_act_target_incline
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_act_target_incline = checked
        }
        IndicatorOnlySwitch {
            id: ttsActualWattKGDelegate
            text: qsTr("Actual Watt/KG")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_act_watt_kg
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_act_watt_kg = checked
        }
        IndicatorOnlySwitch {
            id: ttsAvgWattKGDelegate
            text: qsTr("Average Watt/KG")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_avg_watt_kg
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_avg_watt_kg = checked
        }
        IndicatorOnlySwitch {
            id: ttsMaxWattKGDelegate
            text: qsTr("Max Watt/KG")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.tts_max_watt_kg
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.tts_max_watt_kg = checked
        }
    }  
}
