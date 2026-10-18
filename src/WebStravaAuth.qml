import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtQuick.Dialogs 1.0
import QtGraphicalEffects 1.12
import Qt.labs.settings 1.0
import QtMultimedia 5.15
import QtQuick.Layouts 1.3
import QtWebView 1.1

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
        // With a bigger text: as wide as the screen allows, as tall as the text; the fixed
        // size cut the text
        width: window.textScale !== 1 ? Math.min(380, parent.width - 32) : 380
        height: window.textScale !== 1 ? Math.max(120, connectedText.height + topPadding + bottomPadding) : 120
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
                id: connectedText
                wrapMode: window.labelWrap
                anchors.horizontalCenter: parent.horizontalCenter
                width: window.textScale !== 1 ? popupStravaConnectedWeb.availableWidth : 370
                height: window.textScale !== 1 ? undefined : 120
                text: qsTr("Your Strava account is now connected!<br><br>When you will press STOP on QZ a file<br>will be automatically uploaded to Strava!")
            }
        }
    }
}
