import QtQuick 2.12
// Binding.restoreMode below: the classic look keeps the inherited font and colour
import QtQml 2.15
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12

// A heart rate sensor QZ found nearby while none is chosen (homeform::heartRateSensorSuggestions): after a tap on the
// heart rate tile or the Bluetooth icon, or by the background search of the first 5 minutes when it is on.
// A bar at the bottom rather than a dialog: it covers only the bottom of the page, so it may show during a workout.
// Several sensors: "Choose" opens the list. It stays until answered or the heart rate comes: on a phone left on the
// handlebar the user is busy with the sensor, and a bar that went away by itself was missed.
// Classic look: a Rectangle in the Material colours with an accent outline (UiFrame draws it so); modern look: the
// inverse colours of the other bottom bars (UiNotice) - light on a dark page, dark on a light one: a card in the page
// colours hardly stood out from the tiles under it (1.11 : 1 in the dark theme).
UiFrame {
    id: bar

    // rootItem is set after main.qml is loaded
    readonly property var sensors: window.rootReady ? rootItem.heartRateSensorSuggestions : []

    visible: sensors.length > 0
    // under the toasts (z: Infinity), above the pages
    z: 1000
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    // clear of the system bars and the cutout in any orientation, as the pages. Modern look: 16 from the edges, the
    // margin of the pages, and the padding of the bottom bars (UiNotice: 20 at the sides, 14 at the top and bottom)
    anchors.leftMargin: window.getLeftPadding() + (window.ui.modern ? 16 : 10)
    anchors.rightMargin: window.getRightPadding() + (window.ui.modern ? 16 : 10)
    anchors.bottomMargin: window.getBottomPadding() + (window.ui.modern ? 16 : 10)
    height: content.implicitHeight + content.anchors.topMargin + content.anchors.bottomMargin
    radius: window.ui.modern ? 14 : 8
    fill: window.ui.modern ? window.ui.textMain : Material.background
    stroke: window.ui.modern ? window.ui.outline : Material.accent
    strokeWidth: window.ui.modern ? 0 : 1

    onSensorsChanged: {
        if (sensors.length === 0)
            list.close()
    }

    // the bar takes the taps, not the tiles under it
    MouseArea {
        anchors.fill: parent
        Accessible.ignored: true
    }

    Column {
        id: content
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 12
        anchors.leftMargin: window.ui.modern ? 20 : 12
        anchors.rightMargin: window.ui.modern ? 20 : 12
        anchors.topMargin: window.ui.modern ? 14 : 12
        anchors.bottomMargin: window.ui.modern ? 14 : 12
        spacing: 8

        // Modern look: the text of UiNotice (fontLabel, regular); classic: the bold app font, as before
        Label {
            width: parent.width
            text: bar.sensors.length === 1 ? qsTr("Heart rate sensor found: %1").arg(bar.sensors[0].name)
                                           : qsTr("Heart rate sensors found: %1").arg(bar.sensors.length)
            color: window.ui.modern ? window.ui.bg : window.ui.ink(Material.foreground)
            font.weight: window.ui.modern ? Font.Normal : Font.Bold
            Binding on font.pixelSize { when: window.ui.modern; value: window.ui.fontLabel; restoreMode: Binding.RestoreBindingOrValue }
            wrapMode: Text.WordWrap
        }

        // Flow, not Row: three buttons don't fit one row on a phone once translated
        Flow {
            width: parent.width
            spacing: 8
            layoutDirection: Qt.RightToLeft

            // Modern look: the one action filled with the accent, the others tonal pills
            UiButton {
                highlighted: window.ui.modern
                inverse: true
                text: bar.sensors.length === 1 ? qsTr("Connect") : qsTr("Choose")
                onClicked: {
                    if (bar.sensors.length === 1) {
                        rootItem.heartRateSensorSuggestionConnect(bar.sensors[0].name, bar.sensors[0].address)
                    } else {
                        list.open()
                    }
                }
            }

            UiButton {
                inverse: true
                text: qsTr("Not now")
                onClicked: rootItem.heartRateSensorSuggestionDismiss(false)
            }

            // turns the background search off: not on a bar the user asked for with a tap
            UiButton {
                inverse: true
                visible: window.rootReady && !rootItem.heartRateSensorOfferOnTap
                text: qsTr("Don't ask again")
                onClicked: rootItem.heartRateSensorSuggestionDismiss(true)
            }
        }
    }

    UiPopup {
        id: list
        parent: Overlay.overlay
        modal: true
        focus: true
        // Modern look: the size, padding and corners of the question cards (UiMessageDialog)
        width: window.ui.modern ? Math.min(window.width - 48, 440) : Math.min(window.width - 40, 460)
        height: Math.min(window.height - 60, listColumn.implicitHeight + topPadding + bottomPadding)
        x: Math.round((parent.width - width) / 2)
        y: Math.round((parent.height - height) / 2)
        padding: window.ui.modern ? 24 : 12
        // closed without a choice: the bar is still there
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        // classic: the bar's look with the accent outline as before; modern: the question card
        background: UiFrame {
            radius: window.ui.modern ? 28 : 8
            fill: window.ui.modern ? window.ui.surfaceHigh : Material.background
            stroke: window.ui.modern ? window.ui.outline : Material.accent
            strokeWidth: window.ui.modern && window.ui.dark ? 0 : 1
        }

        // Modern look: the signal as 4 bars (TalkBack reads it in words); the offer starts at -80 dBm
        // (bluetooth::heartRateSensorSuggestMinRssi), so -74, -67 and -60 split it in steps of about 7 dB. 0: the platform did not give the signal
        function signalLevel(rssi) {
            if (rssi >= 0)
                return 0
            return rssi >= -60 ? 4 : rssi >= -67 ? 3 : rssi >= -74 ? 2 : 1
        }
        function signalText(level) {
            return level === 4 ? qsTr("Strong signal") : level === 3 ? qsTr("Good signal")
                 : level === 2 ? qsTr("Fair signal") : level === 1 ? qsTr("Weak signal") : ""
        }

        Column {
            id: listColumn
            width: parent.width
            // Modern look: the gaps, title and rows of the question cards (UiMessageDialog); classic: as before
            spacing: window.ui.modern ? 12 : 8

            Label {
                id: listTitle
                width: parent.width
                text: qsTr("Choose a heart rate sensor")
                color: window.ui.ink(Material.foreground)
                font.weight: window.ui.modern ? Font.DemiBold : Font.Bold
                Binding on font.pixelSize { when: window.ui.modern; value: window.ui.fontTitle; restoreMode: Binding.RestoreBindingOrValue }
                wrapMode: Text.WordWrap
            }

            // the strongest signal first: most likely the user's own sensor. A gym may have many: it scrolls
            ListView {
                id: sensorList
                width: parent.width
                // Modern look: a thin bar while the list moves, in the right padding of the card, not
                // over the signal bars at the right end of the rows. Qt places the bar only when its
                // parent is the list, and a Column would place a child itself: it goes on the content
                // item of the popup (inside the padding), just past its
                // edge, x, y and height set here. Classic look: no bar, as before
                ScrollIndicator.vertical: ScrollIndicator {
                    visible: window.ui.modern
                    parent: list.contentItem
                    x: mirrored ? -Math.round((list.leftPadding + width) / 2)
                                : parent.width + Math.round((list.rightPadding - width) / 2)
                    y: listColumn.y + sensorList.y
                    height: sensorList.height
                }
                // Modern look: what the card leaves after its padding, the title and Cancel (a gym with many
                // sensors, a phone in landscape); classic: the limit as before
                height: Math.min(contentHeight, window.ui.modern
                                 ? window.height - 60 - list.topPadding - list.bottomPadding - listTitle.height
                                   - cancelRow.height - 2 * listColumn.spacing
                                 : window.height - 180)
                clip: true
                model: bar.sensors

                delegate: ItemDelegate {
                    id: sensorRow
                    readonly property int level: list.signalLevel(modelData.rssi)
                    width: ListView.view.width
                    // Modern look: the name in line with the title, the signal as bars on the right, a divider
                    // between the rows, the text of the lists (fontBody, textMain), 56 high or more with a bigger
                    // text (Material 3 lists); classic: the stock row with the dBm in the text, as before
                    height: window.ui.modern ? Math.max(56, Math.max(40, contentItem.implicitHeight) + 16) : implicitHeight
                    Binding on font.pixelSize { when: window.ui.modern; value: window.ui.fontBody; restoreMode: Binding.RestoreBindingOrValue }
                    Binding { target: sensorRow.contentItem; property: "color"; when: window.ui.modern; value: window.ui.textMain; restoreMode: Binding.RestoreBindingOrValue }
                    leftPadding: window.ui.modern ? 0 : padding
                    rightPadding: window.ui.modern ? signalBars.width + 12 : padding
                    // 0 or more: the platform did not give the signal
                    text: window.ui.modern ? modelData.name
                                           : modelData.name + (modelData.rssi < 0 ? "  (" + modelData.rssi + " dBm)" : "")
                    Accessible.name: window.ui.modern && level > 0 ? modelData.name + ", " + list.signalText(level)
                                                                    : modelData.name
                    Accessible.onPressAction: clicked()
                    onClicked: {
                        list.close()
                        rootItem.heartRateSensorSuggestionConnect(modelData.name, modelData.address)
                    }

                    Rectangle {
                        visible: window.ui.modern && index > 0
                        width: parent.width
                        height: 1
                        color: window.ui.outline
                    }

                    Row {
                        id: signalBars
                        visible: window.ui.modern && sensorRow.level > 0
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2
                        Accessible.ignored: true

                        Repeater {
                            model: 4
                            Rectangle {
                                anchors.bottom: parent.bottom
                                width: 4
                                height: 5 + 4 * index
                                radius: 1
                                color: index < sensorRow.level ? window.ui.textMain : window.ui.outline
                            }
                        }
                    }
                }
            }

            // Modern look: a visible way out without a choice (the bar stays); classic: a tap outside, as before.
            // A tonal pill, as Not now on the bar; the text is the Cancel of the question cards,
            // already translated
            Item {
                id: cancelRow
                visible: window.ui.modern
                width: parent.width
                height: cancelButton.height

                UiButton {
                    id: cancelButton
                    anchors.right: parent.right
                    text: qsTranslate("UiMessageDialog", "Cancel")
                    cardSurface: true
                    onClicked: list.close()
                }
            }
        }
    }
}
