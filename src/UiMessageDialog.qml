import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import Qt.labs.platform 1.1 as P

// Yes/No question in place of Qt.labs.platform MessageDialog, with the same properties,
// signals, open()/close() and visible. The platform dialog is a native Android window
// whose look follows the system, not the app: the modern look draws a card in the app
// theme instead. Classic look: the native dialog, as before (this popup stays empty).
Popup {
    id: root

    property string title: ""
    property string text: ""
    property string informativeText: ""
    property int buttons: P.MessageDialog.Ok
    // Modern look only (the native dialog keeps its Yes/No): the answer buttons named after
    // the action ("Stop" / "Cancel"), and the positive one in the danger colour when it
    // cannot be undone
    property string yesText: ""
    property string noText: ""
    property string cancelText: ""
    property bool destructive: false
    // The question stands while a flag of the C++ side is set (visible: rootItem.xxxRequested),
    // and only the answer clears it: the back key closes the card without one, the flag stays
    // set and the next request would not open the card again. True: back answers No.
    property bool backAnswersNo: false

    signal yesClicked()
    signal noClicked()
    signal okClicked()
    signal abortClicked()
    signal cancelClicked()
    signal accepted()
    signal rejected()

    readonly property bool modern: window.ui.modern

    // The card stays tappable during its closing animation: one answer per opening
    property bool answered: false
    // Classic look: the native dialog at once, as a MessageDialog opens, not after the enter
    // transition of this (empty) popup
    onAboutToShow: {
        answered = false
        if (!modern)
            nativeDialog.open()
    }

    function answer(button) {
        if (answered)
            return
        answered = true
        if (button === P.MessageDialog.Yes) yesClicked()
        else if (button === P.MessageDialog.No) noClicked()
        else if (button === P.MessageDialog.Ok) okClicked()
        else if (button === P.MessageDialog.Abort) abortClicked()
        else if (button === P.MessageDialog.Cancel) cancelClicked()
        // The handler can ask the next question at once (the queued Garmin workouts): the flag goes
        // false and true again, the card opens anew and aboutToShow clears answered. Closing it then
        // would drop that question (classic: the native dialog closes with the flag still set)
        if (answered)
            close()
        if (button === P.MessageDialog.Yes || button === P.MessageDialog.Ok) accepted()
        else rejected()
    }

    parent: Overlay.overlay
    modal: modern
    dim: modern
    closePolicy: modern ? Popup.CloseOnEscape : Popup.NoAutoClose
    width: modern ? Math.min(parent.width - 48, 440) : 0
    height: modern ? implicitHeight : 0
    x: Math.round((parent.width - width) / 2)
    // Centred between the system bars: the window goes under them (edge to edge). Read only for
    // an open modern card: on iOS getTopPadding() reads a property with no notify signal, and every
    // closed card of main.qml logged that warning at the start (54 lines, classic look too)
    readonly property real insetTop: modern && visible ? window.getTopPadding() : 0
    readonly property real insetBottom: modern && visible ? window.getBottomPadding() : 0
    y: Math.round(insetTop + (parent.height - insetTop - insetBottom - height) / 2)
    padding: 24
    bottomPadding: 12

    // Material dims with a light veil in the dark theme; the card darkens the page instead
    Overlay.modal: Rectangle {
        color: window.ui.scrim
        Behavior on opacity { NumberAnimation { duration: 150 } }
    }

    // Classic: the native dialog goes at once, as a MessageDialog does when its flag clears. In
    // onClosed it stayed when the code cleared the flag and set it again at once (a new clipboard
    // workout): the opening cancels the exit, closed never comes, and open() of a dialog still
    // shown does nothing, so the old text stayed
    onAboutToHide: {
        if (nativeDialog.visible)
            nativeDialog.close()
    }
    // The back key (or the code closing it) answers No once the card has gone: the answer often
    // asks the next question (the queued Garmin workouts), and an opening from aboutToHide is undone
    // by the exit transition that starts right after it (the card stays opened, transparent)
    onClosed: {
        if (backAnswersNo && !answered) {
            answered = true
            noClicked()
        }
    }

    P.MessageDialog {
        id: nativeDialog
        title: root.title
        text: root.text
        informativeText: root.informativeText
        buttons: root.buttons
        onYesClicked: root.answer(P.MessageDialog.Yes)
        onNoClicked: root.answer(P.MessageDialog.No)
        onOkClicked: root.answer(P.MessageDialog.Ok)
        onAbortClicked: root.answer(P.MessageDialog.Abort)
        onCancelClicked: root.answer(P.MessageDialog.Cancel)
        // Back button: no answer, only closes
        onRejected: if (root.visible) root.close()
    }

    // UiFrame, not Rectangle.border: a thin border breaks up on Android
    background: UiFrame {
        visible: root.modern
        radius: 28
        fill: window.ui.surfaceHigh
        stroke: window.ui.outline
        strokeWidth: window.ui.dark ? 0 : 1
    }

    contentItem: Column {
        visible: root.modern
        spacing: 12

        // The title scrolls with the text: with a large font the title alone can take the screen,
        // and the buttons below must stay on it
        Flickable {
            id: bodyFlick
            width: parent.width
            // The text below the title takes up to 55 % of the window; and the whole is no taller
            // than the room the buttons and the system bars leave, so the buttons stay on screen
            height: Math.min(textColumn.implicitHeight,
                             (titleLabel.visible ? titleLabel.height + textColumn.spacing : 0)
                             + root.parent.height * 0.55,
                             Math.max(window.ui.fontBody * 3,
                                      root.parent.height - root.insetTop - root.insetBottom - 48
                                      - root.topPadding - root.bottomPadding
                                      - buttonsRow.height - parent.spacing))
            contentHeight: textColumn.implicitHeight
            clip: contentHeight > height
            boundsBehavior: Flickable.StopAtBounds

            Column {
                id: textColumn
                width: parent.width
                spacing: 12

                Label {
                    id: titleLabel
                    width: parent.width
                    visible: text.length > 0
                    text: root.title.length > 0 ? root.title : root.text
                    // Text.Wrap: a word longer than the line (large font) breaks instead of running out
                    wrapMode: Text.Wrap
                    font.pixelSize: window.ui.fontTitle
                    font.weight: Font.DemiBold
                    color: window.ui.textMain
                }

                Label {
                    id: body
                    width: parent.width
                    visible: text.length > 0
                    // room for the scroll bar of a long text: without it the ends of the lines ran
                    // under the bar. Always kept, so the width does not depend on the height it makes
                    rightPadding: 12
                    text: root.title.length > 0 && root.text.length > 0
                          ? root.text + (root.informativeText.length > 0 ? "\n\n" + root.informativeText : "")
                          : root.informativeText
                    wrapMode: Text.Wrap
                    font.pixelSize: window.ui.fontBody
                    lineHeight: 1.15
                    color: window.ui.textMuted
                }
            }
            // A long text: the bar stays to show there is more below
            ScrollIndicator.vertical: ScrollIndicator {
                active: bodyFlick.contentHeight > bodyFlick.height
            }
        }

        // Flow, not Row: three translated buttons don't fit one row on a phone (the classic popup
        // of the Android notification question does the same). Laid out from the right
        Flow {
            id: buttonsRow
            width: parent.width
            layoutDirection: Qt.RightToLeft
            spacing: 8
            topPadding: 8

            Repeater {
                // The positive one on the right, the negative ones to its left (and on the next
                // row when they don't fit)
                model: [P.MessageDialog.Ok, P.MessageDialog.Yes, P.MessageDialog.No,
                        P.MessageDialog.Abort, P.MessageDialog.Cancel].filter(function(b) { return (root.buttons & b) !== 0 })
                UiButton {
                    readonly property bool positive: modelData === P.MessageDialog.Yes || modelData === P.MessageDialog.Ok
                    text: modelData === P.MessageDialog.Yes ? (root.yesText.length > 0 ? root.yesText : qsTr("Yes"))
                        : modelData === P.MessageDialog.No ? (root.noText.length > 0 ? root.noText : qsTr("No"))
                        : modelData === P.MessageDialog.Ok ? qsTr("OK")
                        : root.cancelText.length > 0 ? root.cancelText : qsTr("Cancel")
                    // the negative ones a grey pill: a pill reads as a button, accent text did not
                    cardSurface: true
                    highlighted: positive
                    danger: positive && root.destructive
                    onClicked: root.answer(modelData)
                }
            }
        }
    }
}
