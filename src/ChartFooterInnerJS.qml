import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0
import QtWebView 1.1

ColumnLayout {
    anchors.fill: parent
    // Reload when the chart is shown again (the chart button, back to the home page), but not when
    // the drawer closes: the drawer hides only the WebView below, and the page is kept meanwhile
    onVisibleChanged: {
        if (visible)
            webView.reload();
    }
    Settings {
        id: settings
        property int chart_display_mode: 0
    }
    WebView {
        id: webView
        anchors.fill: parent
        url: "http://localhost:" + settings.value("template_inner_QZWS_port") + "/chartjs/" +
             (rootItem.chartTreadmillMode ? "treadmillchartlive.htm" : "chartlive.htm")
        // Hidden while the drawer is open or being dragged: the native WebView of Android and iOS
        // is drawn over everything in the QML scene, the drawer included (QTBUG-58365, QTBUG-98574)
        visible: rootItem.chartFooterVisible && drawer.position === 0
        onLoadingChanged: {
            if (loadRequest.errorString) {
                console.error(loadRequest.errorString);
                console.error("port " + settings.value("template_inner_QZWS_port"));
            } else if (loadRequest.status === WebView.LoadSucceededStatus) {
                // Send chart display mode to the web view
                sendDisplayModeToWebView();
            }
        }
        onVisibleChanged: {
            console.log("onVisibleChanged" + visible)
        }
    }
    
    // Watch for changes in chart display mode setting
    Connections {
        target: settings
        function onChart_display_modeChanged() {
            sendDisplayModeToWebView();
        }
    }
    
    function sendDisplayModeToWebView() {
        if (webView.loading === false) {
            webView.runJavaScript("
                if (window.setChartDisplayMode) {
                    window.setChartDisplayMode(" + settings.chart_display_mode + ");
                }
            ");
        }
    }
}
