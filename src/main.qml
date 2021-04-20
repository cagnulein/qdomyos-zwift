import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtQuick.Dialogs 1.0
import QtGraphicalEffects 1.12

ApplicationWindow {
    id: window
    width: 640
    height: 480
    visibility: Qt.WindowFullScreen
    visible: true
	 objectName: "stack"
    title: qsTr("Stack")

    signal trainprogram_open_clicked(url name)
	 signal gpx_open_clicked(url name)
    signal gpx_save_clicked()
    signal fit_save_clicked()
    signal refresh_bluetooth_devices_clicked()
    signal strava_connect_clicked()

    Popup {
	    id: popup
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
		     text: qsTr("Program has been loaded correctly. Press start to begin!")
			}
		 }
	}

    Popup {
        id: popupSaveFile
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
             text: qsTr("Saved! Check your private folder (Android)<br>or Files App (iOS)")
            }
         }
    }

    Popup {
        id: popupStravaConnected
         parent: Overlay.overlay
         enabled: rootItem.generalPopupVisible
         onEnabledChanged: { if(rootItem.generalPopupVisible) popupStravaConnected.open() }
         onClosed: { rootItem.generalPopupVisible = false; }

         x: Math.round((parent.width - width) / 2)
         y: Math.round((parent.height - height) / 2)
         width: 380
         height: 120
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
             width: 370
             height: 120
             text: qsTr("Your Strava account is now connected!<br><br>When you will save a FIT file it will<br>automatically uploaded to Strava!")
            }
         }
    }

    header: ToolBar {
        contentHeight: toolButton.implicitHeight
        Material.primary: Material.Purple
        id: headerToolbar

        ToolButton {
            id: toolButton
            icon.source: "icons/icons/icon.png"
            text: stackView.depth > 1 ? "\u25C0" : "\u2630"
            font.pixelSize: Qt.application.font.pixelSize * 1.6
            onClicked: {
                if (stackView.depth > 1) {
                    stackView.pop()
                } else {
                    drawer.open()
                }
            }
        }

        Popup {
            id: popupAutoResistance
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
                 text: qsTr("Auto Resistance " + (rootItem.autoResistance?"enabled":"disabled"))
                }
             }
        }

        Timer {
            id: popupAutoResistanceAutoClose
            interval: 2000; running: false; repeat: false
            onTriggered: popupAutoResistance.close();
        }

        ToolButton {
            id: toolButtonAutoResistance
            icon.source: ( rootItem.autoResistance ? "icons/icons/resistance.png" : "icons/icons/pause.png")
            onClicked: { rootItem.autoResistance = !rootItem.autoResistance; console.log("auto resistance toggled " + rootItem.autoResistance); popupAutoResistance.open(); popupAutoResistanceAutoClose.running = true; }
            anchors.right: parent.right
        }

        Label {
            text: stackView.currentItem.title
            anchors.centerIn: parent
        }
    }

    Drawer {
        id: drawer
        width: window.width * 0.66
        height: window.height

        Column {
            anchors.fill: parent

            ItemDelegate {
                text: qsTr("Settings")
                width: parent.width
                onClicked: {
                    stackView.push("settings.qml")
                    drawer.close()
                }
            }
            ItemDelegate {
                text: qsTr("Charts")
                width: parent.width
                onClicked: {
                    stackView.push("ChartsEndWorkout.qml")
                    drawer.close()
                }
            }
            ItemDelegate {
                id: gpx_open
                text: qsTr("Open GPX")
                width: parent.width
                onClicked: {
					     fileDialogGPX.visible = true
                    drawer.close()
                }
            }
            ItemDelegate {
                id: trainprogram_open
                text: qsTr("Open Train Program")
                width: parent.width
                onClicked: {
					     fileDialogTrainProgram.visible = true
                    drawer.close()
                }
            }
            ItemDelegate {
                id: gpx_save
                text: qsTr("Save GPX")
                width: parent.width
                onClicked: {
                    gpx_save_clicked()
                    drawer.close()
                    popupSaveFile.open()
                }
            }
            ItemDelegate {
                id: fit_save
                text: qsTr("Save FIT")
                width: parent.width
                onClicked: {
                    fit_save_clicked()
                    drawer.close()
                    popupSaveFile.open()
                }
            }
            ItemDelegate {
                id: strava_connect
                text: qsTr("Connect to Strava")
                width: parent.width
                onClicked: {
                    strava_connect_clicked()
                    drawer.close()
                }
            }
            ItemDelegate {
                id: help
                text: qsTr("Help")
                width: parent.width
                onClicked: {
                    Qt.openUrlExternally("https://robertoviola.cloud/qdomyos-zwift-guide/");
                    drawer.close()
                }
            }
            ItemDelegate {
                id: community
                text: qsTr("Community")
                width: parent.width
                onClicked: {
                    Qt.openUrlExternally("https://www.facebook.com/groups/149984563348738");
                    drawer.close()
                }
            }
            ItemDelegate {
                text: qsTr("by Roberto Viola")
                width: parent.width
            }
            ItemDelegate {
                text: "version " + Qt.application.version
                width: parent.width
            }
            FileDialog {
				    id: fileDialogTrainProgram
                title: "Please choose a file"
                folder: shortcuts.home
                onAccepted: {
					     console.log("You chose: " + fileDialogTrainProgram.fileUrl)
						  trainprogram_open_clicked(fileDialogTrainProgram.fileUrl)
						  fileDialogTrainProgram.close()
						  popup.open()
                }
                onRejected: {
                    console.log("Canceled")
						  fileDialogTrainProgram.close()
                }
            }
				FileDialog {
				    id: fileDialogGPX
					 title: "Please choose a file"
					 folder: shortcuts.home
					 onAccepted: {
					     console.log("You chose: " + fileDialogGPX.fileUrl)
						  gpx_open_clicked(fileDialogGPX.fileUrl)
						  fileDialogGPX.close()
						  popup.open()
						}
					 onRejected: {
					     console.log("Canceled")
						  fileDialogGPX.close()
						}
					}
        }
    }

    StackView {
        id: stackView
        initialItem: "Home.qml"
        anchors.fill: parent
    }
}
