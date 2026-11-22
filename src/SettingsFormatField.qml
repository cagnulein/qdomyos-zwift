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
// `savedText`: the row binds the saved setting here instead of `text`; the field shows it and
// shows it again whenever the setting changes (after OK too). `dirty` - the value is not the
// saved one (another spelling of it is, see normalized()), the OK button of the row is enabled
// by `valid && dirty`: a grey OK means there is nothing to save. A field without `savedText` is
// never dirty
// Built on UiTextField: the modern look draws it as its other fields (filled, a red ring
// instead of the red line, a time picked on wheels); the classic look is the Material
// TextField as before
UiTextField {
    id: field

    invalid: !valid
    // A time field by its format, not by its text: UiTextField guesses a time once, from the text the
    // field is created with, and an address like "1:23" would pass the guess
    timeShaped: format === "time"
    // The guess of UiTextField sets true over the binding above: undone for the other formats
    onTimeShapedChanged: if (timeShaped && format !== "time") timeShaped = false

    property string format: "time"
    // an address: under the label in a settings row of the modern look (UiTextField.wideText); a
    // time, a height is short and stays beside it
    wideText: format === "ip" || format === "host"
    property var savedText: undefined

    readonly property bool valid: acceptableInput
    readonly property bool dirty: savedText !== undefined
        && (valid ? normalized(text) !== normalized(String(savedText)) : text !== String(savedText))
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

    // Room for the reason under the line (classic) or under the field (modern) while the
    // value is not valid
    readonly property real reasonRoom: valid ? 0 : reason.implicitHeight
    bottomPadding: (modern ? 12 : 16) + reasonRoom
    bottomInset: modernInset + reasonRoom

    onSavedTextChanged: if (savedText !== undefined) text = savedText
    Component.onCompleted: if (savedText !== undefined) text = savedText

    // One spelling of a value for `dirty`: "0:8:0" is the saved "00:08:00", 180.0 cm is 180,
    // 5'10 is 5'10"
    function normalized(t) {
        if (format === "time") {
            var parts = t.split(":")
            for (var i = 0; i < parts.length; i++)
                parts[i] = Number(parts[i])
            return parts.join(":")
        }
        if (format === "heightCm")
            return String(Number(t))
        if (format === "heightFtIn") {
            var out = ""
            for (var k = 0; k < t.length; k++) {
                var c = t[k]
                if (c === "‘" || c === "’")
                    out += "'"
                else if (c !== "\"" && c !== "“" && c !== "”")
                    out += c
            }
            return out
        }
        return t
    }

    Rectangle {
        // Over the underline of the Material TextField (same place, see its background);
        // the modern look has the red ring of UiTextField instead
        visible: !field.valid && !field.modern
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
        font.pixelSize: field.modern ? window.fontHint
            : window.textScale === 1 ? Qt.application.font.pixelSize - 3 : window.fontPx(window.appFontPx - 3)
        color: field.modern ? window.ui.danger : Material.color(Material.Red)
    }
}
