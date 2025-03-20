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
    default property alias accordionContent: contentPlaceholder.data
    spacing: 0

    Layout.fillWidth: true;

    Rectangle {
        id: accordionHeader
        color: "red"
        Layout.alignment: Qt.AlignTop
        Layout.fillWidth: true;
        height: 48

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
        Image {
            y:13
            anchors.right:  parent.right
            anchors.rightMargin: 20
            width: 30; height: 30
            id: indicatImg
            source: "qrc:/icons/arrow-collapse-vertical.png"
        }
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                rootElement.isOpen = !rootElement.isOpen
                if(rootElement.isOpen)
                {
                    indicatImg.source = "qrc:/icons/arrow-expand-vertical.png"
                }else{
                    indicatImg.source = "qrc:/icons/arrow-collapse-vertical.png"
                }
            }
        }
    }

    // This will get filled with the content
    ColumnLayout {
        id: contentPlaceholder
        visible: rootElement.isOpen
        Layout.fillWidth: true;
    }
}
