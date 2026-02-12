import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtQuick.Dialogs 1.0
import QtGraphicalEffects 1.12
import Qt.labs.settings 1.0
import QtMultimedia 5.15
import QtQuick.Window 2.12
import Qt.labs.platform 1.1

ApplicationWindow {
    id: window
    width: 640
    height: 480
    visibility: Qt.WindowFullScreen
    visible: true
	 objectName: "stack"
    title: qsTr("qDomyos-Zwift")
	
	  
    signal start_clicked;
    signal stop_clicked;
    signal lap_clicked;
    signal peloton_start_workout;
    signal peloton_abort_workout;
    signal plus_clicked(string name)
    signal minus_clicked(string name)
    signal largeButton_clicked(string name)

    signal gpx_open_clicked(url name)
    signal gpxpreview_open_clicked(url name)
    signal profile_open_clicked(url name)
    signal trainprogram_open_clicked(url name)
    signal trainprogram_open_other_folder(url name)
    signal gpx_open_other_folder(url name)
    signal trainprogram_preview(url name)
    signal trainprogram_zwo_loaded(string s)
    signal gpx_save_clicked()
    signal fit_save_clicked()
    signal refresh_bluetooth_devices_clicked()
    signal strava_connect_clicked()
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
        property real ui_zoom: 100.0
        property bool theme_tile_icon_enabled: true
        property string theme_tile_background_color: "#303030"
        property string theme_background_color: "#303030"
        property bool theme_tile_shadow_enabled: true
        property string theme_tile_shadow_color: "#9C27B0"
        property int theme_tile_secondline_textsize: 12
		
    }
	
	background: Rectangle {
        anchors.fill: parent
        width: parent.fill
        height: parent.fill
        color: settings.theme_background_color
    }

    StackView {
        id: stackView
        initialItem: "Wizard.qml"
        anchors.fill: parent
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
}
