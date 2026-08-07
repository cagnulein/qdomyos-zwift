import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtGraphicalEffects 1.12
import Qt.labs.settings 1.0

Page {

    title: qsTr("QZ Fitness")
    id: page

    // VoiceOver accessibility - ignore Page itself, only children are accessible
    Accessible.ignored: true

    property alias start: start
    property alias stop: stop
    property alias lap: lap
    property alias row: row

    Settings {
	     id: settings
		  property real ui_zoom: 100.0
		  property bool theme_tile_icon_enabled: true
		  property string theme_background_color: "#303030"
		}

    Item {
        width: parent.width
        height: rootItem.topBarHeight
        id: topBar
        visible: !window.lockTiles

        Row {
            id: row
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            height: topBar.height - 20
            spacing: 5
            padding: 5

            Rectangle {
                width: 50
                height: row.height
					 color: settings.theme_background_color
                Accessible.ignored: true

                Column {
                    id: column
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width
                    height: row.height
                    spacing: 0
                    padding: 0
                    Accessible.ignored: true

                    Rectangle {
                        width: 50
                        height: row.height
                        color: settings.theme_background_color
                        Accessible.ignored: true

                        Image {
                            anchors.verticalCenter: parent.verticalCenter
                            id: treadmill_connection
                            width: 48
                            height: row.height - 52
                            source: "icons/icons/bluetooth-icon.png"
                            enabled: rootItem.device
                            smooth: true

                            // VoiceOver accessibility — ignored when parent is disabled (drawer open)
                            Accessible.role: Accessible.Indicator
                            Accessible.name: qsTr("Bluetooth connection")
                            Accessible.description: rootItem.device ? qsTr("Device connected") : qsTr("Device not connected")
                            Accessible.focusable: treadmill_connection.enabled
                            Accessible.ignored: !page.enabled
                        }
                        ColorOverlay {
                            anchors.fill: treadmill_connection
                            source: treadmill_connection
                            color: treadmill_connection.enabled ? "#00000000" : "#B0D3d3d3"
                        }
                    }
                    Image {
                        anchors.horizontalCenter: parent.horizontalCenter
                        id: treadmill_signal
                        width: 24
                        height: row.height - 76
                        source: rootItem.signal
                        smooth: true
                        Accessible.ignored: true
                    }
                }
            }

            Rectangle {
                width: 120
                height: row.height
					 color: settings.theme_background_color
                Accessible.ignored: true

                RoundButton {
                    icon.source: rootItem.startIcon
                    icon.height: row.height - 54
                    icon.width: 46
                    text: rootItem.startText
                    enabled: true
                    id: start
                    width: 120
                    height: row.height - 4

                    // VoiceOver accessibility — ignored when parent is disabled (drawer open)
                    Accessible.role: Accessible.Button
                    Accessible.name: rootItem.startText
                    Accessible.description: qsTr("Start workout")
                    Accessible.focusable: page.enabled
                    Accessible.ignored: !page.enabled
                }
                ColorOverlay {
                    anchors.fill: start
                    source: start
                    color: rootItem.startColor
                    enabled: rootItem.startColor === "red" ? true : false
                }
            }

            Rectangle {
                width: 120
                height: row.height
					 color: settings.theme_background_color
                Accessible.ignored: true

                RoundButton {
                    icon.source: rootItem.stopIcon
                    icon.height: row.height - 54
                    icon.width: 46
                    text: rootItem.stopText
                    enabled: true
                    id: stop
                    width: 120
                    height: row.height - 4

                    // VoiceOver accessibility — ignored when parent is disabled (drawer open)
                    Accessible.role: Accessible.Button
                    Accessible.name: rootItem.stopText
                    Accessible.description: qsTr("Stop workout")
                    Accessible.focusable: page.enabled
                    Accessible.ignored: !page.enabled
                }
                ColorOverlay {
                    anchors.fill: stop
                    source: stop
                    color: rootItem.stopColor
                    enabled: rootItem.stopColor === "red" ? true : false
                }
            }

            Rectangle {
                id: item2
                width: 50
                height: row.height
					 color: settings.theme_background_color
                Accessible.ignored: true

                RoundButton {
                    anchors.verticalCenter: parent.verticalCenter
                    id: lap
                    width: 48
                    height: row.height - 52
                    icon.source: "icons/icons/lap.png"
                    icon.width: 48
                    icon.height: 48
                    enabled: rootItem.lap
                    smooth: true

                    // VoiceOver accessibility — ignored when parent is disabled (drawer open)
                    Accessible.role: Accessible.Button
                    Accessible.name: qsTr("Lap")
                    Accessible.description: qsTr("Record a new lap")
                    Accessible.focusable: page.enabled && rootItem.lap
                    Accessible.ignored: !page.enabled
                }
                ColorOverlay {
                    anchors.fill: lap
                    source: lap
                    color: lap.enabled ? "#00000000" : "#B0D3d3d3"
                }
            }
        }

        Row {
            id: row1
            width: parent.width
            anchors.bottom: row.bottom
            anchors.bottomMargin: -10

            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                text: rootItem.info
            }
        }

        Label {
            id: lblHelp
            width: parent.width
            anchors.top: row1.bottom
            anchors.topMargin: 30
            text: qsTr("This app should automatically connect to your bike, treadmill, or rower.\n\nIf it doesn't, check that:\n- the Echelon/Domyos app is closed while QZ is running\n- Bluetooth and Bluetooth permissions are enabled\n- your equipment is turned on before starting QZ\n- you have restarted your device\n\nIf your bike or treadmill disconnects every 30 seconds, disable the Virtual Device setting in the left bar.\n\nFor help, contact roberto.viola83@gmail.com.\n\nHave a nice ride!\n\nQZ disclaims liability for incidental or consequential damages and assumes no responsibility for any loss or damage caused by use or misuse of the app.\n\nRoberto Viola")
            wrapMode: Label.WordWrap
            visible: rootItem.labelHelp
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;formeditorZoom:0.6600000262260437;height:480;width:640}
}
##^##*/

