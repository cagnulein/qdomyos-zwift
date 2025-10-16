import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Effects
import Qt.labs.settings
import QtMultimedia
import QtQuick.Layouts
import QtWebView

Item {
    anchors.fill: parent
    height: parent.height
    width: parent.width
    visible: true

    WebView {
        anchors.fill: parent
        height: parent.height
        width: parent.width
        visible: !rootItem.generalPopupVisible
        url: rootItem.getStravaAuthUrl
    }

    Popup {
        id: popupStravaConnectedWeb
        parent: Overlay.overlay
        enabled: rootItem.generalPopupVisible
        onEnabledChanged: { if(rootItem.generalPopupVisible) popupStravaConnectedWeb.open() }
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
                text: qsTr("Your Strava account is now connected!<br><br>When you will press STOP on QZ a file<br>will be automatically uploaded to Strava!")
            }
        }
    }
}
