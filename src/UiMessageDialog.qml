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
    y: Math.round((parent.height - height) / 2)
    padding: 24
    bottomPadding: 12

    // Material dims with a light veil in the dark theme; the card darkens the page instead
    Overlay.modal: Rectangle {
        color: window.ui.scrim
        Behavior on opacity { NumberAnimation { duration: 150 } }
    }

    onClosed: {
        if (nativeDialog.visible)
            nativeDialog.close()
        if (backAnswersNo && !answered)
            noClicked()
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

        Label {
            width: parent.width
            visible: text.length > 0
            text: root.title.length > 0 ? root.title : root.text
            wrapMode: Text.WordWrap
            font.pixelSize: window.ui.fontTitle
            font.weight: Font.DemiBold
            color: window.ui.textMain
        }

        Flickable {
            id: bodyFlick
            width: parent.width
            height: Math.min(body.implicitHeight, root.parent.height * 0.55)
            visible: body.text.length > 0
            contentHeight: body.implicitHeight
            clip: contentHeight > height
            boundsBehavior: Flickable.StopAtBounds
            Label {
                id: body
                width: parent.width
                text: root.title.length > 0 && root.text.length > 0
                      ? root.text + (root.informativeText.length > 0 ? "\n\n" + root.informativeText : "")
                      : root.informativeText
                wrapMode: Text.WordWrap
                font.pixelSize: window.ui.fontBody
                lineHeight: 1.15
                color: window.ui.textMuted
            }
            // A long text: the bar stays to show there is more below
            ScrollIndicator.vertical: ScrollIndicator {
                active: bodyFlick.contentHeight > bodyFlick.height
            }
        }

        Row {
            anchors.right: parent.right
            spacing: 4
            topPadding: 8

            Repeater {
                // Order of the buttons: negative ones first, the positive one on the right
                model: [P.MessageDialog.Cancel, P.MessageDialog.Abort, P.MessageDialog.No,
                        P.MessageDialog.Yes, P.MessageDialog.Ok].filter(function(b) { return (root.buttons & b) !== 0 })
                UiButton {
                    readonly property bool positive: modelData === P.MessageDialog.Yes || modelData === P.MessageDialog.Ok
                    text: modelData === P.MessageDialog.Yes ? (root.yesText.length > 0 ? root.yesText : qsTr("Yes"))
                        : modelData === P.MessageDialog.No ? (root.noText.length > 0 ? root.noText : qsTr("No"))
                        : modelData === P.MessageDialog.Ok ? qsTr("OK")
                        : qsTr("Cancel")
                    flat: !positive
                    highlighted: positive
                    danger: positive && root.destructive
                    onClicked: root.answer(modelData)
                }
            }
        }
    }
}
