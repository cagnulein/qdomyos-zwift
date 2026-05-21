import QtQuick 2.7
import QtQuick.Layouts 1.3

ColumnLayout {
    id: rootElement
    property bool isOpen: false
    property string title: ""
    property alias color: accordionHeader.color
    property alias textColor: accordionText.color
    property alias textFont: accordionText.font.family
    property alias textFontSize: accordionText.font.pixelSize
    property alias indicatRectColor: indicatRect.color
    property string accordionContent: ""
    property color cardBorderColor: Qt.rgba(0.5, 0.5, 0.5, 0.45)
    spacing: 0

    Layout.fillWidth: true;
    Layout.topMargin: 4
    Layout.bottomMargin: 4

    Rectangle {
        id: accordionHeader
        color: "red"
        radius: 6
        border.width: 1
        border.color: rootElement.cardBorderColor
        clip: true
        Layout.alignment: Qt.AlignTop
        Layout.fillWidth: true;
        height: 48

        Accessible.role: Accessible.Button 
        Accessible.name: title 
        Accessible.description: expanded ? "Expanded" : "Collapsed"
        Accessible.onPressAction: toggle()

        Rectangle{
           id:indicatRect
           x: 16; y: 20
           width: 8; height: 8
           radius: 8
           color: "white"
        }

        Text {
            id: accordionText
            x:34;y:13
            color: "#FFFFFF"
            text: rootElement.title
        }
        Text {
            y:13
            anchors.right:  parent.right
            anchors.rightMargin: 20
            width: 30; height: 30
            id: indicatImg
            text: ">"
            font.pixelSize: 24
            color: "white"
        }
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                stackView.push(accordionContent)
            }
        }
    }
}
