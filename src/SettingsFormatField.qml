import QtQuick 2.14
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0

// Text field of a setting kept as text in a fixed format; the OK button of the row is enabled
// by `valid`. A bare text field let anything through: a pace like "5 min" was saved as NaN,
// an IP address field took any text.
// - format "time": hh:mm:ss, as the pace fields show it and timeToPaceSeconds() reads it
// - format "ip": an IPv4 address, or empty (feature off) - for QHostAddress (OSC)
// - format "host": an IPv4 address or a host name, an optional :port, or empty - for addresses
//   that go to connectToHost, a ws:// URL or "adb connect" (192.168.1.10:5555, treadmill.local)
// - format "heightCm" / "heightFtIn": the player height as the field shows it, 2-3 digits of
//   centimetres (a height converted from feet keeps its long fraction) or feet'inches (5'10")
// Characters that cannot belong to the format cannot be typed; an incomplete value shows a red
// line and the expected format under the field, nothing is changed by itself
TextField {
    id: field

    property string format: "time"

    readonly property bool valid: acceptableInput
    readonly property string errorText: valid ? ""
        : format === "ip" ? qsTr("Enter an IP address, e.g. 192.168.1.10")
        : format === "host" ? qsTr("Enter an IP address or a host name, e.g. 192.168.1.10")
        : format === "heightCm" ? qsTr("Enter the height in centimetres")
        : format === "heightFtIn" ? qsTr("Enter the height as feet'inches, e.g. 5'10\"")
        : qsTr("Enter the time as hh:mm:ss")

    validator: RegularExpressionValidator {
        regularExpression: field.format === "ip"
            ? /^$|^((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)\.){3}(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)$/
            : field.format === "host"
            ? /^$|^[A-Za-z0-9]([A-Za-z0-9-]*[A-Za-z0-9])?(\.[A-Za-z0-9]([A-Za-z0-9-]*[A-Za-z0-9])?)*(:\d{1,5})?$/
            : field.format === "heightCm" ? /^\d{2,3}(\.\d*)?$/
            : field.format === "heightFtIn" ? /^[1-8]['‘’]\d{1,2}["“”]?$/
            : /^\d{1,2}:[0-5]?\d:[0-5]?\d$/
    }
    // Full keyboard on purpose: a number keyboard of Android may have no ':' and allow a single '.'

    // Room for the reason under the line while the value is not valid
    bottomPadding: valid ? 16 : 16 + reason.implicitHeight

    Rectangle {
        // Over the underline of the Material TextField (same place, see its background)
        visible: !field.valid
        x: field.leftPadding
        width: field.width - field.leftPadding - field.rightPadding
        height: 2
        y: field.height - field.bottomPadding + 6
        color: Material.color(Material.Red)
    }
    Label {
        id: reason
        visible: !field.valid
        text: field.errorText
        x: field.leftPadding
        width: field.width - field.leftPadding - field.rightPadding
        y: field.height - implicitHeight - 2
        wrapMode: Text.WordWrap
        horizontalAlignment: field.horizontalAlignment
        font.pixelSize: Qt.application.font.pixelSize - 3
        color: Material.color(Material.Red)
    }
}
