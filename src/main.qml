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
    title: Qt.platform.os === "ios" ? "" : qsTr("qDomyos-Zwift")

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
        // Add padding for iPadOS multi-window mode (Stage Manager, Split View, Slide Over)
        // to avoid overlap with window control buttons (red/yellow/green)
        // Check both the native detection and window size comparison for reactivity
        if (Qt.platform.os === "ios") {
            var isMultiWindow = (typeof rootItem !== "undefined" && rootItem && rootItem.iPadMultiWindowMode) ||
                                (window.width < Screen.width - 10);  // Window smaller than screen = multi-window
            if (isMultiWindow) {
                return 15;  // Space for window control buttons
            }
        }
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

    function isConfiguringShortcuts() {
        // Check if a TextField in the shortcuts settings has active focus
        // This prevents global shortcuts from intercepting key presses when configuring them
        var focusItem = window.activeFocusItem;
        if (!focusItem) return false;

        // Walk up the parent hierarchy to check if we're inside settingsShortcutsPane
        var current = focusItem;
        while (current) {
            if (current.objectName === "settingsShortcutsPane") {
                return true;
            }
            current = current.parent;
        }
        return false;
    }

    function shortcutReady(sequence) {
        return settings.shortcuts_enabled && !isConfiguringShortcuts() && String(sequence).length > 0;
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
    signal trainprogram_autostart_requested()
    signal fitfile_preview(string s)
    signal gpx_save_clicked()
    signal fit_save_clicked()
    signal refresh_bluetooth_devices_clicked()
    signal strava_connect_clicked()
    signal peloton_connect_clicked()
    signal intervalsicu_connect_clicked()
    signal intervalsicu_download_todays_workout_clicked()
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

        property bool shortcuts_enabled: false
        property string shortcut_speed_plus: ""
        property string shortcut_speed_minus: ""
        property string shortcut_inclination_plus: ""
        property string shortcut_inclination_minus: ""
        property string shortcut_resistance_plus: ""
        property string shortcut_resistance_minus: ""
        property string shortcut_peloton_resistance_plus: ""
        property string shortcut_peloton_resistance_minus: ""
        property string shortcut_target_resistance_plus: ""
        property string shortcut_target_resistance_minus: ""
        property string shortcut_target_power_plus: ""
        property string shortcut_target_power_minus: ""
        property string shortcut_target_zone_plus: ""
        property string shortcut_target_zone_minus: ""
        property string shortcut_target_speed_plus: ""
        property string shortcut_target_speed_minus: ""
        property string shortcut_target_incline_plus: ""
        property string shortcut_target_incline_minus: ""
        property string shortcut_fan_plus: ""
        property string shortcut_fan_minus: ""
        property string shortcut_peloton_offset_plus: ""
        property string shortcut_peloton_offset_minus: ""
        property string shortcut_peloton_remaining_plus: ""
        property string shortcut_peloton_remaining_minus: ""
        property string shortcut_remaining_time_plus: ""
        property string shortcut_remaining_time_minus: ""
        property string shortcut_gears_plus: ""
        property string shortcut_gears_minus: ""
        property string shortcut_pid_hr_plus: ""
        property string shortcut_pid_hr_minus: ""
        property string shortcut_ext_incline_plus: ""
        property string shortcut_ext_incline_minus: ""
        property string shortcut_biggears_plus: ""
        property string shortcut_biggears_minus: ""
        property string shortcut_avs_cruise: ""
        property string shortcut_avs_climb: ""
        property string shortcut_avs_sprint: ""
        property string shortcut_power_avg: ""
        property string shortcut_erg_mode: ""
        property string shortcut_preset_resistance_1: ""
        property string shortcut_preset_resistance_2: ""
        property string shortcut_preset_resistance_3: ""
        property string shortcut_preset_resistance_4: ""
        property string shortcut_preset_resistance_5: ""
        property string shortcut_preset_speed_1: ""
        property string shortcut_preset_speed_2: ""
        property string shortcut_preset_speed_3: ""
        property string shortcut_preset_speed_4: ""
        property string shortcut_preset_speed_5: ""
        property string shortcut_preset_inclination_1: ""
        property string shortcut_preset_inclination_2: ""
        property string shortcut_preset_inclination_3: ""
        property string shortcut_preset_inclination_4: ""
        property string shortcut_preset_inclination_5: ""
        property string shortcut_preset_powerzone_1: ""
        property string shortcut_preset_powerzone_2: ""
        property string shortcut_preset_powerzone_3: ""
        property string shortcut_preset_powerzone_4: ""
        property string shortcut_preset_powerzone_5: ""
        property string shortcut_preset_powerzone_6: ""
        property string shortcut_preset_powerzone_7: ""
        property string shortcut_auto_resistance: ""
        property string shortcut_lap: ""
        property string shortcut_start_stop: ""
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

    Timer {
       id: pelotonAuthCheck
       interval: 1000  // 1 second delay after startup
       running: true
       repeat: false
       onTriggered: {
           if (settings.peloton_password !== "password") {
               popupPelotonAuth.visible = true
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

    MessageDialog {
        text: "Garmin"
        informativeText: "Workout found:\n" + rootItem.garminWorkoutPromptName +
                         (rootItem.garminWorkoutPromptDate.length > 0 ? "\nDate: " + rootItem.garminWorkoutPromptDate : "") +
                         "\n\nDo you want to start it now?"
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: { rootItem.garmin_start_downloaded_workout(); }
        onNoClicked: { rootItem.garmin_dismiss_downloaded_workout_prompt(); }
        visible: rootItem.garminWorkoutPromptRequested
    }

    MessageDialog {
        id: stravaLogoutConfirm
        text: qsTr("Strava")
        informativeText: qsTr("You are already connected to Strava. Do you want to log out?")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: { rootItem.strava_logout(); }
        onNoClicked: this.visible = false
        visible: false
    }

    MessageDialog {
        id: pelotonLogoutConfirm
        text: qsTr("Peloton")
        informativeText: qsTr("You are already connected to Peloton. Do you want to log out?")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: { rootItem.peloton_logout(); }
        onNoClicked: this.visible = false
        visible: false
    }

    MessageDialog {
        id: intervalsICULogoutConfirm
        text: qsTr("Intervals.icu")
        informativeText: qsTr("You are already connected to Intervals.icu. Do you want to log out?")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: { rootItem.intervalsicu_logout(); }
        onNoClicked: this.visible = false
        visible: false
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
        Accessible.ignored: !drawer.opened

        ScrollView {
            contentWidth: -1
            focus: true
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.fill: parent

            Column {
                anchors.fill: parent
                spacing: 3

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
                        if(CHARTJS)
                            stackView.push("TrainingProgramsListJS.qml")
                        else
                            stackView.push("TrainingProgramsList.qml")
                        stackView.currentItem.trainprogram_open_clicked.connect(trainprogram_open_clicked)
                        stackView.currentItem.trainprogram_open_other_folder.connect(trainprogram_open_other_folder)
                        stackView.currentItem.trainprogram_preview.connect(trainprogram_preview)
                        stackView.currentItem.trainprogram_autostart_requested.connect(trainprogram_autostart_requested)
                        stackView.currentItem.trainprogram_open_clicked.connect(function(url) {
                            stackView.pop();
                         });
                        drawer.close()
                    }
                }
                ItemDelegate {
                    text: qsTr("Workout Editor")
                    width: parent.width
                    onClicked: {
                        var editorPage = stackView.push("WorkoutEditor.qml")
                        if (editorPage) {
                            editorPage.closeRequested.connect(function() {
                                stackView.pop()
                            })
                            // Close editor when workout is started from Save & Start
                            trainprogram_autostart_requested.connect(function() {
                                console.log("[main.qml] trainprogram_autostart_requested received, closing editor")
                                editorPage.closeRequested()
                            })
                        }
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
                    text: "version 2.20.27"
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
                        if (rootItem.isStravaLoggedIn()) {
                            stravaLogoutConfirm.visible = true
                            drawer.close()
                        } else {
                            stackView.push("WebStravaAuth.qml")
                            strava_connect_clicked()
                            drawer.close()
                        }
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
                        if (rootItem.isPelotonLoggedIn()) {
                            pelotonLogoutConfirm.visible = true
                            drawer.close()
                        } else {
                            stackView.push("WebPelotonAuth.qml")
                            stackView.currentItem.goBack.connect(function() {
                                stackView.pop();
                            })
                            peloton_connect_clicked()
                            drawer.close()
                        }
                    }
                }

                ItemDelegate {
                    Image {
                        anchors.left: parent.left;
                        anchors.verticalCenter: parent.verticalCenter
                        source: "icons/icons/garmin-connect-badge.png"
                        fillMode: Image.PreserveAspectFit
                        visible: true
                        width: parent.width
                        height: 48
                    }
                    width: parent.width
                    onClicked: {
                        toolButtonLoadSettings.visible = true;
                        toolButtonSaveSettings.visible = true;
                        stackView.push("settings.qml")
                        if (stackView.currentItem) {
                            if (stackView.currentItem.openGarminSection) {
                                stackView.currentItem.openGarminSection()
                            }
                            if (stackView.currentItem.peloton_connect_clicked) {
                                stackView.currentItem.peloton_connect_clicked.connect(function() {
                                    peloton_connect_clicked()
                                });
                            }
                        }
                        drawer.close()
                    }
                }

				ItemDelegate {
                    Image {
                        anchors.left: parent.left;
                        anchors.verticalCenter: parent.verticalCenter
                        source: "icons/icons/intervals-logo-with-name.png"
                        fillMode: Image.PreserveAspectFit
                        visible: true
                        width: parent.width
                    }
                    width: parent.width
                    onClicked: {
                        if (rootItem.isIntervalsICULoggedIn()) {
                            intervalsICULogoutConfirm.visible = true
                            drawer.close()
                        } else {
                            stackView.push("WebIntervalsICUAuth.qml")
                            intervalsicu_connect_clicked()
                            drawer.close()
                        }
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

    // Wrapper Item to prevent ApplicationWindow from capturing all VoiceOver focus
    Item {
        anchors.fill: parent
        Accessible.ignored: true

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
                if (event.key === Qt.Key_MediaPrevious) {
                    keyMediaPrevious();
                    event.accepted = true;
                } else if (event.key === Qt.Key_MediaNext) {
                    keyMediaNext();
                    event.accepted = true;
                } else {
                    event.accepted = false;
                }
            }

            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_speed_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("speed") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_speed_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("speed") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_inclination_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("inclination") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_inclination_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("inclination") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_resistance_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("resistance") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_resistance_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("resistance") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_peloton_resistance_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("peloton_resistance") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_peloton_resistance_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("peloton_resistance") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_target_resistance_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("target_resistance") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_target_resistance_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("target_resistance") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_target_power_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("target_power") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_target_power_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("target_power") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_target_zone_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("target_zone") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_target_zone_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("target_zone") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_target_speed_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("target_speed") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_target_speed_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("target_speed") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_target_incline_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("target_inclination") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_target_incline_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("target_inclination") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_fan_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("fan") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_fan_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("fan") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_peloton_offset_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("peloton_offset") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_peloton_offset_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("peloton_offset") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_peloton_remaining_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("peloton_remaining") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_peloton_remaining_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("peloton_remaining") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_remaining_time_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("remainingtimetrainprogramrow") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_remaining_time_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("remainingtimetrainprogramrow") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_gears_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("gears") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_gears_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("gears") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_pid_hr_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("pid_hr") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_pid_hr_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("pid_hr") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_ext_incline_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("external_inclination") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_ext_incline_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("external_inclination") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_biggears_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("biggearsplus") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_biggears_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("biggearsminus") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_avs_cruise; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("autoVirtualShiftingCruise") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_avs_climb; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("autoVirtualShiftingClimb") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_avs_sprint; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("autoVirtualShiftingSprint") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_power_avg; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("powerAvg") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_erg_mode; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("erg_mode") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_resistance_1; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_resistance_1") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_resistance_2; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_resistance_2") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_resistance_3; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_resistance_3") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_resistance_4; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_resistance_4") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_resistance_5; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_resistance_5") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_speed_1; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_speed_1") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_speed_2; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_speed_2") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_speed_3; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_speed_3") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_speed_4; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_speed_4") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_speed_5; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_speed_5") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_inclination_1; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_inclination_1") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_inclination_2; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_inclination_2") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_inclination_3; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_inclination_3") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_inclination_4; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_inclination_4") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_inclination_5; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_inclination_5") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_powerzone_1; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_powerzone_1") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_powerzone_2; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_powerzone_2") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_powerzone_3; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_powerzone_3") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_powerzone_4; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_powerzone_4") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_powerzone_5; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_powerzone_5") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_powerzone_6; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_powerzone_6") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_powerzone_7; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_powerzone_7") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_auto_resistance; enabled: shortcutReady(sequence); onActivated: rootItem.setAutoResistance(!rootItem.autoResistance) }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_lap; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLap() }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_start_stop; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardStartStop() }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
