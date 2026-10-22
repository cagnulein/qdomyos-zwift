import QtQuick 2.7
import QtQml 2.15
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

    // Properties
    Settings {
        id: settings
        property string gear_configuration: "1|38|44|true\n2|38|38|true\n3|38|32|true\n4|38|28|true\n5|38|24|true\n6|38|21|true\n7|38|19|true\n8|38|17|true\n9|38|15|true\n10|38|13|true\n11|38|11|true\n12|38|10|true"
        property int gear_crankset_size: 42
        property int gear_cog_size: 14
        property string gear_wheel_size: "700 x 18C"
        property real gear_circumference: 2070
        property bool wahoo_without_wheel_diameter: false
    }

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

    onSettingsChanged: {
        console.log("onSettingsChanged")
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
    }

    // Signals to notify when values change
    signal settingsChanged()
    signal gearConfigurationChanged(var gearRows)
    signal restoreDefaultWheelDiameter()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20
        id: chainringColumn

        // Wahoo Options
        GroupBox {
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
                    font.pixelSize: window.fontHint
                    bottomPadding: window.hintGap
                    color: Material.accent
                }
            }
        }

        // Crankset Size
        GroupBox {
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
                }

                SpinBox {
                    Accessible.name: qsTr("Chainring Size") + ": " + value
                    from: 1
                    to: 999
                    value: selectedCranksetSize
                    onValueChanged: {
                        selectedCranksetSize = value
                        console.log("Crankset Size changed");
                        settingsChanged()
                    }
                }
            }
        }

        // Cog Size
        GroupBox {
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
                }
                SpinBox {
                    Accessible.name: qsTr("Cog Size") + ": " + value
                    from: 1
                    to: 999
                    value: selectedCogSize
                    onValueChanged: {
                        selectedCogSize = value
                        console.log("Cog Size changed");
                        settingsChanged()
                    }
                }
            }
        }

        // Wheel Size
        GroupBox {
            title: qsTr("Virtual Wheel Size")
            Accessible.focusable: true
            Layout.fillWidth: true

            ColumnLayout {
                // With a bigger text the column took the width of the button label in one line and
                // the button ran past the screen edge: it takes the width of the box instead
                width: window.textScale !== 1 ? parent.width : implicitWidth
                UiComboBox {
                       id: wheelSizeCombo
                       Accessible.name: qsTr("Virtual Wheel Size") + ": " + displayText
                       width: parent.width
                       // the box was as wide as at 100 % and cut the wheel size ("700 x 1…")
                       Layout.fillWidth: window.textScale !== 1
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
                          settingsChanged()
                      }
                  }
                }

                // Restore Default Wheel Diameter Button
                Button {
                    text: qsTr("Restore Default Setting to the Trainer")
                    Layout.fillWidth: true
                    // with a bigger text the label wraps in two lines
                    Layout.preferredHeight: window.textScale !== 1 ? Math.max(50, contentItem.implicitHeight + 16) : 50
                    font.bold: true
                    background: Rectangle {
                        color: "#9C27B0"
                        radius: 4
                    }
                    contentItem: Text {
                        wrapMode: window.labelWrap
                        text: parent.text
                        font: parent.font
                        color: "white"
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

        GroupBox {
        title: qsTr("Preset Gear Profiles")
        Accessible.focusable: true
        Layout.fillWidth: true

            UiComboBox {
                id: profileCombo
                Accessible.name: qsTr("Preset Gear Profiles") + ": " + displayText
                width: parent.width
                textRole: "text"
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

                onCurrentIndexChanged: {
                 if (currentIndex >= 0) {
                     loadGearProfile(profileModel.get(currentIndex).value)
                 }
                }
                }
        }


        // Gear Table GroupBox
        GroupBox {
            title: qsTr("Virtual Gear Table")
            Accessible.focusable: true
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: parent.height

            ColumnLayout {
                anchors.fill: parent
                spacing: 10

                // Updated Buttons Row
                // With a bigger text three buttons in a row cut their labels ("Add…"): one under
                // another, each the full width
                GridLayout {
                    Layout.fillWidth: true
                    Layout.preferredHeight: window.textScale > 1 ? -1 : 40
                    columns: window.textScale > 1 ? 1 : 3
                    columnSpacing: 10
                    rowSpacing: 10

                    Button {
                        text: qsTr("Add Gear")
                        Layout.fillWidth: true
                        Layout.preferredHeight: window.fontPx(40)
                        onClicked: addNewGear()
                    }

                    Button {
                        text: qsTr("Clear Selected Gear and Following")
                        Layout.fillWidth: true
                        Layout.preferredHeight: window.fontPx(40)
                        onClicked: {
                            if (gearTable.currentRow >= 0) {
                                clearGearsFromIndex(gearTable.currentRow)
                            }
                        }
                    }

                    Button {
                        text: qsTr("Reset All Gears")
                        Layout.fillWidth: true
                        Layout.preferredHeight: window.fontPx(40)
                        onClicked: initializeGearRows()
                    }
                }

                // Table Header (same as before)
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: window.fontPx(40)
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
                                // a plain Text does not inherit the window font: the text size while it is not 100 %
                                Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                                anchors.centerIn: parent
                                text: qsTr("Gear")
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
                                Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                                anchors.centerIn: parent
                                text: qsTr("Chainring")
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
                                Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                                anchors.centerIn: parent
                                text: qsTr("Rear Cog")
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
                        height: window.fontPx(40)
                        color: gearTable.currentRow === index ? "#e0e0e0" : "white"

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
                                width: parent.width / 3
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
                                    border.width: 1
                                    border.color: "#cccccc"
                                    color: "transparent"
                                }

                                contentItem: Text {
                                    // a plain Text does not inherit the window font: the text size while it is not 100 %
                                    Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                                    text: gear
                                    color: "black"
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    Accessible.ignored: true
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
                                    // with a bigger text the field takes the cell and grows with the row
                                    width: window.textScale !== 1 ? parent.width - 8 : parent.width * 0.8
                                    height: window.fontPx(30)
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
                                        // square, but at most a quarter of the field: the number keeps the middle
                                        width: window.textScale !== 1 ? Math.min(height, parent.width / 4) : height
                                        color: parent.up.pressed ? "#e4e4e4" : "#f6f6f6"
                                        border.color: "#cccccc"

                                        Text {
                                            text: "+"
                                            color: "black"
                                            anchors.centerIn: parent
                                            font.pixelSize: window.fontPx(12)
                                        }
                                    }

                                    down.indicator: Rectangle {
                                        x: 0
                                        height: parent.height
                                        // square, but at most a quarter of the field: the number keeps the middle
                                        width: window.textScale !== 1 ? Math.min(height, parent.width / 4) : height
                                        color: parent.down.pressed ? "#e4e4e4" : "#f6f6f6"
                                        border.color: "#cccccc"

                                        Text {
                                            text: "-"
                                            color: "black"
                                            anchors.centerIn: parent
                                            font.pixelSize: window.fontPx(12)
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
                                    width: window.textScale !== 1 ? parent.width - 8 : parent.width * 0.8
                                    height: window.fontPx(30)
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
                                        // square, but at most a quarter of the field: the number keeps the middle
                                        width: window.textScale !== 1 ? Math.min(height, parent.width / 4) : height
                                        color: parent.up.pressed ? "#e4e4e4" : "#f6f6f6"
                                        border.color: "#cccccc"

                                        Text {
                                            text: "+"
                                            color: "black"
                                            anchors.centerIn: parent
                                            font.pixelSize: window.fontPx(12)
                                        }
                                    }

                                    down.indicator: Rectangle {
                                        x: 0
                                        height: parent.height
                                        // square, but at most a quarter of the field: the number keeps the middle
                                        width: window.textScale !== 1 ? Math.min(height, parent.width / 4) : height
                                        color: parent.down.pressed ? "#e4e4e4" : "#f6f6f6"
                                        border.color: "#cccccc"

                                        Text {
                                            text: "-"
                                            color: "black"
                                            anchors.centerIn: parent
                                            font.pixelSize: window.fontPx(12)
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
