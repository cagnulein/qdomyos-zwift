import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12

// Short notice ("New lap started!", "The tiles are locked now"): the caller opens it and
// most callers close it with their own timer. Classic look: the old 380x60 popup in the
// middle of the page, modal, as before (with a bigger text it wraps and grows). Modern look: a snackbar at the bottom - no dimming,
// the page under it stays usable, a long text wraps instead of running off the card.
UiPopup {
    id: control

    property string text: ""
    readonly property bool snackbar: modern

    // Modern: the snackbar goes by itself. Some notices ("Saved! Check your private folder")
    // have no timer of their caller: the classic popup is modal and went with a tap anywhere,
    // a tap on the page under the snackbar reaches the page instead
    Timer {
        running: control.snackbar && control.opened
        interval: control.text.length > 60 ? 6000 : 4000
        onTriggered: control.close()
    }

    inverse: snackbar
    parent: Overlay.overlay

    x: Math.round((parent.width - width) / 2)
    // Above the Android navigation bar: the window is drawn under it from API 31
    y: snackbar ? parent.height - height - 24 - window.getBottomPadding() : Math.round((parent.height - height) / 2)
    width: snackbar ? Math.min(parent.width - 32, 560) : window.textScale !== 1 ? Math.min(380, parent.width - 32) : 380
    height: snackbar ? Math.max(52, noticeLabel.implicitHeight + topPadding + bottomPadding)
                     : window.textScale !== 1 ? Math.max(60, noticeLabel.height + topPadding + bottomPadding) : 60
    leftPadding: snackbar ? 20 : padding
    rightPadding: snackbar ? 20 : padding
    topPadding: snackbar ? 14 : padding
    bottomPadding: snackbar ? 14 : padding
    modal: !snackbar
    focus: !snackbar
    palette.text: "white"
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    enter: Transition {
        NumberAnimation { property: "opacity"; from: 0.0; to: 1.0 }
    }
    exit: Transition {
        NumberAnimation { property: "opacity"; from: 1.0; to: 0.0 }
    }

    Column {
        anchors.horizontalCenter: control.snackbar ? undefined : parent.horizontalCenter
        anchors.verticalCenter: control.snackbar ? parent.verticalCenter : undefined
        width: control.snackbar ? control.availableWidth : implicitWidth

        Label {
            id: noticeLabel
            anchors.horizontalCenter: control.snackbar ? undefined : parent.horizontalCenter
            width: control.snackbar || window.textScale !== 1 ? control.availableWidth : implicitWidth
            text: control.text
            wrapMode: control.snackbar ? Text.WordWrap : window.labelWrap
            color: control.snackbar ? window.ui.bg : Material.foreground
            font.pixelSize: control.snackbar ? window.ui.fontLabel : control.font.pixelSize
        }
    }
}
