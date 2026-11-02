import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtGraphicalEffects 1.12
import Qt.labs.settings 1.0
import QtQml 2.15

Page {

    title: qsTr("QZ Fitness")
    id: page

    // VoiceOver accessibility - ignore Page itself, only children are accessible
    Accessible.ignored: true

    property alias start: start
    property alias stop: stop
    property alias lap: lap
    property alias row: row
    // Search for the equipment ("Next search in 12 s"), set by Home.qml while no device is connected
    property string searchStatus: ""
    // Home.qml sets the new values from the settings page in it (the background color of the top bar)
    property alias homeFormSettings: settings

    Settings {
	     id: settings
	     Component.onCompleted: window.retakeSettingsSnapshot(settings)
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

                            // VoiceOver accessibility – ignored while the side menu is open (see Home.qml)
                            Accessible.role: Accessible.Indicator
                            Accessible.name: qsTr("Bluetooth connection")
                            Accessible.description: rootItem.device ? qsTr("Device connected") : qsTr("Device not connected")
                            Accessible.focusable: treadmill_connection.enabled
                            Accessible.ignored: !page.enabled || drawer.opened
                        }
                        ColorOverlay {
                            anchors.fill: treadmill_connection
                            source: treadmill_connection
                            color: treadmill_connection.enabled ? "#00000000" : "#B0D3d3d3"
                        }
                        // No device yet: a tap searches right away instead of waiting for the next search
                        // (with a device connected bluetoothSearchNow() does nothing)
                        MouseArea {
                            anchors.fill: parent
                            onClicked: rootItem.bluetoothSearchNow()
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
                    // fixed width: keep the size it had before the Text size setting
                    Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.appFontPx; restoreMode: Binding.RestoreBindingOrValue }
                    enabled: true
                    id: start
                    width: 120
                    height: row.height - 4

                    // VoiceOver accessibility — ignored when parent is disabled (drawer open)
                    Accessible.role: Accessible.Button
                    Accessible.name: rootItem.startText
                    Accessible.description: qsTr("Start workout")
                    Accessible.focusable: page.enabled && !drawer.opened
                    Accessible.ignored: !page.enabled || drawer.opened
                    Accessible.onPressAction: start.clicked()
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
                    Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.appFontPx; restoreMode: Binding.RestoreBindingOrValue }
                    enabled: true
                    id: stop
                    width: 120
                    height: row.height - 4

                    // VoiceOver accessibility — ignored when parent is disabled (drawer open)
                    Accessible.role: Accessible.Button
                    Accessible.name: rootItem.stopText
                    Accessible.description: qsTr("Stop workout")
                    Accessible.focusable: page.enabled && !drawer.opened
                    Accessible.ignored: !page.enabled || drawer.opened
                    Accessible.onPressAction: stop.clicked()
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
                    Accessible.focusable: page.enabled && !drawer.opened && rootItem.lap
                    Accessible.ignored: !page.enabled || drawer.opened
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
            // a bigger text grows down into the gap above the tiles, not up into the buttons
            anchors.bottomMargin: window.textScale !== 1 ? -10 - Math.round(statusLabel.height * (1 - 1 / window.textScale)) : -10

            Label {
                id: statusLabel
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                // the search status may have two lines ("<device> found" and the countdown): the line
                // grows upwards, so they are set closer to keep the first one off the buttons
                horizontalAlignment: Text.AlignHCenter
                lineHeight: text.indexOf("\n") >= 0 ? 0.8 : 1
                text: page.searchStatus !== "" ? page.searchStatus : rootItem.info
            }
        }

        Label {
            id: lblHelp
            width: parent.width
            leftPadding: window.contentSideMargin
            rightPadding: window.contentSideMargin
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

