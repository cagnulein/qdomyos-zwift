import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12

// Only appears on the Home page if a device is missing or no activity metrics
// have arrived. Never blocks or changes the training state.
Item {
    id: support
    width: 56
    height: 56

    property bool activeOnHome: false
    property int missingSeconds: 0
    property int idleSeconds: 0
    property bool showHint: false
    property string reason: ""

    onActiveOnHomeChanged: {
        if (!activeOnHome) {
            missingSeconds = 0
            idleSeconds = 0
            showHint = false
        }
    }

    Timer {
        interval: 1000
        repeat: true
        running: support.activeOnHome
        onTriggered: {
            if (!rootItem.supportDeviceConnected()) {
                support.missingSeconds++
                support.idleSeconds = 0
                support.showHint = support.missingSeconds >= 15
                support.reason = "No device connected"
            } else if (rootItem.supportMetricsActive()) {
                support.missingSeconds = 0
                support.idleSeconds = 0
                support.showHint = false
                support.reason = ""
            } else {
                support.missingSeconds = 0
                support.idleSeconds++
                support.showHint = support.idleSeconds >= 30
                support.reason = "Speed and power at zero"
            }
        }
    }

    RoundButton {
        id: helpButton
        anchors.fill: parent
        visible: support.activeOnHome && support.showHint
        text: "?"
        font.pixelSize: 24
        font.bold: true
        onClicked: supportDialog.open()
        Accessible.role: Accessible.Button
        Accessible.name: qsTr("Contact QZ support")
        Accessible.description: qsTr("Prepare a support email with diagnostics")
    }

    Dialog {
        id: supportDialog
        parent: Overlay.overlay
        title: qsTr("Contact QZ support")
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        width: Math.min(parent.width - 24, 420)
        x: Math.round((parent.width - width) / 2)
        y: Math.max(12, Math.round((parent.height - height) / 2))
        padding: 16

        contentItem: Column {
            spacing: 10

            Label {
                width: parent.width
                wrapMode: Text.WordWrap
                text: qsTr("Tell us what is not working. QZ will add your device and app information.")
            }

            ScrollView {
                width: parent.width
                height: 112
                clip: true

                TextArea {
                    id: issueDescription
                    placeholderText: qsTr("Describe the problem...")
                    wrapMode: TextEdit.Wrap
                    selectByMouse: true
                }
            }

            CheckBox {
                id: includeDebugLog
                width: parent.width
                text: qsTr("Attach the debug log if available")
                checked: true
                Accessible.name: text
            }

            Label {
                width: parent.width
                wrapMode: Text.WordWrap
                font.pixelSize: 12
                opacity: 0.8
                text: qsTr("You can review or edit the email before sending it. Nothing is sent automatically.")
            }

            Row {
                anchors.right: parent.right
                spacing: 8
                Button {
                    text: qsTr("Cancel")
                    onClicked: supportDialog.close()
                }
                Button {
                    text: qsTr("Open email")
                    onClicked: {
                        if (rootItem.composeSupportEmail(issueDescription.text, support.reason,
                                                         includeDebugLog.checked)) {
                            supportDialog.close()
                        }
                    }
                }
            }
        }
    }
}
