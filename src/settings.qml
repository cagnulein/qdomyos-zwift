import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0

Pane {
    id: settingsPane
    width: parent.width
    height: parent.height

    Settings {
        id: settings
        property bool bike_heartrate_service: false
        property int bike_resistance_offset: 4
        property int bike_resistance_gain: 1
    }

    ColumnLayout {
        id: column
        spacing: 0
        anchors.fill: parent

        Label {
            id: rebootLabel
            text: qsTr("Reboot the app in order to apply the settings")
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            color: Material.color(Material.Red)
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
            id: switchDelegate
            text: qsTr("Heart Rate service outside FTMS")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            anchors.left: parent.left
            anchors.right: parent.right
            clip: false
            checked: settings.autoConnect
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.bike_heartrate_service = checked
        }

        RowLayout {
            spacing: 10
            Label {
                id: labelBikeResistanceOffset
                text: qsTr("Zwift Resistance Offset:")
                Layout.preferredWidth: 150
                Layout.fillWidth: true
            }
            TextField {
                Layout.preferredWidth: 60
                id: bikeResistanceOffsetTextField
                text: settings.bike_resistance_offset
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhDigitsOnly
                onAccepted: settings.bike_resistance_offset = text
            }
        }

        RowLayout {
            spacing: 10
            Label {
                id: labelBikeResistanceGain
                text: qsTr("Zwift Resistance Gain:")
                Layout.preferredWidth: 150
                Layout.fillWidth: true
            }
            TextField {
                Layout.preferredWidth: 60
                id: bikeResistanceGainTextField
                text: settings.bike_resistance_gain
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhDigitsOnly
                onAccepted: settings.bike_resistance_gain = text
            }
        }
    }
}
