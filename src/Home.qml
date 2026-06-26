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
    background: Rectangle {
        anchors.fill: parent
        width: parent.fill
        height: parent.fill
        color: settings.theme_background_color

        // VoiceOver accessibility - ignore decorative background
        Accessible.role: Accessible.Pane
        Accessible.ignored: true
    }
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
        property real ui_zoom: 100.0
        property bool theme_tile_icon_enabled: true
        property string theme_tile_background_color: "#303030"
        property string theme_background_color: "#303030"
        property bool theme_tile_shadow_enabled: true
        property string theme_tile_shadow_color: "#9C27B0"
        property int theme_tile_secondline_textsize: 12
        property bool skipLocationServicesDialog: false
    }

    MessageDialog {
        id: messagePelotonAskStart
        text: qsTr("Peloton Workout in progress")
        informativeText: qsTr("Do you want to follow the resistance? ") + rootItem.pelotonProvider
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: {rootItem.pelotonAskStart = false; peloton_start_workout();}
        onNoClicked: {rootItem.pelotonAskStart = false; peloton_abort_workout();}
        visible: rootItem.pelotonAskStart
    }

    Popup {
        id: popupLap
         parent: Overlay.overlay

         x: Math.round((parent.width - width) / 2)
         y: Math.round((parent.height - height) / 2)
         width: 380
         height: 60
         modal: true
         focus: true
         palette.text: "white"
         closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
         enter: Transition
         {
             NumberAnimation { property: "opacity"; from: 0.0; to: 1.0 }
         }
         exit: Transition
         {
             NumberAnimation { property: "opacity"; from: 1.0; to: 0.0 }
         }
         Column {
             anchors.horizontalCenter: parent.horizontalCenter
         Label {
             anchors.horizontalCenter: parent.horizontalCenter
             text: qsTr("New lap started!")
            }
        }
    }

    MessageDialog {
        id: stopConfirmationDialog
        text: qsTr("Stop Workout")
        informativeText: qsTr("Do you really want to stop the current workout?")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: {
            close();
            inner_stop();
        }
        onNoClicked: close()
    }

    Timer {
        id: popupLapAutoClose
        interval: 2000; running: false; repeat: false
        onTriggered: popupLap.close();
    }

    Timer {
        id: checkStartStopFromWeb
        interval: 200; running: true; repeat: true
        onTriggered: {if(rootItem.stopRequested) {rootItem.stopRequested = false; inner_stop(); }}
    }

    property bool locationServiceRequsted: false

    MessageDialog {
        id: locationServicesDialog
        text: qsTr("Permissions Required")
        informativeText: qsTr("QZ requires both Bluetooth and Location Services to be enabled.\nLocation Services are necessary on Android to allow the app to find Bluetooth devices.\nThe GPS will not be used.\n\nWould you like to enable them?")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: {
            locationServiceRequsted = true
            rootItem.enableLocationServices()
        }
        onNoClicked: remindLocationServicesDialog.visible = true
        visible: !rootItem.locationServices() && !locationServiceRequsted && !settings.skipLocationServicesDialog
    }

    MessageDialog {
        id: remindLocationServicesDialog
        text: qsTr("Reminder Preference")
        informativeText: qsTr("Would you like to be reminded about enabling Location Services next time?")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: settings.skipLocationServicesDialog = false
        onNoClicked: settings.skipLocationServicesDialog = true
        visible: false
    }

    MessageDialog {
        text: qsTr("Restart the app")
        informativeText: qsTr("To apply the changes, you need to restart the app.\nWould you like to do that now?")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: Qt.callLater(Qt.quit)
        onNoClicked: this.visible = false;
        visible: locationServiceRequsted
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

    GridView {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.fill: parent
        cellWidth: 175 * settings.ui_zoom / 100
        cellHeight: 130 * settings.ui_zoom / 100
        focus: true
        model: appModel
        leftMargin: { if(OS_VERSION === "Android") (Screen.width % cellWidth) / 2; else (parent.width % cellWidth) / 2; }
        anchors.topMargin: (!window.lockTiles ? rootItem.topBarHeight + 30 : 0)
        id: gridView
        objectName: "gridview"
        onMovementEnded: { headerToolbar.visible = (contentY == 0) || window.lockTiles; }
        Screen.orientationUpdateMask:  Qt.LandscapeOrientation | Qt.PortraitOrientation
        Screen.onPrimaryOrientationChanged:{
            if(OS_VERSION === "Android")
                gridView.leftMargin = (Screen.width % cellWidth) / 2;
            else
                gridView.leftMargin = (parent.width % cellWidth) / 2;
        }

        Accessible.ignored: true

        delegate: Item {
            id: id1
            width: 170 * settings.ui_zoom / 100
            height: 125 * settings.ui_zoom / 100

            visible: visibleItem
            Component.onCompleted: console.log("completed " + objectName)

            // VoiceOver accessibility support
            Accessible.role: largeButton ? Accessible.Button : (writable ? Accessible.Pane : Accessible.StaticText)
            Accessible.name: name + (largeButton ? "" : (": " + value))
            Accessible.description: largeButton ? largeButtonLabel : (secondLine !== "" ? secondLine : (writable ? qsTr("Adjustable. Current value: ") + value : qsTr("Current value: ") + value))
            Accessible.focusable: true

            Behavior on x {
                enabled: id1.state != "active"
                NumberAnimation { duration: 400; easing.type: Easing.OutBack }
            }

            Behavior on y {
                enabled: id1.state != "active"
                NumberAnimation { duration: 400; easing.type: Easing.OutBack }
            }

            SequentialAnimation on rotation {
                NumberAnimation { to:  2; duration: 60 }
                NumberAnimation { to: -2; duration: 120 }
                NumberAnimation { to:  0; duration: 60 }
                running: loc.currentId !== -1 && id1.state !== "active" && window.lockTiles
                loops: Animation.Infinite; alwaysRunToEnd: true
            }

            states: State {
                name: "active"; when: loc.currentId === gridId && window.lockTiles
                PropertyChanges { target: id1; x: loc.mouseX - gridView.x - width/2; y: loc.mouseY - gridView.y - height/2 + gridView.contentY; scale: 0.5; z: 10 }
            }

            transitions: Transition { NumberAnimation { property: "scale"; duration: 200} }

            Rectangle {
                width: 168 * settings.ui_zoom / 100
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
                         bottom: id1.bottom
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
                }
                font.bold: true
                font.pointSize: labelFontSize
                fontSizeMode: Text.Fit
                minimumPointSize: 8
                color: "white"
                text: name
                anchors.left: parent.left
                anchors.leftMargin: 55 * settings.ui_zoom / 100
                width: Math.max(40, parent.width - 61 * settings.ui_zoom / 100)
                height: 40 * settings.ui_zoom / 100
                anchors.topMargin: 20 * settings.ui_zoom / 100
                visible: !largeButton

                // Ignore for VoiceOver - parent Item handles accessibility
                Accessible.ignored: true
            }
            RoundButton {
                objectName: minusName
                autoRepeat: true
                text: "-"
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

    footer: Item {
        id: footerItem
        width: parent.width
        height: footerHeight
        property real footerHeight: (rootItem.chartFooterVisible ? parent.height / 4 : parent.height / 2)
        property real minHeight: parent.height / 4
        property real maxHeight: parent.height * 3 / 4
        anchors.bottom: parent.bottom
        clip: true
        visible: rootItem.chartFooterVisible || rootItem.videoVisible

        Rectangle {
            id: dragHandle
            width: parent.width / 5
            height: 10
            color: "#9C27B0"
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            visible: rootItem.chartFooterVisible || rootItem.videoVisible

            Canvas {
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
                gridView.contentY = Math.max(0, gridView.contentY - scrollSpeed * (1 + factor2 * 2))
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

        function indexAtMouse(mx, my) {
            var cols = Math.max(1, Math.floor(gridView.width / gridView.cellWidth))
            var adjustedY = my + gridView.contentY
            var col = Math.floor(mx / gridView.cellWidth)
            var row = Math.floor(adjustedY / gridView.cellHeight)
            var idx = row * cols + col
            if (idx < 0 || idx >= appModel.count) return -1
            return idx
        }

        id: loc
        enabled: window.lockTiles
        anchors.fill: parent

        onPressAndHold: {
            var idx = indexAtMouse(mouseX, mouseY)
            console.log("onPressAndHold " + idx)
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
            var idx = indexAtMouse(mouseX, mouseY)
            console.log("onReleased tileName=" + tileName + " idx=" + idx + " startIndex=" + startIndex)
            if (currentId !== -1 && idx !== -1 && idx !== startIndex) {
                rootItem.moveTile(tileName, idx, startIndex)
            }
            currentId = -1
            startIndex = -1
            tileName = ""
        }

        onPositionChanged: {
            if (currentId === -1) return
            var edgeZone = 80
            if (mouseY > gridView.height - edgeZone || mouseY < edgeZone)
                autoScrollTimer.running = true
            else
                autoScrollTimer.running = false

            var idx = indexAtMouse(mouseX, mouseY)
            console.log("onPositionChanged " + currentId + " " + idx + " " + newIndex + " " + mouseX + " " + mouseY)
            if (idx !== -1 && idx !== newIndex) {
                newIndex = idx
            }
        }
    }
}
