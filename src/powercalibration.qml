import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0

ScrollView {
    contentWidth: -1
    focus: true
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.fill: parent
    id: powerCalibrationSettingsWindow
    visible: true
    clip: true

    Settings {
        id: settings
        property bool power_calibration_table_enabled: false
        property string power_calibration_table: "0|0\n100|100\n200|200\n300|300\n400|400\n500|500\n600|600"
    }

    ListModel {
        id: powerCalibrationListModel
    }

    property var calibrationRows: []
    property int rowHeight: 55
    property int controlHeight: 43

    Component.onCompleted: loadCalibrationRows()

    function parseNumber(value) {
        return parseFloat(String(value).replace(",", "."))
    }

    function defaultCalibrationRows() {
        return [
            { measured: 0, reference: 0 },
            { measured: 100, reference: 100 },
            { measured: 200, reference: 200 },
            { measured: 300, reference: 300 },
            { measured: 400, reference: 400 },
            { measured: 500, reference: 500 },
            { measured: 600, reference: 600 }
        ]
    }

    function stringToCalibrationRows(tableString) {
        if (!tableString)
            return []

        var rows = []
        var lines = tableString.split("\n")
        for (var i = 0; i < lines.length; i++) {
            var parts = lines[i].split("|")
            if (parts.length < 2)
                continue

            var measured = parseNumber(parts[0])
            var reference = parseNumber(parts[1])
            if (!isNaN(measured) && !isNaN(reference) && measured >= 0 && reference >= 0)
                rows.push({ measured: measured, reference: reference })
        }
        return rows
    }

    function calibrationRowsToString(rows) {
        return rows.map(function(row) {
            return parseNumber(row.measured) + "|" + parseNumber(row.reference)
        }).join("\n")
    }

    function loadCalibrationRows() {
        var parsed = stringToCalibrationRows(settings.power_calibration_table)
        calibrationRows = parsed.length >= 2 ? parsed : defaultCalibrationRows()
        updateCalibrationListModel()
    }

    function saveCalibrationRows() {
        settings.power_calibration_table = calibrationRowsToString(calibrationRows)
    }

    function updateCalibrationListModel() {
        powerCalibrationListModel.clear()
        for (var i = 0; i < calibrationRows.length; i++)
            powerCalibrationListModel.append(calibrationRows[i])
    }

    function updateValue(rowIndex, field, text) {
        var value = parseNumber(text)
        if (isNaN(value) || value < 0)
            value = 0
        calibrationRows[rowIndex][field] = value
        saveCalibrationRows()
        return value
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        IndicatorOnlySwitch {
            text: qsTr("Enable Power Calibration Table")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.power_calibration_table_enabled
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.power_calibration_table_enabled = checked
        }

        Label {
            text: qsTr("Map the power reported by the bike or trainer to power measured by a reference power meter. QZ linearly interpolates between the points. The table is disabled by default; when disabled, Watt Gain and Watt Offset work exactly as before.")
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            font.pixelSize: Qt.application.font.pixelSize - 2
            color: Material.accent
        }

        RowLayout {
            Layout.fillWidth: true

            Button {
                text: qsTr("Add Point")
                Layout.fillWidth: true
                onClicked: {
                    var measured = calibrationRows.length > 0 ? parseNumber(calibrationRows[calibrationRows.length - 1].measured) + 100 : 0
                    var reference = calibrationRows.length > 0 ? parseNumber(calibrationRows[calibrationRows.length - 1].reference) + 100 : 0
                    calibrationRows.push({ measured: measured, reference: reference })
                    updateCalibrationListModel()
                    saveCalibrationRows()
                }
            }

            Button {
                text: qsTr("Reset to Linear Defaults")
                Layout.fillWidth: true
                onClicked: {
                    calibrationRows = defaultCalibrationRows()
                    updateCalibrationListModel()
                    saveCalibrationRows()
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            color: "#f0f0f0"
            border.width: 1
            border.color: "#cccccc"

            Row {
                anchors.fill: parent

                Rectangle {
                    width: parent.width * 0.42
                    height: parent.height
                    border.width: 1
                    border.color: "#cccccc"
                    color: "transparent"
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("Bike / Trainer Watt")
                        font.bold: true
                        color: "black"
                    }
                }

                Rectangle {
                    width: parent.width * 0.42
                    height: parent.height
                    border.width: 1
                    border.color: "#cccccc"
                    color: "transparent"
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("Reference Watt")
                        font.bold: true
                        color: "black"
                    }
                }

                Rectangle {
                    width: parent.width * 0.16
                    height: parent.height
                    border.width: 1
                    border.color: "#cccccc"
                    color: "transparent"
                }
            }
        }

        ListView {
            id: powerCalibrationTable
            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(2, calibrationRows.length) * rowHeight
            clip: true
            model: powerCalibrationListModel

            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

            delegate: Rectangle {
                width: powerCalibrationTable.width
                height: rowHeight
                color: index % 2 === 0 ? "white" : "#fafafa"

                Row {
                    anchors.fill: parent

                    Rectangle {
                        width: parent.width * 0.42
                        height: parent.height
                        border.width: 1
                        border.color: "#cccccc"
                        color: "transparent"

                        TextField {
                            id: measuredTextField
                            anchors.centerIn: parent
                            width: parent.width * 0.9
                            height: controlHeight
                            text: measured
                            color: "black"
                            selectedTextColor: "white"
                            selectionColor: Material.accent
                            horizontalAlignment: Text.AlignHCenter
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            background: Rectangle {
                                color: "white"
                                border.color: "#cccccc"
                                radius: 2
                            }
                            function applyValue() {
                                text = updateValue(index, "measured", text)
                            }
                            onAccepted: applyValue()
                            onEditingFinished: applyValue()
                        }
                    }

                    Rectangle {
                        width: parent.width * 0.42
                        height: parent.height
                        border.width: 1
                        border.color: "#cccccc"
                        color: "transparent"

                        TextField {
                            id: referenceTextField
                            anchors.centerIn: parent
                            width: parent.width * 0.9
                            height: controlHeight
                            text: reference
                            color: "black"
                            selectedTextColor: "white"
                            selectionColor: Material.accent
                            horizontalAlignment: Text.AlignHCenter
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            background: Rectangle {
                                color: "white"
                                border.color: "#cccccc"
                                radius: 2
                            }
                            function applyValue() {
                                text = updateValue(index, "reference", text)
                            }
                            onAccepted: applyValue()
                            onEditingFinished: applyValue()
                        }
                    }

                    Rectangle {
                        width: parent.width * 0.16
                        height: parent.height
                        border.width: 1
                        border.color: "#cccccc"
                        color: "transparent"

                        Button {
                            anchors.centerIn: parent
                            width: Math.min(parent.width * 0.85, 45)
                            height: controlHeight
                            text: "×"
                            enabled: calibrationRows.length > 2
                            onClicked: {
                                if (calibrationRows.length <= 2)
                                    return
                                calibrationRows.splice(index, 1)
                                updateCalibrationListModel()
                                saveCalibrationRows()
                            }
                        }
                    }
                }
            }
        }
    }
}
