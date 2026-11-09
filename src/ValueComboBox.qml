import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

// ComboBox for string settings whose stored value must stay untranslated
// (e.g. "Disabled", "Male", "lower" are compared as-is elsewhere in the code).
// `value` is the raw setting value the caller saves; `labels` maps raw values
// to translated text and is used for display only. Values missing from
// `labels` (such as Bluetooth device names) are shown unchanged.
UiComboBox {
    id: control

    property string value
    property var labels: ({})

    // UiComboBox's list items and labelFor() show these labels
    itemLabels: labels

    displayText: labelFor(value)
    onActivated: value = currentValue

    // savedText (see UiComboBox) is the raw saved value: shown through `value`, compared raw
    dirty: savedText !== undefined && value !== String(savedText)
    function showSaved() {
        if (savedText !== undefined)
            value = savedText
    }
}
