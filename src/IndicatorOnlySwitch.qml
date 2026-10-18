import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0
import QtQuick.Dialogs 1.0

SwitchDelegate {
    id: root

    // Preserve Material styling while making translated setting titles responsive.
    function configureTextItem(item) {
        if (!item)
            return false

        if (item.wrapMode !== undefined && item.maximumLineCount !== undefined) {
            item.wrapMode = Text.WrapAtWordBoundaryOrAnywhere
            item.maximumLineCount = Qt.binding(function () { return window.textScale !== 1 ? 100 : 2 })
            if (item.elide !== undefined)
                item.elide = Text.ElideRight
            return true
        }

        if (item.children) {
            for (var i = 0; i < item.children.length; ++i) {
                if (configureTextItem(item.children[i]))
                    return true
            }
        }

        return false
    }

    function configureContentItem() {
        configureTextItem(contentItem)
    }

    // The title wraps, but its label reports the height of one line: with a bigger text the
    // second line ran into the hint under the switch. The row takes the height of the wrapped title
    Text {
        id: titleProbe
        visible: false
        width: Math.max(0, root.availableWidth - root.indicator.width - root.spacing)
        text: root.text
        font: root.font
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
    }
    Binding on implicitHeight {
        when: window.textScale !== 1
        value: Math.max(root.implicitBackgroundHeight + root.topInset + root.bottomInset,
                        root.implicitContentHeight + root.topPadding + root.bottomPadding,
                        root.implicitIndicatorHeight + root.topPadding + root.bottomPadding,
                        titleProbe.height + root.topPadding + root.bottomPadding)
    }

    Component.onCompleted: Qt.callLater(configureContentItem)
    onContentItemChanged: Qt.callLater(configureContentItem)
    onTextChanged: Qt.callLater(configureContentItem)

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
