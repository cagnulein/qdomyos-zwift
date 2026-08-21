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
    id: customGearSettingsWindow
    visible: true
    clip: true

    Settings {
        id: settings
        property bool gears_custom_table_enabled: false
        property string gears_custom_table: "1|1\n2|2\n3|3\n4|4\n5|5\n6|6\n7|7\n8|8\n9|9\n10|10\n11|11\n12|12\n13|13\n14|14\n15|15\n16|16\n17|17\n18|18\n19|19\n20|20\n21|21\n22|22\n23|23\n24|24"
        property bool power_calibration_table_enabled: false
        property string power_calibration_table: "0|0\n1000|1000"
    }

    ListModel {
        id: gearListModel
    }

    ListModel {
        id: powerCalibrationListModel
    }

    property var gearRows: []
    property var powerCalibrationRows: []
    property int rowHeight: 55
    property int offsetControlHeight: 43

    Component.onCompleted: {
        loadGearRows()
        loadPowerCalibrationRows()
    }

    function clampOffset(value) {
        if (isNaN(value)) {
            return 0
        }
        return Math.max(-100, Math.min(100, value))
    }

    function parseOffset(value) {
        return parseFloat(String(value).replace(",", "."))
    }

    function defaultGearRows() {
        var rows = []
        for (var i = 1; i <= 24; i++) {
            rows.push({ gear: i, offset: i })
        }
        return rows
    }

    function loadGearRows() {
        var parsed = stringToGearRows(settings.gears_custom_table)
        gearRows = parsed.length === 24 ? parsed : defaultGearRows()
        updateGearListModel()
        saveGearRows()
    }

    function stringToGearRows(gearString) {
        if (!gearString) {
            return []
        }

        var rows = []
        var lines = gearString.split("\n")
        for (var i = 0; i < lines.length; i++) {
            var parts = lines[i].split("|")
            if (parts.length < 2) {
                continue
            }
            var gear = parseInt(parts[0])
            var offset = clampOffset(parseOffset(parts[1]))
            if (gear >= 1 && gear <= 24) {
                rows[gear - 1] = { gear: gear, offset: offset }
            }
        }

        var compactRows = []
        for (var j = 1; j <= 24; j++) {
            compactRows.push(rows[j - 1] ? rows[j - 1] : { gear: j, offset: j })
        }
        return compactRows
    }

    function gearRowsToString(rows) {
        return rows.map(function(row) {
            return row.gear + "|" + clampOffset(parseOffset(row.offset))
        }).join("\n")
    }

    function saveGearRows() {
        settings.gears_custom_table = gearRowsToString(gearRows)
    }

    function updateGearListModel() {
        if (!gearListModel) {
            return
        }
        gearListModel.clear()
        for (var i = 0; i < gearRows.length; i++) {
            gearListModel.append(gearRows[i])
        }
    }

    function defaultPowerCalibrationRows() {
        return [
            { measured: 0, reference: 0 },
            { measured: 1000, reference: 1000 }
        ]
    }

    function parsePower(value) {
        return parseFloat(String(value).replace(",", "."))
    }

    function sanitizePower(value) {
        var parsed = parsePower(value)
        if (isNaN(parsed)) {
            return 0
        }
        return Math.max(0, parsed)
    }

    function stringToPowerCalibrationRows(tableString) {
        if (!tableString) {
            return []
        }

        var rows = []
        var lines = tableString.split("\n")
        for (var i = 0; i < lines.length; i++) {
            var parts = lines[i].split("|")
            if (parts.length < 2) {
                continue
            }
            var measured = parsePower(parts[0])
            var reference = parsePower(parts[1])
            if (!isNaN(measured) && !isNaN(reference) && measured >= 0 && reference >= 0) {
                rows.push({ measured: measured, reference: reference })
            }
        }
        return rows
    }

    function powerCalibrationRowsToString(rows) {
        return rows.map(function(row) {
            return sanitizePower(row.measured) + "|" + sanitizePower(row.reference)
        }).join("\n")
    }

    function loadPowerCalibrationRows() {
        var parsed = stringToPowerCalibrationRows(settings.power_calibration_table)
        powerCalibrationRows = parsed.length >= 2 ? parsed : defaultPowerCalibrationRows()
        updatePowerCalibrationListModel()
        savePowerCalibrationRows()
    }

    function savePowerCalibrationRows() {
        settings.power_calibration_table = powerCalibrationRowsToString(powerCalibrationRows)
    }

    function updatePowerCalibrationListModel() {
        if (!powerCalibrationListModel) {
            return
        }
        powerCalibrationListModel.clear()
        for (var i = 0; i < powerCalibrationRows.length; i++) {
            powerCalibrationListModel.append(powerCalibrationRows[i])
        }
    }

    function addPowerCalibrationRow() {
        var measured = 1000
        var reference = 1000
        if (powerCalibrationRows.length > 0) {
            var last = powerCalibrationRows[powerCalibrationRows.length - 1]
            measured = sanitizePower(last.measured) + 100
            reference = sanitizePower(last.reference) + 100
        }
        powerCalibrationRows.push({ measured: measured, reference: reference })
        updatePowerCalibrationListModel()
        savePowerCalibrationRows()
    }

    function removePowerCalibrationRow(index) {
        if (powerCalibrationRows.length <= 2) {
            return
        }
        powerCalibrationRows.splice(index, 1)
        updatePowerCalibrationListModel()
        savePowerCalibrationRows()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        IndicatorOnlySwitch {
            text: qsTr("Enable Custom Gear Table")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.gears_custom_table_enabled
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.gears_custom_table_enabled = checked
        }

        Label {
            text: qsTr("Each gear uses the offset below instead of the raw gear value. QZ applies it automatically to resistance, inclination, or slope depending on the trainer path. Default is linear.")
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            font.pixelSize: Qt.application.font.pixelSize - 2
            color: Material.accent
        }

        Button {
            text: qsTr("Reset to Linear Defaults")
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            onClicked: {
                gearRows = defaultGearRows()
                updateGearListModel()
                saveGearRows()
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
                    width: parent.width / 2
                    height: parent.height
                    border.width: 1
                    border.color: "#cccccc"
                    color: "transparent"

                    Text {
                        anchors.centerIn: parent
                        text: qsTr("Gear")
                        font.bold: true
                        color: "black"
                    }
                }

                Rectangle {
                    width: parent.width / 2
                    height: parent.height
                    border.width: 1
                    border.color: "#cccccc"
                    color: "transparent"

                    Text {
                        anchors.centerIn: parent
                        text: qsTr("Offset")
                        font.bold: true
                        color: "black"
                    }
                }
            }
        }

        ListView {
            id: gearTable
            Layout.fillWidth: true
            Layout.preferredHeight: 24 * rowHeight
            clip: true
            model: gearListModel

            Component.onCompleted: {
                updateGearListModel()
            }

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            delegate: Rectangle {
                width: gearTable.width
                height: rowHeight
                color: index % 2 === 0 ? "white" : "#fafafa"

                Row {
                    anchors.fill: parent

                    Rectangle {
                        width: parent.width / 2
                        height: parent.height
                        border.width: 1
                        border.color: "#cccccc"
                        color: "transparent"

                        Text {
                            anchors.centerIn: parent
                            text: gear
                            color: "black"
                        }
                    }

                    Rectangle {
                        width: parent.width / 2
                        height: parent.height
                        border.width: 1
                        border.color: "#cccccc"
                        color: "transparent"

                        RowLayout {
                            anchors.centerIn: parent
                            width: parent.width * 0.92
                            height: offsetControlHeight
                            spacing: 4

                            Button {
                                text: "-"
                                Layout.preferredWidth: 34
                                Layout.fillHeight: true
                                onClicked: {
                                    gearRows[index].offset = clampOffset(parseOffset(gearRows[index].offset) - 0.5)
                                    offsetTextField.text = gearRows[index].offset
                                    saveGearRows()
                                }
                            }

                            TextField {
                                id: offsetTextField
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                text: offset
                                color: "black"
                                selectedTextColor: "white"
                                selectionColor: Material.accent
                                horizontalAlignment: Text.AlignHCenter
                                background: Rectangle {
                                    color: "white"
                                    border.color: "#cccccc"
                                    radius: 2
                                }
                                function applyOffset() {
                                    var newOffset = clampOffset(parseOffset(text))
                                    gearRows[index].offset = newOffset
                                    text = newOffset
                                    saveGearRows()
                                }
                                onAccepted: applyOffset()
                                onEditingFinished: applyOffset()
                            }

                            Button {
                                text: "+"
                                Layout.preferredWidth: 34
                                Layout.fillHeight: true
                                onClicked: {
                                    gearRows[index].offset = clampOffset(parseOffset(gearRows[index].offset) + 0.5)
                                    offsetTextField.text = gearRows[index].offset
                                    saveGearRows()
                                }
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 2
            color: Material.accent
        }

        Label {
            text: qsTr("Power Calibration Table")
            Layout.fillWidth: true
            font.bold: true
            font.pixelSize: Qt.application.font.pixelSize + 2
        }

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
            text: qsTr("Enter the power reported by the bike/trainer and the matching power from a trusted reference power meter. QZ linearly interpolates between points and extends the first/last segment outside the table range. When disabled, Watt Gain and Watt Offset work exactly as before.")
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            font.pixelSize: Qt.application.font.pixelSize - 2
            color: Material.accent
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Button {
                text: qsTr("Add Point")
                Layout.fillWidth: true
                onClicked: addPowerCalibrationRow()
            }

            Button {
                text: qsTr("Reset to Linear Defaults")
                Layout.fillWidth: true
                onClicked: {
                    powerCalibrationRows = defaultPowerCalibrationRows()
                    updatePowerCalibrationListModel()
                    savePowerCalibrationRows()
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
                    width: parent.width * 0.44
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
                    width: parent.width * 0.44
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
                    width: parent.width * 0.12
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
            Layout.preferredHeight: Math.max(2, powerCalibrationRows.length) * rowHeight
            clip: true
            model: powerCalibrationListModel
            interactive: false

            delegate: Rectangle {
                width: powerCalibrationTable.width
                height: rowHeight
                color: index % 2 === 0 ? "white" : "#fafafa"

                Row {
                    anchors.fill: parent

                    Rectangle {
                        width: parent.width * 0.44
                        height: parent.height
                        border.width: 1
                        border.color: "#cccccc"
                        color: "transparent"

                        TextField {
                            id: measuredTextField
                            anchors.centerIn: parent
                            width: parent.width * 0.9
                            height: offsetControlHeight
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
                                var value = sanitizePower(text)
                                powerCalibrationRows[index].measured = value
                                text = value
                                savePowerCalibrationRows()
                            }
                            onAccepted: applyValue()
                            onEditingFinished: applyValue()
                        }
                    }

                    Rectangle {
                        width: parent.width * 0.44
                        height: parent.height
                        border.width: 1
                        border.color: "#cccccc"
                        color: "transparent"

                        TextField {
                            id: referenceTextField
                            anchors.centerIn: parent
                            width: parent.width * 0.9
                            height: offsetControlHeight
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
                                var value = sanitizePower(text)
                                powerCalibrationRows[index].reference = value
                                text = value
                                savePowerCalibrationRows()
                            }
                            onAccepted: applyValue()
                            onEditingFinished: applyValue()
                        }
                    }

                    Rectangle {
                        width: parent.width * 0.12
                        height: parent.height
                        border.width: 1
                        border.color: "#cccccc"
                        color: "transparent"

                        Button {
                            anchors.centerIn: parent
                            width: Math.min(parent.width - 4, 38)
                            height: offsetControlHeight
                            text: "×"
                            enabled: powerCalibrationRows.length > 2
                            onClicked: removePowerCalibrationRow(index)
                        }
                    }
                }
            }
        }
    }
}
