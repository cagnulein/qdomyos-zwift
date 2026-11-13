import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtGraphicalEffects 1.12

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
    // The button is on the screen: the modern home grid keeps room for it under the last row
    readonly property bool shown: activeOnHome && showHint
    // Modern look: the button, the window and its controls in the theme colours; the classic
    // look stays the Material one of the upstream
    readonly property bool modern: window.ui.modern

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
                support.showHint = support.missingSeconds >= 5
                support.reason = qsTr("No device connected")
            } else if (rootItem.supportMetricsActive()) {
                support.missingSeconds = 0
                support.idleSeconds = 0
                support.showHint = false
                support.reason = ""
            } else {
                support.missingSeconds = 0
                support.idleSeconds++
                support.showHint = support.idleSeconds >= 10
                support.reason = qsTr("Speed and power at zero")
            }
        }
    }

    RoundButton {
        id: helpButton
        anchors.fill: parent
        visible: !support.modern && support.shown
        text: "?"
        font.pixelSize: 24
        font.bold: true
        onClicked: supportDialog.open()
        Accessible.role: Accessible.Button
        Accessible.name: qsTr("Contact QZ support")
        Accessible.description: qsTr("Prepare a support email with diagnostics")
    }

    // Modern look: the small floating action button of Material 3 - 40 dp, corners 12, an accent
    // tinted container with the icon in the accent and a shadow, as it floats over the tiles -
    // in the 56 dp the finger can hit
    AbstractButton {
        id: modernHelpButton
        anchors.fill: parent
        visible: support.modern && support.shown
        onClicked: supportDialog.open()
        background: Rectangle {
            anchors.centerIn: parent
            width: 40
            height: 40
            radius: 12
            color: Qt.tint(window.ui.surface, window.ui.alpha(window.ui.accent,
                                                             modernHelpButton.down ? 0.28 : 0.16))
            layer.enabled: modernHelpButton.visible
            layer.effect: DropShadow {
                transparentBorder: true
                horizontalOffset: 0
                verticalOffset: 2
                radius: 8
                samples: 17
                color: Qt.rgba(0, 0, 0, window.ui.dark ? 0.5 : 0.25)
            }
        }
        contentItem: Item {
            UiIcon {
                anchors.centerIn: parent
                width: 24
                height: 24
                name: "help"
                color: window.ui.accent
            }
        }
        Accessible.role: Accessible.Button
        Accessible.name: qsTr("Contact QZ support")
        Accessible.description: qsTr("Prepare a support email with diagnostics")
        Accessible.focusable: true
        Accessible.onPressAction: supportDialog.open()
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
            color: support.modern ? window.ui.bg
                                  : (Material.theme === Material.Dark ? "#303030" : "#fafafa")
        }

        header: Item {
            implicitHeight: 62 + window.getTopPadding()
            // Modern look: no lines, the page colour runs from the title to the buttons
            Rectangle {
                visible: !support.modern
                height: 1
                anchors.bottom: parent.bottom
                width: parent.width
                color: Material.theme === Material.Dark ? "#555555" : "#dddddd"
            }
            Label {
                text: qsTr("QZ Support")
                font.pixelSize: support.modern ? window.ui.fontHeadline : 20
                font.weight: support.modern ? Font.DemiBold : Font.Bold
                color: support.modern ? window.ui.textMain : Material.foreground
                anchors.left: parent.left
                anchors.leftMargin: 16 + window.getLeftPadding()
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: window.getTopPadding() / 2
            }
            // Even when an input method cannot report its keyboard rectangle,
            // this stays above the keyboard and lets the rider dismiss it.
            ToolButton {
                id: doneTypingButton
                text: qsTr("Done")
                visible: Qt.inputMethod.visible || issueDescription.activeFocus
                anchors.right: closeDialogButton.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: window.getTopPadding() / 2
                Accessible.name: qsTr("Hide keyboard")
                onClicked: {
                    forceActiveFocus()
                    Qt.inputMethod.hide()
                }
            }
            ToolButton {
                id: closeDialogButton
                // Modern look: the close icon of the icon font, as the other icons there; the system
                // font of an older Android may have no U+2715 (the Android emulator of CI draws a box)
                text: support.modern ? "" : "\u2715"
                UiIcon {
                    visible: support.modern
                    anchors.centerIn: parent
                    width: 24
                    height: 24
                    name: "close"
                    color: window.ui.textMain
                }
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
                    color: support.modern ? window.ui.textMain : Material.foreground
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

                    // Classic: the paddings, colours and underline of the Qt 5.15 Material
                    // TextArea as is (qtquickcontrols2 5.15, material/TextArea.qml). Modern: a
                    // filled field with rounded corners like UiTextField, the accent ring shows
                    // the focus. leftPadding/rightPadding unset are the padding of the control
                    topPadding: support.modern ? 12 : 8
                    bottomPadding: support.modern ? 12 : 16
                    leftPadding: support.modern ? 14 : padding
                    rightPadding: support.modern ? 14 : padding
                    color: support.modern ? window.ui.textMain
                                          : (enabled ? Material.foreground : Material.hintTextColor)
                    placeholderTextColor: support.modern ? window.ui.textMuted : Material.hintTextColor
                    background: Item {
                        implicitWidth: 120
                        Rectangle {
                            visible: !support.modern
                            y: parent.height - height - issueDescription.bottomPadding / 2
                            width: parent.width
                            height: issueDescription.activeFocus ? 2 : 1
                            color: issueDescription.activeFocus ? issueDescription.Material.accentColor
                                                                : issueDescription.Material.hintTextColor
                        }
                        UiFrame {
                            visible: support.modern
                            anchors.fill: parent
                            radius: 12
                            fill: window.ui.surfaceHighest
                            stroke: window.ui.accent
                            strokeWidth: issueDescription.activeFocus ? 2 : 0
                        }
                    }
                }

                CheckBox {
                    id: includeDebugLog
                    visible: !support.modern
                    width: parent.width
                    text: qsTr("Attach debug log if available")
                    // the debug log QZ writes by itself for a new user is there to be sent
                    checked: window.rootReady && rootItem.firstSetupLogActive()
                    Accessible.name: text
                }

                // Modern look: the switch of the settings pages instead of the square box; the
                // title wraps on as many lines as it takes
                IndicatorOnlySwitch {
                    id: includeDebugLogSwitch
                    visible: support.modern
                    width: parent.width
                    leftPadding: 0
                    rightPadding: 0
                    text: qsTr("Attach debug log if available")
                    checked: window.rootReady && rootItem.firstSetupLogActive()
                    Accessible.name: text
                    // Qt 5.15: the double tap of the screen reader sends no clicked() to a SwitchDelegate
                    Accessible.onPressAction: toggle()
                    // The whole row toggles, as the check box of the classic look did with its text;
                    // IndicatorOnlySwitch alone takes only a tap on the switch (settings pages)
                    MouseArea {
                        anchors.fill: parent
                        onClicked: includeDebugLogSwitch.toggle()
                    }
                }

                Label {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    font.pixelSize: support.modern ? window.ui.fontCaption : 12
                    opacity: support.modern ? 1 : 0.8
                    color: support.modern ? window.ui.textMuted : Material.foreground
                    text: qsTr("You can review or edit the email before sending it. Nothing is sent automatically.")
                }
            }
        }

        // This is deliberately a real Dialog footer, not another child of the
        // Flickable: Cancel and Open email must remain visible while typing.
        footer: Item {
            // Modern look: taller with the buttons at a large text size, so they stay below the line
            // of the form
            implicitHeight: (support.modern ? Math.max(64, footerButtons.height + 16) : 64)
                            + (Qt.inputMethod.visible ? 0 : window.getBottomPadding())
            Rectangle {
                visible: !support.modern
                width: parent.width
                height: 1
                anchors.top: parent.top
                color: Material.theme === Material.Dark ? "#555555" : "#dddddd"
            }
            Row {
                id: footerButtons
                anchors.right: parent.right
                anchors.rightMargin: 16 + window.getRightPadding()
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 8 + (Qt.inputMethod.visible ? 0 : window.getBottomPadding())
                spacing: 8
                // Modern look: the two buttons share the width, easy to reach with the thumb;
                // classic: as wide as their labels, at the right
                readonly property real halfWidth: (parent.width - 32 - window.getLeftPadding()
                                                   - window.getRightPadding() - spacing) / 2
                UiButton {
                    width: support.modern ? footerButtons.halfWidth : implicitWidth
                    text: qsTr("Cancel")
                    onClicked: {
                        Qt.inputMethod.hide()
                        supportDialog.close()
                    }
                }
                UiButton {
                    width: support.modern ? footerButtons.halfWidth : implicitWidth
                    // Modern look: the action of the window, in the accent
                    highlighted: support.modern
                    text: qsTr("Open email")
                    onClicked: {
                        // Close the software keyboard before opening the native
                        // iOS mail composer / Android share chooser.
                        Qt.inputMethod.hide()
                        var attachLog = support.modern ? includeDebugLogSwitch.checked : includeDebugLog.checked
                        if (rootItem.composeSupportEmail(issueDescription.text, support.reason,
                                                         attachLog)) {
                            supportDialog.close()
                        }
                    }
                }
            }
        }
    }
}
