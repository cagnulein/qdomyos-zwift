import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0
import QtWebView 1.1

Item {
    id: column1
    // vedi trainprogram_open_clicked
    Settings {
        id: settings
    }

    WebView {
        id: webView
        property var rr;
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width

        url: "https://onepeloton.com"
        visible: true
        onLoadingChanged: {
            if (loadRequest.errorString)
                console.error(loadRequest.errorString);
            if (loadRequest.status == WebView.LoadSucceededStatus) {
                console.error("Procedo");
                let loadScr = `
                    var iframe = document.createElement("iframe");

                    iframe.src = "https://www.example.com";
                    iframe.width = "640";
                    iframe.height = "480";
                    iframe.frameBorder = "0";

                    document.body.appendChild(iframe);
                `;
                webView.runJavaScript(loadScr, function(res) {
                });
            }
        }
    }
}
