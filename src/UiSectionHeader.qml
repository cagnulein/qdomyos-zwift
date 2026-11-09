import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3

// Header of a settings section in the modern look: a rounded card for a top-level section,
// a flat row inside an open one, a chevron that turns when the section opens
Rectangle {
    id: header
    property string title: ""
    property bool isOpen: false
    property bool nested: false
    // 1 - a top-level section, 2 and deeper - a subsection (a filled card)
    property int depth: nested ? 2 : 1
    // "expand_more" turns over when the section opens; "chevron_right" opens a page
    property string chevron: "expand_more"
    signal clicked()

    Layout.fillWidth: true
    Layout.leftMargin: nested ? 0 : 8
    Layout.rightMargin: nested ? 0 : 8
    Layout.topMargin: nested ? 2 : 6
    implicitHeight: Math.max(nested ? 48 : 56, headerTitle.implicitHeight + 24)
    radius: nested ? 12 : 16
    color: headerArea.pressed ? window.ui.surfaceHighest
         : isOpen ? window.ui.surfaceHigh
         : depth >= 2 ? window.ui.surfaceHigh
         : nested ? "transparent" : window.ui.surface

    Accessible.role: Accessible.Button
    Accessible.name: title
    Accessible.description: chevron === "expand_more" ? (isOpen ? qsTr("Expanded") : qsTr("Collapsed")) : ""
    Accessible.onPressAction: header.clicked()

    Rectangle {
        visible: header.isOpen && !header.nested
        // On the outline of the section block (1 px outside the header): no gap between them
        x: -1
        width: 4
        height: parent.height - 24
        anchors.verticalCenter: parent.verticalCenter
        radius: 2
        color: window.ui.accent
    }

    Label {
        id: headerTitle
        anchors.left: parent.left
        anchors.leftMargin: header.nested ? 16 : 18
        anchors.right: headerChevron.left
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        text: window.ui.plainTitle(header.title)
        wrapMode: Text.WordWrap
        font.pixelSize: header.nested ? window.ui.fontLabel : window.ui.fontBody
        font.weight: Font.Medium
        color: header.isOpen ? window.ui.accent : window.ui.textMain
    }

    UiIcon {
        id: headerChevron
        anchors.right: parent.right
        anchors.rightMargin: 14
        anchors.verticalCenter: parent.verticalCenter
        width: 24
        height: 24
        name: header.chevron
        color: window.ui.textMuted
        rotation: header.chevron === "expand_more" && header.isOpen ? 180 : 0
        Behavior on rotation { NumberAnimation { duration: 150; easing.type: Easing.OutQuad } }
    }

    MouseArea {
        id: headerArea
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: header.clicked()
    }
}
