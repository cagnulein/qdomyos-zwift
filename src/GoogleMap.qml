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

    // The page of the maps type: the 3D one (googlemaps) or the 2D one (maps2d). Read from the
    // stored settings each time, not from the property of the copy below: that one is read once,
    // when the map is first opened, and the map then stays in its Loader for the whole run
    readonly property string mapsType: loadedPage === "googlemaps" ? "3D" : "2D"
    property string loadedPage: ""
    // A fresh install has no maps_type stored yet: the default of the copy, 3D as in the settings,
    // or the first map would be 2D and the next one 3D, once the copy has written its default
    function storedPage() {
        return settings.value("maps_type", settings.maps_type) === "3D" ? "googlemaps" : "maps2d"
    }
    function loadPage() {
        loadedPage = storedPage()
        // Set once, not bound: the fragment is for this load, a later theme goes through
        // runJavaScript. The new page is off the screen again until it has drawn
        sentTheme = JSON.stringify(pageTheme)
        pageLoaded = false
        pageShown = !window.ui.modern
        webView.url = "http://localhost:" + settings.value("template_inner_QZWS_port") + "/" +
                      loadedPage + "/maps.htm" + window.ui.webThemeFragment()
    }
    // Another maps type: stored at once (setValue writes through, the property of the copy would
    // be written only later), and the other page loaded. The property too: on a fresh install the
    // copy still has its default to write, it would put 3D back over the type just chosen
    function setMapsType(type) {
        settings.maps_type = type
        settings.setValue("maps_type", type)
        if (storedPage() !== loadedPage)
            loadPage()
    }
    // A maps type changed in the settings since the map was last shown: the other page
    StackView.onActivating: {
        if (storedPage() !== loadedPage)
            loadPage()
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
            // The end of the load of the other page, cut short by a new maps type: not this page
            // (Android reports any page finished as succeeded). An event without an address is kept
            var loadedUrl = loadRequest.url.toString()
            if (loadedUrl !== "" && loadedUrl.indexOf("/" + column1.loadedPage + "/") < 0)
                return
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
         loadPage()
     }
}
