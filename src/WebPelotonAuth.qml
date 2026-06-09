import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Dialogs
import Qt.labs.settings
import QtMultimedia
import QtQuick.Layouts
import QtWebView

Item {
    id: pelotonAuthPage
    anchors.fill: parent
    height: parent.height
    width: parent.width
    visible: true
    property bool callbackHandled: false

    // Signal to notify the parent stack when we want to go back
    signal goBack()

    WebView {
        id: pelotonWebView
        anchors.fill: parent
        height: parent.height
        width: parent.width
        visible: !rootItem.pelotonPopupVisible
        url: rootItem.getPelotonAuthUrl

        onLoadingChanged: {
            if (callbackHandled || !loadRequest.url) {
                return;
            }

            var currentUrl = loadRequest.url.toString()
            if (currentUrl.indexOf("https://www.qzfitness.com/peloton/callback") === 0) {
                callbackHandled = true
                rootItem.handleOAuthCallbackFromQml(currentUrl)
            }
        }
    }

    Popup {
        id: popupPelotonConnectedWeb
        parent: Overlay.overlay
        enabled: rootItem.pelotonPopupVisible
        onEnabledChanged: { if(rootItem.pelotonPopupVisible) popupPelotonConnectedWeb.open() }
        onClosed: {
            rootItem.pelotonPopupVisible = false;
            callbackHandled = false;
            pelotonWebView.url = "";
            // Attempt to go back to the previous view after the popup is closed
            goBack();
        }
        x: Math.round((parent.width - width) / 2)
        y: Math.round((parent.height - height) / 2)
        width: 380
        height: 120
        modal: true
        focus: true
        palette.text: "white"
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        enter: Transition {
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0 }
        }

        exit: Transition {
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0 }
        }

        Column {
            anchors.horizontalCenter: parent.horizontalCenter
            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                width: 370
                height: 120
                text: qsTr("Your Peloton account is now connected!")
            }
        }

        // Add a MouseArea to capture clicks anywhere on the popup
        MouseArea {
            anchors.fill: parent
            onClicked: {
                popupPelotonConnectedWeb.close();
            }
        }
    }

    // Component is being completed
    Component.onCompleted: {
        console.log("WebPelotonAuth loaded")
        callbackHandled = false
    }
}
