import QtQuick 2.12
import QtQuick.Controls 2.5
import Qt.labs.settings 1.0
import QtWebView 1.1

Item {
    id: root
    property string title: qsTr("Workout Editor")
    property bool pageLoaded: false

    signal closeRequested()

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
                console.log("[WorkoutEditor VO] waiting for template_inner_QZWS_port")
                return
            }
            var targetUrl = "http://localhost:" + port + "/workouteditor/index.html"
            if (webView.url !== targetUrl) {
                console.log("[WorkoutEditor VO] loading " + targetUrl)
                webView.url = targetUrl
            }
        }
    }

    WebView {
        id: webView
        anchors.fill: parent
        visible: root.pageLoaded
        focus: root.pageLoaded
        onLoadingChanged: {
            console.log("[WorkoutEditor VO] loadingChanged status=" + loadRequest.status + " url=" + loadRequest.url + " error=" + loadRequest.errorString)
            if (loadRequest.status === WebView.LoadSucceededStatus) {
                root.pageLoaded = true
                busy.visible = false
                busy.running = false
                portPoller.stop()
                webView.forceActiveFocus()
                console.log("[WorkoutEditor VO] load succeeded, activeFocus=" + webView.activeFocus + " visible=" + webView.visible + " size=" + webView.width + "x" + webView.height)
                if (OS_VERSION === "iOS") {
                    Qt.callLater(function() {
                        console.log("[WorkoutEditor VO] preparing embedded iOS web view accessibility")
                        rootItem.prepareEmbeddedWebViewForVoiceOver()
                        rootItem.notifyAccessibilityScreenChanged()
                    })
                }
            } else if (loadRequest.status === WebView.LoadFailedStatus) {
                console.log("[WorkoutEditor VO] load failed, restarting poller")
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
        Accessible.role: Accessible.Indicator
        Accessible.name: qsTr("Loading workout editor")
        Accessible.description: qsTr("The workout editor is loading")
        Accessible.focusable: !root.pageLoaded
    }

    Component.onCompleted: portPoller.start()
}
