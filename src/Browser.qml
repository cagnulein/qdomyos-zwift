import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0
import QtWebView 1.1

Item {
    id: column1
    
    signal browser_peloton_time_to_complete(string time_to_complete)
    signal browser_peloton_time_to_start(string time_to_start)

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

        url: "https://members.onepeloton.com"
        visible: true
        onLoadingChanged: {
            if (loadRequest.errorString)
                console.error(loadRequest.errorString);
            if (loadRequest.status == WebView.LoadSucceededStatus) {
                console.error("Procedo");
                let loadScr = `
                    var findAndDisplayElement = function() {
                        var p = document.querySelector('p[data-test-id="time-to-complete"]');
                        if (p) {
                            browser_peloton_time_to_complete(p.innerText);
                            console.log(p.innerText);
                        } else {
                            console.log("Element not found");
                        }

                        p = document.querySelector('p[data-test-id="time-to-start"]');
                        if (p) {
                            browser_peloton_time_to_start(p.innerText);
                            console.log(p.innerText);
                        } else {
                            console.log("Element not found");
                        }
                    };
                    
                    findAndDisplayElement(); // Run immediately on first load
                    setInterval(findAndDisplayElement, 1000); // Then run every 1000 milliseconds (1 second)
                `;
                console.error(loadScr);
                webView.runJavaScript(loadScr, function(res) {
                });
            }
        }
    }
}
