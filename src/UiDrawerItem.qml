import QtQuick 2.12
import QtQuick.Controls 2.5

// Navigation drawer entry of the modern look: icon, label and a pill highlight
ItemDelegate {
    id: control
    property string iconName: ""
    property bool external: false

    width: parent ? parent.width : implicitWidth
    // 48 at 100 %, more as the text grows: from the size, not from the label, which is taller
    // in a CJK fallback font
    height: Math.max(48, Math.round(window.ui.fontLabel * 1.2) + 30)
    leftPadding: 28
    rightPadding: 24
    focusPolicy: Qt.NoFocus

    // Screen reader: a button named after the entry. In Qt 5.15 its double tap does not send
    // clicked() to an ItemDelegate, so the press action does
    Accessible.role: Accessible.Button
    Accessible.name: text
    Accessible.onPressAction: clicked()

    readonly property bool lit: down

    background: Rectangle {
        x: 12
        y: 2
        width: control.width - 24
        height: control.height - 4
        radius: height / 2
        color: control.lit ?window.ui.alpha(window.ui.accent, 0.22)
             : control.hovered ? window.ui.alpha(window.ui.textMain, 0.06)
             : "transparent"
    }

    contentItem: Item {
        UiIcon {
            id: itemIcon
            anchors.verticalCenter: parent.verticalCenter
            width: 22
            height: 22
            name: control.iconName
            color: control.lit ? window.ui.accent : window.ui.textMuted
        }
        Label {
            anchors.left: itemIcon.right
            anchors.leftMargin: 16
            anchors.right: externalIcon.visible ? externalIcon.left : parent.right
            anchors.verticalCenter: parent.verticalCenter
            text: control.text
            font.pixelSize: window.ui.fontLabel
            font.weight: Font.Medium
            color: window.ui.textMain
            elide: Text.ElideRight
        }
        UiIcon {
            id: externalIcon
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            width: 16
            height: 16
            name: "open_in_new"
            color: window.ui.textMuted
            visible: control.external
        }
    }
}
