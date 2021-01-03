import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtGraphicalEffects 1.12

Page {

    title: qsTr("qDomyos-Zwift")
    id: page

    property alias start: start
    property alias stop: stop
    property alias row: row

    Item {
        width: parent.width
        height: rootItem.topBarHeight
        id: topBar

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
                color: Material.backgroundColor
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
                        color: Material.backgroundColor

                        Image {
                            anchors.verticalCenter: parent.verticalCenter
                            id: treadmill_connection
                            width: 48
                            height: row.height - 52
                            source: "icons/icons/bluetooth-icon.png"
                            enabled: rootItem.device
                            smooth: true
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
                    }
                }
            }

            Rectangle {
                width: 120
                height: row.height
                color: Material.backgroundColor
                RoundButton {
                    icon.source: "icons/icons/start.png"
                    icon.height: row.height - 54
                    icon.width: 46
                    text: "Start"
                    enabled: true
                    id: start
                    width: 120
                    height: row.height - 4
                }
            }

            Rectangle {
                width: 120
                height: row.height
                color: Material.backgroundColor
                RoundButton {
                    icon.source: "icons/icons/stop.png"
                    icon.height: row.height - 54
                    icon.width: 46
                    text: "Stop"
                    enabled: true
                    id: stop
                    width: 120
                    height: row.height - 4
                }
            }

            Rectangle {
                id: item2
                width: 50
                height: row.height
                color: Material.backgroundColor
                Image {
                    anchors.verticalCenter: parent.verticalCenter
                    id: zwift_connection
                    width: 48
                    height: row.height - 52
                    source: "icons/icons/zwift-on.png"
                    enabled: rootItem.zwift
                    smooth: true
                }
                ColorOverlay {
                    anchors.fill: zwift_connection
                    source: zwift_connection
                    color: zwift_connection.enabled ? "#00000000" : "#B0D3d3d3"
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
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;formeditorZoom:0.6600000262260437;height:480;width:640}
}
##^##*/

