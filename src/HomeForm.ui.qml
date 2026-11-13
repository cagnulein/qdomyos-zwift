import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtGraphicalEffects 1.12
import Qt.labs.settings 1.0
import QtQml 2.15

Page {

    title: qsTr("QZ Fitness")
    id: page

    // VoiceOver accessibility - ignore Page itself, only children are accessible
    Accessible.ignored: true

    property alias start: start
    property alias stop: stop
    property alias lap: lap
    property alias row: row
    // The Start/Stop row; Home.qml makes it the header of the tile grid, so it scrolls with the tiles
    property alias topBar: topBar
    // Set by Home.qml: whether the modern status line has anything the drawer does not show
    property bool modernInfoShown: true
    // Search for the equipment ("Next search in 12 s"), set by Home.qml while no device is connected
    property string searchStatus: ""
    // Home.qml sets the new values from the settings page in it (the background color of the top bar)
    property alias homeFormSettings: settings
    // Modern look, for the grid of tiles in Home.qml: where the first row starts below the top of
    // the Start/Stop row - 16 dp under the letters of the status line, or under the buttons without
    // it. One line of the status counts (a long workout name is elided to one): two lines come
    // only while searching ("<device> found" and the countdown), mostly before the first
    // connection when the grid is empty, and they grow down
    // The top bar can be turned off (top_bar_enabled: topBarHeight 0, no labels): no controls then,
    // the grid and the status line as before
    readonly property bool modernControlsShown: window.ui.modern && topBar.height > 0
    readonly property real modernControlsBottom: modernRow.y + modernRow.height
    readonly property real modernTilesTop: modernInfoShown ? modernInfo.y + statusLine.height + 13
                                                           : modernControlsBottom + 16

    Settings {
	     id: settings
	     Component.onCompleted: window.retakeSettingsSnapshot(settings)
		  property real ui_zoom: 100.0
		  property bool theme_tile_icon_enabled: true
		  property string theme_background_color: "#303030"
		}

    Item {
        width: parent.width
        height: rootItem.topBarHeight
        id: topBar
        visible: !window.lockTiles

        // One line of the modern status line at the text size, for the place of the tiles under it
        Text {
            id: statusLine
            visible: false
            text: "A"
            font.pixelSize: window.ui.fontCaption
        }

        // Modern controls. They forward to the classic buttons below, which Home.qml listens to:
        // Bluetooth, Start/Pause, Stop and Lap, as the classic row
        Loader {
            id: modernRow
            // Made only in the modern look: the classic look creates none of it
            active: page.modernControlsShown
            visible: page.modernControlsShown
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            // 14 dp under the edge of the toolbar (48 dp, the height of its buttons): from the
            // toolbar icons the space above the pills is about 30 dp, near the 16 dp under them.
            // The row scrolls with the tiles (Home.qml), the toolbar slides over it
            anchors.topMargin: 14
            sourceComponent: Row {
                spacing: 8

                Item {
                    width: 48
                    height: 56
                    Rectangle {
                        anchors.centerIn: parent
                        width: 48
                        height: 48
                        radius: 24
                        // window.equipmentConnected, not rootItem.device: with no equipment that getter flips on
                        // every read to blink the classic Bluetooth icon below - more reads would stop the
                        // blinking there
                        color: window.equipmentConnected ? window.ui.alpha(window.ui.ok, 0.16) : window.ui.surfaceHigh
                        UiIcon {
                            id: modernBtIcon
                            anchors.centerIn: parent
                            width: 24
                            height: 24
                            name: window.equipmentConnected ? "bluetooth_connected" : "bluetooth"
                            color: (window.equipmentConnected || blinkOn) ? window.ui.ok : window.ui.textMuted

                            // Searching for the equipment: the icon blinks green, in step with the classic icon -
                            // it flips on changeOfdevice (once a second), so no timer and no animation run, and one
                            // repaint a second at most. Only while the card is on the screen: a covered home page
                            // does not repaint
                            property bool blinkOn: false
                            Connections {
                                target: rootItem
                                function onChangeOfdevice() {
                                    modernBtIcon.blinkOn = modernBtIcon.visible && !window.equipmentConnected && !modernBtIcon.blinkOn
                                }
                            }
                        }
                        // No device yet: a tap searches right away instead of waiting for the next search
                        // (with a device connected bluetoothSearchNow() does nothing). With a device connected the tap
                        // looks for the chosen heart rate sensor again (reconnectHeartRateBelt() waits for the device),
                        // with none chosen for one to offer
                        MouseArea {
                            anchors.fill: parent
                            onClicked: { rootItem.bluetoothSearchNow(); rootItem.reconnectHeartRateBelt(); }

                            // VoiceOver/TalkBack: a button that searches. The state is in the name: TalkBack (Qt 5.15)
                            // reads no description when the name is set
                            Accessible.role: Accessible.Button
                            // qsTr inside an expression takes the context "HomeForm" (the file name up to
                            // the first dot), lupdate writes "HomeForm.ui": the context is given explicitly
                            Accessible.name: qsTranslate("HomeForm.ui", "Bluetooth connection") + ": "
                                             + (window.equipmentConnected ? qsTranslate("HomeForm.ui", "Device connected")
                                                                          : qsTranslate("HomeForm.ui", "Device not connected"))
                            Accessible.description: window.equipmentConnected ? qsTranslate("HomeForm.ui", "Device connected")
                                                                              : qsTranslate("HomeForm.ui", "Device not connected")
                            Accessible.focusable: true
                            Accessible.onPressAction: { rootItem.bluetoothSearchNow(); rootItem.reconnectHeartRateBelt(); }
                        }
                    }
                }

                AbstractButton {
                    id: modernStart
                    width: 120
                    height: 56
                    onClicked: start.clicked()
                    background: Rectangle {
                        radius: height / 2
                        // Steady green while stopped/paused ("Start"), accent while running ("Pause");
                        // the classic blinking startColor is not used here
                        color: rootItem.startIcon.indexOf("pause") >= 0 ? window.ui.accent : window.ui.a["green"]
                        opacity: modernStart.down ? 0.8 : 1
                    }
                    contentItem: Item {
                        Row {
                            anchors.centerIn: parent
                            spacing: 6
                            UiIcon {
                                anchors.verticalCenter: parent.verticalCenter
                                width: 26
                                height: 26
                                name: rootItem.startIcon.indexOf("pause") >= 0 ? "pause" : "play_arrow"
                                color: window.ui.accentInk
                                visible: rootItem.startIcon !== ""
                            }
                            Label {
                                anchors.verticalCenter: parent.verticalCenter
                                // At most what the pill has left beside the icon: with the elide on, a
                                // label with a long and a short variant ("Resume" in ru, de, cs ...)
                                // shows the first that fits, like the classic button
                                width: Math.min(implicitWidth, modernStart.width - 24 - 26 - 6)
                                elide: Text.ElideRight
                                text: rootItem.startText
                                color: window.ui.accentInk
                                font.pixelSize: window.ui.fontBody
                                font.weight: Font.DemiBold
                                visible: text !== ""
                                // the button carries the name; this node gave the screen reader both
                                // variants ("Продолжить.Далее" in the emulator dump)
                                Accessible.ignored: true
                            }
                        }
                    }
                    Accessible.role: Accessible.Button
                    // only the long variant of the label (see the Label above)
                    Accessible.name: rootItem.startText.split("\u009C")[0]
                    Accessible.description: qsTr("Start workout")
                    Accessible.focusable: true
                    Accessible.onPressAction: start.clicked()
                }

                AbstractButton {
                    id: modernStop
                    width: 120
                    height: 56
                    onClicked: stop.clicked()
                    background: UiFrame {
                        radius: height / 2
                        fill: modernStop.down ? window.ui.surfaceHighest : window.ui.surfaceHigh
                        stroke: window.ui.alpha(window.ui.danger, 0.45)
                        strokeWidth: 1
                    }
                    contentItem: Item {
                        Row {
                            anchors.centerIn: parent
                            spacing: 6
                            UiIcon {
                                anchors.verticalCenter: parent.verticalCenter
                                width: 24
                                height: 24
                                name: "stop"
                                color: window.ui.danger
                            }
                            Label {
                                anchors.verticalCenter: parent.verticalCenter
                                text: rootItem.stopText
                                color: window.ui.textMain
                                font.pixelSize: window.ui.fontBody
                                font.weight: Font.DemiBold
                                visible: text !== ""
                                Accessible.ignored: true
                            }
                        }
                    }
                    Accessible.role: Accessible.Button
                    Accessible.name: rootItem.stopText
                    Accessible.description: qsTr("Stop workout")
                    Accessible.focusable: true
                    Accessible.onPressAction: stop.clicked()
                }

                AbstractButton {
                    id: modernLap
                    width: 48
                    height: 56
                    enabled: rootItem.lap
                    opacity: rootItem.lap ? 1 : 0.4
                    onClicked: lap.clicked()
                    background: Rectangle {
                        anchors.centerIn: parent
                        width: 48
                        height: 48
                        radius: 24
                        color: modernLap.down ? window.ui.surfaceHighest : window.ui.surfaceHigh
                    }
                    contentItem: Item {
                        UiIcon {
                            anchors.centerIn: parent
                            width: 24
                            height: 24
                            name: "flag"
                            color: window.ui.textMain
                        }
                    }
                    Accessible.role: Accessible.Button
                    Accessible.name: qsTr("Lap")
                    Accessible.description: qsTr("Record a new lap")
                    Accessible.focusable: true
                    Accessible.onPressAction: if (modernLap.enabled) lap.clicked()
                }
            }
        }

        Row {
            id: row
            visible: !window.ui.modern
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            height: topBar.height - 20
            spacing: 5
            padding: 5

            Rectangle {
                width: 50
                height: row.height
					 color: settings.theme_background_color
                Accessible.ignored: true

                Column {
                    id: column
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width
                    height: row.height
                    spacing: 0
                    padding: 0
                    Accessible.ignored: true

                    Rectangle {
                        width: 50
                        height: row.height
                        color: settings.theme_background_color
                        Accessible.ignored: true

                        Image {
                            anchors.verticalCenter: parent.verticalCenter
                            id: treadmill_connection
                            width: 48
                            height: row.height - 52
                            source: "icons/icons/bluetooth-icon.png"
                            enabled: rootItem.device
                            smooth: true

                            // VoiceOver accessibility: the MouseArea below is the button, read once
                            Accessible.ignored: true
                        }
                        ColorOverlay {
                            anchors.fill: treadmill_connection
                            source: treadmill_connection
                            color: treadmill_connection.enabled ? "#00000000" : "#B0D3d3d3"
                        }
                        // No device yet: a tap searches right away instead of waiting for the next search
                        // (with a device connected bluetoothSearchNow() does nothing). With a device connected the tap
                        // looks for the chosen heart rate sensor again (reconnectHeartRateBelt() waits for the device),
                        // with none chosen for one to offer
                        MouseArea {
                            anchors.fill: parent
                            onClicked: { rootItem.bluetoothSearchNow(); rootItem.reconnectHeartRateBelt(); }

                            // VoiceOver/TalkBack: a button that searches. The state is in the name: TalkBack (Qt 5.15)
                            // reads no description when the name is set
                            Accessible.role: Accessible.Button
                            Accessible.name: qsTr("Bluetooth connection") + ": " + (rootItem.device ? qsTr("Device connected") : qsTr("Device not connected"))
                            Accessible.description: rootItem.device ? qsTr("Device connected") : qsTr("Device not connected")
                            Accessible.focusable: true
                            Accessible.onPressAction: { rootItem.bluetoothSearchNow(); rootItem.reconnectHeartRateBelt(); }
                        }
                    }
                    Image {
                        anchors.horizontalCenter: parent.horizontalCenter
                        id: treadmill_signal
                        width: 24
                        height: row.height - 76
                        source: rootItem.signal
                        smooth: true
                        Accessible.ignored: true
                    }
                }
            }

            Rectangle {
                width: 120
                height: row.height
					 color: settings.theme_background_color
                Accessible.ignored: true

                RoundButton {
                    icon.source: rootItem.startIcon
                    icon.height: row.height - 54
                    icon.width: 46
                    text: rootItem.startText
                    // fixed width: keep the size it had before the Text size setting
                    Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.appFontPx; restoreMode: Binding.RestoreBindingOrValue }
                    enabled: true
                    id: start
                    width: 120
                    height: row.height - 4

                    // VoiceOver accessibility
                    Accessible.role: Accessible.Button
                    Accessible.name: rootItem.startText
                    Accessible.description: qsTr("Start workout")
                    Accessible.focusable: true
                }
                ColorOverlay {
                    anchors.fill: start
                    source: start
                    color: rootItem.startColor
                    enabled: rootItem.startColor === "red" ? true : false
                }
            }

            Rectangle {
                width: 120
                height: row.height
					 color: settings.theme_background_color
                Accessible.ignored: true

                RoundButton {
                    icon.source: rootItem.stopIcon
                    icon.height: row.height - 54
                    icon.width: 46
                    text: rootItem.stopText
                    Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.appFontPx; restoreMode: Binding.RestoreBindingOrValue }
                    enabled: true
                    id: stop
                    width: 120
                    height: row.height - 4

                    // VoiceOver accessibility
                    Accessible.role: Accessible.Button
                    Accessible.name: rootItem.stopText
                    Accessible.description: qsTr("Stop workout")
                    Accessible.focusable: true
                }
                ColorOverlay {
                    anchors.fill: stop
                    source: stop
                    color: rootItem.stopColor
                    enabled: rootItem.stopColor === "red" ? true : false
                }
            }

            Rectangle {
                id: item2
                width: 50
                height: row.height
					 color: settings.theme_background_color
                Accessible.ignored: true

                RoundButton {
                    anchors.verticalCenter: parent.verticalCenter
                    id: lap
                    width: 48
                    height: row.height - 52
                    icon.source: "icons/icons/lap.png"
                    icon.width: 48
                    icon.height: 48
                    enabled: rootItem.lap
                    smooth: true

                    // VoiceOver accessibility
                    Accessible.role: Accessible.Button
                    Accessible.name: qsTr("Lap")
                    Accessible.description: qsTr("Record a new lap")
                    Accessible.focusable: true
                }
                ColorOverlay {
                    anchors.fill: lap
                    source: lap
                    color: lap.enabled ? "#00000000" : "#B0D3d3d3"
                }
            }
        }

        Row {
            id: row1
            width: parent.width
            anchors.bottom: row.bottom
            // a bigger text grows down into the gap above the tiles, not up into the buttons
            anchors.bottomMargin: window.textScale !== 1 ? -10 - Math.round(statusLabel.height * (1 - 1 / window.textScale)) : -10

            Label {
                id: statusLabel
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                // the search status may have two lines ("<device> found" and the countdown): the line
                // grows upwards, so they are set closer to keep the first one off the buttons
                horizontalAlignment: Text.AlignHCenter
                lineHeight: text.indexOf("\n") >= 0 ? 0.8 : 1
                text: page.searchStatus !== "" ? page.searchStatus : rootItem.info
                visible: !window.ui.modern
            }
        }

        // Modern status line: signal bars and the device status on one line
        Loader {
            id: modernInfo
            // Made only in the modern look, as the controls above
            active: page.modernControlsShown
            // Over the empty state only the last device found and the time to the next search
            // ("HRM-Pro found" and "Next search in 12 s" under it): its heading says the rest
            // ("Looking for your equipment"). The line has its own place under the buttons, so
            // it comes and goes without moving the page
            readonly property bool countdown: page.searchStatus !== ""
                                              && page.searchStatus !== qsTranslate("homeform", "Searching for the device...")
            // with the top bar off it is not shown either: its place is under the controls.
            // It is the state of the buttons, so it stays with them while the tiles scroll
            visible: page.modernControlsShown && page.modernInfoShown && (!modernEmpty.visible || countdown)
            anchors.horizontalCenter: parent.horizontalCenter
            // 8 dp from the buttons to the letters; a second line grows down (see modernTilesTop)
            anchors.top: modernRow.bottom
            anchors.topMargin: 5
            sourceComponent: Row {
                spacing: 8

                Image {
                    visible: !modernEmpty.visible
                    anchors.verticalCenter: parent.verticalCenter
                    width: 20
                    height: 13
                    fillMode: Image.PreserveAspectFit
                    source: rootItem.signal
                    smooth: true
                    Accessible.ignored: true
                    // The bars are white: on a light page paint them in the text colour
                    layer.enabled: !window.ui.dark
                    layer.effect: ColorOverlay { color: window.ui.textMuted }
                }
                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    // "Connecting..." is the untranslated start value of homeform::m_info: before the
                    // first search it means the same as the search status
                    text: page.searchStatus !== "" ? page.searchStatus : (rootItem.info === "Connecting..." ? qsTranslate("homeform", "Searching for the device...") : rootItem.info)
                    // A long workout name stays one line (the tiles start under one line) and
                    // ends with "…" inside the page margins, the signal bars beside it. Qt elides
                    // one-line text only, so the two short lines of the search stay as they are
                    width: Math.min(implicitWidth, page.width - 2 * window.contentSideMargin - 28)
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignHCenter
                    // two lines ("<device> found" and the countdown) grow down, set closer
                    lineHeight: text.indexOf("\n") >= 0 ? 0.8 : 1
                    color: window.ui.textMuted
                    font.pixelSize: window.ui.fontCaption
                }
            }
        }

        Label {
            id: lblHelp
            width: parent.width
            leftPadding: window.contentSideMargin
            rightPadding: window.contentSideMargin
            anchors.top: row1.bottom
            anchors.topMargin: 30
            text: qsTr("This app should automatically connect to your bike/treadmill/rower. <b>If it doesn't, please check</b>:<br>1) your Echelon/Domyos App MUST be closed while qdomyos-zwift is running;<br>2) both Bluetooth and Bluetooth permissions MUST be enabled<br>3) your bike/treadmill/rower should be turned on BEFORE starting this app<br>4) try to restart your device<br><br>If your bike/treadmill disconnects every 30 seconds try to disable the 'virtual device' setting on the left bar.<br><br>In case of issues, please feel free to contact me at roberto.viola83@gmail.com.<br><br><b>Have a nice ride!</b><br/ ><i>QZ specifically disclaims liability for<br>incidental or consequential damages and assumes<br>no responsibility or liability for any loss<br>or damage suffered by any person as a result of<br>the use or misuse of the app.</i><br><br>Roberto Viola")
            wrapMode: Label.WordWrap
            // The look first: the hidden one then never reads rootItem, which is not there yet
            // in the first second (see main.qml rootReady) and would leave both shown
            visible: !window.ui.modern && rootItem.labelHelp
        }

        // Modern look: while nothing is connected, a short "searching" state instead of the
        // wall of text, and a short checklist cut from the same help text (already translated)
        Loader {
            id: modernEmpty
            // Made only in the modern look before the first connection
            active: window.ui.modern && rootItem.labelHelp
            visible: active
            anchors.top: row1.bottom
            anchors.topMargin: 24
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(parent.width - 2 * window.contentSideMargin, 560)
            height: page.height - (row1.y + row1.height) - 24
            sourceComponent: Flickable {
                contentHeight: emptyColumn.height + 24
                // The support button "?" shows mostly in this state ("No device connected"): room for
                // it under the checklist, as under the tiles (Home.qml), eased when it goes
                bottomMargin: window.supportHelpShown ? 56 + 16 : 0
                Behavior on bottomMargin { NumberAnimation { duration: 150; easing.type: Easing.OutQuad } }
                clip: true
                boundsBehavior: Flickable.StopAtBounds

                // qsTr inside an expression looks the text up under the context "HomeForm" (the file
                // name up to the first dot), while lupdate writes "HomeForm.ui": the context is given
                // explicitly
                Column {
                    id: emptyColumn
                    width: parent.width
                    spacing: 12

                    Label {
                        width: parent.width
                        topPadding: 4
                        text: qsTranslate("HomeForm.ui", "Looking for your equipment…")
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        color: window.ui.textMain
                        font.pixelSize: window.ui.fontTitle
                        font.weight: Font.DemiBold
                    }

                    Label {
                        width: parent.width
                        text: qsTranslate("HomeForm.ui", "Turn on your bike, treadmill or rower: QZ connects to it automatically.")
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        color: window.ui.textMuted
                        font.pixelSize: window.ui.fontLabel
                    }

                    Item { width: 1; height: 8 }

                    // "Not connecting?": a short checklist, always open, cut from the old help text
                    // (already translated): the numbered checks and the hint about the virtual device,
                    // then the rest (the e-mail, the disclaimer, the author) in small muted text.
                    // Without the lead sentence (the subtitle says it) and the bold "If it doesn't,
                    // please check" (the heading says it)
                    Rectangle {
                        width: parent.width
                        height: helpColumn.height + 32
                        radius: 16
                        color: window.ui.surface

                        // [checks, rest]
                        readonly property var helpParts: {
                            var t = lblHelp.text
                            var b = t.indexOf("<b>")
                            if (b < 0)
                                return [t, ""]
                            t = t.substring(b)
                            var br = t.indexOf("<br>")
                            if (br < 0)
                                return [t, ""]
                            t = t.substring(br + 4)
                            var p1 = t.indexOf("<br><br>")
                            var p2 = p1 < 0 ? -1 : t.indexOf("<br><br>", p1 + 8)
                            if (p2 < 0)
                                return [t, ""]
                            // The disclaimer is broken into short lines by hand for the wide
                            // classic page: here it wraps by itself, so its line breaks become spaces
                            var rest = t.substring(p2 + 8)
                            var i1 = rest.indexOf("<i>")
                            var i2 = i1 < 0 ? -1 : rest.indexOf("</i>", i1 + 3)
                            if (i2 >= 0) {
                                var x = rest.substring(i1 + 3, i2)
                                var breaks = ["<br>", "<br/>", "<br />", "<br/ >"]
                                for (var k = 0; k < breaks.length; k++)
                                    x = x.split(breaks[k]).join(" ")
                                rest = rest.substring(0, i1 + 3) + x + rest.substring(i2)
                            }
                            return [t.substring(0, p2), rest]
                        }

                        Column {
                            id: helpColumn
                            x: 16
                            y: 16
                            width: parent.width - 32
                            spacing: 8

                            Row {
                                spacing: 10
                                UiIcon {
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: 20
                                    height: 20
                                    name: "help"
                                    color: window.ui.textMuted
                                }
                                Label {
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: qsTr("Not connecting?")
                                    color: window.ui.textMain
                                    font.pixelSize: window.ui.fontLabel
                                    font.weight: Font.Medium
                                }
                            }

                            Label {
                                width: parent.width
                                text: helpColumn.parent.helpParts[0]
                                textFormat: Text.StyledText
                                wrapMode: Text.WordWrap
                                color: window.ui.textMuted
                                font.pixelSize: window.ui.fontCaption
                                lineHeight: 1.1
                            }

                            Label {
                                visible: text !== ""
                                width: parent.width
                                topPadding: 4
                                text: helpColumn.parent.helpParts[1]
                                textFormat: Text.StyledText
                                wrapMode: Text.WordWrap
                                color: window.ui.textMuted
                                opacity: 0.75
                                font.pixelSize: window.ui.fontCaption
                                lineHeight: 1.1
                            }
                        }
                    }
                }
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;formeditorZoom:0.6600000262260437;height:480;width:640}
}
##^##*/

