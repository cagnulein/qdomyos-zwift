import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12

// A heart rate sensor QZ found nearby while none is chosen (homeform::heartRateSensorSuggestions).
// A bar at the bottom rather than a dialog: it covers only the bottom of the page, so it may show during a workout.
// Several sensors: "Choose" opens the list. It stays until answered or the heart rate comes: on a phone left on the
// handlebar the user is busy with the sensor, and a bar that went away by itself was missed.
Rectangle {
    id: bar

    // rootItem is set after main.qml is loaded
    readonly property var sensors: window.rootReady ? rootItem.heartRateSensorSuggestions : []

    visible: sensors.length > 0
    // under the toasts (z: Infinity), above the pages
    z: 1000
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    // clear of the system bars and the cutout in any orientation, as the pages
    anchors.leftMargin: window.getLeftPadding() + 10
    anchors.rightMargin: window.getRightPadding() + 10
    anchors.bottomMargin: window.getBottomPadding() + 10
    height: content.implicitHeight + 24
    radius: 8
    color: Material.background
    border.color: Material.accent
    border.width: 1

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
        spacing: 8

        Label {
            width: parent.width
            text: bar.sensors.length === 1 ? qsTr("Heart rate sensor found: %1").arg(bar.sensors[0].name)
                                           : qsTr("Heart rate sensors found: %1").arg(bar.sensors.length)
            font.bold: true
            wrapMode: Text.WordWrap
        }

        // Flow, not Row: three buttons don't fit one row on a phone once translated
        Flow {
            width: parent.width
            spacing: 8
            layoutDirection: Qt.RightToLeft

            Button {
                text: bar.sensors.length === 1 ? qsTr("Connect") : qsTr("Choose")
                onClicked: {
                    if (bar.sensors.length === 1) {
                        rootItem.heartRateSensorSuggestionConnect(bar.sensors[0].name, bar.sensors[0].address)
                    } else {
                        list.open()
                    }
                }
            }

            Button {
                text: qsTr("Not now")
                onClicked: rootItem.heartRateSensorSuggestionDismiss(false)
            }

            Button {
                text: qsTr("Don't ask again")
                onClicked: rootItem.heartRateSensorSuggestionDismiss(true)
            }
        }
    }

    Popup {
        id: list
        parent: Overlay.overlay
        modal: true
        focus: true
        width: Math.min(window.width - 40, 460)
        height: Math.min(window.height - 60, listColumn.implicitHeight + topPadding + bottomPadding)
        x: Math.round((parent.width - width) / 2)
        y: Math.round((parent.height - height) / 2)
        // closed without a choice: the bar is still there
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            radius: 8
            color: Material.background
            border.color: Material.accent
            border.width: 1
        }

        Column {
            id: listColumn
            width: parent.width
            spacing: 8

            Label {
                width: parent.width
                text: qsTr("Choose a heart rate sensor")
                font.bold: true
                wrapMode: Text.WordWrap
            }

            // the strongest signal first: most likely the user's own sensor. A gym may have many: it scrolls
            ListView {
                width: parent.width
                height: Math.min(contentHeight, window.height - 180)
                clip: true
                model: bar.sensors

                delegate: ItemDelegate {
                    width: ListView.view.width
                    // 0 or more: the platform did not give the signal
                    text: modelData.name + (modelData.rssi < 0 ? "  (" + modelData.rssi + " dBm)" : "")
                    Accessible.name: modelData.name
                    Accessible.onPressAction: clicked()
                    onClicked: {
                        list.close()
                        rootItem.heartRateSensorSuggestionConnect(modelData.name, modelData.address)
                    }
                }
            }
        }
    }
}
