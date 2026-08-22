import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0

ScrollView {
    id: customInclinationResistanceWindow
    contentWidth: -1
    focus: true
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.fill: parent
    clip: true

    property string defaultInclinationResistanceTable: "0|4\n1|6\n2|8\n3|10\n4|11\n5|11.5\n6|12\n8|13\n10|14\n12|15\n15|16"
    property int rowHeight: 55
    property int controlHeight: 43

    Settings {
        id: settings
        property bool custom_inclination_resistance_table_enabled: false
        property string custom_inclination_resistance_table: defaultInclinationResistanceTable
    }

    ListModel {
        id: pointListModel
    }

    Component.onCompleted: loadPoints()

    function parseNumber(value) {
        return parseFloat(String(value).replace(",", "."))
    }

    function formatNumber(value) {
        var number = parseNumber(value)
        if (isNaN(number)) {
            return "0"
        }
        return Number(number.toFixed(2)).toString()
    }

    function normalizeResistance(value) {
        var number = parseNumber(value)
        if (isNaN(number)) {
            return 0
        }
        return Math.max(0, number)
    }

    function defaultPoints() {
        return [
            { inclination: 0, resistance: 4 },
            { inclination: 1, resistance: 6 },
            { inclination: 2, resistance: 8 },
            { inclination: 3, resistance: 10 },
            { inclination: 4, resistance: 11 },
            { inclination: 5, resistance: 11.5 },
            { inclination: 6, resistance: 12 },
            { inclination: 8, resistance: 13 },
            { inclination: 10, resistance: 14 },
            { inclination: 12, resistance: 15 },
            { inclination: 15, resistance: 16 }
        ]
    }

    function stringToPoints(tableString) {
        if (!tableString) {
            return []
        }

        var points = []
        var lines = tableString.replace(/;/g, "\n").split("\n")
        for (var i = 0; i < lines.length; i++) {
            var parts = lines[i].split("|")
            if (parts.length < 2) {
                continue
            }

            var inclination = parseNumber(parts[0])
            var resistance = parseNumber(parts[1])
            if (isNaN(inclination) || isNaN(resistance)) {
                continue
            }

            points.push({
                inclination: inclination,
                resistance: normalizeResistance(resistance)
            })
        }

        points.sort(function(a, b) {
            return a.inclination - b.inclination
        })
        return points
    }

    function setPoints(points) {
        pointListModel.clear()
        for (var i = 0; i < points.length; i++) {
            pointListModel.append({
                inclination: points[i].inclination,
                resistance: points[i].resistance
            })
        }
    }

    function loadPoints() {
        var points = stringToPoints(settings.custom_inclination_resistance_table)
        if (points.length === 0) {
            points = defaultPoints()
        }
        setPoints(points)
        savePoints()
    }

    function pointsToString() {
        var rows = []
        for (var i = 0; i < pointListModel.count; i++) {
            var point = pointListModel.get(i)
            rows.push(formatNumber(point.inclination) + "|" + formatNumber(point.resistance))
        }
        return rows.join("\n")
    }

    function savePoints() {
        settings.custom_inclination_resistance_table = pointsToString()
    }

    function sortPointsAndSave() {
        var points = []
        for (var i = 0; i < pointListModel.count; i++) {
            var point = pointListModel.get(i)
            points.push({
                inclination: parseNumber(point.inclination),
                resistance: normalizeResistance(point.resistance)
            })
        }

        points.sort(function(a, b) {
            return a.inclination - b.inclination
        })
        setPoints(points)
        savePoints()
    }

    function updatePoint(index, role, value) {
        var currentPoint = pointListModel.get(index)
        var parsed = parseNumber(value)
        if (isNaN(parsed)) {
            parsed = role === "inclination" ? currentPoint.inclination : currentPoint.resistance
        }
        if (role === "resistance") {
            parsed = normalizeResistance(parsed)
        }
        pointListModel.setProperty(index, role, parsed)
        savePoints()
        return formatNumber(parsed)
    }

    function adjustPoint(index, role, delta) {
        var point = pointListModel.get(index)
        var current = role === "inclination" ? point.inclination : point.resistance
        var updated = parseNumber(current) + delta
        if (role === "resistance") {
            updated = normalizeResistance(updated)
        }
        pointListModel.setProperty(index, role, updated)
        savePoints()
    }

    function addPoint() {
        var inclination = 0
        var resistance = 1
        if (pointListModel.count > 0) {
            var lastPoint = pointListModel.get(pointListModel.count - 1)
            inclination = parseNumber(lastPoint.inclination) + 1
            resistance = normalizeResistance(lastPoint.resistance)
        }

        pointListModel.append({
            inclination: inclination,
            resistance: resistance
        })
        savePoints()
    }

    function removePoint(index) {
        if (pointListModel.count <= 1) {
            return
        }
        pointListModel.remove(index)
        savePoints()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        IndicatorOnlySwitch {
            text: qsTr("Enable Custom Inclination to Resistance Table")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.custom_inclination_resistance_table_enabled
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.custom_inclination_resistance_table_enabled = checked
        }

        Label {
            text: qsTr("Set the resistance QZ should target at each incline. QZ interpolates automatically between points and uses the nearest endpoint outside the configured range. Changes are saved automatically.")
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            font.pixelSize: Qt.application.font.pixelSize - 2
            color: Material.accent
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
                    width: parent.width * 0.43
                    height: parent.height
                    border.width: 1
                    border.color: "#cccccc"
                    color: "transparent"

                    Text {
                        anchors.centerIn: parent
                        text: qsTr("Inclination (%)")
                        font.bold: true
                        color: "black"
                    }
                }

                Rectangle {
                    width: parent.width * 0.43
                    height: parent.height
                    border.width: 1
                    border.color: "#cccccc"
                    color: "transparent"

                    Text {
                        anchors.centerIn: parent
                        text: qsTr("Resistance")
                        font.bold: true
                        color: "black"
                    }
                }

                Rectangle {
                    width: parent.width * 0.14
                    height: parent.height
                    border.width: 1
                    border.color: "#cccccc"
                    color: "transparent"
                }
            }
        }

        ListView {
            id: pointTable
            Layout.fillWidth: true
            Layout.preferredHeight: pointListModel.count * rowHeight
            clip: true
            model: pointListModel
            interactive: false

            delegate: Rectangle {
                width: pointTable.width
                height: rowHeight
                color: index % 2 === 0 ? "white" : "#fafafa"

                Row {
                    anchors.fill: parent

                    Rectangle {
                        width: parent.width * 0.43
                        height: parent.height
                        border.width: 1
                        border.color: "#cccccc"
                        color: "transparent"

                        RowLayout {
                            anchors.centerIn: parent
                            width: parent.width * 0.94
                            height: controlHeight
                            spacing: 4

                            Button {
                                text: "-"
                                Layout.preferredWidth: 34
                                Layout.fillHeight: true
                                onClicked: adjustPoint(index, "inclination", -0.5)
                            }

                            TextField {
                                id: inclinationField
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                text: formatNumber(inclination)
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
                                    text = updatePoint(index, "inclination", text)
                                    Qt.callLater(sortPointsAndSave)
                                }
                                onAccepted: applyValue()
                                onEditingFinished: applyValue()
                            }

                            Button {
                                text: "+"
                                Layout.preferredWidth: 34
                                Layout.fillHeight: true
                                onClicked: adjustPoint(index, "inclination", 0.5)
                            }
                        }
                    }

                    Rectangle {
                        width: parent.width * 0.43
                        height: parent.height
                        border.width: 1
                        border.color: "#cccccc"
                        color: "transparent"

                        RowLayout {
                            anchors.centerIn: parent
                            width: parent.width * 0.94
                            height: controlHeight
                            spacing: 4

                            Button {
                                text: "-"
                                Layout.preferredWidth: 34
                                Layout.fillHeight: true
                                onClicked: adjustPoint(index, "resistance", -0.5)
                            }

                            TextField {
                                id: resistanceField
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                text: formatNumber(resistance)
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
                                    text = updatePoint(index, "resistance", text)
                                }
                                onAccepted: applyValue()
                                onEditingFinished: applyValue()
                            }

                            Button {
                                text: "+"
                                Layout.preferredWidth: 34
                                Layout.fillHeight: true
                                onClicked: adjustPoint(index, "resistance", 0.5)
                            }
                        }
                    }

                    Rectangle {
                        width: parent.width * 0.14
                        height: parent.height
                        border.width: 1
                        border.color: "#cccccc"
                        color: "transparent"

                        Button {
                            anchors.centerIn: parent
                            width: Math.min(parent.width * 0.78, 44)
                            height: controlHeight
                            text: "×"
                            enabled: pointListModel.count > 1
                            onClicked: removePoint(index)
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Button {
                text: qsTr("Add Point")
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                onClicked: addPoint()
            }

            Button {
                text: qsTr("Reset Example")
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                onClicked: {
                    setPoints(defaultPoints())
                    savePoints()
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
