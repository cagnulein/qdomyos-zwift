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
    // The column with its margins, which are not in its implicit height
    contentHeight: chainringColumn.implicitHeight + 2 * chainringColumn.anchors.margins
    focus: true
    // No anchors: the page is pushed on the StackView, which sizes it to fill and warned about
    // conflicting anchors
    id: gearSettingsWindow
    visible: true
    clip: true

    // Properties
    // The Settings object of settings.qml, given by the page that opens this one: one object
    // for all the settings pages, so a page no longer writes back an old copy of what another
    // page changed
    property var settings

    property int selectedCranksetSize: settings.gear_crankset_size
    property int selectedCogSize: settings.gear_cog_size
    property string selectedWheelSize: settings.gear_wheel_size
    property real selectedCircumference: settings.gear_circumference
    property bool inited: false

    property int initialWheelSizeIndex: {
          // Trova l'indice corretto basato sul valore salvato
          for (let i = 0; i < wheelSizes.count; i++) {
              if (wheelSizes.get(i).text === settings.gear_wheel_size) {
                  return i;
              }
          }
          return 0; // default se non trovato
      }

    Component.onCompleted: {
        if (settings.gear_configuration) {
            gearRows = stringToGearRows(settings.gear_configuration)
        }
        restoreDefaultWheelDiameter.connect(rootItem.restoreDefaultWheelDiameter)
        console.log("Component.onCompleted " + settings.gear_crankset_size + " " + settings.gear_cog_size + " " + settings.gear_wheel_size + " " + settings.gear_circumference)
        wheelSizeCombo.currentIndex = initialWheelSizeIndex
        selectedCranksetSize = settings.gear_crankset_size
        selectedCogSize = settings.gear_cog_size
        inited = true
    }

    function updateSettings() {
        if(!inited)
            return;
        settings.gear_crankset_size = selectedCranksetSize
        settings.gear_cog_size = selectedCogSize
        settings.gear_wheel_size = selectedWheelSize
        settings.gear_circumference = selectedCircumference
    }

    function stringToGearRows(gearString) {
        if (!gearString) return []

        return gearString.split("\n").map(function(row) {
            const parts = row.split("|")
            return {
                gear: parseInt(parts[0]),
                crankset: parseInt(parts[1]),
                cog: parseInt(parts[2]),
                active: parts[3] === "true"
            }
        })
    }

    function gearRowsToString(gearRows) {
        return gearRows.map(function(row) {
            return row.gear + "|" + row.crankset + "|" + row.cog + "|" + row.active
        }).join("\n")
    }

    // Monitora i cambiamenti nelle gear e salva automaticamente
    onGearConfigurationChanged: {
        settings.gear_configuration = gearRowsToString(gearRows)
    }

    onGearSettingsChanged: {
        console.log("onGearSettingsChanged")
        updateSettings()
    }

    Connections {
        target: gearSettingsWindow
        function onGearConfigurationChanged() {
            gearTable.updateGearListModel()
        }
    }

    function loadGearProfile(profileName) {
        if (profileName in gearProfiles) {
            const profile = gearProfiles[profileName]

            // Create new array with copied objects
            var newGears = []
            for (var i = 0; i < profile.gears.length; i++) {
                newGears.push({
                    gear: profile.gears[i].gear,
                    crankset: profile.gears[i].crankset,
                    cog: profile.gears[i].cog,
                    active: profile.gears[i].active
                })
            }

            gearRows = newGears

            // Force update
            var temp = gearRows
            gearRows = []
            gearRows = temp
            gearConfigurationChanged(gearRows)
        }
    }

    // The preset with the gears of this table, -1 if none. Only the table is saved, not the pick,
    // so the list went back to "Select a profile..." every time the page opened and the preset
    // looked lost. Rows taken out of the table stay in the string as inactive and do not count
    function gearProfileIndex(gearString) {
        const table = gearRowsToString(stringToGearRows(gearString).filter(function(row) {
            return row.active
        }))
        for (var i = 0; i < profileModel.count; i++) {
            if (gearRowsToString(gearProfiles[profileModel.get(i).value].gears) === table) {
                return i
            }
        }
        return -1
    }

    property var gearProfiles: {
        "Time Trial": {
            name: "Time Trial (52/36, 10 - 28)",
            gears: [
                { gear: 1, crankset: 36, cog: 28, active: true },
                { gear: 2, crankset: 36, cog: 24, active: true },
                { gear: 3, crankset: 36, cog: 21, active: true },
                { gear: 4, crankset: 36, cog: 19, active: true },
                { gear: 5, crankset: 36, cog: 18, active: true },
                { gear: 6, crankset: 36, cog: 17, active: true },
                { gear: 7, crankset: 36, cog: 16, active: true },
                { gear: 8, crankset: 36, cog: 15, active: true },
                { gear: 9, crankset: 36, cog: 14, active: true },
                { gear: 10, crankset: 52, cog: 19, active: true },
                { gear: 11, crankset: 52, cog: 18, active: true },
                { gear: 12, crankset: 52, cog: 17, active: true },
                { gear: 13, crankset: 52, cog: 16, active: true },
                { gear: 14, crankset: 52, cog: 15, active: true },
                { gear: 15, crankset: 52, cog: 14, active: true },
                { gear: 16, crankset: 52, cog: 13, active: true },
                { gear: 17, crankset: 52, cog: 12, active: true },
                { gear: 18, crankset: 52, cog: 11, active: true },
                { gear: 19, crankset: 52, cog: 10, active: true }
            ]
        },
        "Rolling Hills": {
            name: "Rolling Hills (46/33, 10 - 33)",
            gears: [
                { gear: 1, crankset: 33, cog: 33, active: true },
                { gear: 2, crankset: 33, cog: 28, active: true },
                { gear: 3, crankset: 33, cog: 24, active: true },
                { gear: 4, crankset: 33, cog: 21, active: true },
                { gear: 5, crankset: 33, cog: 19, active: true },
                { gear: 6, crankset: 33, cog: 17, active: true },
                { gear: 7, crankset: 33, cog: 15, active: true },
                { gear: 8, crankset: 46, cog: 19, active: true },
                { gear: 9, crankset: 46, cog: 17, active: true },
                { gear: 10, crankset: 46, cog: 15, active: true },
                { gear: 11, crankset: 46, cog: 14, active: true },
                { gear: 12, crankset: 46, cog: 13, active: true },
                { gear: 13, crankset: 46, cog: 12, active: true },
                { gear: 14, crankset: 46, cog: 11, active: true },
                { gear: 15, crankset: 46, cog: 10, active: true }
            ]
        },
        "Alpine": {
            name: "Alpine (43/30, 10 - 36)",
            gears: [
                { gear: 1, crankset: 30, cog: 36, active: true },
                { gear: 2, crankset: 30, cog: 32, active: true },
                { gear: 3, crankset: 30, cog: 28, active: true },
                { gear: 4, crankset: 30, cog: 24, active: true },
                { gear: 5, crankset: 30, cog: 21, active: true },
                { gear: 6, crankset: 30, cog: 19, active: true },
                { gear: 7, crankset: 30, cog: 17, active: true },
                { gear: 8, crankset: 30, cog: 15, active: true },
                { gear: 9, crankset: 43, cog: 19, active: true },
                { gear: 10, crankset: 43, cog: 17, active: true },
                { gear: 11, crankset: 43, cog: 15, active: true },
                { gear: 12, crankset: 43, cog: 13, active: true },
                { gear: 13, crankset: 43, cog: 12, active: true },
                { gear: 14, crankset: 43, cog: 11, active: true },
                { gear: 15, crankset: 43, cog: 10, active: true }
            ]
        },
        "Reality Bender": {
            name: "Reality Bender (24 even spaced)",
            gears: [
                { gear: 1, crankset: 75, cog: 100, active: true },
                { gear: 2, crankset: 87, cog: 100, active: true },
                { gear: 3, crankset: 99, cog: 100, active: true },
                { gear: 4, crankset: 111, cog: 100, active: true },
                { gear: 5, crankset: 123, cog: 100, active: true },
                { gear: 6, crankset: 138, cog: 100, active: true },
                { gear: 7, crankset: 153, cog: 100, active: true },
                { gear: 8, crankset: 168, cog: 100, active: true },
                { gear: 9, crankset: 186, cog: 100, active: true },
                { gear: 10, crankset: 204, cog: 100, active: true },
                { gear: 11, crankset: 222, cog: 100, active: true },
                { gear: 12, crankset: 240, cog: 100, active: true },
                { gear: 13, crankset: 261, cog: 100, active: true },
                { gear: 14, crankset: 282, cog: 100, active: true },
                { gear: 15, crankset: 303, cog: 100, active: true },
                { gear: 16, crankset: 324, cog: 100, active: true },
                { gear: 17, crankset: 349, cog: 100, active: true },
                { gear: 18, crankset: 374, cog: 100, active: true },
                { gear: 19, crankset: 399, cog: 100, active: true },
                { gear: 20, crankset: 424, cog: 100, active: true },
                { gear: 21, crankset: 454, cog: 100, active: true },
                { gear: 22, crankset: 484, cog: 100, active: true },
                { gear: 23, crankset: 514, cog: 100, active: true },
                { gear: 24, crankset: 549, cog: 100, active: true }
            ]
        },
        "Explorer": {
            name: "Explorer (40, 10 - 46)",
            gears: [
                { gear: 1, crankset: 40, cog: 46, active: true },
                { gear: 2, crankset: 40, cog: 38, active: true },
                { gear: 3, crankset: 40, cog: 32, active: true },
                { gear: 4, crankset: 40, cog: 28, active: true },
                { gear: 5, crankset: 40, cog: 24, active: true },
                { gear: 6, crankset: 40, cog: 21, active: true },
                { gear: 7, crankset: 40, cog: 19, active: true },
                { gear: 8, crankset: 40, cog: 17, active: true },
                { gear: 9, crankset: 40, cog: 15, active: true },
                { gear: 10, crankset: 40, cog: 13, active: true },
                { gear: 11, crankset: 40, cog: 12, active: true },
                { gear: 12, crankset: 40, cog: 11, active: true },
                { gear: 13, crankset: 40, cog: 10, active: true }
            ]
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

    function addNewGear() {
        // Find the first inactive gear or add at the end
        let newGearIndex = gearRows.findIndex(row => !row.active);
        if (newGearIndex === -1) {
            newGearIndex = gearRows.length;
        }

        // Create new gear with default values
        const newGear = {
            gear: newGearIndex + 1,
            crankset: selectedCranksetSize,
            cog: selectedCogSize,
            active: true
        };

        if (newGearIndex < gearRows.length) {
            gearRows[newGearIndex] = newGear;
        } else {
            gearRows.push(newGear);
        }

        // Force update
        var temp = gearRows;
        gearRows = [];
        gearRows = temp;
        gearConfigurationChanged(gearRows);
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
        // Without it the table kept the removed gears and the reset was not saved
        gearConfigurationChanged(gearRows)
    }

    // Signals to notify when values change (not settingsChanged: that is the change signal of
    // the settings property)
    signal gearSettingsChanged()
    signal gearConfigurationChanged(var gearRows)
    signal restoreDefaultWheelDiameter()

    ColumnLayout {
        // Not tied to the bottom: the column height would come from the content height made from
        // it (binding loop on contentHeight)
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: window.ui.modern ? Math.max(16, window.contentSideMargin) : 20
        spacing: window.ui.modern ? 8 : 20 // modern: 8 between the cards, as Material 3
        id: chainringColumn

        // Wahoo Options
        UiGroupBox {
            framed: true
            title: qsTr("Wahoo Options")
            // the title is a stop of its own, as the controls in the group are
            Accessible.focusable: true
            Layout.fillWidth: true

            ColumnLayout {
                IndicatorOnlySwitch {
                    id: wahooWithoutWheelDiameterDelegate
                    // a switch a screen reader can turn: Qt 5.15 gives a SwitchDelegate no action of
                    // its own (and puts back its ListItem role), so it is not even a pressable node,
                    // and a tap in the middle of the row misses the indicator of IndicatorOnlySwitch.
                    // On Android CustomQtActivity presses it as a button; the handler is for the rest
                    Accessible.name: text
                    Accessible.checkable: true
                    Accessible.checked: checked
                    Accessible.onPressAction: {
                        checked = !checked
                        clicked()
                    }
                    text: qsTr("Without Wheel Diameter Protocol")
                    spacing: 0
                    bottomPadding: 0
                    topPadding: 0
                    rightPadding: 0
                    leftPadding: 0
                    clip: false
                    checked: settings.wahoo_without_wheel_diameter
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    Layout.fillWidth: true
                    onClicked: settings.wahoo_without_wheel_diameter = checked
                }

                Label {
                    // a stop of its own: otherwise TalkBack reads the whole group in one go
                    Accessible.focusable: true
                    text: qsTr("Enable this for simplified Wahoo protocol that adds gears directly to grade instead of using wheel diameter changes. Default is false.")
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    Layout.maximumWidth: chainringColumn.width - 20
                    font.pixelSize: window.fontHint // modern look: Body Small, as the hints of the settings
                    bottomPadding: window.hintGap
                    color: window.ui.modern ? window.ui.textMuted : Material.accent
                }
            }
        }

        // Crankset Size
        UiGroupBox {
            framed: true
            title: qsTr("Chainring Size")
            Accessible.focusable: true
            Layout.fillWidth: true

            ColumnLayout {
                Label {
                    Accessible.focusable: true
                    text: qsTr("Tooth count of your chainring on the bike you are currently riding on your trainer - enter 42 for Zwift Ride")
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    Layout.maximumWidth: chainringColumn.width - 20
                    color: window.ui.modern ? window.ui.textMuted : Material.foreground
                    // Modern look: the note of the group is a hint, Body Small; the classic keeps the
                    // page font. The page is made while the look is set
                    Binding on font.pixelSize { when: window.ui.modern; value: window.fontHint; restoreMode: Binding.RestoreBindingOrValue }
                }

                SpinBox {
                    visible: !window.ui.modern
                    Accessible.name: qsTr("Chainring Size") + ": " + value
                    from: 1
                    to: 999
                    value: selectedCranksetSize
                    onValueChanged: {
                        selectedCranksetSize = value
                        console.log("Crankset Size changed");
                        gearSettingsChanged()
                    }
                }
                // Modern look: the buttons of the table below. The Material one kept its +/- lit
                // after a tap and did not match the rest of the page
                UiSpinBox {
                    visible: window.ui.modern
                    Accessible.name: qsTr("Chainring Size") + ": " + value
                    // TalkBack changes the value with increase()/decrease(), which do not emit
                    // valueModified, the signal that saves the field
                    Accessible.onIncreaseAction: {
                        increase()
                        valueModified()
                    }
                    Accessible.onDecreaseAction: {
                        decrease()
                        valueModified()
                    }
                    from: 1
                    to: 999
                    value: selectedCranksetSize
                    onValueModified: {
                        selectedCranksetSize = value
                        console.log("Crankset Size changed");
                        gearSettingsChanged()
                    }
                }
            }
        }

        // Cog Size
        UiGroupBox {
            framed: true
            title: qsTr("Cog Size")
            Accessible.focusable: true
            Layout.fillWidth: true

            ColumnLayout {
                Label {
                    Accessible.focusable: true
                    text: qsTr("Tooth count of your rear cog on your trainer - enter 14 if you have the Zwift Cog")
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    Layout.maximumWidth: chainringColumn.width - 20
                    color: window.ui.modern ? window.ui.textMuted : Material.foreground
                    // Modern look: the note of the group is a hint, Body Small; the classic keeps the
                    // page font. The page is made while the look is set
                    Binding on font.pixelSize { when: window.ui.modern; value: window.fontHint; restoreMode: Binding.RestoreBindingOrValue }
                }
                SpinBox {
                    visible: !window.ui.modern
                    Accessible.name: qsTr("Cog Size") + ": " + value
                    from: 1
                    to: 999
                    value: selectedCogSize
                    onValueChanged: {
                        selectedCogSize = value
                        console.log("Cog Size changed");
                        gearSettingsChanged()
                    }
                }
                // Modern look: the buttons of the table below. The Material one kept its +/- lit
                // after a tap and did not match the rest of the page
                UiSpinBox {
                    visible: window.ui.modern
                    Accessible.name: qsTr("Cog Size") + ": " + value
                    // TalkBack changes the value with increase()/decrease(), which do not emit
                    // valueModified, the signal that saves the field
                    Accessible.onIncreaseAction: {
                        increase()
                        valueModified()
                    }
                    Accessible.onDecreaseAction: {
                        decrease()
                        valueModified()
                    }
                    from: 1
                    to: 999
                    value: selectedCogSize
                    onValueModified: {
                        selectedCogSize = value
                        console.log("Cog Size changed");
                        gearSettingsChanged()
                    }
                }
            }
        }

        // Wheel Size
        UiGroupBox {
            framed: true
            title: qsTr("Virtual Wheel Size")
            Accessible.focusable: true
            Layout.fillWidth: true

            // Modern look: across the card like the profile list below; without a width the
            // layout was as wide as the button text, and the button hung off to the left
            ColumnLayout {
                // With a bigger text the column took the width of the button label in one line and
                // the button ran past the screen edge: it takes the width of the box instead
                width: window.ui.modern || window.textScale !== 1 ? parent.width : implicitWidth
                UiComboBox {
                       id: wheelSizeCombo
                       Accessible.name: qsTr("Virtual Wheel Size") + ": " + displayText
                       width: parent.width
                       // the box was as wide as at 100 % and cut the wheel size ("700 x 1…")
                       Layout.fillWidth: window.ui.modern || window.textScale !== 1
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
                          console.log("wheelSizeCombo changed");
                          gearSettingsChanged()
                      }
                  }
                }

                // Restore Default Wheel Diameter Button
                UiButton {
                    applyOnSelect: false
                    text: qsTr("Restore Default Setting to the Trainer")
                    Layout.fillWidth: true
                    // Modern look: a long label at a large text size takes two lines, not the
                    // width of the page
                    // with a bigger text the label wraps in two lines
                    Layout.preferredHeight: window.ui.modern ? Math.max(48, contentItem.implicitHeight + 24) : (window.textScale !== 1 ? Math.max(50, contentItem.implicitHeight + 16) : 50)
                    font.bold: true
                    background: Rectangle {
                        // Modern look: the tonal pill of the other buttons, the bright purple
                        // stood out of the page palette for a reset
                        color: window.ui.modern ? (parent.down ? window.ui.surfaceHigh : window.ui.surfaceHighest)
                                                : "#9C27B0"
                        radius: window.ui.modern ? height / 2 : 4
                    }
                    contentItem: Text {
                        text: parent.text
                        font: parent.labelFont
                        color: window.ui.ink("white")
                        wrapMode: window.ui.modern ? Text.WordWrap : window.labelWrap
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: {
                        // Emit signal to restore default wheel diameter in the trainer
                        restoreDefaultWheelDiameter()
                    }
                }
            }
        }

        UiGroupBox {
            framed: true
        title: qsTr("Preset Gear Profiles")
        Accessible.focusable: true
        Layout.fillWidth: true

            UiComboBox {
                id: profileCombo
                Accessible.name: qsTr("Preset Gear Profiles") + ": " + displayText
                width: parent.width
                textRole: "text"
                savedIndex: gearProfileIndex(settings.gear_configuration)
                displayText: currentIndex < 0 ? qsTr("Select a profile...") : model.get(currentIndex).text
                model: ListModel {
                 id: profileModel
                }

                Component.onCompleted: {
                 for (var key in gearProfiles) {
                     profileModel.append({
                         text: gearProfiles[key].name,
                         value: key
                     })
                 }
                }

                // Only a pick loads a preset, not the list showing the saved one
                onActivated: {
                 if (index >= 0) {
                     loadGearProfile(profileModel.get(index).value)
                 }
                }
                }
        }


        // Gear Table GroupBox
        UiGroupBox {
            framed: true
            title: qsTr("Virtual Gear Table")
            Accessible.focusable: true
            Layout.fillWidth: true
            // The table is as tall as its rows and the page scrolls as one: a list scrolling inside
            // the scrolling page caught the finger half way down, and in the classic look, which
            // gave the table the rest of the page, the rest was nothing and the rows were not shown

            ColumnLayout {
                width: parent.width
                spacing: 10

                // Updated Buttons Row. Modern look: one button per line, three long labels
                // side by side were cut on a phone
                // With a bigger text three buttons in a row cut their labels ("Add…"): one under
                // another, each the full width
                GridLayout {
                    Layout.fillWidth: true
                    Layout.preferredHeight: window.ui.modern ? -1 : (window.textScale > 1 ? -1 : 40)
                    columns: window.ui.modern ? 1 : (window.textScale > 1 ? 1 : 3)
                    columnSpacing: 10
                    rowSpacing: window.ui.modern ? 8 : 10

                    UiButton {
                        wrapsText: true
                        text: qsTr("Add Gear")
                        Layout.fillWidth: true
                        Layout.preferredHeight: window.ui.modern ? Math.max(48, contentItem.implicitHeight + 24) : window.fontPx(40)
                        onClicked: addNewGear()
                    }

                    UiButton {
                        wrapsText: true
                        text: qsTr("Clear Selected Gear and Following")
                        Layout.fillWidth: true
                        Layout.preferredHeight: window.ui.modern ? Math.max(48, contentItem.implicitHeight + 24) : window.fontPx(40)
                        onClicked: {
                            if (gearTable.currentRow >= 0) {
                                clearGearsFromIndex(gearTable.currentRow)
                            }
                        }
                    }

                    UiButton {
                        wrapsText: true
                        text: qsTr("Reset All Gears")
                        Layout.fillWidth: true
                        Layout.preferredHeight: window.ui.modern ? Math.max(48, contentItem.implicitHeight + 24) : window.fontPx(40)
                        onClicked: initializeGearRows()
                    }
                }

                // Table Header (same as before)
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: window.fontPx(40)
                    color: window.ui.modern ? window.ui.surfaceHigh : "#f0f0f0"
                    border.width: window.ui.modern ? 0 : 1
                    border.color: window.ui.modern ? window.ui.alpha(window.ui.outline, 0.4) : "#cccccc"
                    radius: window.ui.modern ? 12 : 0

                    // Modern look: a narrow gear number, the chainring and cog fields get the room
                    Row {
                        anchors.fill: parent

                        Rectangle {
                            width: parent.width * (window.ui.modern ? 0.24 : 1 / 3)
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
                            width: parent.width * (window.ui.modern ? 0.38 : 1 / 3)
                            height: parent.height
                            border.width: window.ui.modern ? 0 : 1
                            border.color: window.ui.modern ? window.ui.alpha(window.ui.outline, 0.4) : "#cccccc"
                            color: "transparent"

                            Text {
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
                                text: qsTr("Chainring")
                                font.bold: true
                                color: window.ui.ink("black")
                            }
                        }

                        Rectangle {
                            width: parent.width * (window.ui.modern ? 0.38 : 1 / 3)
                            height: parent.height
                            border.width: window.ui.modern ? 0 : 1
                            border.color: window.ui.modern ? window.ui.alpha(window.ui.outline, 0.4) : "#cccccc"
                            color: "transparent"

                            Text {
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
                                text: qsTr("Rear Cog")
                                font.bold: true
                                color: window.ui.ink("black")
                            }
                        }
                    }
                }

                // Table Content
                ListView {
                    id: gearTable
                    Layout.fillWidth: true
                    Layout.preferredHeight: contentHeight
                    // The page margins are not in the implicit height: never squeeze the rows
                    Layout.minimumHeight: contentHeight
                    interactive: false
                    clip: true
                    // Modern look: rounded rows apart from each other
                    spacing: window.ui.modern ? 8 : 0
                    property int currentRow: -1
                    model: ListModel {
                        id: gearListModel
                    }

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AlwaysOff
                    }


                    Component.onCompleted: {
                        updateGearListModel()
                    }

                    function updateGearListModel() {
                        var activeRows = []
                        for (var i = 0; i < gearRows.length; i++) {
                            if (gearRows[i].active) {
                                activeRows.push(gearRows[i])
                            }
                        }
                        // one cell edited: update that row only, so the rows (and the cell
                        // TalkBack is on) are not made anew
                        if (activeRows.length === gearListModel.count) {
                            var changed = -1
                            var changedRows = 0
                            for (var j = 0; j < activeRows.length; j++) {
                                var row = gearListModel.get(j)
                                if (row.gear !== activeRows[j].gear || row.crankset !== activeRows[j].crankset || row.cog !== activeRows[j].cog) {
                                    changed = j
                                    changedRows++
                                }
                            }
                            if (changedRows <= 1) {
                                if (changed >= 0) {
                                    gearListModel.set(changed, activeRows[changed])
                                }
                                return
                            }
                        }
                        gearListModel.clear()
                        for (var k = 0; k < activeRows.length; k++) {
                            gearListModel.append(activeRows[k])
                        }
                    }

                    delegate: Rectangle {
                        width: gearTable.width
                        // the rows, the fields and their -/+ grow with the text
                        height: window.ui.modern ? 52 : window.fontPx(40)
                        radius: window.ui.modern ? 12 : 0
                        color: window.ui.modern ? (gearTable.currentRow === index ? window.ui.alpha(window.ui.accent, 0.14) : window.ui.surface) : (gearTable.currentRow === index ? "#e0e0e0" : "white")

                        MouseArea {
                            anchors.fill: parent
                            onClicked: gearTable.currentRow = index
                        }

                        Row {
                            anchors.fill: parent

                            // Gear Number (non-editable). A tap anywhere on the row chooses it (the
                            // MouseArea above); for a screen reader the number is the button that
                            // does, so "Delete Selected and Following" can be used with TalkBack too
                            AbstractButton {
                                width: parent.width * (window.ui.modern ? 0.24 : 1 / 3)
                                height: parent.height
                                focusPolicy: Qt.NoFocus
                                Accessible.name: qsTr("Gear") + " " + gear
                                // checked on the chosen row, so a screen reader says it; a press
                                // toggles it, so the binding is put back after choosing the row
                                checkable: true
                                checked: gearTable.currentRow === index
                                onClicked: {
                                    gearTable.currentRow = index
                                    checked = Qt.binding(function() { return gearTable.currentRow === index })
                                }

                                background: Rectangle {
                                    border.width: window.ui.modern ? 0 : 1
                                    border.color: window.ui.modern ? window.ui.alpha(window.ui.outline, 0.4) : "#cccccc"
                                    color: "transparent"
                                }

                                contentItem: Text {
                                    // a plain Text does not inherit the window font: the text size while it is not 100 %; modern look: the table header and the gear number Label Large 14
                                    Binding on font.pixelSize { when: window.ui.modern || window.scaledFontPx > 0; value: window.ui.modern ? window.ui.fontLabelLarge : window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                                    text: gear
                                    color: window.ui.ink("black")
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    Accessible.ignored: true
                                }
                            }

                            // Crankset (editable)
                            Rectangle {
                                width: parent.width * (window.ui.modern ? 0.38 : 1 / 3)
                                height: parent.height
                                border.width: window.ui.modern ? 0 : 1
                                border.color: window.ui.modern ? window.ui.alpha(window.ui.outline, 0.4) : "#cccccc"
                                color: "transparent"

                                SpinBox {
                                    id: cranksetSpinBox
                                    // the column headers are shortened in some languages to fit; the
                                    // name a screen reader speaks uses the full label of the field above
                                    Accessible.name: qsTr("Gear") + " " + gear + ", " + qsTr("Chainring Size") + ": " + value
                                    // TalkBack changes the value with increase()/decrease(), which do not emit
                                    // valueModified, the signal that saves the table
                                    Accessible.onIncreaseAction: {
                                        increase()
                                        valueModified()
                                    }
                                    Accessible.onDecreaseAction: {
                                        decrease()
                                        valueModified()
                                    }
                                    anchors.centerIn: parent
                                    // Modern look: no Material paddings and insets, they differ above and below
                                    // and the number sat high in the low field. Bindings that act only then:
                                    // "undefined" in the classic look reset them to the Qt defaults, not to the
                                    // Material ones the classic field had
                                    // with a bigger text the field takes the cell and grows with the row
                                    width: window.textScale !== 1 ? parent.width - 8 : parent.width * 0.8
                                    height: window.ui.modern ? 36 : window.fontPx(30)
                                    Binding on topPadding { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                                    Binding on bottomPadding { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                                    Binding on topInset { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                                    Binding on bottomInset { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                                    // Material pads 8 above and 16 below: the number sat 4 px over the middle
                                    topPadding: window.textScale !== 1 ? 0 : 8
                                    bottomPadding: window.textScale !== 1 ? 0 : 16
                                    from: 1
                                    to: 999
                                    value: crankset
                                    onValueModified: {
                                        gearRows[index].crankset = value
                                        gearConfigurationChanged(gearRows)
                                    }

                                    // Style the SpinBox
                                    contentItem: UiSpinInput {
                                        z: 2
                                        text: cranksetSpinBox.textFromValue(cranksetSpinBox.value, cranksetSpinBox.locale)
                                        // Modern look: the 16 px of the +/- fields (UiSpinBox, the inclination table)
                                        font: window.ui.modern ? Qt.font({ family: cranksetSpinBox.font.family, pixelSize: window.ui.fontBody }) : cranksetSpinBox.font
                                        color: window.ui.ink("black")
                                        selectionColor: window.ui.modern ? window.ui.accent : "#21be2b"
                                        selectedTextColor: window.ui.modern ? window.ui.accentInk : "#ffffff"
                                        horizontalAlignment: Qt.AlignHCenter
                                        verticalAlignment: Qt.AlignVCenter
                                    }

                                    up.indicator: Rectangle {
                                        x: parent.width - width
                                        height: parent.height
                                        // square, but at most a quarter of the field: the number keeps the middle
                                        width: window.textScale !== 1 ? Math.min(height, parent.width / 4) : height
                                        color: window.ui.modern ? (parent.up.pressed ? window.ui.surfaceHigh : window.ui.surfaceHighest) : (parent.up.pressed ? "#e4e4e4" : "#f6f6f6")
                                        border.color: window.ui.modern ? window.ui.alpha(window.ui.outline, 0.4) : "#cccccc"
                                        border.width: window.ui.modern ? 0 : 1
                                        radius: window.ui.modern ? 8 : 0

                                        Text {
                                            visible: !window.ui.modern
                                            text: "+"
                                            color: window.ui.ink("black")
                                            anchors.centerIn: parent
                                            font.pixelSize: window.ui.modern ? 18 : window.fontPx(12)
                                        }
                                        // Modern look: drawn, the glyph sat off the centre of the button
                                        Rectangle {
                                            visible: window.ui.modern
                                            anchors.centerIn: parent
                                            width: 12; height: 2; radius: 1
                                            color: window.ui.textMain
                                        }
                                        Rectangle {
                                            visible: window.ui.modern
                                            anchors.centerIn: parent
                                            width: 2; height: 12; radius: 1
                                            color: window.ui.textMain
                                        }
                                    }

                                    down.indicator: Rectangle {
                                        x: 0
                                        height: parent.height
                                        // square, but at most a quarter of the field: the number keeps the middle
                                        width: window.textScale !== 1 ? Math.min(height, parent.width / 4) : height
                                        color: window.ui.modern ? (parent.down.pressed ? window.ui.surfaceHigh : window.ui.surfaceHighest) : (parent.down.pressed ? "#e4e4e4" : "#f6f6f6")
                                        border.color: window.ui.modern ? window.ui.alpha(window.ui.outline, 0.4) : "#cccccc"
                                        border.width: window.ui.modern ? 0 : 1
                                        radius: window.ui.modern ? 8 : 0

                                        Text {
                                            visible: !window.ui.modern
                                            text: "-"
                                            color: window.ui.ink("black")
                                            anchors.centerIn: parent
                                            font.pixelSize: window.ui.modern ? 18 : window.fontPx(12)
                                        }
                                        // Modern look: drawn, the glyph sat off the centre of the button
                                        Rectangle {
                                            visible: window.ui.modern
                                            anchors.centerIn: parent
                                            width: 12; height: 2; radius: 1
                                            color: window.ui.textMain
                                        }
                                    }

                                    background: Rectangle {
                                        color: window.ui.modern ? window.ui.surfaceHighest : "white"
                                        border.color: window.ui.modern ? window.ui.alpha(window.ui.outline, 0.4) : "#cccccc"
                                        border.width: window.ui.modern ? 0 : 1
                                        radius: window.ui.modern ? 8 : 0
                                    }
                                }
                            }

                            // Rear Cog (editable)
                            Rectangle {
                                width: parent.width * (window.ui.modern ? 0.38 : 1 / 3)
                                height: parent.height
                                border.width: window.ui.modern ? 0 : 1
                                border.color: window.ui.modern ? window.ui.alpha(window.ui.outline, 0.4) : "#cccccc"
                                color: "transparent"

                                SpinBox {
                                    id: cogSpinBox
                                    Accessible.name: qsTr("Gear") + " " + gear + ", " + qsTr("Cog Size") + ": " + value
                                    Accessible.onIncreaseAction: {
                                        increase()
                                        valueModified()
                                    }
                                    Accessible.onDecreaseAction: {
                                        decrease()
                                        valueModified()
                                    }
                                    anchors.centerIn: parent
                                    // Modern look: no Material paddings and insets, they differ above and below
                                    // and the number sat high in the low field. Bindings that act only then:
                                    // "undefined" in the classic look reset them to the Qt defaults, not to the
                                    // Material ones the classic field had
                                    width: window.textScale !== 1 ? parent.width - 8 : parent.width * 0.8
                                    height: window.ui.modern ? 36 : window.fontPx(30)
                                    Binding on topPadding { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                                    Binding on bottomPadding { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                                    Binding on topInset { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                                    Binding on bottomInset { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                                    topPadding: window.textScale !== 1 ? 0 : 8
                                    bottomPadding: window.textScale !== 1 ? 0 : 16
                                    from: 1
                                    to: 999
                                    value: cog
                                    onValueModified: {
                                        gearRows[index].cog = value
                                        gearConfigurationChanged(gearRows)
                                    }

                                    // Style the SpinBox (same as cranksetSpinBox)
                                    contentItem: UiSpinInput {
                                        z: 2
                                        text: cogSpinBox.textFromValue(cogSpinBox.value, cogSpinBox.locale)
                                        font: window.ui.modern ? Qt.font({ family: cogSpinBox.font.family, pixelSize: window.ui.fontBody }) : cogSpinBox.font
                                        color: window.ui.ink("black")
                                        selectionColor: window.ui.modern ? window.ui.accent : "#21be2b"
                                        selectedTextColor: window.ui.modern ? window.ui.accentInk : "#ffffff"
                                        horizontalAlignment: Qt.AlignHCenter
                                        verticalAlignment: Qt.AlignVCenter
                                    }

                                    up.indicator: Rectangle {
                                        x: parent.width - width
                                        height: parent.height
                                        // square, but at most a quarter of the field: the number keeps the middle
                                        width: window.textScale !== 1 ? Math.min(height, parent.width / 4) : height
                                        color: window.ui.modern ? (parent.up.pressed ? window.ui.surfaceHigh : window.ui.surfaceHighest) : (parent.up.pressed ? "#e4e4e4" : "#f6f6f6")
                                        border.color: window.ui.modern ? window.ui.alpha(window.ui.outline, 0.4) : "#cccccc"
                                        border.width: window.ui.modern ? 0 : 1
                                        radius: window.ui.modern ? 8 : 0

                                        Text {
                                            visible: !window.ui.modern
                                            text: "+"
                                            color: window.ui.ink("black")
                                            anchors.centerIn: parent
                                            font.pixelSize: window.ui.modern ? 18 : window.fontPx(12)
                                        }
                                        // Modern look: drawn, the glyph sat off the centre of the button
                                        Rectangle {
                                            visible: window.ui.modern
                                            anchors.centerIn: parent
                                            width: 12; height: 2; radius: 1
                                            color: window.ui.textMain
                                        }
                                        Rectangle {
                                            visible: window.ui.modern
                                            anchors.centerIn: parent
                                            width: 2; height: 12; radius: 1
                                            color: window.ui.textMain
                                        }
                                    }

                                    down.indicator: Rectangle {
                                        x: 0
                                        height: parent.height
                                        // square, but at most a quarter of the field: the number keeps the middle
                                        width: window.textScale !== 1 ? Math.min(height, parent.width / 4) : height
                                        color: window.ui.modern ? (parent.down.pressed ? window.ui.surfaceHigh : window.ui.surfaceHighest) : (parent.down.pressed ? "#e4e4e4" : "#f6f6f6")
                                        border.color: window.ui.modern ? window.ui.alpha(window.ui.outline, 0.4) : "#cccccc"
                                        border.width: window.ui.modern ? 0 : 1
                                        radius: window.ui.modern ? 8 : 0

                                        Text {
                                            visible: !window.ui.modern
                                            text: "-"
                                            color: window.ui.ink("black")
                                            anchors.centerIn: parent
                                            font.pixelSize: window.ui.modern ? 18 : window.fontPx(12)
                                        }
                                        // Modern look: drawn, the glyph sat off the centre of the button
                                        Rectangle {
                                            visible: window.ui.modern
                                            anchors.centerIn: parent
                                            width: 12; height: 2; radius: 1
                                            color: window.ui.textMain
                                        }
                                    }

                                    background: Rectangle {
                                        color: window.ui.modern ? window.ui.surfaceHighest : "white"
                                        border.color: window.ui.modern ? window.ui.alpha(window.ui.outline, 0.4) : "#cccccc"
                                        border.width: window.ui.modern ? 0 : 1
                                        radius: window.ui.modern ? 8 : 0
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
