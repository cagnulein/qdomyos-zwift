import QtQuick 2.7
import QtQuick.Dialogs 1.0
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
		  property string tile_preset_resistance_1_color: "grey"
		  property string tile_preset_resistance_2_color: "grey"
		  property string tile_preset_resistance_3_color: "grey"
		  property string tile_preset_resistance_4_color: "grey"
		  property string tile_preset_resistance_5_color: "grey"
		  property string tile_preset_speed_1_color: "grey"
		  property string tile_preset_speed_2_color: "grey"
		  property string tile_preset_speed_3_color: "grey"
		  property string tile_preset_speed_4_color: "grey"
		  property string tile_preset_speed_5_color: "grey"
		  property string tile_preset_inclination_1_color: "grey"
		  property string tile_preset_inclination_2_color: "grey"
		  property string tile_preset_inclination_3_color: "grey"
		  property string tile_preset_inclination_4_color: "grey"
		  property string tile_preset_inclination_5_color: "grey"
		  property bool tile_avg_watt_lap_enabled: false
		  property int tile_avg_watt_lap_order: 48
        property bool tile_pace_last500m_enabled: true
        property int  tile_pace_last500m_order: 49
        property bool tile_target_pace_enabled: false
        property int  tile_target_pace_order: 50
        property bool tile_step_count_enabled: false
        property int  tile_step_count_order: 51
        property bool tile_erg_mode_enabled: false
        property int  tile_erg_mode_order: 52
        property bool tile_rss_enabled: false
        property int  tile_rss_order: 53        
        property bool tile_biggears_enabled: false
        property int  tile_biggears_order: 54
        property bool tile_biggears_swap: false
        
        property bool tile_preset_powerzone_1_enabled: false
        property int tile_preset_powerzone_1_order: 55
        property real tile_preset_powerzone_1_value: 1.0
        property string tile_preset_powerzone_1_label: "Zone 1"
        property string tile_preset_powerzone_1_color: "white"

        property bool tile_preset_powerzone_2_enabled: false
        property int tile_preset_powerzone_2_order: 56
        property real tile_preset_powerzone_2_value: 2.0
        property string tile_preset_powerzone_2_label: "Zone 2"
        property string tile_preset_powerzone_2_color: "limegreen"

        property bool tile_preset_powerzone_3_enabled: false
        property int tile_preset_powerzone_3_order: 57
        property real tile_preset_powerzone_3_value: 3.0
        property string tile_preset_powerzone_3_label: "Zone 3"
        property string tile_preset_powerzone_3_color: "gold"

        property bool tile_preset_powerzone_4_enabled: false
        property int tile_preset_powerzone_4_order: 58
        property real tile_preset_powerzone_4_value: 4.0
        property string tile_preset_powerzone_4_label: "Zone 4"
        property string tile_preset_powerzone_4_color: "orange"

        property bool tile_preset_powerzone_5_enabled: false
        property int tile_preset_powerzone_5_order: 59
        property real tile_preset_powerzone_5_value: 5.0
        property string tile_preset_powerzone_5_label: "Zone 5"
        property string tile_preset_powerzone_5_color: "darkorange"

        property bool tile_preset_powerzone_6_enabled: false
        property int tile_preset_powerzone_6_order: 60
        property real tile_preset_powerzone_6_value: 6.0
        property string tile_preset_powerzone_6_label: "Zone 6"
        property string tile_preset_powerzone_6_color: "orangered"

        property bool tile_preset_powerzone_7_enabled: false
        property int tile_preset_powerzone_7_order: 61
        property real tile_preset_powerzone_7_value: 7.0
        property string tile_preset_powerzone_7_label: "Zone 7"
        property string tile_preset_powerzone_7_color: "red"        

        property bool tile_hr_time_in_zone_1_enabled: false
        property int  tile_hr_time_in_zone_1_order: 62
        property bool tile_hr_time_in_zone_2_enabled: false
        property int  tile_hr_time_in_zone_2_order: 63
        property bool tile_hr_time_in_zone_3_enabled: false
        property int  tile_hr_time_in_zone_3_order: 64
        property bool tile_hr_time_in_zone_4_enabled: false
        property int  tile_hr_time_in_zone_4_order: 65
        property bool tile_hr_time_in_zone_5_enabled: false
        property int  tile_hr_time_in_zone_5_order: 66

        property bool tile_coretemperature_enabled: false
        property int  tile_coretemperature_order: 67

        property bool tile_heat_time_in_zone_1_enabled: false
        property int  tile_heat_time_in_zone_1_order: 68
        property bool tile_heat_time_in_zone_2_enabled: false
        property int  tile_heat_time_in_zone_2_order: 69
        property bool tile_heat_time_in_zone_3_enabled: false
        property int  tile_heat_time_in_zone_3_order: 70
        property bool tile_heat_time_in_zone_4_enabled: false
        property int  tile_heat_time_in_zone_4_order: 71

        property bool tile_hr_time_in_zone_individual_mode: false

        property bool tile_auto_virtual_shifting_cruise_enabled: false
        property int  tile_auto_virtual_shifting_cruise_order: 72
        property bool tile_auto_virtual_shifting_climb_enabled: false
        property int  tile_auto_virtual_shifting_climb_order: 73
        property bool tile_auto_virtual_shifting_sprint_enabled: false
        property int  tile_auto_virtual_shifting_sprint_order: 74
        property bool tile_negative_inclination_enabled: false
        property int  tile_negative_inclination_order: 75
        property bool tile_avg_pace_enabled: false
        property int  tile_avg_pace_order: 76
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
                    onClicked: {settings.tile_speed_order = speedOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Speed in kilometers per hour. (To set your speed units to miles, go to Settings > General Options > Use Miles unit in UI).")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_inclination_order = inclinationOrderTextField.displayText; toast.show("Setting saved!"); }
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

                Label {
                    text: qsTr("Bike pedal cadence changes color to indicate how your cadence compares to the cadence called out in Peloton classes. The tile displays the following colors: white if there is no target cadence in the program, red if your cadence is lower than the target, green if your cadence matches the target, and orange if your cadence is higher than the target.")
                    font.bold: true
                    font.italic: true
                    font.pixelSize: Qt.application.font.pixelSize - 2
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
                        onClicked: {settings.tile_cadence_order = cadenceOrderTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
            }
        }

        Label {
            text: qsTr("Bike pedal cadence in rotations per minute (RPM) or Treadmill cadence if a shoe-mounted cadence sensor or Apple Watch QZ app is used.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_elevation_order = elevationOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }            

        AccordionCheckElement {
            id: negativeInclinationEnabledAccordion
            title: qsTr("Negative Elevation Gain (Descent)")
            linkedBoolSetting: "tile_negative_inclination_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: negativeInclinationOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_negative_inclination_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = negativeInclinationOrderTextField.currentValue
                     }
                }
                Button {
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_negative_inclination_order = negativeInclinationOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Displays the total negative elevation gain (descent) in meters or feet accumulated during the workout.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_calories_order = caloriesOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Estimated calories burned during session, calculated on weight, age, and watts.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_odometer_order = odometerOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Estimated distance traveled during the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_pace_order = paceOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        AccordionCheckElement {
            id: avgPaceEnabledAccordion
            title: qsTr("Average Pace")
            linkedBoolSetting: "tile_avg_pace_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelavgpaceOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: avgpaceOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_avg_pace_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = avgpaceOrderTextField.currentValue
                     }
                }
                Button {
                    id: okavgpaceOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_avg_pace_order = avgpaceOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Current pace per mile or kilometer (Treadmill, Elliptical and Rower)")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_resistance_order = resistanceOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Displays your bike’s resistance. The +/- buttons can be used to change resistance, if your bike is compatible.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_watt_order = wattOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Displays the watts generated by your current effort. Watt is also referred to as output (for example, in Peloton). If your equipment does not communicate watts, QZ will calculate watts using resistance and cadence.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_weight_loss_order = weightLossOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Estimation of weight loss during the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_avgwatt_order = avgwattOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Average watts produced for the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
				settings: settings
				accordionContent: RowLayout {
				    spacing: 10
					 Label {
					     id: labelavgwattLapOrder
						  text: qsTr("order index:")
						  Layout.fillWidth: true
						  horizontalAlignment: Text.AlignRight
						}
					 ComboBox {
					     id: avgwattLapOrderTextField
						  model: rootItem.tile_order
						  displayText: settings.tile_avg_watt_lap_order
						  Layout.fillHeight: false
						  Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
						  onActivated: {
						      displayText = avgwattLapOrderTextField.currentValue
								}
							}
					 Button {
					     id: okavgwattLapOrderButton
						  text: "OK"
						  Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
						  onClicked: {settings.tile_avg_watt_lap_order = avgwattLapOrderTextField.displayText; toast.show("Setting saved!"); }
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
                    onClicked: {settings.tile_ftp_order = ftpOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Percentage of current FTP and current FTP zone.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_heart_order = heartrateOrderTextField.displayText; toast.show("Setting saved!"); }
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
                    onClicked: {settings.tile_fan_order = fanOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Built-in treadmill fan speed (Treadmill only)")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_jouls_order = joulsOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Cumulative power produced during the session in kilojoules.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_elapsed_order = elapsedOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Total time from start of the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_moving_time_order = movingTimeOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Total time moving during the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_peloton_offset_order = pelotonOffsetOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Allows you to sync resistance and cadence target changes with the Peloton coach’s callouts. If the targets are changing in QZ after the coach’s callouts, use the ‘+’ button to add seconds (essentially speeding QZ up). Use the ‘-’ button to slow QZ down. Use this tile in conjunction with the Remaining Time/Row tile (see below).")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_peloton_remaining_order = pelotonRemainingOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Displays time remaining in Peloton class.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_peloton_difficulty_order = pelotonDifficultyOrderTextField.displayText; toast.show("Setting saved!"); }
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
                    onClicked: {settings.tile_lapelapsed_order = lapElapsedOrderTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_peloton_resistance_order = peloton_resistanceOrderTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
            }
        }

        Label {
            text: qsTr("Resistance of your bike converted to the Peloton bike scale of 1 to 100.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_target_resistance_order = target_resistanceOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Displays target resistance in your bike’s resistance scale. For example, during a Peloton class or Zwift session, you want the resistance displayed in this tile to match the Resistance Tile.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_target_peloton_resistance_order = target_peloton_resistanceOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Displays target resistance converted to the Peloton bike scale of 1 to 100. For example, during a Peloton class, you want the resistance displayed in this tile to match the Peloton Resistance Tile.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_target_cadence_order = target_cadenceOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Displays target cadence.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_target_power_order = target_powerOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Displays target output (watts) when this information is provided by third-party apps.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_target_zone_order = target_zoneOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Displays the target power zone when this information is provided by third-party apps.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_target_speed_order = target_speedOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        AccordionCheckElement {
            id: targetPaceEnabledAccordion
            title: qsTr("Target Pace")
            linkedBoolSetting: "tile_target_pace_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labeltargetpaceOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: target_paceOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_target_pace_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = target_paceOrderTextField.currentValue
                     }
                }
                Button {
                    id: oktarget_paceOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_target_pace_order = target_paceOrderTextField.displayText; toast.show("Setting saved!"); }
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
                    onClicked: {settings.tile_target_incline_order = target_inclineOrderTextField.displayText; toast.show("Setting saved!"); }
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
                    onClicked: {settings.tile_watt_kg_order = watt_kgOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Calculates your output (watts) divided by your weight. This is the primary metric used by Zwift and similar apps to calculate your virtual speed. NOTE: This is a much better metric to use than Output/Watts when comparing your effort to other users. This is why Peloton’s leaderboard, which uses only Output, is flawed.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_gears_order = gearsOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Allows you to change resistance while in Auto-Follow Mode.This tile allows you override the target resistance sent by third-party apps. For example, you would use the Gears Tile to increase resistance and generate more watts for sprinting in Zwift.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
            settings: settings
            accordionContent: ColumnLayout {
                RowLayout {
                    spacing: 10
                    Label {
                        text: qsTr("order index:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ComboBox {
                        id: biggearsOrderTextField
                        model: rootItem.tile_order
                        displayText: settings.tile_biggears_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = biggearsOrderTextField.currentValue
                         }
                    }
                    Button {
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_biggears_order = biggearsOrderTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
                SwitchDelegate {
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
            font.pixelSize: Qt.application.font.pixelSize - 2
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
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
                    onClicked: {settings.tile_remainingtimetrainprogramrow_order = remainingTimeTrainingProgramRowOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Displays the time remaining until the next cadence and/or resistance interval.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
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
                    onClicked: {settings.tile_nextrowstrainprogram_order = nextRowsTrainingProgramOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Displays the next Peloton interval with duration and FTP Zone (in Power Zone classes) or Peloton Resistance (non–Power Zone classes).")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_mets_order = metsOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Displays metabolic equivalents (METs), a measurement of energy expenditure and amount of oxygen used by the body compared to the body at rest. (e.g., 4 METS requires the body to use 4 times as much oxygen than when at rest, which means it requires more energy and burns more calories).")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_targetmets_order = targetmetsOrderTextField.displayText; toast.show("Setting saved!"); }
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
                    onClicked: {settings.tile_datetime_order = datetimeOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Displays the current time.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_strokes_count_order = strokes_countOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("(Rower only) Displays the number of strokes rowed.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
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
                    onClicked: {settings.tile_strokes_length_order = strokes_lengthOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("(Rower only) Displays the stroke length.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_steering_angle_order = steeringAngleOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("(Elite Rizer only) Displays steering angle.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_pid_hr_order = pidHROrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Use this tile to display the target heart rate zone in which you’ve chosen to work out in Settings > Training Program Options.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_ext_incline_order = extInclineOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("(Elite Rizer only) Allows control of the incline of external inclination equipment.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_instantaneous_stride_length_order = strideLengthOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("(requires a compatible footpod with accelerometer; treadmill only) Displays stride while walking or running.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_ground_contact_order = groundContactOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("(requires a compatible footpod with accelerometer; treadmill only) Displays time foot is on contact with ground while walking or running.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    onClicked: {settings.tile_vertical_oscillation_order = verticalOscillationOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("(requires a compatible footpod with accelerometer; treadmill only) Displays the up and down movement while walking or running.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelPacelast500mOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: pacelast500mOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_pace_last500m_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = pacelast500mOrderTextField.currentValue
                     }
                }
                Button {
                    id: okPacelast500mOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_pace_last500m_order = pacelast500mOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        AccordionCheckElement {
            id: stepCountEnabledAccordion
            title: qsTr("Step Count")
            linkedBoolSetting: "tile_step_count_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelStepCountOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: stepCountOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_step_count_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = stepCountOrderTextField.currentValue
                     }
                }
                Button {
                    id: okStepCountOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_step_count_order = stepCountOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        AccordionCheckElement {
            id: ergModeEnabledAccordion
            title: qsTr("Erg Mode")
            linkedBoolSetting: "tile_erg_mode_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelErgModeOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: ergModeOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_erg_mode_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = ergModeOrderTextField.currentValue
                     }
                }
                Button {
                    id: okErgModeOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_erg_mode_order = ergModeOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        AccordionCheckElement {
            title: qsTr("Running Stress Score")
            linkedBoolSetting: "tile_rss_enabled"
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: rssOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_rss_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = rssOrderTextField.currentValue
                     }
                }
                Button {
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_rss_order = rssOrderTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_resistance_1_order = presetResistance1TextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_resistance_1_value = presetResistance1ValueTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_resistance_1_label = presetResistance1LabelTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
					 RowLayout {
					     Label {
						      id: labelPresetResistance1Color
								text: qsTr("color:")
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetResistance1
								title: "Please choose a color"
								onAccepted: {
								    presetResistance1ColorTextField.text = colorPresetResistance1.color
									}
								onRejected: {

                        }
						  }
						  TextField {
						      id: presetResistance1ColorTextField
								text: settings.tile_preset_resistance_1_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetResistance1.visible = true
								}
						  }
						  Button {
						      id: okPresetResistance1ColorButton
								text: "OK"
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_resistance_1_color = presetResistance1ColorTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_resistance_2_order = presetResistance2TextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_resistance_2_value = presetResistance2ValueTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_resistance_2_label = presetResistance2LabelTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
					 RowLayout {
					     Label {
						      id: labelPresetResistance2Color
								text: qsTr("color:")
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetResistance2
								title: "Please choose a color"
								onAccepted: {
								    presetResistance2ColorTextField.text = colorPresetResistance2.color
									}
								onRejected: {

                        }
						  }
						  TextField {
						      id: presetResistance2ColorTextField
								text: settings.tile_preset_resistance_2_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetResistance2.visible = true
								}
						  }
						  Button {
						      id: okPresetResistance2ColorButton
								text: "OK"
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_resistance_2_color = presetResistance2ColorTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_resistance_3_order = presetResistance3TextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_resistance_3_value = presetResistance3ValueTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_resistance_3_label = presetResistance3LabelTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
					 RowLayout {
					     Label {
						      id: labelPresetResistance3Color
								text: qsTr("color:")
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetResistance3
								title: "Please choose a color"
								onAccepted: {
								    presetResistance3ColorTextField.text = colorPresetResistance3.color
									}
								onRejected: {

                        }
						  }
						  TextField {
						      id: presetResistance3ColorTextField
								text: settings.tile_preset_resistance_3_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetResistance3.visible = true
								}
						  }
						  Button {
						      id: okPresetResistance3ColorButton
								text: "OK"
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_resistance_3_color = presetResistance3ColorTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_resistance_4_order = presetResistance4TextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_resistance_4_value = presetResistance4ValueTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_resistance_4_label = presetResistance4LabelTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
					 RowLayout {
					     Label {
						      id: labelPresetResistance4Color
								text: qsTr("color:")
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetResistance4
								title: "Please choose a color"
								onAccepted: {
								    presetResistance4ColorTextField.text = colorPresetResistance4.color
									}
								onRejected: {

                        }
						  }
						  TextField {
						      id: presetResistance4ColorTextField
								text: settings.tile_preset_resistance_4_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetResistance4.visible = true
								}
						  }
						  Button {
						      id: okPresetResistance4ColorButton
								text: "OK"
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_resistance_4_color = presetResistance4ColorTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_resistance_5_order = presetResistance5TextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_resistance_5_value = presetResistance5ValueTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_resistance_5_label = presetResistance5LabelTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
					 RowLayout {
					     Label {
						      id: labelPresetResistance5Color
								text: qsTr("color:")
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetResistance5
								title: "Please choose a color"
								onAccepted: {
								    presetResistance5ColorTextField.text = colorPresetResistance5.color
									}
								onRejected: {

                        }
						  }
						  TextField {
						      id: presetResistance5ColorTextField
								text: settings.tile_preset_resistance_5_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetResistance5.visible = true
								}
						  }
						  Button {
						      id: okPresetResistance5ColorButton
								text: "OK"
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_resistance_5_color = presetResistance5ColorTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_speed_1_order = presetSpeed1TextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_speed_1_value = presetSpeed1ValueTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_speed_1_label = presetSpeed1LabelTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
					 RowLayout {
					     Label {
						      id: labelPresetSpeed1Color
								text: qsTr("color:")
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetSpeed1
								title: "Please choose a color"
								onAccepted: {
								    presetSpeed1ColorTextField.text = colorPresetSpeed1.color
									}
								onRejected: {

                        }
						  }
						  TextField {
						      id: presetSpeed1ColorTextField
								text: settings.tile_preset_speed_1_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetSpeed1.visible = true
								}
						  }
						  Button {
						      id: okPresetSpeed1ColorButton
								text: "OK"
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_speed_1_color = presetSpeed1ColorTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_speed_2_order = presetSpeed2TextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_speed_2_value = presetSpeed2ValueTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_speed_2_label = presetSpeed2LabelTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
					 RowLayout {
					     Label {
						      id: labelPresetSpeed2Color
								text: qsTr("color:")
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetSpeed2
								title: "Please choose a color"
								onAccepted: {
								    presetSpeed2ColorTextField.text = colorPresetSpeed2.color
									}
								onRejected: {

                        }
						  }
						  TextField {
						      id: presetSpeed2ColorTextField
								text: settings.tile_preset_speed_2_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetSpeed2.visible = true
								}
						  }
						  Button {
						      id: okPresetSpeed2ColorButton
								text: "OK"
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_speed_2_color = presetSpeed2ColorTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_speed_3_order = presetSpeed3TextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_speed_3_value = presetSpeed3ValueTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_speed_3_label = presetSpeed3LabelTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
					 RowLayout {
					     Label {
						      id: labelPresetSpeed3Color
								text: qsTr("color:")
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetSpeed3
								title: "Please choose a color"
								onAccepted: {
								    presetSpeed3ColorTextField.text = colorPresetSpeed3.color
									}
								onRejected: {

                        }
						  }
						  TextField {
						      id: presetSpeed3ColorTextField
								text: settings.tile_preset_speed_3_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetSpeed3.visible = true
								}
						  }
						  Button {
						      id: okPresetSpeed3ColorButton
								text: "OK"
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_speed_3_color = presetSpeed3ColorTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_speed_4_order = presetSpeed4TextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_speed_4_value = presetSpeed4ValueTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_speed_4_label = presetSpeed4LabelTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
					 RowLayout {
					     Label {
						      id: labelPresetSpeed4Color
								text: qsTr("color:")
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetSpeed4
								title: "Please choose a color"
								onAccepted: {
								    presetSpeed4ColorTextField.text = colorPresetSpeed4.color
									}
								onRejected: {

                        }
						  }
						  TextField {
						      id: presetSpeed4ColorTextField
								text: settings.tile_preset_speed_4_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetSpeed4.visible = true
								}
						  }
						  Button {
						      id: okPresetSpeed4ColorButton
								text: "OK"
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_speed_4_color = presetSpeed4ColorTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_speed_5_order = presetSpeed5TextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_speed_5_value = presetSpeed5ValueTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_speed_5_label = presetSpeed5LabelTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
					 RowLayout {
					     Label {
						      id: labelPresetSpeed5Color
								text: qsTr("color:")
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetSpeed5
								title: "Please choose a color"
								onAccepted: {
								    presetSpeed5ColorTextField.text = colorPresetSpeed5.color
									}
								onRejected: {

                        }
						  }
						  TextField {
						      id: presetSpeed5ColorTextField
								text: settings.tile_preset_speed_5_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetSpeed5.visible = true
								}
						  }
						  Button {
						      id: okPresetSpeed5ColorButton
								text: "OK"
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_speed_5_color = presetSpeed5ColorTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_inclination_1_order = presetInclination1TextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_inclination_1_value = presetInclination1ValueTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_inclination_1_label = presetInclination1LabelTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
					 RowLayout {
					     Label {
						      id: labelPresetInclination1Color
								text: qsTr("color:")
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetInclination1
								title: "Please choose a color"
								onAccepted: {
								    presetInclination1ColorTextField.text = colorPresetInclination1.color
									}
								onRejected: {

                        }
						  }
						  TextField {
						      id: presetInclination1ColorTextField
								text: settings.tile_preset_inclination_1_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetInclination1.visible = true
								}
						  }
						  Button {
						      id: okPresetInclination1ColorButton
								text: "OK"
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_inclination_1_color = presetInclination1ColorTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_inclination_2_order = presetInclination2TextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_inclination_2_value = presetInclination2ValueTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_inclination_2_label = presetInclination2LabelTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
            }
				RowLayout {
				    Label {
					     id: labelPresetInclination2Color
						  text: qsTr("color:")
						  Layout.fillWidth: true
						  horizontalAlignment: Text.AlignRight
						  }
						ColorDialog {
						  id: colorPresetInclination2
						  title: "Please choose a color"
						  onAccepted: {
						      presetInclination2ColorTextField.text = colorPresetInclination2.color
							  }
						  onRejected: {

                    }
						}
					 TextField {
					     id: presetInclination2ColorTextField
						  text: settings.tile_preset_inclination_2_color
						  Layout.fillHeight: false
						  Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
						  onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
						  onPressed: {
                              if(OS_VERSION !== "Android") colorPresetInclination2.visible = true
						  }
						}
					 Button {
					     id: okPresetInclination2ColorButton
						  text: "OK"
						  Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
						  onClicked: {settings.tile_preset_inclination_2_color = presetInclination2ColorTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_inclination_3_order = presetInclination3TextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_inclination_3_value = presetInclination3ValueTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_inclination_3_label = presetInclination3LabelTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
					 RowLayout {
					     Label {
						      id: labelPresetInclination3Color
								text: qsTr("color:")
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetInclination3
								title: "Please choose a color"
								onAccepted: {
								    presetInclination3ColorTextField.text = colorPresetInclination3.color
									}
								onRejected: {

                        }
						  }
						  TextField {
						      id: presetInclination3ColorTextField
								text: settings.tile_preset_inclination_3_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetInclination3.visible = true
								}
						  }
						  Button {
						      id: okPresetInclination3ColorButton
								text: "OK"
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_inclination_3_color = presetInclination3ColorTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_inclination_4_order = presetInclination4TextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_inclination_4_value = presetInclination4ValueTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_inclination_4_label = presetInclination4LabelTextField.displayText; toast.show("Setting saved!"); }
                    }
						}
					 RowLayout {
					     Label {
						      id: labelPresetInclination4Color
								text: qsTr("color:")
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetInclination4
								title: "Please choose a color"
								onAccepted: {
								    presetInclination4ColorTextField.text = colorPresetInclination4.color
									}
								onRejected: {

                        }
						  }
						  TextField {
						      id: presetInclination4ColorTextField
								text: settings.tile_preset_inclination_4_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetInclination4.visible = true
								}
						  }
						  Button {
						      id: okPresetInclination4ColorButton
								text: "OK"
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_inclination_4_color = presetInclination4ColorTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_inclination_5_order = presetInclination5TextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_inclination_5_value = presetInclination5ValueTextField.displayText; toast.show("Setting saved!"); }
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
                        onClicked: {settings.tile_preset_inclination_5_label = presetInclination5LabelTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
					 RowLayout {
					     Label {
						      id: labelPresetInclination5Color
								text: qsTr("color:")
								Layout.fillWidth: true
								horizontalAlignment: Text.AlignRight
								}
						  ColorDialog {
						      id: colorPresetInclination5
								title: "Please choose a color"
								onAccepted: {
								    presetInclination5ColorTextField.text = colorPresetInclination5.color
									}
								onRejected: {

                        }
						  }
						  TextField {
						      id: presetInclination5ColorTextField
								text: settings.tile_preset_inclination_5_color
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
								onPressed: {
                                    if(OS_VERSION !== "Android") colorPresetInclination5.visible = true
								}
						  }
						  Button {
						      id: okPresetInclination5ColorButton
								text: "OK"
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								onClicked: {settings.tile_preset_inclination_5_color = presetInclination5ColorTextField.displayText; toast.show("Setting saved!"); }
						  }
					 }
            }
        }       
        AccordionCheckElement {
            id: presetPowerZone1EnabledAccordion
            title: qsTr("Preset Power Zone 1")
            linkedBoolSetting: "tile_preset_powerzone_1_enabled"
            settings: settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        text: qsTr("order index:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ComboBox {
                        id: presetPowerZone1OrderTextField
                        model: rootItem.tile_order
                        displayText: settings.tile_preset_powerzone_1_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetPowerZone1OrderTextField.currentValue
                        }
                    }
                    Button {
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_1_order = presetPowerZone1OrderTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("zone value:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetPowerZone1Value
                        text: settings.tile_preset_powerzone_1_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        validator: DoubleValidator {bottom: 1; top: 7;}
                    }
                    Button {
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_1_value = parseFloat(presetPowerZone1Value.text); toast.show("Setting saved!"); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("label:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetPowerZone1Label
                        text: settings.tile_preset_powerzone_1_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    }
                    Button {
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_1_label = presetPowerZone1Label.text; toast.show("Setting saved!"); }
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
                        title: "Please choose a color"
                        onAccepted: {
                            presetPowerzone1ColorTextField.text = colorPresetPowerzone1.color
                        }
                        onRejected: {}
                    }
                    TextField {
                        id: presetPowerzone1ColorTextField
                        text: settings.tile_preset_powerzone_1_color
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        onPressed: {
                            if(OS_VERSION !== "Android") colorPresetPowerzone1.visible = true
                        }
                    }
                    Button {
                        id: okPresetPowerzone1ColorButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_1_color = presetPowerzone1ColorTextField.displayText; toast.show("Setting saved!"); }
                    }
                }                
            }
        }

        AccordionCheckElement {
            id: presetPowerZone2EnabledAccordion
            title: qsTr("Preset Power Zone 2")
            linkedBoolSetting: "tile_preset_powerzone_2_enabled"
            settings: settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        text: qsTr("order index:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ComboBox {
                        id: presetPowerZone2OrderTextField
                        model: rootItem.tile_order
                        displayText: settings.tile_preset_powerzone_2_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetPowerZone2OrderTextField.currentValue
                        }
                    }
                    Button {
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_2_order = presetPowerZone2OrderTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("zone value:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetPowerZone2Value
                        text: settings.tile_preset_powerzone_2_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        validator: DoubleValidator {bottom: 1; top: 7;}
                    }
                    Button {
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_2_value = parseFloat(presetPowerZone2Value.text); toast.show("Setting saved!"); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("label:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetPowerZone2Label
                        text: settings.tile_preset_powerzone_2_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    }
                    Button {
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_2_label = presetPowerZone2Label.text; toast.show("Setting saved!"); }
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
                        title: "Please choose a color"
                        onAccepted: {
                            presetPowerzone2ColorTextField.text = colorPresetPowerzone2.color
                        }
                        onRejected: {}
                    }
                    TextField {
                        id: presetPowerzone2ColorTextField
                        text: settings.tile_preset_powerzone_2_color
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        onPressed: {
                            if(OS_VERSION !== "Android") colorPresetPowerzone2.visible = true
                        }
                    }
                    Button {
                        id: okPresetPowerzone2ColorButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_2_color = presetPowerzone2ColorTextField.displayText; toast.show("Setting saved!"); }
                    }
                }                
            }
        }

        AccordionCheckElement {
            id: presetPowerZone3EnabledAccordion
            title: qsTr("Preset Power Zone 3")
            linkedBoolSetting: "tile_preset_powerzone_3_enabled"
            settings: settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        text: qsTr("order index:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ComboBox {
                        id: presetPowerZone3OrderTextField
                        model: rootItem.tile_order
                        displayText: settings.tile_preset_powerzone_3_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetPowerZone3OrderTextField.currentValue
                        }
                    }
                    Button {
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_3_order = presetPowerZone3OrderTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("zone value:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetPowerZone3Value
                        text: settings.tile_preset_powerzone_3_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        validator: DoubleValidator {bottom: 1; top: 7;}
                    }
                    Button {
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_3_value = parseFloat(presetPowerZone3Value.text); toast.show("Setting saved!"); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("label:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetPowerZone3Label
                        text: settings.tile_preset_powerzone_3_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    }
                    Button {
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_3_label = presetPowerZone3Label.text; toast.show("Setting saved!"); }
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
                        title: "Please choose a color"
                        onAccepted: {
                            presetPowerzone3ColorTextField.text = colorPresetPowerzone3.color
                        }
                        onRejected: {}
                    }
                    TextField {
                        id: presetPowerzone3ColorTextField
                        text: settings.tile_preset_powerzone_3_color
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        onPressed: {
                            if(OS_VERSION !== "Android") colorPresetPowerzone3.visible = true
                        }
                    }
                    Button {
                        id: okPresetPowerzone3ColorButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_3_color = presetPowerzone3ColorTextField.displayText; toast.show("Setting saved!"); }
                    }
                }                
            }
        }

        AccordionCheckElement {
            id: presetPowerZone4EnabledAccordion
            title: qsTr("Preset Power Zone 4")
            linkedBoolSetting: "tile_preset_powerzone_4_enabled"
            settings: settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        text: qsTr("order index:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ComboBox {
                        id: presetPowerZone4OrderTextField
                        model: rootItem.tile_order
                        displayText: settings.tile_preset_powerzone_4_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetPowerZone4OrderTextField.currentValue
                        }
                    }
                    Button {
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_4_order = presetPowerZone4OrderTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("zone value:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetPowerZone4Value
                        text: settings.tile_preset_powerzone_4_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        validator: DoubleValidator {bottom: 1; top: 7;}
                    }
                    Button {
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_4_value = parseFloat(presetPowerZone4Value.text); toast.show("Setting saved!"); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("label:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetPowerZone4Label
                        text: settings.tile_preset_powerzone_4_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    }
                    Button {
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_4_label = presetPowerZone4Label.text; toast.show("Setting saved!"); }
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
                        title: "Please choose a color"
                        onAccepted: {
                            presetPowerzone4ColorTextField.text = colorPresetPowerzone4.color
                        }
                        onRejected: {}
                    }
                    TextField {
                        id: presetPowerzone4ColorTextField
                        text: settings.tile_preset_powerzone_4_color
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        onPressed: {
                            if(OS_VERSION !== "Android") colorPresetPowerzone4.visible = true
                        }
                    }
                    Button {
                        id: okPresetPowerzone4ColorButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_4_color = presetPowerzone4ColorTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
            }
        }

        AccordionCheckElement {
            id: presetPowerZone5EnabledAccordion
            title: qsTr("Preset Power Zone 5")
            linkedBoolSetting: "tile_preset_powerzone_5_enabled"
            settings: settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        text: qsTr("order index:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ComboBox {
                        id: presetPowerZone5OrderTextField
                        model: rootItem.tile_order
                        displayText: settings.tile_preset_powerzone_5_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetPowerZone5OrderTextField.currentValue
                        }
                    }
                    Button {
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_5_order = presetPowerZone5OrderTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("zone value:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetPowerZone5Value
                        text: settings.tile_preset_powerzone_5_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        validator: DoubleValidator {bottom: 1; top: 7;}
                    }
                    Button {
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_5_value = parseFloat(presetPowerZone5Value.text); toast.show("Setting saved!"); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("label:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetPowerZone5Label
                        text: settings.tile_preset_powerzone_5_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    }
                    Button {
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_5_label = presetPowerZone5Label.text; toast.show("Setting saved!"); }
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
                        title: "Please choose a color"
                        onAccepted: {
                            presetPowerzone5ColorTextField.text = colorPresetPowerzone5.color
                        }
                        onRejected: {}
                    }
                    TextField {
                        id: presetPowerzone5ColorTextField
                        text: settings.tile_preset_powerzone_5_color
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        onPressed: {
                            if(OS_VERSION !== "Android") colorPresetPowerzone5.visible = true
                        }
                    }
                    Button {
                        id: okPresetPowerzone5ColorButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_5_color = presetPowerzone5ColorTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
            }
        }

        AccordionCheckElement {
            id: presetPowerZone6EnabledAccordion
            title: qsTr("Preset Power Zone 6")
            linkedBoolSetting: "tile_preset_powerzone_6_enabled"
            settings: settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        text: qsTr("order index:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ComboBox {
                        id: presetPowerZone6OrderTextField
                        model: rootItem.tile_order
                        displayText: settings.tile_preset_powerzone_6_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetPowerZone6OrderTextField.currentValue
                        }
                    }
                    Button {
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_6_order = presetPowerZone6OrderTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("zone value:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetPowerZone6Value
                        text: settings.tile_preset_powerzone_6_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        validator: DoubleValidator {bottom: 1; top: 7;}
                    }
                    Button {
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_6_value = parseFloat(presetPowerZone6Value.text); toast.show("Setting saved!"); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("label:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetPowerZone6Label
                        text: settings.tile_preset_powerzone_6_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    }
                    Button {
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_6_label = presetPowerZone6Label.text; toast.show("Setting saved!"); }
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
                        title: "Please choose a color"
                        onAccepted: {
                            presetPowerzone6ColorTextField.text = colorPresetPowerzone6.color
                        }
                        onRejected: {}
                    }
                    TextField {
                        id: presetPowerzone6ColorTextField
                        text: settings.tile_preset_powerzone_6_color
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        onPressed: {
                            if(OS_VERSION !== "Android") colorPresetPowerzone6.visible = true
                        }
                    }
                    Button {
                        id: okPresetPowerzone6ColorButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_6_color = presetPowerzone6ColorTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
            }
        }

        AccordionCheckElement {
            id: presetPowerZone7EnabledAccordion
            title: qsTr("Preset Power Zone 7")
            linkedBoolSetting: "tile_preset_powerzone_7_enabled"
            settings: settings
            accordionContent: ColumnLayout {
                spacing: 10
                RowLayout {
                    Label {
                        text: qsTr("order index:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ComboBox {
                        id: presetPowerZone7OrderTextField
                        model: rootItem.tile_order
                        displayText: settings.tile_preset_powerzone_7_order
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            displayText = presetPowerZone7OrderTextField.currentValue
                        }
                    }
                    Button {
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_7_order = presetPowerZone7OrderTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("zone value:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetPowerZone7Value
                        text: settings.tile_preset_powerzone_7_value
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        validator: DoubleValidator {bottom: 1; top: 7;}
                    }
                    Button {
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_7_value = parseFloat(presetPowerZone7Value.text); toast.show("Setting saved!"); }
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("label:")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: presetPowerZone7Label
                        text: settings.tile_preset_powerzone_7_label
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    }
                    Button {
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_7_label = presetPowerZone7Label.text; toast.show("Setting saved!"); }
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
                        title: "Please choose a color"
                        onAccepted: {
                            presetPowerzone7ColorTextField.text = colorPresetPowerzone7.color
                        }
                        onRejected: {}
                    }
                    TextField {
                        id: presetPowerzone7ColorTextField
                        text: settings.tile_preset_powerzone_7_color
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        onPressed: {
                            if(OS_VERSION !== "Android") colorPresetPowerzone7.visible = true
                        }
                    }
                    Button {
                        id: okPresetPowerzone7ColorButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {settings.tile_preset_powerzone_7_color = presetPowerzone7ColorTextField.displayText; toast.show("Setting saved!"); }
                    }
                }
            }
        }

        Label {
            text: qsTr("Power zone presets allow quick access to specific training zones with customizable labels and values.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelHrTimeInZone1Order
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: hrTimeInZone1OrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_hr_time_in_zone_1_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = hrTimeInZone1OrderTextField.currentValue
                     }
                }
                Button {
                    id: okHrTimeInZone1OrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_hr_time_in_zone_1_order = hrTimeInZone1OrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Displays total time spent in heart rate Zone 1 or higher during the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelHrTimeInZone2Order
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: hrTimeInZone2OrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_hr_time_in_zone_2_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = hrTimeInZone2OrderTextField.currentValue
                     }
                }
                Button {
                    id: okHrTimeInZone2OrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_hr_time_in_zone_2_order = hrTimeInZone2OrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Displays total time spent in heart rate Zone 2 or higher during the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelHrTimeInZone3Order
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: hrTimeInZone3OrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_hr_time_in_zone_3_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = hrTimeInZone3OrderTextField.currentValue
                     }
                }
                Button {
                    id: okHrTimeInZone3OrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_hr_time_in_zone_3_order = hrTimeInZone3OrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Displays total time spent in heart rate Zone 3 or higher during the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelHrTimeInZone4Order
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: hrTimeInZone4OrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_hr_time_in_zone_4_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = hrTimeInZone4OrderTextField.currentValue
                     }
                }
                Button {
                    id: okHrTimeInZone4OrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_hr_time_in_zone_4_order = hrTimeInZone4OrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Displays total time spent in heart rate Zone 4 or higher during the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelHrTimeInZone5Order
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: hrTimeInZone5OrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_hr_time_in_zone_5_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = hrTimeInZone5OrderTextField.currentValue
                     }
                }
                Button {
                    id: okHrTimeInZone5OrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_hr_time_in_zone_5_order = hrTimeInZone5OrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Displays total time spent in heart rate Zone 5 or higher during the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
                    toast.show("Setting saved!")
                }
            }
        }

        Label {
            text: qsTr("When enabled, each zone shows only the time spent in that specific zone. When disabled (default), each zone shows cumulative time spent in that zone or higher.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelcoretemperatureOrder
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: coretemperatureOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_coretemperature_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = coretemperatureOrderTextField.currentValue
                     }
                }
                Button {
                    id: okcoretemperatureOrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_coretemperature_order = coretemperatureOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Shows Core, Body Temperature and Heat Strain Index from a Core Temperature sensor.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelHeatTimeInZone1Order
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: heatTimeInZone1OrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_heat_time_in_zone_1_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = heatTimeInZone1OrderTextField.currentValue
                     }
                }
                Button {
                    id: okHeatTimeInZone1OrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_heat_time_in_zone_1_order = heatTimeInZone1OrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Displays total time spent in heat Zone 1 during the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelHeatTimeInZone2Order
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: heatTimeInZone2OrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_heat_time_in_zone_2_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = heatTimeInZone2OrderTextField.currentValue
                     }
                }
                Button {
                    id: okHeatTimeInZone2OrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_heat_time_in_zone_2_order = heatTimeInZone2OrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Displays total time spent in heat Zone 2 during the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelHeatTimeInZone3Order
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: heatTimeInZone3OrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_heat_time_in_zone_3_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = heatTimeInZone3OrderTextField.currentValue
                     }
                }
                Button {
                    id: okHeatTimeInZone3OrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_heat_time_in_zone_3_order = heatTimeInZone3OrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Displays total time spent in heat Zone 3 during the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    id: labelHeatTimeInZone4Order
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: heatTimeInZone4OrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_heat_time_in_zone_4_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = heatTimeInZone4OrderTextField.currentValue
                     }
                }
                Button {
                    id: okHeatTimeInZone4OrderButton
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_heat_time_in_zone_4_order = heatTimeInZone4OrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Displays total time spent in heat Zone 4 during the session.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: autoVirtualShiftingCruiseOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_auto_virtual_shifting_cruise_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = autoVirtualShiftingCruiseOrderTextField.currentValue
                     }
                }
                Button {
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_auto_virtual_shifting_cruise_order = autoVirtualShiftingCruiseOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Button tile to switch automatic virtual shifting to Cruise profile.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: autoVirtualShiftingClimbOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_auto_virtual_shifting_climb_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = autoVirtualShiftingClimbOrderTextField.currentValue
                     }
                }
                Button {
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_auto_virtual_shifting_climb_order = autoVirtualShiftingClimbOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Button tile to switch automatic virtual shifting to Climb profile.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
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
            settings: settings
            accordionContent: RowLayout {
                spacing: 10
                Label {
                    text: qsTr("order index:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
                ComboBox {
                    id: autoVirtualShiftingSprintOrderTextField
                    model: rootItem.tile_order
                    displayText: settings.tile_auto_virtual_shifting_sprint_order
                    Layout.fillHeight: false
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onActivated: {
                        displayText = autoVirtualShiftingSprintOrderTextField.currentValue
                     }
                }
                Button {
                    text: "OK"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {settings.tile_auto_virtual_shifting_sprint_order = autoVirtualShiftingSprintOrderTextField.displayText; toast.show("Setting saved!"); }
                }
            }
        }

        Label {
            text: qsTr("Button tile to switch automatic virtual shifting to Sprint profile.")
            font.bold: true
            font.italic: true
            font.pixelSize: Qt.application.font.pixelSize - 2
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            color: Material.color(Material.Lime)
        }        
    }
}
