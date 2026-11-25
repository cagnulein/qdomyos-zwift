import QtQuick 2.12
import QtQuick.Templates 2.12 as T
import QtQuick.Controls 2.12
import QtQuick.Controls.impl 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Material.impl 2.12

// SwitchDelegate of the settings pages: the Qt 5.15 Material SwitchDelegate, copied as is
// (qtquickcontrols2 5.15, src/imports/controls/material/SwitchDelegate.qml), with the
// indicator swapped for UiSwitchIndicator - the stock one in the classic look, the
// Material 3 switch in the modern look.
// Screen reader: as with the stock one, the caller gives it its press action (Qt 5.15 gives a
// SwitchDelegate none), e.g. Accessible.onPressAction: { toggle(); clicked(); } - not here, both
// would run and turn it twice
T.SwitchDelegate {
    id: control

    // The look, false without the app window: IndicatorOnlySwitch is built on this one, and the
    // layout test of the settings labels (tst/main.cpp) loads it in a bare QQmlEngine
    readonly property bool modern: typeof window !== "undefined" && !!window && !!window.ui && window.ui.modern

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding,
                             modernTitleHeight)
    // Modern look: the height of the row for the wrapped title (0 otherwise). A property, so
    // IndicatorOnlySwitch, which sets an implicitHeight of its own, keeps this term
    // A title in two lines or more makes the row higher than the background (+ 1), as the term of
    // IndicatorOnlySwitch for a bigger text: the label places its text again only when the row
    // height changes, and the switches of the settings subpages have no vertical padding - two
    // lines at 100 % (46) stayed under the background (48) and the text could stay half a line
    // low
    readonly property real modernTitleHeight: modern && wrapsTitle
                                              ? Math.max(titleProbe.height + titleGap + topPadding + bottomPadding,
                                                         titleProbe.lineCount > 1
                                                         ? implicitBackgroundHeight + topInset + bottomInset + 1 : 0)
                                              : 0

    // The title wraps (set by IndicatorOnlySwitch: two lines in the classic look, all of it in
    // the modern one); here it stays one line, elided - but on a settings page of the modern look
    // (pageWrap)
    property bool wrapsTitle: pageWrap

    // On a settings page of the modern look (typeScalePage: Tiles Options, the gear tables) the
    // whole title on as many lines as it takes, as IndicatorOnlySwitch: with Body
    // Large at a large text size a long title was cut with an ellipsis. IndicatorOnlySwitch wraps
    // its title itself and sets this false; the classic and the other pages - one line, as before
    property bool pageWrap: inSettingsPage
    // A settings page of the modern look: the row is a card of its section
    readonly property bool inSettingsPage: modern && window.ui.onTypeScalePage(control)
    // The text of the IconLabel (Qt 5.15 gives it no wrapMode of its own): found as
    // IndicatorOnlySwitch.configureTextItem() does
    function setTitleWrap(item, wrap) {
        if (!item)
            return false
        if (item.wrapMode !== undefined && item.maximumLineCount !== undefined) {
            item.wrapMode = wrap ? Text.WrapAtWordBoundaryOrAnywhere : Text.NoWrap
            item.lineHeight = Qt.binding(function () { return control.titleLeading })
            return true
        }
        for (var i = 0; i < item.children.length; ++i)
            if (setTitleWrap(item.children[i], wrap))
                return true
        return false
    }
    function applyPageWrap() {
        if (pageWrap || titleWrapSet)
            titleWrapSet = setTitleWrap(contentItem, pageWrap) && pageWrap
    }
    // set here once: back to one line when the page leaves the modern look
    property bool titleWrapSet: false
    onPageWrapChanged: Qt.callLater(applyPageWrap)
    onContentItemChanged: Qt.callLater(applyPageWrap)
    // an IconLabel makes its text item again when the title was empty (as IndicatorOnlySwitch)
    onTextChanged: Qt.callLater(applyPageWrap)
    Component.onCompleted: Qt.callLater(applyPageWrap)
    // A title in two lines or more fills the row: a little room so it does not touch the hint below
    readonly property real titleGap: modern && titleProbe.lineCount > 1 ? window.ui.fontPx(8) : 0
    // A wrapped title of the modern look: the lines closer, x1.15 of the font (~x1.28 by default).
    // Set on the text of the label (setTitleWrap,
    // IndicatorOnlySwitch.configureTextItem) and on titleProbe, so the row is as high as the title.
    // One line stays as it is: Qt 5.15.0 cuts the only line too (qquicktext.cpp:1264), its height
    // leaves out the offset of the smaller lines - QTBUG-88229, fixed in Qt 6.1 only
    readonly property real titleLeading: modern && wrapsTitle && titleProbe.lineCount > 1 ? 0.9 : 1

    // Modern look: the height of the wrapped title. IconLabel reports the height of one line:
    // at a large text size the second line ran into the hint under the switch
    Text {
        id: titleProbe
        visible: false
        width: Math.max(0, control.availableWidth - control.indicator.width - control.textGap)
        text: control.text
        font: control.font
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        // As many lines as the title (IndicatorOnlySwitch lifts its limit in the modern look)
        maximumLineCount: 100
        // the line count follows the width only: no loop with titleLeading
        lineHeight: control.titleLeading
    }

    padding: 16
    verticalPadding: Material.switchDelegateVerticalPadding
    spacing: 16

    icon.width: 24
    icon.height: 24
    icon.color: enabled ? Material.foreground : Material.hintTextColor

    indicator: UiSwitchIndicator {
        x: control.text ? (control.mirrored ? control.leftPadding : control.width - width - control.rightPadding) : control.leftPadding + (control.availableWidth - width) / 2
        y: control.topPadding + (control.availableHeight - height) / 2
        control: control
    }

    // Room between the title and the switch. The settings pages set spacing: 0 on most
    // switches, and a long title ran into the modern switch; the modern look keeps at least 16
    readonly property real textGap: modern ? Math.max(spacing, 16) : spacing

    contentItem: IconLabel {
        leftPadding: !control.mirrored ? 0 : control.indicator.width + control.textGap
        rightPadding: control.mirrored ? 0 : control.indicator.width + control.textGap

        spacing: control.spacing
        mirrored: control.mirrored
        display: control.display
        alignment: control.display === IconLabel.IconOnly || control.display === IconLabel.TextUnderIcon ? Qt.AlignCenter : Qt.AlignLeft

        icon: control.icon
        text: control.text
        font: control.font
        color: control.enabled ? control.Material.foreground : control.Material.hintTextColor
    }

    background: Rectangle {
        implicitHeight: control.Material.delegateHeight

        color: control.highlighted ? control.Material.listHighlightColor : "transparent"

        Ripple {
            width: parent.width
            height: parent.height
            // On a settings page of the modern look no press wave over the row: it filled the
            // rounded card with a square block; the switch shows the press
            // itself (UiSwitchIndicator, a larger handle), and the switches of the main settings
            // page (IndicatorOnlySwitch, its MouseArea takes the press) never had it. The focus of
            // a keyboard is still shown
            visible: !control.inSettingsPage || control.visualFocus

            clip: visible
            pressed: control.pressed
            anchor: control
            active: control.down || control.visualFocus || control.hovered
            color: control.Material.rippleColor
        }
    }
}
