import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0

ColumnLayout {
    id: rootElement
    property string templateId: ""
    property Settings settings
    Label {
        Layout.preferredWidth: parent.width
        id: ipsLabel
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
            text: qsTr(rootElement.templateId + " Port:")
            Layout.fillWidth: true
        }
        function doSavePort(text) {
            let port = parseInt(text);
            console.log("Saving port for "+rootElement.templateId + " "+ text + " converted "+port);
            if (!isNaN(port) && port>0 && port < 65535)
                settings.setValue("template_"+rootElement.templateId+"_port", port);
        }
        TextField {
            id: textWebServerPort
            text: settings.value("template_"+rootElement.templateId+"_port",6666) + '';
            horizontalAlignment: Text.AlignRight
            Layout.fillHeight: false
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            inputMethodHints: Qt.ImhDigitsOnly
            onAccepted: portRow.doSavePort(text)
        }
        Button {
            id: buttonlabelWebServerPort
            text: "OK"
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            onClicked: function() {
                portRow.doSavePort(textWebServerPort.text);
                ipsLabel.updateAddresses();
            }
        }
    }
}
