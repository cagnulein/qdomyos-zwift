import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtGraphicalEffects 1.12

Page {

    title: qsTr("qDomyos-Zwift")

    ListModel {
        id: appModel

        ListElement {
            name: "Speed (km/h)"
            icon: "icons/icons/speed.png"
            value: "0.0"
            writable: true
        }
        ListElement {
            name: "Inclination (%)"
            icon: "icons/icons/inclination.png"
            value: "0.0"
            writable: true
        }
        ListElement {
            name: "Cadence (bpm)"
            icon: "icons/icons/cadence.png"
            value: "0"
            writable: false
        }
        ListElement {
            name: "Elev. Gain (m)"
            icon: "icons/icons/elevationgain.png"
            value: "0"
            writable: false
        }
        ListElement {
            name: "Calories (KCal)"
            icon: "icons/icons/kcal.png"
            value: "0"
            writable: false
        }
        ListElement {
            name: "Odometer (km)"
            icon: "icons/icons/odometer.png"
            value: "0.0"
            writable: false
        }
        ListElement {
            name: "Pace (m/km)"
            icon: "icons/icons/pace.png"
            value: "0:00"
            writable: false
        }
        ListElement {
            name: "Resistance (%)"
            icon: "icons/icons/resistance.png"
            value: "0"
            writable: true
        }
        ListElement {
            name: "Watt"
            icon: "icons/icons/watt.png"
            value: "0"
            writable: false
        }
        ListElement {
            name: "Heart (bpm)"
            icon: "icons/icons/heart_red.png"
            value: "0"
            writable: false
        }
        ListElement {
            name: "Fan Speed"
            icon: "icons/icons/fan.png"
            value: "0"
            writable: true
        }
    }

    Row {
        id: row
        width: parent.width
        height: 100

        Item {
            Image {
                id: treadmill_connection
                x: 5
                width: 96
                height: 96
                source: "icons/icons/bluetooth-icon.png"
                enabled: false
                smooth: true
            }
            ColorOverlay {
                anchors.fill: treadmill_connection
                source: treadmill_connection
                color: treadmill_connection.enabled ? "#00000000" : "#80800000"
            }
        }

        Item {
            Image {
                id: zwift_connection
                x: 100
                width: 96
                height: 96
                source: "icons/icons/zwift-on.png"
                enabled: false
                smooth: true
            }
            ColorOverlay {
                anchors.fill: zwift_connection
                source: zwift_connection
                color: zwift_connection.enabled ? "#00000000" : "#80800000"
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

        //        highlight: Rectangle {
        //            width: 150
        //           height: 150
        //            color: "lightsteelblue"
        //        }
        delegate: Item {
            id: item1
            width: 150
            height: 180

            Rectangle {
                width: 148
                height: 178
                radius: 3
                border.width: 1
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
                id: myValue
                color: Material.accentColor
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


    /*GridView {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.fill: parent
        cellWidth: 150
        cellHeight: 180
        focus: true
        model: appModel
        leftMargin: 5
        topMargin: 5

        Item {
            id: item2
            width: 150
            height: 180

            Rectangle {
                width: 148
                height: 178
                radius: 3
                border.width: 1
            }
            Image {
                id: treadmill_connection
                x: 5
                anchors {
                    top: 5
                }
                width: 96
                height: 96
                source: "icons/icons/bluetooth-icon.png"
                enabled: false
            }
        }

        Item {
            Rectangle {
                width: 148
                height: 178
                radius: 3
                border.width: 1
            }
            Image {
                id: zwift_connection
                x: 5
                anchors {
                    top: 5
                }
                width: 96
                height: 96
                source: "icons/icons/zwift-on.png"
                enabled: false
            }
        }
    }*/
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/

