import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0
import QtQuick.Dialogs 1.0

SwitchDelegate {
    id: root

    function configureContentItem() {
        if (!contentItem)
            return

        if (contentItem.wrapMode !== undefined)
            contentItem.wrapMode = Text.WrapAtWordBoundaryOrAnywhere
        if (contentItem.maximumLineCount !== undefined)
            contentItem.maximumLineCount = 2
        if (contentItem.elide !== undefined)
            contentItem.elide = Text.ElideRight
    }

    Component.onCompleted: configureContentItem()
    onContentItemChanged: configureContentItem()

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
