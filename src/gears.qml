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
    id: gearSettingsWindow
    visible: true
    clip: true

    // Properties to store the selected values
    property int selectedCranksetSize: 38
    property int selectedCogSize: 44
    property string selectedWheelSize: "700 x 18C"
    property real selectedCircumference: 2070
    property int initialWheelSizeIndex: 0  // indice per "700 x 18C"

    // Add these connections to your root item (ScrollView)
    Connections {
        target: gearSettingsWindow
        function onGearConfigurationChanged() {
            gearTable.updateGearListModel()
        }
    }

    // Initial gear data
    property var gearRows: [
        { gear: 1, crankset: 38, cog: 44, active: true },
        { gear: 2, crankset: 38, cog: 38, active: true },
        { gear: 3, crankset: 38, cog: 32, active: true },
        { gear: 4, crankset: 38, cog: 28, active: true },
        { gear: 5, crankset: 38, cog: 24, active: true },
        { gear: 6, crankset: 38, cog: 21, active: true },
        { gear: 7, crankset: 38, cog: 19, active: true },
        { gear: 8, crankset: 38, cog: 17, active: true },
        { gear: 9, crankset: 38, cog: 15, active: true },
        { gear: 10, crankset: 38, cog: 13, active: true },
        { gear: 11, crankset: 38, cog: 11, active: true },
        { gear: 12, crankset: 38, cog: 10, active: true }
    ]

    // Initialize components
    Component.onCompleted: {
        wheelSizeCombo.currentIndex = initialWheelSizeIndex
    }

    function clearGearsFromIndex(startIndex) {
            for (let i = startIndex; i < gearRows.length; i++) {
                gearRows[i].active = false
            }
            // Force update
            var temp = gearRows
            gearRows = []
            gearRows = temp
            gearConfigurationChanged(gearRows)
    }

    function initializeGearRows() {
        gearRows = [
            { gear: 1, crankset: 38, cog: 44, active: true },
            { gear: 2, crankset: 38, cog: 38, active: true },
            { gear: 3, crankset: 38, cog: 32, active: true },
            { gear: 4, crankset: 38, cog: 28, active: true },
            { gear: 5, crankset: 38, cog: 24, active: true },
            { gear: 6, crankset: 38, cog: 21, active: true },
            { gear: 7, crankset: 38, cog: 19, active: true },
            { gear: 8, crankset: 38, cog: 17, active: true },
            { gear: 9, crankset: 38, cog: 15, active: true },
            { gear: 10, crankset: 38, cog: 13, active: true },
            { gear: 11, crankset: 38, cog: 11, active: true },
            { gear: 12, crankset: 38, cog: 10, active: true }
        ]
        // Force update
        var temp = gearRows
        gearRows = []
        gearRows = temp
    }

    // Signals to notify when values change
    signal settingsChanged(int crankset, int cog, string wheelSize, real circumference)
    signal gearConfigurationChanged(var gearRows)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        // Crankset Size
        GroupBox {
            title: "Crankset Size (tooth count)"
            Layout.fillWidth: true

            SpinBox {
                from: 1
                to: 60
                value: selectedCranksetSize
                onValueChanged: {
                    selectedCranksetSize = value
                    gearSettingsWindow.settingsChanged(selectedCranksetSize, selectedCogSize,
                        selectedWheelSize, selectedCircumference)
                }
            }
        }

        // Cog Size
        GroupBox {
            title: "Cog Size (tooth count)"
            Layout.fillWidth: true

            SpinBox {
                from: 1
                to: 50
                value: selectedCogSize
                onValueChanged: {
                    selectedCogSize = value
                    gearSettingsWindow.settingsChanged(selectedCranksetSize, selectedCogSize,
                        selectedWheelSize, selectedCircumference)
                }
            }
        }

        // Wheel Size
        GroupBox {
            title: "Wheel Size"
            Layout.fillWidth: true

            ComboBox {
                id: wheelSizeCombo
                width: parent.width
                currentIndex: initialWheelSizeIndex
                textRole: "text"
                model: ListModel {
                    id: wheelSizes
                    ListElement { text: "700 x 18C"; circumference: 2070 }
                    ListElement { text: "700 x 19C"; circumference: 2080 }
                    ListElement { text: "700 x 20C"; circumference: 2086 }
                    ListElement { text: "700 x 23C"; circumference: 2096 }
                    ListElement { text: "700 x 25C"; circumference: 2109 }
                    ListElement { text: "700 x 28C"; circumference: 2127 }
                    ListElement { text: "700 x 30C"; circumference: 2140 }
                    ListElement { text: "700 x 32C"; circumference: 2152 }
                    ListElement { text: "700 x 35C"; circumference: 2171 }
                    ListElement { text: "700 x 38C"; circumference: 2190 }
                    ListElement { text: "700 x 40C"; circumference: 2203 }
                    ListElement { text: "700 x 44C"; circumference: 2230 }
                    ListElement { text: "700 x 45C"; circumference: 2234 }
                    ListElement { text: "700 x 47C"; circumference: 2247 }
                    ListElement { text: "700 x 50C"; circumference: 2265 }
                    ListElement { text: "650 x 20C"; circumference: 1938 }
                    ListElement { text: "650 x 23C"; circumference: 1944 }
                    ListElement { text: "650 x 35A"; circumference: 2090 }
                    ListElement { text: "650 x 38B"; circumference: 2105 }
                    ListElement { text: "650 x 38A"; circumference: 2125 }
                    ListElement { text: "12\" x 1.75\""; circumference: 935 }
                    ListElement { text: "12\" x 1.95\""; circumference: 940 }
                    ListElement { text: "14\" x 1.50\""; circumference: 1020 }
                    ListElement { text: "14\" x 1.75\""; circumference: 1055 }
                    ListElement { text: "16\" x 1.50\""; circumference: 1185 }
                    ListElement { text: "16\" x 1.75\""; circumference: 1195 }
                    ListElement { text: "16\" x 2.00\""; circumference: 1245 }
                    ListElement { text: "16\" x 1-1/8\""; circumference: 1290 }
                    ListElement { text: "16\" x 1-3/8\""; circumference: 1300 }
                    ListElement { text: "18\" x 1.50\""; circumference: 1340 }
                    ListElement { text: "18\" x 1.75\""; circumference: 1350 }
                    ListElement { text: "20\" x 1.25\""; circumference: 1450 }
                    ListElement { text: "20\" x 1.35\""; circumference: 1460 }
                    ListElement { text: "20\" x 1.50\""; circumference: 1490 }
                    ListElement { text: "20\" x 1.75\""; circumference: 1515 }
                    ListElement { text: "20\" x 1.95\""; circumference: 1565 }
                    ListElement { text: "20\" x 1-1/8\""; circumference: 1545 }
                    ListElement { text: "20\" x 1-3/8\""; circumference: 1615 }
                    ListElement { text: "22\" x 1-3/8\""; circumference: 1770 }
                    ListElement { text: "22\" x 1-1/2\""; circumference: 1785 }
                    ListElement { text: "24\" x 3/4\" Tubular"; circumference: 1785 }
                    ListElement { text: "24\" x 1\""; circumference: 1753 }
                    ListElement { text: "24\" x 1-1/8\""; circumference: 1795 }
                    ListElement { text: "24\" x 1-1/4\""; circumference: 1905 }
                    ListElement { text: "24\" x 1.75\""; circumference: 1890 }
                    ListElement { text: "24\" x 2.00\""; circumference: 1925 }
                    ListElement { text: "24\" x 2.125\""; circumference: 1965 }
                    ListElement { text: "26\" x 7/8\" Tubular"; circumference: 1920 }
                    ListElement { text: "26\" x 1.25\""; circumference: 1950 }
                    ListElement { text: "26\" x 1.40\""; circumference: 2005 }
                    ListElement { text: "26\" x 1.50\""; circumference: 2010 }
                    ListElement { text: "26\" x 1.75\""; circumference: 2023 }
                    ListElement { text: "26\" x 1.95\""; circumference: 2050 }
                    ListElement { text: "26\" x 2.00\""; circumference: 2055 }
                    ListElement { text: "26\" x 2.10\""; circumference: 2068 }
                    ListElement { text: "26\" x 2.125\""; circumference: 2070 }
                    ListElement { text: "26\" x 2.35\""; circumference: 2083 }
                    ListElement { text: "26\" x 3.00\""; circumference: 2170 }
                    ListElement { text: "26\" x 1-1.0\""; circumference: 1913 }
                    ListElement { text: "26\" x 1\""; circumference: 1952 }
                    ListElement { text: "26\" x 1-1/8\""; circumference: 1970 }
                    ListElement { text: "26\" x 1-3/8\""; circumference: 2068 }
                    ListElement { text: "26\" x 1-1/2\""; circumference: 2100 }
                    ListElement { text: "27\" x 1\""; circumference: 2145 }
                    ListElement { text: "27\" x 1-1/8\""; circumference: 2155 }
                    ListElement { text: "27\" x 1-1/4\""; circumference: 2161 }
                    ListElement { text: "27\" x 1-3/8\""; circumference: 2169 }
                    ListElement { text: "27.5\" / 650B x 1.50\""; circumference: 2079 }
                    ListElement { text: "27.5\" / 650B x 1.95\""; circumference: 2090 }
                    ListElement { text: "27.5\" / 650B x 2.10\""; circumference: 2148 }
                    ListElement { text: "27.5\" / 650B x 2.25\""; circumference: 2182 }
                    ListElement { text: "27.5\" / 650B x 2.3\""; circumference: 2199 }
                    ListElement { text: "27.5\" / 650B x 2.35\""; circumference: 2207 }
                    ListElement { text: "27.5\" / 650B x 2.4\""; circumference: 2213 }
                    ListElement { text: "27.5\" / 650B x 2.5\""; circumference: 2231 }
                    ListElement { text: "27.5\" / 650B x 2.6\""; circumference: 2247 }
                    ListElement { text: "27.5\" / 650B x 2.8\""; circumference: 2279 }
                    ListElement { text: "29\" x 2.1\""; circumference: 2286 }
                    ListElement { text: "29\" x 2.2\""; circumference: 2302 }
                    ListElement { text: "29\" x 2.25\""; circumference: 2310 }
                    ListElement { text: "29\" x 2.3\""; circumference: 2326 }
                    ListElement { text: "29\" x 2.35\""; circumference: 2326 }
                    ListElement { text: "29\" x 2.4\""; circumference: 2333 }
                    ListElement { text: "29\" x 2.5\""; circumference: 2350 }
                    ListElement { text: "29\" x 2.6\""; circumference: 2366 }
                }
                onCurrentIndexChanged: {
                    if (currentIndex >= 0) {
                        selectedWheelSize = model.get(currentIndex).text
                        selectedCircumference = model.get(currentIndex).circumference
                        gearSettingsWindow.settingsChanged(selectedCranksetSize, selectedCogSize,
                            selectedWheelSize, selectedCircumference)
                    }
                }
            }
        }

        // Gear Table GroupBox
        GroupBox {
            title: "Gear Table"
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: parent.height

            ColumnLayout {
                anchors.fill: parent
                spacing: 10

                // Buttons (same as before)
                RowLayout {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    spacing: 10

                    Button {
                        text: "Clear Selected Gear and Following"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 40
                        onClicked: {
                            if (gearTable.currentRow >= 0) {
                                clearGearsFromIndex(gearTable.currentRow)
                            }
                        }
                    }

                    Button {
                        text: "Reset All Gears"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 40
                        onClicked: initializeGearRows()
                    }
                }

                // Table Header (same as before)
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    color: "#f0f0f0"
                    border.width: 1
                    border.color: "#cccccc"

                    Row {
                        anchors.fill: parent

                        Rectangle {
                            width: parent.width / 3
                            height: parent.height
                            border.width: 1
                            border.color: "#cccccc"
                            color: "transparent"

                            Text {
                                anchors.centerIn: parent
                                text: "Gear"
                                font.bold: true
                                color: "black"
                            }
                        }

                        Rectangle {
                            width: parent.width / 3
                            height: parent.height
                            border.width: 1
                            border.color: "#cccccc"
                            color: "transparent"

                            Text {
                                anchors.centerIn: parent
                                text: "Crankset"
                                font.bold: true
                                color: "black"
                            }
                        }

                        Rectangle {
                            width: parent.width / 3
                            height: parent.height
                            border.width: 1
                            border.color: "#cccccc"
                            color: "transparent"

                            Text {
                                anchors.centerIn: parent
                                text: "Rear Cog"
                                font.bold: true
                                color: "black"
                            }
                        }
                    }
                }

                // Table Content
                ListView {
                    id: gearTable
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    property int currentRow: -1
                    model: ListModel {
                        id: gearListModel
                    }

                    Component.onCompleted: {
                        updateGearListModel()
                    }

                    function updateGearListModel() {
                        gearListModel.clear()
                        for (var i = 0; i < gearRows.length; i++) {
                            if (gearRows[i].active) {
                                gearListModel.append(gearRows[i])
                            }
                        }
                    }

                    delegate: Rectangle {
                        width: gearTable.width
                        height: 40
                        color: gearTable.currentRow === index ? "#e0e0e0" : "white"

                        MouseArea {
                            anchors.fill: parent
                            onClicked: gearTable.currentRow = index
                        }

                        Row {
                            anchors.fill: parent

                            // Gear Number (non-editable)
                            Rectangle {
                                width: parent.width / 3
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

                            // Crankset (editable)
                            Rectangle {
                                width: parent.width / 3
                                height: parent.height
                                border.width: 1
                                border.color: "#cccccc"
                                color: "transparent"

                                SpinBox {
                                    id: cranksetSpinBox
                                    anchors.centerIn: parent
                                    width: parent.width * 0.8
                                    height: 30
                                    from: 1
                                    to: 60
                                    value: crankset
                                    onValueModified: {
                                        gearRows[index].crankset = value
                                        gearConfigurationChanged(gearRows)
                                    }

                                    // Style the SpinBox
                                    contentItem: TextInput {
                                        z: 2
                                        text: cranksetSpinBox.textFromValue(cranksetSpinBox.value, cranksetSpinBox.locale)
                                        font: cranksetSpinBox.font
                                        color: "black"
                                        selectionColor: "#21be2b"
                                        selectedTextColor: "#ffffff"
                                        horizontalAlignment: Qt.AlignHCenter
                                        verticalAlignment: Qt.AlignVCenter
                                    }

                                    up.indicator: Rectangle {
                                        x: parent.width - width
                                        height: parent.height
                                        width: height
                                        color: parent.up.pressed ? "#e4e4e4" : "#f6f6f6"
                                        border.color: "#cccccc"

                                        Text {
                                            text: "+"
                                            color: "black"
                                            anchors.centerIn: parent
                                            font.pixelSize: 12
                                        }
                                    }

                                    down.indicator: Rectangle {
                                        x: 0
                                        height: parent.height
                                        width: height
                                        color: parent.down.pressed ? "#e4e4e4" : "#f6f6f6"
                                        border.color: "#cccccc"

                                        Text {
                                            text: "-"
                                            color: "black"
                                            anchors.centerIn: parent
                                            font.pixelSize: 12
                                        }
                                    }

                                    background: Rectangle {
                                        color: "white"
                                        border.color: "#cccccc"
                                    }
                                }
                            }

                            // Rear Cog (editable)
                            Rectangle {
                                width: parent.width / 3
                                height: parent.height
                                border.width: 1
                                border.color: "#cccccc"
                                color: "transparent"

                                SpinBox {
                                    id: cogSpinBox
                                    anchors.centerIn: parent
                                    width: parent.width * 0.8
                                    height: 30
                                    from: 1
                                    to: 50
                                    value: cog
                                    onValueModified: {
                                        gearRows[index].cog = value
                                        gearConfigurationChanged(gearRows)
                                    }

                                    // Style the SpinBox (same as cranksetSpinBox)
                                    contentItem: TextInput {
                                        z: 2
                                        text: cogSpinBox.textFromValue(cogSpinBox.value, cogSpinBox.locale)
                                        font: cogSpinBox.font
                                        color: "black"
                                        selectionColor: "#21be2b"
                                        selectedTextColor: "#ffffff"
                                        horizontalAlignment: Qt.AlignHCenter
                                        verticalAlignment: Qt.AlignVCenter
                                    }

                                    up.indicator: Rectangle {
                                        x: parent.width - width
                                        height: parent.height
                                        width: height
                                        color: parent.up.pressed ? "#e4e4e4" : "#f6f6f6"
                                        border.color: "#cccccc"

                                        Text {
                                            text: "+"
                                            color: "black"
                                            anchors.centerIn: parent
                                            font.pixelSize: 12
                                        }
                                    }

                                    down.indicator: Rectangle {
                                        x: 0
                                        height: parent.height
                                        width: height
                                        color: parent.down.pressed ? "#e4e4e4" : "#f6f6f6"
                                        border.color: "#cccccc"

                                        Text {
                                            text: "-"
                                            color: "black"
                                            anchors.centerIn: parent
                                            font.pixelSize: 12
                                        }
                                    }

                                    background: Rectangle {
                                        color: "white"
                                        border.color: "#cccccc"
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
