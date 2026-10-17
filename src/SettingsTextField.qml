import QtQuick 2.14
import QtQuick.Controls 2.15

// Text field of a free-text setting (a name, an e-mail, a password, a colour, a serial port).
// `savedText`: the row binds the saved setting here instead of `text`; the field shows it and
// shows it again whenever the setting changes (after OK too). `dirty` - the text is not the
// saved one, the OK button of the row is enabled by it: a grey OK means there is nothing to
// save. A field without `savedText` is never dirty
TextField {
    id: field

    property var savedText: undefined
    readonly property bool dirty: savedText !== undefined && text !== String(savedText)

    onSavedTextChanged: if (savedText !== undefined) text = savedText
    Component.onCompleted: if (savedText !== undefined) text = savedText
}
