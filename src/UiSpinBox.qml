import QtQuick 2.12
import QtQuick.Templates 2.12 as T
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Material.impl 2.12

// +/- field: the Qt 5.15 Material SpinBox, copied as is (qtquickcontrols2 5.15,
// src/imports/controls/material/SpinBox.qml), for the classic look. The modern look draws the
// field of the Wahoo gear table: a rounded field, rounded +/- squares, the number centred by its
// digits, no hover and no ripple. On a touch screen the tap counts as hover until the next tap,
// and the Material +/- stayed lit after it.
T.SpinBox {
    id: control

    readonly property bool modern: window.ui.modern

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentItem.implicitWidth +
                            up.implicitIndicatorWidth +
                            down.implicitIndicatorWidth)
    implicitHeight: Math.max(implicitContentHeight + topPadding + bottomPadding,
                             implicitBackgroundHeight,
                             up.implicitIndicatorHeight,
                             down.implicitIndicatorHeight)

    hoverEnabled: modern ? false : Qt.styleHints.useHoverEffects
    spacing: 6
    topPadding: modern ? 0 : 8
    bottomPadding: modern ? 0 : 16
    leftPadding: (control.mirrored ? (up.indicator ? up.indicator.width : 0) : (down.indicator ? down.indicator.width : 0))
    rightPadding: (control.mirrored ? (down.indicator ? down.indicator.width : 0) : (up.indicator ? up.indicator.width : 0))

    validator: IntValidator {
        locale: control.locale.name
        bottom: Math.min(control.from, control.to)
        top: Math.max(control.from, control.to)
    }

    contentItem: UiSpinInput {
        text: control.displayText

        font: control.modern ? window.ui.fontOf(control.font, window.ui.fontBody) : control.font
        color: control.modern ? window.ui.textMain
                              : (enabled ? control.Material.foreground : control.Material.hintTextColor)
        selectionColor: control.modern ? window.ui.accent : control.Material.textSelectionColor
        selectedTextColor: control.modern ? window.ui.accentInk : control.Material.foreground

        cursorDelegate: CursorDelegate { }

        readOnly: !control.editable
        validator: control.validator
        inputMethodHints: control.inputMethodHints
    }

    up.indicator: Item {
        x: control.mirrored ? 0 : parent.width - width
        implicitWidth: control.modern ? 44 : control.Material.touchTarget
        implicitHeight: control.modern ? 44 : control.Material.touchTarget
        height: parent.height
        width: height

        Rectangle {
            visible: control.modern
            anchors.fill: parent
            radius: 12
            color: control.up.pressed ? window.ui.surfaceHigh : window.ui.surfaceHighest
        }

        Ripple {
            visible: !control.modern
            clipRadius: 2
            x: control.spacing
            y: control.spacing
            width: parent.width - 2 * control.spacing
            height: parent.height - 2 * control.spacing
            pressed: control.up.pressed
            active: control.up.pressed || control.up.hovered || control.visualFocus
            color: control.Material.rippleColor
        }

        Rectangle {
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            width: control.modern ? 14 : Math.min(parent.width / 3, parent.height / 3)
            height: 2
            radius: control.modern ? 1 : 0
            color: control.modern ? window.ui.textMain
                                  : (enabled ? control.Material.foreground : control.Material.spinBoxDisabledIconColor)
        }
        Rectangle {
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            width: 2
            height: control.modern ? 14 : Math.min(parent.width / 3, parent.height / 3)
            radius: control.modern ? 1 : 0
            color: control.modern ? window.ui.textMain
                                  : (enabled ? control.Material.foreground : control.Material.spinBoxDisabledIconColor)
        }
    }

    down.indicator: Item {
        x: control.mirrored ? parent.width - width : 0
        implicitWidth: control.modern ? 44 : control.Material.touchTarget
        implicitHeight: control.modern ? 44 : control.Material.touchTarget
        height: parent.height
        width: height

        Rectangle {
            visible: control.modern
            anchors.fill: parent
            radius: 12
            color: control.down.pressed ? window.ui.surfaceHigh : window.ui.surfaceHighest
        }

        Ripple {
            visible: !control.modern
            clipRadius: 2
            x: control.spacing
            y: control.spacing
            width: parent.width - 2 * control.spacing
            height: parent.height - 2 * control.spacing
            pressed: control.down.pressed
            active: control.down.pressed || control.down.hovered || control.visualFocus
            color: control.Material.rippleColor
        }

        Rectangle {
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            width: control.modern ? 14 : parent.width / 3
            height: 2
            radius: control.modern ? 1 : 0
            color: control.modern ? window.ui.textMain
                                  : (enabled ? control.Material.foreground : control.Material.spinBoxDisabledIconColor)
        }
    }

    background: Item {
        implicitWidth: control.modern ? 160 : 192
        implicitHeight: control.modern ? 44 : control.Material.touchTarget

        Rectangle {
            visible: control.modern
            anchors.fill: parent
            radius: 12
            color: window.ui.surfaceHighest
        }

        Rectangle {
            visible: !control.modern
            x: parent.width / 2 - width / 2
            y: parent.y + parent.height - height - control.bottomPadding / 2
            width: control.availableWidth
            height: control.activeFocus ? 2 : 1
            color: control.activeFocus ? control.Material.accentColor : control.Material.hintTextColor
        }
    }
}
