import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtQuick.Dialogs 1.0
import QtGraphicalEffects 1.12
import Qt.labs.settings 1.0

ApplicationWindow {
    id: window
    width: 640
    height: 480
    visibility: Qt.WindowFullScreen
    visible: true
	 objectName: "stack"
    title: qsTr("Stack")

    signal gpx_open_clicked(url name)
    signal trainprogram_open_clicked(url name)
    signal trainprogram_zwo_loaded(string s)
    signal gpx_save_clicked()
    signal fit_save_clicked()
    signal refresh_bluetooth_devices_clicked()
    signal strava_connect_clicked()
    signal loadSettings(url name)
    signal saveSettings(url name)
    signal deleteSettings(url name)
    signal saveProfile()
    signal restart()
    signal volumeUp()
    signal volumeDown()

    property bool lockTiles: false

    Settings {
        id: settings
        property string profile_name: "default"
    }

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
        id: popupClassificaHelper
         parent: Overlay.overlay

       x: Math.round((parent.width - width) / 2)
         y: Math.round((parent.height - height) / 2)
         width: 380
         height: 130
         modal: true
         focus: true
         palette.text: "white"
         onClosed: stackView.push("Classifica.qml");
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
             text: qsTr("QZ Classifica is a realtime viewer about the actual\neffort of every QZ users! If you want to join in,\nchoose a nickname in the general settings\nand enable the QZ Classifica setting in the\nexperimental settings section and\nrestart the app.")
            }
         }
    }

    Popup {
        id: popupWhatsOnZwiftHelper
         parent: Overlay.overlay

       x: Math.round((parent.width - width) / 2)
         y: Math.round((parent.height - height) / 2)
         width: 380
         height: 130
         modal: true
         focus: true
         palette.text: "white"
         closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
         onClosed: {
             stackView.push("WebEngineTest.qml")
             drawer.close()
             stackView.currentItem.trainprogram_zwo_loaded.connect(trainprogram_zwo_loaded)
             stackView.currentItem.trainprogram_zwo_loaded.connect(function(s) {
                 stackView.pop();
              });
         }

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
             text: qsTr("Browse the What's on Zwift workout library<br>and choose your workout. It will<br> be automatically loaded on QZ when you will<br>press the load button on the top!<br><br>QZ is not affiliated with Zwift<br>or https://whatsonzwift.com/ website.")
            }
         }
    }

    Popup {
        id: popupLoadSettings
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
             text: qsTr("Settings has been loaded correctly. Restart the app!")
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
                    toolButtonLoadSettings.visible = false;
                    toolButtonSaveSettings.visible = false;
                    rootItem.sortTiles()
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

        Popup {
            id: popuplockTiles
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
                 text: window.lockTiles ? qsTr("You can move the tiles!") : qsTr("The tiles are locked now")
                }
             }
        }

        Timer {
            id: popuplockTilesAutoClose
            interval: 2000; running: false; repeat: false
            onTriggered: popuplockTiles.close();
        }

        ToolButton {
            id: toolButtonLoadSettings
            icon.source: "icons/icons/tray-arrow-up.png"
            onClicked: {
                stackView.push("SettingsList.qml")
                stackView.currentItem.loadSettings.connect(loadSettings)
                stackView.currentItem.loadSettings.connect(function(url) {
                    stackView.pop();
                    if (stackView.depth > 1) {
                        stackView.pop()
                    }
                    popupLoadSettings.open();
                 });
                drawer.close()
            }
            anchors.right: toolButtonSaveSettings.left
            visible: false
        }

        ToolButton {
            id: toolButtonSaveSettings
            icon.source: "icons/icons/tray-arrow-down.png"
            onClicked: {
                saveSettings("settings");
                popupSaveFile.open()
            }
            anchors.right: toolButtonAutoResistance.left/*toolClassifica.left*/
            visible: false
        }

        /*ToolButton {
            id: toolClassifica
            icon.source: "icons/icons/chart.png"
            onClicked: {  if(settings.classifica_enable) stackView.push("Classifica.qml"); else popupClassificaHelper.open(); }
            anchors.right: toolButtonAutoResistance.left
        }*/

        ToolButton {
            id: toolButtonLockTiles
            icon.source: ( window.lockTiles ? "icons/icons/unlock.png" : "icons/icons/lock.png")
            onClicked: { window.lockTiles = !window.lockTiles; console.log("lock tiles toggled " + window.lockTiles); popuplockTiles.open(); popuplockTilesAutoClose.running = true; }
            anchors.right: toolButtonAutoResistance.left
            visible: !toolButtonSaveSettings.visible
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
                text: qsTr("Profile: ") + settings.profile_name
                width: parent.width
                onClicked: {
                    toolButtonLoadSettings.visible = true;
                    toolButtonSaveSettings.visible = true;
                    stackView.push("profiles.qml")
                    drawer.close()
                }
            }

            ItemDelegate {
                text: qsTr("Settings")
                width: parent.width
                onClicked: {
                    toolButtonLoadSettings.visible = true;
                    toolButtonSaveSettings.visible = true;
                    stackView.push("settings.qml")                    
                    drawer.close()                    
                }
            }
            ItemDelegate {
                text: qsTr("Charts")
                width: parent.width
                onClicked: {
                    console.log(CHARTJS)
                    if(CHARTJS)
                        stackView.push("ChartJsTest.qml")
                    else
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
                    stackView.push("TrainingProgramsList.qml")
                    stackView.currentItem.trainprogram_open_clicked.connect(trainprogram_open_clicked)
                    stackView.currentItem.trainprogram_open_clicked.connect(function(url) {
                        stackView.pop();
                        popup.open();
                     });
                    drawer.close()
                }
            }
            /*
            ItemDelegate {
                text: qsTr("Whats On Zwift™")
                width: parent.width
                onClicked: {
                    popupWhatsOnZwiftHelper.open()
                }
            }*/

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
                text: qsTr("Credits")
                width: parent.width
                onClicked: {
                    stackView.push("Credits.qml")
                    drawer.close()
                }
            }
            ItemDelegate {
                text: "version 2.10.45"
                width: parent.width
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
        focus: true
        Keys.onVolumeUpPressed: { console.log("onVolumeUpPressed"); volumeUp(); }
        Keys.onVolumeDownPressed: { console.log("onVolumeDownPressed"); volumeDown(); }
    }
}
