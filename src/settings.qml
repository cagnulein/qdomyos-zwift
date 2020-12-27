import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0

    ScrollView {
        focus: true
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.fill: parent
        id: settingsPane        

        Settings {
            id: settings
            property bool bike_heartrate_service: false
            property int bike_resistance_offset: 4
            property int bike_resistance_gain: 1
            property real weight: 75.0
            property bool miles_unit: false
            property bool bike_cadence_sensor: false

            property bool tile_speed_enabled: true
            property bool tile_inclination_enabled: true
            property bool tile_cadence_enabled: true
            property bool tile_elevation_enabled: true
            property bool tile_calories_enabled: true
            property bool tile_odometer_enabled: true
            property bool tile_pace_enabled: true
            property bool tile_resistance_enabled: true
            property bool tile_watt_enabled: true
            property bool tile_avgwatt_enabled: true
            property bool tile_heart_enabled: true
            property bool tile_fan_enabled: true
            property bool tile_jouls_enabled: true
            property bool tile_elapsed_enabled: true
            property bool tile_peloton_resistance_enabled: true

            property bool bluetooth_relaxed: false
            property bool battery_service: false
        }

        ColumnLayout {
            id: column
            spacing: 0
            anchors.fill: parent

            Label {
                Layout.preferredWidth: parent.width
                id: rebootLabel
                text: qsTr("Reboot the app in order to apply the settings")
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                verticalAlignment: Text.AlignVCenter
                color: Material.color(Material.Red)                
            }

            Label {
                id: generalLabel
                text: qsTr("General Options")
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                verticalAlignment: Text.AlignVCenter
                color: Material.color(Material.Grey)
            }

            RowLayout {
                spacing: 10
                Label {
                    id: labelWeight
                    text: qsTr("Player Weight (kg):")
                    Layout.fillWidth: true
                }
                TextField {
                    id: weightTextField
                    text: settings.weight
                    horizontalAlignment: Text.AlignRight
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    inputMethodHints: Qt.ImhDigitsOnly
                    onAccepted: settings.weight = text
                }
                Button {
                    id: okWeightButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.weight = weightTextField.text
                }
            }

            SwitchDelegate {
                id: unitDelegate
                text: qsTr("Use Miles unit in UI")
                spacing: 0
                bottomPadding: 0
                topPadding: 0
                rightPadding: 0
                leftPadding: 0
                clip: false
                checked: settings.miles_unit
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: true
                onClicked: settings.miles_unit = checked
            }

            Label {
                id: bikeBridgeLabel
                text: qsTr("Bike Bridge Options")
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                verticalAlignment: Text.AlignVCenter
                color: Material.color(Material.Grey)
            }

            SwitchDelegate {
                id: cadenceSensorDelegate
                text: qsTr("Cadence Sensor (Peloton compatibility)")
                spacing: 0
                bottomPadding: 0
                topPadding: 0
                rightPadding: 0
                leftPadding: 0
                clip: false
                checked: settings.bike_cadence_sensor
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: true
                onClicked: settings.bike_cadence_sensor = checked
            }

            SwitchDelegate {
                id: switchDelegate
                text: qsTr("Heart Rate service outside FTMS")
                spacing: 0
                bottomPadding: 0
                topPadding: 0
                rightPadding: 0
                leftPadding: 0
                clip: false
                checked: settings.bike_heartrate_service
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: true
                onClicked: settings.bike_heartrate_service = checked
            }

            RowLayout {
                spacing: 10
                Label {
                    id: labelBikeResistanceOffset
                    text: qsTr("Zwift Resistance Offset:")
                    Layout.fillWidth: true
                }
                TextField {
                    id: bikeResistanceOffsetTextField
                    text: settings.bike_resistance_offset
                    horizontalAlignment: Text.AlignRight
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    inputMethodHints: Qt.ImhDigitsOnly
                    onAccepted: settings.bike_resistance_offset = text
                }
                Button {
                    id: okBikeResistanceOffsetButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.bike_resistance_offset = bikeResistanceOffsetTextField.text
                }
            }

            RowLayout {
                spacing: 10
                Label {
                    id: labelBikeResistanceGain
                    text: qsTr("Zwift Resistance Gain:")
                    Layout.fillWidth: true
                }
                TextField {
                    id: bikeResistanceGainTextField
                    text: settings.bike_resistance_gain
                    horizontalAlignment: Text.AlignRight
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    inputMethodHints: Qt.ImhDigitsOnly
                    onAccepted: settings.bike_resistance_gain = text
                }
                Button {
                    id: okBikeResistanceGainButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.bike_resistance_gain = bikeResistanceGainTextField.text
                }
            }

            Label {
                id: tileOptionsLabel
                text: qsTr("Tiles Options")
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                verticalAlignment: Text.AlignVCenter
                color: Material.color(Material.Grey)
            }

            SwitchDelegate {
                id: speedEnabledDelegate
                text: qsTr("Speed")
                spacing: 0
                bottomPadding: 0
                topPadding: 0
                rightPadding: 0
                leftPadding: 0
                clip: false
                checked: settings.tile_speed_enabled
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: true
                onClicked: settings.tile_speed_enabled = checked
            }

            SwitchDelegate {
                id: inclinationEnabledDelegate
                text: qsTr("Inclination")
                spacing: 0
                bottomPadding: 0
                topPadding: 0
                rightPadding: 0
                leftPadding: 0
                clip: false
                checked: settings.tile_inclination_enabled
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: true
                onClicked: settings.tile_inclination_enabled = checked
            }

            SwitchDelegate {
                id: cadenceEnabledDelegate
                text: qsTr("Cadence")
                spacing: 0
                bottomPadding: 0
                topPadding: 0
                rightPadding: 0
                leftPadding: 0
                clip: false
                checked: settings.tile_cadence_enabled
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: true
                onClicked: settings.tile_cadence_enabled = checked
            }

            SwitchDelegate {
                id: elevationEnabledDelegate
                text: qsTr("Elevation")
                spacing: 0
                bottomPadding: 0
                topPadding: 0
                rightPadding: 0
                leftPadding: 0
                clip: false
                checked: settings.tile_elevation_enabled
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: true
                onClicked: settings.tile_elevation_enabled = checked
            }

            SwitchDelegate {
                id: caloriesEnabledDelegate
                text: qsTr("Calories")
                spacing: 0
                bottomPadding: 0
                topPadding: 0
                rightPadding: 0
                leftPadding: 0
                clip: false
                checked: settings.tile_calories_enabled
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: true
                onClicked: settings.tile_calories_enabled = checked
            }

            SwitchDelegate {
                id: odometerEnabledDelegate
                text: qsTr("Odometer")
                spacing: 0
                bottomPadding: 0
                topPadding: 0
                rightPadding: 0
                leftPadding: 0
                clip: false
                checked: settings.tile_odometer_enabled
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: true
                onClicked: settings.tile_odometer_enabled = checked
            }

            SwitchDelegate {
                id: paceEnabledDelegate
                text: qsTr("Pace")
                spacing: 0
                bottomPadding: 0
                topPadding: 0
                rightPadding: 0
                leftPadding: 0
                clip: false
                checked: settings.tile_pace_enabled
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: true
                onClicked: settings.tile_pace_enabled = checked
            }

            SwitchDelegate {
                id: resistanceEnabledDelegate
                text: qsTr("Resistance")
                spacing: 0
                bottomPadding: 0
                topPadding: 0
                rightPadding: 0
                leftPadding: 0
                clip: false
                checked: settings.tile_resistance_enabled
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: true
                onClicked: settings.tile_resistance_enabled = checked
            }

            SwitchDelegate {
                id: wattEnabledDelegate
                text: qsTr("Watt")
                spacing: 0
                bottomPadding: 0
                topPadding: 0
                rightPadding: 0
                leftPadding: 0
                clip: false
                checked: settings.tile_watt_enabled
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: true
                onClicked: settings.tile_watt_enabled = checked
            }

            SwitchDelegate {
                id: avgwattEnabledDelegate
                text: qsTr("AVG Watt")
                spacing: 0
                bottomPadding: 0
                topPadding: 0
                rightPadding: 0
                leftPadding: 0
                clip: false
                checked: settings.tile_avgwatt_enabled
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: true
                onClicked: settings.tile_avgwatt_enabled = checked
            }

            SwitchDelegate {
                id: heartEnabledDelegate
                text: qsTr("Heart")
                spacing: 0
                bottomPadding: 0
                topPadding: 0
                rightPadding: 0
                leftPadding: 0
                clip: false
                checked: settings.tile_heart_enabled
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: true
                onClicked: settings.tile_heart_enabled = checked
            }

            SwitchDelegate {
                id: fanEnabledDelegate
                text: qsTr("Fan")
                spacing: 0
                bottomPadding: 0
                topPadding: 0
                rightPadding: 0
                leftPadding: 0
                clip: false
                checked: settings.tile_fan_enabled
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: true
                onClicked: settings.tile_fan_enabled = checked
            }

            SwitchDelegate {
                id: joulsEnabledDelegate
                text: qsTr("Jouls")
                spacing: 0
                bottomPadding: 0
                topPadding: 0
                rightPadding: 0
                leftPadding: 0
                clip: false
                checked: settings.tile_jouls_enabled
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: true
                onClicked: settings.tile_jouls_enabled = checked
            }

            SwitchDelegate {
                id: elapsedEnabledDelegate
                text: qsTr("Elapsed")
                spacing: 0
                bottomPadding: 0
                topPadding: 0
                rightPadding: 0
                leftPadding: 0
                clip: false
                checked: settings.tile_elapsed_enabled
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: true
                onClicked: settings.tile_elapsed_enabled = checked
            }

            SwitchDelegate {
                id: pelotonResistanceEnabledDelegate
                text: qsTr("Peloton Resistance")
                spacing: 0
                bottomPadding: 0
                topPadding: 0
                rightPadding: 0
                leftPadding: 0
                clip: false
                checked: settings.tile_peloton_resistance_enabled
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: true
                onClicked: settings.tile_peloton_resistance_enabled = checked
            }

            Label {
                id: experimentalFeatureLabel
                text: qsTr("Experimental Features")
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                verticalAlignment: Text.AlignVCenter
                color: Material.color(Material.Grey)
            }

            SwitchDelegate {
                id: bluetoothRelaxedDelegate
                text: qsTr("Relaxed Bluetooth for mad devices")
                spacing: 0
                bottomPadding: 0
                topPadding: 0
                rightPadding: 0
                leftPadding: 0
                clip: false
                checked: settings.bluetooth_relaxed
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: true
                onClicked: settings.bluetooth_relaxed = checked
            }

            SwitchDelegate {
                id: batteryServiceDelegate
                text: qsTr("Simulate Battery Service")
                spacing: 0
                bottomPadding: 0
                topPadding: 0
                rightPadding: 0
                leftPadding: 0
                clip: false
                checked: settings.battery_service
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: true
                onClicked: settings.battery_service = checked
            }
        }
    }

/*##^##
Designer {
    D{i:0;formeditorZoom:0.6600000262260437}
}
##^##*/
