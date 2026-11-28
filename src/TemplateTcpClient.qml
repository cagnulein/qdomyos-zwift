import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0

ColumnLayout {
    id: rootElement
    property string templateId: ""
    property Settings settings

    RowLayout {
        spacing: 10
        id: hostRow
        Label {
            id: labelTcpClientIp
            text: qsTr("%1 Host:").arg(rootElement.templateId)
            Layout.fillWidth: true
            wrapMode: window.labelWrap
        }
        function doSaveHost(text) {
            let ipHostCheck = /^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$|^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)+([A-Za-z]|[A-Za-z][A-Za-z0-9\-]*[A-Za-z0-9])$/g;
            let matches = text.match(ipHostCheck);
            console.log("Saving host for "+rootElement.templateId + " "+ text + " converted "+matches);
            if (matches) {
                settings.setValue("template_"+rootElement.templateId+"_ip", text);
            }
        }

        UiTextField {
            id:textTcpClientIp
            text: settings.value("template_"+rootElement.templateId+"_ip","127.0.0.1")
            horizontalAlignment: Text.AlignRight
            // modern look: a value wider than the text area (127.0.0.1 at 170 %) takes the inner padding,
            // 14 down to 0 (the text area of the plain TextField), instead of losing its start; the field
            // keeps its width, so the label does not move; the 2 is the room of the cursor
            leftPadding: window.ui.modern ? Math.max(0, Math.min(14, (width - contentWidth - 2) / 2)) : padding
            rightPadding: leftPadding
            Layout.fillHeight: false
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            //inputMethodHints: Qt.ImhFormattedNumbersOnly
            onAccepted: hostRow.doSaveHost(text)
        }
        UiButton {
            id: buttonTcpClientIp
            text: "OK"
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            onClicked: hostRow.doSaveHost(textTcpClientIp.text)
        }
    }
    RowLayout {
        spacing: 10
        id: portRow
        Label {
            id: labelTcpClientPort
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
            id: textTcpClientPort
            // modern look: the narrow field of the other number fields
            numberField: true
            text: settings.value("template_"+rootElement.templateId+"_port",4321) + "";
            horizontalAlignment: Text.AlignRight
            // modern look: a 5-digit port at a large text size - the inner padding shrinks, as for the host above
            leftPadding: window.ui.modern ? Math.max(0, Math.min(14, (width - contentWidth - 2) / 2)) : padding
            rightPadding: leftPadding
            Layout.fillHeight: false
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            inputMethodHints: Qt.ImhDigitsOnly
            onAccepted: portRow.doSavePort(text)
        }
        UiButton {
            id: buttonlabelTcpClientPort
            text: "OK"
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            onClicked: portRow.doSavePort(textTcpClientPort.text)
        }
    }
}
