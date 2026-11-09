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
    // Inside another section: the modern look draws it as a flat row instead of a card
    readonly property bool nested: {
        for (var p = parent; p; p = p.parent)
            if (p.isOpen !== undefined && p.title !== undefined && p.accordionContent !== undefined)
                return true
        return false
    }

    // The Settings object of the settings page, given to the page this element opens: the
    // pages share one object, so none of them writes back an old copy of what another changed
    property var pageSettings: null
    spacing: 0

    Layout.fillWidth: true;

    // Modern look: the header, made only while the look is on (as in AccordionElement)
    Loader {
        active: window.ui.modern
        visible: active
        Layout.fillWidth: true
        Layout.leftMargin: item ? item.Layout.leftMargin : 0
        Layout.rightMargin: item ? item.Layout.rightMargin : 0
        Layout.topMargin: item ? item.Layout.topMargin : 0
        sourceComponent: UiSectionHeader {
            title: rootElement.title
            isOpen: rootElement.isOpen
            nested: rootElement.nested
            chevron: "chevron_right"
            onClicked: stackView.push(rootElement.accordionContent, { settings: rootElement.pageSettings })
        }
    }

    Rectangle {
        id: accordionHeader
        visible: !window.ui.modern
        color: "red"
        Layout.alignment: Qt.AlignTop
        Layout.fillWidth: true;
        height: window.textScale !== 1 ? Math.max(48, accordionText.y + accordionText.height + 13) : 48
        Layout.preferredHeight: height

        Accessible.role: Accessible.Button 
        Accessible.name: title 
        // The section opens a page, there is nothing to expand: no state, and the press action of a
        // screen reader opens the page like a tap on the header
        Accessible.onPressAction: stackView.push(accordionContent, { settings: pageSettings })

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
        Text {
            // The arrow grew down from the top of its box and sat below the title: it is centred
            // on the first line of the title instead, at any text size
            y: accordionText.y + (accordionText.height / Math.max(1, accordionText.lineCount) - height) / 2
            anchors.right:  parent.right
            anchors.rightMargin: 20
            width: 30; height: implicitHeight
            id: indicatImg
            text: ">"
            font.pixelSize: window.fontPx(24)
            color: "white"
        }
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                stackView.push(accordionContent, { settings: pageSettings })
            }
        }
    }
}
