import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 2.15

ApplicationWindow {
    id: window
    visible: true
    width: 1024
    height: 768
    title: "QZ Settings Visual Harness"
    property bool settings_restart_to_apply: false
    property alias visualCaptureRoot: captureRoot

    QtObject {
        id: visualRootItem
        property var bluetoothDevices: []
        property var metrics: ["speed", "cadence", "heart rate", "power"]
        property var tile_order: []
        property int pelotonLogin: -1
        property int pzpLogin: -1
        property int zwiftLogin: -1
        property bool garminMfaRequested: false
        function hasConnectedDevice() { return false }
        function setNativeShortcutCaptureSuspended(value) {}
        function garmin_connect_logout() {}
        function garmin_connect_login() {}
        function garmin_submit_mfa_code(value) {}
        function clearFiles() {}
        function getProfileDir() { return "" }
    }

    property alias rootItem: visualRootItem
    property var appModel: []

    Item {
        id: captureRoot
        anchors.fill: parent
        StackView {
            id: stackView
            anchors.fill: parent
            initialItem: "settings.qml"
        }
    }
}
