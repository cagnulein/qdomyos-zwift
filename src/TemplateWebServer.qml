import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0

ColumnLayout {
    id: rootElement
    property string templateId: ""
    property Settings settings
    // The classic look: the addresses above the port. The modern look shows them under it
    // (modernIpsLabel), as the description of the port in its frame
    Label {
        Layout.preferredWidth: parent.width
        id: ipsLabel
        visible: !window.ui.modern
        text: qsTr("Server addresses:")
        textFormat: Text.PlainText
        wrapMode: Text.WordWrap
        verticalAlignment: Text.AlignVCenter
        color: Material.color(Material.Red)
        function updateAddresses() {
            let ips = settings.value("template_"+rootElement.templateId+"_ips", []);
            let port = parseInt(settings.value("template_"+rootElement.templateId+"_port", 6666));
            if (isNaN(port) || port<=0 || port > 65535) {
                settings.setValue("template_"+rootElement.templateId+"_port",port = 6666);
            }
            ipsLabel.text = qsTr("Server addresses:");
            ips.forEach(function (ip) {
               ipsLabel.text += " http://" +ip+":" + port;
            });
        }
        Component.onCompleted: updateAddresses()
    }
    RowLayout {
        spacing: 10
        id: portRow
        Label {
            id: labelWebServerPort
            text: qsTr("%1 Port:").arg(rootElement.templateId)
            Layout.fillWidth: true
            wrapMode: window.labelWrap
        }
        function doSavePort(text) {
            let port = parseInt(text);
            console.log("Saving port for "+rootElement.templateId + " "+ text + " converted "+port);
            if (!isNaN(port) && port>0 && port < 65535)
                settings.setValue("template_"+rootElement.templateId+"_port", port);
        }
        UiTextField {
            id: textWebServerPort
            // modern look: the narrow field of the other number fields
            numberField: true
            text: settings.value("template_"+rootElement.templateId+"_port",6666) + '';
            horizontalAlignment: Text.AlignRight
            // modern look: a 5-digit port at a large text size takes the inner padding, 14 down to 0,
            // instead of losing its start; the field keeps its width, so the label does not move;
            // the 2 is the room of the cursor
            leftPadding: window.ui.modern ? Math.max(0, Math.min(14, (width - contentWidth - 2) / 2)) : padding
            rightPadding: leftPadding
            Layout.fillHeight: false
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            inputMethodHints: Qt.ImhDigitsOnly
            onAccepted: portRow.doSavePort(text)
        }
        UiButton {
            id: buttonlabelWebServerPort
            text: "OK"
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            onClicked: function() {
                portRow.doSavePort(textWebServerPort.text);
                ipsLabel.updateAddresses();
            }
        }
    }
    Label {
        id: modernIpsLabel
        visible: window.ui.modern
        Layout.fillWidth: true
        text: ipsLabel.text
        textFormat: Text.PlainText
        wrapMode: Text.WordWrap
        font.pixelSize: window.fontHint
        color: window.ui.textMuted
    }
}
