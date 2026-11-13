import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.impl 2.15
import QtQuick.Templates 2.15 as T
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material.impl 2.15

// ComboBox of the settings pages and the wizard: the Qt 5.15 Material ComboBox, copied as
// is (qtquickcontrols2 5.15, src/imports/controls/material/ComboBox.qml), so nothing changes
// with the modern look off. Modern look: the box is one step lighter than the page (Material
// painted it with dialogColor = the page colour, and it vanished into the page), rounded
// like the text fields and without the shadow; the list is rounded as well.
T.ComboBox {
    id: control

    readonly property bool modern: window.ui.modern

    // Shown text for a raw value: ValueComboBox puts its translated labels here; values
    // missing from the map are shown unchanged
    property var itemLabels: null
    function labelFor(v) {
        return itemLabels && itemLabels.hasOwnProperty(v) ? itemLabels[v] : v
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

    // Width of the widest item, measured when the list opens (#5236)
    property real widestItem: 0

    TextMetrics {
        id: itemMetrics
        font: control.font
    }

    // With a bigger text the label left of the box wraps, but the row still shared its width as
    // if the label were one line, and squeezed the box: the shown item was cut in the middle of
    // a word. The box keeps the width of its text (at most half the window); 0 is the default
    Layout.minimumWidth: window.textScale !== 1 ? Math.min(implicitWidth, window.width / 2) : 0

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

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)

    topInset: 6
    bottomInset: 6

    leftPadding: padding + (!control.mirrored || !indicator || !indicator.visible ? 0 : indicator.width + spacing)
    rightPadding: padding + (control.mirrored || !indicator || !indicator.visible ? 0 : indicator.width + spacing)

    Material.elevation: flat ? control.pressed || control.hovered ? 2 : 0
                             : control.pressed ? 8 : 2
    Material.background: modern ? window.ui.surfaceHighest : (flat ? "transparent" : undefined)
    Material.foreground: flat ? undefined : Material.primaryTextColor

    delegate: MenuItem {
        id: menuItem
        width: ListView.view.width
        text: control.labelFor(control.textRole ? (Array.isArray(control.model) ? modelData[control.textRole] : model[control.textRole]) : modelData)
        // Screen reader: Qt 5.15 gives a MenuItem no accessible action (Qt 6.5 does), so on
        // Android its node is not clickable and a TalkBack touch on an item went to the whole
        // window. The name is set from the text too, as in the menus of the workout history
        Accessible.name: text
        Accessible.onPressAction: clicked()
        Material.foreground: control.currentIndex === index ? ListView.view.contentItem.Material.accent : ListView.view.contentItem.Material.foreground
        highlighted: control.highlightedIndex === index
        hoverEnabled: control.hoverEnabled

        // Material's MenuItem background; modern: the highlight is a rounded block inset from
        // the edges, not a square bar across the whole list
        background: Rectangle {
            x: control.modern ? 8 : 0
            implicitWidth: 200
            implicitHeight: menuItem.Material.menuItemHeight
            width: menuItem.width - 2 * x
            height: menuItem.height
            radius: control.modern ? 10 : 0
            color: menuItem.highlighted ? menuItem.Material.listHighlightColor : "transparent"

            Ripple {
                width: parent.width
                height: parent.height
                clip: visible
                clipRadius: parent.radius
                pressed: menuItem.pressed
                anchor: menuItem
                active: menuItem.down || menuItem.highlighted
                color: menuItem.Material.rippleColor
            }
        }
    }

    indicator: ColorImage {
        x: control.mirrored ? control.padding : control.width - width - control.padding
        y: control.topPadding + (control.availableHeight - height) / 2
        color: control.modern ? window.ui.textMuted
                              : (control.enabled ? control.Material.foreground : control.Material.hintTextColor)
        opacity: control.modern && !control.enabled ? 0.38 : 1
        source: "qrc:/qt-project.org/imports/QtQuick/Controls.2/Material/images/drop-indicator.png"
    }

    contentItem: T.TextField {
        padding: 6
        leftPadding: control.editable ? 2 : control.mirrored ? 0 : (control.modern ? 16 : 12)
        rightPadding: control.editable ? 2 : control.mirrored ? (control.modern ? 16 : 12) : 0

        text: control.editable ? control.editText : control.displayText

        enabled: control.editable
        autoScroll: control.editable
        readOnly: control.down
        inputMethodHints: control.inputMethodHints
        validator: control.validator
        selectByMouse: control.selectTextByMouse

        font: control.font
        color: control.enabled ? control.Material.foreground : control.Material.hintTextColor
        selectionColor: control.Material.accentColor
        selectedTextColor: control.Material.primaryHighlightedTextColor
        verticalAlignment: Text.AlignVCenter

        cursorDelegate: CursorDelegate { }
    }

    background: Rectangle {
        implicitWidth: 120
        implicitHeight: control.Material.buttonHeight

        radius: control.modern ? 12 : control.flat ? 0 : 2
        color: !control.editable ? control.Material.dialogColor : "transparent"

        layer.enabled: !control.modern && control.enabled && !control.editable && control.Material.background.a > 0
        layer.effect: ElevationEffect {
            elevation: control.Material.elevation
        }

        Rectangle {
            visible: control.editable
            y: parent.y + control.baselineOffset
            width: parent.width
            height: control.activeFocus ? 2 : 1
            color: control.editable && control.activeFocus ? control.Material.accentColor : control.Material.hintTextColor
        }

        Ripple {
            clip: control.flat || control.modern
            clipRadius: control.modern ? 12 : control.flat ? 0 : 2
            x: control.editable && control.indicator ? control.indicator.x : 0
            width: control.editable && control.indicator ? control.indicator.width : parent.width
            height: parent.height
            pressed: control.pressed
            anchor: control.editable && control.indicator ? control.indicator : control
            active: control.pressed || control.visualFocus || control.hovered
            color: control.Material.rippleColor
        }
    }

    popup: T.Popup {
        y: control.editable ? control.height - 5 : 0
        // As wide as the longest item (long labels were cut off), not narrower than the field
        // and kept inside the window; the height from the item count, the ListView builds its
        // items lazily (#5236)
        width: Math.min(Math.max(control.width, control.widestItem + 32 + leftPadding + rightPadding),
                        control.Window.width - leftMargin - rightMargin)
        height: Math.min(control.count * control.Material.menuItemHeight + topPadding + bottomPadding,
                         control.Window.height - topMargin - bottomMargin)
        transformOrigin: Item.Top
        // Modal, as the menus of the workout history, so a screen reader's touch on an item cannot
        // reach the row under the list; no dim, so it looks as before
        modal: true
        dim: false
        // The window runs under the status and gesture bars (edge to edge) in both looks, a
        // long list pushed to the top hid its first item under the status bar (#5236)
        topMargin: 12 + window.getTopPadding()
        bottomMargin: 12 + window.getBottomPadding()
        leftMargin: 12
        rightMargin: 12

        Material.theme: control.Material.theme
        Material.accent: control.Material.accent
        Material.primary: control.Material.primary

        // Modern: the list keeps clear of the rounded corners, the highlight of the first and
        // the last item was a square block in them
        topPadding: control.modern ? 8 : 0
        bottomPadding: control.modern ? 8 : 0

        enter: Transition {
            // Fade in only, no grow from 0.9: Qt 5.15 places the list while it is still scaled
            // down, a long list was fitted to the window at 90% of its height, grew past the
            // bottom edge and jumped up a moment later (#5236). The end values are given (as in
            // Qt 6): Qt 5.15 restores opacity and scale after the exit to what they were when it
            // began, and a list closed during its own opening kept a part of them - opened
            // again, it stayed invisible and took the next tap on an item nobody could see
            PropertyAction { property: "scale"; value: 1.0 }
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; easing.type: Easing.OutCubic; duration: 150 }
        }

        exit: Transition {
            NumberAnimation { property: "opacity"; to: 0.0; easing.type: Easing.OutCubic; duration: 150 }
        }

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.delegateModel
            currentIndex: control.highlightedIndex
            highlightMoveDuration: 0

            T.ScrollIndicator.vertical: ScrollIndicator { }
        }

        background: Rectangle {
            radius: control.modern ? 12 : 2
            color: parent.Material.dialogColor

            layer.enabled: control.enabled
            layer.effect: ElevationEffect {
                elevation: 8
            }
        }
    }
}
