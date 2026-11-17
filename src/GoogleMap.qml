import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0
import QtWebView 1.1
import QtQuick.Window 2.2

ColumnLayout {
    signal popupclose()
    id: column1
    spacing: 10
    anchors.fill: parent

    // Modern look, as ChartJsTest.qml: the page takes the app theme (qz-theme-bridge.js) from the
    // URL fragment and through runJavaScript when it changes, and the native view stays off the
    // screen until the page has drawn (it is white before and lies over QML)
    readonly property var pageTheme: window.ui.webTheme
    property bool pageLoaded: false
    property bool pageShown: !window.ui.modern
    readonly property real offScreen: pageShown ? 0 : Screen.width + Screen.height

    // The theme the page has, as JSON: that of the fragment of the first load, then of each script sent
    property string sentTheme: ""
    function applyTheme() {
        sentTheme = JSON.stringify(pageTheme)
        webView.runJavaScript(window.ui.webThemeScript())
    }

    // null (the modern look switched off) takes the theme off the page
    onPageThemeChanged: {
        if (pageLoaded)
            applyTheme()
    }

    Settings {
        id: settings
        property string maps_type: "3D"
    }
    WebView {
        id: webView
        anchors.fill: parent
        anchors.leftMargin: -column1.offScreen
        anchors.rightMargin: column1.offScreen
        visible: true
        onLoadingChanged: {
            if (loadRequest.errorString)
                console.error(loadRequest.errorString);
            if (loadRequest.status === WebView.LoadSucceededStatus) {
                column1.pageLoaded = true
                // A theme change during the load reached the page neither in the fragment nor in a
                // script (the map stays in its Loader after Back: the look switched off meanwhile too)
                if (JSON.stringify(column1.pageTheme) !== column1.sentTheme)
                    column1.applyTheme()
            }
            // Shown when the load ends, failed too: the page has its background by then
            if (loadRequest.status !== WebView.LoadStartedStatus)
                column1.pageShown = true
        }
    }

    // On the item that holds the page, not on the ColumnLayout: a layout warns about anchors on
    // each child item it manages. Off with the page: a page pushed over this one hides it
    BusyIndicator {
        parent: column1.parent
        anchors.centerIn: parent
        running: !column1.pageShown && column1.visible
        visible: running
    }

    // a new app language: the page asks for its texts again
    Connections {
        target: rootItem
        function onAppLanguageChanged() {
            webView.runJavaScript("window.qzRequestTranslations && window.qzRequestTranslations()")
        }
    }

    // Modern look: no Close button (nothing listens to popupclose; the back arrow of the
    // toolbar closes the page), the map runs down to the edge
    Button {
        id: closeButton
        visible: !window.ui.modern
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
         // Set once, not bound: the fragment is for the first load, a later theme goes
         // through runJavaScript
         sentTheme = JSON.stringify(pageTheme)
         webView.url = "http://localhost:" + settings.value("template_inner_QZWS_port") + "/" +
                       (settings.value("maps_type") === "3D" ? "googlemaps" : "maps2d") + "/maps.htm" +
                       window.ui.webThemeFragment()
     }
}
