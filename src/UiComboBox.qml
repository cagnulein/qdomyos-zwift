import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.impl 2.15
import QtQuick.Templates 2.15 as T
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material.impl 2.15
import QtGraphicalEffects 1.12

// ComboBox of the settings pages and the wizard: the Qt 5.15 Material ComboBox, copied as
// is (qtquickcontrols2 5.15, src/imports/controls/material/ComboBox.qml), so nothing changes
// with the modern look off. Modern look: the box is one step lighter than the page (Material
// painted it with dialogColor = the page colour, and it vanished into the page), rounded
// like the text fields and without the shadow; the list is a Material 3 menu: the container
// of UiMenuBackground, items in rounded blocks inset by 8, the chosen one tinted by the accent.
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

    // Modern: the two items of the list pressed last while it is open (-1 - none) - they keep the
    // rounded mask of their ripple (see the delegate)
    property int pressedItemIndex: -1
    property int previousPressedItemIndex: -1
    // Modern: the item under the finger now (-1 - none), it has a rounded block while held
    property int downItemIndex: -1

    // Modern: the tone of the open list. A list on a popup of the surfaceHigh tone (the RPE
    // popup) takes surface, one tone below it as a Material 3 menu in a dialog - with the
    // default it was one colour with the popup
    property color menuColor: window.ui.surfaceHigh

    TextMetrics {
        id: itemMetrics
        font: control.font
    }

    // With a bigger text the label left of the box wraps, but the row still shared its width as
    // if the label were one line, and squeezed the box: the shown item was cut in the middle of
    // a word. The box keeps the width of its text (at most half the window); 0 is the default
    Layout.minimumWidth: window.textScale !== 1 ? Math.min(implicitWidth, window.width / 2) : 0

    // Width of the widest item for the list width (#5236), measured when the list opens;
    // DeviceComboBox calls it too: its items change while the list is open
    function measureItems() {
        var widest = 0
        for (var i = 0; i < control.count; ++i) {
            itemMetrics.text = control.labelFor(control.textAt(i))
            widest = Math.max(widest, itemMetrics.advanceWidth)
        }
        control.widestItem = Math.ceil(widest)
    }

    Connections {
        target: control.popup
        function onAboutToShow() {
            control.measureItems()
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

    // Modern: the item has a rounded block - it is the chosen one, the one highlighted or the
    // one held by a finger
    function hasBlock(i) {
        return i === control.currentIndex || i === control.highlightedIndex
            || i === control.downItemIndex
    }

    delegate: MenuItem {
        id: menuItem
        width: ListView.view.width
        text: control.labelFor(control.textRole ? (Array.isArray(control.model) ? modelData[control.textRole] : model[control.textRole]) : modelData)
        // Screen reader: Qt 5.15 gives a MenuItem no accessible action (Qt 6.5 does), so on
        // Android its node is not clickable and a TalkBack touch on an item went to the whole
        // window. The name is set from the text too, as in the menus of the workout history
        Accessible.name: text
        Accessible.onPressAction: clicked()
        Material.foreground: control.modern ? (control.currentIndex === index ? window.ui.accent : window.ui.textMain)
                                            : control.currentIndex === index ? ListView.view.contentItem.Material.accent : ListView.view.contentItem.Material.foreground
        highlighted: control.highlightedIndex === index
        hoverEnabled: control.hoverEnabled
        onPressedChanged: {
            if (pressed && control.pressedItemIndex !== index) {
                control.previousPressedItemIndex = control.pressedItemIndex
                control.pressedItemIndex = index
            }
        }
        onDownChanged: {
            if (down)
                control.downItemIndex = index
            else if (control.downItemIndex === index)
                control.downItemIndex = -1
        }

        // Material's MenuItem background; modern (Material 3 menu): a rounded block inset from
        // the edges, not a square bar across the whole list - the chosen item a tint of the
        // accent (as a chosen list card), another item under the finger or the keyboard focus
        // the state layer of the text colour (8 %)
        background: Rectangle {
            x: control.modern ? 8 : 0
            implicitWidth: 200
            implicitHeight: menuItem.Material.menuItemHeight
            width: menuItem.width - 2 * x
            height: menuItem.height
            radius: control.modern ? 12 : 0
            color: !control.modern ? (menuItem.highlighted ? menuItem.Material.listHighlightColor : "transparent")
                 : control.currentIndex === index ? window.ui.alpha(window.ui.accent, 0.14)
                 : menuItem.highlighted ? window.ui.alpha(window.ui.textMain, 0.08)
                 : "transparent"

            // Modern: the mask of the rounded block for the ripple below
            Rectangle {
                id: rippleMask
                visible: false
                width: parent.width
                height: parent.height
                radius: parent.radius
            }

            // Qt 5.15 clips the ripple wave by its bounding rectangle - clipRadius rounds only the
            // pressed tint (qquickmaterialripple.cpp, clipNode->setRadius commented out,
            // QTBUG-51894; fixed in Qt 6.5): an item held by a finger showed the wave with square
            // corners. Modern: the wave is cut by the rounded block with a layer, only on the two
            // items pressed last (old exercise bike consoles: at most two small layers). A released
            // wave still grows and fades for 333 ms, so the layer stays after the press until the
            // list closes (popup onClosed comes after the exit transition): a pick, a press taken
            // over by a drag of the list, and that one while another item is pressed right after it
            Item {
                width: parent.width
                height: parent.height
                layer.enabled: control.modern
                               && (menuItem.down || control.pressedItemIndex === index
                                   || control.previousPressedItemIndex === index)
                layer.effect: OpacityMask {
                    maskSource: rippleMask
                }

                Ripple {
                    width: parent.width
                    height: parent.height
                    clip: visible
                    clipRadius: rippleMask.radius
                    pressed: menuItem.pressed
                    anchor: menuItem
                    // Modern: only while pressed - the block of the chosen or highlighted item is
                    // its colour already, the held ripple laid a grey veil over it the whole time
                    // the list was open (it opens with the chosen item highlighted, a touch does
                    // not move that)
                    active: menuItem.down || (!control.modern && menuItem.highlighted)
                    // Modern: the Material 3 state layer, the text colour at 10 % (as UiButton)
                    color: control.modern ? window.ui.alpha(window.ui.textMain, 0.10) : menuItem.Material.rippleColor
                }
            }

            // Modern: a thin line under the item, inset like the highlight, so the items of a long
            // list (heart rate belts with similar names) do not run together. None under the last
            // item and none around a rounded block (the chosen, the highlighted or the pressed item), it is not cut
            Rectangle {
                visible: control.modern && index < control.count - 1
                         && !control.hasBlock(index) && !control.hasBlock(index + 1)
                y: parent.height - height
                width: parent.width
                height: 1
                color: window.ui.outline
                opacity: 0.6
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
            // Modern: no wave, only the pressed tint - Qt 5.15 does not round the wave (see the
            // items of the list)
            pressed: control.modern ? false : control.pressed
            anchor: control.editable && control.indicator ? control.indicator : control
            active: control.pressed || control.visualFocus || control.hovered
            color: control.modern ? window.ui.alpha(window.ui.textMain, 0.10) : control.Material.rippleColor
        }
    }

    popup: T.Popup {
        onClosed: {
            control.pressedItemIndex = -1
            control.previousPressedItemIndex = -1
            control.downItemIndex = -1
        }
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

        // Classic: Material's popup background as before. Modern: the Material 3 menu container
        // (UiMenuBackground) - Material gave the list the page colour (dialogColor is the
        // window's Material.background), and on a card it looked like a hole in it
        background: Item {
            Rectangle {
                visible: !control.modern
                anchors.fill: parent
                radius: 2
                color: Material.dialogColor

                layer.enabled: visible && control.enabled
                layer.effect: ElevationEffect {
                    elevation: 8
                }
            }
            UiMenuBackground {
                visible: control.modern
                anchors.fill: parent
                color: control.menuColor
            }
        }
    }
}
