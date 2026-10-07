import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0
import QtWebView 1.1

ColumnLayout {
    signal popupclose()
    id: column1
    spacing: 10
    anchors.fill: parent
    Settings {
        id: settings
    }

    // WORKAROUND: the Android WebView of Qt does not recover from a turn of the screen while the
    // page is shown. Scrolled down afterwards, it moves without momentum and leaves the page
    // blank below a straight line (up is fine). Seen on a OnePlus 12; to see it: open the charts
    // upright, turn the phone, scroll down. Moving the native view off the screen and back once
    // the turn has settled brings it round, the page stays as it is.
    // Off at the turn of the screen: the page changes its size in steps after it, and a page
    // shown stretched to those steps flashed
    readonly property bool landscape: width > height
    readonly property bool screenLandscape: Screen.width > Screen.height
    // Moved off by a turn, back once the size settles
    property bool turnHidden: false
    onLandscapeChanged: turnStart()
    onScreenLandscapeChanged: turnStart()
    onWidthChanged: if (turnHidden) turnShow.restart()
    onHeightChanged: if (turnHidden) turnShow.restart()

    function turnStart() {
        if (!visible)
            return
        turnHidden = true
        turnShow.restart()
    }

    Timer {
        id: turnShow
        interval: 300
        onTriggered: column1.turnHidden = false
    }

    WebView {
        id: webView
        anchors.fill: parent
        // Off the screen to the left, the same size
        anchors.leftMargin: column1.turnHidden ? -(Screen.width + Screen.height) : 0
        anchors.rightMargin: column1.turnHidden ? Screen.width + Screen.height : 0
        url: "http://localhost:" + settings.value("template_inner_QZWS_port") + "/chartjs/chart.htm"
        visible: true
        onLoadingChanged: {
            if (loadRequest.errorString) {
                console.error(loadRequest.errorString);
                console.error("port " + settings.value("template_inner_QZWS_port"));
            }
        }
    }

    Timer {
        id: chartJscheckStartFromWeb
        interval: 200; running: true; repeat: true
        onTriggered: {if(rootItem.startRequested) {rootItem.startRequested = false; rootItem.stopRequested = false; stackView.pop(); }}
    }

    Timer {
        id: sendMailFallback
        interval: 10000; running: true; repeat: false
        onTriggered: rootItem.sendMail()
    }

    Button {
        id: closeButton
        height: 50
        width: parent.width
        text: "Close"
        Layout.alignment: Qt.AlignCenter | Qt.AlignVCenter
        onClicked: {
            popupclose();
        }
        anchors {
            bottom: parent.bottom
        }
    }
	 Component.onCompleted: {
	     headerToolbar.visible = true;
	 }
}
