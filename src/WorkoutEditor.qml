import QtQuick 2.12
import QtQuick.Controls 2.5
import Qt.labs.settings 1.0
import QtWebView 1.1

Item {
    id: root
    property string title: qsTr("Workout Editor")
    property bool pageLoaded: false

    Settings {
        id: settings
    }

    Timer {
        id: portPoller
        interval: 500
        repeat: true
        running: !root.pageLoaded
        onTriggered: {
            var port = settings.value("template_inner_QZWS_port", 0)
            if (!port) {
                return
            }
            var targetUrl = "http://localhost:" + port + "/workouteditor/index.html"
            if (webView.url !== targetUrl) {
                webView.url = targetUrl
            }
        }
    }

    WebView {
        id: webView
        anchors.fill: parent
        visible: root.pageLoaded
        onLoadingChanged: {
            if (loadRequest.status === WebView.LoadSucceededStatus) {
                root.pageLoaded = true
                busy.visible = false
                busy.running = false
                portPoller.stop()
            } else if (loadRequest.status === WebView.LoadFailedStatus) {
                root.pageLoaded = false
                busy.visible = true
                busy.running = true
                portPoller.start()
            }
        }
    }

    BusyIndicator {
        id: busy
        anchors.centerIn: parent
        visible: !root.pageLoaded
        running: !root.pageLoaded
    }

    Component.onCompleted: portPoller.start()
}
