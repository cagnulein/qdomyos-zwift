import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtQuick.Dialogs 1.0
import QtGraphicalEffects 1.12
import Qt.labs.settings 1.0
import QtMultimedia 5.15
import org.cagnulein.qdomyoszwift 1.0
import QtQuick.Window 2.12
import Qt.labs.platform 1.1
import AndroidStatusBar 1.0

ApplicationWindow {
    id: window
    width: 640
    height: 480
    visibility: Qt.WindowFullScreen
    visible: true
	 objectName: "stack"
    title: qsTr("qDomyos-Zwift")
    
    // Force update on orientation change
    property int currentOrientation: Screen.orientation
    onCurrentOrientationChanged: {
        if (Qt.platform.os === "android") {
            console.log("Orientation changed to:", currentOrientation)
            // Force property binding updates by accessing the properties
            var temp = AndroidStatusBar.height + AndroidStatusBar.navigationBarHeight + AndroidStatusBar.leftInset + AndroidStatusBar.rightInset
        }
    }
    
    // Helper functions for cleaner padding calculations
    function getTopPadding() {
        if (Qt.platform.os !== "android" || AndroidStatusBar.apiLevel < 31) return 0;
        return (Screen.orientation === Qt.PortraitOrientation || Screen.orientation === Qt.InvertedPortraitOrientation) ? 
               AndroidStatusBar.height : AndroidStatusBar.leftInset;
    }
    
    function getBottomPadding() {
        if (Qt.platform.os !== "android" || AndroidStatusBar.apiLevel < 31) return 0;
        return (Screen.orientation === Qt.PortraitOrientation || Screen.orientation === Qt.InvertedPortraitOrientation) ? 
               AndroidStatusBar.navigationBarHeight : AndroidStatusBar.rightInset;
    }
    
    function getLeftPadding() {
        if (Qt.platform.os !== "android" || AndroidStatusBar.apiLevel < 31) return 0;
        return (Screen.orientation === Qt.LandscapeOrientation || Screen.orientation === Qt.InvertedLandscapeOrientation) ? 
               AndroidStatusBar.leftInset : 0;
    }
    
    function getRightPadding() {
        if (Qt.platform.os !== "android" || AndroidStatusBar.apiLevel < 31) return 0;
        return (Screen.orientation === Qt.LandscapeOrientation || Screen.orientation === Qt.InvertedLandscapeOrientation) ? 
               AndroidStatusBar.rightInset : 0;
    }

    signal gpx_open_clicked(url name)
    signal gpxpreview_open_clicked(url name)
    signal profile_open_clicked(url name)
    signal trainprogram_open_clicked(url name)
    signal fitfile_preview_clicked(url name)
    signal trainprogram_open_other_folder(url name)
    signal gpx_open_other_folder(url name)
    signal trainprogram_preview(url name)
    signal trainprogram_zwo_loaded(string s)
    signal fitfile_preview(string s)
    signal gpx_save_clicked()
    signal fit_save_clicked()
    signal refresh_bluetooth_devices_clicked()
    signal strava_connect_clicked()
    signal peloton_connect_clicked()
    signal loadSettings(url name)
    signal saveSettings(url name)
    signal deleteSettings(url name)
    signal restoreSettings()
    signal saveProfile(string profilename)
    signal restart()
    signal volumeUp()
    signal volumeDown()
    signal keyMediaPrevious()
    signal keyMediaNext()
    signal floatingOpen()
    signal openFloatingWindowBrowser();
    signal strava_upload_file_prepare();

    property bool lockTiles: false
    property bool settings_restart_to_apply: false

    Settings {
        id: settings
        property string profile_name: "default"        
        property string theme_status_bar_background_color: "#800080"
        property bool volume_change_gears: false
        property string peloton_username: "username"
        property string peloton_password: "password"
    }


    Store {
        id: iapStore
    }

    Loader {
      id: googleMapUI
      source:"GoogleMap.qml";
      active: false
      onLoaded: { console.log("googleMapUI loaded"); stackView.push(googleMapUI.item); }
    }

    // here in order to cache everything for the SwagBagView
    Product {
        id: productUnlockVowels
        type: Product.Unlockable
        store: iapStore
        identifier: "org.cagnulein.qdomyoszwift.swagbag"

        onPurchaseSucceeded: {
            console.log(identifier + " purchase successful");
            applicationData.vowelsUnlocked = true;
            transaction.finalize();
            pageStack.pop();
        }

        onPurchaseFailed: {
            console.log(identifier + " purchase failed");
            console.log("reason: "
                        + transaction.failureReason === Transaction.CanceledByUser ? "Canceled" : transaction.errorString);
            transaction.finalize();
        }

        onPurchaseRestored: {
            console.log(identifier + " purchase restored");
            applicationData.vowelsUnlocked = true;
            console.log("timestamp: " + transaction.timestamp);
            transaction.finalize();
            pageStack.pop();
        }
    }

    ToastManager {
        id: toast
    }

    Timer {
        interval: 1
        repeat: false
        running: (rootItem.toastRequested !== "")
        onTriggered: {
            toast.show(rootItem.toastRequested);
            rootItem.toastRequested = "";
        }
    }

    /*
    Timer {
        interval: 1000
        repeat: true
        running: true
        property int i: 0
        onTriggered: {
            toast.show("This timer has triggered " + (++i) + " times!");
        }
    }

    Timer {
        interval: 3000
        repeat: true
        running: true
        property int i: 0
        onTriggered: {
            toast.show("This important message has been shown " + (++i) + " times.", 5000);
        }
    }*/

    Keys.onBackPressed: {
        if(OS_VERSION === "Android") {
            toast.show("Pressed it quickly to close the app!")
            timer.pressBack();
        }
    }
    Timer{
        id: timer

        property bool backPressed: false
        repeat: false
        interval: 200//ms
        onTriggered: backPressed = false
        function pressBack(){
            if(backPressed){
                timer.stop()
                backPressed = false
                Qt.callLater(Qt.quit)
            }
            else{
                backPressed = true
                timer.start()
            }
        }
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

    MessageDialog {
           id: popupPelotonAuth
           text: "Peloton Authentication Change"
           informativeText: "Peloton has moved to a new authentication system. Username and password are no longer required.\n\nWould you like to switch to the new authentication method now?"
           buttons: (MessageDialog.Yes | MessageDialog.No)
           onYesClicked: {
               settings.peloton_username = "username"
               settings.peloton_password = "password"
               stackView.push("WebPelotonAuth.qml")
               peloton_connect_clicked()
           }
           onNoClicked: this.visible = false
           visible: false
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

    Popup {
        id: popupPelotonConnected
         parent: Overlay.overlay
         enabled: rootItem.pelotonPopupVisible
         onEnabledChanged: { if(rootItem.pelotonPopupVisible) popupPelotonConnected.open() }
         onClosed: { rootItem.pelotonPopupVisible = false; }

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
             text: qsTr("Your Peloton account is now connected!<br><br>Restart the app to apply this change!")
            }
         }
    }

    Timer {
        id: popupLicenseAutoClose
        interval: 120000; running: rootItem.licensePopupVisible; repeat: false
        onTriggered: popupLicense.close();
    }

    Popup {
        id: popupLicense
         parent: Overlay.overlay
         enabled: rootItem.licensePopupVisible
         onEnabledChanged: { if(rootItem.licensePopupVisible) popupLicense.open() }
         onClosed: { Qt.openUrlExternally("https://www.patreon.com/bePatron?u=45290147"); Qt.callLater(Qt.quit); }

         x: Math.round((parent.width - width) / 2)
         y: Math.round((parent.height - height) / 2)
         width: 580
         height: 230
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
             width: 570
             height: 220
             text: qsTr("Trial time expired!<br><br>Please join the QZ Patreon Membership to unlock the full license!<br>https://www.patreon.com/bePatron?u=45290147<br><br>Then add your patreon email in the email field in the general settings.<br>The App will now close.")
            }
         }
    }

    MessageDialog {
        id: popupRestartApp
        text: "Settings changed"
        informativeText: "In order to apply the changes you need to restart the app.\nDo you want to do it now?"
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: Qt.callLater(Qt.quit)
        onNoClicked: this.visible = false;
        visible: false
    }

    MessageDialog {
        text: "Strava"
        informativeText: "Do you want to upload the workout to Strava?"
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: {strava_upload_file_prepare(); rootItem.stravaUploadRequested = false;}
        onNoClicked: {rootItem.stravaUploadRequested = false;}
        visible: rootItem.stravaUploadRequested
    }

    header: ToolBar {
        contentHeight: toolButton.implicitHeight
        Material.primary: settings.theme_status_bar_background_color
        id: headerToolbar
        topPadding: getTopPadding()
        leftPadding: getLeftPadding()
        rightPadding: getRightPadding()

        ToolButton {
            id: toolButton
            icon.source: "icons/icons/icon.png"
            text: stackView.depth > 1 ? "◄" : "◄"
            font.pixelSize: Qt.application.font.pixelSize * 1.6
            onClicked: {
                if (stackView.depth > 1) {
                    if(window.settings_restart_to_apply === true) {
                        window.settings_restart_to_apply = false;
                        popupRestartApp.visible = true;
                    }

                    stackView.pop()
                    toolButtonLoadSettings.visible = false;
                    toolButtonSaveSettings.visible = false;
                    rootItem.sortTiles()
                } else {
                    drawer.open()
                }
            }
        }

        ToolButton {
            id: toolButtonFloating
            icon.source: "icons/icons/mini-display.png"
            onClicked: { console.log("floating!"); floatingOpen(); }
            anchors.left: toolButton.right
            visible: OS_VERSION === "Android" ? true : false
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
            function loadMaps() {
                if(rootItem.currentCoordinateValid) {
                    console.log("coordinate is valid for map");
                    if(googleMapUI.status === Loader.Ready)
                        stackView.push(googleMapUI.item);
                    else
                        googleMapUI.active = true;

                } else {
                    console.log("coordinate is NOT valid for map");
                }
            }
            id: toolButtonMaps
            icon.source: ( "icons/icons/maps-icon-16.png" )
            onClicked: { loadMaps(); }
            anchors.right: toolButtonChart.left
            visible: rootItem.mapsVisible
        }      

        ToolButton {
            function loadVideo() {
                if(rootItem.currentCoordinateValid || rootItem.trainProgramLoadedWithVideo) {
                    console.log("coordinate is valid for map");
                    //stackView.push("videoPlayback.qml");
                    rootItem.videoVisible = !rootItem.videoVisible
                } else {
                    console.log("coordinate is NOT valid for map");
                }
            }
            id: toolButtonVideo
            icon.source: ( "icons/icons/video.png" )
            onClicked: { loadVideo(); }
            anchors.right: toolButtonMaps.left
            visible: rootItem.videoIconVisible
        }

        ToolButton {
            id: toolButtonChart
            icon.source: ( "icons/icons/chart.png" )
            onClicked: { rootItem.chartFooterVisible = !rootItem.chartFooterVisible }
            anchors.right: toolButtonLockTiles.left
            visible: rootItem.chartIconVisible
        }

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
        topPadding: getTopPadding()
        bottomPadding: getBottomPadding()
        leftPadding: getLeftPadding()
        rightPadding: getRightPadding()

        ScrollView {
            contentWidth: -1
            focus: true
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.fill: parent

            Column {
                anchors.fill: parent

                ItemDelegate {
                    text: qsTr("Profile: ") + settings.profile_name
                    width: parent.width
                    onClicked: {
                        toolButtonLoadSettings.visible = true;
                        toolButtonSaveSettings.visible = true;
                        stackView.push("profiles.qml")
                        stackView.currentItem.profile_open_clicked.connect(profile_open_clicked)
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
                        stackView.currentItem.peloton_connect_clicked.connect(function() {
                            peloton_connect_clicked()
                         });
                        drawer.close()
                    }
                }

            ItemDelegate {
                text: qsTr("Workouts History")
                width: parent.width
                onClicked: {
                    stackView.push("WorkoutsHistory.qml")
                    stackView.currentItem.fitfile_preview_clicked.connect(fitfile_preview_clicked)
                    drawer.close()
                }
            }
                ItemDelegate {
                    text: qsTr("Swag Bag")
                    width: parent.width
                    onClicked: {
                        stackView.push("SwagBagView.qml")
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
                        stackView.push("GPXList.qml")
                        stackView.currentItem.trainprogram_open_clicked.connect(gpx_open_clicked)
                        stackView.currentItem.trainprogram_open_other_folder.connect(gpx_open_other_folder)
                        stackView.currentItem.trainprogram_preview.connect(gpxpreview_open_clicked)
                        stackView.currentItem.trainprogram_open_clicked.connect(function(url) {
                            stackView.pop();
                            popup.open();
                         });
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
                        stackView.currentItem.trainprogram_open_other_folder.connect(trainprogram_open_other_folder)
                        stackView.currentItem.trainprogram_preview.connect(trainprogram_preview)
                        stackView.currentItem.trainprogram_open_clicked.connect(function(url) {
                            stackView.pop();
                            popup.open();
                         });
                        drawer.close()
                    }
                }
                /*
                ItemDelegate {
                    text: qsTr("What's On Zwift™")
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
                    id: wizardItem
                    text: qsTr("Wizard")
                    width: parent.width
                    onClicked: {
                        stackView.push("Wizard.qml")
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
                    text: qsTr("Quit")
                    width: parent.width
                    visible: OS_VERSION === "Other" ? true : false
                    onClicked: {
                        console.log("closing...")
                        Qt.callLater(Qt.quit)
                    }
                }

                ItemDelegate {
                    text: "version 2.20.9"
                    width: parent.width
                }

                ItemDelegate {
                    id: strava_connect
                    Image {
                        anchors.left: parent.left;
                        anchors.verticalCenter: parent.verticalCenter
                        source: "icons/icons/btn_strava_connectwith_orange.png"
                        fillMode: Image.PreserveAspectFit
                        visible: true
                        width: parent.width
                    }
                    width: parent.width
                    onClicked: {
                        stackView.push("WebStravaAuth.qml")
                        strava_connect_clicked()
                        drawer.close()
                    }
                }

                ItemDelegate {
                    Image {
                        anchors.left: parent.left;
                        anchors.verticalCenter: parent.verticalCenter
                        source: "icons/icons/Button_Connect_Rect_DarkMode.png"
                        fillMode: Image.PreserveAspectFit
                        visible: true
                        width: parent.width
                    }
                    width: parent.width
                    onClicked: {
                        stackView.push("WebPelotonAuth.qml")
                        stackView.currentItem.goBack.connect(function() {
                            stackView.pop();
                        })
                        peloton_connect_clicked()
                        drawer.close()
                    }
                }

                    FileDialog {
                        id: fileDialogGPX
                         title: "Please choose a file"
                         folder: "file://" + rootItem.getWritableAppDir() + 'gpx'
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
    }    

    StackView {
        id: stackView
        initialItem: "Home.qml"
        anchors.fill: parent
        anchors.bottomMargin: (Screen.orientation === Qt.PortraitOrientation || Screen.orientation === Qt.InvertedPortraitOrientation) ? getBottomPadding() : 0
        anchors.rightMargin: getRightPadding()
        anchors.leftMargin: getLeftPadding()
        focus: true
        Keys.onVolumeUpPressed: (event)=> { console.log("onVolumeUpPressed"); volumeUp(); event.accepted = settings.volume_change_gears; }
        Keys.onVolumeDownPressed: (event)=> { console.log("onVolumeDownPressed"); volumeDown(); event.accepted = settings.volume_change_gears; }
        Keys.onPressed: (event)=> {
            if (event.key === Qt.Key_MediaPrevious)
                keyMediaPrevious();
            else if (event.key === Qt.Key_MediaNext)
                keyMediaNext();
            else if (OS_VERSION === "Other" && event.key === Qt.Key_Z)
                volumeDown();
            else if (OS_VERSION === "Other" && event.key === Qt.Key_X)
                volumeUp();

            event.accepted = settings.volume_change_gears;
        }
    }
    
    // Grupetto Legal Disclaimer
    GrupettoDisclaimer {
        id: grupettoDisclaimer
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
