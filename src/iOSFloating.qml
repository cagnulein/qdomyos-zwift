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
                    var iframeContainer = document.createElement("iframe");

                    iframeContainer.src = "https://localhost:` + settings.value("template_inner_QZWS_port") + `/floating/floating.htm";
                    iframeContainer.width = "640";
                    iframeContainer.height = "480";
                    iframeContainer.frameBorder = "0";
                    iframeContainer.style.cssText = "position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%); z-index: 1;";

                    document.body.appendChild(iframeContainer);

                    var moveButton = document.createElement("button");
                    var buttonText = document.createTextNode("Move");
                    moveButton.appendChild(buttonText);
                    moveButton.style.cssText = "z-index: 1; position: absolute; bottom: 20px; right: 20px; background-color: white; color: black; border: none; padding: 10px; border-radius: 5px; cursor: pointer;";
                    document.body.appendChild(moveButton);

                    var isMoving = false;
                    var offsetX, offsetY;

                    moveButton.addEventListener('mousedown', function (event) {
                       isMoving = true;
                       offsetX = event.clientX - iframeContainer.offsetLeft;
                       offsetY = event.clientY - iframeContainer.offsetTop;
                        });

                    moveButton.addEventListener('mouseup', function () {
                        isMoving = false;
                    });

                    document.addEventListener('mousemove', function (event) {
                        if (isMoving) {
                          iframeContainer.style.left = event.clientX - offsetX + 'px';
                          iframeContainer.style.top = event.clientY - offsetY + 'px';
                        }
                    });
                `;
                console.error(loadScr);
                webView.runJavaScript(loadScr, function(res) {
                });
            }
        }
    }
}
