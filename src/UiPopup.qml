import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Material.impl 2.12

// Popup of the notices ("The tiles are locked now", "Saved!" ...). Classic look: the
// background of the Qt 5.15 Material Popup, copied as is. Modern look: a rounded card
// in the theme colours, without the shadow. A popup with its own background keeps it.
Popup {
    id: control

    readonly property bool modern: window.ui.modern
    // Modern look: a snackbar (UiNotice) - the inverse colours, dark on a light page and
    // light on a dark one, and no outline
    property bool inverse: false

    // Material dims with a light veil in the dark theme, and the page looks washed out:
    // the modern look darkens it. Classic: the Material Popup overlay as is.
    Overlay.modal: Rectangle {
        color: control.modern ? window.ui.scrim
                              : control.Material.backgroundDimColor
        Behavior on opacity { NumberAnimation { duration: 150 } }
    }
    Overlay.modeless: Rectangle {
        color: control.modern ? window.ui.scrim
                              : control.Material.backgroundDimColor
        Behavior on opacity { NumberAnimation { duration: 150 } }
    }

    // Modern: UiFrame, not Rectangle.border - a thin border breaks up on Android
    background: UiFrame {
        radius: control.modern ? (control.inverse ? 14 : 20) : 2
        fill: control.modern ? (control.inverse ? window.ui.textMain : window.ui.surfaceHigh) : control.Material.dialogColor
        stroke: window.ui.outline
        strokeWidth: control.modern && !control.inverse && !window.ui.dark ? 1 : 0

        layer.enabled: !control.modern && control.Material.elevation > 0
        layer.effect: ElevationEffect {
            elevation: control.Material.elevation
        }
    }
}
