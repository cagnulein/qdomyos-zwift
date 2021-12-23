import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0

//Page {
    ScrollView {
        contentWidth: -1
        focus: true
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.fill: parent
        //anchors.bottom: footerSettings.top
        //anchors.bottomMargin: footerSettings.height + 10
        id: settingsPane

        Settings {
            id: settings
            property real ui_zoom: 100.0
            property bool bike_heartrate_service: false
            property int bike_resistance_offset: 4
            property real bike_resistance_gain_f: 1.0
            property bool zwift_erg: false
            property real zwift_erg_filter: 10.0
            property real zwift_erg_filter_down: 10.0
            property bool zwift_negative_inclination_x2: false

            property bool speed_power_based: false
            property int bike_resistance_start: 1
            property int age: 35.0
            property real weight: 75.0
            property real ftp: 200.0
            property string user_email: ""
            property string user_nickname: ""
            property bool miles_unit: false
            property bool pause_on_start: false
            property bool pause_on_start_treadmill: false
            property bool continuous_moving: false
            property bool bike_cadence_sensor: false
            property bool run_cadence_sensor: false
            property bool bike_power_sensor: false
            property string heart_rate_belt_name: "Disabled"
            property bool heart_ignore_builtin: false
            property bool kcal_ignore_builtin: false

            property bool ant_cadence: false
            property bool ant_heart: false
            property bool ant_garmin: false

            property bool top_bar_enabled: true

            property string peloton_username: "username"
            property string peloton_password: "password"
            property string peloton_difficulty: "lower"
            property string peloton_cadence_metric: "Cadence"
            property string peloton_heartrate_metric: "Heart Rate"
            property string peloton_date: "Before Title"
            property bool peloton_description_link: true

            property string pzp_username: "username"
            property string pzp_password: "username"

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
            property bool tile_mets_enabled: false
            property int  tile_mets_order: 28
            property bool tile_targetmets_enabled: false
            property int  tile_targetmets_order: 29
            property bool tile_steering_angle_enabled: false
            property int  tile_steering_angle_order: 30

            property real heart_rate_zone1: 70.0
            property real heart_rate_zone2: 80.0
            property real heart_rate_zone3: 90.0
            property real heart_rate_zone4: 100.0
            property bool heart_max_override_enable: false
            property real heart_max_override_value: 195.0

            property real peloton_gain: 1.0
            property real peloton_offset: 0

            property string treadmill_pid_heart_zone: "Disabled"

            property bool domyos_treadmill_buttons: false
            property bool domyos_treadmill_distance_display: true
            property bool domyos_treadmill_display_invert: false

            property real domyos_bike_cadence_filter: 0.0            
            property bool domyos_bike_display_calories: true

            property real domyos_elliptical_speed_ratio: 1.0

            property bool eslinker_cadenza: true

            property string echelon_watttable: "Echelon"

            property real proform_wheel_ratio: 0.33
            property bool proform_tour_de_france_clc: false
            property bool proform_tdf_jonseed_watt: false
            property bool proform_studio: false

            property double horizon_gr7_cadence_multiplier: 1.0

            property int  fitshow_user_id: 0x006E13AA

            property bool inspire_peloton_formula: false
            property bool inspire_peloton_formula2: false

            property bool hammer_racer_s: false

            property bool yesoul_peloton_formula: false

            property bool nordictrack_10_treadmill: true
            //property bool proform_treadmill_995i: false

            property bool toorx_3_0: false
            property bool jtx_fitness_sprint_treadmill: false
            property bool dkn_endurun_treadmill: false
            property bool trx_route_key: false
            property bool bh_spada_2: false
            property bool toorx_bike: false
            property bool jll_IC400_bike: false
            property bool fytter_ri08_bike: false
            property bool asviva_bike: false
            property bool hertz_xr_770: false

            property int  m3i_bike_id: 256
            property int  m3i_bike_speed_buffsize: 90
            property bool m3i_bike_qt_search: false
            property bool m3i_bike_kcal: true

            property bool snode_bike: false
            property bool fitplus_bike: false

            property int flywheel_filter: 2
            property bool flywheel_life_fitness_ic8: false

            property bool sole_treadmill_inclination: false
            property bool sole_treadmill_miles: true
            property bool sole_treadmill_f65: false

            property bool schwinn_bike_resistance: false

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
            property bool power_avg_5s: false
            property bool instant_power_on_pause: false

            property real speed_offset: 0
            property real speed_gain: 1

            property string filter_device: "Disabled"
            property string strava_suffix: "#QZ"

            property string cadence_sensor_name: "Disabled"
            property bool cadence_sensor_as_bike: false
            property real cadence_sensor_speed_ratio: 0.33
            property real power_hr_pwr1: 200
            property real power_hr_hr1: 150
            property real power_hr_pwr2: 230
            property real power_hr_hr2: 170

            property string power_sensor_name: "Disabled"
            property bool power_sensor_as_bike: false
            property bool power_sensor_as_treadmill: false
            property bool powr_sensor_running_cadence_double: false

            property string elite_rizer_name: "Disabled"
            property string elite_sterzo_smart_name: "Disabled"

            property string ftms_accessory_name: "Disabled"
            property real ss2k_shift_step: 900

            property bool fitmetria_fanfit_enable: false
            property string fitmetria_fanfit_mode: "Heart"
            property real fitmetria_fanfit_min: 0
            property real fitmetria_fanfit_max: 100

            property bool virtualbike_forceresistance: true
            property bool bluetooth_relaxed: false
            property bool bluetooth_30m_hangs: false
            property bool battery_service: false
            property bool service_changed: false
            property bool virtual_device_enabled: true
            property bool ios_peloton_workaround: true
            property bool android_wakelock: true
            property bool log_debug: false
            property bool virtual_device_onlyheart: false
            property bool virtual_device_echelon: false
            property bool virtual_device_ifit: false
            property bool virtual_device_rower: false
            property bool virtual_device_force_bike: false
            property bool volume_change_gears: false
            property bool applewatch_fakedevice: false
        }

        ColumnLayout {
            id: column1
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
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
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
                            text: qsTr("Player Weight") + "(" + (settings.miles_unit?"lbs":"kg") + ")"
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: weightTextField
                            text: (settings.miles_unit?settings.weight * 2.20462:settings.weight)
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.weight = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okWeightButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.weight = (settings.miles_unit?weightTextField.text / 2.20462:weightTextField.text)
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
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
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
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okFTPButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.ftp = ftpTextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelNickname
                            text: qsTr("Nickname:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: nicknameTextField
                            text: settings.user_nickname
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onAccepted: settings.user_nickname = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okNicknameButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.user_nickname = nicknameTextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelEmail
                            text: qsTr("Email:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: emailTextField
                            text: settings.user_email
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onAccepted: settings.user_email = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okEmailButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.user_email = emailTextField.text
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

                    SwitchDelegate {
                        id: continuousMovingDelegate
                        text: qsTr("Continuous Moving")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.continuous_moving
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.continuous_moving = checked
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
                id: heartRateOptionsAccordion
                title: qsTr("Heart Rate Options")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                accordionContent: ColumnLayout {
                    spacing: 10
                    SwitchDelegate {
                        id: switchDelegate
                        text: qsTr("Heart Rate service outside FTMS")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.bike_heartrate_service
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.bike_heartrate_service = checked
                    }
                    SwitchDelegate {
                        id: switchBultinDelegate
                        text: qsTr("Disable HRM from Machinery")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.heart_ignore_builtin
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.heart_ignore_builtin = checked
                    }
                    SwitchDelegate {
                        id: switchBultinKcalDelegate
                        text: qsTr("Disable KCal from Machinery")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.kcal_ignore_builtin
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.kcal_ignore_builtin = checked
                    }
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
                            onClicked: settings.heart_rate_belt_name = heartBeltNameTextField.displayText;
                        }
                    }

                    Label {
                        id: appleWatchLabel
                        text: qsTr("Apple Watch users: leave it disabled! Just open the app on your watch")
                        font.bold: true
                        font.italic: true
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

                    AccordionElement {
                        id: heartRateZoneAccordion
                        title: qsTr("Heart Rate Zone Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
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
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
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
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
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
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
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
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    id: okHeartRateZone4Button
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: settings.heart_rate_zone4 = heartRateZone4TextField.text
                                }
                            }

                            AccordionElement {
                                id: heartRatemaxOverrideAccordion
                                title: qsTr("Heart Rate Max Override")
                                indicatRectColor: Material.color(Material.Grey)
                                textColor: Material.color(Material.Red)
                                color: Material.backgroundColor
                                accordionContent: ColumnLayout {
                                    spacing: 10
                                    SwitchDelegate {
                                        id: heartRateMaxOverrideDelegate
                                        text: qsTr("Override Heart Rate Max Calc.")
                                        spacing: 0
                                        bottomPadding: 0
                                        topPadding: 0
                                        rightPadding: 0
                                        leftPadding: 0
                                        clip: false
                                        checked: settings.heart_max_override_enable
                                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                        Layout.fillWidth: true
                                        onClicked: settings.heart_max_override_enable = checked
                                    }

                                    RowLayout {
                                        spacing: 10
                                        Label {
                                            id: labelHeartRateMaxOverrideValue
                                            text: qsTr("Max Heart Rate")
                                            Layout.fillWidth: true
                                        }
                                        TextField {
                                            id: heartRateMaxOverrideValueTextField
                                            text: settings.heart_max_override_value
                                            horizontalAlignment: Text.AlignRight
                                            Layout.fillHeight: false
                                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                            inputMethodHints: Qt.ImhDigitsOnly
                                            onAccepted: settings.heart_max_override_value = text
                                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                        }
                                        Button {
                                            id: okHeartRateMaxOverrideValue
                                            text: "OK"
                                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                            onClicked: settings.heart_max_override_value = heartRateMaxOverrideValueTextField.text
                                        }
                                    }
                                }
                            }
                        }
                    }

                    AccordionElement {
                        id: powerFromHeartRateAccordion
                        title: qsTr("Power from Heart Rate Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            spacing: 0
                            RowLayout {
                                spacing: 10
                                Label {
                                    id: labelPowerFromHeartPWR1
                                    text: qsTr("Session 1 Watt:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: powerFromHeartPWR1TextField
                                    text: settings.power_hr_pwr1
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                                    onAccepted: settings.power_hr_pwr1 = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    id: okPowerFromHeartPWR1
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: settings.power_hr_pwr1 = powerFromHeartPWR1TextField.text
                                }
                            }

                            RowLayout {
                                spacing: 10
                                Label {
                                    id: labelPowerFromHeartHR1
                                    text: qsTr("Session 1 HR:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: powerFromHeartHR1TextField
                                    text: settings.power_hr_hr1
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                                    onAccepted: settings.power_hr_hr1 = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    id: okPowerFromHeartHR1
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: settings.power_hr_hr1 = powerFromHeartHR1TextField.text
                                }
                            }

                            RowLayout {
                                spacing: 10
                                Label {
                                    id: labelPowerFromHeartPWR2
                                    text: qsTr("Session 2 Watt:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: powerFromHeartPWR2TextField
                                    text: settings.power_hr_pwr2
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                                    onAccepted: settings.power_hr_pwr2 = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    id: okPowerFromHeartPWR2
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: settings.power_hr_pwr2 = powerFromHeartPWR2TextField.text
                                }
                            }

                            RowLayout {
                                spacing: 10
                                Label {
                                    id: labelPowerFromHeartHR2
                                    text: qsTr("Session 2 HR:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: powerFromHeartHR2TextField
                                    text: settings.power_hr_hr2
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                                    onAccepted: settings.power_hr_hr2 = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    id: okPowerFromHeartHR2
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: settings.power_hr_hr2 = powerFromHeartHR2TextField.text
                                }
                            }
                        }
                    }
                }
            }

            AccordionElement {
                id: bikeOptionsAccordion
                title: qsTr("Bike Options")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                //width: 640
                //anchors.top: acc1.bottom
                //anchors.topMargin: 10
                accordionContent: ColumnLayout {
                    spacing: 0
                    SwitchDelegate {
                        id: speedPowerBasedDelegate
                        text: qsTr("Speed calculates on Power")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.speed_power_based
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.speed_power_based = checked
                    }
                    SwitchDelegate {
                        id: zwiftErgDelegate
                        text: qsTr("Zwift Workout/Erg Mode")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.zwift_erg
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.zwift_erg = checked
                    }
                    SwitchDelegate {
                        id: zwiftNegativeIncliantionX2Delegate
                        text: qsTr("Double Negative Inclination")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.zwift_negative_inclination_x2
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.zwift_negative_inclination_x2 = checked
                    }
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
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
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
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
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
                            id: labelZwiftErgFilter
                            text: qsTr("Zwift ERG Watt Up Filter:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: zwiftErgFilterTextField
                            text: settings.zwift_erg_filter
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.zwift_erg_filter = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okzwiftErgFilterButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.zwift_erg_filter = zwiftErgFilterTextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelZwiftErgDownFilter
                            text: qsTr("Zwift ERG Watt Down Filter:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: zwiftErgDownFilterTextField
                            text: settings.zwift_erg_filter_down
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.zwift_erg_filter_down = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okzwiftErgDownFilterButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.zwift_erg_filter_down = zwiftErgDownFilterTextField.text
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
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okBikeResistanceStartButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.bike_resistance_start = bikeResistanceStartTextField.text
                        }
                    }
                }

                AccordionElement {
                    id: schwinnBikeAccordion
                    title: qsTr("Schwinn Bike Options")
                    indicatRectColor: Material.color(Material.Grey)
                    textColor: Material.color(Material.Yellow)
                    color: Material.backgroundColor
                    accordionContent: ColumnLayout {
                        spacing: 0
                        SwitchDelegate {
                            id: schwinnBikeResistanceDelegate
                            text: qsTr("Calc. Resistance")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.schwinn_bike_resistance
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.schwinn_bike_resistance = checked
                        }
                    }
                }
                AccordionElement {
                    id: horizonBikeAccordion
                    title: qsTr("Horizon Bike Options")
                    indicatRectColor: Material.color(Material.Grey)
                    textColor: Material.color(Material.Yellow)
                    color: Material.backgroundColor
                    accordionContent: RowLayout {
                        spacing: 10
                        Label {
                            id: labelHorizonGr7CadenceMultiplier
                            text: qsTr("GR7 Cadence Multiplier:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: horizonGr7CadenceMultiplierTextField
                            text: settings.horizon_gr7_cadence_multiplier
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.horizon_gr7_cadence_multiplier = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okhorizonGr7CadenceMultiplierButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.horizon_gr7_cadence_multiplier = horizonGr7CadenceMultiplierTextField.text
                        }
                    }
                }

                AccordionElement {
                    id: echelonBikeOptionsAccordion
                    title: qsTr("Echelon Bike Options")
                    indicatRectColor: Material.color(Material.Grey)
                    textColor: Material.color(Material.Yellow)
                    color: Material.backgroundColor
                    //width: 640
                    //anchors.top: acc1.bottom
                    //anchors.topMargin: 10
                    accordionContent: ColumnLayout {
                        spacing: 0
                        RowLayout {
                            spacing: 10
                            Label {
                                id: labelEchelonWattTable
                                text: qsTr("Watt Profile:")
                                Layout.fillWidth: true
                            }
                            ComboBox {
                                id: echelonWattTableTextField
                                model: [ "Echelon", "mgarcea" ]
                                displayText: settings.echelon_watttable
                                Layout.fillHeight: false
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                onActivated: {
                                    console.log("combomodel activated" + echelonWattTableTextField.currentIndex)
                                    displayText = echelonWattTableTextField.currentValue
                                 }

                            }
                            Button {
                                id: okEchelonWattTable
                                text: "OK"
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                onClicked: settings.echelon_watttable = echelonWattTableTextField.displayText
                            }
                        }
                    }
                }

                AccordionElement {
                    id: inspireBikeAccordion
                    title: qsTr("Inspire Bike Options")
                    indicatRectColor: Material.color(Material.Grey)
                    textColor: Material.color(Material.Yellow)
                    color: Material.backgroundColor
                    accordionContent: ColumnLayout {
                        spacing: 0
                    SwitchDelegate {
                        id: inspirePelotonFormulaDelegate
                        text: qsTr("Advanced Formula (15/3/2021)")
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
                    SwitchDelegate {
                        id: inspirePelotonFormula2Delegate
                        text: qsTr("Advanced Formula (14/7/2021)")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.inspire_peloton_formula2
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.inspire_peloton_formula2 = checked
                    }
                    }
                }

                AccordionElement {
                    id: hammerBikeAccordion
                    title: qsTr("Hammer Racer Bike Options")
                    indicatRectColor: Material.color(Material.Grey)
                    textColor: Material.color(Material.Yellow)
                    color: Material.backgroundColor
                    accordionContent: SwitchDelegate {
                        id: hammerBikeDelegate
                        text: qsTr("Enable support")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.hammer_racer_s
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.hammer_racer_s = checked
                    }
                }
                AccordionElement {
                    id: yesoulBikeAccordion
                    title: qsTr("Yesoul Bike Options")
                    indicatRectColor: Material.color(Material.Grey)
                    textColor: Material.color(Material.Yellow)
                    color: Material.backgroundColor
                    accordionContent: SwitchDelegate {
                        id: yesoulBikeDelegate
                        text: qsTr("Yesoul New Peloton Formula")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.yesoul_peloton_formula
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.yesoul_peloton_formula = checked
                    }
                }

                AccordionElement {
                    id: snodeBikeAccordion
                    title: qsTr("Snode Bike Options")
                    indicatRectColor: Material.color(Material.Grey)
                    textColor: Material.color(Material.Yellow)
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
                    id: fitplusBikeAccordion
                    title: qsTr("Fitplus Bike Options")
                    indicatRectColor: Material.color(Material.Grey)
                    textColor: Material.color(Material.Yellow)
                    color: Material.backgroundColor
                    accordionContent: SwitchDelegate {
                        id: fitplusBikeDelegate
                        text: qsTr("Fit Plus Bike")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.fitplus_bike
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.fitplus_bike = checked
                    }
                }
                AccordionElement {
                    id: flywheelBikeAccordion
                    title: qsTr("Flywheel Bike Options")
                    indicatRectColor: Material.color(Material.Grey)
                    textColor: Material.color(Material.Yellow)
                    color: Material.backgroundColor
                    accordionContent: ColumnLayout {
                        spacing: 0
                        RowLayout {
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
                                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                            }
                            Button {
                                id: okflywheelBikeFilterButton
                                text: "OK"
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                onClicked: settings.flywheel_filter = flywheelBikeFilterTextField.text
                            }
                        }
                        SwitchDelegate {
                            id: lifeFitnessIC8Delegate
                            text: qsTr("Life Fitness IC8")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.flywheel_life_fitness_ic8
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.flywheel_life_fitness_ic8 = checked
                        }
                    }
                }
                AccordionElement {
                    id: domyosBikeAccordion
                    title: qsTr("Domyos Bike Options")
                    indicatRectColor: Material.color(Material.Grey)
                    textColor: Material.color(Material.Yellow)
                    color: Material.backgroundColor
                    accordionContent: RowLayout {
                        spacing: 10
                        Label {
                            id: labelDomyosBikeCadenceFilter
                            text: qsTr("Cadence Filter:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: domyosBikeCadenceFilterTextField
                            text: settings.domyos_bike_cadence_filter
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.domyos_bike_cadence_filter = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okDomyosBikeCadenceFilter
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.domyos_bike_cadence_filter = domyosBikeCadenceFilterTextField.text
                        }
                    }
                    SwitchDelegate {
                        id: domyosBikeCaloriesDisplayDelegate
                        text: qsTr("Fix Calories/Km to Console")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.domyos_bike_display_calories
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.domyos_bike_display_calories = checked
                    }
                }
                AccordionElement {
                    id: proformBikeAccordion
                    title: qsTr("Proform Bike Options")
                    indicatRectColor: Material.color(Material.Grey)
                    textColor: Material.color(Material.Yellow)
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
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okproformBikeWheelRatioButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.proform_wheel_ratio = proformBikeWheelRatioTextField.text
                        }
                    }
                    SwitchDelegate {
                        id: tourDeFranceCLCdelegate
                        text: qsTr("Tour de France CLC")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.proform_tour_de_france_clc
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.proform_tour_de_france_clc = checked
                    }
                    SwitchDelegate {
                        id: proformStudiodelegate
                        text: qsTr("Proform Studio Bike")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.proform_studio
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.proform_studio = checked
                    }
                    SwitchDelegate {
                        id: proformTdfJonseedWattdelegate
                        text: qsTr("TDF CBC Jonseed Watt table")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.proform_tdf_jonseed_watt
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.proform_tdf_jonseed_watt = checked
                    }
                }

                AccordionElement {
                    id: m3iBikeAccordion
                    title: qsTr("M3i Bike Options")
                    indicatRectColor: Material.color(Material.Grey)
                    textColor: Material.color(Material.Yellow)
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
                                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
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
                                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                            }
                            Button {
                                id: okm3iBikeSpeedBuffsizeButton
                                text: "OK"
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                onClicked: settings.m3i_bike_speed_buffsize = m3iBikeSpeedBuffsizeTextField.text
                            }
                        }

                        SwitchDelegate {
                            id: m3iBikeKcalDelegate
                            text: qsTr("Use KCal from the Bike")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.m3i_bike_kcal
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.m3i_bike_kcal = checked
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
                }
            }

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
                }
            }

            AccordionElement {
                id: pelotonAccordion
                title: qsTr("Peloton Options") + "\uD83E\uDD47"
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                accordionContent: ColumnLayout {
                    spacing: 0

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelPelotonUsername
                            text: qsTr("Username:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: pelotonUsernameTextField
                            text: settings.peloton_username
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onAccepted: settings.peloton_username = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okPelotonUsernameButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.peloton_username = pelotonUsernameTextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelPelotonPassword
                            text: qsTr("Password:") + ((rootItem.pelotonLogin===-1)?"":(rootItem.pelotonLogin===1?"\u2705":"\u274c"))
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: pelotonPasswordTextField
                            text: settings.peloton_password
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhHiddenText
                            echoMode: TextInput.PasswordEchoOnEdit
                            onAccepted: settings.peloton_password = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okPelotonPasswordButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.peloton_password = pelotonPasswordTextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelPelotonDifficulty
                            text: qsTr("Difficulty:")
                            Layout.fillWidth: true
                        }
                        ComboBox {
                            id: pelotonDifficultyTextField
                            model: [ "lower", "upper", "average" ]
                            displayText: settings.peloton_difficulty
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onActivated: {
                                console.log("combomodel activated" + pelotonDifficultyTextField.currentIndex)
                                displayText = pelotonDifficultyTextField.currentValue
                             }

                        }
                        Button {
                            id: okPelotonDifficultyButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.peloton_difficulty = pelotonDifficultyTextField.displayText
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelPZPUsername
                            text: qsTr("PZP Username:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: pzpUsernameTextField
                            text: settings.pzp_username
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onAccepted: settings.pzp_username = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okPZPUsernameButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.pzp_username = pzpUsernameTextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelPZPPassword
                            text: qsTr("PZP Password:") + ((rootItem.pzpLogin===-1)?"":(rootItem.pzpLogin===1?"\u2705":"\u274c"))
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: pzpPasswordTextField
                            text: settings.pzp_password
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhHiddenText
                            echoMode: TextInput.PasswordEchoOnEdit
                            onAccepted: settings.pzp_password = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okPZPPasswordButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.pzp_password = pzpPasswordTextField.text
                        }
                    }

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
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
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
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okPelotonOffsetButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.peloton_offset = pelotonOffsetTextField.text
                        }
                    }
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

                    /*
                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelPelotonCadenceMetric
                            text: qsTr("Override Cadence Metric:")
                            Layout.fillWidth: true
                        }
                        ComboBox {
                            id: pelotonCadenceMetricTextField
                            model: rootItem.metrics
                            displayText: settings.peloton_cadence_metric
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onActivated: {
                                console.log("combomodel activated" + pelotonCadenceMetricTextField.currentIndex)
                                displayText = pelotonCadenceMetricTextField.currentValue
                             }

                        }
                        Button {
                            id: okPelotonCadenceMetric
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.peloton_cadence_metric = pelotonCadenceMetricTextField.displayText;
                        }
                    }*/

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelPelotonHeartRateMetric
                            text: qsTr("Override HR Metric:")
                            Layout.fillWidth: true
                        }
                        ComboBox {
                            id: pelotonHeartRateMetricTextField
                            model: rootItem.metrics
                            displayText: settings.peloton_heartrate_metric
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onActivated: {
                                console.log("combomodel activated" + pelotonHeartRateMetricTextField.currentIndex)
                                displayText = pelotonHeartRateMetricTextField.currentValue
                             }

                        }
                        Button {
                            id: okPelotonHeartRateMetric
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.peloton_heartrate_metric = pelotonHeartRateMetricTextField.displayText;
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelpelotonDateOnStrava
                            text: qsTr("Date on Strava:")
                            Layout.fillWidth: true
                        }
                        ComboBox {
                            id: pelotonDateOnStravaTextField
                            model: [ "Before Title", "After Title", "Disabled" ]
                            displayText: settings.peloton_date
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onActivated: {
                                console.log("combomodel activated" + pelotonDateOnStravaTextField.currentIndex)
                                displayText = pelotonDateOnStravaTextField.currentValue
                            }

                        }
                        Button {
                            id: okPelotonDateOnStrava
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.peloton_date = pelotonDateOnStravaTextField.displayText
                        }
                    }

                    SwitchDelegate {
                        id: pelotonDescriptionLinkDelegate
                        text: qsTr("Activity Link in Strava")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.peloton_description_link
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.peloton_description_link = checked
                    }
                }
            }

            AccordionElement {
                id: trainingProgramOptionsAccordion
                title: qsTr("Training Program Options")
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
                            id: labelTreadmillPidHR
                            text: qsTr("PID on Heart Zone:")
                            Layout.fillWidth: true
                        }
                        ComboBox {
                            id: treadmillPidHRTextField
                            model: [ "Disabled", "1", "2","3","4","5" ]
                            displayText: settings.treadmill_pid_heart_zone
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onActivated: {
                                console.log("combomodel activated" + treadmillPidHRTextField.currentIndex)
                                displayText = treadmillPidHRTextField.currentValue
                             }

                        }
                        Button {
                            id: okTreadmillPidHR
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.treadmill_pid_heart_zone = treadmillPidHRTextField.displayText
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
                                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
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
                                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
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
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                                onAccepted: settings.trainprogram_speed_min = text
                                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
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
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                                onAccepted: settings.trainprogram_speed_max = text
                                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
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
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                                onAccepted: settings.trainprogram_incline_min = text
                                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
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
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                                onAccepted: settings.trainprogram_incline_max = text
                                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
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
                                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
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
                                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
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
            }

            AccordionElement {
                id:treadmillAccordion
                title: qsTr("Treadmill Options")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                accordionContent: ColumnLayout {
                    spacing: 0
                    SwitchDelegate {
                        id: treadmillAsABikeDelegate
                        text: qsTr("Treadmill as a Bike")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.virtual_device_force_bike
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.virtual_device_force_bike = checked
                    }
                }

                SwitchDelegate {
                    id: pauseOnStartTreadmillDelegate
                    text: qsTr("Pause when App Starts")
                    spacing: 0
                    bottomPadding: 0
                    topPadding: 0
                    rightPadding: 0
                    leftPadding: 0
                    clip: false
                    checked: settings.pause_on_start_treadmill
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    Layout.fillWidth: true
                    onClicked: settings.pause_on_start_treadmill = checked
                }

                AccordionElement {
                    id: proformTreadmillAccordion
                    title: qsTr("Proform/Nordictrack Options")
                    indicatRectColor: Material.color(Material.Grey)
                    textColor: Material.color(Material.Yellow)
                    color: Material.backgroundColor
                    accordionContent: ColumnLayout {
                        spacing: 0
                        SwitchDelegate {
                            id: nordictrack10Delegate
                            text: qsTr("Nordictrack 10")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.nordictrack_10_treadmill
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.nordictrack_10_treadmill = checked
                        }
                        /*
                        SwitchDelegate {
                            id: proform995iDelegate
                            text: qsTr("Proform 995i")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.proform_treadmill_995i
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.proform_treadmill_995i = checked
                        }*/
                    }
                }

                AccordionElement {
                    id: domyosTreadmillAccordion
                    title: qsTr("Domyos Treadmill Options")
                    indicatRectColor: Material.color(Material.Grey)
                    textColor: Material.color(Material.Yellow)
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

                        SwitchDelegate {
                            id: domyosTreadmillDisplayInvertdelegate
                            text: qsTr("Fix Distance on Display")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.domyos_treadmill_display_invert
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.domyos_treadmill_display_invert = checked
                        }
                    }
                }

                AccordionElement {
                    id:soleTreadmillAccordion
                    title: qsTr("Sole Treadmill Options")
                    indicatRectColor: Material.color(Material.Grey)
                    textColor: Material.color(Material.Yellow)
                    color: Material.backgroundColor
                    accordionContent: ColumnLayout {
                        spacing: 0
                        SwitchDelegate {
                            id: soleInclinationDelegate
                            text: qsTr("Inclination (experimental)")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.sole_treadmill_inclination
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.sole_treadmill_inclination = checked
                        }
                        SwitchDelegate {
                            id: soleMilesDelegate
                            text: qsTr("Miles unit from the device")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.sole_treadmill_miles
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.sole_treadmill_miles = checked
                        }
                        SwitchDelegate {
                            id: soleF65Delegate
                            text: qsTr("Sole F65")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.sole_treadmill_f65
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.sole_treadmill_f65 = checked
                        }
                    }
                }

                AccordionElement {
                    id: fitshowAccordion
                    title: qsTr("Fitshow Treadmill Options")
                    indicatRectColor: Material.color(Material.Grey)
                    textColor: Material.color(Material.Yellow)
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
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
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
                    id: eslinkerTreadmillAccordion
                    title: qsTr("ESLinker Treadmill Options")
                    indicatRectColor: Material.color(Material.Grey)
                    textColor: Material.color(Material.Yellow)
                    color: Material.backgroundColor
                    accordionContent: ColumnLayout {
                        spacing: 0
                        SwitchDelegate {
                            id: eslinkerTreadmillCadenzaDelegate
                            text: qsTr("Cadenza Treadmill (Bodytone)")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.eslinker_cadenza
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.eslinker_cadenza = checked
                        }
                    }
                }
            }

            AccordionElement {
                id: toorxTreadmillAccordion
                title: qsTr("Toorx/iConsole Options")
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
                        id: bhSpadaDelegate
                        text: qsTr("BH SPADA Compatibility")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.bh_spada_2
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.bh_spada_2 = checked
                    }

                    SwitchDelegate {
                        id: jtxFitnessSprintTreadmillDelegate
                        text: qsTr("JTX Fitness Sprint Treadmill")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.jtx_fitness_sprint_treadmill
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.jtx_fitness_sprint_treadmill = checked
                    }

                    SwitchDelegate {
                        id: dknEndurunTreadmillDelegate
                        text: qsTr("DKN Endurn Treadmill")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.dkn_endurun_treadmill
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.dkn_endurun_treadmill = checked
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
                    SwitchDelegate {
                        id: toorxBikeJLLIC400Delegate
                        text: qsTr("JLL IC400 Bike")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.jll_IC400_bike
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.jll_IC400_bike = checked
                    }
                    SwitchDelegate {
                        id: toorxBikeFytterRI08Delegate
                        text: qsTr("Fytter RI08 Bike")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.fytter_ri08_bike
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.fytter_ri08_bike = checked
                    }
                    SwitchDelegate {
                        id: toorxBikeASVIVADelegate
                        text: qsTr("Asviva Bike")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.asviva_bike
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.asviva_bike = checked
                    }
                    SwitchDelegate {
                        id: toorxBikeHertzXR770Delegate
                        text: qsTr("Hertz XR 770 Bike")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.hertz_xr_770
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.hertz_xr_770 = checked
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
                        inputMethodHints: Qt.ImhDigitsOnly
                        onAccepted: settings.domyos_elliptical_speed_ratio = text
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
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
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
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
                            text: qsTr("Watt Gain (max value 2.00):")
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
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okWattGainButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.watt_gain = wattGainTextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelspeedOffset
                            text: qsTr("Speed Offset")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: speedOffsetTextField
                            text: settings.speed_offset
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.speed_offset = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okspeedOffsetButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.speed_offset = speedOffsetTextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelspeedGain
                            text: qsTr("Speed Gain:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: speedGainTextField
                            text: settings.speed_gain
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.speed_gain = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okSpeedGainButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.speed_gain = speedGainTextField.text
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
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okStravaSuffixButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.strava_suffix = stravaSuffixTextField.text
                        }
                    }

                    SwitchDelegate {
                        id: volumeChangeGearsDelegate
                        text: qsTr("Volumes buttons change gears")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.volume_change_gears
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.volume_change_gears = checked
                    }

                    SwitchDelegate {
                        id: powerAvg5s
                        text: qsTr("Power Average 5 sec.")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.power_avg_5s
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.power_avg_5s = checked
                    }

                    SwitchDelegate {
                        id: instantPowerOnPause
                        text: qsTr("Instant Power on Pause")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.instant_power_on_pause
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.instant_power_on_pause = checked
                    }
                }
            }

            AccordionElement {
                id: accesoriesAccordion
                title: qsTr("Accessories")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                accordionContent: ColumnLayout {
                    spacing: 10

                    AccordionElement {
                        id: cadenceSensorOptionsAccordion
                        title: qsTr("Cadence Sensor Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            spacing: 10

                            Label {
                                id: cadenceSensorLabel
                                text: qsTr("Don't touch these settings if your bike works properly!")
                                font.bold: true
                                font.italic: true
                                font.pixelSize: 9
                                textFormat: Text.PlainText
                                wrapMode: Text.WordWrap
                                verticalAlignment: Text.AlignVCenter
                                color: Material.color(Material.Red)
                            }

                            SwitchDelegate {
                                id: cadenceSensorAsBikeDelegate
                                text: qsTr("Cadence Sensor as a Bike")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.cadence_sensor_as_bike
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: settings.cadence_sensor_as_bike = checked
                            }
                            RowLayout {
                                spacing: 10
                                Label {
                                    id: labelCadenceSensorName
                                    text: qsTr("Cadence Sensor:")
                                    Layout.fillWidth: true
                                }
                                ComboBox {
                                    id: cadenceSensorNameTextField
                                    model: rootItem.bluetoothDevices
                                    displayText: settings.cadence_sensor_name
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onActivated: {
                                        console.log("combomodel activated" + cadenceSensorNameTextField.currentIndex)
                                        displayText = cadenceSensorNameTextField.currentValue
                                    }

                                }
                                Button {
                                    id: okCadenceSensorNameButton
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: settings.cadence_sensor_name = cadenceSensorNameTextField.displayText;
                                }
                            }

                            Button {
                                id: refreshCadenceSensorNameButton
                                text: "Refresh Devices List"
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                onClicked: refresh_bluetooth_devices_clicked();
                            }
                            RowLayout {
                                spacing: 10
                                Label {
                                    id: labelCadenceSpeedRatio
                                    text: qsTr("Wheel Ratio:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: cadenceSpeedRatioTextField
                                    text: settings.cadence_sensor_speed_ratio
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    onAccepted: settings.cadence_sensor_speed_ratio = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    id: okCadenceSpeedRatio
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: settings.cadence_sensor_speed_ratio = cadenceSpeedRatioTextField.text
                                }
                            }
                        }
                    }

                    AccordionElement {
                        id: powerSensorOptionsAccordion
                        title: qsTr("Power Sensor Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            spacing: 10
                            SwitchDelegate {
                                id: powerSensorAsBikeDelegate
                                text: qsTr("Power Sensor as a Bike")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.power_sensor_as_bike
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: settings.power_sensor_as_bike = checked
                            }
                            SwitchDelegate {
                                id: powerSensorAsTreadmillDelegate
                                text: qsTr("Power Sensor as a Treadmill")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.power_sensor_as_treadmill
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: settings.power_sensor_as_treadmill = checked
                            }
                            SwitchDelegate {
                                id: powerSensorRunCadenceDoubleDelegate
                                text: qsTr("Doubling Cadence for Run")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.powr_sensor_running_cadence_double
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: settings.powr_sensor_running_cadence_double = checked
                            }

                            RowLayout {
                                spacing: 10
                                Label {
                                    id: labelPowerSensorName
                                    text: qsTr("Power Sensor:")
                                    Layout.fillWidth: true
                                }
                                ComboBox {
                                    id: powerSensorNameTextField
                                    model: rootItem.bluetoothDevices
                                    displayText: settings.power_sensor_name
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onActivated: {
                                        console.log("combomodel activated" + powerSensorNameTextField.currentIndex)
                                        displayText = powerSensorNameTextField.currentValue
                                    }

                                }
                                Button {
                                    id: okPowerSensorNameButton
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: settings.power_sensor_name = powerSensorNameTextField.displayText;
                                }
                            }

                            Button {
                                id: refreshPowerSensorNameButton
                                text: "Refresh Devices List"
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                onClicked: refresh_bluetooth_devices_clicked();
                            }
                        }
                    }

                    AccordionElement {
                        id: eliteAccesoriesAccordion
                        title: qsTr("Elite™ Products")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            spacing: 10
                            AccordionElement {
                                id: eliteRizerOptionsAccordion
                                title: qsTr("Elite Rizer Options")
                                indicatRectColor: Material.color(Material.Grey)
                                textColor: Material.color(Material.Blue)
                                color: Material.backgroundColor
                                accordionContent: ColumnLayout {
                                    spacing: 10
                                    RowLayout {
                                        spacing: 10
                                        Label {
                                            id: labelEliteRizerName
                                            text: qsTr("Elite Rizer:")
                                            Layout.fillWidth: true
                                        }
                                        ComboBox {
                                            id: eliteRizerNameTextField
                                            model: rootItem.bluetoothDevices
                                            displayText: settings.elite_rizer_name
                                            Layout.fillHeight: false
                                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                            onActivated: {
                                                console.log("combomodel activated" + eliteRizerNameTextField.currentIndex)
                                                displayText = eliteRizerNameTextField.currentValue
                                            }

                                        }
                                        Button {
                                            id: okEliteRizerNameButton
                                            text: "OK"
                                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                            onClicked: settings.elite_rizer_name = eliteRizerNameTextField.displayText;
                                        }
                                    }

                                    Button {
                                        id: refreshEliteRizerNameButton
                                        text: "Refresh Devices List"
                                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                        onClicked: refresh_bluetooth_devices_clicked();
                                    }
                                }
                            }
                            AccordionElement {
                                id: eliteSterzoSmartOptionsAccordion
                                title: qsTr("Elite Sterzo Smart Options")
                                indicatRectColor: Material.color(Material.Grey)
                                textColor: Material.color(Material.Blue)
                                color: Material.backgroundColor
                                accordionContent: ColumnLayout {
                                    spacing: 10
                                    RowLayout {
                                        spacing: 10
                                        Label {
                                            id: labelEliteSterzoSmartName
                                            text: qsTr("Elite Sterzo Smart:")
                                            Layout.fillWidth: true
                                        }
                                        ComboBox {
                                            id: eliteSterzoSmartNameTextField
                                            model: rootItem.bluetoothDevices
                                            displayText: settings.elite_sterzo_smart_name
                                            Layout.fillHeight: false
                                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                            onActivated: {
                                                console.log("combomodel activated" + eliteSterzoSmartNameTextField.currentIndex)
                                                displayText = eliteSterzoSmartNameTextField.currentValue
                                            }

                                        }
                                        Button {
                                            id: okEliteSterzoSmartNameButton
                                            text: "OK"
                                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                            onClicked: settings.elite_sterzo_smart_name = eliteSterzoSmartNameTextField.displayText;
                                        }
                                    }

                                    Button {
                                        id: refreshEliteSterzoSmartNameButton
                                        text: "Refresh Devices List"
                                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                        onClicked: refresh_bluetooth_devices_clicked();
                                    }
                                }
                            }
                        }
                    }

                    AccordionElement {
                        id: ftmsAccessoryOptionsAccordion
                        title: qsTr("SmartSpin2k Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            spacing: 10
                            RowLayout {
                                spacing: 10
                                Label {
                                    id: labelFTMSAccessoryName
                                    text: qsTr("SmartSpin2k device:")
                                    Layout.fillWidth: true
                                }
                                ComboBox {
                                    id: ftmsAccessoryNameTextField
                                    model: rootItem.bluetoothDevices
                                    displayText: settings.ftms_accessory_name
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onActivated: {
                                        console.log("combomodel activated" + ftmsAccessoryNameTextField.currentIndex)
                                        displayText = ftmsAccessoryNameTextField.currentValue
                                    }

                                }
                                Button {
                                    id: okFTMSAccessoryNameButton
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: settings.ftms_accessory_name = ftmsAccessoryNameTextField.displayText;
                                }
                            }

                            Button {
                                id: refreshFTMSAccessoryNameButton
                                text: "Refresh Devices List"
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                onClicked: refresh_bluetooth_devices_clicked();
                            }

                            RowLayout {
                                spacing: 10
                                Label {
                                    id: labelSS2KShiftStep
                                    text: qsTr("Shift Step")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: ss2kShiftStepTextField
                                    text: settings.ss2k_shift_step
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    onAccepted: settings.ss2k_shift_step = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    id: okSS2kShiftStep
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: settings.ss2k_shift_step = ss2kShiftStepTextField.text
                                }
                            }
                        }
                    }

                    AccordionElement {
                        id: fitmetriaFanFitOptionsAccordion
                        title: qsTr("Fitmetria Fitfan™ Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor

                        accordionContent: ColumnLayout {
                            spacing: 0
                            SwitchDelegate {
                                id: fitmetriaFanFitDelegate
                                text: qsTr("Enable")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.fitmetria_fanfit_enable
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: settings.fitmetria_fanfit_enable = checked
                            }

                            RowLayout {
                                spacing: 10
                                Label {
                                    id: labelFitmetriaFanFitMode
                                    text: qsTr("Mode:")
                                    Layout.fillWidth: true
                                }
                                ComboBox {
                                    id: fitmetriaFanFitModeTextField
                                    model: [ "Heart", "Power", "Manual" ]
                                    displayText: settings.fitmetria_fanfit_mode
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onActivated: {
                                        console.log("combomodel activated" + fitmetriaFanFitModeTextField.currentIndex)
                                        displayText = fitmetriaFanFitModeTextField.currentValue
                                    }

                                }
                                Button {
                                    id: okFitmetriaFanFitModeTextField
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: settings.fitmetria_fanfit_mode = fitmetriaFanFitModeTextField.displayText
                                }
                            }
                            RowLayout {
                                spacing: 10
                                Label {
                                    id: labelFitmetriaFanFitMin
                                    text: qsTr("Min. value (0-100):")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: fitmetriaFanFitMinTextField
                                    text: settings.fitmetria_fanfit_min
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    onAccepted: settings.fitmetria_fanfit_min = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    id: okFitmetriaFanFitMin
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: settings.fitmetria_fanfit_min = fitmetriaFanFitMinTextField.text
                                }
                            }
                            RowLayout {
                                spacing: 10
                                Label {
                                    id: labelFitmetriaFanFitMax
                                    text: qsTr("Max value (0-100):")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: fitmetriaFanFitMaxTextField
                                    text: settings.fitmetria_fanfit_max
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    onAccepted: settings.fitmetria_fanfit_max = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    id: okFitmetriaFanFitMax
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: settings.fitmetria_fanfit_max = fitmetriaFanFitMaxTextField.text
                                }
                            }
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
                        id: bluetooth30mHangsDelegate
                        text: qsTr("Bluetooth hangs after 30m")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.bluetooth_30m_hangs
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.bluetooth_30m_hangs = checked
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
                        id: virtualDeviceIfitDelegate
                        text: qsTr("Virtual iFit")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.virtual_device_ifit
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.virtual_device_ifit = checked
                    }

                    SwitchDelegate {
                        id: virtualDeviceRowerDelegate
                        text: qsTr("Virtual Rower")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.virtual_device_rower
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.virtual_device_rower = checked
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
                        id: runCadenceSensorDelegate
                        text: qsTr("Run Cadence Sensor")
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

                    AccordionElement {
                        id: templateSettingsAccordion
                        title: qsTr("Template Settings")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Grey)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            id: templateSettingsContent
                        }
                        Component.onCompleted: function() {
                            let template_ids = settings.value("template_user_ids", []);
                            console.log("template_ids current val "+template_ids);
                            if (template_ids) {
                                let accordionCheckComponent = Qt.createComponent("AccordionCheckElement.qml");
                                let componentMap = {};
                                template_ids.forEach(function(template_id) {
                                    console.log("template_id current "+template_id);
                                    let template_type = settings.value("template_" + template_id + "_type", "");
                                    if (template_type) {
                                        console.log("template_type current "+template_type);
                                        if (!componentMap[template_type])
                                            componentMap[template_type] = Qt.createComponent("Template" + template_type + ".qml");
                                        let component = componentMap[template_type];
                                        if (component) {
                                            let key_enabled = "template_" + template_id + "_enabled";
                                            console.log("Creating component object for id "+template_id);
                                            let template_object = component.createObject(null,
                                                                                         {
                                                                                             settings: settings,
                                                                                             templateId: template_id
                                                                                         });
                                            let accordionCheck = accordionCheckComponent.createObject(templateSettingsContent,
                                                                                                      {
                                                                                                          title: template_id +" (" + template_type +")",
                                                                                                          settings: settings,
                                                                                                          linkedBoolSetting: key_enabled,
                                                                                                          accordionContent: template_object
                                                                                                      });
                                        }
                                    }
                                });
                            }
                        }
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
                        id: appleWatchFakeDeviceDelegate
                        text: qsTr("Apple Watch Fake Device")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.applewatch_fakedevice
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.applewatch_fakedevice = checked
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
