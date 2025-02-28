import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
//import QtGraphicalEffects 1.15
import Qt.labs.settings 1.0

Page {

    title: qsTr("QZ Fitness")
    id: page

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
                Column {
                    id: column
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width
                    height: row.height
                    spacing: 0
                    padding: 0
                    Rectangle {
                        width: 50
                        height: row.height
                        color: settings.theme_background_color

                        Image {
                            anchors.verticalCenter: parent.verticalCenter
                            id: treadmill_connection
                            width: 48
                            height: row.height - 52
                            source: "icons/icons/bluetooth-icon.png"
                            enabled: rootItem.device
                            smooth: true
                        }/*
                        ColorOverlay {
                            anchors.fill: treadmill_connection
                            source: treadmill_connection
                            color: treadmill_connection.enabled ? "#00000000" : "#B0D3d3d3"
                        }*/
                    }
                    Image {
                        anchors.horizontalCenter: parent.horizontalCenter
                        id: treadmill_signal
                        width: 24
                        height: row.height - 76
                        source: rootItem.signal
                        smooth: true
                    }
                }
            }

            Rectangle {
                width: 120
                height: row.height
					 color: settings.theme_background_color
                RoundButton {
                    icon.source: rootItem.startIcon
                    icon.height: row.height - 54
                    icon.width: 46
                    text: rootItem.startText
                    enabled: true
                    id: start
                    width: 120
                    height: row.height - 4
                }/*
                ColorOverlay {
                    anchors.fill: start
                    source: start
                    color: rootItem.startColor
                    enabled: rootItem.startColor === "red" ? true : false
                }*/
            }

            Rectangle {
                width: 120
                height: row.height
					 color: settings.theme_background_color

                RoundButton {
                    icon.source: rootItem.stopIcon
                    icon.height: row.height - 54
                    icon.width: 46
                    text: rootItem.stopText
                    enabled: true
                    id: stop
                    width: 120
                    height: row.height - 4
                }/*
                ColorOverlay {
                    anchors.fill: stop
                    source: stop
                    color: rootItem.stopColor
                    enabled: rootItem.stopColor === "red" ? true : false
                }*/
            }

            Rectangle {
                id: item2
                width: 50
                height: row.height
					 color: settings.theme_background_color
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
                }/*
                ColorOverlay {
                    anchors.fill: lap
                    source: lap
                    color: lap.enabled ? "#00000000" : "#B0D3d3d3"
                }*/
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
            text: "This app should automatically connect to your bike/treadmill/rower. <b>If it doesn't, please check</b>:<br>1) your Echelon/Domyos App MUST be closed while qdomyos-zwift is running;<br>2) bluetooth and bluetooth permission MUST be on<br>3) your bike/treadmill/rower should be turned on BEFORE starting this app<br>4) try to restart your device<br><br>If your bike/treadmill disconnects every 30 seconds try to disable the 'virtual device' setting on the left bar.<br><br>In case of issues, please feel free to contact me at roberto.viola83@gmail.com.<br><br><b>Have a nice ride!</b><br/ ><i>QZ specifically disclaims liability for<br>incidental or consequential damages and assumes<br>no responsibility or liability for any loss<br>or damage suffered by any person as a result of<br>the use or misuse of the app.</i><br><br>Roberto Viola"
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

