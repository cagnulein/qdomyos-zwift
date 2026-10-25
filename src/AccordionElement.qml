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
    default property alias accordionContent: contentLoader.sourceComponent

    // Signal emitted when content becomes visible
    signal contentBecameVisible()

    spacing: 0
    Layout.fillWidth: true

    Rectangle {
        id: accordionHeader
        color: "red"
        Layout.alignment: Qt.AlignTop
        Layout.fillWidth: true
        height: window.textScale !== 1 ? Math.max(48, accordionText.y + accordionText.height + 13) : 48
        Layout.preferredHeight: height

        Accessible.role: Accessible.Button         
        Accessible.name: title 
        // The state follows isOpen; the press action of a screen reader opens or closes the section
        // like a tap on the header
        Accessible.description: rootElement.isOpen ? "Expanded" : "Collapsed"
        Accessible.onPressAction: rootElement.isOpen = !rootElement.isOpen

        Rectangle {
            id: indicatRect
            // Centred on the first line of the title: a fixed y sat below it with a smaller or bigger font
            x: 16; y: accordionText.y + (accordionText.height / Math.max(1, accordionText.lineCount) - height) / 2
            width: 8; height: 8
            radius: 8
            color: "white"
        }

        Text {
            id: accordionText
            x: 34; y: 13
            color: "#FFFFFF"
            font: window.headerFont
            text: rootElement.title
            // A bigger text can run under the arrow: a long title wraps, and the header grows
            width: window.textScale !== 1 ? indicatImg.x - x - 8 : implicitWidth
            wrapMode: window.labelWrap
        }

        Image {
            // Centred on the first line of the title, like the dot
            y: accordionText.y + (accordionText.height / Math.max(1, accordionText.lineCount) - height) / 2
            anchors.right: parent.right
            anchors.rightMargin: 20
            width: 30; height: 30
            id: indicatImg
            // Bound to isOpen: a section opened from code (a search result) shows the right arrow
            source: rootElement.isOpen ? "qrc:/icons/arrow-expand-vertical.png" : "qrc:/icons/arrow-collapse-vertical.png"
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: rootElement.isOpen = !rootElement.isOpen
        }
    }

    // Loader with enhanced visibility handling
    Loader {
        id: contentLoader
        active: rootElement.isOpen
        visible: false // Start invisible
        Layout.fillWidth: true
        asynchronous: false

        onLoaded: {
            if (item) {
                item.Layout.fillWidth = true
                visible = true
                rootElement.contentBecameVisible()
            }
        }

        // Handle visibility changes
        onVisibleChanged: {
            if (visible && status === Loader.Ready) {
                rootElement.contentBecameVisible()
            }
        }
    }

    // Handle accordion closing
    onIsOpenChanged: {
        if (!isOpen) {
            contentLoader.visible = false
        }
    }
}
