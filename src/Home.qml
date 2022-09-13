import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtGraphicalEffects 1.12
import QtQuick.Window 2.12
import Qt.labs.settings 1.0
import Qt.labs.platform 1.1
import QtMultimedia 5.15

HomeForm{
    objectName: "home"
    signal start_clicked;
    signal stop_clicked;
    signal lap_clicked;
    signal peloton_start_workout;
    signal peloton_abort_workout;
    signal plus_clicked(string name)
    signal minus_clicked(string name)

    Settings {
        id: settings
        property real ui_zoom: 100.0
    }

    MessageDialog {
        id: messagePelotonAskStart
        text: "Peloton Workout in progress"
        informativeText: "Do you want to follow the resistance? " + rootItem.pelotonProvider
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

    Timer {
        id: popupLapAutoClose
        interval: 2000; running: false; repeat: false
        onTriggered: popupLap.close();
    }

    start.onClicked: { start_clicked(); }
    stop.onClicked: {
        stop_clicked();
        rootItem.save_screenshot();
        if(CHARTJS)
            stackView.push("ChartJsTest.qml")
        else
            stackView.push("ChartsEndWorkout.qml")
    }
    lap.onClicked: { lap_clicked(); popupLap.open(); popupLapAutoClose.running = true; }

    Component.onCompleted: { console.log("completed"); }

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

            //        highlight: Rectangle {
            //            width: 150
            //           height: 150
            //            color: "lightsteelblue"
            //        }
            delegate: Item {
                id: id1
                width: 170 * settings.ui_zoom / 100
                height: 125 * settings.ui_zoom / 100

                visible: visibleItem
                Component.onCompleted: console.log("completed " + objectName)

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
                    PropertyChanges { target: id1; x: loc.mouseX - gridView.x - width/2; y: loc.mouseY - gridView.y - height/2; scale: 0.5; z: 10 }
                }

                transitions: Transition { NumberAnimation { property: "scale"; duration: 200} }

                Rectangle {
                    width: 168 * settings.ui_zoom / 100
                    height: 123 * settings.ui_zoom / 100
                    radius: 3
                    border.width: 1
                    border.color: "purple"
                    color: Material.backgroundColor
                    id: rect
                }

                DropShadow {
                    anchors.fill: rect
                    cached: true
                    horizontalOffset: 3
                    verticalOffset: 3
                    radius: 8.0
                    samples: 16
                    color: Material.color(Material.Purple)
                    source: rect
                }

                Timer {
                    id: toggleIconTimer
                    interval: 500; running: true; repeat: true
                    onTriggered: { if(identificator === "inclination" && rootItem.autoInclinationEnabled()) myIcon.visible = !myIcon.visible; else myIcon.visible = true; }
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
                    font.pointSize: valueFontSize * settings.ui_zoom / 100
                    font.bold: true
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
                    font.pointSize: 12 * settings.ui_zoom / 100
                    font.bold: false
                }
                Text {
                    id: myText
                    anchors {
                        top: myIcon.top
                    }
                    font.bold: true
                         font.pointSize: labelFontSize
                    color: "white"
                    text: name
                    anchors.left: parent.left
                    anchors.leftMargin: 55 * settings.ui_zoom / 100
                    anchors.topMargin: 20 * settings.ui_zoom / 100
                }
                RoundButton {
                    objectName: minusName
                    autoRepeat: true
                    text: "-"
                    onClicked: minus_clicked(objectName)
                    visible: writable
                    anchors.top: myValue.top
                    anchors.left: parent.left
                    anchors.leftMargin: 2
                    width: 48 * settings.ui_zoom / 100
                    height: 48 * settings.ui_zoom / 100
                }
                RoundButton {
                    autoRepeat: true
                    objectName: plusName
                    text: "+"
                    onClicked: plus_clicked(objectName)
                    visible: writable
                    anchors.top: myValue.top
                    anchors.right: parent.right
                    anchors.rightMargin: 2
                    width: 48 * settings.ui_zoom / 100
                    height: 48 * settings.ui_zoom / 100
                }

                /*MouseArea {
                    anchors.fill: parent
                    onClicked: parent.GridView.view.currentIndex = index
                }*/
            }
        }

        footer:
            Rectangle {
                visible: showVideo
                anchors.top: gridView.bottom
                width: parent.width
                height: parent.height / 2

                Timer {
                    id: pauseTimer
                    interval: 1000; running: true; repeat: true
                    onTriggered: { if(visible == true) { (rootItem.currentSpeed > 0  ?
                                        videoPlaybackHalf.play() :
                                        videoPlaybackHalf.pause()) } }
                }

                onVisibleChanged: {
                    if(visible === true) {
                        console.log("mediaPlayer onCompleted: " + rootItem.videoPath)
                        console.log("videoRate: " + rootItem.videoRate)
                        videoPlaybackHalf.source = rootItem.videoPath
                        //videoPlaybackHalf.playbackRate = rootItem.videoRate

                        videoPlaybackHalf.seek(rootItem.videoPosition)
                        videoPlaybackHalf.play()
                        videoPlaybackHalf.muted = true
                    } else {
                        videoPlaybackHalf.stop()
                    }

                }

                MediaPlayer {
                       id: videoPlaybackHalf
                       autoPlay: false
                       playbackRate: rootItem.videoRate

                       onError: {
                           if (videoPlaybackHalf.NoError !== error) {
                               console.log("[qmlvideo] VideoItem.onError error " + error + " errorString " + errorString)
                           }
                       }

                   }

                VideoOutput {
                         id:videoPlayer
                         anchors.fill: parent
                         source: videoPlaybackHalf
                     }
            }

    MouseArea {
        property int currentId: -1 // Original position in model
        property int newIndex // Current Position in model
        property int index:  (Math.floor(gridView.width / gridView.cellWidth) * Math.floor(mouseY / gridView.cellHeight)) + Math.floor(mouseX / gridView.cellWidth)  //  gridView.indexAt(mouseX - gridView.x, mouseY - gridView.y) // Item underneath cursor

        id: loc
        enabled: window.lockTiles
        anchors.fill: parent
        onPressAndHold: { console.log("onPressAndHold " + index); if(index !== -1) currentId = appModel[newIndex = index].gridId; else currentId = -1; }
        onReleased: {
            console.log("onReleased " + currentId + " " + index );
            if (currentId !== -1 && index !== -1 && index !== newIndex) {
                rootItem.moveTile(appModel[currentId].name, index, newIndex);
            } currentId = -1
        }

        onPositionChanged: {
            console.log("onPositionChanged " + currentId + " " + index + " " + newIndex + " " + mouseX + " " + mouseY)
            if (currentId !== -1 && index !== -1 && index !== newIndex) {
                //appModel.move(newIndex, newIndex = index)
            }
        }
    }
}
