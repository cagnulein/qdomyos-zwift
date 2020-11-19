import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12

Page {

    title: qsTr("qDomyos-Zwift")

    ListModel {
        id: appModel
        ListElement {
            name: "Speed (km/h)"
            icon: "icons/icons/speed.png"
            value: "0.0"
        }
        ListElement {
            name: "Inclination (%)"
            icon: "icons/icons/inclination.png"
            value: "0.0"
        }
        ListElement {
            name: "Cadence"
            icon: "icons/icons/cadence.png"
            value: "0"
        }
        ListElement {
            name: "Elev. Gain (m)"
            icon: "icons/icons/elevationgain.png"
            value: "0"
        }
        ListElement {
            name: "Calories (KCal)"
            icon: "icons/icons/kcal.png"
            value: "0"
        }
        ListElement {
            name: "Odometer (km)"
            icon: "icons/icons/odometer.png"
            value: "0.0"
        }
        ListElement {
            name: "Pace (m/km)"
            icon: "icons/icons/pace.png"
            value: "0:00"
        }
        ListElement {
            name: "Resistance (%)"
            icon: "icons/icons/resistance.png"
            value: "0"
        }
        ListElement {
            name: "Watt"
            icon: "icons/icons/watt.png"
            value: "0"
        }
    }
    GridView {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.fill: parent
		  cellWidth: 150
		  cellHeight: 150
        focus: true
        model: appModel

        highlight: Rectangle {
		      width: 150
				height: 150
            color: "lightsteelblue"
        }
        delegate: Item {
		      width: 150
				height: 150

            Image {
                id: myIcon
                y: 0
                width: 48
                height: 48
                anchors.horizontalCenter: parent.horizontalCenter
                source: icon
            }
            Text {
                id: myValue
                color: Material.accentColor
                anchors {
                    verticalCenter: parent.verticalCenter
                    horizontalCenter: parent.horizontalCenter
                }
                text: value
                horizontalAlignment: Text.AlignHCenter
                font.pointSize: 48
                font.bold: true
            }
            Text {
                anchors {
                    top: myValue.bottom
                    horizontalCenter: parent.horizontalCenter
                }
					 font.bold: true
                text: name
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

