import QtQuick 2.7
import QtQml 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0

ScrollView {
    id: customInclinationResistanceWindow
    contentWidth: -1
    // The column with its top and bottom margins, which are not in its implicit height
    contentHeight: inclinationColumn.implicitHeight + 2 * inclinationColumn.anchors.topMargin
    focus: true
    // No anchors: the page is pushed on the StackView, which sizes it to fill and warned about
    // conflicting anchors
    clip: true

    property string defaultInclinationResistanceTable: "0|4\n1|6\n2|8\n3|10\n4|11\n5|11.5\n6|12\n8|13\n10|14\n12|15\n15|16"
    // Modern look: the +/- fields 52 high, 8 between them (4 above and 4 below each)
    property int rowHeight: window.ui.modern ? 60 : 55
    property int controlHeight: 43

    // Modern look: a +/- field of the table, as on the Wahoo Options page. The classic one is a
    // text field between two buttons; its "-" and "+" did not fit the modern button and showed
    // as empty circles
    component Stepper: Rectangle {
        id: stepper
        property alias text: stepperInput.text
        // The column title, for a screen reader: the field and its - and + say what they change
        property string label
        signal step(real delta)
        signal edited(string value)
        // A number typed but not confirmed is applied first: a MouseArea does not take the focus
        // from the field, so its editing does not finish before the step (the classic buttons do)
        function stepBy(delta) {
            stepper.edited(stepperInput.text)
            stepper.step(delta)
        }
        radius: 12
        color: window.ui.surfaceHighest

        MouseArea {
            id: stepperMinus
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            // Not square: at 52 high, a 360 dp screen left 28 dp for the number between them
            width: 44
            onClicked: stepper.stepBy(-0.5)
            Accessible.role: Accessible.Button
            Accessible.name: qsTr("Decrease %1").arg(stepper.label)
            Accessible.onPressAction: stepper.stepBy(-0.5)
            Rectangle { anchors.fill: parent; radius: 12; color: window.ui.surfaceHigh; visible: stepperMinus.pressed }
            Rectangle { anchors.centerIn: parent; width: 12; height: 2; radius: 1; color: window.ui.textMain }
        }
        MouseArea {
            id: stepperPlus
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 44
            onClicked: stepper.stepBy(0.5)
            Accessible.role: Accessible.Button
            Accessible.name: qsTr("Increase %1").arg(stepper.label)
            Accessible.onPressAction: stepper.stepBy(0.5)
            Rectangle { anchors.fill: parent; radius: 12; color: window.ui.surfaceHigh; visible: stepperPlus.pressed }
            Rectangle { anchors.centerIn: parent; width: 12; height: 2; radius: 1; color: window.ui.textMain }
            Rectangle { anchors.centerIn: parent; width: 2; height: 12; radius: 1; color: window.ui.textMain }
        }
        UiSpinInput {
            id: stepperInput
            anchors.left: stepperMinus.right
            anchors.right: stepperPlus.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            font.pixelSize: window.ui.fontBody
            color: window.ui.textMain
            selectionColor: window.ui.accent
            selectedTextColor: window.ui.accentInk
            inputMethodHints: Qt.ImhFormattedNumbersOnly
            Accessible.name: stepper.label + ": " + text
            onAccepted: stepper.edited(text)
            onEditingFinished: stepper.edited(text)
        }
    }

    // The Settings object of settings.qml, given by the page that opens this one: one object
    // for all the settings pages, so a page no longer writes back an old copy of what another
    // page changed
    property var settings

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

    // Fills the rows in place, not with clear() + append(): an emptied table made the page shorter for
    // a moment and scrolled it back to the top (Reset Example at the end of the page). The fields
    // follow the model - their handlers keep the text bindings - so a row given another point shows it
    function setPoints(points) {
        while (pointListModel.count > points.length) {
            pointListModel.remove(pointListModel.count - 1)
        }
        for (var i = 0; i < points.length; i++) {
            var row = {
                inclination: points[i].inclination,
                resistance: points[i].resistance
            }
            if (i < pointListModel.count) {
                pointListModel.set(i, row)
            } else {
                pointListModel.append(row)
            }
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

    // Sorts the rows with move(), not with setPoints(): the number of rows does not change, so the
    // page keeps its scroll, and a moved row keeps its delegate - the field being edited goes with its
    // point instead of showing another one, and is not destroyed (a destroyed field finished its
    // editing from a row that was gone, index -1)
    function sortPointsAndSave() {
        for (var i = 0; i < pointListModel.count - 1; i++) {
            var lowest = i
            for (var j = i + 1; j < pointListModel.count; j++) {
                if (parseNumber(pointListModel.get(j).inclination) < parseNumber(pointListModel.get(lowest).inclination)) {
                    lowest = j
                }
            }
            if (lowest !== i) {
                pointListModel.move(lowest, i, 1)
            }
        }
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
        id: inclinationColumn
        // Top, left and right only: filling the content item too tied the column height to the
        // content height made from it (binding loop on contentHeight)
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 10
        anchors.leftMargin: window.ui.modern ? Math.max(16, window.contentSideMargin) : window.contentSideMargin
        anchors.rightMargin: window.ui.modern ? Math.max(16, window.contentSideMargin) : window.contentSideMargin
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
            font.pixelSize: window.ui.modern ? window.ui.fontCaption : window.fontHint
            bottomPadding: window.hintGap
            color: window.ui.modern ? window.ui.textMuted : Material.accent
        }

        // Modern look: the column titles over the fields, no grid
        RowLayout {
            visible: window.ui.modern
            Layout.fillWidth: true
            Layout.topMargin: 4
            spacing: 8
            Label {
                text: qsTr("Inclination (%)")
                color: window.ui.textMuted
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                wrapMode: Text.WordWrap
            }
            Label {
                text: qsTr("Resistance")
                color: window.ui.textMuted
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                wrapMode: Text.WordWrap
            }
            Item { Layout.preferredWidth: 48 }
        }

        Rectangle {
            visible: !window.ui.modern
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
                        // a plain Text does not inherit the window font: the text size while it is not 100 %
                        Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
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
                        Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
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
                color: window.ui.modern ? "transparent" : (index % 2 === 0 ? "white" : "#fafafa")

                RowLayout {
                    visible: window.ui.modern
                    anchors.fill: parent
                    anchors.topMargin: 4
                    anchors.bottomMargin: 4
                    spacing: 8

                    Stepper {
                        Layout.fillWidth: true
                        Layout.preferredWidth: 1
                        Layout.fillHeight: true
                        label: qsTr("Inclination (%)")
                        text: formatNumber(inclination)
                        // After each change the text is bound to the model again, not set to a
                        // number: a set text drops the binding and keeps an old number when
                        // setPoints() gives the row another point; the new binding also replaces
                        // what was typed (5,0 -> 5)
                        onStep: {
                            adjustPoint(index, "inclination", delta)
                            text = Qt.binding(function() { return formatNumber(inclination) })
                            Qt.callLater(sortPointsAndSave)
                        }
                        // index -1: the row is going away - its remove button is a MouseArea that
                        // does not take the focus, so the field finishes its editing afterwards
                        onEdited: {
                            if (index < 0)
                                return
                            updatePoint(index, "inclination", value)
                            text = Qt.binding(function() { return formatNumber(inclination) })
                            Qt.callLater(sortPointsAndSave)
                        }
                    }

                    Stepper {
                        Layout.fillWidth: true
                        Layout.preferredWidth: 1
                        Layout.fillHeight: true
                        label: qsTr("Resistance")
                        text: formatNumber(resistance)
                        onStep: {
                            adjustPoint(index, "resistance", delta)
                            text = Qt.binding(function() { return formatNumber(resistance) })
                        }
                        onEdited: {
                            if (index < 0)
                                return
                            updatePoint(index, "resistance", value)
                            text = Qt.binding(function() { return formatNumber(resistance) })
                        }
                    }

                    MouseArea {
                        id: removeButton
                        Layout.preferredWidth: 48
                        Layout.fillHeight: true
                        enabled: pointListModel.count > 1
                        onClicked: removePoint(index)
                        Accessible.role: Accessible.Button
                        // Several rows: the name says which point goes
                        Accessible.name: qsTr("Remove the point at inclination %1").arg(formatNumber(inclination))
                        Accessible.onPressAction: {
                            if (enabled)
                                removePoint(index)
                        }
                        Rectangle {
                            anchors.fill: parent
                            radius: 12
                            color: removeButton.pressed ? window.ui.surfaceHigh : "transparent"
                        }
                        UiIcon {
                            anchors.centerIn: parent
                            width: 20
                            height: 20
                            name: "close"
                            color: window.ui.textMuted
                            opacity: removeButton.enabled ? 1 : 0.35
                        }
                    }
                }

                Row {
                    visible: !window.ui.modern
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

                            UiButton {
                                text: "-"
                                Layout.preferredWidth: 34
                                Layout.fillHeight: true
                                // The text bound to the model again, as in the modern Stepper
                                onClicked: {
                                    adjustPoint(index, "inclination", -0.5)
                                    inclinationField.text = Qt.binding(function() { return formatNumber(inclination) })
                                    Qt.callLater(sortPointsAndSave)
                                }
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
                                    updatePoint(index, "inclination", text)
                                    text = Qt.binding(function() { return formatNumber(inclination) })
                                    Qt.callLater(sortPointsAndSave)
                                }
                                onAccepted: applyValue()
                                onEditingFinished: applyValue()
                            }

                            UiButton {
                                text: "+"
                                Layout.preferredWidth: 34
                                Layout.fillHeight: true
                                onClicked: {
                                    adjustPoint(index, "inclination", 0.5)
                                    inclinationField.text = Qt.binding(function() { return formatNumber(inclination) })
                                    Qt.callLater(sortPointsAndSave)
                                }
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

                            UiButton {
                                text: "-"
                                Layout.preferredWidth: 34
                                Layout.fillHeight: true
                                onClicked: {
                                    adjustPoint(index, "resistance", -0.5)
                                    resistanceField.text = Qt.binding(function() { return formatNumber(resistance) })
                                }
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
                                    updatePoint(index, "resistance", text)
                                    text = Qt.binding(function() { return formatNumber(resistance) })
                                }
                                onAccepted: applyValue()
                                onEditingFinished: applyValue()
                            }

                            UiButton {
                                text: "+"
                                Layout.preferredWidth: 34
                                Layout.fillHeight: true
                                onClicked: {
                                    adjustPoint(index, "resistance", 0.5)
                                    resistanceField.text = Qt.binding(function() { return formatNumber(resistance) })
                                }
                            }
                        }
                    }

                    Rectangle {
                        width: parent.width * 0.14
                        height: parent.height
                        border.width: 1
                        border.color: "#cccccc"
                        color: "transparent"

                        UiButton {
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

            UiButton {
                wrapsText: true
                text: qsTr("Add Point")
                Layout.fillWidth: true
                Layout.preferredHeight: window.ui.modern ? Math.max(48, contentItem.implicitHeight + 24) : 40
                onClicked: addPoint()
            }

            UiButton {
                wrapsText: true
                text: qsTr("Reset Example")
                Layout.fillWidth: true
                Layout.preferredHeight: window.ui.modern ? Math.max(48, contentItem.implicitHeight + 24) : 40
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
