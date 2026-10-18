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
            titleText = item
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
    // second line ran into the hint under the switch. The row takes the height of the wrapped title:
    // the text item of the label (its content height counts the wrapped lines) or, until it is
    // found, a hidden copy of the title. The first three terms are those of the Material style, so
    // at 100 % the height is the one of the style
    property var titleText: null
    Text {
        id: titleProbe
        visible: false
        width: Math.max(0, root.availableWidth - root.indicator.width - root.spacing)
        text: root.text
        font: root.font
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
    }
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding,
                             window.textScale !== 1
                                 ? Math.max(titleProbe.height, titleText ? titleText.contentHeight : 0) + topPadding + bottomPadding
                                 : 0)

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
