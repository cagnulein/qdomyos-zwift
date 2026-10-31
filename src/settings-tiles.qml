import QtQuick 2.7
import QtQml 2.15
import QtQuick.Dialogs 1.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0

ScrollView {
    contentWidth: -1
    // The page height from the column itself, as on the settings page: after a turn of the screen
    // ScrollView stopped following the height of its only child on a phone
    contentHeight: tilesColumn.implicitHeight
    leftPadding: window.contentSideMargin
    rightPadding: window.contentSideMargin
    focus: true
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.fill: parent
    //anchors.bottom: footerSettings.top
    //anchors.bottomMargin: footerSettings.height + 10
    id: settingsTilesPane

    readonly property real speedPresetMilesConversion: 0.621371

    function formatPresetSpeedValue(speedKmh) {
        var visibleValue = settings.miles_unit ? speedKmh * speedPresetMilesConversion : speedKmh
        return Number(visibleValue).toFixed(1).replace(/\.0$/, "")
    }

    function parsePresetSpeedValue(speedText) {
        var parsedValue = parseFloat(speedText)
        if (isNaN(parsedValue))
            return 0
        return settings.miles_unit ? parsedValue / speedPresetMilesConversion : parsedValue
    }

    // The Settings object of settings.qml, given by the page that opens this one: one object
    // for all the settings pages, so a page no longer writes back an old copy of what another
    // page changed
    property var settings


    ColumnLayout {
        id: tilesColumn
        spacing: 0
        // Not tied to the bottom: the height of the column is the content height of the page, so
        // fill made an anchor loop; on a turn of the screen Qt gave up on it and the height stayed
        // as it was - a section opened after it could not be scrolled to its end
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        NewPageElement {
            id: labelShortcutsSettings
            title: qsTr("Keyboard Shortcuts ⌨️")
            indicatRectColor: Material.color(Material.Grey)
            textColor: Material.color(Material.Grey)
            color: Material.backgroundColor
            accordionContent: "settings-shortcuts.qml"
            pageSettings: settingsTilesPane.settings
        }

        AccordionCheckElement {
            id: speedEnabledAccordion
            title: qsTr("Speed")
            linkedBoolSetting: "tile_speed_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelSpeedOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: speedOrderTextField
                    Accessible.name: speedEnabledAccordion.title + ", " + labelSpeedOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_speed_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = speedOrderTextField.currentValue
                     }
                }
                Button {
                    id: okSpeedOrderButton
                    text: qsTr("OK")
                    enabled: speedOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_speed_order = speedOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Speed in kilometers per hour. (To set your speed units to miles, go to Settings > General Options > Use Miles unit in UI).")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: inclinationEnabledAccordion
            title: qsTr("Inclination")
            linkedBoolSetting: "tile_inclination_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelinclinationOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: inclinationOrderTextField
                    Accessible.name: inclinationEnabledAccordion.title + ", " + labelinclinationOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_inclination_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = inclinationOrderTextField.currentValue
                     }
                }
                Button {
                    id: okinclinationOrderButton
                    text: qsTr("OK")
                    enabled: inclinationOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_inclination_order = inclinationOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        AccordionCheckElement {
            id: cadenceEnabledAccordion
            title: qsTr("Cadence")
            linkedBoolSetting: "tile_cadence_enabled"
            settings: settingsTilesPane.settings
            accordionContent:  ColumnLayout {
                SwitchDelegate {
                    // on iOS the title of a switch kept the size of the style while the text around it grew: with a size other than 100 % it takes the one of the window font
                    Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.scaledFontPx; restoreMode: Binding.RestoreBindingOrValue }
                    id: cadenceColorEnabled
                    text: qsTr("Enable Cadence color")
                    spacing: 0
                    bottomPadding: 0
                    topPadding: 0
                    rightPadding: 0
                    leftPadding: 0
                    clip: false
                    checked: settings.tile_cadence_color_enabled
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    Layout.fillWidth: true
                    onClicked: settings.tile_cadence_color_enabled = checked
                }

                Label {
                    text: qsTr("Bike pedal cadence changes color to indicate how your cadence compares to the cadence called out in Peloton classes. The tile displays the following colors: white if there is no target cadence in the program, red if your cadence is lower than the target, green if your cadence matches the target, and orange if your cadence is higher than the target.")
                    font.bold: true
                    font.italic: true
                    font.pixelSize: window.fontHint
                    bottomPadding: window.hintGap
                    textFormat: Text.PlainText
                    wrapMode: Text.WordWrap
                    verticalAlignment: Text.AlignVCenter
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    Layout.fillWidth: true
                    color: Material.color(Material.Lime)
                }

                RowLayout {
                    spacing: 10
                    Label {
                        id: labelcadenceOrder
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: cadenceOrderTextField
                        Accessible.name: cadenceEnabledAccordion.title + ", " + labelcadenceOrder.text
                        model: rootItem.tile_order
                        savedText: settings.tile_cadence_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = cadenceOrderTextField.currentValue
                         }
                    }
                    Button {
                        id: okcadenceOrderButton
                        text: qsTr("OK")
                        enabled: cadenceOrderTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_cadence_order = cadenceOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
            }
        }

        Label {
            text: qsTr("Bike pedal cadence in rotations per minute (RPM) or Treadmill cadence if a shoe-mounted cadence sensor or Apple Watch QZ app is used.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: elevationEnabledAccordion
            title: qsTr("Elevation Gain")
            linkedBoolSetting: "tile_elevation_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelelevationOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: elevationOrderTextField
                    Accessible.name: elevationEnabledAccordion.title + ", " + labelelevationOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_elevation_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = elevationOrderTextField.currentValue
                     }
                }
                Button {
                    id: okelevationOrderButton
                    text: qsTr("OK")
                    enabled: elevationOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_elevation_order = elevationOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }            

        AccordionCheckElement {
            id: negativeInclinationEnabledAccordion
            title: qsTr("Negative Elevation Gain (Descent)")
            linkedBoolSetting: "tile_negative_inclination_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: negativeInclinationOrderTextField
                    Accessible.name: negativeInclinationEnabledAccordion.title + ", " + qsTr("order index:")
                    model: rootItem.tile_order
                    savedText: settings.tile_negative_inclination_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = negativeInclinationOrderTextField.currentValue
                     }
                }
                Button {
                    text: qsTr("OK")
                    enabled: negativeInclinationOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_negative_inclination_order = negativeInclinationOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Displays the total negative elevation gain (descent) in meters or feet accumulated during the workout.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: caloriesEnabledAccordion
            title: qsTr("Calories")
            linkedBoolSetting: "tile_calories_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelcaloriesOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: caloriesOrderTextField
                    Accessible.name: caloriesEnabledAccordion.title + ", " + labelcaloriesOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_calories_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = caloriesOrderTextField.currentValue
                     }
                }
                Button {
                    id: okcaloriesOrderButton
                    text: qsTr("OK")
                    enabled: caloriesOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_calories_order = caloriesOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Estimated calories burned during session, calculated on weight, age, and watts.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: odometerEnabledAccordion
            title: qsTr("Odometer")
            linkedBoolSetting: "tile_odometer_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelodometerOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: odometerOrderTextField
                    Accessible.name: odometerEnabledAccordion.title + ", " + labelodometerOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_odometer_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = odometerOrderTextField.currentValue
                     }
                }
                Button {
                    id: okodometerOrderButton
                    text: qsTr("OK")
                    enabled: odometerOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_odometer_order = odometerOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Estimated distance traveled during the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: paceEnabledAccordion
            title: qsTr("Pace")
            linkedBoolSetting: "tile_pace_enabled"
            settings: settingsTilesPane.settings
            accordionContent: ColumnLayout {
                SwitchDelegate {
                    Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.scaledFontPx; restoreMode: Binding.RestoreBindingOrValue }
                    id: paceColorEnabled
                    text: qsTr("Enable Pace color")
                    spacing: 0
                    bottomPadding: 0
                    topPadding: 0
                    rightPadding: 0
                    leftPadding: 0
                    clip: false
                    checked: settings.tile_pace_color_enabled
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    Layout.fillWidth: true
                    onClicked: settings.tile_pace_color_enabled = checked
                }

                RowLayout {
                    spacing: 10
                    Label {
                        id: labelpaceOrder
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: paceOrderTextField
                        Accessible.name: paceEnabledAccordion.title + ", " + labelpaceOrder.text
                        model: rootItem.tile_order
                        savedText: settings.tile_pace_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = paceOrderTextField.currentValue
                         }
                    }
                    Button {
                        id: okpaceOrderButton
                        text: qsTr("OK")
                        enabled: paceOrderTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_pace_order = paceOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
            }
        }

        AccordionCheckElement {
            id: avgPaceEnabledAccordion
            title: qsTr("Average Pace")
            linkedBoolSetting: "tile_avg_pace_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelavgpaceOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: avgpaceOrderTextField
                    Accessible.name: avgPaceEnabledAccordion.title + ", " + labelavgpaceOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_avg_pace_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = avgpaceOrderTextField.currentValue
                     }
                }
                Button {
                    id: okavgpaceOrderButton
                    text: qsTr("OK")
                    enabled: avgpaceOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_avg_pace_order = avgpaceOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        AccordionCheckElement {
            id: gradeAdjustedPaceEnabledAccordion
            title: qsTr("Grade Adjusted Pace")
            linkedBoolSetting: "tile_grade_adjusted_pace_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelgradeAdjustedPaceOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: gradeAdjustedPaceOrderTextField
                    Accessible.name: gradeAdjustedPaceEnabledAccordion.title + ", " + labelgradeAdjustedPaceOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_grade_adjusted_pace_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = gradeAdjustedPaceOrderTextField.currentValue
                     }
                }
                Button {
                    id: okgradeAdjustedPaceOrderButton
                    text: "OK"
                    enabled: gradeAdjustedPaceOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_grade_adjusted_pace_order = gradeAdjustedPaceOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Current pace per mile or kilometer (Treadmill, Elliptical and Rower)")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        Label {
            text: qsTr("Flat-equivalent pace computed from treadmill incline using the Minetti cost model.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: resistanceEnabledAccordion
            title: qsTr("Resistance")
            linkedBoolSetting: "tile_resistance_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelresistanceOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: resistanceOrderTextField
                    Accessible.name: resistanceEnabledAccordion.title + ", " + labelresistanceOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_resistance_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = resistanceOrderTextField.currentValue
                     }
                }
                Button {
                    id: okresistanceOrderButton
                    text: qsTr("OK")
                    enabled: resistanceOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_resistance_order = resistanceOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Displays your bike’s resistance. The +/- buttons can be used to change resistance, if your bike is compatible.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: wattEnabledAccordion
            title: qsTr("Watt")
            linkedBoolSetting: "tile_watt_enabled"
            settings: settingsTilesPane.settings
            accordionContent:  ColumnLayout {
                SwitchDelegate {
                    Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.scaledFontPx; restoreMode: Binding.RestoreBindingOrValue }
                    id: wattColorEnabled
                    text: qsTr("Enable Watt color")
                    spacing: 0
                    bottomPadding: 0
                    topPadding: 0
                    rightPadding: 0
                    leftPadding: 0
                    clip: false
                    checked: settings.tile_watt_color_enabled
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    Layout.fillWidth: true
                    onClicked: settings.tile_watt_color_enabled = checked
                }

                RowLayout {
                    spacing: 10
                    Label {
                        id: labelwattOrder
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: wattOrderTextField
                        Accessible.name: wattEnabledAccordion.title + ", " + labelwattOrder.text
                        model: rootItem.tile_order
                        savedText: settings.tile_watt_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = wattOrderTextField.currentValue
                         }
                    }
                    Button {
                        id: okwattOrderButton
                        text: qsTr("OK")
                        enabled: wattOrderTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_watt_order = wattOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
            }
        }

        Label {
            text: qsTr("Displays the watts generated by your current effort. Watt is also referred to as output (for example, in Peloton). If your equipment does not communicate watts, QZ will calculate watts using resistance and cadence.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: weightLossEnabledAccordion
            title: qsTr("Weight loss")
            linkedBoolSetting: "tile_weight_loss_enabled"
            settings: settingsTilesPane.settings
            accordionContent:  RowLayout {
                spacing: 10
                Label {
                    id: labelweightLossOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: weightLossOrderTextField
                    Accessible.name: weightLossEnabledAccordion.title + ", " + labelweightLossOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_weight_loss_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = weightLossOrderTextField.currentValue
                     }
                }
                Button {
                    id: okweightLossOrderButton
                    text: qsTr("OK")
                    enabled: weightLossOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_weight_loss_order = weightLossOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Estimation of weight loss during the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }


        AccordionCheckElement {
            id: avgwattEnabledAccordion
            title: qsTr("AVG Watt")
            linkedBoolSetting: "tile_avgwatt_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelavgwattOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: avgwattOrderTextField
                    Accessible.name: avgwattEnabledAccordion.title + ", " + labelavgwattOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_avgwatt_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = avgwattOrderTextField.currentValue
                     }
                }
                Button {
                    id: okavgwattOrderButton
                    text: qsTr("OK")
                    enabled: avgwattOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_avgwatt_order = avgwattOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Average watts produced for the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }


        AccordionCheckElement {
		      id: avgwattLapEnabledAccordion
				title: qsTr("AVG Watt Lap")
				linkedBoolSetting: "tile_avg_watt_lap_enabled"
				settings: settingsTilesPane.settings
				accordionContent: RowLayout {
				    spacing: 10
					 Label {
					     id: labelavgwattLapOrder
						  text: qsTr("order index:")
        wrapMode: window.labelWrap
						  Layout.fillWidth: true
						  horizontalAlignment: Text.AlignRight
						}
					 UiComboBox {
					     id: avgwattLapOrderTextField
					     Accessible.name: avgwattLapEnabledAccordion.title + ", " + labelavgwattLapOrder.text
						  model: rootItem.tile_order
						  savedText: settings.tile_avg_watt_lap_order
						  Layout.fillHeight: false
						  Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
						  onActivated: {
						      displayText = avgwattLapOrderTextField.currentValue
								}
							}
					 Button {
					     id: okavgwattLapOrderButton
						  text: qsTr("OK")
						  enabled: avgwattLapOrderTextField.dirty
						  Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
						  onClicked: {settings.tile_avg_watt_lap_order = avgwattLapOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
						}
					}
				}

        AccordionCheckElement {
            id: ftpEnabledAccordion
            title: "FTP %"
            linkedBoolSetting: "tile_ftp_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelftpOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: ftpOrderTextField
                    Accessible.name: ftpEnabledAccordion.title + ", " + labelftpOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_ftp_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = ftpOrderTextField.currentValue
                     }
                }
                Button {
                    id: okftpOrderButton
                    text: qsTr("OK")
                    enabled: ftpOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_ftp_order = ftpOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Percentage of current FTP and current FTP zone.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: heartEnabledAccordion
            title: qsTr("Heart")
            linkedBoolSetting: "tile_heart_enabled"
            settings: settingsTilesPane.settings
            accordionContent: ColumnLayout {
                SwitchDelegate {
                    Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.scaledFontPx; restoreMode: Binding.RestoreBindingOrValue }
                    id: heartShowAsPercentSwitch
                    text: qsTr("Show as %FC Max")
                    spacing: 0
                    bottomPadding: 0
                    topPadding: 0
                    rightPadding: 0
                    leftPadding: 0
                    clip: false
                    checked: settings.tile_heart_show_as_percent
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    Layout.fillWidth: true
                    onClicked: settings.tile_heart_show_as_percent = checked
                }

                Label {
                    text: qsTr("When enabled, displays heart rate as percentage of maximum heart rate (%FC Max) instead of BPM. AVG and MAX values will also show percentages.")
                    font.bold: true
                    font.italic: true
                    font.pixelSize: window.fontHint
                    bottomPadding: window.hintGap
                    textFormat: Text.PlainText
                    wrapMode: Text.WordWrap
                    verticalAlignment: Text.AlignVCenter
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    Layout.fillWidth: true
                    color: Material.color(Material.Lime)
                }

                RowLayout {
                    spacing: 10
                    Label {
                        id: labelheartrateOrder
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: heartrateOrderTextField
                        Accessible.name: heartEnabledAccordion.title + ", " + labelheartrateOrder.text
                        model: rootItem.tile_order
                        savedText: settings.tile_heart_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = heartrateOrderTextField.currentValue
                         }
                    }
                    Button {
                        id: okheartrateOrderButton
                        text: qsTr("OK")
                        enabled: heartrateOrderTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_heart_order = heartrateOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
            }
        }

        AccordionCheckElement {
            id: fanEnabledAccordion
            title: qsTr("Fan")
            linkedBoolSetting: "tile_fan_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelfanOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: fanOrderTextField
                    Accessible.name: fanEnabledAccordion.title + ", " + labelfanOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_fan_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = fanOrderTextField.currentValue
                     }
                }
                Button {
                    id: okfanOrderButton
                    text: qsTr("OK")
                    enabled: fanOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_fan_order = fanOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Built-in treadmill fan speed (Treadmill only)")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: joulsEnabledAccordion
            title: qsTr("Jouls")
            linkedBoolSetting: "tile_jouls_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labeljoulsOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: joulsOrderTextField
                    Accessible.name: joulsEnabledAccordion.title + ", " + labeljoulsOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_jouls_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = joulsOrderTextField.currentValue
                     }
                }
                Button {
                    id: okjoulsOrderButton
                    text: qsTr("OK")
                    enabled: joulsOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_jouls_order = joulsOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Cumulative power produced during the session in kilojoules.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: elapsedEnabledAccordion
            title: qsTr("Elapsed")
            linkedBoolSetting: "tile_elapsed_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelelapsedOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: elapsedOrderTextField
                    Accessible.name: elapsedEnabledAccordion.title + ", " + labelelapsedOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_elapsed_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = elapsedOrderTextField.currentValue
                     }
                }
                Button {
                    id: okelapsedOrderButton
                    text: qsTr("OK")
                    enabled: elapsedOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_elapsed_order = elapsedOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Total time from start of the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: movingTimeEnabledAccordion
            title: qsTr("Moving Time")
            linkedBoolSetting: "tile_moving_time_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelmovingTimeOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: movingTimeOrderTextField
                    Accessible.name: movingTimeEnabledAccordion.title + ", " + labelmovingTimeOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_moving_time_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = movingTimeOrderTextField.currentValue
                     }
                }
                Button {
                    id: okmovingTimeOrderButton
                    text: qsTr("OK")
                    enabled: movingTimeOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_moving_time_order = movingTimeOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Total time moving during the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: pelotonOffsetEnabledAccordion
            title: qsTr("Peloton Offset")
            linkedBoolSetting: "tile_peloton_offset_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelpelotonOffsetOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: pelotonOffsetOrderTextField
                    Accessible.name: pelotonOffsetEnabledAccordion.title + ", " + labelpelotonOffsetOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_peloton_offset_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = pelotonOffsetOrderTextField.currentValue
                     }
                }
                Button {
                    id: okpelotonOffsetOrderButton
                    text: qsTr("OK")
                    enabled: pelotonOffsetOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_peloton_offset_order = pelotonOffsetOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Allows you to sync resistance and cadence target changes with the Peloton coach’s callouts. If the targets are changing in QZ after the coach’s callouts, use the ‘+’ button to add seconds (essentially speeding QZ up). Use the ‘-’ button to slow QZ down. Use this tile in conjunction with the Remaining Time/Row tile (see below).")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: pelotonRemainingEnabledAccordion
            title: qsTr("Peloton Remaining")
            linkedBoolSetting: "tile_peloton_remaining_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelPelotonRemainingOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: pelotonRemainingOrderTextField
                    Accessible.name: pelotonRemainingEnabledAccordion.title + ", " + labelPelotonRemainingOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_peloton_remaining_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = pelotonRemainingOrderTextField.currentValue
                     }
                }
                Button {
                    id: okPelotonRemainingOrderButton
                    text: qsTr("OK")
                    enabled: pelotonRemainingOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_peloton_remaining_order = pelotonRemainingOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Displays time remaining in Peloton class.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        /*
        AccordionCheckElement {
            id: pelotonDifficultyEnabledAccordion
            title: qsTr("Peloton Difficulty")
            linkedBoolSetting: "tile_peloton_difficulty_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelpelotonDifficultyOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: pelotonDifficultyOrderTextField
                    model: rootItem.tile_order
                    savedText: settings.tile_peloton_difficulty_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = pelotonDifficultyOrderTextField.currentValue
                     }
                }
                Button {
                    id: okpelotonDifficultyOrderButton
                    text: qsTr("OK")
                    enabled: pelotonDifficultyOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_peloton_difficulty_order = pelotonDifficultyOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }*/

        AccordionCheckElement {
            id: lapElapsedEnabledAccordion
            title: qsTr("Lap Elapsed")
            linkedBoolSetting: "tile_lapelapsed_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labellapElapsedOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: lapElapsedOrderTextField
                    Accessible.name: lapElapsedEnabledAccordion.title + ", " + labellapElapsedOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_lapelapsed_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = lapElapsedOrderTextField.currentValue
                     }
                }
                Button {
                    id: oklapElapsedOrderButton
                    text: qsTr("OK")
                    enabled: lapElapsedOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_lapelapsed_order = lapElapsedOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        AccordionCheckElement {
            id: pelotonResistanceEnabledAccordion
            title: qsTr("Peloton Resistance")
            linkedBoolSetting: "tile_peloton_resistance_enabled"
            settings: settingsTilesPane.settings
            accordionContent: ColumnLayout {
                SwitchDelegate {
                    Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.scaledFontPx; restoreMode: Binding.RestoreBindingOrValue }
                    id: pelotonResistanceColorEnabled
                    text: qsTr("Enable Peloton Resistance color")
                    spacing: 0
                    bottomPadding: 0
                    topPadding: 0
                    rightPadding: 0
                    leftPadding: 0
                    clip: false
                    checked: settings.tile_peloton_resistance_color_enabled
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    Layout.fillWidth: true
                    onClicked: settings.tile_peloton_resistance_color_enabled = checked
                }
                RowLayout {
                    spacing: 10
                    Label {
                        id: labelpeloton_resistanceOrder
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: peloton_resistanceOrderTextField
                        Accessible.name: pelotonResistanceEnabledAccordion.title + ", " + labelpeloton_resistanceOrder.text
                        model: rootItem.tile_order
                        savedText: settings.tile_peloton_resistance_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = peloton_resistanceOrderTextField.currentValue
                         }
                    }
                    Button {
                        id: okpeloton_resistanceOrderButton
                        text: qsTr("OK")
                        enabled: peloton_resistanceOrderTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_peloton_resistance_order = peloton_resistanceOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
            }
        }

        Label {
            text: qsTr("Resistance of your bike converted to the Peloton bike scale of 1 to 100.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: targetResistanceEnabledAccordion
            title: qsTr("Target Resistance")
            linkedBoolSetting: "tile_target_resistance_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labeltarget_resistanceOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: target_resistanceOrderTextField
                    Accessible.name: targetResistanceEnabledAccordion.title + ", " + labeltarget_resistanceOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_target_resistance_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = target_resistanceOrderTextField.currentValue
                     }
                }
                Button {
                    id: oktarget_resistanceOrderButton
                    text: qsTr("OK")
                    enabled: target_resistanceOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_target_resistance_order = target_resistanceOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Displays target resistance in your bike’s resistance scale. For example, during a Peloton class or Zwift session, you want the resistance displayed in this tile to match the Resistance Tile. During a Peloton class (bike only), +/- shifts the class's resistance target up or down for the rest of the ride.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }


        AccordionCheckElement {
            id: targetPelotonResistanceEnabledAccordion
            title: qsTr("Target Peloton Resistance")
            linkedBoolSetting: "tile_target_peloton_resistance_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labeltarget_peloton_resistanceOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: target_peloton_resistanceOrderTextField
                    Accessible.name: targetPelotonResistanceEnabledAccordion.title + ", " + labeltarget_peloton_resistanceOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_target_peloton_resistance_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = target_peloton_resistanceOrderTextField.currentValue
                     }
                }
                Button {
                    id: oktarget_peloton_resistanceOrderButton
                    text: qsTr("OK")
                    enabled: target_peloton_resistanceOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_target_peloton_resistance_order = target_peloton_resistanceOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Displays target resistance converted to the Peloton bike scale of 1 to 100. For example, during a Peloton class, you want the resistance displayed in this tile to match the Peloton Resistance Tile.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: targetCadenceEnabledAccordion
            title: qsTr("Target Cadence")
            linkedBoolSetting: "tile_target_cadence_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labeltarget_cadenceOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: target_cadenceOrderTextField
                    Accessible.name: targetCadenceEnabledAccordion.title + ", " + labeltarget_cadenceOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_target_cadence_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = target_cadenceOrderTextField.currentValue
                     }
                }
                Button {
                    id: oktarget_cadenceOrderButton
                    text: qsTr("OK")
                    enabled: target_cadenceOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_target_cadence_order = target_cadenceOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Displays target cadence.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: targetPowerEnabledAccordion
            title: qsTr("Target Power")
            linkedBoolSetting: "tile_target_power_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labeltarget_powerOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: target_powerOrderTextField
                    Accessible.name: targetPowerEnabledAccordion.title + ", " + labeltarget_powerOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_target_power_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = target_powerOrderTextField.currentValue
                     }
                }
                Button {
                    id: oktarget_powerOrderButton
                    text: qsTr("OK")
                    enabled: target_powerOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_target_power_order = target_powerOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Displays target output (watts) when this information is provided by third-party apps.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }


        AccordionCheckElement {
            id: targetZoneEnabledAccordion
            title: qsTr("Target Power Zone")
            linkedBoolSetting: "tile_target_zone_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labeltarget_zoneOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: target_zoneOrderTextField
                    Accessible.name: targetZoneEnabledAccordion.title + ", " + labeltarget_zoneOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_target_zone_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = target_zoneOrderTextField.currentValue
                     }
                }
                Button {
                    id: oktarget_zoneOrderButton
                    text: qsTr("OK")
                    enabled: target_zoneOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_target_zone_order = target_zoneOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Displays the target power zone when this information is provided by third-party apps.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: targetSpeedEnabledAccordion
            title: qsTr("Target Speed")
            linkedBoolSetting: "tile_target_speed_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labeltargetspeedOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: target_speedOrderTextField
                    Accessible.name: targetSpeedEnabledAccordion.title + ", " + labeltargetspeedOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_target_speed_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = target_speedOrderTextField.currentValue
                     }
                }
                Button {
                    id: oktarget_speedOrderButton
                    text: qsTr("OK")
                    enabled: target_speedOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_target_speed_order = target_speedOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        AccordionCheckElement {
            id: requiredSpeedEnabledAccordion
            title: qsTr("Required Speed (Target Average)")
            linkedBoolSetting: "tile_required_speed_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: requiredSpeedOrderLabel
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: requiredSpeedOrderField
                    Accessible.name: requiredSpeedEnabledAccordion.title + ", " + requiredSpeedOrderLabel.text
                    model: rootItem.tile_order
                    savedText: settings.value("tile_required_speed_order", 34)
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: { displayText = requiredSpeedOrderField.currentValue }
                }
                Button {
                    text: qsTr("OK")
                    enabled: requiredSpeedOrderField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: { settings.setValue("tile_required_speed_order", requiredSpeedOrderField.displayText); requiredSpeedOrderField.savedText = requiredSpeedOrderField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Treadmill only. Displays the speed needed to reach a target average by a chosen duration. Tap the tile to change either goal; it never controls the treadmill.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: targetPaceEnabledAccordion
            title: qsTr("Target Pace")
            linkedBoolSetting: "tile_target_pace_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labeltargetpaceOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: target_paceOrderTextField
                    Accessible.name: targetPaceEnabledAccordion.title + ", " + labeltargetpaceOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_target_pace_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = target_paceOrderTextField.currentValue
                     }
                }
                Button {
                    id: oktarget_paceOrderButton
                    text: qsTr("OK")
                    enabled: target_paceOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_target_pace_order = target_paceOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        AccordionCheckElement {
            id: targetInclineEnabledAccordion
            title: qsTr("Target Incline")
            linkedBoolSetting: "tile_target_incline_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labeltarget_inclineOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: target_inclineOrderTextField
                    Accessible.name: targetInclineEnabledAccordion.title + ", " + labeltarget_inclineOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_target_incline_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = target_inclineOrderTextField.currentValue
                     }
                }
                Button {
                    id: oktarget_inclineOrderButton
                    text: qsTr("OK")
                    enabled: target_inclineOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_target_incline_order = target_inclineOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }
        AccordionCheckElement {
            id: wattKgEnabledAccordion
            title: qsTr("Watt/Kg")
            linkedBoolSetting: "tile_watt_kg_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelwatt_kgOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: watt_kgOrderTextField
                    Accessible.name: wattKgEnabledAccordion.title + ", " + labelwatt_kgOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_watt_kg_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = watt_kgOrderTextField.currentValue
                     }
                }
                Button {
                    id: okwatt_kgOrderButton
                    text: qsTr("OK")
                    enabled: watt_kgOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_watt_kg_order = watt_kgOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Calculates your output (watts) divided by your weight. This is the primary metric used by Zwift and similar apps to calculate your virtual speed. NOTE: This is a much better metric to use than Output/Watts when comparing your effort to other users. This is why Peloton’s leaderboard, which uses only Output, is flawed.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: gearsEnabledAccordion
            title: qsTr("Gears")
            linkedBoolSetting: "tile_gears_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelgearsOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: gearsOrderTextField
                    Accessible.name: gearsEnabledAccordion.title + ", " + labelgearsOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_gears_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = gearsOrderTextField.currentValue
                     }
                }
                Button {
                    id: okgearsOrderButton
                    text: qsTr("OK")
                    enabled: gearsOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_gears_order = gearsOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Allows you to change resistance while in Auto-Follow Mode.This tile allows you override the target resistance sent by third-party apps. For example, you would use the Gears Tile to increase resistance and generate more watts for sprinting in Zwift.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            title: qsTr("Gears Big Buttons")
            linkedBoolSetting: "tile_biggears_enabled"
            settings: settingsTilesPane.settings
            accordionContent: ColumnLayout {
                RowLayout {
                    spacing: 10
                    Label {
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: biggearsOrderTextField
                        Accessible.name: qsTr("Gears Big Buttons") + ", " + qsTr("order index:")
                        model: rootItem.tile_order
                        savedText: settings.tile_biggears_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = biggearsOrderTextField.currentValue
                         }
                    }
                    Button {
                        text: qsTr("OK")
                        enabled: biggearsOrderTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_biggears_order = biggearsOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                SwitchDelegate {
                    Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.scaledFontPx; restoreMode: Binding.RestoreBindingOrValue }
                    text: qsTr("Swap Buttons")
                    spacing: 0
                    bottomPadding: 0
                    topPadding: 0
                    rightPadding: 0
                    leftPadding: 0
                    clip: false
                    checked: settings.tile_biggears_swap
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    Layout.fillWidth: true
                    onClicked: settings.tile_biggears_swap = checked
                }
            }
        }

        Label {
            text: qsTr("It shows 2 big gear buttons on the UI")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: remainingTimeTrainingProgramRowEnabledAccordion
            //: Tile title: time left in the current step (row) of the workout program, not rowing and not a lap.
            title: qsTr("Remaining Time/Row")
            linkedBoolSetting: "tile_remainingtimetrainprogramrow_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelremainingTimeTrainingProgramRowOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: remainingTimeTrainingProgramRowOrderTextField
                    Accessible.name: remainingTimeTrainingProgramRowEnabledAccordion.title + ", " + labelremainingTimeTrainingProgramRowOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_remainingtimetrainprogramrow_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = remainingTimeTrainingProgramRowOrderTextField.currentValue
                     }
                }
                Button {
                    id: okremainingTimeTrainingProgramRowOrderButton
                    text: qsTr("OK")
                    enabled: remainingTimeTrainingProgramRowOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_remainingtimetrainprogramrow_order = remainingTimeTrainingProgramRowOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Displays the time remaining until the next cadence and/or resistance interval.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: nextRowsTrainingProgramRowEnabledAccordion
            //: Tile title: the next steps (rows) of the loaded workout program, not a table row and not rowing.
            title: qsTr("Next Rows")
            linkedBoolSetting: "tile_nextrowstrainprogram_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelnextRowsTrainingProgramOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: nextRowsTrainingProgramOrderTextField
                    Accessible.name: nextRowsTrainingProgramRowEnabledAccordion.title + ", " + labelnextRowsTrainingProgramOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_nextrowstrainprogram_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = nextRowsTrainingProgramOrderTextField.currentValue
                     }
                }
                Button {
                    id: oknextRowsTrainingProgramOrderButton
                    text: qsTr("OK")
                    enabled: nextRowsTrainingProgramOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_nextrowstrainprogram_order = nextRowsTrainingProgramOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Displays the next Peloton interval with duration and FTP Zone (in Power Zone classes) or Peloton Resistance (non–Power Zone classes).")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: metsEnabledAccordion
            title: qsTr("METS")
            linkedBoolSetting: "tile_mets_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelmetsOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: metsOrderTextField
                    Accessible.name: metsEnabledAccordion.title + ", " + labelmetsOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_mets_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = metsOrderTextField.currentValue
                     }
                }
                Button {
                    id: okmetsOrderButton
                    text: qsTr("OK")
                    enabled: metsOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_mets_order = metsOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Displays metabolic equivalents (METs), a measurement of energy expenditure and amount of oxygen used by the body compared to the body at rest. (e.g., 4 METS requires the body to use 4 times as much oxygen than when at rest, which means it requires more energy and burns more calories).")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: targetMetsEnabledAccordion
            title: qsTr("Target METS")
            linkedBoolSetting: "tile_targetmets_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labeltargetmetsOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: targetmetsOrderTextField
                    Accessible.name: targetMetsEnabledAccordion.title + ", " + labeltargetmetsOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_targetmets_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = targetmetsOrderTextField.currentValue
                     }
                }
                Button {
                    id: oktargetmetsOrderButton
                    text: qsTr("OK")
                    enabled: targetmetsOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_targetmets_order = targetmetsOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        AccordionCheckElement {
            id: datetimeEnabledAccordion
            title: qsTr("Time")
            linkedBoolSetting: "tile_datetime_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labeldatetimeOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: datetimeOrderTextField
                    Accessible.name: datetimeEnabledAccordion.title + ", " + labeldatetimeOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_datetime_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = datetimeOrderTextField.currentValue
                     }
                }
                Button {
                    id: okdatetimeOrderButton
                    text: qsTr("OK")
                    enabled: datetimeOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_datetime_order = datetimeOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Displays the current time.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: targetStrokesCountAccordion
            title: qsTr("Strokes Count")
            linkedBoolSetting: "tile_strokes_count_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelstrokes_countOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: strokes_countOrderTextField
                    Accessible.name: targetStrokesCountAccordion.title + ", " + labelstrokes_countOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_strokes_count_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = strokes_countOrderTextField.currentValue
                     }
                }
                Button {
                    id: okstrokes_countOrderButton
                    text: qsTr("OK")
                    enabled: strokes_countOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_strokes_count_order = strokes_countOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("(Rower only) Displays the number of strokes rowed.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: targetStrokesLengthAccordion
            title: qsTr("Stroke Length")
            linkedBoolSetting: "tile_strokes_length_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelstrokes_lengthOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: strokes_lengthOrderTextField
                    Accessible.name: targetStrokesLengthAccordion.title + ", " + labelstrokes_lengthOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_strokes_length_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = strokes_lengthOrderTextField.currentValue
                     }
                }
                Button {
                    id: okstrokes_lengthOrderButton
                    text: qsTr("OK")
                    enabled: strokes_lengthOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_strokes_length_order = strokes_lengthOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("(Rower only) Displays the stroke length.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: targetSteeringAngleEnabledAccordion
            title: qsTr("Steering Angle")
            linkedBoolSetting: "tile_steering_angle_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelsteeringAngleOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: steeringAngleOrderTextField
                    Accessible.name: targetSteeringAngleEnabledAccordion.title + ", " + labelsteeringAngleOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_steering_angle_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = steeringAngleOrderTextField.currentValue
                     }
                }
                Button {
                    id: oksteeringAngleOrderButton
                    text: qsTr("OK")
                    enabled: steeringAngleOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_steering_angle_order = steeringAngleOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("(Elite Rizer only) Displays steering angle.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: targetPIDHrAccordion
            title: qsTr("PID HR Zone")
            linkedBoolSetting: "tile_pid_hr_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelPIDHROrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: pidHROrderTextField
                    Accessible.name: targetPIDHrAccordion.title + ", " + labelPIDHROrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_pid_hr_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = pidHROrderTextField.currentValue
                     }
                }
                Button {
                    id: okpidHROrderButton
                    text: qsTr("OK")
                    enabled: pidHROrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_pid_hr_order = pidHROrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Use this tile to display the target heart rate zone in which you’ve chosen to work out in Settings > Training Program Options.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: extInclineAccordion
            title: qsTr("External Incline")
            linkedBoolSetting: "tile_ext_incline_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelExtInclineOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: extInclineOrderTextField
                    Accessible.name: extInclineAccordion.title + ", " + labelExtInclineOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_ext_incline_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = extInclineOrderTextField.currentValue
                     }
                }
                Button {
                    id: okextInclineOrderButton
                    text: qsTr("OK")
                    enabled: extInclineOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_ext_incline_order = extInclineOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("(Elite Rizer only) Allows control of the incline of external inclination equipment.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: strideLength
            title: qsTr("Stride Length")
            linkedBoolSetting: "tile_instantaneous_stride_length_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelStrideLengthOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: strideLengthOrderTextField
                    Accessible.name: strideLength.title + ", " + labelStrideLengthOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_instantaneous_stride_length_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = strideLengthOrderTextField.currentValue
                     }
                }
                Button {
                    id: okStrideLengthOrderButton
                    text: qsTr("OK")
                    enabled: strideLengthOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_instantaneous_stride_length_order = strideLengthOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("(requires a compatible footpod with accelerometer; treadmill only) Displays stride while walking or running.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: groundContact
            title: qsTr("Ground Contact")
            linkedBoolSetting: "tile_ground_contact_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelGroundContactOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: groundContactOrderTextField
                    Accessible.name: groundContact.title + ", " + labelGroundContactOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_ground_contact_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = groundContactOrderTextField.currentValue
                     }
                }
                Button {
                    id: okGroundContactOrderButton
                    text: qsTr("OK")
                    enabled: groundContactOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_ground_contact_order = groundContactOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("(requires a compatible footpod with accelerometer; treadmill only) Displays time foot is on contact with ground while walking or running.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: verticalOscillation
            title: qsTr("Vertical Oscillation")
            linkedBoolSetting: "tile_vertical_oscillation_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelVerticalOscillationOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: verticalOscillationOrderTextField
                    Accessible.name: verticalOscillation.title + ", " + labelVerticalOscillationOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_vertical_oscillation_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = verticalOscillationOrderTextField.currentValue
                     }
                }
                Button {
                    id: okVerticalOscillationOrderButton
                    text: qsTr("OK")
                    enabled: verticalOscillationOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_vertical_oscillation_order = verticalOscillationOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("(requires a compatible footpod with accelerometer; treadmill only) Displays the up and down movement while walking or running.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: paceLast500mEnabledAccordion
            title: qsTr("Pace Last 500m")
            linkedBoolSetting: "tile_pace_last500m_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelPacelast500mOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: pacelast500mOrderTextField
                    Accessible.name: paceLast500mEnabledAccordion.title + ", " + labelPacelast500mOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_pace_last500m_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = pacelast500mOrderTextField.currentValue
                     }
                }
                Button {
                    id: okPacelast500mOrderButton
                    text: qsTr("OK")
                    enabled: pacelast500mOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_pace_last500m_order = pacelast500mOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        AccordionCheckElement {
            id: stepCountEnabledAccordion
            title: qsTr("Step Count")
            linkedBoolSetting: "tile_step_count_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelStepCountOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: stepCountOrderTextField
                    Accessible.name: stepCountEnabledAccordion.title + ", " + labelStepCountOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_step_count_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = stepCountOrderTextField.currentValue
                     }
                }
                Button {
                    id: okStepCountOrderButton
                    text: qsTr("OK")
                    enabled: stepCountOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_step_count_order = stepCountOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        AccordionCheckElement {
            id: ergModeEnabledAccordion
            title: qsTr("Erg Mode")
            linkedBoolSetting: "tile_erg_mode_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelErgModeOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: ergModeOrderTextField
                    Accessible.name: ergModeEnabledAccordion.title + ", " + labelErgModeOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_erg_mode_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = ergModeOrderTextField.currentValue
                     }
                }
                Button {
                    id: okErgModeOrderButton
                    text: qsTr("OK")
                    enabled: ergModeOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_erg_mode_order = ergModeOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        AccordionCheckElement {
            title: qsTr("Running Stress Score")
            linkedBoolSetting: "tile_rss_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: rssOrderTextField
                    Accessible.name: qsTr("Running Stress Score") + ", " + qsTr("order index:")
                    model: rootItem.tile_order
                    savedText: settings.tile_rss_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = rssOrderTextField.currentValue
                     }
                }
                Button {
                    text: qsTr("OK")
                    enabled: rssOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_rss_order = rssOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }        

        AccordionCheckElement {
            id: presetResistance1EnabledAccordion
            title: qsTr("Preset Resistance 1")
            linkedBoolSetting: "tile_preset_resistance_1_enabled"
            settings: settingsTilesPane.settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetResistance1Order
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: presetResistance1TextField
                        Accessible.name: presetResistance1EnabledAccordion.title + ", " + labelPresetResistance1Order.text
                        model: rootItem.tile_order
                        savedText: settings.tile_preset_resistance_1_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetResistance1TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetResistance1OrderButton
                        text: qsTr("OK")
                        enabled: presetResistance1TextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_resistance_1_order = presetResistance1TextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetResistance1Value
                        text: qsTr("value:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetResistance1ValueTextField
                        Accessible.name: presetResistance1EnabledAccordion.title + ", " + labelPresetResistance1Value.text
                        savedText: settings.tile_preset_resistance_1_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetResistance1ValueButton
                        text: qsTr("OK")
                        enabled: presetResistance1ValueTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_resistance_1_value = presetResistance1ValueTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetResistance1Label
                        text: qsTr("label:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetResistance1LabelTextField
                        Accessible.name: presetResistance1EnabledAccordion.title + ", " + labelPresetResistance1Label.text
                        savedText: settings.tile_preset_resistance_1_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetResistance1LabelButton
                        text: qsTr("OK")
                        enabled: presetResistance1LabelTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_resistance_1_label = presetResistance1LabelTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
					 RowLayout {
					     Label {
						      id: labelPresetResistance1Color
								text: qsTr("color:")
								wrapMode: window.labelWrap
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetResistance1
								title: qsTr("Please choose a color")
								onAccepted: {
								    presetResistance1ColorTextField.text = colorPresetResistance1.color
									}
								onRejected: {

                        }
						  }
						  SettingsTextField {
						      id: presetResistance1ColorTextField
						      Accessible.name: presetResistance1EnabledAccordion.title + ", " + labelPresetResistance1Color.text
								savedText: settings.tile_preset_resistance_1_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetResistance1.visible = true
								}
						  }
						  Button {
						      id: okPresetResistance1ColorButton
								text: qsTr("OK")
								enabled: presetResistance1ColorTextField.dirty
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_resistance_1_color = presetResistance1ColorTextField.displayText; toast.show(qsTr("Setting saved!")); }
						  }
					 }
            }
        }
        AccordionCheckElement {
            id: presetResistance2EnabledAccordion
            title: qsTr("Preset Resistance 2")
            linkedBoolSetting: "tile_preset_resistance_2_enabled"
            settings: settingsTilesPane.settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetResistance2Order
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: presetResistance2TextField
                        Accessible.name: presetResistance2EnabledAccordion.title + ", " + labelPresetResistance2Order.text
                        model: rootItem.tile_order
                        savedText: settings.tile_preset_resistance_2_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetResistance2TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetResistance2OrderButton
                        text: qsTr("OK")
                        enabled: presetResistance2TextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_resistance_2_order = presetResistance2TextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetResistance2Value
                        text: qsTr("value:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetResistance2ValueTextField
                        Accessible.name: presetResistance2EnabledAccordion.title + ", " + labelPresetResistance2Value.text
                        savedText: settings.tile_preset_resistance_2_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetResistance2ValueButton
                        text: qsTr("OK")
                        enabled: presetResistance2ValueTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_resistance_2_value = presetResistance2ValueTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetResistance2Label
                        text: qsTr("label:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetResistance2LabelTextField
                        Accessible.name: presetResistance2EnabledAccordion.title + ", " + labelPresetResistance2Label.text
                        savedText: settings.tile_preset_resistance_2_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetResistance2LabelButton
                        text: qsTr("OK")
                        enabled: presetResistance2LabelTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_resistance_2_label = presetResistance2LabelTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
					 RowLayout {
					     Label {
						      id: labelPresetResistance2Color
								text: qsTr("color:")
								wrapMode: window.labelWrap
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetResistance2
								title: qsTr("Please choose a color")
								onAccepted: {
								    presetResistance2ColorTextField.text = colorPresetResistance2.color
									}
								onRejected: {

                        }
						  }
						  SettingsTextField {
						      id: presetResistance2ColorTextField
						      Accessible.name: presetResistance2EnabledAccordion.title + ", " + labelPresetResistance2Color.text
								savedText: settings.tile_preset_resistance_2_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetResistance2.visible = true
								}
						  }
						  Button {
						      id: okPresetResistance2ColorButton
								text: qsTr("OK")
								enabled: presetResistance2ColorTextField.dirty
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_resistance_2_color = presetResistance2ColorTextField.displayText; toast.show(qsTr("Setting saved!")); }
						  }
					 }
            }
        }
        AccordionCheckElement {
            id: presetResistance3EnabledAccordion
            title: qsTr("Preset Resistance 3")
            linkedBoolSetting: "tile_preset_resistance_3_enabled"
            settings: settingsTilesPane.settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetResistance3Order
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: presetResistance3TextField
                        Accessible.name: presetResistance3EnabledAccordion.title + ", " + labelPresetResistance3Order.text
                        model: rootItem.tile_order
                        savedText: settings.tile_preset_resistance_3_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetResistance3TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetResistance3OrderButton
                        text: qsTr("OK")
                        enabled: presetResistance3TextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_resistance_3_order = presetResistance3TextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetResistance3Value
                        text: qsTr("value:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetResistance3ValueTextField
                        Accessible.name: presetResistance3EnabledAccordion.title + ", " + labelPresetResistance3Value.text
                        savedText: settings.tile_preset_resistance_3_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetResistance3ValueButton
                        text: qsTr("OK")
                        enabled: presetResistance3ValueTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_resistance_3_value = presetResistance3ValueTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetResistance3Label
                        text: qsTr("label:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetResistance3LabelTextField
                        Accessible.name: presetResistance3EnabledAccordion.title + ", " + labelPresetResistance3Label.text
                        savedText: settings.tile_preset_resistance_3_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetResistance3LabelButton
                        text: qsTr("OK")
                        enabled: presetResistance3LabelTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_resistance_3_label = presetResistance3LabelTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
					 RowLayout {
					     Label {
						      id: labelPresetResistance3Color
								text: qsTr("color:")
								wrapMode: window.labelWrap
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetResistance3
								title: qsTr("Please choose a color")
								onAccepted: {
								    presetResistance3ColorTextField.text = colorPresetResistance3.color
									}
								onRejected: {

                        }
						  }
						  SettingsTextField {
						      id: presetResistance3ColorTextField
						      Accessible.name: presetResistance3EnabledAccordion.title + ", " + labelPresetResistance3Color.text
								savedText: settings.tile_preset_resistance_3_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetResistance3.visible = true
								}
						  }
						  Button {
						      id: okPresetResistance3ColorButton
								text: qsTr("OK")
								enabled: presetResistance3ColorTextField.dirty
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_resistance_3_color = presetResistance3ColorTextField.displayText; toast.show(qsTr("Setting saved!")); }
						  }
					 }
            }
        }
        AccordionCheckElement {
            id: presetResistance4EnabledAccordion
            title: qsTr("Preset Resistance 4")
            linkedBoolSetting: "tile_preset_resistance_4_enabled"
            settings: settingsTilesPane.settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetResistance4Order
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: presetResistance4TextField
                        Accessible.name: presetResistance4EnabledAccordion.title + ", " + labelPresetResistance4Order.text
                        model: rootItem.tile_order
                        savedText: settings.tile_preset_resistance_4_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetResistance4TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetResistance4OrderButton
                        text: qsTr("OK")
                        enabled: presetResistance4TextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_resistance_4_order = presetResistance4TextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetResistance4Value
                        text: qsTr("value:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetResistance4ValueTextField
                        Accessible.name: presetResistance4EnabledAccordion.title + ", " + labelPresetResistance4Value.text
                        savedText: settings.tile_preset_resistance_4_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetResistance4ValueButton
                        text: qsTr("OK")
                        enabled: presetResistance4ValueTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_resistance_4_value = presetResistance4ValueTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetResistance4Label
                        text: qsTr("label:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetResistance4LabelTextField
                        Accessible.name: presetResistance4EnabledAccordion.title + ", " + labelPresetResistance4Label.text
                        savedText: settings.tile_preset_resistance_4_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetResistance4LabelButton
                        text: qsTr("OK")
                        enabled: presetResistance4LabelTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_resistance_4_label = presetResistance4LabelTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
					 RowLayout {
					     Label {
						      id: labelPresetResistance4Color
								text: qsTr("color:")
								wrapMode: window.labelWrap
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetResistance4
								title: qsTr("Please choose a color")
								onAccepted: {
								    presetResistance4ColorTextField.text = colorPresetResistance4.color
									}
								onRejected: {

                        }
						  }
						  SettingsTextField {
						      id: presetResistance4ColorTextField
						      Accessible.name: presetResistance4EnabledAccordion.title + ", " + labelPresetResistance4Color.text
								savedText: settings.tile_preset_resistance_4_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetResistance4.visible = true
								}
						  }
						  Button {
						      id: okPresetResistance4ColorButton
								text: qsTr("OK")
								enabled: presetResistance4ColorTextField.dirty
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_resistance_4_color = presetResistance4ColorTextField.displayText; toast.show(qsTr("Setting saved!")); }
						  }
					 }
            }
        }
        AccordionCheckElement {
            id: presetResistance5EnabledAccordion
            title: qsTr("Preset Resistance 5")
            linkedBoolSetting: "tile_preset_resistance_5_enabled"
            settings: settingsTilesPane.settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetResistance5Order
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: presetResistance5TextField
                        Accessible.name: presetResistance5EnabledAccordion.title + ", " + labelPresetResistance5Order.text
                        model: rootItem.tile_order
                        savedText: settings.tile_preset_resistance_5_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetResistance5TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetResistance5OrderButton
                        text: qsTr("OK")
                        enabled: presetResistance5TextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_resistance_5_order = presetResistance5TextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetResistance5Value
                        text: qsTr("value:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetResistance5ValueTextField
                        Accessible.name: presetResistance5EnabledAccordion.title + ", " + labelPresetResistance5Value.text
                        savedText: settings.tile_preset_resistance_5_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetResistance5ValueButton
                        text: qsTr("OK")
                        enabled: presetResistance5ValueTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_resistance_5_value = presetResistance5ValueTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetResistance5Label
                        text: qsTr("label:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetResistance5LabelTextField
                        Accessible.name: presetResistance5EnabledAccordion.title + ", " + labelPresetResistance5Label.text
                        savedText: settings.tile_preset_resistance_5_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetResistance5LabelButton
                        text: qsTr("OK")
                        enabled: presetResistance5LabelTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_resistance_5_label = presetResistance5LabelTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
					 RowLayout {
					     Label {
						      id: labelPresetResistance5Color
								text: qsTr("color:")
								wrapMode: window.labelWrap
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetResistance5
								title: qsTr("Please choose a color")
								onAccepted: {
								    presetResistance5ColorTextField.text = colorPresetResistance5.color
									}
								onRejected: {

                        }
						  }
						  SettingsTextField {
						      id: presetResistance5ColorTextField
						      Accessible.name: presetResistance5EnabledAccordion.title + ", " + labelPresetResistance5Color.text
								savedText: settings.tile_preset_resistance_5_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetResistance5.visible = true
								}
						  }
						  Button {
						      id: okPresetResistance5ColorButton
								text: qsTr("OK")
								enabled: presetResistance5ColorTextField.dirty
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_resistance_5_color = presetResistance5ColorTextField.displayText; toast.show(qsTr("Setting saved!")); }
						  }
					 }
            }
        }
        AccordionCheckElement {
            id: presetSpeed1EnabledAccordion
            title: qsTr("Preset Speed 1")
            linkedBoolSetting: "tile_preset_speed_1_enabled"
            settings: settingsTilesPane.settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetSpeed1Order
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: presetSpeed1TextField
                        Accessible.name: presetSpeed1EnabledAccordion.title + ", " + labelPresetSpeed1Order.text
                        model: rootItem.tile_order
                        savedText: settings.tile_preset_speed_1_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetSpeed1TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetSpeed1OrderButton
                        text: qsTr("OK")
                        enabled: presetSpeed1TextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_speed_1_order = presetSpeed1TextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetSpeed1Value
                        text: qsTr("value:") + " (" + (settings.miles_unit ? qsTr("mph") : qsTr("km/h")) + ")"
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetSpeed1ValueTextField
                        Accessible.name: presetSpeed1EnabledAccordion.title + ", " + labelPresetSpeed1Value.text
                        savedText: settingsTilesPane.formatPresetSpeedValue(settings.tile_preset_speed_1_value)
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetSpeed1ValueButton
                        text: qsTr("OK")
                        enabled: presetSpeed1ValueTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {
                            settings.tile_preset_speed_1_value = settingsTilesPane.parsePresetSpeedValue(presetSpeed1ValueTextField.displayText);
                            presetSpeed1ValueTextField.text = settingsTilesPane.formatPresetSpeedValue(settings.tile_preset_speed_1_value);
                            toast.show(qsTr("Setting saved!"));
                        }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetSpeed1Label
                        text: qsTr("label:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetSpeed1LabelTextField
                        Accessible.name: presetSpeed1EnabledAccordion.title + ", " + labelPresetSpeed1Label.text
                        savedText: settings.tile_preset_speed_1_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetSpeed1LabelButton
                        text: qsTr("OK")
                        enabled: presetSpeed1LabelTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_speed_1_label = presetSpeed1LabelTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
					 RowLayout {
					     Label {
						      id: labelPresetSpeed1Color
								text: qsTr("color:")
								wrapMode: window.labelWrap
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetSpeed1
								title: qsTr("Please choose a color")
								onAccepted: {
								    presetSpeed1ColorTextField.text = colorPresetSpeed1.color
									}
								onRejected: {

                        }
						  }
						  SettingsTextField {
						      id: presetSpeed1ColorTextField
						      Accessible.name: presetSpeed1EnabledAccordion.title + ", " + labelPresetSpeed1Color.text
								savedText: settings.tile_preset_speed_1_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetSpeed1.visible = true
								}
						  }
						  Button {
						      id: okPresetSpeed1ColorButton
								text: qsTr("OK")
								enabled: presetSpeed1ColorTextField.dirty
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_speed_1_color = presetSpeed1ColorTextField.displayText; toast.show(qsTr("Setting saved!")); }
						  }
					 }
            }
        }
        AccordionCheckElement {
            id: presetSpeed2EnabledAccordion
            title: qsTr("Preset Speed 2")
            linkedBoolSetting: "tile_preset_speed_2_enabled"
            settings: settingsTilesPane.settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetSpeed2Order
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: presetSpeed2TextField
                        Accessible.name: presetSpeed2EnabledAccordion.title + ", " + labelPresetSpeed2Order.text
                        model: rootItem.tile_order
                        savedText: settings.tile_preset_speed_2_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetSpeed2TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetSpeed2OrderButton
                        text: qsTr("OK")
                        enabled: presetSpeed2TextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_speed_2_order = presetSpeed2TextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetSpeed2Value
                        text: qsTr("value:") + " (" + (settings.miles_unit ? qsTr("mph") : qsTr("km/h")) + ")"
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetSpeed2ValueTextField
                        Accessible.name: presetSpeed2EnabledAccordion.title + ", " + labelPresetSpeed2Value.text
                        savedText: settingsTilesPane.formatPresetSpeedValue(settings.tile_preset_speed_2_value)
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetSpeed2ValueButton
                        text: qsTr("OK")
                        enabled: presetSpeed2ValueTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {
                            settings.tile_preset_speed_2_value = settingsTilesPane.parsePresetSpeedValue(presetSpeed2ValueTextField.displayText);
                            presetSpeed2ValueTextField.text = settingsTilesPane.formatPresetSpeedValue(settings.tile_preset_speed_2_value);
                            toast.show(qsTr("Setting saved!"));
                        }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetSpeed2Label
                        text: qsTr("label:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetSpeed2LabelTextField
                        Accessible.name: presetSpeed2EnabledAccordion.title + ", " + labelPresetSpeed2Label.text
                        savedText: settings.tile_preset_speed_2_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetSpeed2LabelButton
                        text: qsTr("OK")
                        enabled: presetSpeed2LabelTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_speed_2_label = presetSpeed2LabelTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
					 RowLayout {
					     Label {
						      id: labelPresetSpeed2Color
								text: qsTr("color:")
								wrapMode: window.labelWrap
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetSpeed2
								title: qsTr("Please choose a color")
								onAccepted: {
								    presetSpeed2ColorTextField.text = colorPresetSpeed2.color
									}
								onRejected: {

                        }
						  }
						  SettingsTextField {
						      id: presetSpeed2ColorTextField
						      Accessible.name: presetSpeed2EnabledAccordion.title + ", " + labelPresetSpeed2Color.text
								savedText: settings.tile_preset_speed_2_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetSpeed2.visible = true
								}
						  }
						  Button {
						      id: okPresetSpeed2ColorButton
								text: qsTr("OK")
								enabled: presetSpeed2ColorTextField.dirty
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_speed_2_color = presetSpeed2ColorTextField.displayText; toast.show(qsTr("Setting saved!")); }
						  }
					 }
            }
        }
        AccordionCheckElement {
            id: presetSpeed3EnabledAccordion
            title: qsTr("Preset Speed 3")
            linkedBoolSetting: "tile_preset_speed_3_enabled"
            settings: settingsTilesPane.settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetSpeed3Order
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: presetSpeed3TextField
                        Accessible.name: presetSpeed3EnabledAccordion.title + ", " + labelPresetSpeed3Order.text
                        model: rootItem.tile_order
                        savedText: settings.tile_preset_speed_3_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetSpeed3TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetSpeed3OrderButton
                        text: qsTr("OK")
                        enabled: presetSpeed3TextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_speed_3_order = presetSpeed3TextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetSpeed3Value
                        text: qsTr("value:") + " (" + (settings.miles_unit ? qsTr("mph") : qsTr("km/h")) + ")"
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetSpeed3ValueTextField
                        Accessible.name: presetSpeed3EnabledAccordion.title + ", " + labelPresetSpeed3Value.text
                        savedText: settingsTilesPane.formatPresetSpeedValue(settings.tile_preset_speed_3_value)
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetSpeed3ValueButton
                        text: qsTr("OK")
                        enabled: presetSpeed3ValueTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {
                            settings.tile_preset_speed_3_value = settingsTilesPane.parsePresetSpeedValue(presetSpeed3ValueTextField.displayText);
                            presetSpeed3ValueTextField.text = settingsTilesPane.formatPresetSpeedValue(settings.tile_preset_speed_3_value);
                            toast.show(qsTr("Setting saved!"));
                        }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetSpeed3Label
                        text: qsTr("label:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetSpeed3LabelTextField
                        Accessible.name: presetSpeed3EnabledAccordion.title + ", " + labelPresetSpeed3Label.text
                        savedText: settings.tile_preset_speed_3_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetSpeed3LabelButton
                        text: qsTr("OK")
                        enabled: presetSpeed3LabelTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_speed_3_label = presetSpeed3LabelTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
					 RowLayout {
					     Label {
						      id: labelPresetSpeed3Color
								text: qsTr("color:")
        wrapMode: window.labelWrap
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetSpeed3
								title: qsTr("Please choose a color")
								onAccepted: {
								    presetSpeed3ColorTextField.text = colorPresetSpeed3.color
									}
								onRejected: {

                        }
						  }
						  SettingsTextField {
						      id: presetSpeed3ColorTextField
						      Accessible.name: presetSpeed3EnabledAccordion.title + ", " + labelPresetSpeed3Color.text
								savedText: settings.tile_preset_speed_3_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetSpeed3.visible = true
								}
						  }
						  Button {
						      id: okPresetSpeed3ColorButton
								text: qsTr("OK")
								enabled: presetSpeed3ColorTextField.dirty
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_speed_3_color = presetSpeed3ColorTextField.displayText; toast.show(qsTr("Setting saved!")); }
						  }
					 }
            }
        }
        AccordionCheckElement {
            id: presetSpeed4EnabledAccordion
            title: qsTr("Preset Speed 4")
            linkedBoolSetting: "tile_preset_speed_4_enabled"
            settings: settingsTilesPane.settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetSpeed4Order
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: presetSpeed4TextField
                        Accessible.name: presetSpeed4EnabledAccordion.title + ", " + labelPresetSpeed4Order.text
                        model: rootItem.tile_order
                        savedText: settings.tile_preset_speed_4_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetSpeed4TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetSpeed4OrderButton
                        text: qsTr("OK")
                        enabled: presetSpeed4TextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_speed_4_order = presetSpeed4TextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetSpeed4Value
                        text: qsTr("value:") + " (" + (settings.miles_unit ? qsTr("mph") : qsTr("km/h")) + ")"
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetSpeed4ValueTextField
                        Accessible.name: presetSpeed4EnabledAccordion.title + ", " + labelPresetSpeed4Value.text
                        savedText: settingsTilesPane.formatPresetSpeedValue(settings.tile_preset_speed_4_value)
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetSpeed4ValueButton
                        text: qsTr("OK")
                        enabled: presetSpeed4ValueTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {
                            settings.tile_preset_speed_4_value = settingsTilesPane.parsePresetSpeedValue(presetSpeed4ValueTextField.displayText);
                            presetSpeed4ValueTextField.text = settingsTilesPane.formatPresetSpeedValue(settings.tile_preset_speed_4_value);
                            toast.show(qsTr("Setting saved!"));
                        }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetSpeed4Label
                        text: qsTr("label:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetSpeed4LabelTextField
                        Accessible.name: presetSpeed4EnabledAccordion.title + ", " + labelPresetSpeed4Label.text
                        savedText: settings.tile_preset_speed_4_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetSpeed4LabelButton
                        text: qsTr("OK")
                        enabled: presetSpeed4LabelTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_speed_4_label = presetSpeed4LabelTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
					 RowLayout {
					     Label {
						      id: labelPresetSpeed4Color
								text: qsTr("color:")
        wrapMode: window.labelWrap
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetSpeed4
								title: qsTr("Please choose a color")
								onAccepted: {
								    presetSpeed4ColorTextField.text = colorPresetSpeed4.color
									}
								onRejected: {

                        }
						  }
						  SettingsTextField {
						      id: presetSpeed4ColorTextField
						      Accessible.name: presetSpeed4EnabledAccordion.title + ", " + labelPresetSpeed4Color.text
								savedText: settings.tile_preset_speed_4_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetSpeed4.visible = true
								}
						  }
						  Button {
						      id: okPresetSpeed4ColorButton
								text: qsTr("OK")
								enabled: presetSpeed4ColorTextField.dirty
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_speed_4_color = presetSpeed4ColorTextField.displayText; toast.show(qsTr("Setting saved!")); }
						  }
					 }
            }
        }
        AccordionCheckElement {
            id: presetSpeed5EnabledAccordion
            title: qsTr("Preset Speed 5")
            linkedBoolSetting: "tile_preset_speed_5_enabled"
            settings: settingsTilesPane.settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetSpeed5Order
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: presetSpeed5TextField
                        Accessible.name: presetSpeed5EnabledAccordion.title + ", " + labelPresetSpeed5Order.text
                        model: rootItem.tile_order
                        savedText: settings.tile_preset_speed_5_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetSpeed5TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetSpeed5OrderButton
                        text: qsTr("OK")
                        enabled: presetSpeed5TextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_speed_5_order = presetSpeed5TextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetSpeed5Value
                        text: qsTr("value:") + " (" + (settings.miles_unit ? qsTr("mph") : qsTr("km/h")) + ")"
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetSpeed5ValueTextField
                        Accessible.name: presetSpeed5EnabledAccordion.title + ", " + labelPresetSpeed5Value.text
                        savedText: settingsTilesPane.formatPresetSpeedValue(settings.tile_preset_speed_5_value)
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetSpeed5ValueButton
                        text: qsTr("OK")
                        enabled: presetSpeed5ValueTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {
                            settings.tile_preset_speed_5_value = settingsTilesPane.parsePresetSpeedValue(presetSpeed5ValueTextField.displayText);
                            presetSpeed5ValueTextField.text = settingsTilesPane.formatPresetSpeedValue(settings.tile_preset_speed_5_value);
                            toast.show(qsTr("Setting saved!"));
                        }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetSpeed5Label
                        text: qsTr("label:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetSpeed5LabelTextField
                        Accessible.name: presetSpeed5EnabledAccordion.title + ", " + labelPresetSpeed5Label.text
                        savedText: settings.tile_preset_speed_5_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetSpeed5LabelButton
                        text: qsTr("OK")
                        enabled: presetSpeed5LabelTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_speed_5_label = presetSpeed5LabelTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
					 RowLayout {
					     Label {
						      id: labelPresetSpeed5Color
								text: qsTr("color:")
        wrapMode: window.labelWrap
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetSpeed5
								title: qsTr("Please choose a color")
								onAccepted: {
								    presetSpeed5ColorTextField.text = colorPresetSpeed5.color
									}
								onRejected: {

                        }
						  }
						  SettingsTextField {
						      id: presetSpeed5ColorTextField
						      Accessible.name: presetSpeed5EnabledAccordion.title + ", " + labelPresetSpeed5Color.text
								savedText: settings.tile_preset_speed_5_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetSpeed5.visible = true
								}
						  }
						  Button {
						      id: okPresetSpeed5ColorButton
								text: qsTr("OK")
								enabled: presetSpeed5ColorTextField.dirty
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_speed_5_color = presetSpeed5ColorTextField.displayText; toast.show(qsTr("Setting saved!")); }
						  }
					 }
            }
        }
        AccordionCheckElement {
            id: presetInclination1EnabledAccordion
            title: qsTr("Preset Inclination 1")
            linkedBoolSetting: "tile_preset_inclination_1_enabled"
            settings: settingsTilesPane.settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetInclination1Order
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: presetInclination1TextField
                        Accessible.name: presetInclination1EnabledAccordion.title + ", " + labelPresetInclination1Order.text
                        model: rootItem.tile_order
                        savedText: settings.tile_preset_inclination_1_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetInclination1TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetInclination1OrderButton
                        text: qsTr("OK")
                        enabled: presetInclination1TextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_inclination_1_order = presetInclination1TextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetInclination1Value
                        text: qsTr("value:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetInclination1ValueTextField
                        Accessible.name: presetInclination1EnabledAccordion.title + ", " + labelPresetInclination1Value.text
                        savedText: settings.tile_preset_inclination_1_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetInclination1ValueButton
                        text: qsTr("OK")
                        enabled: presetInclination1ValueTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_inclination_1_value = presetInclination1ValueTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetInclination1Label
                        text: qsTr("label:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetInclination1LabelTextField
                        Accessible.name: presetInclination1EnabledAccordion.title + ", " + labelPresetInclination1Label.text
                        savedText: settings.tile_preset_inclination_1_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetInclination1LabelButton
                        text: qsTr("OK")
                        enabled: presetInclination1LabelTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_inclination_1_label = presetInclination1LabelTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
					 RowLayout {
					     Label {
						      id: labelPresetInclination1Color
								text: qsTr("color:")
        wrapMode: window.labelWrap
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetInclination1
								title: qsTr("Please choose a color")
								onAccepted: {
								    presetInclination1ColorTextField.text = colorPresetInclination1.color
									}
								onRejected: {

                        }
						  }
						  SettingsTextField {
						      id: presetInclination1ColorTextField
						      Accessible.name: presetInclination1EnabledAccordion.title + ", " + labelPresetInclination1Color.text
								savedText: settings.tile_preset_inclination_1_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetInclination1.visible = true
								}
						  }
						  Button {
						      id: okPresetInclination1ColorButton
								text: qsTr("OK")
								enabled: presetInclination1ColorTextField.dirty
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_inclination_1_color = presetInclination1ColorTextField.displayText; toast.show(qsTr("Setting saved!")); }
						  }
					 }
            }
        }
        AccordionCheckElement {
            id: presetInclination2EnabledAccordion
            title: qsTr("Preset Inclination 2")
            linkedBoolSetting: "tile_preset_inclination_2_enabled"
            settings: settingsTilesPane.settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetInclination2Order
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: presetInclination2TextField
                        Accessible.name: presetInclination2EnabledAccordion.title + ", " + labelPresetInclination2Order.text
                        model: rootItem.tile_order
                        savedText: settings.tile_preset_inclination_2_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetInclination2TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetInclination2OrderButton
                        text: qsTr("OK")
                        enabled: presetInclination2TextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_inclination_2_order = presetInclination2TextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetInclination2Value
                        text: qsTr("value:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetInclination2ValueTextField
                        Accessible.name: presetInclination2EnabledAccordion.title + ", " + labelPresetInclination2Value.text
                        savedText: settings.tile_preset_inclination_2_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetInclination2ValueButton
                        text: qsTr("OK")
                        enabled: presetInclination2ValueTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_inclination_2_value = presetInclination2ValueTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetInclination2Label
                        text: qsTr("label:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetInclination2LabelTextField
                        Accessible.name: presetInclination2EnabledAccordion.title + ", " + labelPresetInclination2Label.text
                        savedText: settings.tile_preset_inclination_2_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetInclination2LabelButton
                        text: qsTr("OK")
                        enabled: presetInclination2LabelTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_inclination_2_label = presetInclination2LabelTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
            }
				RowLayout {
				    Label {
					     id: labelPresetInclination2Color
						  text: qsTr("color:")
        wrapMode: window.labelWrap
						  Layout.fillWidth: true
						  horizontalAlignment: Text.AlignRight
						  }
						ColorDialog {
						  id: colorPresetInclination2
						  title: qsTr("Please choose a color")
						  onAccepted: {
						      presetInclination2ColorTextField.text = colorPresetInclination2.color
							  }
						  onRejected: {

                    }
						}
					 SettingsTextField {
					     id: presetInclination2ColorTextField
					     Accessible.name: presetInclination2EnabledAccordion.title + ", " + labelPresetInclination2Color.text
						  savedText: settings.tile_preset_inclination_2_color
						  Layout.fillHeight: false
						  Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
						  onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
						  onPressed: {
                              if(OS_VERSION !== "Android") colorPresetInclination2.visible = true
						  }
						}
					 Button {
					     id: okPresetInclination2ColorButton
						  text: qsTr("OK")
						  enabled: presetInclination2ColorTextField.dirty
						  Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
						  onClicked: {settings.tile_preset_inclination_2_color = presetInclination2ColorTextField.displayText; toast.show(qsTr("Setting saved!")); }
						}
					}
        }
        AccordionCheckElement {
            id: presetInclination3EnabledAccordion
            title: qsTr("Preset Inclination 3")
            linkedBoolSetting: "tile_preset_inclination_3_enabled"
            settings: settingsTilesPane.settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetInclination3Order
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: presetInclination3TextField
                        Accessible.name: presetInclination3EnabledAccordion.title + ", " + labelPresetInclination3Order.text
                        model: rootItem.tile_order
                        savedText: settings.tile_preset_inclination_3_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetInclination3TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetInclination3OrderButton
                        text: qsTr("OK")
                        enabled: presetInclination3TextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_inclination_3_order = presetInclination3TextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetInclination3Value
                        text: qsTr("value:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetInclination3ValueTextField
                        Accessible.name: presetInclination3EnabledAccordion.title + ", " + labelPresetInclination3Value.text
                        savedText: settings.tile_preset_inclination_3_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetInclination3ValueButton
                        text: qsTr("OK")
                        enabled: presetInclination3ValueTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_inclination_3_value = presetInclination3ValueTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetInclination3Label
                        text: qsTr("label:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetInclination3LabelTextField
                        Accessible.name: presetInclination3EnabledAccordion.title + ", " + labelPresetInclination3Label.text
                        savedText: settings.tile_preset_inclination_3_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetInclination3LabelButton
                        text: qsTr("OK")
                        enabled: presetInclination3LabelTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_inclination_3_label = presetInclination3LabelTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
					 RowLayout {
					     Label {
						      id: labelPresetInclination3Color
								text: qsTr("color:")
        wrapMode: window.labelWrap
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetInclination3
								title: qsTr("Please choose a color")
								onAccepted: {
								    presetInclination3ColorTextField.text = colorPresetInclination3.color
									}
								onRejected: {

                        }
						  }
						  SettingsTextField {
						      id: presetInclination3ColorTextField
						      Accessible.name: presetInclination3EnabledAccordion.title + ", " + labelPresetInclination3Color.text
								savedText: settings.tile_preset_inclination_3_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetInclination3.visible = true
								}
						  }
						  Button {
						      id: okPresetInclination3ColorButton
								text: qsTr("OK")
								enabled: presetInclination3ColorTextField.dirty
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_inclination_3_color = presetInclination3ColorTextField.displayText; toast.show(qsTr("Setting saved!")); }
						  }
					 }
            }
        }
        AccordionCheckElement {
            id: presetInclination4EnabledAccordion
            title: qsTr("Preset Inclination 4")
            linkedBoolSetting: "tile_preset_inclination_4_enabled"
            settings: settingsTilesPane.settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetInclination4Order
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: presetInclination4TextField
                        Accessible.name: presetInclination4EnabledAccordion.title + ", " + labelPresetInclination4Order.text
                        model: rootItem.tile_order
                        savedText: settings.tile_preset_inclination_4_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetInclination4TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetInclination4OrderButton
                        text: qsTr("OK")
                        enabled: presetInclination4TextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_inclination_4_order = presetInclination4TextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetInclination4Value
                        text: qsTr("value:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetInclination4ValueTextField
                        Accessible.name: presetInclination4EnabledAccordion.title + ", " + labelPresetInclination4Value.text
                        savedText: settings.tile_preset_inclination_4_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetInclination4ValueButton
                        text: qsTr("OK")
                        enabled: presetInclination4ValueTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_inclination_4_value = presetInclination4ValueTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetInclination4Label
                        text: qsTr("label:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetInclination4LabelTextField
                        Accessible.name: presetInclination4EnabledAccordion.title + ", " + labelPresetInclination4Label.text
                        savedText: settings.tile_preset_inclination_4_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetInclination4LabelButton
                        text: qsTr("OK")
                        enabled: presetInclination4LabelTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_inclination_4_label = presetInclination4LabelTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
						}
					 RowLayout {
					     Label {
						      id: labelPresetInclination4Color
								text: qsTr("color:")
        wrapMode: window.labelWrap
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetInclination4
								title: qsTr("Please choose a color")
								onAccepted: {
								    presetInclination4ColorTextField.text = colorPresetInclination4.color
									}
								onRejected: {

                        }
						  }
						  SettingsTextField {
						      id: presetInclination4ColorTextField
						      Accessible.name: presetInclination4EnabledAccordion.title + ", " + labelPresetInclination4Color.text
								savedText: settings.tile_preset_inclination_4_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetInclination4.visible = true
								}
						  }
						  Button {
						      id: okPresetInclination4ColorButton
								text: qsTr("OK")
								enabled: presetInclination4ColorTextField.dirty
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_inclination_4_color = presetInclination4ColorTextField.displayText; toast.show(qsTr("Setting saved!")); }
						  }
					 }
            }
        }
        AccordionCheckElement {
            id: presetInclination5EnabledAccordion
            title: qsTr("Preset Inclination 5")
            linkedBoolSetting: "tile_preset_inclination_5_enabled"
            settings: settingsTilesPane.settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetInclination5Order
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: presetInclination5TextField
                        Accessible.name: presetInclination5EnabledAccordion.title + ", " + labelPresetInclination5Order.text
                        model: rootItem.tile_order
                        savedText: settings.tile_preset_inclination_5_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetInclination5TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetInclination5OrderButton
                        text: qsTr("OK")
                        enabled: presetInclination5TextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_inclination_5_order = presetInclination5TextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetInclination5Value
                        text: qsTr("value:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetInclination5ValueTextField
                        Accessible.name: presetInclination5EnabledAccordion.title + ", " + labelPresetInclination5Value.text
                        savedText: settings.tile_preset_inclination_5_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetInclination5ValueButton
                        text: qsTr("OK")
                        enabled: presetInclination5ValueTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_inclination_5_value = presetInclination5ValueTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetInclination5Label
                        text: qsTr("label:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetInclination5LabelTextField
                        Accessible.name: presetInclination5EnabledAccordion.title + ", " + labelPresetInclination5Label.text
                        savedText: settings.tile_preset_inclination_5_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetInclination5LabelButton
                        text: qsTr("OK")
                        enabled: presetInclination5LabelTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_inclination_5_label = presetInclination5LabelTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
					 RowLayout {
					     Label {
						      id: labelPresetInclination5Color
								text: qsTr("color:")
        wrapMode: window.labelWrap
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetInclination5
								title: qsTr("Please choose a color")
								onAccepted: {
								    presetInclination5ColorTextField.text = colorPresetInclination5.color
									}
								onRejected: {

                        }
						  }
						  SettingsTextField {
						      id: presetInclination5ColorTextField
						      Accessible.name: presetInclination5EnabledAccordion.title + ", " + labelPresetInclination5Color.text
								savedText: settings.tile_preset_inclination_5_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetInclination5.visible = true
								}
						  }
						  Button {
						      id: okPresetInclination5ColorButton
								text: qsTr("OK")
								enabled: presetInclination5ColorTextField.dirty
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_inclination_5_color = presetInclination5ColorTextField.displayText; toast.show(qsTr("Setting saved!")); }
						  }
					 }
            }
        }       
        AccordionCheckElement {
            id: presetPowerZone1EnabledAccordion
            title: qsTr("Preset Power Zone 1")
            linkedBoolSetting: "tile_preset_powerzone_1_enabled"
            settings: settingsTilesPane.settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: presetPowerZone1OrderTextField
                        Accessible.name: presetPowerZone1EnabledAccordion.title + ", " + qsTr("order index:")
                        model: rootItem.tile_order
                        savedText: settings.tile_preset_powerzone_1_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetPowerZone1OrderTextField.currentValue
                        }
                    }
                    Button {
                        text: qsTr("OK")
                        enabled: presetPowerZone1OrderTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_1_order = presetPowerZone1OrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("zone value:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetPowerZone1Value
                        Accessible.name: presetPowerZone1EnabledAccordion.title + ", " + qsTr("zone value:")
                        savedText: settings.tile_preset_powerzone_1_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        validator: DoubleValidator {bottom: 1; top: 7;}
                    }
                    Button {
                        text: qsTr("OK")
                        enabled: presetPowerZone1Value.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_1_value = parseFloat(presetPowerZone1Value.text); toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("label:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetPowerZone1Label
                        Accessible.name: presetPowerZone1EnabledAccordion.title + ", " + qsTr("label:")
                        savedText: settings.tile_preset_powerzone_1_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    }
                    Button {
                        text: qsTr("OK")
                        enabled: presetPowerZone1Label.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_1_label = presetPowerZone1Label.text; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetPowerzone1Color
                        text: qsTr("color:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ColorDialog {
                        id: colorPresetPowerzone1
                        title: qsTr("Please choose a color")
                        onAccepted: {
                            presetPowerzone1ColorTextField.text = colorPresetPowerzone1.color
                        }
                        onRejected: {}
                    }
                    SettingsTextField {
                        id: presetPowerzone1ColorTextField
                        Accessible.name: presetPowerZone1EnabledAccordion.title + ", " + labelPresetPowerzone1Color.text
                        savedText: settings.tile_preset_powerzone_1_color
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        onPressed: {
                            if(OS_VERSION !== "Android") colorPresetPowerzone1.visible = true
                        }
                    }
                    Button {
                        id: okPresetPowerzone1ColorButton
                        text: qsTr("OK")
                        enabled: presetPowerzone1ColorTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_1_color = presetPowerzone1ColorTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }                
            }
        }

        AccordionCheckElement {
            id: presetPowerZone2EnabledAccordion
            title: qsTr("Preset Power Zone 2")
            linkedBoolSetting: "tile_preset_powerzone_2_enabled"
            settings: settingsTilesPane.settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: presetPowerZone2OrderTextField
                        Accessible.name: presetPowerZone2EnabledAccordion.title + ", " + qsTr("order index:")
                        model: rootItem.tile_order
                        savedText: settings.tile_preset_powerzone_2_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetPowerZone2OrderTextField.currentValue
                        }
                    }
                    Button {
                        text: qsTr("OK")
                        enabled: presetPowerZone2OrderTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_2_order = presetPowerZone2OrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("zone value:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetPowerZone2Value
                        Accessible.name: presetPowerZone2EnabledAccordion.title + ", " + qsTr("zone value:")
                        savedText: settings.tile_preset_powerzone_2_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        validator: DoubleValidator {bottom: 1; top: 7;}
                    }
                    Button {
                        text: qsTr("OK")
                        enabled: presetPowerZone2Value.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_2_value = parseFloat(presetPowerZone2Value.text); toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("label:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetPowerZone2Label
                        Accessible.name: presetPowerZone2EnabledAccordion.title + ", " + qsTr("label:")
                        savedText: settings.tile_preset_powerzone_2_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    }
                    Button {
                        text: qsTr("OK")
                        enabled: presetPowerZone2Label.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_2_label = presetPowerZone2Label.text; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetPowerzone2Color
                        text: qsTr("color:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ColorDialog {
                        id: colorPresetPowerzone2
                        title: qsTr("Please choose a color")
                        onAccepted: {
                            presetPowerzone2ColorTextField.text = colorPresetPowerzone2.color
                        }
                        onRejected: {}
                    }
                    SettingsTextField {
                        id: presetPowerzone2ColorTextField
                        Accessible.name: presetPowerZone2EnabledAccordion.title + ", " + labelPresetPowerzone2Color.text
                        savedText: settings.tile_preset_powerzone_2_color
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        onPressed: {
                            if(OS_VERSION !== "Android") colorPresetPowerzone2.visible = true
                        }
                    }
                    Button {
                        id: okPresetPowerzone2ColorButton
                        text: qsTr("OK")
                        enabled: presetPowerzone2ColorTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_2_color = presetPowerzone2ColorTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }                
            }
        }

        AccordionCheckElement {
            id: presetPowerZone3EnabledAccordion
            title: qsTr("Preset Power Zone 3")
            linkedBoolSetting: "tile_preset_powerzone_3_enabled"
            settings: settingsTilesPane.settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: presetPowerZone3OrderTextField
                        Accessible.name: presetPowerZone3EnabledAccordion.title + ", " + qsTr("order index:")
                        model: rootItem.tile_order
                        savedText: settings.tile_preset_powerzone_3_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetPowerZone3OrderTextField.currentValue
                        }
                    }
                    Button {
                        text: qsTr("OK")
                        enabled: presetPowerZone3OrderTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_3_order = presetPowerZone3OrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("zone value:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetPowerZone3Value
                        Accessible.name: presetPowerZone3EnabledAccordion.title + ", " + qsTr("zone value:")
                        savedText: settings.tile_preset_powerzone_3_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        validator: DoubleValidator {bottom: 1; top: 7;}
                    }
                    Button {
                        text: qsTr("OK")
                        enabled: presetPowerZone3Value.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_3_value = parseFloat(presetPowerZone3Value.text); toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("label:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetPowerZone3Label
                        Accessible.name: presetPowerZone3EnabledAccordion.title + ", " + qsTr("label:")
                        savedText: settings.tile_preset_powerzone_3_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    }
                    Button {
                        text: qsTr("OK")
                        enabled: presetPowerZone3Label.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_3_label = presetPowerZone3Label.text; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetPowerzone3Color
                        text: qsTr("color:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ColorDialog {
                        id: colorPresetPowerzone3
                        title: qsTr("Please choose a color")
                        onAccepted: {
                            presetPowerzone3ColorTextField.text = colorPresetPowerzone3.color
                        }
                        onRejected: {}
                    }
                    SettingsTextField {
                        id: presetPowerzone3ColorTextField
                        Accessible.name: presetPowerZone3EnabledAccordion.title + ", " + labelPresetPowerzone3Color.text
                        savedText: settings.tile_preset_powerzone_3_color
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        onPressed: {
                            if(OS_VERSION !== "Android") colorPresetPowerzone3.visible = true
                        }
                    }
                    Button {
                        id: okPresetPowerzone3ColorButton
                        text: qsTr("OK")
                        enabled: presetPowerzone3ColorTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_3_color = presetPowerzone3ColorTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }                
            }
        }

        AccordionCheckElement {
            id: presetPowerZone4EnabledAccordion
            title: qsTr("Preset Power Zone 4")
            linkedBoolSetting: "tile_preset_powerzone_4_enabled"
            settings: settingsTilesPane.settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: presetPowerZone4OrderTextField
                        Accessible.name: presetPowerZone4EnabledAccordion.title + ", " + qsTr("order index:")
                        model: rootItem.tile_order
                        savedText: settings.tile_preset_powerzone_4_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetPowerZone4OrderTextField.currentValue
                        }
                    }
                    Button {
                        text: qsTr("OK")
                        enabled: presetPowerZone4OrderTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_4_order = presetPowerZone4OrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("zone value:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetPowerZone4Value
                        Accessible.name: presetPowerZone4EnabledAccordion.title + ", " + qsTr("zone value:")
                        savedText: settings.tile_preset_powerzone_4_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        validator: DoubleValidator {bottom: 1; top: 7;}
                    }
                    Button {
                        text: qsTr("OK")
                        enabled: presetPowerZone4Value.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_4_value = parseFloat(presetPowerZone4Value.text); toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("label:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetPowerZone4Label
                        Accessible.name: presetPowerZone4EnabledAccordion.title + ", " + qsTr("label:")
                        savedText: settings.tile_preset_powerzone_4_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    }
                    Button {
                        text: qsTr("OK")
                        enabled: presetPowerZone4Label.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_4_label = presetPowerZone4Label.text; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetPowerzone4Color
                        text: qsTr("color:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ColorDialog {
                        id: colorPresetPowerzone4
                        title: qsTr("Please choose a color")
                        onAccepted: {
                            presetPowerzone4ColorTextField.text = colorPresetPowerzone4.color
                        }
                        onRejected: {}
                    }
                    SettingsTextField {
                        id: presetPowerzone4ColorTextField
                        Accessible.name: presetPowerZone4EnabledAccordion.title + ", " + labelPresetPowerzone4Color.text
                        savedText: settings.tile_preset_powerzone_4_color
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        onPressed: {
                            if(OS_VERSION !== "Android") colorPresetPowerzone4.visible = true
                        }
                    }
                    Button {
                        id: okPresetPowerzone4ColorButton
                        text: qsTr("OK")
                        enabled: presetPowerzone4ColorTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_4_color = presetPowerzone4ColorTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
            }
        }

        AccordionCheckElement {
            id: presetPowerZone5EnabledAccordion
            title: qsTr("Preset Power Zone 5")
            linkedBoolSetting: "tile_preset_powerzone_5_enabled"
            settings: settingsTilesPane.settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: presetPowerZone5OrderTextField
                        Accessible.name: presetPowerZone5EnabledAccordion.title + ", " + qsTr("order index:")
                        model: rootItem.tile_order
                        savedText: settings.tile_preset_powerzone_5_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetPowerZone5OrderTextField.currentValue
                        }
                    }
                    Button {
                        text: qsTr("OK")
                        enabled: presetPowerZone5OrderTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_5_order = presetPowerZone5OrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("zone value:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetPowerZone5Value
                        Accessible.name: presetPowerZone5EnabledAccordion.title + ", " + qsTr("zone value:")
                        savedText: settings.tile_preset_powerzone_5_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        validator: DoubleValidator {bottom: 1; top: 7;}
                    }
                    Button {
                        text: qsTr("OK")
                        enabled: presetPowerZone5Value.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_5_value = parseFloat(presetPowerZone5Value.text); toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("label:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetPowerZone5Label
                        Accessible.name: presetPowerZone5EnabledAccordion.title + ", " + qsTr("label:")
                        savedText: settings.tile_preset_powerzone_5_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    }
                    Button {
                        text: qsTr("OK")
                        enabled: presetPowerZone5Label.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_5_label = presetPowerZone5Label.text; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetPowerzone5Color
                        text: qsTr("color:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ColorDialog {
                        id: colorPresetPowerzone5
                        title: qsTr("Please choose a color")
                        onAccepted: {
                            presetPowerzone5ColorTextField.text = colorPresetPowerzone5.color
                        }
                        onRejected: {}
                    }
                    SettingsTextField {
                        id: presetPowerzone5ColorTextField
                        Accessible.name: presetPowerZone5EnabledAccordion.title + ", " + labelPresetPowerzone5Color.text
                        savedText: settings.tile_preset_powerzone_5_color
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        onPressed: {
                            if(OS_VERSION !== "Android") colorPresetPowerzone5.visible = true
                        }
                    }
                    Button {
                        id: okPresetPowerzone5ColorButton
                        text: qsTr("OK")
                        enabled: presetPowerzone5ColorTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_5_color = presetPowerzone5ColorTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
            }
        }

        AccordionCheckElement {
            id: presetPowerZone6EnabledAccordion
            title: qsTr("Preset Power Zone 6")
            linkedBoolSetting: "tile_preset_powerzone_6_enabled"
            settings: settingsTilesPane.settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: presetPowerZone6OrderTextField
                        Accessible.name: presetPowerZone6EnabledAccordion.title + ", " + qsTr("order index:")
                        model: rootItem.tile_order
                        savedText: settings.tile_preset_powerzone_6_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetPowerZone6OrderTextField.currentValue
                        }
                    }
                    Button {
                        text: qsTr("OK")
                        enabled: presetPowerZone6OrderTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_6_order = presetPowerZone6OrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("zone value:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetPowerZone6Value
                        Accessible.name: presetPowerZone6EnabledAccordion.title + ", " + qsTr("zone value:")
                        savedText: settings.tile_preset_powerzone_6_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        validator: DoubleValidator {bottom: 1; top: 7;}
                    }
                    Button {
                        text: qsTr("OK")
                        enabled: presetPowerZone6Value.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_6_value = parseFloat(presetPowerZone6Value.text); toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("label:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetPowerZone6Label
                        Accessible.name: presetPowerZone6EnabledAccordion.title + ", " + qsTr("label:")
                        savedText: settings.tile_preset_powerzone_6_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    }
                    Button {
                        text: qsTr("OK")
                        enabled: presetPowerZone6Label.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_6_label = presetPowerZone6Label.text; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetPowerzone6Color
                        text: qsTr("color:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ColorDialog {
                        id: colorPresetPowerzone6
                        title: qsTr("Please choose a color")
                        onAccepted: {
                            presetPowerzone6ColorTextField.text = colorPresetPowerzone6.color
                        }
                        onRejected: {}
                    }
                    SettingsTextField {
                        id: presetPowerzone6ColorTextField
                        Accessible.name: presetPowerZone6EnabledAccordion.title + ", " + labelPresetPowerzone6Color.text
                        savedText: settings.tile_preset_powerzone_6_color
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        onPressed: {
                            if(OS_VERSION !== "Android") colorPresetPowerzone6.visible = true
                        }
                    }
                    Button {
                        id: okPresetPowerzone6ColorButton
                        text: qsTr("OK")
                        enabled: presetPowerzone6ColorTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_6_color = presetPowerzone6ColorTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
            }
        }

        AccordionCheckElement {
            id: presetPowerZone7EnabledAccordion
            title: qsTr("Preset Power Zone 7")
            linkedBoolSetting: "tile_preset_powerzone_7_enabled"
            settings: settingsTilesPane.settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        text: qsTr("order index:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    UiComboBox {
                        id: presetPowerZone7OrderTextField
                        Accessible.name: presetPowerZone7EnabledAccordion.title + ", " + qsTr("order index:")
                        model: rootItem.tile_order
                        savedText: settings.tile_preset_powerzone_7_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetPowerZone7OrderTextField.currentValue
                        }
                    }
                    Button {
                        text: qsTr("OK")
                        enabled: presetPowerZone7OrderTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_7_order = presetPowerZone7OrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("zone value:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetPowerZone7Value
                        Accessible.name: presetPowerZone7EnabledAccordion.title + ", " + qsTr("zone value:")
                        savedText: settings.tile_preset_powerzone_7_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        validator: DoubleValidator {bottom: 1; top: 7;}
                    }
                    Button {
                        text: qsTr("OK")
                        enabled: presetPowerZone7Value.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_7_value = parseFloat(presetPowerZone7Value.text); toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("label:")
                        wrapMode: window.labelWrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    SettingsTextField {
                        id: presetPowerZone7Label
                        Accessible.name: presetPowerZone7EnabledAccordion.title + ", " + qsTr("label:")
                        savedText: settings.tile_preset_powerzone_7_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    }
                    Button {
                        text: qsTr("OK")
                        enabled: presetPowerZone7Label.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_7_label = presetPowerZone7Label.text; toast.show(qsTr("Setting saved!")); }
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetPowerzone7Color
                        text: qsTr("color:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ColorDialog {
                        id: colorPresetPowerzone7
                        title: qsTr("Please choose a color")
                        onAccepted: {
                            presetPowerzone7ColorTextField.text = colorPresetPowerzone7.color
                        }
                        onRejected: {}
                    }
                    SettingsTextField {
                        id: presetPowerzone7ColorTextField
                        Accessible.name: presetPowerZone7EnabledAccordion.title + ", " + labelPresetPowerzone7Color.text
                        savedText: settings.tile_preset_powerzone_7_color
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        onPressed: {
                            if(OS_VERSION !== "Android") colorPresetPowerzone7.visible = true
                        }
                    }
                    Button {
                        id: okPresetPowerzone7ColorButton
                        text: qsTr("OK")
                        enabled: presetPowerzone7ColorTextField.dirty
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_7_color = presetPowerzone7ColorTextField.displayText; toast.show(qsTr("Setting saved!")); }
                    }
                }
            }
        }

        Label {
            text: qsTr("Power zone presets allow quick access to specific training zones with customizable labels and values.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: hrTimeInZone1EnabledAccordion
            title: qsTr("Heart Rate Time in Zone 1+")
            linkedBoolSetting: "tile_hr_time_in_zone_1_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelHrTimeInZone1Order
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: hrTimeInZone1OrderTextField
                    Accessible.name: hrTimeInZone1EnabledAccordion.title + ", " + labelHrTimeInZone1Order.text
                    model: rootItem.tile_order
                    savedText: settings.tile_hr_time_in_zone_1_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = hrTimeInZone1OrderTextField.currentValue
                     }
                }
                Button {
                    id: okHrTimeInZone1OrderButton
                    text: qsTr("OK")
                    enabled: hrTimeInZone1OrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_hr_time_in_zone_1_order = hrTimeInZone1OrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Displays total time spent in heart rate Zone 1 or higher during the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: hrTimeInZone2EnabledAccordion
            title: qsTr("Heart Rate Time in Zone 2+")
            linkedBoolSetting: "tile_hr_time_in_zone_2_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelHrTimeInZone2Order
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: hrTimeInZone2OrderTextField
                    Accessible.name: hrTimeInZone2EnabledAccordion.title + ", " + labelHrTimeInZone2Order.text
                    model: rootItem.tile_order
                    savedText: settings.tile_hr_time_in_zone_2_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = hrTimeInZone2OrderTextField.currentValue
                     }
                }
                Button {
                    id: okHrTimeInZone2OrderButton
                    text: qsTr("OK")
                    enabled: hrTimeInZone2OrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_hr_time_in_zone_2_order = hrTimeInZone2OrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Displays total time spent in heart rate Zone 2 or higher during the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: hrTimeInZone3EnabledAccordion
            title: qsTr("Heart Rate Time in Zone 3+")
            linkedBoolSetting: "tile_hr_time_in_zone_3_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelHrTimeInZone3Order
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: hrTimeInZone3OrderTextField
                    Accessible.name: hrTimeInZone3EnabledAccordion.title + ", " + labelHrTimeInZone3Order.text
                    model: rootItem.tile_order
                    savedText: settings.tile_hr_time_in_zone_3_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = hrTimeInZone3OrderTextField.currentValue
                     }
                }
                Button {
                    id: okHrTimeInZone3OrderButton
                    text: qsTr("OK")
                    enabled: hrTimeInZone3OrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_hr_time_in_zone_3_order = hrTimeInZone3OrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Displays total time spent in heart rate Zone 3 or higher during the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: hrTimeInZone4EnabledAccordion
            title: qsTr("Heart Rate Time in Zone 4+")
            linkedBoolSetting: "tile_hr_time_in_zone_4_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelHrTimeInZone4Order
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: hrTimeInZone4OrderTextField
                    Accessible.name: hrTimeInZone4EnabledAccordion.title + ", " + labelHrTimeInZone4Order.text
                    model: rootItem.tile_order
                    savedText: settings.tile_hr_time_in_zone_4_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = hrTimeInZone4OrderTextField.currentValue
                     }
                }
                Button {
                    id: okHrTimeInZone4OrderButton
                    text: qsTr("OK")
                    enabled: hrTimeInZone4OrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_hr_time_in_zone_4_order = hrTimeInZone4OrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Displays total time spent in heart rate Zone 4 or higher during the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: hrTimeInZone5EnabledAccordion
            title: qsTr("Heart Rate Time in Zone 5+")
            linkedBoolSetting: "tile_hr_time_in_zone_5_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelHrTimeInZone5Order
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: hrTimeInZone5OrderTextField
                    Accessible.name: hrTimeInZone5EnabledAccordion.title + ", " + labelHrTimeInZone5Order.text
                    model: rootItem.tile_order
                    savedText: settings.tile_hr_time_in_zone_5_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = hrTimeInZone5OrderTextField.currentValue
                     }
                }
                Button {
                    id: okHrTimeInZone5OrderButton
                    text: qsTr("OK")
                    enabled: hrTimeInZone5OrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_hr_time_in_zone_5_order = hrTimeInZone5OrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Displays total time spent in heart rate Zone 5 or higher during the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        RowLayout {
            spacing: 10
            Layout.fillWidth: true

            CheckBox {
                id: hrTimeInZoneIndividualModeCheckBox
                text: qsTr("Show individual zone times (instead of cumulative)")
                Layout.fillWidth: true
                checked: settings.tile_hr_time_in_zone_individual_mode
                onClicked: {
                    settings.tile_hr_time_in_zone_individual_mode = checked
                    toast.show(qsTr("Setting saved!"))
                }
            }
        }

        Label {
            text: qsTr("When enabled, each zone shows only the time spent in that specific zone. When disabled (default), each zone shows cumulative time spent in that zone or higher.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Orange)
        }

        AccordionCheckElement {
            id: coreTemperatureAccordion
            title: qsTr("Core Temperature")
            linkedBoolSetting: "tile_coretemperature_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelcoretemperatureOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: coretemperatureOrderTextField
                    Accessible.name: coreTemperatureAccordion.title + ", " + labelcoretemperatureOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_coretemperature_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = coretemperatureOrderTextField.currentValue
                     }
                }
                Button {
                    id: okcoretemperatureOrderButton
                    text: qsTr("OK")
                    enabled: coretemperatureOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_coretemperature_order = coretemperatureOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Shows Core, Body Temperature and Heat Strain Index from a Core Temperature sensor.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: heatTimeInZone1EnabledAccordion
            title: qsTr("Heat Time in Zone 1")
            linkedBoolSetting: "tile_heat_time_in_zone_1_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelHeatTimeInZone1Order
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: heatTimeInZone1OrderTextField
                    Accessible.name: heatTimeInZone1EnabledAccordion.title + ", " + labelHeatTimeInZone1Order.text
                    model: rootItem.tile_order
                    savedText: settings.tile_heat_time_in_zone_1_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = heatTimeInZone1OrderTextField.currentValue
                     }
                }
                Button {
                    id: okHeatTimeInZone1OrderButton
                    text: qsTr("OK")
                    enabled: heatTimeInZone1OrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_heat_time_in_zone_1_order = heatTimeInZone1OrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Displays total time spent in heat Zone 1 during the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: heatTimeInZone2EnabledAccordion
            title: qsTr("Heat Time in Zone 2")
            linkedBoolSetting: "tile_heat_time_in_zone_2_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelHeatTimeInZone2Order
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: heatTimeInZone2OrderTextField
                    Accessible.name: heatTimeInZone2EnabledAccordion.title + ", " + labelHeatTimeInZone2Order.text
                    model: rootItem.tile_order
                    savedText: settings.tile_heat_time_in_zone_2_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = heatTimeInZone2OrderTextField.currentValue
                     }
                }
                Button {
                    id: okHeatTimeInZone2OrderButton
                    text: qsTr("OK")
                    enabled: heatTimeInZone2OrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_heat_time_in_zone_2_order = heatTimeInZone2OrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Displays total time spent in heat Zone 2 during the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: heatTimeInZone3EnabledAccordion
            title: qsTr("Heat Time in Zone 3")
            linkedBoolSetting: "tile_heat_time_in_zone_3_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelHeatTimeInZone3Order
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: heatTimeInZone3OrderTextField
                    Accessible.name: heatTimeInZone3EnabledAccordion.title + ", " + labelHeatTimeInZone3Order.text
                    model: rootItem.tile_order
                    savedText: settings.tile_heat_time_in_zone_3_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = heatTimeInZone3OrderTextField.currentValue
                     }
                }
                Button {
                    id: okHeatTimeInZone3OrderButton
                    text: qsTr("OK")
                    enabled: heatTimeInZone3OrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_heat_time_in_zone_3_order = heatTimeInZone3OrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Displays total time spent in heat Zone 3 during the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: heatTimeInZone4EnabledAccordion
            title: qsTr("Heat Time in Zone 4")
            linkedBoolSetting: "tile_heat_time_in_zone_4_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelHeatTimeInZone4Order
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: heatTimeInZone4OrderTextField
                    Accessible.name: heatTimeInZone4EnabledAccordion.title + ", " + labelHeatTimeInZone4Order.text
                    model: rootItem.tile_order
                    savedText: settings.tile_heat_time_in_zone_4_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = heatTimeInZone4OrderTextField.currentValue
                     }
                }
                Button {
                    id: okHeatTimeInZone4OrderButton
                    text: qsTr("OK")
                    enabled: heatTimeInZone4OrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_heat_time_in_zone_4_order = heatTimeInZone4OrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Displays total time spent in heat Zone 4 during the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: autoVirtualShiftingCruiseEnabledAccordion
            title: qsTr("Auto Virtual Shifting Cruise")
            linkedBoolSetting: "tile_auto_virtual_shifting_cruise_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: autoVirtualShiftingCruiseOrderTextField
                    Accessible.name: autoVirtualShiftingCruiseEnabledAccordion.title + ", " + qsTr("order index:")
                    model: rootItem.tile_order
                    savedText: settings.tile_auto_virtual_shifting_cruise_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = autoVirtualShiftingCruiseOrderTextField.currentValue
                     }
                }
                Button {
                    text: qsTr("OK")
                    enabled: autoVirtualShiftingCruiseOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_auto_virtual_shifting_cruise_order = autoVirtualShiftingCruiseOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Button tile to switch automatic virtual shifting to Cruise profile.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: autoVirtualShiftingClimbEnabledAccordion
            title: qsTr("Auto Virtual Shifting Climb")
            linkedBoolSetting: "tile_auto_virtual_shifting_climb_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: autoVirtualShiftingClimbOrderTextField
                    Accessible.name: autoVirtualShiftingClimbEnabledAccordion.title + ", " + qsTr("order index:")
                    model: rootItem.tile_order
                    savedText: settings.tile_auto_virtual_shifting_climb_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = autoVirtualShiftingClimbOrderTextField.currentValue
                     }
                }
                Button {
                    text: qsTr("OK")
                    enabled: autoVirtualShiftingClimbOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_auto_virtual_shifting_climb_order = autoVirtualShiftingClimbOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Button tile to switch automatic virtual shifting to Climb profile.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: autoVirtualShiftingSprintEnabledAccordion
            title: qsTr("Auto Virtual Shifting Sprint")
            linkedBoolSetting: "tile_auto_virtual_shifting_sprint_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: autoVirtualShiftingSprintOrderTextField
                    Accessible.name: autoVirtualShiftingSprintEnabledAccordion.title + ", " + qsTr("order index:")
                    model: rootItem.tile_order
                    savedText: settings.tile_auto_virtual_shifting_sprint_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = autoVirtualShiftingSprintOrderTextField.currentValue
                     }
                }
                Button {
                    text: qsTr("OK")
                    enabled: autoVirtualShiftingSprintOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_auto_virtual_shifting_sprint_order = autoVirtualShiftingSprintOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Button tile to switch automatic virtual shifting to Sprint profile.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: powerAvgEnabledAccordion
            title: qsTr("Power Averaging")
            linkedBoolSetting: "tile_power_avg_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: powerAvgOrderTextField
                    Accessible.name: powerAvgEnabledAccordion.title + ", " + qsTr("order index:")
                    model: rootItem.tile_order
                    savedText: settings.tile_power_avg_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = powerAvgOrderTextField.currentValue
                     }
                }
                Button {
                    text: qsTr("OK")
                    enabled: powerAvgOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_power_avg_order = powerAvgOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Button tile to cycle through power averaging modes: Off, 3s avg (harmonic), 5s avg (harmonic). Tap to cycle between modes. Only for bikes.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }

        AccordionCheckElement {
            id: hrvEnabledAccordion
            title: qsTr("HRV (Heart Rate Variability)")
            linkedBoolSetting: "tile_hrv_enabled"
            settings: settingsTilesPane.settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelhrvOrder
                    text: qsTr("order index:")
                    wrapMode: window.labelWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                UiComboBox {
                    id: hrvOrderTextField
                    Accessible.name: hrvEnabledAccordion.title + ", " + labelhrvOrder.text
                    model: rootItem.tile_order
                    savedText: settings.tile_hrv_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = hrvOrderTextField.currentValue
                     }
                }
                Button {
                    id: okhrvOrderButton
                    text: qsTr("OK")
                    enabled: hrvOrderTextField.dirty
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_hrv_order = hrvOrderTextField.displayText; toast.show(qsTr("Setting saved!")); }
                }
            }
        }

        Label {
            text: qsTr("Shows Heart Rate Variability (HRV) from a compatible heart rate belt. Displays RMSSD value in milliseconds.")
            font.bold: true
            font.italic: true
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }
    }
}
