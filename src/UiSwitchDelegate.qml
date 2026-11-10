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

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding,
                             modernTitleHeight)
    // Modern look: the height of the row for the wrapped title (0 otherwise). A property, so a
    // caller that sets an implicitHeight of its own can keep this term
    readonly property real modernTitleHeight: window.ui.modern && wrapsTitle
                                              ? titleProbe.height + titleGap + topPadding + bottomPadding : 0

    // The title wraps (set by a caller with a long title); by default it stays one line, elided
    property bool wrapsTitle: false
    // A title in two lines or more fills the row: a little room so it does not touch the hint below
    readonly property real titleGap: titleProbe.lineCount > 1 ? window.ui.fontPx(8) : 0

    // Modern look: the height of the wrapped title. IconLabel reports the height of one line:
    // at a large text size the second line ran into the hint under the switch
    Text {
        id: titleProbe
        visible: false
        width: Math.max(0, control.availableWidth - control.indicator.width - control.textGap)
        text: control.text
        font: control.font
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        // As many lines as the title
        maximumLineCount: 100
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
    readonly property real textGap: window.ui.modern ? Math.max(spacing, 16) : spacing

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

            clip: visible
            pressed: control.pressed
            anchor: control
            active: control.down || control.visualFocus || control.hovered
            color: control.Material.rippleColor
        }
    }
}
