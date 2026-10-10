import QtQuick 2.14
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0

// Text field of a number setting. The OK button of the row writes `value` and is enabled by
// `valid`: a bare text field let "72,5", "170.5.5" or "7a" through, the assignment to the
// double setting failed without a word, and a negative weight was saved as typed.
// - one number only: a minus only when `signed`, no point when `decimals` is 0, extra
//   decimals dropped while typing (a longer value already shown, e.g. pounds, stays valid)
// - a decimal comma (the keyboard of the phone's language) counts as a point: `value` is the
//   number, whatever the separator
// - `minimum` / `maximum` when set: out of range is not valid
// - not valid: a red line and the reason under the field, also for a value already saved
//   before this check existed; nothing is changed or reset by itself
// The rule is a regular expression, not IntValidator/DoubleValidator: those follow the
// locale of the phone (comma or point), the very thing that broke the decimal input
TextField {
    id: field

    property bool signed: false
    property int decimals: 2
    property var minimum: undefined
    property var maximum: undefined

    readonly property real value: Number(text.replace(",", "."))
    readonly property bool inRange: (minimum === undefined || value >= minimum)
                                    && (maximum === undefined || value <= maximum)
    readonly property bool valid: acceptableInput && inRange
    readonly property string errorText: valid ? ""
        : /^-/.test(text) && !signed ? qsTr("The value cannot be negative")
        : !acceptableInput ? (decimals === 0 ? qsTr("Enter a whole number") : qsTr("Enter a number"))
        : minimum !== undefined && maximum !== undefined ? qsTr("Allowed range: %1 to %2").arg(minimum).arg(maximum)
        : minimum !== undefined ? qsTr("Minimum: %1").arg(minimum)
        : qsTr("Maximum: %1").arg(maximum)

    validator: RegularExpressionValidator {
        regularExpression: field.decimals === 0
                           ? (field.signed ? /^-?\d+$/ : /^\d+$/)
                           : (field.signed ? /^-?(\d+([.,]\d*)?|[.,]\d+)$/ : /^(\d+([.,]\d*)?|[.,]\d+)$/)
    }

    // Number keyboard on Android: the comma it may offer counts as a point (see `value`).
    // iOS keeps the full keyboard (d09a2cf96: its decimal separator)
    inputMethodHints: Qt.platform.os === "android"
                      ? (signed || decimals > 0 ? Qt.ImhFormattedNumbersOnly : Qt.ImhDigitsOnly)
                      : Qt.ImhNone

    // Room for the reason under the line while the value is not valid
    bottomPadding: valid ? 16 : 16 + reason.implicitHeight

    onTextEdited: {
        if (decimals <= 0)
            return
        var point = text.search(/[.,]/)
        if (point >= 0 && text.length - point - 1 > decimals) {
            var pos = Math.min(cursorPosition, point + 1 + decimals)
            text = text.substring(0, point + 1 + decimals)
            cursorPosition = pos
        }
    }

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
