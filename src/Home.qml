import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtGraphicalEffects 1.12

HomeForm{
    objectName: "home"
    signal start_clicked;
    signal stop_clicked;
    signal plus_clicked(string name)
    signal minus_clicked(string name)

    start.onClicked: { start_clicked(); }
    stop.onClicked: { stop_clicked(); }

    Component.onCompleted: { console.log("completed");  }

    GridView {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.fill: parent
        cellWidth: 175
        cellHeight: 125
        focus: true
        model: appModel
        leftMargin: { (parent.width % 175) / 2;  }
        anchors.topMargin: 150
        id: gridView
        objectName: "gridview"

        //        highlight: Rectangle {
        //            width: 150
        //           height: 150
        //            color: "lightsteelblue"
        //        }
        delegate: Item {
            id: id1
            width: 175
            height: 125

            Component.onCompleted: console.log("completed " + objectName)

            Rectangle {
                width: 173
                height: 123
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
                objectName: minusName
                text: "-"
                onClicked: minus_clicked(objectName)
                visible: writable
                anchors.top: myValue.top
                anchors.left: parent.left
                anchors.leftMargin: 2
                width: 48
                height: 48
            }
            RoundButton {
                objectName: plusName
                text: "+"
                onClicked: plus_clicked(objectName)
                visible: writable
                anchors.top: myValue.top
                anchors.right: parent.right
                anchors.rightMargin: 2
                width: 48
                height: 48
            }

            /*MouseArea {
                anchors.fill: parent
                onClicked: parent.GridView.view.currentIndex = index
            }*/
        }
    }
}
