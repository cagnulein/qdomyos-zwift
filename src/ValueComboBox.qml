import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

// ComboBox for string settings whose stored value must stay untranslated
// (e.g. "Disabled", "Male", "lower" are compared as-is elsewhere in the code).
// `value` is the raw setting value the caller saves; `labels` maps raw values
// to translated text and is used for display only. Values missing from
// `labels` (such as Bluetooth device names) are shown unchanged.
ComboBox {
    id: control

    property string value
    property var labels: ({})

    function labelFor(v) {
        return labels.hasOwnProperty(v) ? labels[v] : v
    }

    displayText: labelFor(value)
    onActivated: value = currentValue

    // Same as the Material style's default delegate; only the text goes through labelFor()
    delegate: MenuItem {
        width: ListView.view.width
        text: control.labelFor(modelData)
        Material.foreground: control.currentIndex === index ? ListView.view.contentItem.Material.accent : ListView.view.contentItem.Material.foreground
        highlighted: control.highlightedIndex === index
        hoverEnabled: control.hoverEnabled
    }
}
