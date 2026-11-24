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

    // Modern look: the page takes the app theme colours (qz-theme-bridge.js). They go in the URL
    // fragment for the first load and through runJavaScript when the theme changes later, so an
    // open workout is not reloaded. Null in the classic look: the page keeps its own dark palette;
    // null sent to an open page (the modern look switched off) takes the theme off
    readonly property var pageTheme: window.ui.webTheme

    onPageThemeChanged: {
        if (pageLoaded)
            webView.runJavaScript(window.ui.webThemeScript())
    }

    // Back (main.qml, navigateBack): in the modern look a list of the page opens as a menu of the
    // page (qz-modern-select.js), and Back closes it first, as it closes the system list. Only the
    // page knows whether one is open, and it answers through the callback: Back is taken here and
    // the page is left in the answer when no menu was open
    function handleBack() {
        if (!window.ui.modern || !root.pageLoaded)
            return false
        webView.runJavaScript("window.qzMenuBack ? window.qzMenuBack() : false", function(closed) {
            if (!closed && root.StackView.view && root.StackView.view.currentItem === root)
                window.navigateBack(false)
        })
        return true
    }

    // A workout started (Save & Start here, Start Workout in the program list) closes the editor.
    // The page listens itself, so the handler goes away with it:
    // one connected in main.qml per opening outlived the page and called it after it was destroyed.
    // The kept editor of the modern look is closed by main.qml (onTrainprogram_autostart_requested)
    Connections {
        target: window
        enabled: root !== window.keptWorkoutEditor
        function onTrainprogram_autostart_requested() {
            console.log("[WorkoutEditor.qml] trainprogram_autostart_requested received, closing editor")
            // closeEditor() also dismisses the native modal (VoiceOver path) before the pop
            root.closeEditor()
        }
    }

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

    // The page this item asked the WebView to load, kept as a string: webView.url reads back in
    // script as an object, never === a string, so compared with it every tick set the URL again,
    // and on Android each set starts the load over - a page slower than one tick never loaded.
    // Cleared when a load fails, to try again
    property string requestedPage: ""

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
            var page = "http://localhost:" + port + "/workouteditor/index.html"
            // Already loading this page: a theme change meanwhile is applied when it has loaded
            if (root.requestedPage === page) {
                return
            }
            console.log("[WorkoutEditor VO] loading " + page)
            root.requestedPage = page
            webView.url = page + window.ui.webThemeFragment()
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
                // A theme change during the load only moved the fragment: apply the current one
                if (root.pageTheme)
                    webView.runJavaScript(window.ui.webThemeScript())
                busy.visible = false
                busy.running = false
                portPoller.stop()
                webView.forceActiveFocus()
                console.log("[WorkoutEditor VO] load succeeded firstLoad=" + firstLoad + " activeFocus=" + webView.activeFocus + " visible=" + webView.visible + " size=" + webView.width + "x" + webView.height)
                if (firstLoad && OS_VERSION === "iOS") {
                    voAccessibilityTimer.restart()
                }
                // loaded after Back (portPoller): hidden already, so off the server at once
                if (root.StackView.status !== StackView.Active)
                    webView.runJavaScript("window.main_ws_pause && main_ws_pause(true)")
            } else if (loadRequest.status === WebView.LoadFailedStatus) {
                console.log("[WorkoutEditor VO] load failed, restarting poller")
                root.pageLoaded = false
                root.requestedPage = ""
                busy.visible = true
                busy.running = true
                portPoller.start()
            }
        }
    }

    // Shown again without a reload (the modern look keeps the page, main.qml; any look when a page
    // above it is closed): the page asks again for the connected machine, FTP, the units and the
    // saved workouts. Not at the first opening: the page is still loading and asks itself
    StackView.onActivated: {
        if (root.pageLoaded)
            webView.runJavaScript("window.main_ws_pause && main_ws_pause(false);"
                                  + " window.qzEditorReopened && window.qzEditorReopened()")
    }
    // Hidden (kept by the modern look, or under another page): off the server until shown again,
    // or it would take every update of the workout for nothing (main_ws_manager.js)
    StackView.onDeactivated: {
        if (root.pageLoaded)
            webView.runJavaScript("window.main_ws_pause && main_ws_pause(true)")
    }

    // a new app language: the page asks for its texts again (a reload would lose the edits)
    Connections {
        target: rootItem
        function onAppLanguageChanged() {
            webView.runJavaScript("window.qzRequestTranslations && window.qzRequestTranslations()")
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
