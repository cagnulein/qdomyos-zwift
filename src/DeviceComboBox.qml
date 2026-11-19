import QtQuick 2.15

// List of the Bluetooth devices found (equipment, heart rate belt, sensors). Opening it starts a
// search, so a device just switched on no longer needs "Refresh Devices List" and a second opening.
// rootItem.bluetoothDevices is sorted by signal, so while the list is open the devices found are
// added at the end and the rows stay where they are: a row must not move away from under the finger.
// The list is made again, sorted, at the next opening.
ValueComboBox {
    id: control

    textRole: "name"
    valueRole: "name"
    model: ListModel {
        id: rows
    }

    // rootItem appears after the window is built
    readonly property var found: typeof rootItem !== "undefined" && rootItem ? rootItem.bluetoothDevices : []

    // "Name (80%)" -> "Name": the signal of a device changes between two lists. Only the signal is
    // cut: "Name (DirCon)" is another entry than the same name found over Bluetooth
    function deviceName(row) {
        var bracket = row.lastIndexOf(" (")
        return bracket < 0 || !row.endsWith("%)") ? row : row.substring(0, bracket)
    }

    // rows.clear() sets currentIndex to -1: the row of the value is set again, so the list shows it
    // chosen and Enter without a highlighted row takes it, as with the model of the plain ComboBox
    function fill() {
        rows.clear()
        var current = -1
        for (var i = 0; i < found.length; ++i) {
            rows.append({ "name": found[i] })
            if (current < 0 && value !== "" && deviceName(found[i]) === deviceName(value))
                current = i
        }
        currentIndex = current
        measureItems()
    }

    function addNewDevices() {
        var shown = {}
        for (var i = 0; i < rows.count; ++i)
            shown[deviceName(rows.get(i).name)] = true
        var added = false
        for (var j = 0; j < found.length; ++j) {
            if (shown[deviceName(found[j])] !== true) {
                rows.append({ "name": found[j] })
                added = true
            }
        }
        if (added)
            measureItems()
    }

    onFoundChanged: {
        if (popup.visible)
            addNewDevices()
    }

    Connections {
        target: control.popup
        function onAboutToShow() {
            control.fill()
            if (typeof rootItem !== "undefined" && rootItem)
                rootItem.searchForDeviceList()
        }
    }
}
