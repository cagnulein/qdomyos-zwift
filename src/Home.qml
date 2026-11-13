import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtGraphicalEffects 1.12
import QtQuick.Window 2.12
import Qt.labs.settings 1.0
import Qt.labs.platform 1.1
import QtMultimedia 5.15

HomeForm {
    objectName: "home"
    // Modern look: rootItem comes from C++ only after main.qml is loaded (main.cpp). Until
    // then every binding on it fails and leaves its default, visible, so the first frames
    // drew the classic help text and the unstyled buttons over the searching state. The
    // content waits for it, the background is drawn at once
    contentItem.visible: !window.ui.modern || typeof rootItem !== "undefined"
    // Modern look: the line under Start/Stop only while it names a loaded workout or the search.
    // "<device> found" (homeform.cpp) is in the drawer header already; the signal bars go with
    // the line
    modernInfoShown: {
        // The search for the equipment ("Next search in 12 s") is not in the drawer
        if (searchStatus !== "")
            return true
        if (typeof rootItem === "undefined" || !rootItem || !rootItem.info)
            return false
        // "Connecting..." is the start value of homeform::m_info, kept by equipment that does not
        // write its own status (the fake devices): connected, the drawer header says "Device
        // connected", and the line would read "Searching for the device..." beside the green card
        if (rootItem.info === "Connecting..." && window.equipmentConnected)
            return false
        // homeform writes "<device> found" into info once the device is found (name + " found");
        // a loaded workout puts its name there
        return !String(rootItem.info).endsWith(" found")
    }
    background: Rectangle {
        anchors.fill: parent
        width: parent.fill
        height: parent.fill
        color: window.ui.modern ? window.ui.bg : settings.theme_background_color

        // VoiceOver accessibility - ignore decorative background
        Accessible.role: Accessible.Pane
        Accessible.ignored: true
    }
    // The Start/Stop row is the header of the grid (see GridView.header below), so it scrolls with
    // the tiles as one page. It spans the page, not just the grid, so its buttons stay centred
    topBar.x: -gridView.x
    topBar.width: gridBox.width
    // Below the room for the toolbar, which hangs over the top of the grid
    topBar.y: gridView.barRoom

    signal start_clicked;
    signal stop_clicked;
    signal lap_clicked;
    signal peloton_start_workout;
    signal peloton_abort_workout;
    signal plus_clicked(string name)
    signal minus_clicked(string name)
    signal largeButton_clicked(string name)

    Settings {
        id: settings
        Component.onCompleted: window.retakeSettingsSnapshot(settings)
        property real ui_zoom: 100.0
        property bool theme_tile_icon_enabled: true
        property string theme_tile_background_color: "#303030"
        property string theme_background_color: "#303030"
        property bool theme_tile_shadow_enabled: true
        property string theme_tile_shadow_color: "#9C27B0"
        property int theme_tile_secondline_textsize: 12
        property bool skipLocationServicesDialog: false
        property bool trainprogram_sound_on_segment: false
    }

    // The settings page writes through its own Settings copy, and a copy reads the stored values
    // only when it is created, so the tile size and the tile colors waited for a restart (#466).
    // The settings page sends each new value here (settings.qml), and it is set in both copies.
    // Assigning one key makes Qt.labs.settings write the whole copy back, so every key of these
    // copies that the settings page can change is sent, and none of them is left stale.
    Connections {
        target: window
        function onTileSettingChanged(key, value) {
            if (settings[key] !== undefined && settings[key] !== value) {
                // skipLocationServicesDialog is written back too. homeform.cpp resets it when the
                // GPS is on (Android), after this copy has read it: apply the same rule here
                if (Qt.platform.os === "android" && typeof rootItem !== "undefined" && rootItem.locationServices())
                    settings.skipLocationServicesDialog = false
                settings[key] = value
            }
            if (homeFormSettings[key] !== undefined && homeFormSettings[key] !== value)
                homeFormSettings[key] = value
        }
    }

    SoundEffect {
        id: trainingProgramSegmentSound
        source: "qrc:/sounds/training-program-segment.wav"
        volume: 0.9
    }

    Connections {
        target: rootItem
        onTrainingProgramIntervalSoundRequested: trainingProgramSegmentSound.play()
    }

    UiMessageDialog {
        id: messagePelotonAskStart
        text: qsTr("Peloton Workout in progress")
        informativeText: qsTr("Do you want to follow the resistance? ") + rootItem.pelotonProvider
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: {rootItem.pelotonAskStart = false; peloton_start_workout();}
        onNoClicked: {rootItem.pelotonAskStart = false; peloton_abort_workout();}
        visible: rootItem.pelotonAskStart
    }

    UiNotice {
        id: popupLap
        text: qsTr("New lap started!")
    }

    UiMessageDialog {
        id: stopConfirmationDialog
        text: qsTr("Stop Workout")
        informativeText: qsTr("Do you really want to stop the current workout?")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        yesText: qsTr("Stop")
        noText: qsTr("Cancel")
        destructive: true
        onYesClicked: {
            close();
            inner_stop();
        }
        onNoClicked: close()
    }

    // Optional post-workout popup (settings.rpe_feel_popup_enabled) asking for perceived exertion
    // and how the user felt. Saving the FIT file (rootItem.finalizeFitSave) is suspended until this
    // popup is answered, so the values can be embedded in the FIT file before it's written/uploaded.
    UiPopup {
        id: rpeFeelPopup
        parent: Overlay.overlay

        x: Math.round((parent.width - width) / 2)
        y: Math.round((parent.height - height) / 2)
        // Modern: a card that fits a phone in portrait (420 is wider than most of them) and as
        // tall as its content; classic: the fixed size as before, and with a bigger text as wide
        // as the screen allows, as tall as the content (Skip/Save fell out)
        width: modern ? Math.min(parent.width - 48, 440)
            : window.textScale !== 1 ? Math.min(420, parent.width - 32) : 420
        height: modern ? rpeColumn.implicitHeight + 2 * 16 + topPadding + bottomPadding
            : window.textScale !== 1
            ? Math.min(parent.height - 40, Math.max(340, rpeColumn.implicitHeight + 32 + topPadding + bottomPadding))
            : 340
        modal: true
        focus: true
        palette.text: "white"
        closePolicy: Popup.NoAutoClose

        property int selectedRpe: 5
        property int selectedFeel: 50
        readonly property var rpeLabels: [
            qsTr("Rest"), qsTr("Very Light"), qsTr("Light"), qsTr("Moderate"),
            qsTr("Somewhat Hard"), qsTr("Hard"), qsTr("Harder"), qsTr("Very Hard"),
            qsTr("Very Very Hard"), qsTr("Extremely Hard"), qsTr("Maximum Effort")
        ]

        Column {
            id: rpeColumn
            anchors.fill: parent
            anchors.margins: 16
            spacing: 14

            Label {
                text: qsTr("How was this workout?")
                // Modern: the title of UiMessageDialog
                font.weight: rpeFeelPopup.modern ? Font.DemiBold : Font.Bold
                font.pixelSize: rpeFeelPopup.modern ? window.ui.fontTitle : window.fontPx(18)
                color: window.ui.ink(Material.foreground)
                width: parent.width
                wrapMode: Text.WordWrap
            }

            Label {
                text: qsTr("Perceived Exertion (RPE): ") + rpeFeelPopup.selectedRpe + " - " + rpeFeelPopup.rpeLabels[rpeFeelPopup.selectedRpe]
                width: parent.width
                wrapMode: Text.WordWrap
                color: window.ui.ink(Material.foreground)
            }

            Slider {
                id: rpeSlider
                width: parent.width
                from: 0
                to: 10
                stepSize: 1
                value: rpeFeelPopup.selectedRpe
                onValueChanged: rpeFeelPopup.selectedRpe = value
            }

            Label {
                text: qsTr("How did you feel?")
                width: parent.width
                wrapMode: Text.WordWrap
                color: window.ui.ink(Material.foreground)
            }

            UiComboBox {
                id: feelCombo
                width: parent.width
                model: [qsTr("Very Bad"), qsTr("Bad"), qsTr("OK"), qsTr("Good"), qsTr("Very Good")]
                currentIndex: 2
                onCurrentIndexChanged: rpeFeelPopup.selectedFeel = currentIndex * 25
            }

            Row {
                spacing: 12
                anchors.horizontalCenter: parent.horizontalCenter

                UiButton {
                    text: qsTr("Skip")
                    cardSurface: true
                    onClicked: {
                        rpeFeelPopup.close();
                        rootItem.finalizeFitSave(-1, -1);
                        finish_stop();
                    }
                }

                UiButton {
                    text: qsTr("Save")
                    highlighted: true
                    onClicked: {
                        rpeFeelPopup.close();
                        rootItem.finalizeFitSave(rpeFeelPopup.selectedRpe, rpeFeelPopup.selectedFeel);
                        finish_stop();
                    }
                }
            }
        }
    }

    Timer {
        id: popupLapAutoClose
        interval: 2000; running: false; repeat: false
        onTriggered: popupLap.close();
    }

    Timer {
        id: searchStatusTimer
        // Always running: rootItem.device blinks while no device is connected (it drives the icon),
        // and bluetoothSearchStatus() is empty once a device is connected
        interval: 1000; repeat: true; triggeredOnStart: true; running: true
        onTriggered: searchStatus = rootItem.bluetoothSearchStatus()
    }

    Timer {
        id: checkStartStopFromWeb
        interval: 200; running: true; repeat: true
        onTriggered: {if(rootItem.stopRequested) {rootItem.stopRequested = false; inner_stop(); }}
    }

    property bool locationServiceRequsted: false
    // Bluetooth and Location as Android reports them now, not only at app start: switched on from
    // the quick settings, the questions about them go away.
    // Home.qml is created before homeform sets rootItem: until then assume "on", so the question
    // doesn't flash at start; setting rootItem re-evaluates the binding with the real value
    property bool locationServicesOn: typeof rootItem === "undefined" || rootItem.locationServices()
    // "No" answered: not asked again until the next start
    property bool locationServicesDeclined: false

    Timer {
        interval: 2000; repeat: true
        // only while a question about them is open: devices without GPS never report Location on
        running: !locationServicesOn && (locationServicesDialog.visible || locationServiceRequsted)
        onTriggered: locationServicesOn = rootItem.refreshLocationServices()
    }

    UiMessageDialog {
        id: locationServicesDialog
        text: qsTr("Permissions Required")
        informativeText: qsTr("QZ needs Bluetooth enabled to discover fitness equipment. On Android 11 and earlier, Android also requires Location permission and Location Services for Bluetooth discovery. QZ does not use your location data. On Android 12 and later, Nearby devices permission allows Bluetooth discovery without Location.\n\nWould you like to enable the required services?")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: {
            locationServiceRequsted = true
            rootItem.enableLocationServices()
        }
        onNoClicked: {
            locationServicesDeclined = true
            remindLocationServicesDialog.visible = true
        }
        visible: !locationServicesOn && !locationServiceRequsted && !locationServicesDeclined && !settings.skipLocationServicesDialog
    }

    UiMessageDialog {
        id: remindLocationServicesDialog
        text: qsTr("Reminder Preference")
        informativeText: qsTr("Would you like to be reminded about enabling Bluetooth or Location Services next time?")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: settings.skipLocationServicesDialog = false
        onNoClicked: settings.skipLocationServicesDialog = true
        visible: false
    }

    UiMessageDialog {
        text: qsTr("Restart the app")
        informativeText: qsTr("To apply the changes, you need to restart the app.\nWould you like to do that now?")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: Qt.callLater(Qt.quit)
        onNoClicked: this.visible = false;
        visible: locationServiceRequsted && !locationServicesOn
    }

    Timer {
        interval: 200; running: true; repeat: false
        onTriggered: {
            if(rootItem.firstRun()) {
                stackView.push("Wizard.qml")
            }
        }
    }

    function inner_stop() {
        stop_clicked();
        if (rootItem.rpeFeelPopupEnabled()) {
            rpeFeelPopup.selectedRpe = 5;
            rpeFeelPopup.selectedFeel = 50;
            // A change of language rebuilds the list and moves it to the first item
            feelCombo.currentIndex = 2;
            rpeFeelPopup.open();
        } else {
            finish_stop();
        }
    }

    function finish_stop() {
        rootItem.save_screenshot();
        if(CHARTJS)
            stackView.push("ChartJsTest.qml")
        else
            stackView.push("ChartsEndWorkout.qml")
    }

    start.onClicked: { start_clicked(); }
    stop.onClicked: {
        if (rootItem.confirmStopEnabled()) {
            stopConfirmationDialog.open();
        } else {
            inner_stop();
        }
    }
    lap.onClicked: { lap_clicked(); popupLap.open(); popupLapAutoClose.running = true; }

    Component.onCompleted: {
        console.log("home.qml completed");
    }

    // The scrolled area: the page, cut off at its edges. It is as wide as the page, so the shadows
    // of the outer columns stay. It reaches up under the toolbar, which hangs over the page
    // (main.qml), so the tiles can take its place once it has slid away. Not while the tiles are
    // being moved: the toolbar stays then, and moving them needs the grid where the page is
    Item {
        id: gridBox
        anchors.fill: parent
        anchors.topMargin: window.lockTiles ? 0 : -headerToolbar.contentHeight
        clip: true
        // The page background reaches up with it: behind the tiles there the window colour would
        // show, not the theme one
        Rectangle {
            width: parent.width
            height: gridView.barRoom
            color: window.ui.modern ? window.ui.bg : settings.theme_background_color
        }
    }

    GridView {
        parent: gridBox
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        // The tiles stretch to fill the row: the space left over by a whole number of columns
        // is shared between the columns, up to a quarter of the tile width, and what remains
        // centres the grid. Measured on the page, so the Android insets are excluded.
        // The grid is exactly as wide as its columns and centred by the anchor above, not
        // by leftMargin: Qt 5.15 does not recount the columns when leftMargin changes, so
        // after a resize a row could come out one column short
        readonly property real tileBaseWidth: 175 * settings.ui_zoom / 100
        // Every tile is narrower than its cell by tileGap and sits at the cell's left edge, so the
        // last column ends with the gap: the grid is shifted by half the gap, so the outer margins
        // are equal. The modern look also adds the page side margin of its other screens (the
        // "Not connecting?" card, the header): the outer tile edges line up with the card
        readonly property real tileGap: 5 * settings.ui_zoom / 100
        readonly property real tileSideMargin: window.ui.modern ? window.contentSideMargin : 0
        readonly property real tileRowWidth: parent.width - (window.ui.modern ? 2 * tileSideMargin - tileGap : 0)
        // Columns counted on the whole page, as before the margins: on a 360 dp phone the margins
        // would leave one column instead of two. The tiles get narrower there instead (165 dp)
        readonly property int tileColumns: Math.max(1, Math.floor(parent.width / tileBaseWidth))
        cellWidth: Math.floor(Math.min(tileRowWidth / tileColumns, tileBaseWidth * 1.25))
        width: tileColumns * cellWidth
        anchors.horizontalCenterOffset: tileGap / 2
        cellHeight: 130 * settings.ui_zoom / 100
        focus: true
        model: appModel
        // While the tiles are being moved the gap above the first row is part of the content,
        // so it scrolls away with the tiles instead of staying as an empty band
        topMargin: window.lockTiles ? 30 : 0
        // Modern look: while the support button "?" is in the corner the list goes on under the
        // last row by the button and its 16 dp margin (main.qml), so at the end it covers no tile
        bottomMargin: window.ui.modern && window.supportHelpShown ? 56 + 16 : 0
        // Eased: when the button goes with the list at its end, the tiles glide down by the room
        // instead of jumping (Flickable fixes contentY at once on a smaller margin)
        Behavior on bottomMargin { NumberAnimation { duration: 150; easing.type: Easing.OutQuad } }
        // The room for the toolbar above the Start/Stop row, so at the top it covers nothing, then
        // the row and the gap under it, part of the content; none while the tiles are being moved
        // (the row is hidden then and the grid starts below the toolbar).
        // Modern look: the status line is part of the row, the first row starts under it
        // (HomeForm modernTilesTop); with the top bar off a smaller gap when the status line is
        // not shown
        readonly property real barRoom: window.lockTiles ? 0 : headerToolbar.contentHeight
        readonly property real rowsRoom: window.lockTiles ? 0
                                         : barRoom + (modernControlsShown ? modernTilesTop
                                                      : rootItem.topBarHeight + (window.ui.modern && !modernInfoShown ? 12 : 30))
        header: Item {
            width: gridView.width
            height: gridView.rowsRoom
            // Under the tiles, as in the page before: the help text below the row (shown while
            // searching) reaches down over the tiles. GridView would put the header at z 1, over them
            z: -1
            Component.onCompleted: topBar.parent = this
        }
        // At the top when the tiles get locked or unlocked: stay at the top, with the row or the gap
        function keepAtTop() {
            if (contentY <= 0)
                contentY = -topMargin - rowsRoom
        }
        onTopMarginChanged: keepAtTop()
        // Modern look: the status line comes and goes with a loaded workout, and the room above
        // the first row with it. Only a list at the top goes to the new top; a scrolled one keeps
        // its tiles where they are (the first row at the edge of the shown area has contentY <= 0
        // as well)
        property real lastRowsRoom: 0
        onRowsRoomChanged: {
            if (!window.ui.modern)
                keepAtTop()
            else if (contentY <= -topMargin - lastRowsRoom + 1)
                contentY = -topMargin - rowsRoom
            lastRowsRoom = rowsRoom
        }
        // Modern look before connection: the grid is still empty and the "searching" state
        // (HomeForm modernEmpty) is in its header: the grid does not scroll then, the state does
        interactive: !window.lockTiles && !(window.ui.modern && rootItem.labelHelp)
        id: gridView
        objectName: "gridview"

        // The app toolbar slides away while the tiles scroll down and comes back as soon as they
        // scroll up, 1:1 with them (Material "scroll|enterAlways"): it hangs over the grid
        // (main.qml), so the tiles never move under the finger. scrolledNow, not contentY: at
        // start originY follows contentY later. Past the top or the end the view only springs
        // back, which does not move the toolbar, and at the top it is always shown
        readonly property real scrolledNow: contentY - originY
        property real lastBarScroll: 0
        onScrolledNowChanged: {
            var s = Math.min(Math.max(scrolledNow, 0), Math.max(contentHeight + bottomMargin - height, 0))
            var d = s - lastBarScroll
            lastBarScroll = s
            if (window.lockTiles || barSnap.running)
                return
            // Only the user's scrolling (a drag or its flick) moves it: the grid also moves on a
            // rotation or a resize, with no snap to follow, and left it half shown
            if (moving)
                headerToolbar.barShift = Math.max(0, Math.min(headerToolbar.contentHeight, headerToolbar.barShift + d, s))
            else
                headerToolbar.barShift = Math.min(headerToolbar.barShift, s)
        }
        // Snap: once the finger is off, a half-shown toolbar goes on to the nearer end (Material
        // "snap"). Within its room above the Start/Stop row the list goes with it, so the toolbar
        // never comes to rest over the row: to the top, or to the row under the status bar
        NumberAnimation { id: barSnap; target: headerToolbar; property: "barShift"; duration: 150; easing.type: Easing.OutQuad }
        NumberAnimation { id: listSnap; target: gridView; property: "contentY"; duration: 150; easing.type: Easing.OutQuad }
        onMovementStarted: {
            barSnap.stop()
            listSnap.stop()
        }
        onMovementEnded: {
            if (window.lockTiles)
                return
            var b = headerToolbar.barShift, h = headerToolbar.contentHeight
            // the end of the list: with the room under the last row (bottomMargin) while the
            // support button is shown, or the toolbar snap pulled that row back under the button
            var maxS = Math.max(contentHeight + bottomMargin - height, 0)
            var s = Math.min(Math.max(scrolledNow, 0), maxS)
            if (s > 0 && s < h) {
                // The toolbar follows the list back to the top by itself (barShift <= s). A list
                // longer than the page by less than the toolbar ends first: then the toolbar goes
                // only as far as the list, so the last row stays in view
                var end = Math.min(h, maxS)
                var hide = b >= end / 2
                listSnap.to = originY + (hide ? end : 0)
                listSnap.start()
                if (hide) {
                    barSnap.to = end
                    barSnap.start()
                }
            } else if (b > 0 && b < h) {
                barSnap.to = b < h / 2 ? 0 : h
                barSnap.start()
            }
        }
        // Moving the tiles shows the toolbar (main.qml); after it the toolbar starts shown again
        Connections {
            target: window
            function onLockTilesChanged() {
                listSnap.stop()
                headerToolbar.barShift = 0
            }
        }
        Screen.orientationUpdateMask:  Qt.LandscapeOrientation | Qt.PortraitOrientation | Qt.InvertedLandscapeOrientation | Qt.InvertedPortraitOrientation
        Screen.onPrimaryOrientationChanged:{
            // Nothing to recompute: cellWidth and the grid width follow the page width
        }

        Accessible.ignored: true

        delegate: Item {
            id: id1
            width: gridView.cellWidth - gridView.tileGap
            height: 125 * settings.ui_zoom / 100

            visible: visibleItem
            Component.onCompleted: console.log("completed " + objectName)

            // VoiceOver accessibility support. The search on the heart rate tile is its own button (heartTap): a press
            // action here would make every tile clickable for Android, also the ones that do nothing on a tap
            Accessible.role: largeButton ? Accessible.Button : (writable ? Accessible.Pane : Accessible.StaticText)
            Accessible.name: name + (largeButton ? "" : (": " + value))
            Accessible.description: largeButton ? largeButtonLabel : (secondLine !== "" ? secondLine : (writable ? qsTr("Adjustable. Current value: ") + value : qsTr("Current value: ") + value))
            Accessible.focusable: true


            SequentialAnimation on rotation {
                NumberAnimation { to:  2; duration: 60 }
                NumberAnimation { to: -2; duration: 120 }
                NumberAnimation { to:  0; duration: 60 }
                running: loc.currentId !== -1 && id1.state !== "active" && window.lockTiles
                loops: Animation.Infinite; alwaysRunToEnd: true
            }

            states: State {
                name: "active"; when: loc.currentId === gridId && window.lockTiles
                PropertyChanges { target: id1; opacity: 0.3 }
            }

            transitions: Transition { NumberAnimation { property: "opacity"; duration: 200} }

            // Only the tile of the current look is created: hidden, the other one still kept
            // its elements and bindings, and its texts took every new value. Switching the
            // look with the page open replaces the tile
            Loader {
                anchors.fill: parent
                sourceComponent: window.ui.modern ? modernTileComponent : classicTileComponent
            }

            // The heart rate tile: a tap does what a tap on the Bluetooth icon does – with no machine yet it searches
            // for it (bluetoothSearchNow(), nothing with one connected), and it looks for the heart rate sensor
            // (homeform::reconnectHeartRateBelt()): the chosen one again or, with none chosen, one nearby to offer.
            // Not while the tiles are being moved (loc takes the presses then). Here and not in a tile component:
            // the same in both looks, over the card of either (rect, modernCard: this size).
            // For TalkBack and VoiceOver a button after the tile, named by the tile only: its value changes on its own,
            // and on the other tiles there is no such node
            MouseArea {
                id: heartTap
                width: id1.width - 2 * settings.ui_zoom / 100
                height: 123 * settings.ui_zoom / 100
                enabled: identificator === "heart" && !window.lockTiles
                onClicked: { rootItem.bluetoothSearchNow(); rootItem.reconnectHeartRateBelt(); }
                Accessible.role: Accessible.Button
                Accessible.name: name
                Accessible.focusable: true
                Accessible.ignored: !enabled
                Accessible.onPressAction: { if (enabled) { rootItem.bluetoothSearchNow(); rootItem.reconnectHeartRateBelt(); } }
            }

            // Modern tile: a card with the label and a small icon on top, the value in the
            // middle, the zone colour as a bar on the left edge and as the value colour
            Component {
                id: modernTileComponent
            Item {
                id: modernTile
                anchors.fill: parent
                readonly property real zoom: settings.ui_zoom / 100
                readonly property bool zoned: !largeButton && valueFontColor !== "white"

                // UiFrame, not Rectangle.border: a thin border breaks up on Android
                UiFrame {
                    id: modernCard
                    width: modernTile.width - 2 * modernTile.zoom
                    height: 123 * modernTile.zoom
                    radius: 16 * modernTile.zoom
                    fill: window.ui.surface
                    stroke: window.ui.alpha(window.ui.textMain, 0.06)
                    strokeWidth: 1
                    visible: !largeButton
                    Accessible.ignored: true
                }

                Rectangle {
                    visible: modernTile.zoned
                    x: 5 * modernTile.zoom
                    width: 4 * modernTile.zoom
                    height: modernCard.height - 36 * modernTile.zoom
                    anchors.verticalCenter: modernCard.verticalCenter
                    radius: width / 2
                    color: valueFontColor
                    Accessible.ignored: true
                }

                Image {
                    id: modernIcon
                    x: 12 * modernTile.zoom
                    y: 10 * modernTile.zoom
                    width: 20 * modernTile.zoom
                    height: 20 * modernTile.zoom
                    source: icon
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    visible: settings.theme_tile_icon_enabled && !largeButton
                    Accessible.ignored: true
                    // Some tile icons are plain black and some plain white, so either kind
                    // gets lost on one of the themes: paint those in the text colour.
                    // Coloured icons (heart, watt, kcal, resistance) stay as they are.
                    readonly property var monoIcons: ["cadence", "clock", "elevationgain", "fan", "inclination",
                                                      "joul", "odometer", "pace", "speed"]
                    // the file name of the icon without the folder and ".png"
                    readonly property string iconName: {
                        var s = String(source)
                        var name = s.substring(s.lastIndexOf("/") + 1)
                        return name.endsWith(".png") ? name.substring(0, name.length - 4) : ""
                    }
                    readonly property bool mono: monoIcons.indexOf(iconName) >= 0
                    layer.enabled: mono
                    layer.effect: ColorOverlay { color: window.ui.textMuted }
                }

                Text {
                    anchors.left: modernIcon.visible ? modernIcon.right : modernCard.left
                    anchors.leftMargin: (modernIcon.visible ? 6 : 12) * modernTile.zoom
                    anchors.right: modernCard.right
                    anchors.rightMargin: 10 * modernTile.zoom
                    anchors.verticalCenter: modernIcon.verticalCenter
                    text: name
                    color: window.ui.textMuted
                    font.pixelSize: 13 * modernTile.zoom
                    font.weight: Font.Medium
                    elide: Text.ElideRight
                    visible: !largeButton
                    Accessible.ignored: true
                }

                Text {
                    id: modernValue
                    anchors.horizontalCenter: modernCard.horizontalCenter
                    y: 34 * modernTile.zoom
                    width: Math.max(40, modernCard.width - (writable ? 100 : 20) * modernTile.zoom)
                    height: 50 * modernTile.zoom
                    text: value
                    color: modernTile.zoned ? window.ui.zoneInk(valueFontColor) : window.ui.textMain
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: valueFontSize * modernTile.zoom
                    fontSizeMode: Text.Fit
                    minimumPointSize: 10
                    font.bold: true
                    visible: !largeButton
                    Accessible.ignored: true
                }

                Text {
                    anchors.top: modernValue.bottom
                    anchors.horizontalCenter: modernCard.horizontalCenter
                    width: modernCard.width - 16 * modernTile.zoom
                    height: 26 * modernTile.zoom
                    text: secondLine
                    color: window.ui.textMuted
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: settings.theme_tile_secondline_textsize * modernTile.zoom
                    fontSizeMode: Text.Fit
                    minimumPointSize: 7
                    visible: !largeButton
                    Accessible.ignored: true
                }

                RoundButton {
                    id: modernMinus
                    objectName: minusName
                    autoRepeat: true
                    visible: writable && !largeButton
                    // A touch area of 64 x 80, easier to hit while riding, around the circle as it
                    // always looked: 32 (a 44 button less the 6 insets of Material), 12 from the
                    // edge, on the value's centre
                    x: (6 - 8) * modernTile.zoom
                    y: modernValue.y + modernValue.height / 2 + 3 * modernTile.zoom - height / 2
                    width: 64 * modernTile.zoom
                    height: 80 * modernTile.zoom
                    topInset: 0
                    bottomInset: 0
                    leftInset: 0
                    rightInset: 0
                    onClicked: minus_clicked(objectName)
                    background: Item {
                        Rectangle {
                            x: 14 * modernTile.zoom
                            y: parent.height / 2 - 19 * modernTile.zoom
                            width: 32 * modernTile.zoom
                            height: 32 * modernTile.zoom
                            radius: width / 2
                            color: modernMinus.down ? window.ui.alpha(window.ui.accent, 0.35) : window.ui.surfaceHighest
                            UiIcon {
                                anchors.centerIn: parent
                                width: 22 * modernTile.zoom
                                height: 22 * modernTile.zoom
                                name: "remove"
                                color: window.ui.textMain
                            }
                        }
                    }
                    contentItem: Item {}

                    Accessible.role: Accessible.Button
                    Accessible.name: qsTr("Decrease ") + name
                    Accessible.description: qsTr("Decrease the value of ") + name
                    Accessible.focusable: true
                    Accessible.onPressAction: { minus_clicked(objectName) }
                }

                RoundButton {
                    id: modernPlus
                    objectName: plusName
                    autoRepeat: true
                    visible: writable && !largeButton
                    // A touch area of 64 x 80, easier to hit while riding, around the circle as it
                    // always looked: 32 (a 44 button less the 6 insets of Material), 12 from the
                    // edge, on the value's centre
                    x: modernCard.width - width + (8 - 6) * modernTile.zoom
                    y: modernValue.y + modernValue.height / 2 + 3 * modernTile.zoom - height / 2
                    width: 64 * modernTile.zoom
                    height: 80 * modernTile.zoom
                    topInset: 0
                    bottomInset: 0
                    leftInset: 0
                    rightInset: 0
                    onClicked: plus_clicked(objectName)
                    background: Item {
                        Rectangle {
                            x: 18 * modernTile.zoom
                            y: parent.height / 2 - 19 * modernTile.zoom
                            width: 32 * modernTile.zoom
                            height: 32 * modernTile.zoom
                            radius: width / 2
                            color: modernPlus.down ? window.ui.alpha(window.ui.accent, 0.35) : window.ui.surfaceHighest
                            UiIcon {
                                anchors.centerIn: parent
                                width: 22 * modernTile.zoom
                                height: 22 * modernTile.zoom
                                name: "add"
                                color: window.ui.textMain
                            }
                        }
                    }
                    contentItem: Item {}

                    Accessible.role: Accessible.Button
                    Accessible.name: qsTr("Increase ") + name
                    Accessible.description: qsTr("Increase the value of ") + name
                    Accessible.focusable: true
                    Accessible.onPressAction: { plus_clicked(objectName) }
                }

                RoundButton {
                    id: modernLargeButton
                    objectName: identificator
                    autoRepeat: true
                    visible: largeButton
                    width: modernTile.width - 2 * modernTile.zoom
                    height: 123 * modernTile.zoom
                    onClicked: largeButton_clicked(objectName)
                    background: Rectangle {
                        radius: 16 * modernTile.zoom
                        color: largeButtonColor
                        opacity: modernLargeButton.down ? 0.75 : 1
                    }
                    contentItem: Text {
                        text: largeButtonLabel
                        color: "white"
                        font.pixelSize: 20 * modernTile.zoom
                        font.bold: true
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    Accessible.role: Accessible.Button
                    Accessible.name: largeButtonLabel
                    Accessible.description: name + ": " + largeButtonLabel
                    Accessible.focusable: true
                    Accessible.onPressAction: { largeButton_clicked(objectName) }
                }
            }
            }

            // Classic tile
            Component {
                id: classicTileComponent
            Item {
                anchors.fill: parent

            Rectangle {
                width: id1.width - 2 * settings.ui_zoom / 100
                height: 123 * settings.ui_zoom / 100
                radius: 3
                border.width: 1
                border.color: (settings.theme_tile_shadow_enabled ? settings.theme_tile_shadow_color : settings.theme_tile_background_color)
                color: settings.theme_tile_background_color
                id: rect

                // Ignore for VoiceOver - decorative background only
                Accessible.ignored: true
            }

            DropShadow {
                visible: settings.theme_tile_shadow_enabled
                anchors.fill: rect
                cached: true
                horizontalOffset: 3
                verticalOffset: 3
                radius: 8.0
                samples: 16
                color: settings.theme_tile_shadow_color
                source: rect
            }

            Timer {
                id: toggleIconTimer
                interval: 500; running: true; repeat: true
                onTriggered: { if(identificator === "inclination" && rootItem.autoInclinationEnabled()) myIcon.visible = !myIcon.visible; else myIcon.visible = settings.theme_tile_icon_enabled && !largeButton; }
            }

            Image {
                id: myIcon
                x: 5
                anchors {
                         // parent, not id1: the classic tile sits in a wrapper filling id1
                         bottom: parent.bottom
                }
                width: 48 * settings.ui_zoom / 100
                height: 48 * settings.ui_zoom / 100
                source: icon
                visible: settings.theme_tile_icon_enabled && !largeButton

                // Ignore for VoiceOver - decorative only
                Accessible.ignored: true
            }
            Text {
                objectName: "value"
                id: myValue
                color: valueFontColor
                y: 0
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }
                text: value
                horizontalAlignment: Text.AlignHCenter
                width: Math.max(50, parent.width - (writable ? 100 * settings.ui_zoom / 100 : 12 * settings.ui_zoom / 100))
                height: 58 * settings.ui_zoom / 100
                font.pointSize: valueFontSize * settings.ui_zoom / 100
                fontSizeMode: Text.Fit
                minimumPointSize: 10
                font.bold: true
                visible: !largeButton

                // Ignore for VoiceOver - parent Item handles accessibility
                Accessible.ignored: true
            }
            Text {
                objectName: "secondLine"
                id: secondLineText
                color: "white"
                y: myValue.bottom
                anchors {
                    top: myValue.bottom
                    horizontalCenter: parent.horizontalCenter
                }
                text: secondLine
                horizontalAlignment: Text.AlignHCenter
                width: Math.max(50, parent.width - 12 * settings.ui_zoom / 100)
                height: 24 * settings.ui_zoom / 100
                font.pointSize: settings.theme_tile_secondline_textsize * settings.ui_zoom / 100
                fontSizeMode: Text.Fit
                minimumPointSize: 7
                font.bold: false
                visible: !largeButton

                // Ignore for VoiceOver - parent Item handles accessibility
                Accessible.ignored: true
            }
            Text {
                id: myText
                anchors {
                    top: myIcon.top
                    horizontalCenter: settings.theme_tile_icon_enabled ? undefined : parent.horizontalCenter
                    left: settings.theme_tile_icon_enabled ? parent.left : undefined
                }
                font.bold: true
                font.pointSize: labelFontSize
                fontSizeMode: Text.Fit
                minimumPointSize: 8
                color: "white"
                text: name
                horizontalAlignment: settings.theme_tile_icon_enabled ? Text.AlignLeft : Text.AlignHCenter
                anchors.leftMargin: settings.theme_tile_icon_enabled ? 55 * settings.ui_zoom / 100 : 0
                width: Math.max(40, parent.width - (settings.theme_tile_icon_enabled ? 61 : 12) * settings.ui_zoom / 100)
                height: 40 * settings.ui_zoom / 100
                anchors.topMargin: 20 * settings.ui_zoom / 100
                visible: !largeButton

                // Ignore for VoiceOver - parent Item handles accessibility
                Accessible.ignored: true
            }
            RoundButton {
                objectName: minusName
                autoRepeat: true
                // the minus sign, not a hyphen: as wide as the "+" and on the same line
                text: "−"
                // the sign is a share of the button, so it grows with Tile size
                font.pixelSize: Math.round(height * 0.45)
                onClicked: minus_clicked(objectName)
                visible: writable && !largeButton
                anchors.top: myValue.top
                anchors.left: parent.left
                anchors.leftMargin: 2
                width: 48 * settings.ui_zoom / 100
                height: 48 * settings.ui_zoom / 100

                // VoiceOver accessibility
                Accessible.role: Accessible.Button
                Accessible.name: qsTr("Decrease ") + name
                Accessible.description: qsTr("Decrease the value of ") + name
                Accessible.focusable: true
                Accessible.onPressAction: { minus_clicked(objectName) }
            }
            RoundButton {
                autoRepeat: true
                objectName: plusName
                text: "+"
                font.pixelSize: Math.round(height * 0.45)
                onClicked: plus_clicked(objectName)
                visible: writable && !largeButton
                anchors.top: myValue.top
                anchors.right: parent.right
                anchors.rightMargin: 2
                width: 48 * settings.ui_zoom / 100
                height: 48 * settings.ui_zoom / 100

                // VoiceOver accessibility
                Accessible.role: Accessible.Button
                Accessible.name: qsTr("Increase ") + name
                Accessible.description: qsTr("Increase the value of ") + name
                Accessible.focusable: true
                Accessible.onPressAction: { plus_clicked(objectName) }
            }
            RoundButton {
                autoRepeat: true
                objectName: identificator
                text: largeButtonLabel
                onClicked: largeButton_clicked(objectName)
                visible: largeButton
                anchors.fill: rect
                      background: Rectangle {
                          color: largeButtonColor
                            radius: 20
                            }
                font.pointSize: 20 * settings.ui_zoom / 100

                // VoiceOver accessibility
                Accessible.role: Accessible.Button
                Accessible.name: largeButtonLabel
                Accessible.description: name + ": " + largeButtonLabel
                Accessible.focusable: true
                Accessible.onPressAction: { largeButton_clicked(objectName) }
            }
            }
            }
        }
    }

    footer: Item {
        id: footerItem
        width: parent.width
        height: footerHeight
        property real footerHeight: (rootItem.chartFooterVisible ? parent.height / 4 : parent.height / 2)
        property real minHeight: parent.height / 4
        property real maxHeight: parent.height * 3 / 4
        anchors.bottom: parent.bottom
        // Modern look: the shadow of the strip falls above the footer, so the chart and the
        // video clip themselves instead
        clip: !window.ui.modern
        visible: rootItem.chartFooterVisible || rootItem.videoVisible

        // Modern look: the strip is the top of a bottom sheet holding the chart (T-441) –
        // surfaceHigh with rounded top corners and a soft shadow above, a pill in the middle;
        // the whole strip is the touch area. Classic: the purple bar with three lines, as in master
        Rectangle {
            id: dragHandle
            width: window.ui.modern ? parent.width : parent.width / 5
            height: window.ui.modern ? 16 : 10
            color: window.ui.modern ? "transparent" : "#9C27B0"
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            visible: rootItem.chartFooterVisible || rootItem.videoVisible

            // Rounded at the top only: the lower corners reach under the chart, which covers them
            Rectangle {
                visible: window.ui.modern
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: parent.height + radius
                radius: 16
                color: window.ui.surfaceHigh
                layer.enabled: visible
                layer.effect: DropShadow {
                    transparentBorder: true
                    horizontalOffset: 0
                    verticalOffset: -1
                    radius: 6
                    samples: 13
                    color: Qt.rgba(0, 0, 0, window.ui.dark ? 0.45 : 0.10)
                }
            }

            Rectangle {
                visible: window.ui.modern
                width: 48
                height: 5
                radius: height / 2
                anchors.centerIn: parent
                color: Qt.rgba(window.ui.textMuted.r, window.ui.textMuted.g, window.ui.textMuted.b, 0.55)
            }

            Canvas {
                visible: !window.ui.modern
                anchors.fill: parent
                onPaint: {
                    var ctx = getContext("2d");
                    ctx.strokeStyle = "#FFFFFF";
                    ctx.lineWidth = 2;

                    for (var i = 0; i < 3; i++) {
                        ctx.beginPath();
                        ctx.moveTo(0, (i + 1) * parent.height / 4);
                        ctx.lineTo(parent.width, (i + 1) * parent.height / 4);
                        ctx.stroke();
                    }
                }
            }

            MouseArea {
                id: dragArea
                anchors.fill: parent
                cursorShape: Qt.SizeVerCursor

                property real startY: 0
                property real startHeight: 0

                onPressed: {
                    startY = mouseY
                    startHeight = footerItem.height
                }

                onMouseYChanged: {
                    if (pressed) {
                        var newHeight = Math.max(footerItem.minHeight, Math.min(footerItem.maxHeight, startHeight + startY - mouseY))
                        footerItem.footerHeight = newHeight
                    }
                }
            }
        }

        Rectangle {
            id: chartFooterRectangle
            visible: rootItem.chartFooterVisible
            clip: window.ui.modern
            anchors.top: dragHandle.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            ChartFooter {
                anchors.fill: parent
                visible: rootItem.chartFooterVisible
            }
        }

        Rectangle {
            objectName: "footerrectangle"
            visible: rootItem.videoVisible
            clip: window.ui.modern
            anchors.top: dragHandle.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            onVisibleChanged: {
                if(visible === true) {
                    console.log("mediaPlayer onCompleted: " + rootItem.videoPath)
                    console.log("videoRate: " + rootItem.videoRate)
                    videoPlaybackHalf.source = rootItem.videoPath
                    videoPlaybackHalf.seek(rootItem.videoPosition)
                    videoPlaybackHalf.play()
                    videoPlaybackHalf.muted = rootItem.currentCoordinateValid
                } else {
                    videoPlaybackHalf.stop()
                }
            }

            MediaPlayer {
                id: videoPlaybackHalf
                objectName: "videoplaybackhalf"
                autoPlay: false
                playbackRate: rootItem.videoRate

                onError: {
                    if (videoPlaybackHalf.NoError !== error) {
                        console.log("[qmlvideo] VideoItem.onError error " + error + " errorString " + errorString)
                    }
                }
            }

            VideoOutput {
                id: videoPlayer
                anchors.fill: parent
                source: videoPlaybackHalf
            }
        }
    }

    Item {
        id: ghostTile
        visible: loc.currentId !== -1 && window.lockTiles
        x: loc.mouseX - width / 2
        y: loc.mouseY - height / 2
        width: 85 * settings.ui_zoom / 100
        height: 63 * settings.ui_zoom / 100
        z: 200

        // UiFrame: in the modern look a thin border breaks up on Android
        UiFrame {
            anchors.fill: parent
            radius: window.ui.modern ? 12 : 3
            fill: window.ui.modern ? window.ui.surfaceHighest : settings.theme_tile_background_color
            opacity: 0.9
            strokeWidth: 2
            stroke: window.ui.modern ? window.ui.accent : settings.theme_tile_shadow_color

            Text {
                anchors.centerIn: parent
                color: window.ui.ink("white")
                text: loc.tileName
                font.pointSize: 10 * settings.ui_zoom / 100
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                width: parent.width - 8
                wrapMode: Text.WordWrap
            }
        }
    }

    Timer {
        id: autoScrollTimer
        interval: 50
        repeat: true
        running: false
        property real scrollSpeed: 15
        onTriggered: {
            if (loc.currentId === -1) { running = false; return; }
            var edgeZone = 80
            if (loc.mouseY > gridView.height - edgeZone) {
                var factor = (loc.mouseY - (gridView.height - edgeZone)) / edgeZone
                gridView.contentY = Math.min(
                    gridView.contentHeight - gridView.height,
                    gridView.contentY + scrollSpeed * (1 + factor * 2)
                )
            } else if (loc.mouseY < edgeZone) {
                var factor2 = (edgeZone - loc.mouseY) / edgeZone
                gridView.contentY = Math.max(-gridView.topMargin, gridView.contentY - scrollSpeed * (1 + factor2 * 2))
            } else {
                running = false
            }
        }
    }

    MouseArea {
        property int currentId: -1
        property int newIndex
        property int startIndex: -1
        property string tileName: ""
        property real lastScrollY: 0
        property bool isSwiping: false

        function indexAtMouse(mx, my) {
            var cols = gridView.tileColumns
            // The gap above the first row counts as the first row
            var adjustedY = Math.max(0, my + gridView.contentY)
            var col = Math.floor((mx - gridView.x) / gridView.cellWidth)
            if (col < 0 || col >= cols) return -1
            var row = Math.floor(adjustedY / gridView.cellHeight)
            var idx = row * cols + col
            if (idx < 0 || idx >= appModel.count) return -1
            return idx
        }

        id: loc
        enabled: window.lockTiles
        anchors.fill: parent

        onPressed: {
            lastScrollY = mouseY
            isSwiping = false
        }

        onPressAndHold: {
            if (isSwiping) return
            var idx = indexAtMouse(mouseX, mouseY)
            if (idx !== -1) {
                startIndex = idx
                newIndex = idx
                currentId = appModel[idx].gridId
                tileName = appModel[idx].name
            } else {
                currentId = -1
                tileName = ""
            }
        }

        onReleased: {
            autoScrollTimer.running = false
            if (currentId !== -1) {
                var idx = indexAtMouse(mouseX, mouseY)
                if (idx !== -1 && idx !== startIndex)
                    rootItem.moveTile(tileName, idx, startIndex)
            }
            currentId = -1
            startIndex = -1
            tileName = ""
            isSwiping = false
        }

        onPositionChanged: {
            if (currentId === -1) {
                // No drag in progress: scroll the grid like a normal flick
                var dy = mouseY - lastScrollY
                if (Math.abs(dy) > 5) isSwiping = true
                gridView.contentY = Math.max(-gridView.topMargin,
                    Math.min(gridView.contentHeight - gridView.height,
                             gridView.contentY - dy))
                lastScrollY = mouseY
                return
            }
            var edgeZone = 80
            if (mouseY > gridView.height - edgeZone || mouseY < edgeZone)
                autoScrollTimer.running = true
            else
                autoScrollTimer.running = false

            var idx = indexAtMouse(mouseX, mouseY)
            if (idx !== -1 && idx !== newIndex) newIndex = idx
        }
    }
}
