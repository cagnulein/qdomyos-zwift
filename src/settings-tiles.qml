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
    //anchors.bottom: footerSettings.top
    //anchors.bottomMargin: footerSettings.height + 10
    id: settingsTilesPane

    Settings {
        id: settings
        property bool tile_speed_enabled: true
        property int  tile_speed_order: 0
        property bool tile_inclination_enabled: true
        property int  tile_inclination_order: 1
        property bool tile_cadence_enabled: true
        property int  tile_cadence_order: 2
        property bool tile_elevation_enabled: true
        property int  tile_elevation_order: 3
        property bool tile_calories_enabled: true
        property int  tile_calories_order: 4
        property bool tile_odometer_enabled: true
        property int  tile_odometer_order: 5
        property bool tile_pace_enabled: true
        property int  tile_pace_order: 6
        property bool tile_resistance_enabled: true
        property int  tile_resistance_order: 7
        property bool tile_watt_enabled: true
        property int  tile_watt_order: 8
        property bool tile_weight_loss_enabled: false
        property int  tile_weight_loss_order: 24
        property bool tile_avgwatt_enabled: true
        property int  tile_avgwatt_order: 9
        property bool tile_ftp_enabled: true
        property int  tile_ftp_order: 10
        property bool tile_heart_enabled: true
        property int  tile_heart_order: 11
        property bool tile_fan_enabled: true
        property int  tile_fan_order: 12
        property bool tile_jouls_enabled: true
        property int  tile_jouls_order: 13
        property bool tile_elapsed_enabled: true
        property int  tile_elapsed_order: 14
        property bool tile_lapelapsed_enabled: false
        property int  tile_lapelapsed_order: 17
        property bool tile_moving_time_enabled: false
        property int  tile_moving_time_order: 21
        property bool tile_peloton_offset_enabled: false
        property int  tile_peloton_offset_order: 22
        property bool tile_peloton_difficulty_enabled: false
        property int  tile_peloton_difficulty_order: 32
        property bool tile_peloton_resistance_enabled: true
        property int  tile_peloton_resistance_order: 15
        property bool tile_datetime_enabled: true
        property int  tile_datetime_order: 16
        property bool tile_target_resistance_enabled: true
        property int  tile_target_resistance_order: 15
        property bool tile_target_peloton_resistance_enabled: false
        property int  tile_target_peloton_resistance_order: 21
        property bool tile_target_cadence_enabled: false
        property int  tile_target_cadence_order: 19
        property bool tile_target_power_enabled: false
        property int  tile_target_power_order: 20
        property bool tile_target_zone_enabled: false
        property int  tile_target_zone_order: 24
        property bool tile_target_speed_enabled: false
        property int  tile_target_speed_order: 27
        property bool tile_target_incline_enabled: false
        property int  tile_target_incline_order: 28
        property bool tile_strokes_count_enabled: false
        property int  tile_strokes_count_order: 22
        property bool tile_strokes_length_enabled: false
        property int  tile_strokes_length_order: 23
        property bool tile_watt_kg_enabled: false
        property int  tile_watt_kg_order: 25
        property bool tile_gears_enabled: false
        property int  tile_gears_order: 26
        property bool tile_remainingtimetrainprogramrow_enabled: false
        property int  tile_remainingtimetrainprogramrow_order: 27
        property bool tile_nextrowstrainprogram_enabled: false
        property int  tile_nextrowstrainprogram_order: 31
        property bool tile_mets_enabled: false
        property int  tile_mets_order: 28
        property bool tile_targetmets_enabled: false
        property int  tile_targetmets_order: 29
        property bool tile_steering_angle_enabled: false
        property int  tile_steering_angle_order: 30
        property bool tile_pid_hr_enabled: false
        property int  tile_pid_hr_order: 31
        property bool tile_ext_incline_enabled: false
        property int  tile_ext_incline_order: 32
        property bool tile_cadence_color_enabled: false
        property bool tile_peloton_remaining_enabled: false
        property int  tile_peloton_remaining_order: 22
        property bool tile_peloton_resistance_color_enabled: false
        property bool tile_instantaneous_stride_length_enabled: false
        property int  tile_instantaneous_stride_length_order: 32
        property bool tile_ground_contact_enabled: false
        property int  tile_ground_contact_order: 33
        property bool tile_vertical_oscillation_enabled: false
        property int  tile_vertical_oscillation_order: 34
        property bool tile_preset_resistance_1_enabled: false
        property int tile_preset_resistance_1_order: 33
        property real tile_preset_resistance_1_value: 1.0
        property string tile_preset_resistance_1_label: "Res. 1"
        property bool tile_preset_resistance_2_enabled: false
        property int tile_preset_resistance_2_order: 34
        property real tile_preset_resistance_2_value: 10.0
        property string tile_preset_resistance_2_label: "Res. 10"
        property bool tile_preset_resistance_3_enabled: false
        property int tile_preset_resistance_3_order: 35
        property real tile_preset_resistance_3_value: 20.0
        property string tile_preset_resistance_3_label: "Res. 20"
        property bool tile_preset_resistance_4_enabled: false
        property int tile_preset_resistance_4_order: 36
        property real tile_preset_resistance_4_value: 25.0
        property string tile_preset_resistance_4_label: "Res. 25"
        property bool tile_preset_resistance_5_enabled: false
        property int tile_preset_resistance_5_order: 37
        property real tile_preset_resistance_5_value: 30.0
        property string tile_preset_resistance_5_label: "Res. 30"
        property bool tile_preset_speed_1_enabled: false
        property int tile_preset_speed_1_order: 38
        property real tile_preset_speed_1_value: 5.0
        property string tile_preset_speed_1_label: "5 km/h"
        property bool tile_preset_speed_2_enabled: false
        property int tile_preset_speed_2_order: 39
        property real tile_preset_speed_2_value: 7.0
        property string tile_preset_speed_2_label: "7 km/h"
        property bool tile_preset_speed_3_enabled: false
        property int tile_preset_speed_3_order: 40
        property real tile_preset_speed_3_value: 10.0
        property string tile_preset_speed_3_label: "10 km/h"
        property bool tile_preset_speed_4_enabled: false
        property int tile_preset_speed_4_order: 41
        property real tile_preset_speed_4_value: 11.0
        property string tile_preset_speed_4_label: "11 km/h"
        property bool tile_preset_speed_5_enabled: false
        property int tile_preset_speed_5_order: 42
        property real tile_preset_speed_5_value: 12.0
        property string tile_preset_speed_5_label: "12 km/h"
        property bool tile_preset_inclination_1_enabled: false
        property int tile_preset_inclination_1_order: 43
        property real tile_preset_inclination_1_value: 0.0
        property string tile_preset_inclination_1_label: "0%"
        property bool tile_preset_inclination_2_enabled: false
        property int tile_preset_inclination_2_order: 44
        property real tile_preset_inclination_2_value: 1.0
        property string tile_preset_inclination_2_label: "1%"
        property bool tile_preset_inclination_3_enabled: false
        property int tile_preset_inclination_3_order: 45
        property real tile_preset_inclination_3_value: 2.0
        property string tile_preset_inclination_3_label: "2%"
        property bool tile_preset_inclination_4_enabled: false
        property int tile_preset_inclination_4_order: 46
        property real tile_preset_inclination_4_value: 3.0
        property string tile_preset_inclination_4_label: "3%"
        property bool tile_preset_inclination_5_enabled: false
        property int tile_preset_inclination_5_order: 47
        property real tile_preset_inclination_5_value: 4.0
        property string tile_preset_inclination_5_label: "4%"
    }


    ColumnLayout {
        spacing: 0
        anchors.fill: parent

        AccordionCheckElement {
            id: speedEnabledAccordion
            title: qsTr("Speed")
            linkedBoolSetting: "tile_speed_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelSpeedOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: speedOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_speed_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = speedOrderTextField.currentValue
                     }
                }
                Button {
                    id: okSpeedOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_speed_order = speedOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: inclinationEnabledAccordion
            title: qsTr("Inclination")
            linkedBoolSetting: "tile_inclination_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelinclinationOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: inclinationOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_inclination_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = inclinationOrderTextField.currentValue
                     }
                }
                Button {
                    id: okinclinationOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_inclination_order = inclinationOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: cadenceEnabledAccordion
            title: qsTr("Cadence")
            linkedBoolSetting: "tile_cadence_enabled"
            settings: settings
            accordionContent:  ColumnLayout {
                SwitchDelegate {
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
                RowLayout {
                    spacing: 10
                    Label {
                        id: labelcadenceOrder
                        text: qsTr("order index:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ComboBox {
                        id: cadenceOrderTextField
                        model: rootItem.tile_order
                        displayText: settings.tile_cadence_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = cadenceOrderTextField.currentValue
                         }
                    }
                    Button {
                        id: okcadenceOrderButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_cadence_order = cadenceOrderTextField.displayText
                    }
                }
            }
        }

        AccordionCheckElement {
            id: elevationEnabledAccordion
            title: qsTr("Elevation")
            linkedBoolSetting: "tile_elevation_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelelevationOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: elevationOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_elevation_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = elevationOrderTextField.currentValue
                     }
                }
                Button {
                    id: okelevationOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_elevation_order = elevationOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: caloriesEnabledAccordion
            title: qsTr("Calories")
            linkedBoolSetting: "tile_calories_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelcaloriesOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: caloriesOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_calories_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = caloriesOrderTextField.currentValue
                     }
                }
                Button {
                    id: okcaloriesOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_calories_order = caloriesOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: odometerEnabledAccordion
            title: qsTr("Odometer")
            linkedBoolSetting: "tile_odometer_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelodometerOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: odometerOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_odometer_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = odometerOrderTextField.currentValue
                     }
                }
                Button {
                    id: okodometerOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_odometer_order = odometerOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: paceEnabledAccordion
            title: qsTr("Pace")
            linkedBoolSetting: "tile_pace_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelpaceOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: paceOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_pace_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = paceOrderTextField.currentValue
                     }
                }
                Button {
                    id: okpaceOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_pace_order = paceOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: resistanceEnabledAccordion
            title: qsTr("Resistance")
            linkedBoolSetting: "tile_resistance_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelresistanceOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: resistanceOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_resistance_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = resistanceOrderTextField.currentValue
                     }
                }
                Button {
                    id: okresistanceOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_resistance_order = resistanceOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: wattEnabledAccordion
            title: qsTr("Watt")
            linkedBoolSetting: "tile_watt_enabled"
            settings: settings
            accordionContent:  RowLayout {
                spacing: 10
                Label {
                    id: labelwattOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: wattOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_watt_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = wattOrderTextField.currentValue
                     }
                }
                Button {
                    id: okwattOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_watt_order = wattOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: weightLossEnabledAccordion
            title: qsTr("Weight loss")
            linkedBoolSetting: "tile_weight_loss_enabled"
            settings: settings
            accordionContent:  RowLayout {
                spacing: 10
                Label {
                    id: labelweightLossOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: weightLossOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_weight_loss_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = weightLossOrderTextField.currentValue
                     }
                }
                Button {
                    id: okweightLossOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_weight_loss_order = weightLossOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: avgwattEnabledAccordion
            title: qsTr("AVG Watt")
            linkedBoolSetting: "tile_avgwatt_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelavgwattOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: avgwattOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_avgwatt_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = avgwattOrderTextField.currentValue
                     }
                }
                Button {
                    id: okavgwattOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_avgwatt_order = avgwattOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: ftpEnabledAccordion
            title: qsTr("FTP %")
            linkedBoolSetting: "tile_ftp_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelftpOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: ftpOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_ftp_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = ftpOrderTextField.currentValue
                     }
                }
                Button {
                    id: okftpOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_ftp_order = ftpOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: heartEnabledAccordion
            title: qsTr("Heart")
            linkedBoolSetting: "tile_heart_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelheartrateOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: heartrateOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_heart_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = heartrateOrderTextField.currentValue
                     }
                }
                Button {
                    id: okheartrateOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_heart_order = heartrateOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: fanEnabledAccordion
            title: qsTr("Fan")
            linkedBoolSetting: "tile_fan_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelfanOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: fanOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_fan_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = fanOrderTextField.currentValue
                     }
                }
                Button {
                    id: okfanOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_fan_order = fanOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: joulsEnabledAccordion
            title: qsTr("Jouls")
            linkedBoolSetting: "tile_jouls_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labeljoulsOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: joulsOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_jouls_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = joulsOrderTextField.currentValue
                     }
                }
                Button {
                    id: okjoulsOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_jouls_order = joulsOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: elapsedEnabledAccordion
            title: qsTr("Elapsed")
            linkedBoolSetting: "tile_elapsed_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelelapsedOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: elapsedOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_elapsed_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = elapsedOrderTextField.currentValue
                     }
                }
                Button {
                    id: okelapsedOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_elapsed_order = elapsedOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: movingTimeEnabledAccordion
            title: qsTr("Moving Time")
            linkedBoolSetting: "tile_moving_time_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelmovingTimeOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: movingTimeOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_moving_time_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = movingTimeOrderTextField.currentValue
                     }
                }
                Button {
                    id: okmovingTimeOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_moving_time_order = movingTimeOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: pelotonOffsetEnabledAccordion
            title: qsTr("Peloton Offset")
            linkedBoolSetting: "tile_peloton_offset_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelpelotonOffsetOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: pelotonOffsetOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_peloton_offset_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = pelotonOffsetOrderTextField.currentValue
                     }
                }
                Button {
                    id: okpelotonOffsetOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_peloton_offset_order = pelotonOffsetOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: pelotonRemainingEnabledAccordion
            title: qsTr("Peloton Remaining")
            linkedBoolSetting: "tile_peloton_remaining_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelPelotonRemainingOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: pelotonRemainingOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_peloton_remaining_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = pelotonRemainingOrderTextField.currentValue
                     }
                }
                Button {
                    id: okPelotonRemainingOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_peloton_remaining_order = pelotonRemainingOrderTextField.displayText
                }
            }
        }

        /*
        AccordionCheckElement {
            id: pelotonDifficultyEnabledAccordion
            title: qsTr("Peloton Difficulty")
            linkedBoolSetting: "tile_peloton_difficulty_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelpelotonDifficultyOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: pelotonDifficultyOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_peloton_difficulty_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = pelotonDifficultyOrderTextField.currentValue
                     }
                }
                Button {
                    id: okpelotonDifficultyOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_peloton_difficulty_order = pelotonDifficultyOrderTextField.displayText
                }
            }
        }*/

        AccordionCheckElement {
            id: lapElapsedEnabledAccordion
            title: qsTr("Lap Elapsed")
            linkedBoolSetting: "tile_lapelapsed_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labellapElapsedOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: lapElapsedOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_lapelapsed_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = lapElapsedOrderTextField.currentValue
                     }
                }
                Button {
                    id: oklapElapsedOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_lapelapsed_order = lapElapsedOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: pelotonResistanceEnabledAccordion
            title: qsTr("Peloton Resistance")
            linkedBoolSetting: "tile_peloton_resistance_enabled"
            settings: settings
            accordionContent: ColumnLayout {
                SwitchDelegate {
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
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ComboBox {
                        id: peloton_resistanceOrderTextField
                        model: rootItem.tile_order
                        displayText: settings.tile_peloton_resistance_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = peloton_resistanceOrderTextField.currentValue
                         }
                    }
                    Button {
                        id: okpeloton_resistanceOrderButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_peloton_resistance_order = peloton_resistanceOrderTextField.displayText
                    }
                }
            }
        }

        AccordionCheckElement {
            id: targetResistanceEnabledAccordion
            title: qsTr("Target Resistance")
            linkedBoolSetting: "tile_target_resistance_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labeltarget_resistanceOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: target_resistanceOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_target_resistance_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = target_resistanceOrderTextField.currentValue
                     }
                }
                Button {
                    id: oktarget_resistanceOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_target_resistance_order = target_resistanceOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: targetPelotonResistanceEnabledAccordion
            title: qsTr("Target Peloton Resistance")
            linkedBoolSetting: "tile_target_peloton_resistance_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labeltarget_peloton_resistanceOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: target_peloton_resistanceOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_target_peloton_resistance_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = target_peloton_resistanceOrderTextField.currentValue
                     }
                }
                Button {
                    id: oktarget_peloton_resistanceOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_target_peloton_resistance_order = target_peloton_resistanceOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: targetCadenceEnabledAccordion
            title: qsTr("Target Cadence")
            linkedBoolSetting: "tile_target_cadence_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labeltarget_cadenceOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: target_cadenceOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_target_cadence_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = target_cadenceOrderTextField.currentValue
                     }
                }
                Button {
                    id: oktarget_cadenceOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_target_cadence_order = target_cadenceOrderTextField.displayText
                }
            }
        }
        AccordionCheckElement {
            id: targetPowerEnabledAccordion
            title: qsTr("Target Power")
            linkedBoolSetting: "tile_target_power_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labeltarget_powerOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: target_powerOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_target_power_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = target_powerOrderTextField.currentValue
                     }
                }
                Button {
                    id: oktarget_powerOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_target_power_order = target_powerOrderTextField.displayText
                }
            }
        }
        AccordionCheckElement {
            id: targetZoneEnabledAccordion
            title: qsTr("Target Power Zone")
            linkedBoolSetting: "tile_target_zone_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labeltarget_zoneOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: target_zoneOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_target_zone_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = target_zoneOrderTextField.currentValue
                     }
                }
                Button {
                    id: oktarget_zoneOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_target_zone_order = target_zoneOrderTextField.displayText
                }
            }
        }
        AccordionCheckElement {
            id: targetSpeedEnabledAccordion
            title: qsTr("Target Speed")
            linkedBoolSetting: "tile_target_speed_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labeltargetspeedOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: target_speedOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_target_speed_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = target_speedOrderTextField.currentValue
                     }
                }
                Button {
                    id: oktarget_speedOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_target_speed_order = target_speedOrderTextField.displayText
                }
            }
        }
        AccordionCheckElement {
            id: targetInclineEnabledAccordion
            title: qsTr("Target Incline")
            linkedBoolSetting: "tile_target_incline_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labeltarget_inclineOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: target_inclineOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_target_incline_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = target_inclineOrderTextField.currentValue
                     }
                }
                Button {
                    id: oktarget_inclineOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_target_incline_order = target_inclineOrderTextField.displayText
                }
            }
        }
        AccordionCheckElement {
            id: wattKgEnabledAccordion
            title: qsTr("Watt/Kg")
            linkedBoolSetting: "tile_watt_kg_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelwatt_kgOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: watt_kgOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_watt_kg_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = watt_kgOrderTextField.currentValue
                     }
                }
                Button {
                    id: okwatt_kgOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_watt_kg_order = watt_kgOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: gearsEnabledAccordion
            title: qsTr("Gears")
            linkedBoolSetting: "tile_gears_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelgearsOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: gearsOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_gears_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = gearsOrderTextField.currentValue
                     }
                }
                Button {
                    id: okgearsOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_gears_order = gearsOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: remainingTimeTrainingProgramRowEnabledAccordion
            title: qsTr("Remaining Time/Row")
            linkedBoolSetting: "tile_remainingtimetrainprogramrow_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelremainingTimeTrainingProgramRowOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: remainingTimeTrainingProgramRowOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_remainingtimetrainprogramrow_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = remainingTimeTrainingProgramRowOrderTextField.currentValue
                     }
                }
                Button {
                    id: okremainingTimeTrainingProgramRowOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_remainingtimetrainprogramrow_order = remainingTimeTrainingProgramRowOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: nextRowsTrainingProgramRowEnabledAccordion
            title: qsTr("Next Rows")
            linkedBoolSetting: "tile_nextrowstrainprogram_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelnextRowsTrainingProgramOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: nextRowsTrainingProgramOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_nextrowstrainprogram_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = nextRowsTrainingProgramOrderTextField.currentValue
                     }
                }
                Button {
                    id: oknextRowsTrainingProgramOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_nextrowstrainprogram_order = nextRowsTrainingProgramOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: metsEnabledAccordion
            title: qsTr("METS")
            linkedBoolSetting: "tile_mets_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelmetsOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: metsOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_mets_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = metsOrderTextField.currentValue
                     }
                }
                Button {
                    id: okmetsOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_mets_order = metsOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: targetMetsEnabledAccordion
            title: qsTr("Target METS")
            linkedBoolSetting: "tile_targetmets_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labeltargetmetsOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: targetmetsOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_targetmets_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = targetmetsOrderTextField.currentValue
                     }
                }
                Button {
                    id: oktargetmetsOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_targetmets_order = targetmetsOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: datetimeEnabledAccordion
            title: qsTr("Time")
            linkedBoolSetting: "tile_datetime_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labeldatetimeOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: datetimeOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_datetime_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = datetimeOrderTextField.currentValue
                     }
                }
                Button {
                    id: okdatetimeOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_datetime_order = datetimeOrderTextField.displayText
                }
            }
        }
        AccordionCheckElement {
            id: targetStrokesCountAccordion
            title: qsTr("Strokes Count")
            linkedBoolSetting: "tile_strokes_count_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelstrokes_countOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: strokes_countOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_strokes_count_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = strokes_countOrderTextField.currentValue
                     }
                }
                Button {
                    id: okstrokes_countOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_strokes_count_order = strokes_countOrderTextField.displayText
                }
            }
        }
        AccordionCheckElement {
            id: targetStrokesLengthAccordion
            title: qsTr("Strokes Length")
            linkedBoolSetting: "tile_strokes_length_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelstrokes_lengthOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: strokes_lengthOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_strokes_length_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = strokes_lengthOrderTextField.currentValue
                     }
                }
                Button {
                    id: okstrokes_lengthOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_strokes_length_order = strokes_lengthOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: targetSteeringAngleEnabledAccordion
            title: qsTr("Steering Angle")
            linkedBoolSetting: "tile_steering_angle_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelsteeringAngleOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: steeringAngleOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_steering_angle_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = steeringAngleOrderTextField.currentValue
                     }
                }
                Button {
                    id: oksteeringAngleOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_steering_angle_order = steeringAngleOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: targetPIDHrAccordion
            title: qsTr("PID HR Zone")
            linkedBoolSetting: "tile_pid_hr_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelPIDHROrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: pidHROrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_pid_hr_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = pidHROrderTextField.currentValue
                     }
                }
                Button {
                    id: okpidHROrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_pid_hr_order = pidHROrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: extInclineAccordion
            title: qsTr("External Incline")
            linkedBoolSetting: "tile_ext_incline_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelExtInclineOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: extInclineOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_ext_incline_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = extInclineOrderTextField.currentValue
                     }
                }
                Button {
                    id: okextInclineOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_ext_incline_order = extInclineOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: strideLength
            title: qsTr("Stride Length")
            linkedBoolSetting: "tile_instantaneous_stride_length_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelStrideLengthOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: strideLengthOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_instantaneous_stride_length_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = strideLengthOrderTextField.currentValue
                     }
                }
                Button {
                    id: okStrideLengthOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_instantaneous_stride_length_order = strideLengthOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: groundContact
            title: qsTr("Ground Contact")
            linkedBoolSetting: "tile_ground_contact_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelGroundContactOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: groundContactOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_ground_contact_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = groundContactOrderTextField.currentValue
                     }
                }
                Button {
                    id: okGroundContactOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_ground_contact_order = groundContactOrderTextField.displayText
                }
            }
        }

        AccordionCheckElement {
            id: verticalOscillation
            title: qsTr("Vertical Oscillation")
            linkedBoolSetting: "tile_vertical_oscillation_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelVerticalOscillationOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: verticalOscillationOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_vertical_oscillation_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = verticalOscillationOrderTextField.currentValue
                     }
                }
                Button {
                    id: okVerticalOscillationOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: settings.tile_vertical_oscillation_order = verticalOscillationOrderTextField.displayText
                }
            }
        }
        AccordionCheckElement {
            id: presetResistance1EnabledAccordion
            title: qsTr("Preset Resistance 1")
            linkedBoolSetting: "tile_preset_resistance_1_enabled"
            settings: settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetResistance1Order
                        text: qsTr("order index:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ComboBox {
                        id: presetResistance1TextField
                        model: rootItem.tile_order
                        displayText: settings.tile_preset_resistance_1_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetResistance1TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetResistance1OrderButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_resistance_1_order = presetResistance1TextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetResistance1Value
                        text: qsTr("value:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetResistance1ValueTextField
                        text: settings.tile_preset_resistance_1_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetResistance1ValueButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_resistance_1_value = presetResistance1ValueTextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetResistance1Label
                        text: qsTr("label:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetResistance1LabelTextField
                        text: settings.tile_preset_resistance_1_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetResistance1LabelButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_resistance_1_label = presetResistance1LabelTextField.displayText
                    }
                }
            }
        }
        AccordionCheckElement {
            id: presetResistance2EnabledAccordion
            title: qsTr("Preset Resistance 2")
            linkedBoolSetting: "tile_preset_resistance_2_enabled"
            settings: settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetResistance2Order
                        text: qsTr("order index:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ComboBox {
                        id: presetResistance2TextField
                        model: rootItem.tile_order
                        displayText: settings.tile_preset_resistance_2_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetResistance2TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetResistance2OrderButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_resistance_2_order = presetResistance2TextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetResistance2Value
                        text: qsTr("value:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetResistance2ValueTextField
                        text: settings.tile_preset_resistance_2_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetResistance2ValueButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_resistance_2_value = presetResistance2ValueTextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetResistance2Label
                        text: qsTr("label:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetResistance2LabelTextField
                        text: settings.tile_preset_resistance_2_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetResistance2LabelButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_resistance_2_label = presetResistance2LabelTextField.displayText
                    }
                }
            }
        }
        AccordionCheckElement {
            id: presetResistance3EnabledAccordion
            title: qsTr("Preset Resistance 3")
            linkedBoolSetting: "tile_preset_resistance_3_enabled"
            settings: settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetResistance3Order
                        text: qsTr("order index:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ComboBox {
                        id: presetResistance3TextField
                        model: rootItem.tile_order
                        displayText: settings.tile_preset_resistance_3_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetResistance3TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetResistance3OrderButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_resistance_3_order = presetResistance3TextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetResistance3Value
                        text: qsTr("value:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetResistance3ValueTextField
                        text: settings.tile_preset_resistance_3_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetResistance3ValueButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_resistance_3_value = presetResistance3ValueTextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetResistance3Label
                        text: qsTr("label:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetResistance3LabelTextField
                        text: settings.tile_preset_resistance_3_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetResistance3LabelButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_resistance_3_label = presetResistance3LabelTextField.displayText
                    }
                }
            }
        }
        AccordionCheckElement {
            id: presetResistance4EnabledAccordion
            title: qsTr("Preset Resistance 4")
            linkedBoolSetting: "tile_preset_resistance_4_enabled"
            settings: settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetResistance4Order
                        text: qsTr("order index:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ComboBox {
                        id: presetResistance4TextField
                        model: rootItem.tile_order
                        displayText: settings.tile_preset_resistance_4_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetResistance4TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetResistance4OrderButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_resistance_4_order = presetResistance4TextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetResistance4Value
                        text: qsTr("value:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetResistance4ValueTextField
                        text: settings.tile_preset_resistance_4_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetResistance4ValueButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_resistance_4_value = presetResistance4ValueTextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetResistance4Label
                        text: qsTr("label:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetResistance4LabelTextField
                        text: settings.tile_preset_resistance_4_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetResistance4LabelButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_resistance_4_label = presetResistance4LabelTextField.displayText
                    }
                }
            }
        }
        AccordionCheckElement {
            id: presetResistance5EnabledAccordion
            title: qsTr("Preset Resistance 5")
            linkedBoolSetting: "tile_preset_resistance_5_enabled"
            settings: settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetResistance5Order
                        text: qsTr("order index:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ComboBox {
                        id: presetResistance5TextField
                        model: rootItem.tile_order
                        displayText: settings.tile_preset_resistance_5_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetResistance5TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetResistance5OrderButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_resistance_5_order = presetResistance5TextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetResistance5Value
                        text: qsTr("value:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetResistance5ValueTextField
                        text: settings.tile_preset_resistance_5_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetResistance5ValueButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_resistance_5_value = presetResistance5ValueTextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetResistance5Label
                        text: qsTr("label:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetResistance5LabelTextField
                        text: settings.tile_preset_resistance_5_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetResistance5LabelButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_resistance_5_label = presetResistance5LabelTextField.displayText
                    }
                }
            }
        }
        AccordionCheckElement {
            id: presetSpeed1EnabledAccordion
            title: qsTr("Preset Speed 1")
            linkedBoolSetting: "tile_preset_speed_1_enabled"
            settings: settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetSpeed1Order
                        text: qsTr("order index:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ComboBox {
                        id: presetSpeed1TextField
                        model: rootItem.tile_order
                        displayText: settings.tile_preset_speed_1_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetSpeed1TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetSpeed1OrderButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_speed_1_order = presetSpeed1TextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetSpeed1Value
                        text: qsTr("value:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetSpeed1ValueTextField
                        text: settings.tile_preset_speed_1_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetSpeed1ValueButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_speed_1_value = presetSpeed1ValueTextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetSpeed1Label
                        text: qsTr("label:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetSpeed1LabelTextField
                        text: settings.tile_preset_speed_1_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetSpeed1LabelButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_speed_1_label = presetSpeed1LabelTextField.displayText
                    }
                }
            }
        }
        AccordionCheckElement {
            id: presetSpeed2EnabledAccordion
            title: qsTr("Preset Speed 2")
            linkedBoolSetting: "tile_preset_speed_2_enabled"
            settings: settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetSpeed2Order
                        text: qsTr("order index:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ComboBox {
                        id: presetSpeed2TextField
                        model: rootItem.tile_order
                        displayText: settings.tile_preset_speed_2_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetSpeed2TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetSpeed2OrderButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_speed_2_order = presetSpeed2TextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetSpeed2Value
                        text: qsTr("value:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetSpeed2ValueTextField
                        text: settings.tile_preset_speed_2_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetSpeed2ValueButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_speed_2_value = presetSpeed2ValueTextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetSpeed2Label
                        text: qsTr("label:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetSpeed2LabelTextField
                        text: settings.tile_preset_speed_2_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetSpeed2LabelButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_speed_2_label = presetSpeed2LabelTextField.displayText
                    }
                }
            }
        }
        AccordionCheckElement {
            id: presetSpeed3EnabledAccordion
            title: qsTr("Preset Speed 3")
            linkedBoolSetting: "tile_preset_speed_3_enabled"
            settings: settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetSpeed3Order
                        text: qsTr("order index:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ComboBox {
                        id: presetSpeed3TextField
                        model: rootItem.tile_order
                        displayText: settings.tile_preset_speed_3_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetSpeed3TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetSpeed3OrderButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_speed_3_order = presetSpeed3TextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetSpeed3Value
                        text: qsTr("value:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetSpeed3ValueTextField
                        text: settings.tile_preset_speed_3_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetSpeed3ValueButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_speed_3_value = presetSpeed3ValueTextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetSpeed3Label
                        text: qsTr("label:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetSpeed3LabelTextField
                        text: settings.tile_preset_speed_3_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetSpeed3LabelButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_speed_3_label = presetSpeed3LabelTextField.displayText
                    }
                }
            }
        }
        AccordionCheckElement {
            id: presetSpeed4EnabledAccordion
            title: qsTr("Preset Speed 4")
            linkedBoolSetting: "tile_preset_speed_4_enabled"
            settings: settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetSpeed4Order
                        text: qsTr("order index:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ComboBox {
                        id: presetSpeed4TextField
                        model: rootItem.tile_order
                        displayText: settings.tile_preset_speed_4_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetSpeed4TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetSpeed4OrderButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_speed_4_order = presetSpeed4TextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetSpeed4Value
                        text: qsTr("value:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetSpeed4ValueTextField
                        text: settings.tile_preset_speed_4_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetSpeed4ValueButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_speed_4_value = presetSpeed4ValueTextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetSpeed4Label
                        text: qsTr("label:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetSpeed4LabelTextField
                        text: settings.tile_preset_speed_4_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetSpeed4LabelButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_speed_4_label = presetSpeed4LabelTextField.displayText
                    }
                }
            }
        }
        AccordionCheckElement {
            id: presetSpeed5EnabledAccordion
            title: qsTr("Preset Speed 5")
            linkedBoolSetting: "tile_preset_speed_5_enabled"
            settings: settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetSpeed5Order
                        text: qsTr("order index:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ComboBox {
                        id: presetSpeed5TextField
                        model: rootItem.tile_order
                        displayText: settings.tile_preset_speed_5_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetSpeed5TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetSpeed5OrderButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_speed_5_order = presetSpeed5TextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetSpeed5Value
                        text: qsTr("value:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetSpeed5ValueTextField
                        text: settings.tile_preset_speed_5_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetSpeed5ValueButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_speed_5_value = presetSpeed5ValueTextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetSpeed5Label
                        text: qsTr("label:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetSpeed5LabelTextField
                        text: settings.tile_preset_speed_5_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetSpeed5LabelButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_speed_5_label = presetSpeed5LabelTextField.displayText
                    }
                }
            }
        }
        AccordionCheckElement {
            id: presetInclination1EnabledAccordion
            title: qsTr("Preset Inclination 1")
            linkedBoolSetting: "tile_preset_inclination_1_enabled"
            settings: settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetInclination1Order
                        text: qsTr("order index:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ComboBox {
                        id: presetInclination1TextField
                        model: rootItem.tile_order
                        displayText: settings.tile_preset_inclination_1_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetInclination1TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetInclination1OrderButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_inclination_1_order = presetInclination1TextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetInclination1Value
                        text: qsTr("value:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetInclination1ValueTextField
                        text: settings.tile_preset_inclination_1_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetInclination1ValueButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_inclination_1_value = presetInclination1ValueTextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetInclination1Label
                        text: qsTr("label:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetInclination1LabelTextField
                        text: settings.tile_preset_inclination_1_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetInclination1LabelButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_inclination_1_label = presetInclination1LabelTextField.displayText
                    }
                }
            }
        }
        AccordionCheckElement {
            id: presetInclination2EnabledAccordion
            title: qsTr("Preset Inclination 2")
            linkedBoolSetting: "tile_preset_inclination_2_enabled"
            settings: settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetInclination2Order
                        text: qsTr("order index:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ComboBox {
                        id: presetInclination2TextField
                        model: rootItem.tile_order
                        displayText: settings.tile_preset_inclination_2_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetInclination2TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetInclination2OrderButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_inclination_2_order = presetInclination2TextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetInclination2Value
                        text: qsTr("value:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetInclination2ValueTextField
                        text: settings.tile_preset_inclination_2_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetInclination2ValueButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_inclination_2_value = presetInclination2ValueTextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetInclination2Label
                        text: qsTr("label:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetInclination2LabelTextField
                        text: settings.tile_preset_inclination_2_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetInclination2LabelButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_inclination_2_label = presetInclination2LabelTextField.displayText
                    }
                }
            }
        }
        AccordionCheckElement {
            id: presetInclination3EnabledAccordion
            title: qsTr("Preset Inclination 3")
            linkedBoolSetting: "tile_preset_inclination_3_enabled"
            settings: settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetInclination3Order
                        text: qsTr("order index:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ComboBox {
                        id: presetInclination3TextField
                        model: rootItem.tile_order
                        displayText: settings.tile_preset_inclination_3_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetInclination3TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetInclination3OrderButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_inclination_3_order = presetInclination3TextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetInclination3Value
                        text: qsTr("value:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetInclination3ValueTextField
                        text: settings.tile_preset_inclination_3_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetInclination3ValueButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_inclination_3_value = presetInclination3ValueTextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetInclination3Label
                        text: qsTr("label:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetInclination3LabelTextField
                        text: settings.tile_preset_inclination_3_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetInclination3LabelButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_inclination_3_label = presetInclination3LabelTextField.displayText
                    }
                }
            }
        }
        AccordionCheckElement {
            id: presetInclination4EnabledAccordion
            title: qsTr("Preset Inclination 4")
            linkedBoolSetting: "tile_preset_inclination_4_enabled"
            settings: settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetInclination4Order
                        text: qsTr("order index:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ComboBox {
                        id: presetInclination4TextField
                        model: rootItem.tile_order
                        displayText: settings.tile_preset_inclination_4_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetInclination4TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetInclination4OrderButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_inclination_4_order = presetInclination4TextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetInclination4Value
                        text: qsTr("value:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetInclination4ValueTextField
                        text: settings.tile_preset_inclination_4_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetInclination4ValueButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_inclination_4_value = presetInclination4ValueTextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetInclination4Label
                        text: qsTr("label:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetInclination4LabelTextField
                        text: settings.tile_preset_inclination_4_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetInclination4LabelButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_inclination_4_label = presetInclination4LabelTextField.displayText
                    }
                }
            }
        }
        AccordionCheckElement {
            id: presetInclination5EnabledAccordion
            title: qsTr("Preset Inclination 5")
            linkedBoolSetting: "tile_preset_inclination_5_enabled"
            settings: settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        id: labelPresetInclination5Order
                        text: qsTr("order index:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ComboBox {
                        id: presetInclination5TextField
                        model: rootItem.tile_order
                        displayText: settings.tile_preset_inclination_5_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetInclination5TextField.currentValue
                        }
                    }
                    Button {
                        id: okPresetInclination5OrderButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_inclination_5_order = presetInclination5TextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetInclination5Value
                        text: qsTr("value:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetInclination5ValueTextField
                        text: settings.tile_preset_inclination_5_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetInclination5ValueButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_inclination_5_value = presetInclination5ValueTextField.displayText
                    }
                }
                RowLayout {
                    Label {
                        id: labelPresetInclination5Label
                        text: qsTr("label:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetInclination5LabelTextField
                        text: settings.tile_preset_inclination_5_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okPresetInclination5LabelButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.tile_preset_inclination_5_label = presetInclination5LabelTextField.displayText
                    }
                }
            }
        }
    }
}
