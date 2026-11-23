import QtQuick 2.7
import QtQml 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0

ScrollView {
    // Modern look: the rows Body Large, the role of a settings row (window.ui.rowFont)
    font: window.ui.modern ? window.ui.rowFont : window.font
    // Buttons and lists of this page take Label Large (window.ui.onTypeScalePage)
    readonly property bool typeScalePage: true
    contentWidth: -1
    // The column with its top and bottom margins, which are not in its implicit height
    contentHeight: customGearsColumn.implicitHeight + 2 * customGearsColumn.anchors.topMargin
    focus: true
    // No anchors: the page is pushed on the StackView, which sizes it to fill and warned about
    // conflicting anchors
    id: customGearSettingsWindow
    visible: true
    clip: true

    // The Settings object of settings.qml, given by the page that opens this one: one object
    // for all the settings pages, so a page no longer writes back an old copy of what another
    // page changed
    property var settings

    ListModel {
        id: gearListModel
    }

    property var gearRows: []
    // the rows, the fields and their -/+ grow with the text like the Wahoo gear table
    // (from 40 and 30), never below their size at 100 %
    property int rowHeight: window.textScale !== 1 ? Math.max(55, window.fontPx(40)) : 55
    property int offsetControlHeight: window.textScale !== 1 ? Math.max(43, window.fontPx(30)) : 43

    Component.onCompleted: {
        loadGearRows()
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

    ColumnLayout {
        id: customGearsColumn
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
            font.pixelSize: window.fontHint // modern look: Body Small, as the hints of the settings
            bottomPadding: window.hintGap
            color: window.ui.modern ? window.ui.textMuted : Material.accent
        }

        UiButton {
            wrapsText: true
            text: qsTr("Reset to Linear Defaults")
            Layout.fillWidth: true
            Layout.preferredHeight: window.ui.modern ? Math.max(48, contentItem.implicitHeight + 24) : window.fontPx(40)
            onClicked: {
                gearRows = defaultGearRows()
                updateGearListModel()
                saveGearRows()
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            color: window.ui.modern ? window.ui.surfaceHigh : "#f0f0f0"
            border.width: window.ui.modern ? 0 : 1
            border.color: window.ui.modern ? window.ui.alpha(window.ui.outline, 0.4) : "#cccccc"
            radius: window.ui.modern ? 12 : 0

            // Two equal columns; the offset field is narrow inside its own (a field over two thirds of
            // the row put the - and + far apart)
            Row {
                anchors.fill: parent

                Rectangle {
                    width: parent.width / 2
                    height: parent.height
                    border.width: window.ui.modern ? 0 : 1
                    border.color: window.ui.modern ? window.ui.alpha(window.ui.outline, 0.4) : "#cccccc"
                    color: "transparent"

                    Text {
                        // a plain Text does not inherit the window font: the text size while it is not 100 %; modern look: the table header and the gear number Label Large 14
                        Binding on font.pixelSize { when: window.ui.modern || window.scaledFontPx > 0; value: window.ui.modern ? window.ui.fontLabelLarge : window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                        anchors.centerIn: parent
                        // Modern look: one line inside its column, smaller if it does not fit
                        width: window.ui.modern ? parent.width - 8 : implicitWidth
                        horizontalAlignment: window.ui.modern ? Text.AlignHCenter : Text.AlignLeft
                        fontSizeMode: window.ui.modern ? Text.HorizontalFit : Text.FixedSize
                        // A shrunk text keeps the height of the full size: it fills the cell and stays in the middle
                        height: window.ui.modern ? parent.height : implicitHeight
                        verticalAlignment: window.ui.modern ? Text.AlignVCenter : Text.AlignTop
                        minimumPixelSize: 9
                        text: qsTr("Gear")
                        font.bold: true
                        color: window.ui.ink("black")
                    }
                }

                Rectangle {
                    width: parent.width / 2
                    height: parent.height
                    border.width: window.ui.modern ? 0 : 1
                    border.color: window.ui.modern ? window.ui.alpha(window.ui.outline, 0.4) : "#cccccc"
                    color: "transparent"

                    Text {
                        Binding on font.pixelSize { when: window.ui.modern || window.scaledFontPx > 0; value: window.ui.modern ? window.ui.fontLabelLarge : window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                        anchors.centerIn: parent
                        width: window.ui.modern ? parent.width - 8 : implicitWidth
                        horizontalAlignment: window.ui.modern ? Text.AlignHCenter : Text.AlignLeft
                        fontSizeMode: window.ui.modern ? Text.HorizontalFit : Text.FixedSize
                        // A shrunk text keeps the height of the full size: it fills the cell and stays in the middle
                        height: window.ui.modern ? parent.height : implicitHeight
                        verticalAlignment: window.ui.modern ? Text.AlignVCenter : Text.AlignTop
                        minimumPixelSize: 9
                        text: qsTr("Offset")
                        font.bold: true
                        color: window.ui.ink("black")
                    }
                }
            }
        }

        ListView {
            id: gearTable
            Layout.fillWidth: true
            // Modern look: all the rows in the page height, the page scrolls them - a list scrolling
            // inside the scrolled page caught the swipe, and the end of the table stayed out of reach
            Layout.preferredHeight: window.ui.modern ? contentHeight : 24 * rowHeight
            Layout.minimumHeight: window.ui.modern ? contentHeight : 0
            interactive: !window.ui.modern
            clip: true
            // Modern look: rounded rows apart from each other
            spacing: window.ui.modern ? 8 : 0
            model: gearListModel

            Component.onCompleted: {
                updateGearListModel()
            }

            ScrollBar.vertical: ScrollBar {
                policy: window.ui.modern ? ScrollBar.AlwaysOff : ScrollBar.AsNeeded
            }

            delegate: Rectangle {
                width: gearTable.width
                height: window.ui.modern ? 52 : rowHeight
                radius: window.ui.modern ? 12 : 0
                color: window.ui.modern ? window.ui.surface : (index % 2 === 0 ? "white" : "#fafafa")

                Row {
                    anchors.fill: parent

                    Rectangle {
                        width: parent.width / 2
                        height: parent.height
                        border.width: window.ui.modern ? 0 : 1
                        border.color: window.ui.modern ? window.ui.alpha(window.ui.outline, 0.4) : "#cccccc"
                        color: "transparent"

                        Text {
                            Binding on font.pixelSize { when: window.ui.modern || window.scaledFontPx > 0; value: window.ui.modern ? window.ui.fontLabelLarge : window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                            anchors.centerIn: parent
                            text: gear
                            color: window.ui.ink("black")
                        }
                    }

                    Rectangle {
                        id: offsetCell
                        width: parent.width / 2
                        height: parent.height
                        border.width: window.ui.modern ? 0 : 1
                        border.color: window.ui.modern ? window.ui.alpha(window.ui.outline, 0.4) : "#cccccc"
                        color: "transparent"

                        // A number typed but not confirmed is applied first: the -/+ of the modern
                        // look are MouseAreas, they do not take the focus from the field
                        function stepOffset(delta) {
                            if (window.ui.modern)
                                offsetTextField.applyOffset()
                            gearRows[index].offset = clampOffset(parseOffset(gearRows[index].offset) + delta)
                            offsetTextField.text = gearRows[index].offset
                            saveGearRows()
                        }

                        // Modern look: one rounded field with the -/+ squares inside, as the
                        // Wahoo table; the pill buttons were too narrow for their signs
                        Rectangle {
                            visible: window.ui.modern
                            anchors.fill: offsetRow
                            radius: 8
                            color: window.ui.surfaceHighest
                        }

                        RowLayout {
                            id: offsetRow
                            anchors.centerIn: parent
                            width: window.ui.modern ? Math.min(parent.width * 0.88, 180) : parent.width * 0.92
                            height: window.ui.modern ? 36 : offsetControlHeight
                            spacing: window.ui.modern ? 0 : 4

                            UiButton {
                                visible: !window.ui.modern
                                text: "-"
                                Layout.preferredWidth: window.fontPx(34)
                                Layout.fillHeight: true
                                onClicked: offsetCell.stepOffset(-0.5)
                            }

                            Rectangle {
                                visible: window.ui.modern
                                Layout.preferredWidth: height
                                Layout.fillHeight: true
                                radius: 8
                                color: minusArea.pressed ? window.ui.surfaceHigh : window.ui.surfaceHighest
                                Rectangle {
                                    anchors.centerIn: parent
                                    width: 12; height: 2; radius: 1
                                    color: window.ui.textMain
                                }
                                MouseArea {
                                    id: minusArea
                                    anchors.fill: parent
                                    Accessible.role: Accessible.Button
                                    Accessible.name: qsTr("Decrease the offset of gear %1").arg(gear)
                                    Accessible.onPressAction: offsetCell.stepOffset(-0.5)
                                    onClicked: offsetCell.stepOffset(-0.5)
                                }
                            }

                            TextField {
                                id: offsetTextField
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                text: offset
                                color: window.ui.ink("black")
                                selectedTextColor: window.ui.modern ? window.ui.accentInk : "white"
                                selectionColor: window.ui.modern ? window.ui.accent : Material.accent
                                horizontalAlignment: Text.AlignHCenter
                                // Material pads 8 above and 16 below: with a bigger text the number sat
                                // well over the middle of the field
                                topPadding: window.textScale !== 1 ? 0 : 8
                                bottomPadding: window.textScale !== 1 ? 0 : 16
                                // Modern look: the number keyboard, as the inclination table fields
                                inputMethodHints: window.ui.modern ? Qt.ImhFormattedNumbersOnly : Qt.ImhNone
                                // Modern look: no Material paddings and insets, they differ above
                                // and below and the number sat high in the low field. Bindings that
                                // act only then: "undefined" in the classic look reset them to the Qt
                                // defaults, not to the Material ones the classic field had
                                Binding on topPadding { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                                Binding on bottomPadding { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                                Binding on topInset { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                                Binding on bottomInset { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                                Binding on verticalAlignment { when: window.ui.modern; value: Text.AlignVCenter; restoreMode: Binding.RestoreBindingOrValue }
                                // The 16 px of the +/- fields (UiSpinBox, the inclination table)
                                Binding on font.pixelSize { when: window.ui.modern; value: window.ui.fontBody; restoreMode: Binding.RestoreBindingOrValue }
                                background: Rectangle {
                                    color: window.ui.modern ? "transparent" : "white"
                                    border.color: "#cccccc"
                                    border.width: window.ui.modern ? 0 : 1
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

                            UiButton {
                                visible: !window.ui.modern
                                text: "+"
                                Layout.preferredWidth: window.fontPx(34)
                                Layout.fillHeight: true
                                onClicked: offsetCell.stepOffset(0.5)
                            }

                            Rectangle {
                                visible: window.ui.modern
                                Layout.preferredWidth: height
                                Layout.fillHeight: true
                                radius: 8
                                color: plusArea.pressed ? window.ui.surfaceHigh : window.ui.surfaceHighest
                                Rectangle {
                                    anchors.centerIn: parent
                                    width: 12; height: 2; radius: 1
                                    color: window.ui.textMain
                                }
                                Rectangle {
                                    anchors.centerIn: parent
                                    width: 2; height: 12; radius: 1
                                    color: window.ui.textMain
                                }
                                MouseArea {
                                    id: plusArea
                                    anchors.fill: parent
                                    Accessible.role: Accessible.Button
                                    Accessible.name: qsTr("Increase the offset of gear %1").arg(gear)
                                    Accessible.onPressAction: offsetCell.stepOffset(0.5)
                                    onClicked: offsetCell.stepOffset(0.5)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
