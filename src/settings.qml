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
            property real zwift_inclination_offset: 0
            property real zwift_inclination_gain: 1.0
            property real echelon_resistance_offset: 0
            property real echelon_resistance_gain: 1.0

            property bool speed_power_based: false
            property int bike_resistance_start: 1
            property int age: 35.0
            property real weight: 75.0
            property real ftp: 200.0
            property string user_email: ""
            property string user_nickname: ""
            property bool miles_unit: false
            property bool pause_on_start: false
            property bool treadmill_force_speed: false
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

            property real heart_rate_zone1: 70.0
            property real heart_rate_zone2: 80.0
            property real heart_rate_zone3: 90.0
            property real heart_rate_zone4: 100.0
            property bool heart_max_override_enable: false
            property real heart_max_override_value: 195.0

            property real peloton_gain: 1.0
            property real peloton_offset: 0

            property string treadmill_pid_heart_zone: "Disabled"
            property real pacef_1mile: 250
            property real pacef_5km: 300
            property real pacef_10km: 320
            property real pacef_halfmarathon: 340
            property real pacef_marathon: 360
            property string pace_default: "Half Marathon"

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
            property bool proform_tdf_10: false

            property double horizon_gr7_cadence_multiplier: 1.0

            property int  fitshow_user_id: 0x13AA

            property bool inspire_peloton_formula: false
            property bool inspire_peloton_formula2: false

            property bool hammer_racer_s: false

            property bool pafers_treadmill: false

            property bool yesoul_peloton_formula: false

            property bool nordictrack_10_treadmill: true
            property bool nordictrack_t65s_treadmill: false
            //property bool proform_treadmill_995i: false

            property bool toorx_3_0: false
            property bool toorx_65s_evo: false
            property bool jtx_fitness_sprint_treadmill: false
            property bool dkn_endurun_treadmill: false
            property bool trx_route_key: false
            property bool bh_spada_2: false
            property bool toorx_bike: false
            property bool toorx_ftms: false
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
            property bool virtufit_etappe: false

            property int flywheel_filter: 2
            property bool flywheel_life_fitness_ic8: false

            property bool sole_treadmill_inclination: false
            property bool sole_treadmill_miles: true
            property bool sole_treadmill_f65: false
            property bool sole_treadmill_f63: false
            property bool sole_treadmill_tt8: false

            property bool schwinn_bike_resistance: false
            property bool schwinn_bike_resistance_v2: value

            property bool technogym_myrun_treadmill_experimental: false

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
            property bool virtual_device_bluetooth: true
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

            // from version 2.10.15
            property real zwift_erg_resistance_down: 0.0
            property real zwift_erg_resistance_up: 999.0

            // from version 2.10.16
            property bool horizon_paragon_x: false

            // from version 2.10.18
            property real treadmill_step_speed: 0.5
            property real treadmill_step_incline: 0.5

            // from version 2.10.19
            property bool  fitshow_anyrun: false

            // from version 2.10.21
            property bool nordictrack_s30_treadmill: false

            // from version 2.10.23
            // not used anymore because it's an elliptical not a treadmill. Don't remove this
            // it will cause corruption in the settings
            property bool nordictrack_fs5i_treadmill: false

            // from version 2.10.26
            property bool renpho_peloton_conversion_v2: false

            // from version 2.10.27
            property real ss2k_resistance_sample_1: 20
            property real ss2k_shift_step_sample_1: 0
            property real ss2k_resistance_sample_2: 30
            property real ss2k_shift_step_sample_2: 0
            property real ss2k_resistance_sample_3: 40
            property real ss2k_shift_step_sample_3: 0
            property real ss2k_resistance_sample_4: 50
            property real ss2k_shift_step_sample_4: 0

            property bool  fitshow_truetimer: false

            // from version 2.10.28
            property real elite_rizer_gain: 1.0
            property bool tile_ext_incline_enabled: false
            property int  tile_ext_incline_order: 32

            // from version 2.10.41
            property bool reebok_fr30_treadmill: false

            // from version 2.10.44
            property bool horizon_treadmill_7_8: false

            // from version 2.10.45
            property string profile_name: "default"

            // from version 2.10.46
            property bool tile_cadence_color_enabled: false
            property bool tile_peloton_remaining_enabled: false
            property int  tile_peloton_remaining_order: 22
            property bool tile_peloton_resistance_color_enabled: false

            // from version 2.10.49
            property bool dircon_yes: true
            property int dircon_server_base_port: 36866

            // from version 2.10.56
            property bool ios_cache_heart_device: true

            // from version 2.10.57
            property int app_opening: 0

            // from version 2.10.62
            property string proformtdf4ip: ""

            // from version 2.10.72
            property bool fitfiu_mc_v460: false
            property real bike_weight: 0

            // from version 2.10.77
            property bool kingsmith_encrypt_v2: false

            // from version 2.10.81
            property bool proform_treadmill_9_0: false

            // from version 2.10.85
            property bool proform_treadmill_1800i: false

            // from version 2.10.91
            property real cadence_offset: 0
            property real cadence_gain: 1
            property bool sp_ht_9600ie: false

            // from version 2.10.92
            property bool tts_enabled: false
            property int tts_summary_sec: 120
            property bool tts_act_speed: false
            property bool tts_avg_speed: true
            property bool tts_max_speed: false
            property bool tts_act_inclination: false
            property bool tts_act_cadence: false
            property bool tts_avg_cadence: true
            property bool tts_max_cadence: false
            property bool tts_act_elevation: true
            property bool tts_act_calories: true
            property bool tts_act_odometer: true
            property bool tts_act_pace: false
            property bool tts_avg_pace: true
            property bool tts_max_pace: false
            property bool tts_act_resistance: true
            property bool tts_avg_resistance: true
            property bool tts_max_resistance: false
            property bool tts_act_watt: false
            property bool tts_avg_watt: true
            property bool tts_max_watt: true
            property bool tts_act_ftp: false
            property bool tts_avg_ftp: true
            property bool tts_max_ftp: false
            property bool tts_act_heart: true
            property bool tts_avg_heart: true
            property bool tts_max_heart: false
            property bool tts_act_jouls: true
            property bool tts_act_elapsed: true
            property bool tts_act_peloton_resistance: false
            property bool tts_avg_peloton_resistance: false
            property bool tts_max_peloton_resistance: false
            property bool tts_act_target_peloton_resistance: true
            property bool tts_act_target_cadence: true
            property bool tts_act_target_power: true
            property bool tts_act_target_zone: true
            property bool tts_act_target_speed: true
            property bool tts_act_target_incline: true
            property bool tts_act_watt_kg: false
            property bool tts_avg_watt_kg: false
            property bool tts_max_watt_kg: false

            // from version 2.10.96
            property bool fakedevice_elliptical: false

            // from version 2.10.99
            property string nordictrack_2950_ip: ""

            // from version 2.10.102
            property bool tile_instantaneous_stride_length_enabled: false
            property int  tile_instantaneous_stride_length_order: 32
            property bool tile_ground_contact_enabled: false
            property int  tile_ground_contact_order: 33
            property bool tile_vertical_oscillation_enabled: false
            property int  tile_vertical_oscillation_order: 34
            property string sex: "Male"

            // from version 2.10.111
            property string maps_type: "3D"

            // from version 2.10.112
            property real ss2k_max_resistance: 100
            property real ss2k_min_resistance: 0

            // from version 2.11.10
            property bool proform_treadmill_se: false

            // from version 2.11.14
            property string proformtreadmillip: ""

            // from version 2.11.22
            property bool kingsmith_encrypt_v3: false

            // from version 2.11.38
            property string tdf_10_ip: ""

            // from version 2.11.41
            property bool fakedevice_treadmill: false

            // from version 2.11.43
            property int video_playback_window_s: 12
        }

        function paddingZeros(text, limit) {
          if (text.length < limit) {
            return paddingZeros("0" + text, limit);
          } else {
            return text;
          }
        }

        function formatLimitDecimals(value, decimals) {
          const stringValue = value.toString();
          if(stringValue.includes('e')) {
              // TODO: remove exponential notation
              throw 'invald number';
          } else {
            const [integerPart, decimalPart] = stringValue.split('.');
            if(decimalPart) {
              return +[integerPart, decimalPart.slice(0, decimals)].join('.')
            } else {
              return integerPart;
            }
          }
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
                            //inputMethodHints: Qt.ImhFormattedNumbersOnly
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
                            //inputMethodHints: Qt.ImhFormattedNumbersOnly
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
                            id: labelSex
                            text: qsTr("Gender:")
                            Layout.fillWidth: true
                        }
                        ComboBox {
                            id: sexTextField
                            model: [ "Male", "Female" ]
                            displayText: settings.sex
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onActivated: {
                                console.log("combomodel activated" + sexTextField.currentIndex)
                                displayText = sexTextField.currentValue
                             }

                        }
                        Button {
                            id: okSex
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.sex = sexTextField.displayText
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
                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelBikeWeight
                            text: qsTr("Bike Weight") + "(" + (settings.miles_unit?"lbs":"kg") + ")"
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: bikeweightTextField
                            text: (settings.miles_unit?settings.bike_weight * 2.20462:settings.bike_weight)
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            //inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.bike_weight = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okBikeWeightButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.bike_weight = (settings.miles_unit?bikeweightTextField.text / 2.20462:bikeweightTextField.text)
                        }
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
                            //inputMethodHints: Qt.ImhFormattedNumbersOnly
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
                            id: labelZwiftErgResistanceDown
                            text: qsTr("ERG Min. Resistance:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: zwiftErgResistanceDownTextField
                            text: settings.zwift_erg_resistance_down
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.zwift_erg_resistance_down = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okzwiftErgResistanceDownButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.zwift_erg_resistance_down = zwiftErgResistanceDownTextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelZwiftErgResistanceUp
                            text: qsTr("ERG Max. Resistance:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: zwiftErgResistanceUpTextField
                            text: settings.zwift_erg_resistance_up
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.zwift_erg_resistance_up = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okzwiftErgResistanceUpButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.zwift_erg_resistance_up = zwiftErgResistanceUpTextField.text
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
                        SwitchDelegate {
                            id: schwinnBikeResistanceV2Delegate
                            text: qsTr("Resistance Alternative Calc.")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.schwinn_bike_resistance_v2
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.schwinn_bike_resistance_v2 = checked
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
                            //inputMethodHints: Qt.ImhFormattedNumbersOnly
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
                        RowLayout {
                            spacing: 10
                            Label {
                                id: labelEchelonResistanceGain
                                text: qsTr("Resistance Gain:")
                                Layout.fillWidth: true
                            }
                            TextField {
                                id: echelonResistanceGainTextField
                                text: settings.echelon_resistance_gain
                                horizontalAlignment: Text.AlignRight
                                Layout.fillHeight: false
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                //inputMethodHints: Qt.ImhFormattedNumbersOnly
                                onAccepted: settings.echelon_resistance_gain = text
                                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                            }
                            Button {
                                id: okechelonResistanceGainButton
                                text: "OK"
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                onClicked: settings.echelon_resistance_gain = echelonResistanceGainTextField.text
                            }
                        }
                        RowLayout {
                            spacing: 10
                            Label {
                                id: labelEchelonResistanceOffset
                                text: qsTr("Resistance Offset:")
                                Layout.fillWidth: true
                            }
                            TextField {
                                id: echelonResistanceOffsetTextField
                                text: settings.echelon_resistance_offset
                                horizontalAlignment: Text.AlignRight
                                Layout.fillHeight: false
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                                onAccepted: settings.echelon_resistance_offset = text
                                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                            }
                            Button {
                                id: okechelonResistanceOffsetButton
                                text: "OK"
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                onClicked: settings.echelon_resistance_offset = echelonResistanceOffsetTextField.text
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
                    id: renphoBikeAccordion
                    title: qsTr("Renpho Bike Options")
                    indicatRectColor: Material.color(Material.Grey)
                    textColor: Material.color(Material.Yellow)
                    color: Material.backgroundColor
                    accordionContent: ColumnLayout {
                        spacing: 0
                        SwitchDelegate {
                            id: renphoPelotonFormulaDelegate
                            text: qsTr("New Peloton Formula (11/02/2022)")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.renpho_peloton_conversion_v2
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.renpho_peloton_conversion_v2 = checked
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
                    id: cardioFitBikeAccordion
                    title: qsTr("CardioFIT Bike Options")
                    indicatRectColor: Material.color(Material.Grey)
                    textColor: Material.color(Material.Yellow)
                    color: Material.backgroundColor
                    accordionContent: SwitchDelegate {
                        id: spht9600iEBikeDelegate
                        text: qsTr("SP-HT-9600iE")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.sp_ht_9600ie
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.sp_ht_9600ie = checked
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
                    accordionContent:
                    SwitchDelegate {
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
                    SwitchDelegate {
                        id: virtufitEtappeBikeDelegate
                        text: qsTr("Virtufit Etappe 2.0 Bike")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.virtufit_etappe
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.virtufit_etappe = checked
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
                            //inputMethodHints: Qt.ImhFormattedNumbersOnly
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
                        id: proformTDF10odelegate
                        text: qsTr("Proform TDF 1.0")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.proform_tdf_10
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.proform_tdf_10 = checked
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
                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelproformTDF4IP
                            text: qsTr("TDF4 IP:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: proformTDF4IPTextField
                            text: settings.proformtdf4ip
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            //inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.proformtdf4ip = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okproformTDF4IPButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.proformtdf4ip = proformTDF4IPTextField.text
                        }
                    }
                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelproformTDFCompanionIP
                            text: qsTr("TDF Companion IP:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: proformTDFCompanionIPTextField
                            text: settings.tdf_10_ip
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            //inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.tdf_10_ip = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okproformTDFCompanionIPButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.tdf_10_ip = proformTDFCompanionIPTextField.text
                        }
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
                            //inputMethodHints: Qt.ImhFormattedNumbersOnly
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


                RowLayout {
                    spacing: 10
                    Label {
                        id: labelTrainProgramPace1mile
                        text: qsTr("1 mile pace (total time):")
                        Layout.fillWidth: true
                    }
                    TextField {
                        id: trainProgramPace1mileTextField
                        text: (paddingZeros(formatLimitDecimals((settings.pacef_1mile * 1.60934) / 3600,0).toString(), 2) + ":" + paddingZeros(formatLimitDecimals(((settings.pacef_1mile * 1.60934) / 60) % 60,0).toString(), 2) + ":" + paddingZeros(formatLimitDecimals((((settings.pacef_1mile * 1.60934) % 60)),0).toString(), 2))
                        horizontalAlignment: Text.AlignRight
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        //inputMethodHints: Qt.ImhDigitsOnly
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okTrainProgramPace1Mile
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: { settings.pacef_1mile = (((parseInt(trainProgramPace1mileTextField.text.split(":")[0]) * 3600) + (parseInt(trainProgramPace1mileTextField.text.split(":")[1]) * 60) + parseInt(trainProgramPace1mileTextField.text.split(":")[2]))) / 1.60934;}
                    }
                }
                RowLayout {
                    spacing: 10
                    Label {
                        id: labelTrainProgramPace5km
                        text: qsTr("5 km pace (total time):")
                        Layout.fillWidth: true
                    }
                    TextField {
                        id: trainProgramPace5kmTextField
                        text: (paddingZeros(formatLimitDecimals((settings.pacef_5km * 5) / 3600,0).toString(), 2) + ":" + paddingZeros(formatLimitDecimals(((settings.pacef_5km * 5) / 60) % 60,0).toString(), 2) + ":" + paddingZeros((formatLimitDecimals(((settings.pacef_5km * 5) % 60),0)).toString(), 2))
                        horizontalAlignment: Text.AlignRight
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        //inputMethodHints: Qt.ImhDigitsOnly
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okTrainProgramPace5km
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: { settings.pacef_5km = (((parseInt(trainProgramPace5kmTextField.text.split(":")[0]) * 3600) + (parseInt(trainProgramPace5kmTextField.text.split(":")[1]) * 60) + parseInt(trainProgramPace5kmTextField.text.split(":")[2]))) / 5;}
                    }
                }
                RowLayout {
                    spacing: 10
                    Label {
                        id: labelTrainProgramPace10km
                        text: qsTr("10 km pace (total time):")
                        Layout.fillWidth: true
                    }
                    TextField {
                        id: trainProgramPace10kmTextField
                        text: (paddingZeros(formatLimitDecimals((settings.pacef_10km * 10) / 3600,0).toString(), 2) + ":" + paddingZeros(formatLimitDecimals(((settings.pacef_10km * 10) / 60) % 60,0).toString(), 2) + ":" + paddingZeros((formatLimitDecimals(((settings.pacef_10km * 10) % 60),0)).toString(), 2))
                        horizontalAlignment: Text.AlignRight
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        //inputMethodHints: Qt.ImhDigitsOnly
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okTrainProgramPace10KM
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: { settings.pacef_10km = (((parseInt(trainProgramPace10kmTextField.text.split(":")[0]) * 3600) + (parseInt(trainProgramPace10kmTextField.text.split(":")[1]) * 60) + parseInt(trainProgramPace10kmTextField.text.split(":")[2]))) / 10;}
                    }
                }
                RowLayout {
                    spacing: 10
                    Label {
                        id: labelTrainProgramPaceHalfMarathon
                        text: qsTr("Half Marathon pace (total time):")
                        Layout.fillWidth: true
                    }
                    TextField {
                        id: trainProgramPaceHalfMarathonTextField
                        text: (paddingZeros(formatLimitDecimals((settings.pacef_halfmarathon * 21) / 3600,0).toString(), 2) + ":" + paddingZeros(formatLimitDecimals(((settings.pacef_halfmarathon * 21) / 60) % 60,0).toString(), 2) + ":" + paddingZeros((formatLimitDecimals(((settings.pacef_halfmarathon * 21) % 60),0)).toString(), 2))
                        horizontalAlignment: Text.AlignRight
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        //inputMethodHints: Qt.ImhDigitsOnly
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okTrainProgramPaceHalfMarathon
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: { settings.pacef_halfmarathon = (((parseInt(trainProgramPaceHalfMarathonTextField.text.split(":")[0]) * 3600) + (parseInt(trainProgramPaceHalfMarathonTextField.text.split(":")[1]) * 60) + parseInt(trainProgramPaceHalfMarathonTextField.text.split(":")[2]))) / 21;}
                    }
                }
                RowLayout {
                    spacing: 10
                    Label {
                        id: labelTrainProgramPaceMarathon
                        text: qsTr("Marathon pace (total time):")
                        Layout.fillWidth: true
                    }
                    TextField {
                        id: trainProgramPaceMarathonTextField
                        text: (paddingZeros(formatLimitDecimals((settings.pacef_marathon * 42) / 3600,0).toString(), 2) + ":" + paddingZeros(formatLimitDecimals(((settings.pacef_marathon * 42) / 60) % 60,0).toString(), 2) + ":" + paddingZeros((formatLimitDecimals(((settings.pacef_marathon * 42) % 60),0)).toString(), 2))
                        horizontalAlignment: Text.AlignRight
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        //inputMethodHints: Qt.ImhDigitsOnly
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okTrainProgramPaceMarathon
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: { settings.pacef_marathon = (((parseInt(trainProgramPaceMarathonTextField.text.split(":")[0]) * 3600) + (parseInt(trainProgramPaceMarathonTextField.text.split(":")[1]) * 60) + parseInt(trainProgramPaceMarathonTextField.text.split(":")[2]))) / 42;}
                    }
                }

                RowLayout {
                    spacing: 10
                    Label {
                        id: labelTreadmillPaceDefault
                        text: qsTr("Default Pace:")
                        Layout.fillWidth: true
                    }
                    ComboBox {
                        id: treadmillPaceDefaultTextField
                        model: [ "1 mile", "5 km", "10 km","Half Marathon","Marathon", ]
                        displayText: settings.pace_default
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onActivated: {
                            console.log("combomodel activated" + treadmillPaceDefaultTextField.currentIndex)
                            displayText = treadmillPaceDefaultTextField.currentValue
                         }

                    }
                    Button {
                        id: okTreadmillPaceDefault
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.pace_default = treadmillPaceDefaultTextField.displayText
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
                                //inputMethodHints: Qt.ImhFormattedNumbersOnly
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
                                //inputMethodHints: Qt.ImhFormattedNumbersOnly
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
                                //inputMethodHints: Qt.ImhFormattedNumbersOnly
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
                                //inputMethodHints: Qt.ImhFormattedNumbersOnly
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
                    id: treadmillForceSpeedDelegate
                    text: qsTr("Treadmill Speed Forcing")
                    spacing: 0
                    bottomPadding: 0
                    topPadding: 0
                    rightPadding: 0
                    leftPadding: 0
                    clip: false
                    checked: settings.treadmill_force_speed
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    Layout.fillWidth: true
                    onClicked: settings.treadmill_force_speed = checked
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

                RowLayout {
                    spacing: 10
                    Label {
                        id: labelTreadmillStepSpeed
                        text: qsTr("Speed Step:")
                        Layout.fillWidth: true
                    }
                    TextField {
                        id: treadmillSpeedStepTextField
                        text: (settings.miles_unit?settings.treadmill_step_speed * 0.621371:settings.treadmill_step_speed).toFixed(1)
                        horizontalAlignment: Text.AlignRight
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        inputMethodHints: Qt.ImhDigitsOnly
                        onAccepted: settings.treadmill_step_speed = text
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okTreadmillSpeedStepButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.treadmill_step_speed = (settings.miles_unit?treadmillSpeedStepTextField.text * 1.60934:treadmillSpeedStepTextField.text)
                    }
                }

                RowLayout {
                    spacing: 10
                    Label {
                        id: labelTreadmillStepInclination
                        text: qsTr("Inclination Step:")
                        Layout.fillWidth: true
                    }
                    TextField {
                        id: treadmillInclinationStepTextField
                        text: settings.treadmill_step_incline
                        horizontalAlignment: Text.AlignRight
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        inputMethodHints: Qt.ImhDigitsOnly
                        onAccepted: settings.treadmill_step_incline = text
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }
                    Button {
                        id: okTreadmillInclinationStepButton
                        text: "OK"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: settings.treadmill_step_incline = treadmillInclinationStepTextField.text
                    }
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
                        SwitchDelegate {
                            id: nordictrackT65SDelegate
                            text: qsTr("Nordictrack T6.5S")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.nordictrack_t65s_treadmill
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.nordictrack_t65s_treadmill = checked
                        }
                        SwitchDelegate {
                            id: nordictrackS30Delegate
                            text: qsTr("Nordictrack S30")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.nordictrack_s30_treadmill
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.nordictrack_s30_treadmill = checked
                        }
                        SwitchDelegate {
                            id: proform1800iDelegate
                            text: qsTr("Proform 1800i")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.proform_treadmill_1800i
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.proform_treadmill_1800i = checked
                        }
                        SwitchDelegate {
                            id: proformSEDelegate
                            text: qsTr("Proform SE")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.proform_treadmill_se
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.proform_treadmill_se = checked
                        }
                        RowLayout {
                            spacing: 10
                            Label {
                                id: labelproformtreadmillip
                                text: qsTr("Proform IP:")
                                Layout.fillWidth: true
                            }
                            TextField {
                                id: proformtreadmillIPTextField
                                text: settings.proformtreadmillip
                                horizontalAlignment: Text.AlignRight
                                Layout.fillHeight: false
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                //inputMethodHints: Qt.ImhFormattedNumbersOnly
                                onAccepted: settings.proformtreadmillip = text
                                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                            }
                            Button {
                                id: okproformtreadmillIPButton
                                text: "OK"
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                onClicked: settings.proformtreadmillip = proformtreadmillIPTextField.text
                            }
                        }
                        RowLayout {
                            spacing: 10
                            Label {
                                id: labelnordictrack2950IP
                                text: qsTr("Nordictrack 2950 IP:")
                                Layout.fillWidth: true
                            }
                            TextField {
                                id: nordictrack2950IPTextField
                                text: settings.nordictrack_2950_ip
                                horizontalAlignment: Text.AlignRight
                                Layout.fillHeight: false
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                //inputMethodHints: Qt.ImhFormattedNumbersOnly
                                onAccepted: settings.nordictrack_2950_ip = text
                                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                            }
                            Button {
                                id: oknordictrack2950IPButton
                                text: "OK"
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                onClicked: settings.nordictrack_2950_ip = nordictrack2950IPTextField.text
                            }
                        }
                        SwitchDelegate {
                            id: proform90IDelegate
                            text: qsTr("Proform 9.0")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.proform_treadmill_9_0
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.proform_treadmill_9_0 = checked
                        }
                        /*
                        SwitchDelegate {
                            id: nordictrackFS5IDelegate
                            text: qsTr("Nordictrack FS5i")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.nordictrack_fs5i_treadmill
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.nordictrack_fs5i_treadmill = checked
                        }*/
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
                    id: pafersTreadmillAccordion
                    title: qsTr("Pafers Options")
                    indicatRectColor: Material.color(Material.Grey)
                    textColor: Material.color(Material.Yellow)
                    color: Material.backgroundColor
                    accordionContent: ColumnLayout {
                        spacing: 0
                        SwitchDelegate {
                            id: pafersTreadmillDelegate
                            text: qsTr("Pafers Treadmill")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.pafers_treadmill
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.pafers_treadmill = checked
                        }
                    }
                }

                AccordionElement {
                    id: kingsmithTreadmillAccordion
                    title: qsTr("KingSmith Options")
                    indicatRectColor: Material.color(Material.Grey)
                    textColor: Material.color(Material.Yellow)
                    color: Material.backgroundColor
                    accordionContent: ColumnLayout {
                        spacing: 0
                        SwitchDelegate {
                            id: kingSmithTreadmillDelegate
                            text: qsTr("WalkingPad X21")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.kingsmith_encrypt_v2
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.kingsmith_encrypt_v2 = checked
                        }

                        SwitchDelegate {
                            id: kingSmithV3TreadmillDelegate
                            text: qsTr("WalkingPad X21 v2")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.kingsmith_encrypt_v3
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.kingsmith_encrypt_v3 = checked
                        }
                    }
                }

                AccordionElement {
                    id: runnerTTreadmillAccordion
                    title: qsTr("RunnerT Options")
                    indicatRectColor: Material.color(Material.Grey)
                    textColor: Material.color(Material.Yellow)
                    color: Material.backgroundColor
                    accordionContent: ColumnLayout {
                        spacing: 0
                        SwitchDelegate {
                            id: fitfiuMCV460TreadmillDelegate
                            text: qsTr("Fitfiu MC-460")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.fitfiu_mc_v460
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.fitfiu_mc_v460 = checked
                        }
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
                            id: soleF63Delegate
                            text: qsTr("Sole F63")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.sole_treadmill_f63
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.sole_treadmill_f63 = checked
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
                        SwitchDelegate {
                            id: soleTT8Delegate
                            text: qsTr("Sole TT8")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.sole_treadmill_tt8
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.sole_treadmill_tt8 = checked
                        }
                    }
                }

                AccordionElement {
                    id: technogymTreadmillAccordion
                    title: qsTr("Technogym Options")
                    indicatRectColor: Material.color(Material.Grey)
                    textColor: Material.color(Material.Yellow)
                    color: Material.backgroundColor
                    accordionContent: ColumnLayout {
                        spacing: 0
                        SwitchDelegate {
                            id: myrunDelegate
                            text: qsTr("MyRun Experimental")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.technogym_myrun_treadmill_experimental
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.technogym_myrun_treadmill_experimental = checked
                        }
                    }
                }

                AccordionElement {
                    id: fitshowAccordion
                    title: qsTr("Fitshow Treadmill Options")
                    indicatRectColor: Material.color(Material.Grey)
                    textColor: Material.color(Material.Yellow)
                    color: Material.backgroundColor
                    accordionContent: ColumnLayout {
                        SwitchDelegate {
                            id: fitshowAnyrunDelegate
                            text: qsTr("AnyRun")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.fitshow_anyrun
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.fitshow_anyrun = checked
                        }
                        SwitchDelegate {
                            id: fitshowTruetimerDelegate
                            text: qsTr("True timer")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.fitshow_truetimer
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.fitshow_truetimer = checked
                        }
                        RowLayout {
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

                AccordionElement {
                    id: horizonTreadmillAccordion
                    title: qsTr("Horizon Treadmill Options")
                    indicatRectColor: Material.color(Material.Grey)
                    textColor: Material.color(Material.Yellow)
                    color: Material.backgroundColor
                    accordionContent: ColumnLayout {
                        spacing: 0
                        SwitchDelegate {
                            id: horizonParagonXTreadmillCadenzaDelegate
                            text: qsTr("Paragon X")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.horizon_paragon_x
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.horizon_paragon_x = checked
                        }
                        SwitchDelegate {
                            id: horizon78TreadmillDelegate
                            text: qsTr("Horizon 7.8 start issue")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.horizon_treadmill_7_8
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.horizon_treadmill_7_8 = checked
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
                        id: trxsevoDelegate
                        text: qsTr("TRX 65s EVO")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.toorx_65s_evo
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.toorx_65s_evo = checked
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
                        id: reebokFR30TreadmillDelegate
                        text: qsTr("Reebok FR30 Treadmill")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.reebok_fr30_treadmill
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.reebok_fr30_treadmill = checked
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
                        id: toorxFTMSBikeDelegate
                        text: qsTr("Toorx FTMS Bike")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.toorx_ftms
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.toorx_ftms = checked
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
                            text: qsTr("Watt Gain:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: wattGainTextField
                            text: settings.watt_gain
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            //inputMethodHints: Qt.ImhFormattedNumbersOnly
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
                            //inputMethodHints: Qt.ImhFormattedNumbersOnly
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

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelcadenceOffset
                            text: qsTr("Cadence Offset")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: cadenceOffsetTextField
                            text: settings.cadence_offset
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.cadence_offset = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okcadenceOffsetButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.cadence_offset = cadenceOffsetTextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelcadenceGain
                            text: qsTr("Cadence Gain:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: cadenceGainTextField
                            text: settings.cadence_gain
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            //inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.cadence_gain = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okCadenceGainButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.cadence_gain = speedGainTextField.text
                        }
                    }

                    Label {
                        id: stravaLabel
                        text: qsTr("Strava")
                        textFormat: Text.PlainText
                        wrapMode: Text.WordWrap
                        verticalAlignment: Text.AlignVCenter
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

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelTreadmillInclinationOffset
                            text: qsTr("Zwift Inclination Offset:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: treadmillInclinationOffsetTextField
                            text: settings.zwift_inclination_offset
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.zwift_inclination_offset = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okTreadmillInclinationOffsetButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.zwift_inclination_offset = treadmillInclinationOffsetTextField.text
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelTreadmillInclinationGain
                            text: qsTr("Zwift Inclination Gain:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: treadmillInclinationGainTextField
                            text: settings.zwift_inclination_gain
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            //inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.zwift_inclination_gain = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okTreadmillInclinationGainButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.zwift_inclination_gain = treadmillInclinationGainTextField.text
                        }
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
                                RowLayout {
                                    spacing: 10
                                    Label {
                                        id: labelEliteRizerGain
                                        text: qsTr("Difficulty/Gain:")
                                        Layout.fillWidth: true
                                    }
                                    TextField {
                                        id: eliteRizerGainTextField
                                        text: settings.elite_rizer_gain
                                        horizontalAlignment: Text.AlignRight
                                        Layout.fillHeight: false
                                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                        //inputMethodHints: Qt.ImhFormattedNumbersOnly
                                        onAccepted: settings.elite_rizer_gain = text
                                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                    }
                                    Button {
                                        id: okEliteRizerGainButton
                                        text: "OK"
                                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                        onClicked: settings.elite_rizer_gain = eliteRizerGainTextField.text
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
                            RowLayout {
                                spacing: 10
                                Label {
                                    id: labelSS2KMaxResistance
                                    text: qsTr("Max Resistance")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: ss2kMaxResistanceTextField
                                    text: settings.ss2k_max_resistance
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    onAccepted: settings.ss2k_max_resistance = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    id: okSS2kMaxResistance
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: settings.ss2k_max_resistance = ss2kMaxResistanceTextField.text
                                }
                            }
                            RowLayout {
                                spacing: 10
                                Label {
                                    id: labelSS2KMinResistance
                                    text: qsTr("Min Resistance")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: ss2kMinResistanceTextField
                                    text: settings.ss2k_min_resistance
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    onAccepted: settings.ss2k_min_resistance = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    id: okSS2kMinResistance
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: settings.ss2k_min_resistance = ss2kMinResistanceTextField.text
                                }
                            }

                            AccordionElement {
                                id: ftmsAccessoryAdvancedOptionsAccordion
                                title: qsTr("Advanced SmartSpin2k Calibration")
                                indicatRectColor: Material.color(Material.Grey)
                                textColor: Material.color(Material.Blue)
                                color: Material.backgroundColor
                                accordionContent: ColumnLayout {
                                    spacing: 10
                                    RowLayout {
                                        spacing: 10
                                        Label {
                                            id: labelSS2KResistanceSample1
                                            text: qsTr("Resistance Sample 1")
                                            Layout.fillWidth: true
                                        }
                                        TextField {
                                            id: ss2kResistanceSample1TextField
                                            text: settings.ss2k_resistance_sample_1
                                            horizontalAlignment: Text.AlignRight
                                            Layout.fillHeight: false
                                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                            inputMethodHints: Qt.ImhDigitsOnly
                                            onAccepted: settings.resistance_sample_1 = text
                                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                        }
                                        Button {
                                            id: okSS2kResistanceSample1
                                            text: "OK"
                                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                            onClicked: settings.ss2k_resistance_sample_1 = ss2kResistanceSample1TextField.text
                                        }
                                    }
                                    RowLayout {
                                        Label {
                                            id: labelSS2KShiftStepSample1
                                            text: qsTr("Shift Step Sample 1")
                                            Layout.fillWidth: true
                                        }
                                        TextField {
                                            id: ss2kShiftStepSample1TextField
                                            text: settings.ss2k_shift_step_sample_1
                                            horizontalAlignment: Text.AlignRight
                                            Layout.fillHeight: false
                                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                            inputMethodHints: Qt.ImhDigitsOnly
                                            onAccepted: settings.ss2k_shift_step_sample_1 = text
                                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                        }
                                        Button {
                                            id: okSS2kShiftStepSample1
                                            text: "OK"
                                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                            onClicked: settings.ss2k_shift_step_sample_1 = ss2kShiftStepSample1TextField.text
                                        }
                                    }
                                    RowLayout {
                                        spacing: 10
                                        Label {
                                            id: labelSS2KResistanceSample2
                                            text: qsTr("Resistance Sample 2")
                                            Layout.fillWidth: true
                                        }
                                        TextField {
                                            id: ss2kResistanceSample2TextField
                                            text: settings.ss2k_resistance_sample_2
                                            horizontalAlignment: Text.AlignRight
                                            Layout.fillHeight: false
                                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                            inputMethodHints: Qt.ImhDigitsOnly
                                            onAccepted: settings.resistance_sample_2 = text
                                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                        }
                                        Button {
                                            id: okSS2kResistanceSample2
                                            text: "OK"
                                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                            onClicked: settings.ss2k_resistance_sample_2 = ss2kResistanceSample2TextField.text
                                        }
                                    }
                                    RowLayout {
                                        Label {
                                            id: labelSS2KShiftStepSample2
                                            text: qsTr("Shift Step Sample 2")
                                            Layout.fillWidth: true
                                        }
                                        TextField {
                                            id: ss2kShiftStepSample2TextField
                                            text: settings.ss2k_shift_step_sample_2
                                            horizontalAlignment: Text.AlignRight
                                            Layout.fillHeight: false
                                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                            inputMethodHints: Qt.ImhDigitsOnly
                                            onAccepted: settings.ss2k_shift_step_sample_2 = text
                                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                        }
                                        Button {
                                            id: okSS2kShiftStepSample2
                                            text: "OK"
                                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                            onClicked: settings.ss2k_shift_step_sample_2 = ss2kShiftStepSample2TextField.text
                                        }
                                    }
                                    RowLayout {
                                        spacing: 10
                                        Label {
                                            id: labelSS2KResistanceSample3
                                            text: qsTr("Resistance Sample 3")
                                            Layout.fillWidth: true
                                        }
                                        TextField {
                                            id: ss2kResistanceSample3TextField
                                            text: settings.ss2k_resistance_sample_3
                                            horizontalAlignment: Text.AlignRight
                                            Layout.fillHeight: false
                                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                            inputMethodHints: Qt.ImhDigitsOnly
                                            onAccepted: settings.resistance_sample_3 = text
                                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                        }
                                        Button {
                                            id: okSS2kResistanceSample3
                                            text: "OK"
                                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                            onClicked: settings.ss2k_resistance_sample_3 = ss2kResistanceSample3TextField.text
                                        }
                                    }
                                    RowLayout {
                                        Label {
                                            id: labelSS2KShiftStepSample3
                                            text: qsTr("Shift Step Sample 3")
                                            Layout.fillWidth: true
                                        }
                                        TextField {
                                            id: ss2kShiftStepSample3TextField
                                            text: settings.ss2k_shift_step_sample_3
                                            horizontalAlignment: Text.AlignRight
                                            Layout.fillHeight: false
                                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                            inputMethodHints: Qt.ImhDigitsOnly
                                            onAccepted: settings.ss2k_shift_step_sample_3 = text
                                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                        }
                                        Button {
                                            id: okSS2kShiftStepSample3
                                            text: "OK"
                                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                            onClicked: settings.ss2k_shift_step_sample_3 = ss2kShiftStepSample3TextField.text
                                        }
                                    }
                                    RowLayout {
                                        spacing: 10
                                        Label {
                                            id: labelSS2KResistanceSample4
                                            text: qsTr("Resistance Sample 4")
                                            Layout.fillWidth: true
                                        }
                                        TextField {
                                            id: ss2kResistanceSample4TextField
                                            text: settings.ss2k_resistance_sample_4
                                            horizontalAlignment: Text.AlignRight
                                            Layout.fillHeight: false
                                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                            inputMethodHints: Qt.ImhDigitsOnly
                                            onAccepted: settings.resistance_sample_4 = text
                                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                        }
                                        Button {
                                            id: okSS2kResistanceSample4
                                            text: "OK"
                                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                            onClicked: settings.ss2k_resistance_sample_4 = ss2kResistanceSample4TextField.text
                                        }
                                    }
                                    RowLayout {
                                        Label {
                                            id: labelSS2KShiftStepSample4
                                            text: qsTr("Shift Step Sample 4")
                                            Layout.fillWidth: true
                                        }
                                        TextField {
                                            id: ss2kShiftStepSample4TextField
                                            text: settings.ss2k_shift_step_sample_4
                                            horizontalAlignment: Text.AlignRight
                                            Layout.fillHeight: false
                                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                            inputMethodHints: Qt.ImhDigitsOnly
                                            onAccepted: settings.ss2k_shift_step_sample_4 = text
                                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                        }
                                        Button {
                                            id: okSS2kShiftStepSample4
                                            text: "OK"
                                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                            onClicked: settings.ss2k_shift_step_sample_4 = ss2kShiftStepSample4TextField.text
                                        }
                                    }
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

            NewPageElement {
                id: labelTTSSettings
                title: qsTr("TTS (Text to Speech) Settings 🔊")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                accordionContent: "settings-tts.qml"
            }

            AccordionElement {
                id: mapsAccordion
                title: qsTr("Maps 🗺️")
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
                            id: labelMapsType
                            text: qsTr("Maps Type:")
                            Layout.fillWidth: true
                        }
                        ComboBox {
                            id: mapsTypeTextField
                            model: [ "2D", "3D" ]
                            displayText: settings.maps_type
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onActivated: {
                                console.log("combomodel activated" + mapsTypeTextField.currentIndex)
                                displayText = mapsTypeTextField.currentValue
                            }

                        }
                        Button {
                            id: okMapsType
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.maps_type = mapsTypeTextField.displayText
                        }
                    }
                }
            }

            AccordionElement {
                id: videoAccordion
                title: qsTr("Video 🎥")
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
                            id: labelVideoWindow
                            text: qsTr("Window Time (sec.):")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: videoWindowTextField
                            text: settings.video_playback_window_s
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.video_playback_window_s = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okVideoWindow
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: settings.video_playback_window_s = videoWindowTextField.text
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
                    AccordionCheckElement {
                        id: virtualDeviceAccordion
                        title: qsTr("Enable Virtual Device")
                        linkedBoolSetting: "virtual_device_enabled"
                        settings: settings
                        accordionContent: ColumnLayout {
                            AccordionCheckElement {
                                id: virtualBeviceBluetoothAccordion
                                title: qsTr("Virtual Device Bluetooth")
                                linkedBoolSetting: "virtual_device_bluetooth"
                                settings: settings
                                accordionContent: ColumnLayout {
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
                                }
                            }
                            AccordionCheckElement {
                                id: dirconAccordion
                                title: qsTr("Wahoo direct connect")
                                linkedBoolSetting: "dircon_yes"
                                settings: settings
                                accordionContent: ColumnLayout {
                                    spacing: 0
                                    RowLayout {
                                        spacing: 10
                                        Label {
                                            id: labelDirconServerPort
                                            text: qsTr("Server Port:")
                                            Layout.fillWidth: true
                                        }
                                        TextField {
                                            id: dirconServerPortTextField
                                            text: settings.dircon_server_base_port
                                            horizontalAlignment: Text.AlignRight
                                            Layout.fillHeight: false
                                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                            inputMethodHints: Qt.ImhDigitsOnly
                                            onAccepted: settings.dircon_server_base_port = text
                                        }
                                        Button {
                                            id: okDirconServerPort
                                            text: "OK"
                                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                            onClicked: settings.dircon_server_base_port = dirconServerPortTextField.text
                                        }
                                    }
                                }
                            }
                        }
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
                        text: qsTr("Fake Device")
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
                        id: fakeTreadmillDelegate
                        text: qsTr("Fake Treadmill")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.fakedevice_treadmill
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.fakedevice_treadmill = checked
                    }

                    SwitchDelegate {
                        id: fakeEllipticalDelegate
                        text: qsTr("Fake Elliptical")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.fakedevice_elliptical
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.fakedevice_elliptical = checked
                    }

                    SwitchDelegate {
                        id: appleHeartCacheDelegate
                        text: qsTr("iOS Heart Caching")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.ios_cache_heart_device
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.ios_cache_heart_device = checked
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

                    Button {
                        id: clearLogs
                        text: "Clear History"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: rootItem.clearFiles();
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
