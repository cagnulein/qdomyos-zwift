import QtQuick 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Material.impl 2.12
import QtQuick.Templates 2.12 as T

// Switch of the settings pages: the Qt 5.15 Material Switch, copied as is
// (qtquickcontrols2 5.15, src/imports/controls/material/Switch.qml), with the indicator
// swapped for UiSwitchIndicator - the stock one in the classic look, the Material 3 switch
// in the modern look (without the round ripple, which belongs to the classic handle).
T.Switch {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)

    padding: 8
    spacing: 8

    indicator: UiSwitchIndicator {
        x: text ? (control.mirrored ? control.width - width - control.rightPadding : control.leftPadding) : control.leftPadding + (control.availableWidth - width) / 2
        y: control.topPadding + (control.availableHeight - height) / 2
        control: control

        Ripple {
            visible: !window.ui.modern
            x: parent.handle.x + parent.handle.width / 2 - width / 2
            y: parent.handle.y + parent.handle.height / 2 - height / 2
            width: 28; height: 28
            pressed: control.pressed
            active: control.down || control.visualFocus || control.hovered
            color: control.checked ? control.Material.highlightedRippleColor : control.Material.rippleColor
        }
    }

    contentItem: Text {
        leftPadding: control.indicator && !control.mirrored ? control.indicator.width + control.spacing : 0
        rightPadding: control.indicator && control.mirrored ? control.indicator.width + control.spacing : 0

        text: control.text
        font: control.font
        color: control.enabled ? control.Material.foreground : control.Material.hintTextColor
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
    }
}
