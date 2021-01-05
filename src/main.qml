import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtQuick.Dialogs 1.0

ApplicationWindow {
    id: window
    width: 640
    height: 480
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
		 height: 50
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
                }
            }
            ItemDelegate {
                id: fit_save
                text: qsTr("Save FIT")
                width: parent.width
                onClicked: {
                    fit_save_clicked()
                    drawer.close()
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
                text: qsTr("by Roberto Viola")
                width: parent.width
/*                onClicked: {
                    stackView.push("Page2Form.ui.qml")
                    drawer.close()
                }*/
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
