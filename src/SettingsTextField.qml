import QtQuick 2.14
import QtQuick.Controls 2.15

// Text field of a free-text setting (a name, an e-mail, a password, a colour, a serial port).
// `savedText`: the row binds the saved setting here instead of `text`; the field shows it and
// shows it again whenever the setting changes (after OK too). `dirty` - the text is not the
// saved one, the OK button of the row is enabled by it: a grey OK means there is nothing to
// save. A field without `savedText` is never dirty. A word still being typed on a predictive
// keyboard (Android) is in preeditText, not yet in text: it counts as a change, or the OK stayed
// grey and could not take the tap that commits the word
// Built on UiTextField: the modern look draws it as its other fields; the classic look is the
// Material TextField as before
UiTextField {
    id: field

    // a free text: under the label in a settings row of the modern look (UiTextField.wideText)
    wideText: true
    property var savedText: undefined
    readonly property bool dirty: savedText !== undefined
                                  && (text !== String(savedText) || preeditText !== "")

    // A free text is never a time: UiTextField guesses one from the text ("12:34"), and the modern look would
    // make a label or a password like that read-only, on wheels
    onTimeShapedChanged: if (timeShaped) timeShaped = false

    onSavedTextChanged: if (savedText !== undefined) text = savedText
    Component.onCompleted: if (savedText !== undefined) text = savedText
}
