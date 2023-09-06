import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0
import QtWebView 1.1

ColumnLayout {
    anchors.fill: parent
    Settings {
        id: settings
    }
    WebView {
        id: webView
        anchors.fill: parent
        url: "http://localhost:" + settings.value("template_inner_QZWS_port") + "/chartjs/chartlive.htm"
        visible: rootItem.chartFooterVisible
        onLoadingChanged: {
            if (loadRequest.errorString) {
                console.error(loadRequest.errorString);
                console.error("port " + settings.value("template_inner_QZWS_port"));
            }
        }
        onVisibleChanged: {
            console.log("onVisibleChanged" + visible)
            if(visible === true) {
                reload();
            }
        }
    }
}
