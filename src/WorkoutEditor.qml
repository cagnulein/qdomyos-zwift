import QtQuick 2.12
import QtQuick.Controls 2.5
import Qt.labs.settings 1.0
import QtWebView 1.1

Item {
    id: root
    property string title: qsTr("Workout Editor")
    property bool pageLoaded: false
    // When VoiceOver is active on iOS we present the editor as a NATIVE full-screen
    // modal WKWebView (see ios_accessibility_helper) instead of the embedded Qt
    // WebView, because Qt's accessibility bridge cannot expose the embedded web
    // content to VoiceOver as a navigable tree. In native mode the embedded WebView
    // below is never created/shown.
    property bool nativeMode: false

    signal closeRequested()

    Settings {
        id: settings
    }

    // Bridge: the native modal's "Done" button → homeform.nativeWorkoutEditorClosed
    // → pop this page.
    Connections {
        target: rootItem
        function onNativeWorkoutEditorClosed() {
            if (root.nativeMode) {
                console.log("[WorkoutEditor] native editor closed → closeRequested")
                root.closeRequested()
            }
        }
    }

    // Called by the page host (or autostart) to close. In native mode it also
    // dismisses the UIKit modal before popping.
    function closeEditor() {
        if (root.nativeMode) {
            rootItem.dismissNativeWorkoutEditor()
        }
        root.closeRequested()
    }

    Timer {
        id: portPoller
        interval: 500
        repeat: true
        running: false
        onTriggered: {
            var port = settings.value("template_inner_QZWS_port", 0)
            if (!port) {
                console.log("[WorkoutEditor VO] waiting for template_inner_QZWS_port")
                return
            }
            if (root.nativeMode) {
                // Port is ready: hand off to the native full-screen modal editor.
                console.log("[WorkoutEditor] port ready → presenting native modal editor")
                portPoller.stop()
                rootItem.presentNativeWorkoutEditor()
                return
            }
            var targetUrl = "http://localhost:" + port + "/workouteditor/index.html"
            if (webView.url !== targetUrl) {
                console.log("[WorkoutEditor VO] loading " + targetUrl)
                webView.url = targetUrl
            }
        }
    }

    // voAccessibilityTimer: fires once after load to configure VoiceOver for the
    // embedded WKWebView. Using Accessible.ignored = true on the WebView below tells
    // Qt NOT to create a virtual UIAccessibilityElement covering this area, so iOS
    // falls through to WKWebView's native HTML accessibility tree instead.
    Timer {
        id: voAccessibilityTimer
        interval: 300
        repeat: false
        onTriggered: {
            if (OS_VERSION === "iOS") {
                console.log("[WorkoutEditor VO] voAccessibilityTimer fired – calling prepareEmbeddedWebViewForVoiceOver")
                rootItem.prepareEmbeddedWebViewForVoiceOver()
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
                // Guard: only set up VoiceOver on the FIRST successful load.
                // UIAccessibilityScreenChangedNotification (posted by our helper)
                // can cause Qt to recreate the WKWebView internally, which fires a
                // second LoadSucceededStatus. Without this guard, that spurious event
                // would retrigger the accessibility setup → infinite loop.
                var firstLoad = !root.pageLoaded
                root.pageLoaded = true
                busy.visible = false
                busy.running = false
                portPoller.stop()
                webView.forceActiveFocus()
                console.log("[WorkoutEditor VO] load succeeded firstLoad=" + firstLoad + " activeFocus=" + webView.activeFocus + " visible=" + webView.visible + " size=" + webView.width + "x" + webView.height)
                if (firstLoad && OS_VERSION === "iOS") {
                    voAccessibilityTimer.restart()
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

    Component.onCompleted: {
        if (OS_VERSION === "iOS" && rootItem.isVoiceOverRunning()) {
            root.nativeMode = true
            console.log("[WorkoutEditor] VoiceOver active on iOS → using native modal editor")
        } else {
            console.log("[WorkoutEditor] using embedded Qt WebView editor")
        }
        // In both modes we poll for the QZWS port; the poller then either loads the
        // embedded WebView or presents the native modal.
        portPoller.start()
    }
}
