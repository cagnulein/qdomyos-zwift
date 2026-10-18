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

    // With a bigger text a title in two lines ran into the hint under the switch. The label (IconLabel
    // of Qt 5.15) gives its text the width first: the title wraps there, and the position the label
    // sets next is the one it worked out for one line, so the second line was half a line lower
    // (emulator log, 39172099490: the row high enough, the text moved). A wrapped title makes the row
    // higher than the one for one line, so the label places the text again, 6 px clear above and
    // below. The first three terms are those of the Material style: at 100 % the height is the one of
    // the style
    property var titleText: null
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding,
                             window.textScale !== 1 && titleText && titleText.lineCount > 1
                                 ? Math.max(implicitBackgroundHeight + topInset + bottomInset + 1,
                                            titleText.contentHeight + topPadding + bottomPadding + 12)
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
