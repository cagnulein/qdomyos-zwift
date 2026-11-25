import QtQuick 2.12
import QtQuick.Templates 2.12 as T
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Material.impl 2.12

// GroupBox of the settings pages: the Qt 5.15 Material GroupBox, copied as is
// (qtquickcontrols2 5.15, src/imports/controls/material/GroupBox.qml), so nothing changes
// with the modern look off. Modern look: a filled card with rounded corners, the title inside
// it in the accent colour, no frame line; framed - a thin frame on the page colour, no card.
T.GroupBox {
    id: control

    readonly property bool modern: window.ui.modern
    // Modern look: a thin frame instead of the fill
    property bool framed: false

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding,
                            implicitLabelWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    spacing: modern ? 8 : 6
    padding: modern ? 16 : 12
    topPadding: modern ? 14 + (implicitLabelWidth > 0 ? implicitLabelHeight + spacing : 0)
                       : Material.frameVerticalPadding + (implicitLabelWidth > 0 ? implicitLabelHeight + spacing : 0)
    bottomPadding: modern ? 12 : Material.frameVerticalPadding

    label: Text {
        x: control.leftPadding
        y: control.modern ? 14 : 0
        width: control.availableWidth

        text: control.title
        // Modern: the title of a group, Title Medium 16 (was 15 DemiBold)
        font: control.modern ? window.ui.fontOf(control.font, window.ui.fontBody, Font.Medium)
                             : control.font
        color: control.modern ? window.ui.accent
                              : (control.enabled ? control.Material.foreground : control.Material.hintTextColor)
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
    }

    // UiFrame, not Rectangle.border: in the modern look the thin frame of framed broke up on
    // Android along its top edge. The classic look is the same Rectangle with its border
    background: UiFrame {
        y: control.modern ? 0 : control.topPadding - control.bottomPadding
        width: parent.width
        height: control.modern ? parent.height : parent.height - control.topPadding + control.bottomPadding

        radius: control.modern ? 16 : 2
        fill: control.modern ? (control.framed ? "transparent" : window.ui.surface)
                             : (control.Material.elevation > 0 ? control.Material.backgroundColor : "transparent")
        stroke: control.modern ? window.ui.outline : control.Material.frameColor
        strokeWidth: control.modern ? (control.framed ? 1 : 0) : 1

        layer.enabled: !control.modern && control.enabled && control.Material.elevation > 0
        layer.effect: ElevationEffect {
            elevation: control.Material.elevation
        }
    }
}
