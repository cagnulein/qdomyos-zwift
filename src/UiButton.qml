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
    // After the list's own onActivated (it sets displayText, which the OK handler reads).
    // Only while the OK is enabled, that is the list holds a change: ComboBox emits activated
    // again when the picked item is the current one, and clicked() from here would save it
    // and show "Setting saved!" though nothing changed
    function applySelection() {
        if (control.modern && control.applyOnSelect && control.enabled)
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
    // Modern: Label Large 14 on a settings page, whose rows are Body Large 16 (ui.labelPx);
    // elsewhere the size it inherits, as before
    readonly property bool inTypeScalePage: modern && window.ui.onTypeScalePage(control)
    readonly property font labelFont: !modern ? font : Qt.font({
        family: font.family, pixelSize: inTypeScalePage ? window.ui.labelPx(font.pixelSize) : font.pixelSize,
        italic: font.italic,
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
    // Modern look: a button on an inverse surface - the textMain fill of UiNotice and of the heart
    // rate sensor bar (HeartRateSuggestion). The accent of the page reads poorly there (violet
    // #B69DF8 on #E4E6EB is about 1.8 : 1), so it takes the accent of the opposite theme, which
    // is made to read on that kind of surface; the tonal pill is the page colour at 12 %.
    // The classic look ignores it
    property bool inverse: false
    readonly property bool modernInverse: modern && inverse
    // Modern look: a tonal pill on a card or a question (surfaceHigh). The surfaceHighest of the pill
    // is about 1.1 : 1 there and the pill is not seen, so it takes textMain at 10 % (12 % in the
    // dark), like the danger pill takes the danger colour at 16 %. The classic look ignores it
    property bool cardSurface: false
    // the accent of the same name in the opposite theme
    readonly property color inverseAccent: {
        const u = window.ui
        if (!modernInverse)
            return u.accent
        const set = u.dark ? u.lightAccents : u.accents
        return set[u.accentName] || set["violet"]
    }
    // the accentInk of the opposite theme
    readonly property color inverseAccentInk: window.ui.dark ? "#FFFFFF" : "#12101A"
    readonly property color labelColor: control.modernInverse
               ? (!control.enabled ? window.ui.alpha(window.ui.bg, 0.5)
                  : control.highlighted && !control.flat ? control.inverseAccentInk
                  : control.flat ? control.inverseAccent : window.ui.bg)
               : control.modern
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
        color: control.modernInverse
               ? (control.flat ? "transparent"
                  : !control.enabled ? window.ui.alpha(window.ui.bg, 0.08)
                  : control.highlighted ? control.inverseAccent : window.ui.alpha(window.ui.bg, 0.12))
               : control.modern
               ? (control.flat ? "transparent"
                  : !control.enabled ? window.ui.alpha(window.ui.textMain, 0.08)
                  : control.savePending ? window.ui.accent
                  : control.danger ? window.ui.alpha(window.ui.danger, 0.16)
                  : control.highlighted ? window.ui.accent
                  : control.cardSurface ? window.ui.alpha(window.ui.textMain, window.ui.dark ? 0.12 : 0.10)
                  : window.ui.surfaceHighest)
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
            // Modern: no wave, only the pressed tint. Qt 5.15 clips the wave by the bounding
            // rectangle (qquickmaterialripple.cpp, clipNode->setRadius is commented out, QTBUG-51894;
            // fixed in Qt 6.5) - clipRadius rounds the tint, not the wave, and a pill showed square
            // corners
            pressed: control.modern ? false : control.pressed
            anchor: control
            active: control.down || control.visualFocus || control.hovered
            // Modern: the Material 3 state layer - the label colour at 10 %, so a dark label darkens
            // the pill and a light one lightens it on any fill (the Qt 5 ripple is black 6 % or white
            // 12.5 % by the theme alone, whatever the pill)
            color: control.modern ? window.ui.alpha(control.labelColor, 0.10)
                 : control.flat && control.highlighted ? control.Material.highlightedRippleColor : control.Material.rippleColor
        }
    }
}
