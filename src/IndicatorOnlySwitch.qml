import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0
import QtQuick.Dialogs 1.0

SwitchDelegate {
    id: root
    Layout.topMargin: 3
    Layout.bottomMargin: 3
    background: Rectangle {
        color: "transparent"
        radius: 6
        border.width: 1
        border.color: Qt.rgba(0.5, 0.5, 0.5, 0.38)
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            if (mouse.x > parent.width - parent.indicator.width) {
                root.checked = !root.checked
                root.clicked()
            }
        }
    }
}
