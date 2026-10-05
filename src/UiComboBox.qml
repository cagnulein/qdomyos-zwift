import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

// ComboBox of the settings pages and the wizard. On Android 12+ the window runs under the
// status and navigation bars (edge to edge), and with the default 12 px margins a long list
// (App Language, tile order) was pushed under the status bar, hiding its first item.
ComboBox {
    id: control

    // Shown text of an item; ValueComboBox maps raw values to translated labels
    function labelFor(v) {
        return v
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
