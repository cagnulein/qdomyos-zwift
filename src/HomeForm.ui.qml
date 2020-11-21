import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtGraphicalEffects 1.12

Page {

    title: qsTr("qDomyos-Zwift")
    id: page

    property alias start: start
    property alias stop: stop

    Item {
        width: parent.width
        height: 120

        Row {
            id: row
            anchors.horizontalCenter: parent.horizontalCenter
            height: 100
            spacing: 5
            padding: 5

            Rectangle {
                width: 100
                height: 100
                color: Material.backgroundColor
                Image {
                    id: treadmill_connection
                    width: 96
                    height: 96
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

            Rectangle {
                width: 120
                height: 100
                color: Material.backgroundColor
                RoundButton {
                    icon.source: "icons/icons/start.png"
                    icon.height: 46
                    icon.width: 46
                    text: "Start"
                    enabled: true
                    id: start
                    width: 120
                    height: 96
                }
            }

            Rectangle {
                width: 120
                height: 100
                color: Material.backgroundColor
                RoundButton {
                    icon.source: "icons/icons/stop.png"
                    icon.height: 46
                    icon.width: 46
                    text: "Stop"
                    enabled: true
                    id: stop
                    width: 120
                    height: 96
                }
            }

            Rectangle {
                id: item2
                width: 100
                height: 100
                color: Material.backgroundColor
                Image {
                    id: zwift_connection
                    width: 96
                    height: 96
                    source: "icons/icons/zwift-on.png"
                    enabled: rootItem.zwift
                    smooth: true
                }
                ColorOverlay {
                    anchors.fill: zwift_connection
                    source: zwift_connection
                    color:  zwift_connection.enabled ? "#00000000" : "#B0D3d3d3"
                }
            }
        }
    }

    GridView {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.fill: parent
        cellWidth: 150
        cellHeight: 180
        focus: true
        model: appModel
        leftMargin: 5
        topMargin: row.height
        id: gridView
        objectName: "gridview"

        //        highlight: Rectangle {
        //            width: 150
        //           height: 150
        //            color: "lightsteelblue"
        //        }
        delegate: Item {
            id: id1
            width: 150
            height: 180

            Rectangle {
                width: 148
                height: 178
                radius: 3
                border.width: 1
                color: Material.backgroundColor
            }

            Image {
                id: myIcon
                x: 5
                anchors {
                    top: myValue.bottom
                }
                width: 48
                height: 48
                source: icon
            }
            Text {
                objectName: "value"
                id: myValue
                color: Material.textSelectionColor
                y: 0
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }
                text: value
                horizontalAlignment: Text.AlignHCenter
                font.pointSize: 48
                font.bold: true
            }
            Text {
                id: myText
                anchors {
                    top: myIcon.top
                }
                font.bold: true
                color: "white"
                text: name
                anchors.left: parent.left
                anchors.leftMargin: 55
                anchors.topMargin: 20
            }
            RoundButton {
                id: minus
                text: "-"
                visible: writable
                anchors.top: myIcon.bottom
                anchors.left: parent.left
                anchors.leftMargin: 10
                width: 48
                height: 48
            }
            RoundButton {
                id: plus
                text: "+"
                visible: writable
                anchors.top: myIcon.bottom
                anchors.right: parent.right
                anchors.rightMargin: 10
                width: 48
                height: 48
            }

            MouseArea {
                anchors.fill: parent
                onClicked: parent.GridView.view.currentIndex = index
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/

