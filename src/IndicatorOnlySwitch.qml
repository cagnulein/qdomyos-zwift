import QtQuick 2.7
import QtQml 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0
import QtQuick.Dialogs 1.0

UiSwitchDelegate {
    id: root
    wrapsTitle: true
    // it wraps its title itself (configureTextItem), not by the one of UiSwitchDelegate
    pageWrap: false

    // Preserve Material styling while making translated setting titles responsive.
    function configureTextItem(item) {
        if (!item)
            return false

        if (item.wrapMode !== undefined && item.maximumLineCount !== undefined) {
            // set once: again, "maximumLineCount = 2" would undo the Binding of a bigger text
            if (item === titleText)
                return true
            item.wrapMode = Text.WrapAtWordBoundaryOrAnywhere
            // two lines; the modern look and a bigger text lift the limit (the Binding below)
            item.maximumLineCount = 2
            if (item.elide !== undefined)
                item.elide = Text.ElideRight
            // a wrapped title of the modern look: the lines closer (UiSwitchDelegate.titleLeading)
            item.lineHeight = Qt.binding(function () { return root.titleLeading })
            // last: the Bindings below start with it and keep these values to restore at 100 %
            titleText = item
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
    // the style. The last one is the wrapped title of the modern look (UiSwitchDelegate): this
    // implicitHeight replaces the one of the base component
    property var titleText: null
    // The text size of the app (main.qml). Without the app window, as in the layout test of the
    // settings labels, the switch is the one of 100 %
    readonly property real textScale: typeof window !== "undefined" && window ? window.textScale : 1
    readonly property int scaledFontPx: typeof window !== "undefined" && window ? window.scaledFontPx : 0
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding,
                             textScale !== 1 && titleText && titleText.lineCount > 1
                                 ? Math.max(implicitBackgroundHeight + topInset + bottomInset + 1,
                                            titleText.contentHeight + topPadding + bottomPadding + 12)
                                 : 0,
                             modernTitleHeight)

    // On iOS the title kept the size of the style while the text around it grew (the hint under it
    // too): with a size other than 100 % it takes the one of the window font, family, weight and
    // style stay those of the style. At 100 % the size of the style, as before
    // (a Binding on "font.pixelSize" did not give the size back at 100 %: set here both ways)
    property bool titleFontSet: false
    function applyTitleFont() {
        if (!titleText)
            return
        if (scaledFontPx > 0) {
            // the size of the switch font: the window font (scaledFontPx) as before, Body Large
            // on a settings page of the modern look, which sets it on its root
            titleText.font.pixelSize = root.modern && font.pixelSize > 0 ? font.pixelSize : scaledFontPx
            titleFontSet = true
        } else if (titleFontSet && contentItem && contentItem.font !== undefined) {
            titleText.font = contentItem.font
            titleFontSet = false
        }
    }
    onScaledFontPxChanged: applyTitleFont()
    onFontChanged: applyTitleFont()
    onTitleTextChanged: applyTitleFont()
    // With a bigger text a long title needs more than two lines; back to two at 100 %. The modern
    // look: the whole title, on as many lines as it takes (the ellipsis hid the end of a long
    // translation at a large text size)
    Binding {
        target: root.titleText
        property: "maximumLineCount"
        value: 100
        when: root.titleText !== null && (root.textScale !== 1 || root.modern)
        restoreMode: Binding.RestoreBindingOrValue
    }

    Component.onCompleted: Qt.callLater(configureContentItem)
    onContentItemChanged: Qt.callLater(configureContentItem)
    onTextChanged: Qt.callLater(configureContentItem)

    MouseArea {
        anchors.fill: parent
        onClicked: {
            // With a right-to-left system language (Hebrew, Arabic) Qt 5 mirrors the control and
            // draws the switch on the left (QQuickControl::isMirrored() follows the locale), while
            // the touch position here is not mirrored: count the touch on the side it is drawn
            var onIndicator = root.mirrored ? mouse.x < root.indicator.width
                                            : mouse.x > root.width - root.indicator.width
            if (onIndicator) {
                root.checked = !root.checked
                root.clicked()
            }
        }
    }
}
