import QtQuick 2.12
import QtQuick.Templates 2.12 as T
import QtQuick.Controls 2.12
import QtQuick.Controls.impl 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Material.impl 2.12

// Button of the settings pages. Classic look: the Qt 5.15 Material Button, copied as is
// (qtquickcontrols2 5.15, src/imports/controls/material/Button.qml), so nothing changes
// with the modern look off. Modern look: a flat tonal pill, the accent for highlighted.
T.Button {
    id: control

    readonly property bool modern: window.ui.modern
    // Modern look: a destructive action (Delete) in the danger colour instead of the accent;
    // the classic look ignores it and keeps whatever Material.background the caller sets
    property bool danger: false
    // Modern look: the OK button right after a drop-down list saves as soon as an item is picked,
    // as in the system settings, and is hidden. False keeps the button: a list whose OK asks
    // for a restart, or a button that is not a save (Skip, Restore Default)
    property bool applyOnSelect: true
    property Item comboField: null
    visible: !(modern && applyOnSelect && comboField)
    // The OK button right after a settings field or list that knows its saved value (savedText or
    // savedIndex): enabled only while the field holds a change. Modern look: then filled
    // with the accent – the change waits for this tap; grey, its label is fainter than of other
    // disabled buttons
    property Item savedField: null
    // The same for an OK whose row enables it only on a change itself (App Language after its list)
    property bool saveGated: false
    readonly property bool gated: savedField !== null || saveGated
    readonly property bool savePending: modern && gated && enabled
    // After the list's own onActivated (it sets displayText, which the OK handler reads)
    function applySelection() {
        if (control.modern && control.applyOnSelect)
            control.clicked()
    }
    Component.onCompleted: {
        if (!parent)
            return
        const siblings = parent.children
        let i = 0
        while (i < siblings.length && siblings[i] !== control)
            ++i
        const before = i > 0 ? siblings[i - 1] : null
        if (before && before.activated !== undefined && before.popup !== undefined) {
            comboField = before
            before.activated.connect(function () { Qt.callLater(control.applySelection) })
        }
        if (before && before.dirty !== undefined
                && (before.savedText !== undefined || before.savedIndex !== undefined))
            savedField = before
    }

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    topInset: 6
    bottomInset: 6
    padding: 12
    horizontalPadding: modern ? padding + 6 : padding - 4
    spacing: 6

    // The font of the label. Classic look: the button font as it is, as the stock Button
    // draws it. Modern look: the same font in DemiBold and mixed case. Not written into
    // font itself: in Qt 5 a write of one attribute keeps the size the button inherits at
    // that moment, and a button made at another text size kept it after the size or the
    // look changed. A weight set on an instance (font.bold) still wins
    readonly property font labelFont: !modern ? font : Qt.font({
        family: font.family, pixelSize: font.pixelSize, italic: font.italic,
        letterSpacing: font.letterSpacing, hintingPreference: font.hintingPreference,
        weight: font.weight !== Font.Normal ? font.weight : Font.DemiBold,
        capitalization: Font.MixedCase
    })

    icon.width: 24
    icon.height: 24
    icon.color: !enabled ? Material.hintTextColor :
        flat && highlighted ? Material.accentColor :
        highlighted ? Material.primaryHighlightedTextColor : Material.foreground

    Material.elevation: flat ? control.down || control.hovered ? 2 : 0
                             : control.down ? 8 : 2
    Material.background: flat ? "transparent" : undefined

    // Modern look: the label wraps on words instead of eliding, and the button grows to the
    // lines – for a long label in a narrow button at a large text size. Text only: the icon
    // is dropped. The classic look keeps the one-line IconLabel, unless wrapsScaled is set
    property bool wrapsText: false
    // Classic look: wraps as well while the text size is not 100 % (a button of a settings
    // section: one line at 170 % made the whole section wider than the screen)
    property bool wrapsScaled: false
    readonly property color labelColor: control.modern
               ? (!control.enabled ? (control.gated ? window.ui.alpha(window.ui.textMuted, 0.55)
                                                         : window.ui.textMuted)
                  : control.savePending ? window.ui.accentInk
                  : control.danger ? window.ui.danger
                  : control.highlighted && !control.flat ? window.ui.accentInk
                  : control.flat ? window.ui.accent : window.ui.textMain)
               : (!control.enabled ? control.Material.hintTextColor :
                  control.flat && control.highlighted ? control.Material.accentColor :
                  control.highlighted ? control.Material.primaryHighlightedTextColor : control.Material.foreground)

    contentItem: Loader {
        sourceComponent: control.wrapsText && (control.modern || control.wrapsScaled && window.textScale !== 1)
                         ? wrappedLabel : iconLabel
    }

    Component {
        id: wrappedLabel
        Text {
            text: control.text
            font: control.labelFont
            color: control.labelColor
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    Component {
        id: iconLabel
        IconLabel {
            spacing: control.spacing
            mirrored: control.mirrored
            display: control.display

            icon: control.icon
            text: control.text
            font: control.labelFont
            color: control.labelColor
        }
    }

    background: Rectangle {
        implicitWidth: 64
        implicitHeight: control.Material.buttonHeight

        radius: control.modern ? height / 2 : 2
        color: control.modern
               ? (control.flat ? "transparent"
                  : !control.enabled ? window.ui.alpha(window.ui.textMain, 0.08)
                  : control.savePending ? window.ui.accent
                  : control.danger ? window.ui.alpha(window.ui.danger, 0.16)
                  : control.highlighted ? window.ui.accent : window.ui.surfaceHighest)
               : (!control.enabled ? control.Material.buttonDisabledColor :
                  control.highlighted ? control.Material.highlightedButtonColor : control.Material.buttonColor)

        PaddedRectangle {
            y: parent.height - 4
            width: parent.width
            height: 4
            radius: 2
            topPadding: -2
            clip: true
            visible: !control.modern && control.checkable && (!control.highlighted || control.flat)
            color: control.checked && control.enabled ? control.Material.accentColor : control.Material.secondaryTextColor
        }

        // The layer is disabled when the button color is transparent so you can do
        // Material.background: "transparent" and get a proper flat button without needing
        // to set Material.elevation as well. The modern pill has no shadow.
        layer.enabled: !control.modern && control.enabled && control.Material.buttonColor.a > 0
        layer.effect: ElevationEffect {
            elevation: control.Material.elevation
        }

        Ripple {
            clipRadius: control.modern ? height / 2 : 2
            width: parent.width
            height: parent.height
            pressed: control.pressed
            anchor: control
            active: control.down || control.visualFocus || control.hovered
            color: control.flat && control.highlighted ? control.Material.highlightedRippleColor : control.Material.rippleColor
        }
    }
}
