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
        id: settingsPane

        Settings {
            id: settings
            property real ui_zoom: 100.0
            property bool bike_heartrate_service: false
            property int bike_resistance_offset: 4
            property real bike_resistance_gain_f: 1.0
            property int bike_resistance_start: 1
            property int age: 35.0
            property real weight: 75.0
            property real ftp: 200.0
            property bool miles_unit: false
            property bool pause_on_start: false
            property bool bike_cadence_sensor: false
            property bool run_cadence_sensor: false
            property bool bike_power_sensor: false
            property string heart_rate_belt_name: "Disabled"

            property bool ant_cadence: false
            property bool ant_heart: false
            property bool ant_garmin: false

            property bool top_bar_enabled: true

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
            property bool tile_peloton_resistance_enabled: true
            property int  tile_peloton_resistance_order: 15
            property bool tile_datetime_enabled: true
            property int  tile_datetime_order: 16
            property bool tile_target_resistance_enabled: true
            property int  tile_target_resistance_order: 15

            property real heart_rate_zone1: 70.0
            property real heart_rate_zone2: 80.0
            property real heart_rate_zone3: 90.0
            property real heart_rate_zone4: 100.0

            property real peloton_gain: 1.0
            property real peloton_offset: 0

            property bool domyos_treadmill_buttons: false
            property bool domyos_treadmill_distance_display: true

            property real domyos_elliptical_speed_ratio: 1.0

            property real proform_wheel_ratio: 0.33

            property int  fitshow_user_id: 0x006E13AA

            property bool inspire_peloton_formula: false

            property bool toorx_3_0: false
            property bool trx_route_key: false
            property bool toorx_bike: false

            property int  m3i_bike_id: 256
            property int  m3i_bike_speed_buffsize: 90
            property bool m3i_bike_qt_search: false

            property bool snode_bike: false

            property int flywheel_filter: 2

            property bool trainprogram_random: false
            property int trainprogram_total: 60
            property real trainprogram_period_seconds: 60
            property real trainprogram_speed_min: 8
            property real trainprogram_speed_max: 16
            property real trainprogram_incline_min: 0
            property real trainprogram_incline_max: 15
            property real trainprogram_resistance_min: 1
            property real trainprogram_resistance_max: 32

            property real watt_offset: 0
            property real watt_gain: 1

            property string filter_device: "Disabled"
            property string strava_suffix: "#qdomyos-zwift"

            property bool virtualbike_forceresistance: true
            property bool bluetooth_relaxed: false
            property bool battery_service: false
            property bool service_changed: false
            property bool virtual_device_enabled: true
            property bool ios_peloton_workaround: true
            property bool android_wakelock: true
            property bool log_debug: false
            property bool virtual_device_onlyheart: false
            property bool virtual_device_echelon: false
        }

        ColumnLayout {
            id: column
            spacing: 0
            anchors.fill: parent

            Label {
                Layout.preferredWidth: parent.width
                id: rebootLabel
                text: qsTr("Reboot the app in order to apply the settings")
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                verticalAlignment: Text.AlignVCenter
                color: Material.color(Material.Red)                
            }

            AccordionElement {
                id: generalOptionsAccordion
                title: qsTr("General Options")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                //width: 640
                //anchors.top: acc1.bottom
                //anchors.topMargin: 10
                accordionContent: ColumnLayout {
                    spacing: 0
                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelUiZoom
                            text: qsTr("UI Zoom:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: uiZoomTextField
                            text: settings.ui_zoom
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.ui_zoom = text
                        }
                        Button {
                            id: okUiZoomButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.ui_zoom = uiZoomTextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelWeight
                            text: qsTr("Player Weight (kg):")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: weightTextField
                            text: settings.weight
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.weight = text
                        }
                        Button {
                            id: okWeightButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.weight = weightTextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelAge
                            text: qsTr("Player Age:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: ageTextField
                            text: settings.age
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.age = text
                        }
                        Button {
                            id: okAgeButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.age = ageTextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelFTP
                            text: qsTr("FTP value:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: ftpTextField
                            text: settings.ftp
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.ftp = text
                        }
                        Button {
                            id: okFTPButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.ftp = ftpTextField.text
                        }
                    }

                    AccordionCheckElement {
                        id: heartOutsideAccordion
                        title: qsTr("Heart Rate service Enabled")
                        linkedBoolSetting: "bike_heartrate_service"
                        settings: settings
                        invert: true
                        accordionContent: ColumnLayout {
                            spacing: 10
                            RowLayout {
                                spacing: 10
                                Label {
                                    id: labelHeartRateBelt
                                    text: qsTr("Heart Belt Name:")
                                    Layout.fillWidth: true
                                }
                                ComboBox {
                                    id: heartBeltNameTextField
                                    model: rootItem.bluetoothDevices
                                    displayText: settings.heart_rate_belt_name
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onActivated: {
                                        console.log("combomodel activated" + heartBeltNameTextField.currentIndex)
                                        displayText = heartBeltNameTextField.currentValue
                                     }

                                }
                                Button {
                                    id: okHeartBeltNameButton
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: settings.heart_rate_belt_name = heartBeltNameTextField.displayText
                                }
                            }

                            Label {
                                id: appleWatchLabel
                                text: qsTr("Apple Watch users: leave it disabled! Just open the app on your watch")
                                font.bold: yes
                                font.italic: yes
                                font.pixelSize: 8
                                textFormat: Text.PlainText
                                wrapMode: Text.WordWrap
                                verticalAlignment: Text.AlignVCenter
                                color: Material.color(Material.Red)
                            }

                            Button {
                                id: refreshHeartBeltNameButton
                                text: "Refresh Devices List"
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                onClicked: refresh_bluetooth_devices_clicked();
                            }
                        }
                    }


                    SwitchDelegate {
                        id: unitDelegate
                        text: qsTr("Use Miles unit in UI")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.miles_unit
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.miles_unit = checked
                    }

                    SwitchDelegate {
                        id: pauseOnStartDelegate
                        text: qsTr("Pause when App Starts")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.pause_on_start
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.pause_on_start = checked
                    }
                }
            }

            AccordionElement {
                id: bikeBridgeOptionsAccordion
                title: qsTr("Bike Bridge Options (Peloton only!)")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                //width: 640
                //anchors.top: acc1.bottom
                //anchors.topMargin: 10
                accordionContent: ColumnLayout {
                    spacing: 0
                    SwitchDelegate {
                        id: cadenceSensorDelegate
                        text: qsTr("Cycling Cadence Sensor (Peloton compatibility)")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.bike_cadence_sensor
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.bike_cadence_sensor = checked
                    }

                    SwitchDelegate {
                        id: runCadenceSensorDelegate
                        text: qsTr("Run Cadence Sensor (Peloton compatibility)")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.run_cadence_sensor
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.run_cadence_sensor = checked
                    }
                }
            }

            /*Label {
                id: zwiftInfoLabel
                text: qsTr("Zwift users: keep this setting off")
                font.bold: yes
                font.italic: yes
                font.pixelSize: 8
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                verticalAlignment: Text.AlignVCenter
                color: Material.color(Material.Red)
            }*/

            AccordionElement {
                id: bikeResistanceOptionsAccordion
                title: qsTr("Bike resistance Options")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                //width: 640
                //anchors.top: acc1.bottom
                //anchors.topMargin: 10
                accordionContent: ColumnLayout {
                    spacing: 0
                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelBikeResistanceOffset
                            text: qsTr("Zwift Resistance Offset:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: bikeResistanceOffsetTextField
                            text: settings.bike_resistance_offset
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.bike_resistance_offset = text
                        }
                        Button {
                            id: okBikeResistanceOffsetButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.bike_resistance_offset = bikeResistanceOffsetTextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelBikeResistanceGain
                            text: qsTr("Zwift Resistance Gain:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: bikeResistanceGainTextField
                            text: settings.bike_resistance_gain_f
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.bike_resistance_gain_f = text
                        }
                        Button {
                            id: okBikeResistanceGainButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.bike_resistance_gain_f = bikeResistanceGainTextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelBikeResistanceStart
                            text: qsTr("Resistance at Startup:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: bikeResistanceStartTextField
                            text: settings.bike_resistance_start
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.bike_resistance_start = text
                        }
                        Button {
                            id: okBikeResistanceStartButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.bike_resistance_start = bikeResistanceStartTextField.text
                        }
                    }
                }
            }

            AccordionElement {
                id: uiAntOptionsAccordion
                title: qsTr("Ant+ Options (only for some Android)")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                //width: 640
                //anchors.top: acc1.bottom
                //anchors.topMargin: 10
                accordionContent:  ColumnLayout {
                    spacing: 0
                    SwitchDelegate {
                        id: antCadenceDelegate
                        text: qsTr("Ant+ Cadence")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.ant_cadence
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.ant_cadence = checked
                    }

                    SwitchDelegate {
                        id: antHeartDelegate
                        text: qsTr("Ant+ Heart")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.ant_heart
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.ant_heart = checked
                    }
                }
            }
/*
            SwitchDelegate {
                id: antGarminDelegate
                text: qsTr("Ant+ Garmin Compatibility")
                spacing: 0
                bottomPadding: 0
                topPadding: 0
                rightPadding: 0
                leftPadding: 0
                clip: false
                checked: settings.ant_garmin
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: true
                onClicked: settings.ant_garmin = checked
            }*/

            AccordionElement {
                id: uiGeneralOptionsAccordion
                title: qsTr("General UI Options")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                //width: 640
                //anchors.top: acc1.bottom
                //anchors.topMargin: 10
                accordionContent: ColumnLayout {
                    spacing: 0
                    SwitchDelegate {
                        id: topBarEnabledDelegate
                        text: qsTr("Top Bar Enabled")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.top_bar_enabled
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.top_bar_enabled = checked
                    }

                    AccordionElement {
                        id: tileOptionsAccordion
                        title: qsTr("Tiles Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Grey)
                        color: Material.backgroundColor
                        //width: 640
                        //anchors.top: acc1.bottom
                        //anchors.topMargin: 10
                        accordionContent: ColumnLayout {
                            spacing: 0
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
                                accordionContent: RowLayout {
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
                                accordionContent: RowLayout {
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
                        }
                    }
                }
            }

            AccordionElement {
                id: heartRateZoneAccordion
                title: qsTr("Heart Rate Zone Options")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                accordionContent: ColumnLayout {
                    spacing: 0
                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelheartRateZone1Ratio
                            text: qsTr("Zone 1 %:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: heartRateZone1TextField
                            text: settings.heart_rate_zone1
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.heart_rate_zone1 = text
                        }
                        Button {
                            id: okHeartRateZone1Button
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.heart_rate_zone1 = heartRateZone1TextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelheartRateZone2Ratio
                            text: qsTr("Zone 2 %:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: heartRateZone2TextField
                            text: settings.heart_rate_zone2
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.heart_rate_zone2 = text
                        }
                        Button {
                            id: okHeartRateZone2Button
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.heart_rate_zone2 = heartRateZone2TextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelheartRateZone3Ratio
                            text: qsTr("Zone 3 %:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: heartRateZone3TextField
                            text: settings.heart_rate_zone3
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.heart_rate_zone3 = text
                        }
                        Button {
                            id: okHeartRateZone3Button
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.heart_rate_zone3 = heartRateZone3TextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelheartRateZone4Ratio
                            text: qsTr("Zone 4 %:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: heartRateZone4TextField
                            text: settings.heart_rate_zone4
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.heart_rate_zone4 = text
                        }
                        Button {
                            id: okHeartRateZone4Button
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.heart_rate_zone4 = heartRateZone4TextField.text
                        }
                    }
                }
            }
            AccordionElement {
                id: pelotonAccordion
                title: qsTr("Peloton Options")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                accordionContent: ColumnLayout {
                    spacing: 0
                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelPelotonGain
                            text: qsTr("Conversion Gain:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: pelotonGainTextField
                            text: settings.peloton_gain
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.peloton_gain = text
                        }
                        Button {
                            id: okPelotonGainButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.peloton_gain = pelotonGainTextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelPelotonOffset
                            text: qsTr("Conversion Offset:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: pelotonOffsetTextField
                            text: settings.peloton_offset
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.peloton_offset = text
                        }
                        Button {
                            id: okPelotonOffsetButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.peloton_offset = pelotonOffsetTextField.text
                        }
                    }
                }
            }
            AccordionElement {
                id: domyosTreadmillAccordion
                title: qsTr("Domyos Treadmill Options")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                accordionContent: ColumnLayout {
                    spacing: 0
                    SwitchDelegate {
                        id: domyosTreadmillButtonsDelegate
                        text: qsTr("Speed/Inclination Buttons")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.domyos_treadmill_buttons
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.domyos_treadmill_buttons = checked
                    }

                    SwitchDelegate {
                        id: domyosTreadmillDistanceDisplayDelegate
                        text: qsTr("Distance on Console")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.domyos_treadmill_distance_display
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.domyos_treadmill_distance_display = checked
                    }
                }
            }

            AccordionElement {
                id: inspireBikeAccordion
                title: qsTr("Inspire Bike Options")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                accordionContent: SwitchDelegate {
                    id: inspirePelotonFormulaDelegate
                    text: qsTr("Advanced Peloton Formula")
                    spacing: 0
                    bottomPadding: 0
                    topPadding: 0
                    rightPadding: 0
                    leftPadding: 0
                    clip: false
                    checked: settings.inspire_peloton_formula
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    Layout.fillWidth: true
                    onClicked: settings.inspire_peloton_formula = checked
                }
            }
            AccordionElement {
                id: toorxTreadmillAccordion
                title: qsTr("Toorx Treadmill Options")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                accordionContent: ColumnLayout {
                    spacing: 0
                    SwitchDelegate {
                        id: toorxRouteKeyDelegate
                        text: qsTr("TRX ROUTE KEY Compatibility")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.trx_route_key
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.trx_route_key = checked
                    }

                    SwitchDelegate {
                        id: toorxTreadmill30Delegate
                        text: qsTr("Toorx 3.0 Compatibility")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.toorx_3_0
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.toorx_3_0 = checked
                    }

                    SwitchDelegate {
                        id: toorxBikeDelegate
                        text: qsTr("Toorx/iConsole Bike")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.toorx_bike
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.toorx_bike = checked
                    }
                }
            }
            AccordionElement {
                id: snodeBikeAccordion
                title: qsTr("Snode Bike Options")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                accordionContent: SwitchDelegate {
                    id: snodeBikeDelegate
                    text: qsTr("Snode Bike")
                    spacing: 0
                    bottomPadding: 0
                    topPadding: 0
                    rightPadding: 0
                    leftPadding: 0
                    clip: false
                    checked: settings.snode_bike
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    Layout.fillWidth: true
                    onClicked: settings.snode_bike = checked
                }
            }
            AccordionElement {
                id: flywheelBikeAccordion
                title: qsTr("Flywheel Bike Options")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                accordionContent: RowLayout {
                    spacing: 10
                    Label {
                        id: labelflywheelBikeFilter
                        text: qsTr("Samples Filter:")
                        Layout.fillWidth: true
                    }
                    TextField {
                        id: flywheelBikeFilterTextField
                        text: settings.flywheel_filter
                        horizontalAlignment: Text.AlignRight
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        inputMethodHints: Qt.ImhDigitsOnly
                        onAccepted: settings.flywheel_filter = text
                    }
                    Button {
                        id: okflywheelBikeFilterButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.flywheel_filter = flywheelBikeFilterTextField.text
                    }
                }
            }
            AccordionElement {
                id: domyosEllipticalAccordion
                title: qsTr("Domyos Elliptical Options")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                accordionContent: RowLayout {
                    spacing: 10
                    Label {
                        id: labelDomyosEllipticalSpeedRatio
                        text: qsTr("Speed Ratio:")
                        Layout.fillWidth: true
                    }
                    TextField {
                        id: domyosEllipticalSpeedRatioTextField
                        text: settings.domyos_elliptical_speed_ratio
                        horizontalAlignment: Text.AlignRight
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                        onAccepted: settings.domyos_elliptical_speed_ratio = text
                    }
                    Button {
                        id: okDomyosEllipticalRatioButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.domyos_elliptical_speed_ratio = domyosEllipticalSpeedRatioTextField.text
                    }
                }
            }
            AccordionElement {
                id: proformBikeAccordion
                title: qsTr("Proform Bike Options")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                accordionContent: RowLayout {
                    spacing: 10
                    Label {
                        id: labelproformBikeWheelRatio
                        text: qsTr("Wheel Ratio:")
                        Layout.fillWidth: true
                    }
                    TextField {
                        id: proformBikeWheelRatioTextField
                        text: settings.proform_wheel_ratio
                        horizontalAlignment: Text.AlignRight
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                        onAccepted: settings.proform_wheel_ratio = text
                    }
                    Button {
                        id: okproformBikeWheelRatioButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.proform_wheel_ratio = proformBikeWheelRatioTextField.text
                    }
                }
            }
            AccordionElement {
                id: fitshowAccordion
                title: qsTr("Fitshow Treadmill Options")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                accordionContent: RowLayout {
                    spacing: 10
                    Label {
                        id: labelfitshowTreadmillUserId
                        text: qsTr("User ID:")
                        Layout.fillWidth: true
                    }
                    TextField {
                        id: fitshowTreadmillUserIdTextField
                        text: settings.fitshow_user_id
                        horizontalAlignment: Text.AlignRight
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        inputMethodHints: Qt.ImhDigitsOnly
                        onAccepted: settings.fitshow_user_id = text
                    }
                    Button {
                        id: okfitshowTreadmillUserIdButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.fitshow_user_id = fitshowTreadmillUserIdTextField.text
                    }
                }
            }
            AccordionElement {
                id: m3iBikeAccordion
                title: qsTr("M3i Bike Options")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                //width: 640
                //anchors.top: acc1.bottom
                //anchors.topMargin: 10
                accordionContent: ColumnLayout {
                    spacing: 0
                    SwitchDelegate {
                        id: m3iBikeQtSearchDelegate
                        text: qsTr("Use QT search on Android / iOS")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.m3i_bike_qt_search
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.m3i_bike_qt_search = checked
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelm3iBikeId
                            text: qsTr("Bike ID:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: m3iBikeIdTextField
                            text: settings.m3i_bike_id
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.m3i_bike_id = text
                        }
                        Button {
                            id: okm3iBikeIdButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.m3i_bike_id = m3iBikeIdTextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelm3iBikeSpeedBuffsize
                            text: qsTr("Speed Buffer Size:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: m3iBikeSpeedBuffsizeTextField
                            text: settings.m3i_bike_speed_buffsize
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.m3i_bike_speed_buffsize = text
                        }
                        Button {
                            id: okm3iBikeSpeedBuffsizeButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.m3i_bike_speed_buffsize = m3iBikeSpeedBuffsizeTextField.text
                        }
                    }
                }
            }
            AccordionCheckElement {
                id: trainingProgramRandomAccordion
                title: qsTr("Training Program Random Options")
                linkedBoolSetting: "trainprogram_random"
                settings: settings
                accordionContent: ColumnLayout {
                    spacing: 0
                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelTrainProgramRandomDuration
                            text: qsTr("Duration (minutes):")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: trainProgramRandomDurationTextField
                            text: settings.trainprogram_total
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.trainprogram_total = text
                        }
                        Button {
                            id: okTrainProgramRandomDuration
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.trainprogram_total = trainProgramRandomDurationTextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelTrainProgramRandomPeriod
                            text: qsTr("Period (seconds):")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: trainProgramRandomPeriodTextField
                            text: settings.trainprogram_period_seconds
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.trainprogram_period_seconds = text
                        }
                        Button {
                            id: okTrainProgramRandomPeriod
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.trainprogram_period_seconds = trainProgramRandomPeriodTextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelTrainProgramRandomSpeedMin
                            text: qsTr("Speed min.:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: trainProgramRandomSpeedMinTextField
                            text: settings.trainprogram_speed_min
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.trainprogram_speed_min = text
                        }
                        Button {
                            id: okTrainProgramRandomSpeedMin
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.trainprogram_speed_min = trainProgramRandomSpeedMinTextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelTrainProgramRandomSpeedMax
                            text: qsTr("Speed max.:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: trainProgramRandomSpeedMaxTextField
                            text: settings.trainprogram_speed_max
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.trainprogram_speed_max = text
                        }
                        Button {
                            id: okTrainProgramRandomSpeedMax
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.trainprogram_speed_max = trainProgramRandomSpeedMaxTextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelTrainProgramRandomInclineMin
                            text: qsTr("Incline min.:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: trainProgramRandomInclineMinTextField
                            text: settings.trainprogram_incline_min
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.trainprogram_incline_min = text
                        }
                        Button {
                            id: okTrainProgramRandomInclineMin
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.trainprogram_incline_min = trainProgramRandomInclineMinTextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelTrainProgramRandomInclineMax
                            text: qsTr("Incline max.:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: trainProgramRandomInclineMaxTextField
                            text: settings.trainprogram_incline_max
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.trainprogram_incline_max = text
                        }
                        Button {
                            id: okTrainProgramRandomInclineMax
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.trainprogram_incline_max = trainProgramRandomInclineMaxTextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelTrainProgramRandomResistanceMin
                            text: qsTr("Resistance min.:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: trainProgramRandomResistanceMinTextField
                            text: settings.trainprogram_resistance_min
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.trainprogram_resistance_min = text
                        }
                        Button {
                            id: okTrainProgramRandomResistanceMin
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.trainprogram_resistance_min = trainProgramRandomResistanceMinTextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelTrainProgramRandomResistanceMax
                            text: qsTr("Resistance max.:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: trainProgramRandomResistanceMaxTextField
                            text: settings.trainprogram_resistance_max
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.trainprogram_resistance_max = text
                        }
                        Button {
                            id: okTrainProgramRandomResistanceMax
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.trainprogram_resistance_max = trainProgramRandomResistanceMaxTextField.text
                        }
                    }
                }
            }

            AccordionElement {
                id: advancedSettingsAccordion
                title: qsTr("Advanced Settings")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                //width: 640
                //anchors.top: acc1.bottom
                //anchors.topMargin: 10
                accordionContent: ColumnLayout {
                    spacing: 0
                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelFilterDevice
                            text: qsTr("Manual Device:")
                            Layout.fillWidth: true
                        }
                        ComboBox {
                            id: filterDeviceTextField
                            model: rootItem.bluetoothDevices
                            displayText: settings.filter_device
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onActivated: {
                                console.log("combomodel activated" + filterDeviceTextField.currentIndex)
                                displayText = filterDeviceTextField.currentValue
                             }

                        }
                        Button {
                            id: okFilterDeviceButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.filter_device = filterDeviceTextField.displayText
                        }
                    }

                    Button {
                        id: refreshFilterDeviceButton
                        text: "Refresh Devices List"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: refresh_bluetooth_devices_clicked();
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelwattOffset
                            text: qsTr("Watt Offset (only <0):")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: wattOffsetTextField
                            text: settings.watt_offset
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.watt_offset = text
                        }
                        Button {
                            id: okwattOffsetButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.watt_offset = wattOffsetTextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelwattGain
                            text: qsTr("Watt Gain (max value 1.25):")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: wattGainTextField
                            text: settings.watt_gain
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.watt_gain = text
                        }
                        Button {
                            id: okWattGainButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.watt_gain = wattGainTextField.text
                        }
                    }

                    Label {
                        id: stravaLabel
                        text: qsTr("Strava")
                        textFormat: Text.PlainText
                        wrapMode: Text.WordWrap
                        verticalAlignment: Text.AlignVCenter
                        color: Material.color(Material.Grey)
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelStravaSuffix
                            text: qsTr("Suffix activity:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: stravaSuffixTextField
                            text: settings.strava_suffix
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onAccepted: settings.strava_suffix = text
                        }
                        Button {
                            id: okStravaSuffixButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.strava_suffix = stravaSuffixTextField.text
                        }
                    }
                }
            }

            AccordionElement {
                id: experimentalFeatureAccordion
                title: qsTr("Experimental Features")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                //width: 640
                //anchors.top: acc1.bottom
                //anchors.topMargin: 10
                accordionContent: ColumnLayout {
                    spacing: 0
                    SwitchDelegate {
                        id: bluetoothRelaxedDelegate
                        text: qsTr("Relaxed Bluetooth for mad devices")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.bluetooth_relaxed
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.bluetooth_relaxed = checked
                    }

                    SwitchDelegate {
                        id: batteryServiceDelegate
                        text: qsTr("Simulate Battery Service")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.battery_service
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.battery_service = checked
                    }
        /*
                    SwitchDelegate {
                        id: serviceChangedDelegate
                        text: qsTr("Service Changed Service")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.service_changed
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.service_changed = checked
                    }
        */
                    SwitchDelegate {
                        id: virtualDeviceEnabledDelegate
                        text: qsTr("Virtual Device")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.virtual_device_enabled
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.virtual_device_enabled = checked
                    }

                    SwitchDelegate {
                        id: virtualDeviceOnlyHeartDelegate
                        text: qsTr("Virtual Heart Only")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.virtual_device_onlyheart
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.virtual_device_onlyheart = checked
                    }

                    SwitchDelegate {
                        id: virtualDeviceEchelonDelegate
                        text: qsTr("Virtual Echelon")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.virtual_device_echelon
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.virtual_device_echelon = checked
                    }

                    SwitchDelegate {
                        id: virtualBikeForceResistanceDelegate
                        text: qsTr("Zwift Force Resistance")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.virtualbike_forceresistance
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.virtualbike_forceresistance = checked
                    }

                    SwitchDelegate {
                        id: bikePowerSensorDelegate
                        text: qsTr("Bike Power Sensor")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.bike_power_sensor
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.bike_power_sensor = checked
                    }

                    SwitchDelegate {
                        id: iosPelotonWorkaroundDelegate
                        text: qsTr("iOS Peloton Workaround")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.ios_peloton_workaround
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.ios_peloton_workaround = checked
                    }

                    SwitchDelegate {
                        id: androidWakeLockDelegate
                        text: qsTr("Android WakeLock")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.android_wakelock
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.android_wakelock = checked
                    }

                    SwitchDelegate {
                        id: logDebugDelegate
                        text: qsTr("Debug Log")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.log_debug
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.log_debug = checked
                    }
                }
            }
        }
    }

/*##^##
Designer {
    D{i:0;formeditorZoom:0.6600000262260437}
}
##^##*/
