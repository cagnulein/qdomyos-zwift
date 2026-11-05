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
        // Do not dismiss the report accidentally by tapping beside it.
        closePolicy: Popup.CloseOnEscape
        padding: 0
        margins: 0

        readonly property bool mobile: Qt.platform.os === "ios" || Qt.platform.os === "android"
        // Qt reports the software keyboard in window coordinates. On Android
        // adjustResize may already have reduced the overlay: clamp to its height.
        readonly property real visibleBottom: {
            var available = parent ? parent.height : 0
            if (!mobile || !Qt.inputMethod.visible)
                return available
            var keyboard = Qt.inputMethod.keyboardRectangle
            if (keyboard.height <= 0)
                return available
            var top = keyboard.y > 0 ? keyboard.y : available - keyboard.height
            return Math.max(0, Math.min(available, top))
        }

        x: mobile ? 0 : Math.round((parent.width - width) / 2)
        y: mobile ? 0 : Math.round((parent.height - height) / 2)
        width: mobile ? parent.width : Math.min(parent.width - 24, 480)
        // Full screen before typing; the fixed footer moves above the keyboard
        // while the body becomes scrollable on compact phones and in landscape.
        height: mobile ? visibleBottom : Math.min(parent.height - 24, 560)

        background: Rectangle {
            color: Material.theme === Material.Dark ? "#303030" : "#fafafa"
        }

        header: Item {
            implicitHeight: 62 + window.getTopPadding()
            Rectangle {
                height: 1
                anchors.bottom: parent.bottom
                width: parent.width
                color: Material.theme === Material.Dark ? "#555555" : "#dddddd"
            }
            Label {
                text: supportDialog.title
                font.pixelSize: 20
                font.bold: true
                anchors.left: parent.left
                anchors.leftMargin: 16 + window.getLeftPadding()
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: window.getTopPadding() / 2
            }
            ToolButton {
                text: "\u2715"
                anchors.right: parent.right
                anchors.rightMargin: 8 + window.getRightPadding()
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: window.getTopPadding() / 2
                Accessible.name: qsTr("Close support request")
                onClicked: {
                    Qt.inputMethod.hide()
                    supportDialog.close()
                }
            }
        }

        contentItem: Flickable {
            id: reportScroll
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            contentWidth: width
            contentHeight: reportForm.implicitHeight + 32
            flickableDirection: Flickable.VerticalFlick
            interactive: contentHeight > height

            Column {
                id: reportForm
                x: 16 + window.getLeftPadding()
                y: 16
                width: Math.max(0, reportScroll.width - 32 - window.getLeftPadding() - window.getRightPadding())
                spacing: 14

                Label {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    text: qsTr("Tell us what is not working. QZ will add your device and app information.")
                }

                TextArea {
                    id: issueDescription
                    width: parent.width
                    height: 150
                    placeholderText: qsTr("Describe the problem...")
                    wrapMode: TextEdit.Wrap
                    selectByMouse: true
                    Accessible.name: qsTr("Describe the problem")
                }

                CheckBox {
                    id: includeDebugLog
                    width: parent.width
                    text: qsTr("Attach debug log if available")
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
            }
        }

        // This is deliberately a real Dialog footer, not another child of the
        // Flickable: Cancel and Open email must remain visible while typing.
        footer: Item {
            implicitHeight: 64 + (Qt.inputMethod.visible ? 0 : window.getBottomPadding())
            Rectangle {
                width: parent.width
                height: 1
                anchors.top: parent.top
                color: Material.theme === Material.Dark ? "#555555" : "#dddddd"
            }
            Row {
                anchors.right: parent.right
                anchors.rightMargin: 16 + window.getRightPadding()
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 8 + (Qt.inputMethod.visible ? 0 : window.getBottomPadding())
                spacing: 8
                Button {
                    text: qsTr("Cancel")
                    onClicked: {
                        Qt.inputMethod.hide()
                        supportDialog.close()
                    }
                }
                Button {
                    text: qsTr("Open email")
                    onClicked: {
                        // Close the software keyboard before opening the native
                        // iOS mail composer / Android share chooser.
                        Qt.inputMethod.hide()
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
