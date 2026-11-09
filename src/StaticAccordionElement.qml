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
    // How deep the section is: 1 - a top-level one, 2 - inside it, and so on (as AccordionElement)
    readonly property int depth: {
        var d = 1
        for (var p = parent; p; p = p.parent)
            if (p.isOpen !== undefined && p.title !== undefined && p.accordionContent !== undefined)
                d++
        return d
    }
    // Inside another section: the modern look draws it as a flat row instead of a card
    readonly property bool nested: depth > 1

    spacing: 0

    Layout.fillWidth: true;

    // Modern look: a short copy of the header at the top of the page while the open section
    // scrolls under it (as AccordionElement)
    UiStickyHeader {
        section: rootElement
        header: sectionHeader
    }

    UiSectionHeader {
        id: sectionHeader
        visible: window.ui.modern
        title: rootElement.title
        isOpen: rootElement.isOpen
        nested: rootElement.nested
        depth: rootElement.depth
        chevron: "expand_more"
        onClicked: rootElement.isOpen = !rootElement.isOpen
    }

    Rectangle {
        id: accordionHeader
        visible: !window.ui.modern
        color: "red"
        Layout.alignment: Qt.AlignTop
        Layout.fillWidth: true;
        height: window.textScale !== 1 ? Math.max(48, accordionText.y + accordionText.height + 13) : 48
        Layout.preferredHeight: height

        Rectangle{
           id:indicatRect
           // Centred on the first line of the title: a fixed y sat below it with a smaller or bigger font
           x: 16; y: accordionText.y + (accordionText.height / Math.max(1, accordionText.lineCount) - height) / 2
           width: 8; height: 8
           radius: 8
           color: "white"
        }

        Text {
            id: accordionText
            x:34;y:13
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
            anchors.right:  parent.right
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

    // This will get filled with the content. Held in a plain item: the frames of the modern look
    // lie beside the content there, a column would lay them out as rows
    Item {
        id: contentBox
        visible: rootElement.isOpen
        Layout.fillWidth: true
        // Set on a change, not bound: a binding made a loop with the layout of the column
        implicitHeight: 0
        Connections {
            target: contentPlaceholder
            function onImplicitHeightChanged() { contentBox.implicitHeight = contentPlaceholder.implicitHeight }
        }
        Component.onCompleted: implicitHeight = contentPlaceholder.implicitHeight
        // Modern look: inside the block of the section and inside the frames of its settings
        Layout.leftMargin: settingFrames.insetLeft
        Layout.rightMargin: settingFrames.insetRight
        Layout.topMargin: settingFrames.insetTop
        Layout.bottomMargin: settingFrames.insetBottom
        // Under the header: the block frame starts behind it
        z: -1

        UiSettingFrames {
            id: settingFrames
            z: -1
            content: contentPlaceholder.children.length > 0 ? contentPlaceholder.children[0] : null
            header: sectionHeader
            depth: rootElement.depth
        }

        ColumnLayout {
            id: contentPlaceholder
            width: parent.width
        }
    }
}
