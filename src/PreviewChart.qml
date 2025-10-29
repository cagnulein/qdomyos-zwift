import QtQuick 2.7
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
    WebView {
        id: webView
        anchors.fill: parent
        url: "http://localhost:" + settings.value("template_inner_QZWS_port") + "/previewchart/chart.htm"
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
        onTriggered: {if(rootItem.startRequested) {rootItem.startRequested = false; rootItem.stopRequested = false; if (typeof window !== 'undefined' && window.goBack) { window.goBack(); } }}
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
