import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

// ComboBox of the settings pages and the wizard. On Android 12+ the window runs under the
// status and navigation bars (edge to edge), and with the default 12 px margins a long list
// (App Language, tile order) was pushed under the status bar, hiding its first item.
ComboBox {
    id: control

    // Shown text of an item; ValueComboBox maps raw values to translated labels
    function labelFor(v) {
        return v
    }

    // A settings row binds the saved setting here instead of displayText (savedText) or
    // currentIndex (savedIndex); the list shows it and shows it again whenever the setting
    // changes (after OK too). `dirty` - another item is picked than the saved one, the OK button
    // of the row is enabled by it: a grey OK means there is nothing to save. A list without
    // either (the wizard) is never dirty
    property var savedText: undefined
    property var savedIndex: undefined
    property bool dirty: (savedText !== undefined && displayText !== String(savedText))
                         || (savedIndex !== undefined && currentIndex !== savedIndex)
    // ValueComboBox shows a saved raw value through its `value`
    function showSaved() {
        if (savedText !== undefined)
            displayText = savedText
        if (savedIndex !== undefined)
            currentIndex = savedIndex
    }
    onSavedTextChanged: control.showSaved()
    onSavedIndexChanged: control.showSaved()
    Component.onCompleted: {
        control.showSaved()
        // Screen reader: the text inside the box (the Material style's TextField) is a node of
        // its own, so a TalkBack swipe also stopped on it and read the bare value ("75"); the box
        // itself already reads its name and value. None of these boxes is editable
        if (!control.editable && control.contentItem)
            control.contentItem.Accessible.ignored = true
    }

    // Width of the widest item, measured when the list opens
    property real widestItem: 0

    TextMetrics {
        id: itemMetrics
        font: control.font
    }

    popup.topMargin: 12 + window.getTopPadding()
    popup.bottomMargin: 12 + window.getBottomPadding()
    popup.leftMargin: 12
    popup.rightMargin: 12

    // With a bigger text the label left of the box wraps, but the row still shared its width as
    // if the label were one line, and squeezed the box: the shown item was cut in the middle of
    // a word. The box keeps the width of its text (at most half the window); 0 is the default
    Layout.minimumWidth: window.textScale !== 1 ? Math.min(implicitWidth, window.width / 2) : 0

    // Modal, as the menus of the workout history, so a screen reader's touch on an item cannot
    // reach the row under the list; no dim, so it looks as before
    popup.modal: true
    popup.dim: false

    // Same as the Material style's default delegate, with a press action for a screen reader:
    // Qt 5.15 gives a MenuItem no accessible action (Qt 6.5 does), so on Android its node is not
    // clickable and a TalkBack touch on an item went to the whole window. The name is set from
    // the text too, as in the menus of the workout history
    delegate: MenuItem {
        width: ListView.view.width
        text: control.textRole ? (Array.isArray(control.model) ? modelData[control.textRole] : model[control.textRole]) : modelData
        Accessible.name: text
        Accessible.onPressAction: clicked()
        Material.foreground: control.currentIndex === index ? ListView.view.contentItem.Material.accent : ListView.view.contentItem.Material.foreground
        highlighted: control.highlightedIndex === index
        hoverEnabled: control.hoverEnabled
    }

    // The stock list takes its height from the ListView, which creates its items only as
    // they come into view: a long list opened short over its field and a moment later grew
    // up to the top. All items have the same height, so the full height is known at once.
    popup.height: Math.min(count * Material.menuItemHeight + popup.topPadding + popup.bottomPadding,
                           control.Window.height - popup.topMargin - popup.bottomMargin)

    // The stock list grows from 0.9 to its full size as it opens, and Qt places it while it is
    // still scaled down: a long list was fitted to the window at 90% of its height, then grew
    // past the bottom edge and jumped up a moment later. The list only fades in and out now;
    // the scale is reset as well, since Qt 5.15 does not always restore it after the stock
    // shrinking exit (a list opened again stayed at 90%).
    popup.enter: Transition {
        PropertyAction { property: "scale"; value: 1.0 }
        NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; easing.type: Easing.OutCubic; duration: 150 }
    }
    popup.exit: Transition {
        NumberAnimation { property: "opacity"; to: 0.0; easing.type: Easing.OutCubic; duration: 150 }
    }

    // The stock list is as wide as the field, and long labels ("Auto (system language)",
    // "Portuguese (Brazil)" in some languages) were cut off
    popup.width: Math.min(Math.max(control.width, widestItem + 32 + popup.leftPadding + popup.rightPadding),
                          control.Window.width - popup.leftMargin - popup.rightMargin)

    Connections {
        target: control.popup
        function onAboutToShow() {
            var widest = 0
            for (var i = 0; i < control.count; ++i) {
                itemMetrics.text = control.labelFor(control.textAt(i))
                widest = Math.max(widest, itemMetrics.advanceWidth)
            }
            control.widestItem = Math.ceil(widest)
        }
    }
}
