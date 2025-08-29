import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0
import QtQuick.Dialogs 1.0
import Qt.labs.platform 1.1

//Page {
    ScrollView {
        contentWidth: -1
        focus: true
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.fill: parent
        //anchors.bottom: footerSettings.top
        //anchors.bottomMargin: footerSettings.height + 10
        id: settingsPane        

        signal peloton_connect_clicked()

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
            property int video_playback_window_s: 12 // not used

            // from version 2.11.62
            property string horizon_treadmill_profile_user1: "user1"
            property string horizon_treadmill_profile_user2: "user2"
            property string horizon_treadmill_profile_user3: "user3"
            property string horizon_treadmill_profile_user4: "user4"
            property string horizon_treadmill_profile_user5: "user5"

            // from version 2.11.63
            property bool nordictrack_gx_2_7: false

            // from version 2.11.65
            property real rolling_resistance: 0.005

            // from version 2.11.67
            property bool eslinker_ypoo: false

            // from version 2.11.69
            property bool wahoo_rgt_dircon: false            

            // from version 2.11.73
            property bool tts_description_enabled: true

            // from version 2.11.80
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

            // from version 2.11.85
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

            // from version 2.11.87
            property bool nordictrack_t70_treadmill: false

            // from version 2.11.94
            property real crrGain: 0
            property real cwGain: 0

            // from version 2.12.1
            property bool proform_treadmill_cadence_lt: false

            // from version 2.12.3
            property bool trainprogram_stop_at_end: false

            // from version 2.12.5
            property bool domyos_elliptical_inclination: true
            property bool gpx_loop: false

            // from version 2.12.6
            property bool android_notification: false

            // from version 2.12.8
            property bool kingsmith_encrypt_v4: false

            // from versiomn 2.12.11
            property bool horizon_treadmill_disable_pause: false

            // from version 2.12.13
            property bool domyos_bike_500_profile_v1: false

            // from version 2.12.14
            property bool ss2k_peloton: false

            // from version 2.12.16
            property string computrainer_serialport: ""

            // from version 2.12.18
            property bool strava_virtual_activity: true

            // from version 2.12.29
            property bool powr_sensor_running_cadence_half_on_strava: false
            property bool nordictrack_ifit_adb_remote: false
            property int floating_height: 210
            property int floating_width: 370

            // from version 2.12.32
            property int floating_transparency: 80

            // from version 2.12.34
            property bool floating_startup: false

            // from version 2.12.35
            property bool norditrack_s25i_treadmill: false

            // from version 2.12.36
            property bool toorx_ftms_treadmill: false

            // from version 2.12.38
            property bool nordictrack_t65s_83_treadmill: false
            property bool horizon_treadmill_suspend_stats_pause: false

            // from version 2.12.39
            property bool sportstech_sx600: false

            // from version 2.12.41
            property bool sole_elliptical_inclination: false

            // from version 2.12.43
            property bool proform_hybrid_trainer_xt: false
            property bool gears_restore_value: false
            property int gears_current_value: 0 // unused

            // from version 2.12.44
            property bool tile_pace_last500m_enabled: true
            property int  tile_pace_last500m_order: 49

            // from version 2.12.51
            property bool treadmill_difficulty_gain_or_offset: false
            property bool pafers_treadmill_bh_iboxster_plus: false

            // from version 2.12.52
            property bool proform_cycle_trainer_400: false

            // from version 2.12.58
            property bool fitshow_treadmill_miles: false
            property bool proform_hybrid_trainer_PFEL03815: false
            property int schwinn_resistance_smooth: 0

            // from version 2.12.59
            property bool peloton_workout_ocr: false
            property bool peloton_bike_ocr: false

            // from version 2.12.60
            property double treadmill_inclination_override_0: 0.0
            property double treadmill_inclination_override_05: 0.5
            property double treadmill_inclination_override_10: 1.0
            property double treadmill_inclination_override_15: 1.5
            property double treadmill_inclination_override_20: 2.0
            property double treadmill_inclination_override_25: 2.5
            property double treadmill_inclination_override_30: 3.0
            property double treadmill_inclination_override_35: 3.5
            property double treadmill_inclination_override_40: 4.0
            property double treadmill_inclination_override_45: 4.5
            property double treadmill_inclination_override_50: 5.0
            property double treadmill_inclination_override_55: 5.5
            property double treadmill_inclination_override_60: 6.0
            property double treadmill_inclination_override_65: 6.5
            property double treadmill_inclination_override_70: 7.0
            property double treadmill_inclination_override_75: 7.5
            property double treadmill_inclination_override_80: 8.0
            property double treadmill_inclination_override_85: 8.5
            property double treadmill_inclination_override_90: 9.0
            property double treadmill_inclination_override_95: 9.5
            property double treadmill_inclination_override_100: 10.0
            property double treadmill_inclination_override_105: 10.5
            property double treadmill_inclination_override_110: 11.0
            property double treadmill_inclination_override_115: 11.5
            property double treadmill_inclination_override_120: 12.0
            property double treadmill_inclination_override_125: 12.5
            property double treadmill_inclination_override_130: 13.0
            property double treadmill_inclination_override_135: 13.5
            property double treadmill_inclination_override_140: 14.0
            property double treadmill_inclination_override_145: 14.5
            property double treadmill_inclination_override_150: 15.0

            // from version 2.12.61
            property bool sole_elliptical_e55: false
            property bool horizon_treadmill_force_ftms: false

            // from version 2.12.64
            property int treadmill_pid_heart_min: 0
            property int treadmill_pid_heart_max: 0

            // from version 2.12.65
            property bool nordictrack_elliptical_c7_5: false

            // from version 2.12.66
            property bool renpho_bike_double_resistance: false

            // from version 2.12.69
            property bool nordictrack_incline_trainer_x7i: false

            // from version 2.12.71
            property bool strava_auth_external_webbrowser: false

            // from version 2.12.72
            property bool gears_from_bike: false

            // from version 2.13.4
            property bool peloton_spinups_autoresistance: true

            // from version 2.13.10
            property bool eslinker_costaway: false

            // from version 2.13.14
            property double treadmill_inclination_ovveride_gain: 1.0
            property double treadmill_inclination_ovveride_offset: 0.0

            // from version 2.13.15
            property bool bh_spada_2_watt: false
            property bool tacx_neo2_peloton: false

            // from version 2.13.16
            property bool sole_treadmill_inclination_fast: false

            // from version 2.13.17
            property bool zwift_ocr: false

            // from version 2.13.18
            property bool gem_module_inclination: false

            // from version 2.13.19
            property bool treadmill_simulate_inclination_with_speed: false

            // from version 2.13.26
            property bool garmin_companion: false

            // from version 2.13.27
            property bool peloton_companion_workout_ocr: false

            // from version 2.13.31
            property bool iconcept_elliptical: false

            // from version 2.13.37
            property bool theme_tile_icon_enabled: true
            property string theme_tile_background_color: "#303030"
            property string theme_status_bar_background_color: "#800080"

            // from version 2.13.43
            property string theme_background_color: "#303030"
            property bool theme_tile_shadow_enabled: true
            property string theme_tile_shadow_color: "#9C27B0"

            // from version 2.13.44
            property double gears_gain: 1.0
            property double gears_current_value_f: 0

            // from version 2.13.45
            property bool proform_treadmill_8_0: false

            // from version 2.13.50
            property bool zero_zt2500_treadmill: false

            // from version 2.13.52
            property bool kingsmith_encrypt_v5: false

            // from version 2.13.58
            property int peloton_rower_level: 1

            // from version 2.13.61
            property bool tile_target_pace_enabled: false
            property int  tile_target_pace_order: 50
            property bool tts_act_target_pace: false

            // from version 2.13.62
            property string csafe_rower: ""

            // from version 2.13.63
            property string ftms_rower: "Disabled"

            // from version 2.13.71
            property int theme_tile_secondline_textsize: 12

            // from version 2.13.80
            property bool fakedevice_rower: false

            // from version 2.13.81
            property bool proform_bike_sb: false            

            // from version 2.13.86
            property bool zwift_workout_ocr: false

            // from version 2.13.96
            property bool zwift_ocr_climb_portal: false
            property int poll_device_time: 200

            // from version 2.13.99
            property bool proform_bike_PFEVEX71316_1: false
            property bool schwinn_bike_resistance_v3: false

            // from version 2.15.2
            property bool watt_ignore_builtin: true

            // from version 2.16.4
            property bool proform_treadmill_z1300i: false

            // from version 2.16.5
            property string ftms_bike: "Disabled"
            property string ftms_treadmill: "Disabled"

            // from version 2.16.6
            property real ant_speed_offset: 0
            property real ant_speed_gain: 1

            // from version 2.16.12
            property bool proform_rower_sport_rl: false

            // from version 2.16.13
            property bool strava_date_prefix: false

            // from version 2.16.17
            property bool race_mode: false

            // from version 2.16.22
            property bool proform_pro_1000_treadmill: false
            property bool saris_trainer: false

            // from version 2.16.23
            property bool proform_studio_NTEX71021: false
            property bool nordictrack_x22i: false

            // from version 2.16.25
            property bool iconsole_elliptical: false            
            property real autolap_distance: 0
            property bool nordictrack_s20_treadmill: false

            // from version 2.16.28
            property bool freemotion_coachbike_b22_7: false

            // from version 2.16.29
            property bool proform_cycle_trainer_300_ci: false
            property bool kingsmith_encrypt_g1_walking_pad: false
            property bool proform_bike_225_csx: false

            // from version 2.16.30
            property bool proform_treadmill_l6_0s: false
            property string proformtdf1ip: ""
            property string zwift_username: ""
            property string zwift_password: ""

            // from version 2.16.31
            property bool garmin_bluetooth_compatibility: false
            property bool norditrack_s25_treadmill: false
            property bool proform_8_5_treadmill: false
            property real treadmill_incline_min: -100
            property real treadmill_incline_max: 100

            // from version 2.16.32
            property bool proform_2000_treadmill: false
            property bool android_documents_folder: false
            property bool zwift_api_autoinclination: true

            // from version 2.16.34
            property real domyos_treadmill_button_5kmh: 5.0
            property real domyos_treadmill_button_10kmh: 10.0
            property real domyos_treadmill_button_16kmh: 16.0
            property real domyos_treadmill_button_22kmh: 22.0

            // from version 2.16.35
            property bool proform_treadmill_sport_8_5: false

            // from version 2.16.37
            property bool domyos_treadmill_t900a: false

            // from version 2.16.38
            property bool enerfit_SPX_9500: false

            // from version 2.16.39
            property bool proform_treadmill_505_cst: false

            // from version 2.16.40
            property bool nordictrack_treadmill_t8_5s: false
            
            // from version 2.16.41
            property bool proform_treadmill_705_cst: false
            property bool zwift_click: false

            // from version 2.16.42
            property bool hop_sport_hs_090h_bike: false
            property bool zwift_play: false
            property bool nordictrack_treadmill_x14i: false
            property int zwift_api_poll: 5

            // from version 2.16.43
            property bool tile_step_count_enabled: false
            property int  tile_step_count_order: 51            

            // from version 2.16.44
            property bool tile_erg_mode_enabled: false
            property int  tile_erg_mode_order: 52

            // from version 2.16.45
            property bool toorx_srx_3500: false  
            property real inclination_delay_seconds: 0.0

            // from version 2.16.47
            property string ergDataPoints: ""
            property bool proform_tdf_10_0: false
            property bool proform_carbon_tl: false
            property bool proform_proshox2: false            

            // from version 2.16.51
            property bool nordictrack_GX4_5_bike: false            

            // from version 2.16.52
            property real ftp_run: 200.0
            property bool tile_rss_enabled: false
            property int  tile_rss_order: 53
            property string treadmillDataPoints: ""

            // from version 2.16.54
            property bool nordictrack_s20i_treadmill: false
            property bool stryd_speed_instead_treadmill: false
            property bool proform_595i_proshox2: false

            // from version 2.16.55
            property bool proform_treadmill_8_7: false

            // from version 2.16.56
            property bool proform_bike_325_csx: false

            // from version 2.16.58
            property string strava_upload_mode: "Always"

            // from version 2.16.59
            property bool proform_treadmill_705_cst_V78_239: false

            // from version 2.16.62
            property bool stryd_add_inclination_gain: false
            property bool toorx_bike_srx_500: false

            // from version 2.16.66
            property bool atletica_lightspeed_treadmill: false

            // from version 2.16.68
            property int peloton_treadmill_level: 1
            property bool nordictrackadbbike_resistance: false
            property bool proform_treadmill_carbon_t7: false
            property bool nordictrack_treadmill_exp_5i: false
            property int dircon_id: 0
            property string proform_elliptical_ip: ""

            // from version 2.16.69
            property bool antbike: false
            property bool domyosbike_notfmts: false

            // from version 2.16.70
            property bool gears_volume_debouncing: false
            property bool tile_biggears_enabled: false
            property int  tile_biggears_order: 54
            property bool domyostreadmill_notfmts: false
            property bool zwiftplay_swap: false
            property bool gears_zwift_ratio: false
            property bool domyos_bike_500_profile_v2: false
            property double gears_offset: 0.0

            property bool proform_carbon_tl_PFTL59720: false

            // from version 2.16.71
            property bool proform_treadmill_sport_70: false
            property string peloton_date_format: "MM/dd/yy"
            property bool force_resistance_instead_inclination: false
            property bool proform_treadmill_575i: false

            // from version 2.18.1
            property bool zwift_play_emulator: false

            // from version 2.18.2
            property string gear_configuration: "1|38|44|true\n2|38|38|true\n3|38|32|true\n4|38|28|true\n5|38|24|true\n6|38|21|true\n7|38|19|true\n8|38|17|true\n9|38|15|true\n10|38|13|true\n11|38|11|true\n12|38|10|true"
            property int gear_crankset_size: 42
            property int gear_cog_size: 14
            property string gear_wheel_size: "700 x 18C"
            property real gear_circumference: 2070

            property bool watt_bike_emulator: false

            property bool restore_specific_gear: false
            property bool skipLocationServicesDialog: false
            property bool trainprogram_pid_pushy: true
            property real min_inclination: -999

            // from version 2.18.3
            property bool proform_performance_400i: false

            // from version 2.18.5
            property bool proform_treadmill_c700: false
            property bool sram_axs_controller: false
            property bool proform_treadmill_c960i: false

            // from version 2.18.6
            property string mqtt_host: ""
            property int mqtt_port: 1883
            property string mqtt_username: ""
            property string mqtt_password: ""
            property string mqtt_deviceid: "default"
            property bool peloton_auto_start_with_intro: false
            property bool peloton_auto_start_without_intro: false

            // from version 2.18.7
            property bool nordictrack_tseries5_treadmill: false
            property bool proform_carbon_tl_PFTL59722c: false

            // from version 2.18.9
            property bool nordictrack_gx_44_pro: false

            // from version 2.18.10
            property string csafe_elliptical_port: ""
            property string osc_ip: ""
            property int osc_port: 9000

            // from version 2.18.11
            property bool strava_treadmill: true
            property bool iconsole_rower: false

            // from version 2.18.14
            property bool proform_treadmill_1500_pro: false

            // from version 2.18.15
            property bool proform_505_cst_80_44: false

            // from version 2.18.16
            property bool proform_trainer_8_0: false

            // from version 2.18.18
            property bool tile_biggears_swap: false
            property bool treadmill_follow_wattage: false
            property bool fit_file_garmin_device_training_effect: false
            property bool proform_treadmill_705_cst_V80_44: false

            // from version 2.18.19
            property string  peloton_accesstoken: ""
            property string  peloton_refreshtoken: ""
            property string  peloton_lastrefresh: ""
            property string  peloton_expires: ""
            property string  peloton_code: ""

            property bool nordictrack_treadmill_1750_adb: false

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

            property bool proform_bike_PFEVEX71316_0: false
            property bool real_inclination_to_virtual_treamill_bridge: false
            property bool stryd_inclination_instead_treadmill: false

            // 2.18.20
            property bool domyos_elliptical_fmts: false
            property bool proform_xbike: false            
            property bool proform_225_csx_PFEX32925_INT_0: false
            property string peloton_current_user_id: ""

            // 2.18.22
            property bool trainprogram_pid_ignore_inclination: false
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

            // 2.18.25
            property bool zwift_gear_ui_aligned: false
            property bool tacxneo2_disable_negative_inclination: false

            // 2.18.26
            property bool proform_performance_300i: false
            property bool android_antbike: false

            property bool tile_coretemperature_enabled: false
            property int  tile_coretemperature_order: 67

            property bool nordictrack_t65s_treadmill_81_miles: false
            property bool nordictrack_elite_800: false
            property bool ios_btdevice_native: false            
            property string inclinationResistancePoints: ""
            property int floatingwindow_type: 0
            property bool horizon_treadmill_7_0_at_24: false  // not used

            property bool nordictrack_treadmill_ultra_le: false            

            property bool tile_heat_time_in_zone_1_enabled: false
            property int  tile_heat_time_in_zone_1_order: 68
            property bool tile_heat_time_in_zone_2_enabled: false
            property int  tile_heat_time_in_zone_2_order: 69
            property bool tile_heat_time_in_zone_3_enabled: false
            property int  tile_heat_time_in_zone_3_order: 70
            property bool tile_heat_time_in_zone_4_enabled: false
            property int  tile_heat_time_in_zone_4_order: 71

            property bool proform_treadmill_carbon_tls: false
            
            // 2.19.1            
            property bool proform_treadmill_995i: false
            property bool rogue_echo_bike: false
            property int fit_file_garmin_device_training_effect_device: 3122            

            // 2.19.2
            property bool tile_hr_time_in_zone_individual_mode: false
            property bool wahoo_without_wheel_diameter: false

            // 2.20.3
            property bool technogym_group_cycle: false
            property int ant_bike_device_number: 0
            property int ant_heart_device_number: 0
            property int peloton_treadmill_walk_level: 1
            property int pid_heart_zone_erg_mode_watt_step: 5
            
            // Automatic Virtual Shifting settings
            property bool automatic_virtual_shifting_enabled: false
            property int automatic_virtual_shifting_gear_up_cadence: 95
            property real automatic_virtual_shifting_gear_up_time: 2.0
            property int automatic_virtual_shifting_gear_down_cadence: 65
            property real automatic_virtual_shifting_gear_down_time: 2.0
            property int automatic_virtual_shifting_profile: 0
            property int automatic_virtual_shifting_climb_gear_up_cadence: 95
            property real automatic_virtual_shifting_climb_gear_up_time: 2.0
            property int automatic_virtual_shifting_climb_gear_down_cadence: 65
            property real automatic_virtual_shifting_climb_gear_down_time: 2.0
            property int automatic_virtual_shifting_sprint_gear_up_cadence: 95
            property real automatic_virtual_shifting_sprint_gear_up_time: 2.0
            property int automatic_virtual_shifting_sprint_gear_down_cadence: 65
            property real automatic_virtual_shifting_sprint_gear_down_time: 2.0
            property bool tile_auto_virtual_shifting_cruise_enabled: false
            property int tile_auto_virtual_shifting_cruise_order: 55
            property bool tile_auto_virtual_shifting_climb_enabled: false
            property int tile_auto_virtual_shifting_climb_order: 56
            property bool tile_auto_virtual_shifting_sprint_enabled: false
            property int tile_auto_virtual_shifting_sprint_order: 57
            property string proform_rower_ip: ""
            property string ftms_elliptical: "Disabled"
            property bool calories_active_only: false
            property real height: 175.0
            property bool calories_from_hr: false
            property int bike_power_offset: 0
            property int chart_display_mode: 0
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

        Component.onCompleted: window.settings_restart_to_apply = false;

        ColumnLayout {
            id: column1
            spacing: 0
            anchors.fill: parent

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
                            onClicked: { settings.ui_zoom = uiZoomTextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                        }
                    }
                    Label {
                        text: qsTr("This changes the size of the tiles that display your metrics. The default is 100%. To fit more tiles on your screen, choose a smaller percentage. To make them larger, choose a percentage over 100%. Do not enter the percent symbol")
                        font.bold: true
                        font.italic: true
                        font.pixelSize: Qt.application.font.pixelSize - 2
                        textFormat: Text.PlainText
                        wrapMode: Text.WordWrap
                        verticalAlignment: Text.AlignVCenter
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        width: column1.width * 0.8
                        Layout.fillWidth: true
                        color: Material.color(Material.Lime)
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
                            onClicked: { settings.weight = (settings.miles_unit?weightTextField.text / 2.20462:weightTextField.text); toast.show("Setting saved!"); }
                        }
                    }
                    Label {
                        text: qsTr("Enter your weight in kilograms so QZ can more accurately calculate calories burned. NOTE: If you choose to use miles as the unit for distance traveled, you will be asked to enter your weight in pounds (lbs).")
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
                            id: labelHeight
                            text: qsTr("Player Height") + "(" + (settings.miles_unit?"ft/in":"cm") + ")"
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: heightTextField
                            text: settings.miles_unit ? Math.floor(settings.height / 30.48) + "'" + Math.round((settings.height % 30.48) / 2.54) + '"' : settings.height
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            //inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: {
                                if (settings.miles_unit) {
                                    var parts = text.match(/(\d+)'(\d+)"/);
                                    if (parts) {
                                        settings.height = parseInt(parts[1]) * 30.48 + parseInt(parts[2]) * 2.54;
                                    }
                                } else {
                                    settings.height = text;
                                }
                            }
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okHeightButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: {
                                if (settings.miles_unit) {
                                    var parts = heightTextField.text.match(/(\d+)'(\d+)"/);
                                    if (parts) {
                                        settings.height = parseInt(parts[1]) * 30.48 + parseInt(parts[2]) * 2.54;
                                    }
                                } else {
                                    settings.height = heightTextField.text;
                                }
                                toast.show("Setting saved!");
                            }
                        }
                    }
                    Label {
                        text: qsTr("Enter your height for more accurate BMR and active calories calculation. Use centimeters for metric or feet'inches\" format (e.g., 5'10\") for imperial units.")
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
                            onClicked: { settings.age = ageTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Enter your age so that calories burned can be more accurately calculated.")
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
                            onClicked: { settings.sex = sexTextField.displayText; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Select your gender so that calories burned can be more accurately calculated.")
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
                            onClicked: { settings.ftp = ftpTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("If you train to specific output (or watts) levels, for example in Peloton Power Zone classes,and have taken an FTP test (Functional Threshold Power), enter your FTP here. This number is used to calculate your Power Zones (Zones 1 to 7 for Peloton and 1 to 6 for Zwift).")
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
                            text: qsTr("Critical Power Run value:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: ftpRunTextField
                            text: settings.ftp_run
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.ftp_run = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.ftp_run = ftpRunTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("If you train to specific output (or watts) levels, for example with Stryd,and have taken an CP test (Critical Power Test), enter your CP here. This number is used to calculate your RSS.")
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
                            onClicked: { settings.user_nickname = nicknameTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("No need to enter data here. It is for a possible future QZ feature.")
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
                            onClicked: { settings.user_email = emailTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Enter your email address to receive an automated email with stats and charts when you hit STOP at the end of each workout. Make sure there are no spaces before or after the email address; this is the most common reason the automated email is not sent. Privacy Note: Email addresses are not collected by the developer and are only saved locally on your device.")
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

                    IndicatorOnlySwitch {
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

                    Label {
                        text: qsTr("Turn on if you want QZ to display distance traveled in miles. Default is off and set to kilometers.")
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

                    IndicatorOnlySwitch {
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

                    Label {
                        text: qsTr("Turn on to set QZ to always open in PAUSE mode. This is important for Peloton classes so that you can sync the start of your QZ workout with the start of the Peloton class. Turn off to have QZ start tracking and timing your workout as soon as it opens.")
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

                    IndicatorOnlySwitch {
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

                    Label {
                        text: qsTr("Turn this on for: - Peloton Bootcamp classes or other workouts that are on and off the bike or treadmill. QZ will continue to track your workout even when you step away from your equipment. - Capturing non-equipment-based workouts, such as yoga or strength training. NOTE: All such workouts are labeled as “Rides” in Strava, but you can edit the label in Strava.")
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

            /*Label {
                id: zwiftInfoLabel
                text: qsTr("Zwift users: keep this setting off")
                font.bold: yes
                font.italic: yes
                font.pixelSize: Qt.application.font.pixelSize - 2
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
                    spacing: 0
                    IndicatorOnlySwitch {
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
                        onClicked: { settings.bike_heartrate_service = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("(For Android Version 10 and above, this setting cannot be changed. This setting can be changed for Android Version 9 and below and for iOS.) When this setting is turned off, QZ sends heart rate data in a format designed to improve compatibility with third-party apps, such as Zwift and Peloton. Default is off.")
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

                    IndicatorOnlySwitch {
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

                    Label {
                        text: qsTr("Turn this on to prevent a built-in heart rate monitor (HRM) on your exercise equipment from sending that data to QZ. This allows QZ to connect to your external HRM, such as a chest band or Apple Watch.")
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

                    IndicatorOnlySwitch {
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

                    Label {
                        text: qsTr("This prevents your bike or treadmill from sending its calories-burned calculation to QZ and defaults to QZ's more accurate calculation.")
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

                    IndicatorOnlySwitch {
                        id: switchActiveCaloriesOnlyDelegate
                        text: qsTr("Calculate Active Calories Only")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.calories_active_only
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.calories_active_only = checked
                    }

                    Label {
                        text: qsTr("Enable to calculate only active calories (excluding basal metabolic rate) similar to Apple Watch. When disabled, total calories including BMR are calculated. This affects both display and Apple Health integration.")
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

                    IndicatorOnlySwitch {
                        id: switchCaloriesFromHRDelegate
                        text: qsTr("Calculate Calories from Heart Rate")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.calories_from_hr
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.calories_from_hr = checked
                    }

                    Label {
                        text: qsTr("Enable to calculate calories based on heart rate data instead of power. Requires heart rate sensor connection for accurate calorie estimation.")
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

                    Label {
                        id: labelHeartRateBelt
                        text: qsTr("Heart Belt Name:")
                        Layout.fillWidth: true
                    }
                    RowLayout {
                        spacing: 10
                        ComboBox {
                            id: heartBeltNameTextField
                            model: rootItem.bluetoothDevices
                            displayText: settings.heart_rate_belt_name
                            Layout.fillHeight: false
                            Layout.fillWidth: true
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
                            onClicked: { settings.heart_rate_belt_name = heartBeltNameTextField.displayText; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        id: appleWatchLabel
                        text: qsTr("Apple Watch users: leave it disabled! Just open the app on your watch")
                        font.bold: true
                        font.italic: true
                        font.pixelSize: Qt.application.font.pixelSize - 2
                        textFormat: Text.PlainText
                        wrapMode: Text.WordWrap
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillWidth: true
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
                                    onClicked: { settings.heart_rate_zone1 = heartRateZone1TextField.text; toast.show("Setting saved!"); }
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
                                    onClicked: { settings.heart_rate_zone2 = heartRateZone2TextField.text; toast.show("Setting saved!"); }
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
                                    onClicked: { settings.heart_rate_zone3 = heartRateZone3TextField.text; toast.show("Setting saved!"); }
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
                                    onClicked: { settings.heart_rate_zone4 = heartRateZone4TextField.text; toast.show("Setting saved!"); }
                                }
                            }

                            Label {
                                text: qsTr("Zone 5 will be calculated automatically based on Zone 4 end percentage and max HR.")
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

                            Label {
                                text: qsTr("Choose the percentages for where you want your zones 1-4 to end and click OK.")
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

                            AccordionElement {
                                id: heartRatemaxOverrideAccordion
                                title: qsTr("Heart Rate Max Override")
                                indicatRectColor: Material.color(Material.Grey)
                                textColor: Material.color(Material.Red)
                                color: Material.backgroundColor
                                accordionContent: ColumnLayout {
                                    spacing: 10
                                    IndicatorOnlySwitch {
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
                                            onClicked: { settings.heart_max_override_value = heartRateMaxOverrideValueTextField.text; toast.show("Setting saved!"); }
                                        }
                                    }
                                }
                            }

                            Label {
                                text: qsTr("QZ uses a standard age-based calculation for maximum heart rate and then sets the heart rate zones based on that max heart rate. If you know your actual max heart rate (the highest your heart rate is known to reach), turn this option on and enter your actual max heart rate. Then click OK.")
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
                                            onClicked: { settings.power_hr_pwr1 = powerFromHeartPWR1TextField.text; toast.show("Setting saved!"); }
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
                                            onClicked: { settings.power_hr_hr1 = powerFromHeartHR1TextField.text; toast.show("Setting saved!"); }
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
                                            onClicked: { settings.power_hr_pwr2 = powerFromHeartPWR2TextField.text; toast.show("Setting saved!"); }
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
                                            onClicked: { settings.power_hr_hr2 = powerFromHeartHR2TextField.text; toast.show("Setting saved!"); }
                                        }
                                    }
                                }
                            }
                            Label {
                                text: qsTr("Expand the bars to the right to display the options under this setting. These settings are used to calculate power (watts) for bikes that do not have power meters. Instead QZ estimates power from your cadence and heart rate. You can calibrate how QZ calculates your power from heart rate as follows: If you know that at a stable pace you produce 100W of power at a heart rate of 150 BPM and 150W at 170 BPM, you can add these values under Sessions 1 and 2 Watt and HR and QZ will calculate your power based on that trend line.")
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
                    IndicatorOnlySwitch {
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
                    Label {
                        text: qsTr("QZ calculates speed based on your pedal cadence (RPMs). Enable this setting if you want your speed to be calculated based on your power output (watts), as Zwift and some other apps do. Default is off.")
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

                    IndicatorOnlySwitch {
                        id: gearsRestoreDelegate
                        text: qsTr("Restore Gears on Startup")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.gears_restore_value
                        enabled: !gearsRestoreValueDelegate.checked
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.gears_restore_value = checked
                    }

                    Label {
                        text: qsTr("QZ will remember the last Gears value and it will restore on startup")
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

                    // Add the new specific gear value restore setting
                    IndicatorOnlySwitch {
                        id: gearsRestoreValueDelegate
                        text: qsTr("Restore Specific Gear Value")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.restore_specific_gear
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: {
                            settings.restore_specific_gear = checked
                            if (checked) {
                                settings.gears_restore_value = false
                            }
                        }
                    }

                    RowLayout {
                        spacing: 10
                        enabled: gearsRestoreValueDelegate.checked
                        opacity: enabled ? 1.0 : 0.5

                        Label {
                            text: qsTr("Gear Value:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: specificGearValueField
                            text: settings.gears_current_value_f
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            enabled: gearsRestoreValueDelegate.checked
                            onAccepted: settings.gears_current_value_f = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            enabled: gearsRestoreValueDelegate.checked
                            onClicked: {
                                settings.gears_current_value_f = specificGearValueField.text
                                toast.show("Setting saved!")
                            }
                        }
                    }

                    Label {
                        text: qsTr("Specify a particular gear value to be restored at startup. This will override the 'Restore Gears on Startup' setting.")
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
                            id: labelRollingResistance
                            text: qsTr("Rolling Resistance Factor")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: rollingreistanceTextField
                            text: settings.rolling_resistance
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            //inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.rolling_resistance = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okRollingResistanceButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.rolling_resistance = rollingreistanceTextField.text; toast.show("Setting saved!"); }
                        }
                    }
                    Label {
                        id: labelRollingResistanceInfo
                        text: qsTr("0.005 = Clinchers\n0.004 = Tubulars\n0.012 = MTB")
                        font.italic: true
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        font.pixelSize: Qt.application.font.pixelSize - 2
                        color: "steelblue"
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
                            onClicked: { settings.bike_weight = (settings.miles_unit?bikeweightTextField.text / 2.20462:bikeweightTextField.text); toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Enables QZ to include the weight of your bike when calculating speed. For example, if you are competing against yourself on VZfit, adding bike weight will “level the playing field” against your virtual self. If you have set QZ to calculate distance in miles, enter the bike weight in pounds (lbs). Default unit is kilograms (kgs).")
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
                            id: labelCRRGain
                            text: qsTr("Rolling Res. Gain")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: crrGainTextField
                            text: settings.crrGain
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            //inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.crrGain = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okCRRGainButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.crrGain = crrGainTextField.text; toast.show("Setting saved!"); }
                        }
                    }
                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelCWGain
                            text: qsTr("Wind Res. Gain")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: cwGainTextField
                            text: settings.cwGain
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            //inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.cwGain = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okCWGainButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.cwGain = cwGainTextField.text; toast.show("Setting saved!"); }
                        }
                    }
                    IndicatorOnlySwitch {
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

                    Label {
                        text: qsTr("Enable this setting ONLY when using Zwift in ERG (workout) Mode. QZ will communicate the target resistance (or automatically adjust your resistance if your bike has this capability) to match the target watts based on your cadence (RPM). In ERG Mode, the changes in road slope will not affect target resistance, as is the case in Simulation Mode. Default is off.")
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
                            onClicked: { settings.bike_resistance_offset = bikeResistanceOffsetTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("This setting sets your “flat road” in Zwift. All communicated resistance changes will be based on this setting. The value entered is personal preference and will be dependent on your level of fitness. The suggested value for Echelon bikes is between 18 and 20. Default is 4.")
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
                            id: labelBikePowerOffset
                            text: qsTr("Zwift Power Offset (W):")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: bikePowerOffsetTextField
                            text: settings.bike_power_offset
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.bike_power_offset = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okBikePowerOffsetButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.bike_power_offset = bikePowerOffsetTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Add an offset in watts to the requested power from apps like Zwift. Positive values increase power, negative values decrease it. Default is 0.")
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
                            onClicked: { settings.bike_resistance_gain_f = bikeResistanceGainTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("(for bikes and treadmills when using “treadmill as a bike” setting). This setting scales the resistance from your bike or the speed from your treadmill before sending it to Zwift. Default is 1.")
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
                            onClicked: { settings.zwift_erg_filter = zwiftErgFilterTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("In ERG Mode or during a Power Zone workout on Peloton, the app sends a “target output” request. If the output requested doesn’t match your current output (calculated using cadence and resistance level), your target resistance will change to help you get closer to the target output. If the filter is set to higher values, you will get less adjustment of the target resistance and you will have to increase your cadence to match the target output. The Up and Down Watt Filter settings are the upper and lower margin before the adjustment of resistance is communicated. Example: if the up and down filters are set to 10 and the target output is 100 watts, a change of your resistance will only be communicated if your bike produces less than 90 Watts or more than 110 Watts. Default is 10.")
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
                            onClicked: { settings.zwift_erg_filter_down = zwiftErgDownFilterTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("See above. Default is 10.")
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
                            id: labelZwiftErgResistanceDown
                            text: qsTr("Min. Resistance:")
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
                            onClicked: { settings.zwift_erg_resistance_down = zwiftErgResistanceDownTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Use this setting to set a minimum target resistance. For example, if you do not want to ride at a resistance below 25, enter a value of 25 and QZ will not set a target resistance below 25. Default is 0.")
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
                            id: labelZwiftErgResistanceUp
                            text: qsTr("Max. Resistance:")
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
                            onClicked: { settings.zwift_erg_resistance_up = zwiftErgResistanceUpTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Similar to the above, but sets a maximum target resistance. Default is 999.")
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
                            onClicked: { settings.bike_resistance_start = bikeResistanceStartTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("(only for bikes with electronically-controlled resistance): Enter the resistance level you want QZ to set at startup. Default is 1.")
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
                            text: qsTr("Gears Gain:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: gearsGainTextField
                            text: settings.gears_gain
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            //inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.gears_gain = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.gears_gain = gearsGainTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Applies a multiplier to the gears. Default is 1.")
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
                            text: qsTr("Gears Offset:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: gearsOffsetTextField
                            text: settings.gears_offset
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            //inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.gears_offset = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.gears_offset = gearsOffsetTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Applies an offset to the gears. Default is 0.")
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

                    AccordionElement {
                        id: automaticVirtualShiftingAccordion
                        title: qsTr("Automatic Virtual Shifting")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Red)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            spacing: 0

                            IndicatorOnlySwitch {
                                id: automaticVirtualShiftingEnabledDelegate
                                text: qsTr("Enable Automatic Virtual Shifting")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.automatic_virtual_shifting_enabled
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: settings.automatic_virtual_shifting_enabled = checked
                            }

                            Label {
                                text: qsTr("Enable automatic gear shifting based on cadence thresholds. When enabled, QZ will automatically shift gears up or down based on your pedaling cadence.")
                                font.bold: true
                                font.italic: true
                                font.pixelSize: Qt.application.font.pixelSize - 2
                                textFormat: Text.PlainText
                                wrapMode: Text.WordWrap
                                verticalAlignment: Text.AlignVCenter
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                color: Material.color(Material.Lime)
                                visible: automaticVirtualShiftingEnabledDelegate.checked
                            }

                            RowLayout {
                                spacing: 10
                                visible: automaticVirtualShiftingEnabledDelegate.checked
                                Label {
                                    text: qsTr("Profile:")
                                    Layout.fillWidth: true
                                }
                                ComboBox {
                                    id: automaticVirtualShiftingProfileComboBox
                                    model: ["Cruise", "Climb", "Sprint"]
                                    currentIndex: settings.automatic_virtual_shifting_profile
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onActivated: {
                                        settings.automatic_virtual_shifting_profile = currentIndex
                                    }
                                }
                            }

                            Label {
                                text: qsTr("Cruise Profile Settings")
                                font.bold: true
                                font.pixelSize: Qt.application.font.pixelSize + 1
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                color: Material.color(Material.Grey)
                                visible: automaticVirtualShiftingEnabledDelegate.checked
                            }

                            RowLayout {
                                spacing: 10
                                visible: automaticVirtualShiftingEnabledDelegate.checked
                                Label {
                                    text: qsTr("Cruise - Gear Up Cadence (RPM):")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: automaticVirtualShiftingGearUpCadenceTextField
                                    text: settings.automatic_virtual_shifting_gear_up_cadence
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    onAccepted: settings.automatic_virtual_shifting_gear_up_cadence = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.automatic_virtual_shifting_gear_up_cadence = automaticVirtualShiftingGearUpCadenceTextField.text; toast.show("Setting saved!"); }
                                }
                            }

                            RowLayout {
                                spacing: 10
                                visible: automaticVirtualShiftingEnabledDelegate.checked
                                Label {
                                    text: qsTr("Cruise - Gear Up Time (seconds):")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: automaticVirtualShiftingGearUpTimeTextField
                                    text: settings.automatic_virtual_shifting_gear_up_time
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                                    onAccepted: settings.automatic_virtual_shifting_gear_up_time = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.automatic_virtual_shifting_gear_up_time = automaticVirtualShiftingGearUpTimeTextField.text; toast.show("Setting saved!"); }
                                }
                            }

                            RowLayout {
                                spacing: 10
                                visible: automaticVirtualShiftingEnabledDelegate.checked
                                Label {
                                    text: qsTr("Cruise - Gear Down Cadence (RPM):")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: automaticVirtualShiftingGearDownCadenceTextField
                                    text: settings.automatic_virtual_shifting_gear_down_cadence
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    onAccepted: settings.automatic_virtual_shifting_gear_down_cadence = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.automatic_virtual_shifting_gear_down_cadence = automaticVirtualShiftingGearDownCadenceTextField.text; toast.show("Setting saved!"); }
                                }
                            }

                            RowLayout {
                                spacing: 10
                                visible: automaticVirtualShiftingEnabledDelegate.checked
                                Label {
                                    text: qsTr("Cruise - Gear Down Time (seconds):")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: automaticVirtualShiftingGearDownTimeTextField
                                    text: settings.automatic_virtual_shifting_gear_down_time
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                                    onAccepted: settings.automatic_virtual_shifting_gear_down_time = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.automatic_virtual_shifting_gear_down_time = automaticVirtualShiftingGearDownTimeTextField.text; toast.show("Setting saved!"); }
                                }
                            }

                            Label {
                                text: qsTr("Climb Profile Settings")
                                font.bold: true
                                font.pixelSize: Qt.application.font.pixelSize + 1
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                color: Material.color(Material.Grey)
                                visible: automaticVirtualShiftingEnabledDelegate.checked
                            }

                            RowLayout {
                                spacing: 10
                                visible: automaticVirtualShiftingEnabledDelegate.checked
                                Label {
                                    text: qsTr("Climb - Gear Up Cadence (RPM):")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: automaticVirtualShiftingClimbGearUpCadenceTextField
                                    text: settings.automatic_virtual_shifting_climb_gear_up_cadence
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    onAccepted: settings.automatic_virtual_shifting_climb_gear_up_cadence = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.automatic_virtual_shifting_climb_gear_up_cadence = automaticVirtualShiftingClimbGearUpCadenceTextField.text; toast.show("Setting saved!"); }
                                }
                            }

                            RowLayout {
                                spacing: 10
                                visible: automaticVirtualShiftingEnabledDelegate.checked
                                Label {
                                    text: qsTr("Climb - Gear Up Time (seconds):")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: automaticVirtualShiftingClimbGearUpTimeTextField
                                    text: settings.automatic_virtual_shifting_climb_gear_up_time
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                                    onAccepted: settings.automatic_virtual_shifting_climb_gear_up_time = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.automatic_virtual_shifting_climb_gear_up_time = automaticVirtualShiftingClimbGearUpTimeTextField.text; toast.show("Setting saved!"); }
                                }
                            }

                            RowLayout {
                                spacing: 10
                                visible: automaticVirtualShiftingEnabledDelegate.checked
                                Label {
                                    text: qsTr("Climb - Gear Down Cadence (RPM):")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: automaticVirtualShiftingClimbGearDownCadenceTextField
                                    text: settings.automatic_virtual_shifting_climb_gear_down_cadence
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    onAccepted: settings.automatic_virtual_shifting_climb_gear_down_cadence = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.automatic_virtual_shifting_climb_gear_down_cadence = automaticVirtualShiftingClimbGearDownCadenceTextField.text; toast.show("Setting saved!"); }
                                }
                            }

                            RowLayout {
                                spacing: 10
                                visible: automaticVirtualShiftingEnabledDelegate.checked
                                Label {
                                    text: qsTr("Climb - Gear Down Time (seconds):")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: automaticVirtualShiftingClimbGearDownTimeTextField
                                    text: settings.automatic_virtual_shifting_climb_gear_down_time
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                                    onAccepted: settings.automatic_virtual_shifting_climb_gear_down_time = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.automatic_virtual_shifting_climb_gear_down_time = automaticVirtualShiftingClimbGearDownTimeTextField.text; toast.show("Setting saved!"); }
                                }
                            }

                            Label {
                                text: qsTr("Sprint Profile Settings")
                                font.bold: true
                                font.pixelSize: Qt.application.font.pixelSize + 1
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                color: Material.color(Material.Grey)
                                visible: automaticVirtualShiftingEnabledDelegate.checked
                            }

                            RowLayout {
                                spacing: 10
                                visible: automaticVirtualShiftingEnabledDelegate.checked
                                Label {
                                    text: qsTr("Sprint - Gear Up Cadence (RPM):")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: automaticVirtualShiftingSprintGearUpCadenceTextField
                                    text: settings.automatic_virtual_shifting_sprint_gear_up_cadence
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    onAccepted: settings.automatic_virtual_shifting_sprint_gear_up_cadence = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.automatic_virtual_shifting_sprint_gear_up_cadence = automaticVirtualShiftingSprintGearUpCadenceTextField.text; toast.show("Setting saved!"); }
                                }
                            }

                            RowLayout {
                                spacing: 10
                                visible: automaticVirtualShiftingEnabledDelegate.checked
                                Label {
                                    text: qsTr("Sprint - Gear Up Time (seconds):")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: automaticVirtualShiftingSprintGearUpTimeTextField
                                    text: settings.automatic_virtual_shifting_sprint_gear_up_time
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                                    onAccepted: settings.automatic_virtual_shifting_sprint_gear_up_time = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.automatic_virtual_shifting_sprint_gear_up_time = automaticVirtualShiftingSprintGearUpTimeTextField.text; toast.show("Setting saved!"); }
                                }
                            }

                            RowLayout {
                                spacing: 10
                                visible: automaticVirtualShiftingEnabledDelegate.checked
                                Label {
                                    text: qsTr("Sprint - Gear Down Cadence (RPM):")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: automaticVirtualShiftingSprintGearDownCadenceTextField
                                    text: settings.automatic_virtual_shifting_sprint_gear_down_cadence
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    onAccepted: settings.automatic_virtual_shifting_sprint_gear_down_cadence = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.automatic_virtual_shifting_sprint_gear_down_cadence = automaticVirtualShiftingSprintGearDownCadenceTextField.text; toast.show("Setting saved!"); }
                                }
                            }

                            RowLayout {
                                spacing: 10
                                visible: automaticVirtualShiftingEnabledDelegate.checked
                                Label {
                                    text: qsTr("Sprint - Gear Down Time (seconds):")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: automaticVirtualShiftingSprintGearDownTimeTextField
                                    text: settings.automatic_virtual_shifting_sprint_gear_down_time
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                                    onAccepted: settings.automatic_virtual_shifting_sprint_gear_down_time = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.automatic_virtual_shifting_sprint_gear_down_time = automaticVirtualShiftingSprintGearDownTimeTextField.text; toast.show("Setting saved!"); }
                                }
                            }
                        }
                    }

                    Label {
                        text: qsTr("FTMS Bike:")
                        Layout.fillWidth: true
                    }
                    RowLayout {
                        spacing: 10
                        ComboBox {
                            id: ftmsBikeTextField
                            model: rootItem.bluetoothDevices
                            displayText: settings.ftms_bike
                            Layout.fillHeight: false
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onActivated: {
                                console.log("combomodel activated" + ftmsBikeTextField.currentIndex)
                                displayText = ftmsBikeTextField.currentValue
                             }

                        }
                        Button {
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.ftms_bike = ftmsBikeTextField.displayText; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("If you have a generic FTMS bike and the tiles doesn't appear on the main QZ screen, select here the bluetooth name of your bike.")
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
                
                    Label {
                        text: qsTr("Expand the bars to the right to display the options under this setting. Select your specific model (if it is listed) and leave all other settings on default. If you encounter problems or have a question about the QZ settings for your equipment, open a support ticket on GitHub or ask the QZ community on the QZ Facebook Group.")
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

                    NewPageElement {
                        title: qsTr("Wahoo Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: "gears.qml"
                    }

                    AccordionElement {
                        id: schwinnBikeAccordion
                        title: qsTr("Schwinn Bike Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            spacing: 0
                            IndicatorOnlySwitch {
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
                            IndicatorOnlySwitch {
                                id: schwinnBikeResistanceV2Delegate
                                text: qsTr("Res. Alternative Calc. v2")
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
                            IndicatorOnlySwitch {
                                text: qsTr("Res. Alternative Calc. v3")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.schwinn_bike_resistance_v3
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: settings.schwinn_bike_resistance_v3 = checked
                            }
                            RowLayout {
                                spacing: 10
                                Label {
                                    id: labelSchwinnResistancSmoothing
                                    text: qsTr("Resistance Smoothing:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: scwhinnResistanceSmoothTextField
                                    text: settings.schwinn_resistance_smooth
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    //inputMethodHints: Qt.ImhFormattedNumbersOnly
                                    onAccepted: settings.schwinn_resistance_smooth = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    id: okschwinnResistanceSmoothButton
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.schwinn_resistance_smooth = scwhinnResistanceSmoothTextField.text; toast.show("Setting saved!"); }
                                }
                            }
                            Label {
                                text: qsTr("Since this bike doesn't send resistance over bluetooth, QZ is calculating it using cadence and wattage. The result could be a little 'jumpy' and so, with this setting, you can filter the resistance tile value. The unit is a pure resistance level, so putting 5 means that you will see a resistance changes only when the resistance is changing by 5 levels.")
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
                                onClicked: { settings.horizon_gr7_cadence_multiplier = horizonGr7CadenceMultiplierTextField.text; toast.show("Setting saved!"); }
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
                                    onClicked: { settings.echelon_watttable = echelonWattTableTextField.displayText; toast.show("Setting saved!"); }
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
                                    onClicked: { settings.echelon_resistance_gain = echelonResistanceGainTextField.text; toast.show("Setting saved!"); }
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
                                    onClicked: { settings.echelon_resistance_offset = echelonResistanceOffsetTextField.text; toast.show("Setting saved!"); }
                                }
                            }
                            IndicatorOnlySwitch {
                                text: qsTr("Change gears using knob (Experimental)")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.gears_from_bike
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: settings.gears_from_bike = checked
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
                        IndicatorOnlySwitch {
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
                        IndicatorOnlySwitch {
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
                            IndicatorOnlySwitch {
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
                            IndicatorOnlySwitch {
                                text: qsTr("Use 0.5 resistance lvls")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.renpho_bike_double_resistance
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: settings.renpho_bike_double_resistance = checked
                            }
                        }
                    }

                    AccordionElement {
                        id: hammerBikeAccordion
                        title: qsTr("Hammer Racer Bike Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: IndicatorOnlySwitch {
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
                            onClicked: { settings.hammer_racer_s = checked; window.settings_restart_to_apply = true; }
                        }
                    }

                    AccordionElement {
                        title: qsTr("Saris/Cycleops Hammer trainer Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: IndicatorOnlySwitch {
                            text: qsTr("Enable support")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.saris_trainer
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: { settings.saris_trainer = checked; window.settings_restart_to_apply = true; }
                        }
                    }
                    
                    AccordionElement {
                        id: cardioFitBikeAccordion
                        title: qsTr("CardioFIT Bike Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: IndicatorOnlySwitch {
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
                            onClicked: { settings.sp_ht_9600ie = checked; window.settings_restart_to_apply = true; }
                        }
                    }
                    AccordionElement {
                        id: yesoulBikeAccordion
                        title: qsTr("Yesoul Bike Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: IndicatorOnlySwitch {
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
                        accordionContent: IndicatorOnlySwitch {
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
                            onClicked: { settings.snode_bike = checked; window.settings_restart_to_apply = true; }
                        }
                    }
                    AccordionElement {
                        id: fitplusBikeAccordion
                        title: qsTr("Fitplus Bike Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent:
												ColumnLayout {
                            spacing: 0
                        IndicatorOnlySwitch {
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
                            onClicked: { settings.fitplus_bike = checked; window.settings_restart_to_apply = true; }
                        }
                        IndicatorOnlySwitch {
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
                            onClicked: { settings.virtufit_etappe = checked; window.settings_restart_to_apply = true; }
                        }
                        IndicatorOnlySwitch {
                            id: sportstechSx600BikeDelegate
                            text: qsTr("Sportstech SX600 bike")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.sportstech_sx600
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: { settings.sportstech_sx600 = checked; window.settings_restart_to_apply = true; }
                        }
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
                                    onClicked: { settings.flywheel_filter = flywheelBikeFilterTextField.text; toast.show("Setting saved!"); }
                                }
                            }
                            IndicatorOnlySwitch {
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
                                onClicked: { settings.flywheel_life_fitness_ic8 = checked; window.settings_restart_to_apply = true; }
                            }
                        }
                    }
                    AccordionElement {
                        id: domyosBikeAccordion
                        title: qsTr("Domyos Bike Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            spacing: 0
                            RowLayout {
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
                                    onClicked: { settings.domyos_bike_cadence_filter = domyosBikeCadenceFilterTextField.text; toast.show("Setting saved!"); }
                                }
                            }
                            IndicatorOnlySwitch {
                                text: qsTr("Ignore FTMS")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.domyosbike_notfmts
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: settings.domyosbike_notfmts = checked
                            }
                            IndicatorOnlySwitch {
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
                            IndicatorOnlySwitch {
                                text: qsTr("Bike 500 wattage profile")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.domyos_bike_500_profile_v1
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.domyos_bike_500_profile_v1 = checked; settings.domyos_bike_500_profile_v2 = false; }
                            }
                            IndicatorOnlySwitch {
                                text: qsTr("Bike 500 wattage profile v2")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.domyos_bike_500_profile_v2
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.domyos_bike_500_profile_v2 = checked; settings.domyos_bike_500_profile_v1 = false; }
                            }
                        }
                    }                
                    AccordionElement {
                        title: qsTr("Tacx Neo Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            IndicatorOnlySwitch {
                                text: qsTr("Peloton Configuration")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.tacx_neo2_peloton
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: settings.tacx_neo2_peloton = checked
                            }

                            IndicatorOnlySwitch {
                                text: qsTr("Disable Negative Inclination due to gear")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.tacxneo2_disable_negative_inclination
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: settings.tacxneo2_disable_negative_inclination = checked
                            }
                            Label {
                                text: qsTr("Enabling this QZ will ignore changing gears if the value is too low for this trainer. Default: disabled.")
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
                    AccordionElement {
                        id: proformBikeAccordion
                        title: qsTr("Proform/Norditrack Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            RowLayout {
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
                                    onClicked: { settings.proform_wheel_ratio = proformBikeWheelRatioTextField.text; toast.show("Setting saved!"); }
                                }
                            }

                            Label {
                                text: qsTr("Specific Model:")
                                Layout.fillWidth: true
                            }

                            ComboBox {
                                Layout.fillWidth: true
                                id: bikeModelComboBox
                                property bool initialized: false

                                model: [
                                    "Other",
                                    "Tour de France CLC",
                                    "Proform Studio Bike",
                                    "Proform Studio Bike NTEX71021",
                                    "Freemotion Coachbike B22.7",
                                    "Proform TDF 1.0",
                                    "TDF 1.0 PFEVEX71316.1",
                                    "Proform TDF 10",
                                    "NordicTrack GX 2.7",
                                    "NordicTrack GX 4.5",
                                    "Cycle Trainer 300 CI",
                                    "Cycle Trainer 400",
                                    "Proform 225 CSX",
                                    "Proform 325 CSX / Healthrider H30X",
                                    "Proform SB",
                                    "Nordictrack GX 4.4 Pro",
                                    "TDF 1.0 PFEVEX71316.0",
                                    "Proform XBike",
                                    "Proform 225 CSX PFEX32925 INT.0"
                                ]

                                // Initialize when the accordion content becomes visible
                                Connections {
                                    target: parent.parent  // Connect to the AccordionElement
                                    function onContentBecameVisible() {
                                        if (!bikeModelComboBox.initialized) {
                                            bikeModelComboBox.initializeModel();
                                        }
                                    }
                                }

                                function initializeModel() {
                                    if (initialized) return;

                                    console.log("Initializing bike model ComboBox");

                                    var selectedModel = settings.proform_tour_de_france_clc ? 1 :
                                                    settings.proform_studio ? 2 :
                                                    settings.proform_studio_NTEX71021 ? 3 :
                                                    settings.freemotion_coachbike_b22_7 ? 4 :
                                                    settings.proform_tdf_10 ? 5 :
                                                    settings.proform_bike_PFEVEX71316_1 ? 6 :
                                                    settings.proform_tdf_10_0 ? 7 :
                                                    settings.nordictrack_gx_2_7 ? 8 :
                                                    settings.nordictrack_GX4_5_bike ? 9 :
                                                    settings.proform_cycle_trainer_300_ci ? 10 :
                                                    settings.proform_cycle_trainer_400 ? 11 :
                                                    settings.proform_bike_225_csx ? 12 :
                                                    settings.proform_bike_325_csx ? 13 :
                                                    settings.proform_bike_sb ? 14 :
                                                    settings.nordictrack_gx_44_pro ? 15 :
                                                    settings.proform_bike_PFEVEX71316_0 ? 16 :
                                                    settings.proform_xbike ? 17 :
                                                    settings.proform_225_csx_PFEX32925_INT_0 ? 18 : 0;

                                    console.log("bikeModelComboBox selected model: " + selectedModel);
                                    if (selectedModel >= 0) {
                                        currentIndex = selectedModel;
                                    }
                                    initialized = true;
                                }

                                onCurrentIndexChanged: {
                                    if (!initialized) return;

                                    console.log("bikeModelComboBox onCurrentIndexChanged " + currentIndex);

                                    // Reset all settings first
                                    settings.proform_tour_de_france_clc = false;
                                    settings.proform_studio = false;
                                    settings.proform_studio_NTEX71021 = false;
                                    settings.freemotion_coachbike_b22_7 = false;
                                    settings.proform_tdf_10 = false;
                                    settings.proform_bike_PFEVEX71316_1 = false;
                                    settings.proform_tdf_10_0 = false;
                                    settings.nordictrack_gx_2_7 = false;
                                    settings.nordictrack_GX4_5_bike = false;
                                    settings.proform_cycle_trainer_300_ci = false;
                                    settings.proform_cycle_trainer_400 = false;
                                    settings.proform_bike_225_csx = false;
                                    settings.proform_bike_325_csx = false;
                                    settings.proform_bike_sb = false;
                                    settings.nordictrack_gx_44_pro = false;
                                    settings.proform_bike_PFEVEX71316_0 = false;
                                    settings.proform_xbike = false;
                                    settings.proform_225_csx_PFEX32925_INT_0 = false;

                                    // Set corresponding setting for selected model
                                    switch (currentIndex) {
                                        case 1: settings.proform_tour_de_france_clc = true; break;
                                        case 2: settings.proform_studio = true; break;
                                        case 3: settings.proform_studio_NTEX71021 = true; break;
                                        case 4: settings.freemotion_coachbike_b22_7 = true; break;
                                        case 5: settings.proform_tdf_10 = true; break;
                                        case 6: settings.proform_bike_PFEVEX71316_1 = true; break;
                                        case 7: settings.proform_tdf_10_0 = true; break;
                                        case 8: settings.nordictrack_gx_2_7 = true; break;
                                        case 9: settings.nordictrack_GX4_5_bike = true; break;
                                        case 10: settings.proform_cycle_trainer_300_ci = true; break;
                                        case 11: settings.proform_cycle_trainer_400 = true; break;
                                        case 12: settings.proform_bike_225_csx = true; break;
                                        case 13: settings.proform_bike_325_csx = true; break;
                                        case 14: settings.proform_bike_sb = true; break;
                                        case 15: settings.nordictrack_gx_44_pro = true; break;
                                        case 16: settings.proform_bike_PFEVEX71316_0 = true; break;
                                        case 17: settings.proform_xbike = true; break;
                                        case 18: settings.proform_225_csx_PFEX32925_INT_0 = true; break;
                                    }

                                    window.settings_restart_to_apply = true;
                                }
                            }

                            IndicatorOnlySwitch {
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
                                    text: qsTr("TDF1 IP:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: proformTDF1IPTextField
                                    text: settings.proformtdf1ip
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    //inputMethodHints: Qt.ImhFormattedNumbersOnly
                                    onAccepted: settings.proformtdf1ip = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.proformtdf1ip = proformTDF1IPTextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                                }
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
                                    onClicked: { settings.proformtdf4ip = proformTDF4IPTextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
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
                                    onClicked: { settings.tdf_10_ip = proformTDFCompanionIPTextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                                }
                            }
                            IndicatorOnlySwitch {
                                id: proformBikeAdbRemotedelegate
                                text: qsTr("ADB Remote")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.nordictrack_ifit_adb_remote
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.nordictrack_ifit_adb_remote = checked; window.settings_restart_to_apply = true; }
                            }
                            IndicatorOnlySwitch {
                                text: qsTr("Use Resistance instead of Inc.")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.nordictrackadbbike_resistance
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.nordictrackadbbike_resistance = checked; }
                            }
                        }
                    }

                    AccordionElement {
                        id: computrainerBikeAccordion
                        title: qsTr("Computrainer Bike Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        RowLayout {
                            spacing: 10
                            Label {
                                id: labelcomputrainerSerialPort
                                text: qsTr("Serial Port:")
                                Layout.fillWidth: true
                            }
                            TextField {
                                id: computrainerSerialPortTextField
                                text: settings.computrainer_serialport
                                horizontalAlignment: Text.AlignRight
                                Layout.fillHeight: false
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                //inputMethodHints: Qt.ImhFormattedNumbersOnly
                                onAccepted: settings.computrainer_serialport = text
                                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                            }
                            Button {
                                id: okcomputrainerSerialPortButton
                                text: "OK"
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                onClicked: { settings.computrainer_serialport = computrainerSerialPortTextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
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
                            IndicatorOnlySwitch {
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
                                onClicked: { settings.m3i_bike_qt_search = checked; window.settings_restart_to_apply = true; }
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
                                    onClicked: { settings.m3i_bike_id = m3iBikeIdTextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
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
                                    onClicked: { settings.m3i_bike_speed_buffsize = m3iBikeSpeedBuffsizeTextField.text; toast.show("Setting saved!"); }
                                }
                            }

                            IndicatorOnlySwitch {
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

                    AccordionElement {
                        id: soleBikeAccordion
                        title: qsTr("Sole Bike Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            spacing: 0
                            IndicatorOnlySwitch {
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
                        }
                    }

                    AccordionElement {
                        id: technogymBikeAccordion
                        title: qsTr("Technogym Bike Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            spacing: 0
                            IndicatorOnlySwitch {
                                id: technogymGroupCycleDelegate
                                text: qsTr("Group Cycle")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.technogym_group_cycle
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { 
                                    settings.technogym_group_cycle = checked; 
                                    if (checked) {
                                        settings.android_antbike = true;
                                    }
                                    window.settings_restart_to_apply = true; 
                                }
                            }
                            
                            RowLayout {
                                spacing: 10
                                Label {
                                    text: qsTr("ANT+ Bike Device Number (0=Auto):")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: antBikeDeviceNumberTextField
                                    text: settings.ant_bike_device_number
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    onAccepted: settings.ant_bike_device_number = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    id: okAntBikeDeviceNumberButton
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.ant_bike_device_number = antBikeDeviceNumberTextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                                }
                            }
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

                    Label {
                        id: antSpeedLabel
                        text: qsTr("Set 100mm as wheel circumference in settings of ant+ speed sensor")
                        font.bold: true
                        font.italic: true
                        font.pixelSize: Qt.application.font.pixelSize - 2
                        textFormat: Text.PlainText
                        wrapMode: Text.WordWrap
                        verticalAlignment: Text.AlignVCenter
                        color: Material.color(Material.Red)
                        Layout.fillWidth: true
                    }

                    IndicatorOnlySwitch {
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
                        onClicked: { settings.ant_cadence = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("Turn this on if you need to use ANT+ along with Bluetooth. Power is also sent.")
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
                            text: qsTr("ANT+ Speed Offset")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: antspeedOffsetTextField
                            text: settings.ant_speed_offset
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.ant_speed_offset = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.ant_speed_offset = antspeedOffsetTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("You can increase/decrease your speed sent over ANT+. The number you enter as an Offset adds that amount to your speed.")
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
                            text: qsTr("ANT+ Speed Gain:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: antspeedGainTextField
                            text: settings.ant_speed_gain
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            //inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.ant_speed_gain = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.ant_speed_gain = antspeedGainTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("You can increase/decrease your speed output sent over ANT+. For example, to use a rower to cycle in Zwift, you could double your speed output to better match your cycling speed. The number you enter is a multiplier applied to your actual speed.")
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

                    IndicatorOnlySwitch {
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
                        onClicked: { settings.ant_heart = checked; window.settings_restart_to_apply = true; }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            text: qsTr("ANT+ Heart Device Number (0=Auto):")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: antHeartDeviceNumberTextField
                            text: settings.ant_heart_device_number
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.ant_heart_device_number = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okAntHeartDeviceNumberButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.ant_heart_device_number = antHeartDeviceNumberTextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("This setting enables receiving the heart rate from an external HRM over ANT+ instead of from QZ.")
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

                    IndicatorOnlySwitch {
                        text: qsTr("Ant+ Bike")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.android_antbike
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.android_antbike = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("Use this to connect to your bike using ANT+ instead bluetooth. Default: Disabled")
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
/*
            IndicatorOnlySwitch {
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

            NewPageElement {
                id: tileOptionsAccordion
                title: qsTr("Tiles Options")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                accordionContent: "settings-tiles.qml"
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
                    IndicatorOnlySwitch {
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
                        onClicked: { settings.top_bar_enabled = checked; window.settings_restart_to_apply = true; }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelFloatingWindowType
                            text: qsTr("Floating Window Type:")
                            Layout.fillWidth: true
                        }
                        ComboBox {
                            id: floatingWindowTypeComboBox
                            model: ["Classic", "Horizontal"]
                            currentIndex: settings.floatingwindow_type
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onActivated: {
                                console.log("floatingwindow_type activated" + floatingWindowTypeComboBox.currentIndex)
                            }
                        }
                        Button {
                            id: okFloatingWindowTypeButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.floatingwindow_type = floatingWindowTypeComboBox.currentIndex; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Choose the floating window layout type. Classic uses the standard floating.htm file, while Horizontal uses the hfloating.htm file for horizontal layout.")
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

                    Label {
                        text: qsTr("Allows continuous display of the Start/Pause and Stop buttons across the top of the screen during your workouts. Default is on.")
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
                            id: labelFloatingWidth
                            text: qsTr("Floating Window Width:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: floatingWidthField
                            text: settings.floating_width
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onAccepted: settings.floating_width = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okFloatingWidthButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.floating_width = floatingWidthField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Android Only: width of the floating window.")
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
                            id: labelFloatingHeight
                            text: qsTr("Floating Window Height:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: floatingHeightField
                            text: settings.floating_height
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onAccepted: settings.floating_height = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okFloatingHeightButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.floating_height = floatingHeightField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Android Only: height of the floating window.")
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
                            id: labelFloatingTransparency
                            text: qsTr("Floating Window % Transparency:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: floatingTransparencyField
                            text: settings.floating_transparency
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onAccepted: settings.floating_transparency = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okFloatingTransparencyButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.floating_transparency = floatingTransparencyField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Android Only: transparency percentage of the floating window.")
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

                    IndicatorOnlySwitch {
                        id: floatingStartupDelegate
                        text: qsTr("Floating Window Startup")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.floating_startup
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.floating_startup = checked
                    }

                    Label {
                        text: qsTr("Android Only: if enabled the floating window will start as soon as the fitness devices is connected.")
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

                    Button {
                        text: "Open Floating on a Browser"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: openFloatingWindowBrowser();
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            id: labelChartDisplayMode
                            text: qsTr("Chart Display Mode:")
                            Layout.fillWidth: true
                        }
                        ComboBox {
                            id: chartDisplayModeComboBox
                            model: ["Both Charts", "Heart Rate Only", "Power Only"]
                            currentIndex: settings.chart_display_mode
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onActivated: {
                                console.log("chart_display_mode activated" + chartDisplayModeComboBox.currentIndex)
                            }
                        }
                        Button {
                            id: okChartDisplayModeButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.chart_display_mode = chartDisplayModeComboBox.currentIndex; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Choose which charts to display in the footer: both heart rate and power charts, only heart rate chart, or only power chart.")
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

                    AccordionElement {
                        id: themesOptionsAccordion
                        title: qsTr("UI Themes")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            spacing: 0
                            IndicatorOnlySwitch {
                                id: tilesIconsDelegate
                                text: qsTr("Tiles Icons")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.theme_tile_icon_enabled
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: settings.theme_tile_icon_enabled = checked
                            }

                            RowLayout {
                                spacing: 10
                                Label {
                                    text: qsTr("Background Color:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                      id: backgroundColorTextField
                                      text: settings.theme_background_color
                                      Layout.fillHeight: false
                                      Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                      onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                      onPressed: {
                                          if(OS_VERSION !== "Android") backgroundColorDialog.visible = true
                                      }
                                }
                                Button {
                                    id: okBackgroundColor
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.theme_background_color = backgroundColorTextField.text; toast.show("Setting saved!"); }
                                }
                                ColorDialog {
                                    id: backgroundColorDialog
                                    title: "Please choose a color"
                                    onAccepted: {
                                        backgroundColorTextField.text = this.color
                                        visible = false;
                                    }
                                    onRejected: visible = false;
                                }
                            }

                            RowLayout {
                                spacing: 10
                                Label {
                                    id: labelBackgroundColor
                                    text: qsTr("Tiles Background Color:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                      id: tilebackgroundColorTextField
                                      text: settings.theme_tile_background_color
                                      Layout.fillHeight: false
                                      Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                      onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                      onPressed: {
                                          if(OS_VERSION !== "Android") tilebackgroundColorDialog.visible = true
                                      }
                                }
                                Button {
                                    id: oktileBackgroundColor
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.theme_tile_background_color = tilebackgroundColorTextField.text; toast.show("Setting saved!"); }
                                }
                                ColorDialog {
                                    id: tilebackgroundColorDialog
                                    title: "Please choose a color"
                                    onAccepted: {
                                        tilebackgroundColorTextField.text = this.color
                                        visible = false;
                                    }
                                    onRejected: visible = false;
                                }
                            }

                            IndicatorOnlySwitch {
                                text: qsTr("Tiles Shadow")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.theme_tile_shadow_enabled
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: settings.theme_tile_shadow_enabled = checked
                            }

                            RowLayout {
                                spacing: 10
                                Label {
                                    text: qsTr("Tiles Shadow Color:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                      id: tileShadowColorTextField
                                      text: settings.theme_tile_shadow_color
                                      Layout.fillHeight: false
                                      Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                      onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                      onPressed: {
                                          if(OS_VERSION !== "Android") tileShadowColorDialog.visible = true
                                      }
                                }
                                Button {
                                    id: oktileShadowColor
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.theme_tile_shadow_color = tileShadowColorTextField.text; toast.show("Setting saved!"); }
                                }
                                ColorDialog {
                                    id: tileShadowColorDialog
                                    title: "Please choose a color"
                                    onAccepted: {
                                        tileShadowColorTextField.text = this.color
                                        visible = false;
                                    }
                                    onRejected: visible = false;
                                }
                            }

                            RowLayout {
                                spacing: 10
                                Label {
                                    text: qsTr("Statusbar Background Color:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                      id: statusbarbackgroundColorTextField
                                      text: settings.theme_status_bar_background_color
                                      Layout.fillHeight: false
                                      Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                      onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                      onPressed: {
                                          if(OS_VERSION !== "Android") statusbarbackgroundColorDialog.visible = true
                                      }
                                }
                                Button {
                                    id: okStatusbarBackgroundColor
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.theme_status_bar_background_color = statusbarbackgroundColorTextField.text; toast.show("Setting saved!"); }
                                }
                                ColorDialog {
                                    id: statusbarbackgroundColorDialog
                                    title: "Please choose a color"
                                    onAccepted: {
                                        statusbarbackgroundColorTextField.text = this.color
                                        visible = false;
                                    }
                                    onRejected: visible = false;
                                }
                            }

                            RowLayout {
                                spacing: 10
                                Label {
                                    text: qsTr("2nd line tile text size:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: secondLineTextSizeField
                                    text: settings.theme_tile_secondline_textsize
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onAccepted: settings.theme_tile_secondline_textsize = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.theme_tile_secondline_textsize = secondLineTextSizeField.text; window.settings_restart_to_apply = true;  toast.show("Setting saved!"); }
                                }
                            }
                        }
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
/*
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
                            onClicked: { settings.peloton_username = pelotonUsernameTextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                        }
                    }


                    Label {
                        text: qsTr("Enter the email address you use to login to Peloton (NOT your leaderboard name). Ensure there are no spaces before or after your email. Click OK.")
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
                            onClicked: { settings.peloton_password = pelotonPasswordTextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Enter the password you use to login to Peloton. Click OK. If you have entered the correct login credentials and the QZ is able to access your account, you will see a when you reopen QZ. This is a secure login, not accessible by anyone but you.")
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
*/

                    ItemDelegate {
                        Image {
                            anchors.left: parent.left;
                            anchors.verticalCenter: parent.verticalCenter
                            source: "icons/icons/Button_Connect_Rect_DarkMode.png"
                            fillMode: Image.PreserveAspectFit
                            visible: true
                            width: parent.width
                        }
                        Layout.fillWidth: true
                        onClicked: {
                            stackView.push("WebPelotonAuth.qml")
                            peloton_connect_clicked()
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
                            onClicked: { settings.peloton_difficulty = pelotonDifficultyTextField.displayText; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Typically, Peloton coaches call out a range for target incline, resistance and/or speed. Use this setting to choose the difficulty of the target QZ communicates. Difficulty level can be set to lower, upper or average. Click OK.")
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
                            text: qsTr("Treadmill Level:")
                            Layout.fillWidth: true
                        }
                        ComboBox {
                            id: pelotonTreadmillLevelTextField
                            model: [ "1", "2", "3", "4", "5", "6", "7", "8", "9", "10" ]
                            displayText: settings.peloton_treadmill_level
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onActivated: {
                                console.log("combomodel activated" + pelotonTreadmillLevelTextField.currentIndex)
                                displayText = pelotonTreadmillLevelTextField.currentValue
                             }

                        }
                        Button {
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.peloton_treadmill_level = parseInt(pelotonTreadmillLevelTextField.displayText); toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Difficulty level for peloton treadmill classes. 1 is easy 10 is hard.")
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
                            text: qsTr("Treadmill Walk Level:")
                            Layout.fillWidth: true
                        }
                        ComboBox {
                            id: pelotonTreadmillWalkLevelTextField
                            model: [ "1", "2", "3", "4", "5", "6", "7", "8", "9", "10" ]
                            displayText: settings.peloton_treadmill_walk_level
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onActivated: {
                                console.log("combomodel activated" + pelotonTreadmillWalkLevelTextField.currentIndex)
                                displayText = pelotonTreadmillWalkLevelTextField.currentValue
                             }

                        }
                        Button {
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.peloton_treadmill_walk_level = parseInt(pelotonTreadmillWalkLevelTextField.displayText); toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Difficulty level for peloton treadmill walking classes. 1 is easy 10 is hard.")
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
                            text: qsTr("Rower Level:")
                            Layout.fillWidth: true
                        }
                        ComboBox {
                            id: pelotonRowerLevelTextField
                            model: [ "1", "2", "3", "4", "5", "6", "7", "8", "9", "10" ]
                            displayText: settings.peloton_rower_level
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onActivated: {
                                console.log("combomodel activated" + pelotonRowerLevelTextField.currentIndex)
                                displayText = pelotonRowerLevelTextField.currentValue
                             }

                        }
                        Button {
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.peloton_rower_level = parseInt(pelotonRowerLevelTextField.displayText); toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Difficulty level for peloton rower classes. 1 is easy 10 is hard.")
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
                            onClicked: { settings.pzp_username = pzpUsernameTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("As of 4/1/2022, this feature is broken due to a Power Zone Pack (PZP) website change. Leave (or change back to) the default of “username” (without quotation marks, all lowercase and all one word) until further notice.")
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
                            onClicked: { settings.pzp_password = pzpPasswordTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("As of 4/1/2022, this feature is broken due to a Power Zone Pack (PZP) website change. Leave this setting blank until further notice.")
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
                            onClicked: { settings.peloton_gain = pelotonGainTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Conversion gain is a multiplier. Use this setting to align the Peloton resistance calculated by QZ with the relative effort required by your bike. In most cases the default values will be correct.")
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
                            onClicked: { settings.peloton_offset = pelotonOffsetTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Increases the resistance that QZ displays in the Peloton Resistance tile. If QZ’s calculated conversion from your bike’s resistance scale to Peloton’s seems too low, the number you enter here will be added to the calculated resistance without increasing your effort or actual resistance. (Example: If QZ displays Peloton Resistance of 30 and you enter 5, QZ will display 35.)")
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

                    IndicatorOnlySwitch {
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
                        onClicked: { settings.bike_cadence_sensor = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("Turn this on to send cadence to Peloton over Bluetooth. Default is off.")
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

                    IndicatorOnlySwitch {
                        text: qsTr("Auto Start (with intro)")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.peloton_auto_start_with_intro
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.peloton_auto_start_with_intro = checked; if(settings.peloton_auto_start_with_intro === true) { settings.peloton_auto_start_without_intro = false; } }
                    }

                    Label {
                        text: qsTr("Turn this on to start a workout automatically when you start a workout on Peloton (waiting the intro). Default is off.")
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

                    IndicatorOnlySwitch {
                        text: qsTr("Auto Start (without intro)")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.peloton_auto_start_without_intro
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.peloton_auto_start_without_intro = checked; if(settings.peloton_auto_start_without_intro === true) { settings.peloton_auto_start_with_intro = false; } }
                    }

                    Label {
                        text: qsTr("Turn this on to start a workout automatically when you start a workout on Peloton (skipping the intro). Default is off.")
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
                            onClicked: { settings.peloton_heartrate_metric = pelotonHeartRateMetricTextField.displayText; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("By default, QZ communicates heart rate to Peloton. Use this setting to change the metric that appears on the Peloton screen.")
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
                            onClicked: { settings.peloton_date = pelotonDateOnStravaTextField.displayText; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Allows you to choose whether you would like the Peloton class air date to display before or after the class title on Strava.")
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
                            text: qsTr("Date Format:")
                            Layout.fillWidth: true
                        }
                        ComboBox {
                            id: pelotonDateFormatTextField
                            model: [ "MM/dd/yy", "yy/MM/dd" ]
                            displayText: settings.peloton_date_format
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onActivated: {
                                console.log("combomodel activated" + pelotonDateFormatTextField.currentIndex)
                                displayText = pelotonDateFormatTextField.currentValue
                            }

                        }
                        Button {
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.peloton_date_format = pelotonDateFormatTextField.displayText; toast.show("Setting saved!"); }
                        }
                    }

                    IndicatorOnlySwitch {
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

                    Label {
                        text: qsTr("Turn this on if you want QZ to capture a link to the Peloton class and display it in Strava.")
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

                    IndicatorOnlySwitch {
                        text: qsTr("Spinups Autoresistance")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.peloton_spinups_autoresistance
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.peloton_spinups_autoresistance = checked
                    }

                    Label {
                        text: qsTr("By default, QZ treats Spin-UPS in Power Zone rides as an increasing ramp to warm you up. You can disable this, to leave the resistance up to you.")
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

                    IndicatorOnlySwitch {
                        id: pelotonWorkoutOCRDelegate
                        text: qsTr("Peloton Auto Sync (Experimental)")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.peloton_workout_ocr
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.peloton_workout_ocr = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("Only for Android where QZ is running on the same Peloton device. This setting enables the AI (Artificial Intelligence) on QZ that will read the peloton workout screen and will adjust the peloton offset in order to stay in sync in realtime with your Peloton workout. A popup about screen recording will appear in order to notify this.")
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

                    IndicatorOnlySwitch {
                        text: qsTr("Peloton Auto Sync Companion (Exp.)")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.peloton_companion_workout_ocr
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.peloton_companion_workout_ocr = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("This setting enables the AI (Artificial Intelligence) on the QZ Companion AI app that will read the peloton workout screen and will adjust the peloton offset in order to stay in sync in realtime with your Peloton workout.")
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
                    IndicatorOnlySwitch {
                        id: pelotonBikeOCRDelegate
                        text: qsTr("Peloton Bike/Bike+ (Experimental)")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.peloton_bike_ocr
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.peloton_bike_ocr = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("Only if you are on a real Peloton Bike/Bike+! This will allow QZ to collect metrics from your Bike/Bike+ and send it to Zwift. Peloton Free ride must running.")
                        font.bold: true
                        font.italic: true
                        font.pixelSize: Qt.application.font.pixelSize - 2
                        textFormat: Text.PlainText
                        wrapMode: Text.WordWrap
                        verticalAlignment: Text.AlignVCenter
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        color: Material.color(Material.Lime)
                    }*/
                }
            }

            AccordionElement {
                title: qsTr("Zwift Options") + "\uD83E\uDD47"
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                accordionContent: ColumnLayout {
                    spacing: 0

                    RowLayout {
                        spacing: 10
                        Label {
                            text: qsTr("Username:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: zwiftUsernameTextField
                            text: settings.zwift_username
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onAccepted: settings.zwift_username = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okZwiftUsernameButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.zwift_username = zwiftUsernameTextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                        }
                    }


                    Label {
                        text: qsTr("Enter the email address you use to login to Zwift. Ensure there are no spaces before or after your email. Click OK.")
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
                            id: labelZwiftPassword
                            text: qsTr("Password:") + ((rootItem.zwiftLogin===-1)?"":(rootItem.zwiftLogin===1?"\u2705":"\u274c"))
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: zwiftPasswordTextField
                            text: settings.zwift_password
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhHiddenText
                            echoMode: TextInput.PasswordEchoOnEdit
                            onAccepted: settings.zwift_password = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okZwiftPasswordButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.zwift_password = zwiftPasswordTextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        //text: qsTr("Enter the password you use to login to Zwift. Click OK. If you have entered the correct login credentials and the QZ is able to access your account, you will see a when you reopen QZ. This is a secure login, not accessible by anyone but you.")
                        text: qsTr("Enter the password you use to login to Zwift. Click OK.")
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

                    MessageDialog {
                        id: zwiftPlaySettingsDialog
                        text: "Zwift Play & Click Settings"
                        informativeText: "Would you like to disable Zwift Play and Zwift Click settings? Having them enabled together with 'Get gears from Zwift' may cause conflicts."
                        buttons: (MessageDialog.Yes | MessageDialog.No)
                        onYesClicked: {
                            settings.zwift_play = false;
                            settings.zwift_click = false;
                            settings.zwift_play_emulator = true;
                            window.settings_restart_to_apply = true;
                        }
                        visible: false
                    }

                    IndicatorOnlySwitch {
                        text: qsTr("Get Gears from Zwift")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.zwift_play_emulator
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: {
                             if (checked && !settings.zwift_play_emulator) {  // Only show dialog when enabling
                                 if (settings.zwift_play || settings.zwift_click) {
                                     zwiftPlaySettingsDialog.visible = true;
                                 }
                                 settings.watt_bike_emulator = false;
                             }
                             window.settings_restart_to_apply = true;
                             settings.zwift_play_emulator = checked;
                         }
                    }

                    Label {
                        text: qsTr("This setting bring virtual gearing from zwift to all the bikes directly from the Zwift interface. You have to configure zwift: Wahoo virtual device from QZ as for power and cadence, and your QZ device as resistance. Default: disabled.")
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
                    IndicatorOnlySwitch {
                        text: qsTr("Show Gears to Zwift Only")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.watt_bike_emulator
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.watt_bike_emulator = checked; if(checked) { settings.zwift_play_emulator = false; } window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("This setting shows the actual gear from qz to Zwift. Negative values are not displayed on zwift and it could have also limitation to higher gain value. Default: disabled.")
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
*/

                    IndicatorOnlySwitch {
                        text: qsTr("Align Gear Value on Both Zwift and QZ")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.zwift_gear_ui_aligned
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.zwift_gear_ui_aligned = checked; }
                    }

                    Label {
                        text: qsTr("By default QZ is showing the actual gears from the bike. Enabling this, QZ will show the same gears that you see on Zwift. This doesn't affect the real gear value one the bike. Default: disabled.")
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
                            text: qsTr("Poll Time:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: zwiftPollTimeTextField
                            text: settings.zwift_api_poll
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onAccepted: settings.zwift_api_poll = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.zwift_api_poll = zwiftPollTimeTextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                        }
                    }


                    Label {
                        text: qsTr("Define the number of delay seconds between each inclination change from Zwift. This value can't be less than 5. Default: 5")
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

                    IndicatorOnlySwitch {
                        text: qsTr("Zwift Treadmill Auto Inclination")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.zwift_api_autoinclination
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.zwift_api_autoinclination = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("Only for Android and iOS: QZ will read the inclination in real time from the Zwift app and will adjust the inclination on your treadmill. It doesn't work on workout")
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

                    IndicatorOnlySwitch {
                        text: qsTr("Zwift Treadmill Auto Inclination")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.zwift_ocr
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.zwift_ocr = checked; settings.zwift_workout_ocr = false; settings.zwift_ocr_climb_portal = false; settings.android_notification = true; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("Only for PC where QZ is running on the same Zwift device. This setting enables the AI (Artificial Intelligence) on QZ that will read the Zwift inclination from the Zwift app and will adjust the inclination on your treadmill. A popup about screen recording will appear in order to notify this.")
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

                    IndicatorOnlySwitch {
                        text: qsTr("Zwift Treadmill Climb Portal")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.zwift_ocr_climb_portal
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.zwift_ocr_climb_portal = checked; settings.zwift_workout_ocr = false; settings.zwift_ocr = false; settings.android_notification = true; window.settings_restart_to_apply = true; }
                    }

                    IndicatorOnlySwitch {
                        text: qsTr("Zwift Treadmill Auto Workout")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.zwift_workout_ocr
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.zwift_workout_ocr = checked; settings.zwift_ocr = false; settings.zwift_ocr_climb_portal = false; settings.android_notification = true; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("Only for PC where QZ is running on the same Zwift device. This setting enables the AI (Artificial Intelligence) on QZ that will read the Zwift inclination and speed from the Zwift app during a workout and will adjust the inclination and the speed on your treadmill. A popup about screen recording will appear in order to notify this.")
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

            AccordionElement {
                title: qsTr("Garmin Options") + "\uD83E\uDD47"
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                accordionContent: ColumnLayout {
                    spacing: 0

                    IndicatorOnlySwitch {
                        text: qsTr("Garmin Bluetooth Sensor")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.garmin_bluetooth_compatibility
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.garmin_bluetooth_compatibility = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("If you want to send metrics to your Garmin device from your Mac, enable this. Otherwise leave it disabled.")
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

                    IndicatorOnlySwitch {
                        text: qsTr("Enable Companion App")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.garmin_companion
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.garmin_companion = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("You have to install the QZ Companion App on your Garmin Watch/Computer first.")
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

                    IndicatorOnlySwitch {
                        text: qsTr("Ant+ Bike Over Garmin Watch")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.antbike
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.antbike = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("Use your garmin watch to get the ANT+ metrics from a bike")
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

                    IndicatorOnlySwitch {
                        id: trainprogramStopAtEndDelegate
                        text: qsTr("Stop Treadmill at the End")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.trainprogram_stop_at_end
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.trainprogram_stop_at_end = checked
                    }

                    Label {
                        text: qsTr("Treadmill only: enabling this if you want that QZ will stop the tape at the end of the current train program.")
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

                    IndicatorOnlySwitch {
                        text: qsTr("Treadmill Auto-adjust speed by power")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.treadmill_follow_wattage
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.treadmill_follow_wattage = checked
                    }

                    Label {
                        text: qsTr("Treadmill only: Automatically adjusts speed to maintain consistent power output. Speed adjustments occur on incline changes and adapt to manual speed modifications.")
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
                            onClicked: { settings.treadmill_pid_heart_zone = treadmillPidHRTextField.displayText; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("QZ controls your treadmill or bike to keep you within a chosen Heart Rate Zone. Turn on, set a target heart rate (HR) zone in which to train and click OK. For example, enter 2 to train in HR zone 2 and the treadmill will auto adjust the speed (or resistance on a bike) to maintain your heart rate in zone 2. QZ gradually increases or decreases your speed (or bike resistance) in small increments every 40 seconds to reach and maintain your target HR zone. During a workout, you can display and use the ‘+’ and ‘-’ button on the PID HR Zone tile to change the target HR zone.")
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
                            text: qsTr("PID on HR min:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: treadmillPidHRminTextField
                            text: settings.treadmill_pid_heart_min
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            //inputMethodHints: Qt.ImhDigitsOnly
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.treadmill_pid_heart_min = treadmillPidHRminTextField.text ; toast.show("Setting saved!"); }
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            text: qsTr("PID on HR max:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: treadmillPidHRmaxTextField
                            text: settings.treadmill_pid_heart_max
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            //inputMethodHints: Qt.ImhDigitsOnly
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.treadmill_pid_heart_max = treadmillPidHRmaxTextField.text ; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Alternatively to 'PID on Heart Zone' setting you can use this couple of settings in order to specify a HR range.")
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

                    IndicatorOnlySwitch {
                        text: qsTr("PID 'Pushy'")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.trainprogram_pid_pushy
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.trainprogram_pid_pushy = checked
                    }

                    Label {
                        text: qsTr("Enabling this the PID is trying to motivate yourself to always increase a little the effort trying anyway to keep you in the zone. Default: Enabled.")
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

                    IndicatorOnlySwitch {
                        text: qsTr("PID Ignore Inclination")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.trainprogram_pid_ignore_inclination
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.trainprogram_pid_ignore_inclination = checked
                    }

                    Label {
                        text: qsTr("Enabling this the PID will ignore the inclination changes. Default: Disabled.")
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
                            onClicked: { settings.pacef_1mile = (((parseInt(trainProgramPace1mileTextField.text.split(":")[0]) * 3600) + (parseInt(trainProgramPace1mileTextField.text.split(":")[1]) * 60) + parseInt(trainProgramPace1mileTextField.text.split(":")[2]))) / 1.60934; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Enter your 1 mile time goal, click OK. This setting will be used when you’re following a training program with the speed control. These settings should also match the Zwift app settings. More info: https://github.com/cagnulein/qdomyos-zwift/issues/609.")
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
                            onClicked: { settings.pacef_5km = (((parseInt(trainProgramPace5kmTextField.text.split(":")[0]) * 3600) + (parseInt(trainProgramPace5kmTextField.text.split(":")[1]) * 60) + parseInt(trainProgramPace5kmTextField.text.split(":")[2]))) / 5; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("See 1 Mile Pace above; same except 5 km instead of 1 mile.")
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
                            onClicked: { settings.pacef_10km = (((parseInt(trainProgramPace10kmTextField.text.split(":")[0]) * 3600) + (parseInt(trainProgramPace10kmTextField.text.split(":")[1]) * 60) + parseInt(trainProgramPace10kmTextField.text.split(":")[2]))) / 10; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("See 1 Mile Pace above; same except 10 km instead of 1 mile.")
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
                            onClicked: { settings.pacef_halfmarathon = (((parseInt(trainProgramPaceHalfMarathonTextField.text.split(":")[0]) * 3600) + (parseInt(trainProgramPaceHalfMarathonTextField.text.split(":")[1]) * 60) + parseInt(trainProgramPaceHalfMarathonTextField.text.split(":")[2]))) / 21; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("See 1 Mile Pace above; same except half marathon distance instead of 1 mile.")
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
                            onClicked: { settings.pacef_marathon = (((parseInt(trainProgramPaceMarathonTextField.text.split(":")[0]) * 3600) + (parseInt(trainProgramPaceMarathonTextField.text.split(":")[1]) * 60) + parseInt(trainProgramPaceMarathonTextField.text.split(":")[2]))) / 42; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("See 1 Mile Pace above; same except marathon distance instead of 1 mile.")
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
                            onClicked: { settings.pace_default = treadmillPaceDefaultTextField.displayText; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Select the default Pace to be used when the ZWO file does not indicate a precise pace.")
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
                            id: labelPidHeartZoneErgModeWattStep
                            text: qsTr("ERG Mode Watt Step:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: pidHeartZoneErgModeWattStepTextField
                            text: settings.pid_heart_zone_erg_mode_watt_step.toString()
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            inputMethodHints: Qt.ImhDigitsOnly
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okPidHeartZoneErgModeWattStep
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.pid_heart_zone_erg_mode_watt_step = parseInt(pidHeartZoneErgModeWattStepTextField.text); toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Set the wattage step increment for ERG mode heart rate zone training. Default: 5 watts.")
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
                                    onClicked: { settings.trainprogram_total = trainProgramRandomDurationTextField.text; toast.show("Setting saved!"); }
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
                                    onClicked: { settings.trainprogram_period_seconds = trainProgramRandomPeriodTextField.text; toast.show("Setting saved!"); }
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
                                    onClicked: { settings.trainprogram_speed_min = trainProgramRandomSpeedMinTextField.text; toast.show("Setting saved!"); }
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
                                    onClicked: { settings.trainprogram_speed_max = trainProgramRandomSpeedMaxTextField.text; toast.show("Setting saved!"); }
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
                                    onClicked: { settings.trainprogram_incline_min = trainProgramRandomInclineMinTextField.text; toast.show("Setting saved!"); }
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
                                    onClicked: { settings.trainprogram_incline_max = trainProgramRandomInclineMaxTextField.text; toast.show("Setting saved!"); }
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
                                    onClicked: { settings.trainprogram_resistance_min = trainProgramRandomResistanceMinTextField.text; toast.show("Setting saved!"); }
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
                                    onClicked: { settings.trainprogram_resistance_max = trainProgramRandomResistanceMaxTextField.text; toast.show("Setting saved!"); }
                                }
                            }
                        }
                    }

                    Label {
                        text: qsTr("Turn on and enter your choices for workout time (in minutes and seconds) and the maximum and minimum speed, incline (treadmill), and resistance (bike) and QZ will randomly change your speed and resistance or incline accordingly for the period of time you have selected.")
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

            AccordionElement {
                id:treadmillAccordion
                title: qsTr("Treadmill Options")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                accordionContent: ColumnLayout {
                    spacing: 0
                    IndicatorOnlySwitch {
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
                        onClicked: { settings.virtual_device_force_bike = checked; window.settings_restart_to_apply = true; }
                    }
                    Label {
                        text: qsTr("Turn on to convert your treadmill output to bike output when riding on Zwift. QZ sends your treadmill metrics to Zwift over Bluetooth so that you can participate as a bike rider. Default is off.")
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

                    IndicatorOnlySwitch {
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

                    Label {
                        text: qsTr("Turn this on to have QZ control the speed of your treadmill during, for example, Peloton classes based on the coach’s speed callouts. Your speed will be in the low, upper or average range based on your Peloton Options > Difficulty setting. Default is off.")
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

                    IndicatorOnlySwitch {
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

                    Label {
                        text: qsTr("Turn this on to have QZ go into Pause mode upon opening when using a treadmill. This is for treadmills only. Default is off.")
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

                    IndicatorOnlySwitch {
                        id: treadmillDifficultyGainOffsetDelegate
                        text: qsTr("Difficulty offset based")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.treadmill_difficulty_gain_or_offset
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.treadmill_difficulty_gain_or_offset = checked
                    }

                    Label {
                        text: qsTr("Target Speed and Target Incline tile offer a way to increase/decrease the current difficulty with the plus/minus buttons. By default, with this setting disabled, the speed and the inclination change with a 3% gain for every pressure. Switching this ON, QZ will add a 0.1 speed offset or a 0.5 incline offset instead.")
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
                            //inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.treadmill_step_speed = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okTreadmillSpeedStepButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.treadmill_step_speed = (settings.miles_unit?treadmillSpeedStepTextField.text * 1.60934:treadmillSpeedStepTextField.text); toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("(Speed Tile) This controls the amount of the increase or decrease in the speed (in kph/mph) when you press the plus or minus button in the Speed Tile. Default is 0.5 kph.")
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
                            text: qsTr("Min. Inclination:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: treadmillInclinationMinTextField
                            text: settings.treadmill_incline_min
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            //inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.treadmill_incline_min = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.treadmill_incline_min = treadmillInclinationMinTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("This overrides the minimum inclination value of your treadmill (in order to reduce the inclination movement). Default is -100")
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
                            text: qsTr("Max. Inclination:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: treadmillInclinationMaxTextField
                            text: settings.treadmill_incline_max
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            //inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.treadmill_incline_max = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.treadmill_incline_max = treadmillInclinationMaxTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("This overrides the maximum inclination value of your treadmill (in order to reduce the inclination movement). Default is -100")
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

                    NewPageElement {
                        title: qsTr("Inclination Overrides")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Grey)
                        color: Material.backgroundColor
                        accordionContent: "settings-treadmill-inclination-override.qml"
                    }

                    Label {
                        text: qsTr("Overrides the default inclination values sent from the treadmill")
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

                    IndicatorOnlySwitch {
                        text: qsTr("Simulate Inclination with Speed")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.treadmill_simulate_inclination_with_speed
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.treadmill_simulate_inclination_with_speed = checked
                    }

                    Label {
                        text: qsTr("For treadmills without inclination: turning this on and QZ will transform inclination requests into speed changes.")
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

                    Label {
                        text: qsTr("FTMS Treadmill:")
                        Layout.fillWidth: true
                    }
                    RowLayout {
                        spacing: 10
                        ComboBox {
                            id: ftmsTreadmillTextField
                            model: rootItem.bluetoothDevices
                            displayText: settings.ftms_treadmill
                            Layout.fillHeight: false
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onActivated: {
                                console.log("combomodel activated" + ftmsTreadmillTextField.currentIndex)
                                displayText = ftmsTreadmillTextField.currentValue
                            }

                        }
                        Button {
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.ftms_treadmill = ftmsTreadmillTextField.displayText; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("If you have a generic FTMS bike and the tiles doesn't appear on the main QZ screen, select here the bluetooth name of your bike.")
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

                    Label {
                        text: qsTr("Expand the bars to the right to display the options under this setting. Select your specific model (if it is listed) and leave all other settings on default. If you encounter problems or have a question about settings for your specific equipment with QZ, click here to open a support ticket on GitHub or ask the QZ community on the QZ Facebook Group.")
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

                    AccordionElement {
                        id: proformTreadmillAccordion
                        title: qsTr("Proform/Nordictrack Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            spacing: 0    
                            Label {
                                text: qsTr("Specific Model:")
                                Layout.fillWidth: true
                            }

                            ComboBox {
                                Layout.fillWidth: true
                                id: treadmillModelComboBox
                                property bool initialized: false

                                model: [
                                    "Other",
                                    "Nordictrack S25",
                                    "Nordictrack S25i",
                                    "Nordictrack Incline Trainer x7i",
                                    "NordicTrack X22i",
                                    "Nordictrack 10",
                                    "Nordictrack T8.5s",
                                    "Proform 2000 (not pro)",
                                    "Proform 505 CST",
                                    "Proform 8.5",
                                    "Proform Sport 8.5",
                                    "Proform Pro 1000",
                                    "Nordictrack L6.0S",
                                    "Nordictrack T6.5S v81",
                                    "Nordictrack T6.5S v83",
                                    "Nordictrack T7.0",
                                    "Nordictrack S20",
                                    "Nordictrack S30",
                                    "Proform 1800i",
                                    "Proform/NordicTrack z1300i",
                                    "Proform SE",
                                    "Proform Cadence LT",
                                    "Proform 8.0",
                                    "Proform 9.0",
                                    "Proform 705 CST",
                                    "Nordictrack x14i",
                                    "Proform Carbon TL",
                                    "Proform Proshox 2",
                                    "Nordictrack S20i",
                                    "Proform 595i",
                                    "Proform 8.7",
                                    "Proform 705 CST V78.239",
                                    "Proform Carbon T7",
                                    "Nordictrack EXP 5i",
                                    "Proform Carbon TL PFTL59720",
                                    "Proform Sport 7.0",
                                    "Proform 575i",
                                    "Proform Performance 400i",
                                    "Proform C700",
                                    "Proform C960i",
                                    "Nordictrack T Series 5",
                                    "Proform Carbon TL PFTL59722c",
                                    "Proform 1500 Pro",
                                    "Proform 505 CST v.80.44",
                                    "Proform Trainer 8.0",
                                    "Proform 705 CST v.80.44",
                                    "Nordictrack 1750",
                                    "Proform Performance 300i",
                                    "Nordictrack T6.5S v81 Miles",
                                    "Nordictrack Elite 800",
                                    "Nordictrack Ultra LE",
                                    "Proform Carbon TLS",
                                    "Proform 995i",
                                ]

                                // Initialize when the accordion content becomes visible
                                Connections {
                                    target: parent.parent  // Connect to the AccordionElement
                                    function onContentBecameVisible() {
                                        if (!treadmillModelComboBox.initialized) {
                                            treadmillModelComboBox.initializeModel();
                                        }
                                    }
                                }

                                function initializeModel() {
                                    if (initialized) return;

                                    console.log("Initializing treadmill model ComboBox");

                                    var selectedModel = settings.norditrack_s25_treadmill ? 1 :
                                                    settings.norditrack_s25i_treadmill ? 2 :
                                                    settings.nordictrack_incline_trainer_x7i ? 3 :
                                                    settings.nordictrack_x22i ? 4 :
                                                    settings.nordictrack_10_treadmill ? 5 :
                                                    settings.nordictrack_treadmill_t8_5s ? 6 :
                                                    settings.proform_2000_treadmill ? 7 :
                                                    settings.proform_treadmill_505_cst ? 8 :
                                                    settings.proform_8_5_treadmill ? 9 :
                                                    settings.proform_treadmill_sport_8_5 ? 10 :
                                                    settings.proform_pro_1000_treadmill ? 11 :
                                                    settings.proform_treadmill_l6_0s ? 12 :
                                                    settings.nordictrack_t65s_treadmill ? 13 :
                                                    settings.nordictrack_t65s_83_treadmill ? 14 :
                                                    settings.nordictrack_t70_treadmill ? 15 :
                                                    settings.nordictrack_s20_treadmill ? 16 :
                                                    settings.nordictrack_s30_treadmill ? 17 :
                                                    settings.proform_treadmill_1800i ? 18 :
                                                    settings.proform_treadmill_z1300i ? 19 :
                                                    settings.proform_treadmill_se ? 20 :
                                                    settings.proform_treadmill_cadence_lt ? 21 :
                                                    settings.proform_treadmill_8_0 ? 22 :
                                                    settings.proform_treadmill_9_0 ? 23 :
                                                    settings.proform_treadmill_705_cst ? 24 :
                                                    settings.nordictrack_treadmill_x14i ? 25 :
                                                    settings.proform_carbon_tl ? 26 :
                                                    settings.proform_proshox2 ? 27 :
                                                    settings.nordictrack_s20i_treadmill ? 28 :
                                                    settings.proform_595i_proshox2 ? 29 :
                                                    settings.proform_treadmill_8_7 ? 30 :
                                                    settings.proform_treadmill_705_cst_V78_239 ? 31 :
                                                    settings.proform_treadmill_carbon_t7 ? 32 :
                                                    settings.nordictrack_treadmill_exp_5i ? 33 :
                                                    settings.proform_carbon_tl_PFTL59720 ? 34 :
                                                    settings.proform_treadmill_sport_70 ? 35 :
                                                    settings.proform_treadmill_575i ? 36 :
                                                    settings.proform_performance_400i ? 37 :
                                                    settings.proform_treadmill_c700 ? 38 :
                                                    settings.proform_treadmill_c960i ? 39 :
                                                    settings.nordictrack_tseries5_treadmill ? 40 :
                                                    settings.proform_carbon_tl_PFTL59722c ? 41 :
                                                    settings.proform_treadmill_1500_pro ? 42 :
                                                    settings.proform_505_cst_80_44 ? 43 :
                                                    settings.proform_trainer_8_0 ? 44 :
                                                    settings.proform_treadmill_705_cst_V80_44 ? 45 :
                                                    settings.nordictrack_treadmill_1750_adb ? 46 : 
                                                    settings.proform_performance_300i ? 47 :
                                                    settings.nordictrack_t65s_treadmill_81_miles ? 48 : 
                                                    settings.nordictrack_elite_800 ? 49 :
                                                    settings.nordictrack_treadmill_ultra_le ? 50 :
                                                    settings.proform_treadmill_carbon_tls ? 51 :
                                                    settings.proform_treadmill_995i ? 52 : 0;

                                    console.log("treadmillModelComboBox selected model: " + selectedModel);
                                    if (selectedModel >= 0) {
                                        currentIndex = selectedModel;
                                    }
                                    initialized = true;
                                }

                                onCurrentIndexChanged: {
                                    if (!initialized) return;

                                    console.log("treadmillModelComboBox onCurrentIndexChanged " + currentIndex);

                                    // Reset all settings
                                    settings.norditrack_s25_treadmill = false;
                                    settings.norditrack_s25i_treadmill = false;
                                    settings.nordictrack_incline_trainer_x7i = false;
                                    settings.nordictrack_x22i = false;
                                    settings.nordictrack_10_treadmill = false;
                                    settings.nordictrack_treadmill_t8_5s = false;
                                    settings.proform_2000_treadmill = false;
                                    settings.proform_treadmill_505_cst = false;
                                    settings.proform_8_5_treadmill = false;
                                    settings.proform_treadmill_sport_8_5 = false;
                                    settings.proform_pro_1000_treadmill = false;
                                    settings.proform_treadmill_l6_0s = false;
                                    settings.nordictrack_t65s_treadmill = false;
                                    settings.nordictrack_t65s_83_treadmill = false;
                                    settings.nordictrack_t70_treadmill = false;
                                    settings.nordictrack_s20_treadmill = false;
                                    settings.nordictrack_s30_treadmill = false;
                                    settings.proform_treadmill_1800i = false;
                                    settings.proform_treadmill_z1300i = false;
                                    settings.proform_treadmill_se = false;
                                    settings.proform_treadmill_cadence_lt = false;
                                    settings.proform_treadmill_8_0 = false;
                                    settings.proform_treadmill_9_0 = false;
                                    settings.proform_treadmill_705_cst = false;
                                    settings.nordictrack_treadmill_x14i = false;
                                    settings.proform_carbon_tl = false;
                                    settings.proform_proshox2 = false;
                                    settings.nordictrack_s20i_treadmill = false;
                                    settings.proform_595i_proshox2 = false;
                                    settings.proform_treadmill_8_7 = false;
                                    settings.proform_treadmill_705_cst_V78_239 = false;
                                    settings.proform_treadmill_carbon_t7 = false;
                                    settings.nordictrack_treadmill_exp_5i = false;
                                    settings.proform_carbon_tl_PFTL59720 = false;
                                    settings.proform_treadmill_sport_70 = false;
                                    settings.proform_treadmill_575i = false;
                                    settings.proform_performance_300i = false;
                                    settings.proform_performance_400i = false;
                                    settings.proform_treadmill_c700 = false;
                                    settings.proform_treadmill_c960i = false;
                                    settings.nordictrack_tseries5_treadmill = false;
                                    settings.proform_carbon_tl_PFTL59722c = false;
                                    settings.proform_treadmill_1500_pro = false;
                                    settings.proform_505_cst_80_44 = false;
                                    settings.proform_trainer_8_0 = false;
                                    settings.proform_treadmill_705_cst_V80_44 = false;
                                    settings.nordictrack_treadmill_1750_adb = false;
                                    settings.nordictrack_t65s_treadmill_81_miles = false;
                                    settings.nordictrack_elite_800 = false;
                                    settings.nordictrack_treadmill_ultra_le = false;
                                    settings.proform_treadmill_carbon_tls = false;
                                    settings.proform_treadmill_995i = false;

                                    // Set new setting based on selection
                                    switch (currentIndex) {
                                        case 1: settings.norditrack_s25_treadmill = true; break;
                                        case 2: settings.norditrack_s25i_treadmill = true; break;
                                        case 3: settings.nordictrack_incline_trainer_x7i = true; break;
                                        case 4: settings.nordictrack_x22i = true; break;
                                        case 5: settings.nordictrack_10_treadmill = true; break;
                                        case 6: settings.nordictrack_treadmill_t8_5s = true; break;
                                        case 7: settings.proform_2000_treadmill = true; break;
                                        case 8: settings.proform_treadmill_505_cst = true; break;
                                        case 9: settings.proform_8_5_treadmill = true; break;
                                        case 10: settings.proform_treadmill_sport_8_5 = true; break;
                                        case 11: settings.proform_pro_1000_treadmill = true; break;
                                        case 12: settings.proform_treadmill_l6_0s = true; break;
                                        case 13: settings.nordictrack_t65s_treadmill = true; break;
                                        case 14: settings.nordictrack_t65s_83_treadmill = true; break;
                                        case 15: settings.nordictrack_t70_treadmill = true; break;
                                        case 16: settings.nordictrack_s20_treadmill = true; break;
                                        case 17: settings.nordictrack_s30_treadmill = true; break;
                                        case 18: settings.proform_treadmill_1800i = true; break;
                                        case 19: settings.proform_treadmill_z1300i = true; break;
                                        case 20: settings.proform_treadmill_se = true; break;
                                        case 21: settings.proform_treadmill_cadence_lt = true; break;
                                        case 22: settings.proform_treadmill_8_0 = true; break;
                                        case 23: settings.proform_treadmill_9_0 = true; break;
                                        case 24: settings.proform_treadmill_705_cst = true; break;
                                        case 25: settings.nordictrack_treadmill_x14i = true; break;
                                        case 26: settings.proform_carbon_tl = true; break;
                                        case 27: settings.proform_proshox2 = true; break;
                                        case 28: settings.nordictrack_s20i_treadmill = true; break;
                                        case 29: settings.proform_595i_proshox2 = true; break;
                                        case 30: settings.proform_treadmill_8_7 = true; break;
                                        case 31: settings.proform_treadmill_705_cst_V78_239 = true; break;
                                        case 32: settings.proform_treadmill_carbon_t7 = true; break;
                                        case 33: settings.nordictrack_treadmill_exp_5i = true; break;
                                        case 34: settings.proform_carbon_tl_PFTL59720 = true; break;
                                        case 35: settings.proform_treadmill_sport_70 = true; break;
                                        case 36: settings.proform_treadmill_575i = true; break;
                                        case 37: settings.proform_performance_400i = true; break;
                                        case 38: settings.proform_treadmill_c700 = true; break;
                                        case 39: settings.proform_treadmill_c960i = true; break;
                                        case 40: settings.nordictrack_tseries5_treadmill = true; break;
                                        case 41: settings.proform_carbon_tl_PFTL59722c = true; break;
                                        case 42: settings.proform_treadmill_1500_pro = true; break;
                                        case 43: settings.proform_505_cst_80_44 = true; break;
                                        case 44: settings.proform_trainer_8_0 = true; break;
                                        case 45: settings.proform_treadmill_705_cst_V80_44 = true; break;
                                        case 46: settings.nordictrack_treadmill_1750_adb = true; break;
                                        case 47: settings.proform_performance_300i = true; break;
                                        case 48: settings.nordictrack_t65s_treadmill_81_miles = true; break;
                                        case 49: settings.nordictrack_elite_800 = true; break;
                                        case 50: settings.nordictrack_treadmill_ultra_le = true; break;
                                        case 51: settings.proform_treadmill_carbon_tls = true; break;
                                        case 52: settings.proform_treadmill_995i = true; break;
                                    }

                                    window.settings_restart_to_apply = true;
                                }
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
                                    onClicked: { settings.proformtreadmillip = proformtreadmillIPTextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
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
                                    onClicked: { settings.nordictrack_2950_ip = nordictrack2950IPTextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                                }
                            }
                            IndicatorOnlySwitch {
                                id: proformTreadmillAdbRemotedelegate
                                text: qsTr("ADB Remote")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.nordictrack_ifit_adb_remote
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.nordictrack_ifit_adb_remote = checked; window.settings_restart_to_apply = true; }
                            }
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
                            IndicatorOnlySwitch {
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
                                onClicked: { settings.pafers_treadmill = checked; window.settings_restart_to_apply = true; }
                            }
                            IndicatorOnlySwitch {
                                id: bhIboxsterPlusDelegate
                                text: qsTr("BH IBoxster Plus")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.pafers_treadmill_bh_iboxster_plus
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.pafers_treadmill_bh_iboxster_plus = checked; window.settings_restart_to_apply = true; }
                            }
                        }
                    }

                    AccordionElement {
                        title: qsTr("GEM Module Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            spacing: 0
                            IndicatorOnlySwitch {
                                text: qsTr("Inclination")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.gem_module_inclination
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.gem_module_inclination = checked; window.settings_restart_to_apply = true; }
                            }
                        }
                    }

                    AccordionElement {
                        title: qsTr("Echelon Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            spacing: 0
                            IndicatorOnlySwitch {
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
                            IndicatorOnlySwitch {
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
                                onClicked: { settings.kingsmith_encrypt_v2 = checked; settings.kingsmith_encrypt_g1_walking_pad = false; settings.kingsmith_encrypt_v3 = false; settings.kingsmith_encrypt_v4 = false; settings.kingsmith_encrypt_v5 = false; window.settings_restart_to_apply = true; }
                            }

                            IndicatorOnlySwitch {
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
                                onClicked: { settings.kingsmith_encrypt_v3 = checked; settings.kingsmith_encrypt_g1_walking_pad = false; settings.kingsmith_encrypt_v2 = false; settings.kingsmith_encrypt_v4 = false; settings.kingsmith_encrypt_v5 = false; window.settings_restart_to_apply = true; }
                            }

                            IndicatorOnlySwitch {
                                id: kingSmithV4TreadmillDelegate
                                text: qsTr("WalkingPad X21 v3")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.kingsmith_encrypt_v4
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.kingsmith_encrypt_v4 = checked; settings.kingsmith_encrypt_g1_walking_pad = false; settings.kingsmith_encrypt_v3 = false; settings.kingsmith_encrypt_v2 = false; settings.kingsmith_encrypt_v5 = false; window.settings_restart_to_apply = true; }
                            }

                            IndicatorOnlySwitch {
                                text: qsTr("WalkingPad X21 v4")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.kingsmith_encrypt_v5
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.kingsmith_encrypt_v5 = checked; settings.kingsmith_encrypt_g1_walking_pad = false; settings.kingsmith_encrypt_v3 = false; settings.kingsmith_encrypt_v2 = false; settings.kingsmith_encrypt_v4 = false; window.settings_restart_to_apply = true; }
                            }

                            IndicatorOnlySwitch {
                                text: qsTr("WalkingPad G1")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.kingsmith_encrypt_g1_walking_pad
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.kingsmith_encrypt_g1_walking_pad = checked; settings.kingsmith_encrypt_v5 = false; settings.kingsmith_encrypt_v3 = false; settings.kingsmith_encrypt_v2 = false; settings.kingsmith_encrypt_v4 = false; window.settings_restart_to_apply = true; }
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
                            IndicatorOnlySwitch {
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
                                onClicked: { settings.fitfiu_mc_v460 = checked; window.settings_restart_to_apply = true; }
                            }
                            IndicatorOnlySwitch {
                                text: qsTr("Zero ZT-2500")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.zero_zt2500_treadmill
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.zero_zt2500_treadmill = checked; window.settings_restart_to_apply = true; }
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
                            IndicatorOnlySwitch {
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

                            IndicatorOnlySwitch {
                                text: qsTr("T900")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.domyos_treadmill_t900a
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: settings.domyos_treadmill_t900a = checked
                            }                        

                            IndicatorOnlySwitch {
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

                            IndicatorOnlySwitch {
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

                            RowLayout {
                                spacing: 10
                                Label {
                                    text: qsTr("Remap 5km/h button:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: domyosTreadmillButton5KmhTimeTextField
                                    text: settings.domyos_treadmill_button_5kmh
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    onAccepted: settings.domyos_treadmill_button_5kmh = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.domyos_treadmill_button_5kmh = domyosTreadmillButton5KmhTimeTextField.text; toast.show("Setting saved!"); window.settings_restart_to_apply = true;}
                                }
                            }

                            RowLayout {
                                spacing: 10
                                Label {
                                    text: qsTr("Remap 10km/h button:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: domyosTreadmillButton10KmhTimeTextField
                                    text: settings.domyos_treadmill_button_10kmh
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    onAccepted: settings.domyos_treadmill_button_10kmh = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.domyos_treadmill_button_10kmh = domyosTreadmillButton10KmhTimeTextField.text; toast.show("Setting saved!"); window.settings_restart_to_apply = true;}
                                }
                            }

                            RowLayout {
                                spacing: 10
                                Label {
                                    text: qsTr("Remap 16km/h button:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: domyosTreadmillButton16KmhTimeTextField
                                    text: settings.domyos_treadmill_button_16kmh
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    onAccepted: settings.domyos_treadmill_button_16kmh = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.domyos_treadmill_button_16kmh = domyosTreadmillButton16KmhTimeTextField.text; toast.show("Setting saved!"); window.settings_restart_to_apply = true;}
                                }
                            }

                            RowLayout {
                                spacing: 10
                                Label {
                                    text: qsTr("Remap 22km/h button:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: domyosTreadmillButton22KmhTimeTextField
                                    text: settings.domyos_treadmill_button_22kmh
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    onAccepted: settings.domyos_treadmill_button_22kmh = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.domyos_treadmill_button_22kmh = domyosTreadmillButton22KmhTimeTextField.text; toast.show("Setting saved!"); window.settings_restart_to_apply = true;}
                                }
                            }

                            RowLayout {
                                spacing: 10
                                Label {
                                    text: qsTr("Pool time (ms):")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: pollDeviceTimeTextField
                                    text: settings.poll_device_time
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    onAccepted: settings.poll_device_time = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.poll_device_time = pollDeviceTimeTextField.text; toast.show("Setting saved!"); window.settings_restart_to_apply = true;}
                                }
                            }
                            Label {
                                text: qsTr("Default: 200. Change this only if you have random issues with speed or inclination (try to put 300)")
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

                    AccordionElement {
                        id:soleTreadmillAccordion
                        title: qsTr("Sole Treadmill Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            spacing: 0
                            IndicatorOnlySwitch {
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
                                onClicked: { settings.sole_treadmill_inclination = checked; window.settings_restart_to_apply = true; }
                            }
                            IndicatorOnlySwitch {
                                text: qsTr("Fast Inclination (experimental)")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.sole_treadmill_inclination_fast
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: settings.sole_treadmill_inclination_fast = checked
                            }
                            IndicatorOnlySwitch {
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
                            IndicatorOnlySwitch {
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
                            IndicatorOnlySwitch {
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
                                onClicked: { settings.sole_treadmill_f65 = checked; window.settings_restart_to_apply = true; }
                            }
                            IndicatorOnlySwitch {
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
                                onClicked: { settings.sole_treadmill_tt8 = checked; window.settings_restart_to_apply = true; }
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
                            IndicatorOnlySwitch {
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
                                onClicked: { settings.technogym_myrun_treadmill_experimental = checked; window.settings_restart_to_apply = true; }
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
                            IndicatorOnlySwitch {
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
                                onClicked: { settings.fitshow_anyrun = checked; window.settings_restart_to_apply = true; }
                            }
                            IndicatorOnlySwitch {
                                text: qsTr("Atletica Lightspeed")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.atletica_lightspeed_treadmill
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.atletica_lightspeed_treadmill = checked; window.settings_restart_to_apply = true; }
                            }
                            IndicatorOnlySwitch {
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
                            IndicatorOnlySwitch {
                                id: fitshowMilesDelegate
                                text: qsTr("Miles unit from the device")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.fitshow_treadmill_miles
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: settings.fitshow_treadmill_miles = checked
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
                                    onClicked: { settings.fitshow_user_id = fitshowTreadmillUserIdTextField.text; toast.show("Setting saved!"); }
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
                            IndicatorOnlySwitch {
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
                                onClicked: { settings.eslinker_cadenza = checked; window.settings_restart_to_apply = true; }
                            }
                            IndicatorOnlySwitch {
                                id: eslinkerTreadmillYpooDelegate
                                text: qsTr("YPOO Mini Change")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.eslinker_ypoo
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.eslinker_ypoo = checked; window.settings_restart_to_apply = true; }
                            }
                            IndicatorOnlySwitch {
                                text: qsTr("Costaway Folding")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.eslinker_costaway
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.eslinker_costaway = checked; window.settings_restart_to_apply = true; }
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
                            IndicatorOnlySwitch {
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
                                onClicked: { settings.horizon_paragon_x = checked; window.settings_restart_to_apply = true; }
                            }
                            IndicatorOnlySwitch {
                                id: horizonFTMSTreadmillCadenzaDelegate
                                text: qsTr("Force Using FTMS")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.horizon_treadmill_force_ftms
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.horizon_treadmill_force_ftms = checked; window.settings_restart_to_apply = true; }
                            }
                            IndicatorOnlySwitch {
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
                                onClicked: { settings.horizon_treadmill_7_8 = checked; window.settings_restart_to_apply = true; }
                            }

                            IndicatorOnlySwitch {
                                id: horizonTreadmillDisablePauseDelegate
                                text: qsTr("Disable Pause")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.horizon_treadmill_disable_pause
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: settings.horizon_treadmill_disable_pause = checked
                            }

                            IndicatorOnlySwitch {
                                id: horizonTreadmillSuspendStatsPauseDelegate
                                text: qsTr("Supends stats while paused")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.horizon_treadmill_suspend_stats_pause
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: settings.horizon_treadmill_suspend_stats_pause = checked
                            }

                            RowLayout {
                                spacing: 10
                                Label {
                                    id: labelhorizonTreadmillProfile1
                                    text: qsTr("User 1:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: horizonTreadmillProfile1TextField
                                    text: settings.horizon_treadmill_profile_user1
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onAccepted: settings.horizon_treadmill_profile_user1 = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    id: okhorizonTreadmillProfile1Button
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.horizon_treadmill_profile_user1 = horizonTreadmillProfile1TextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                                }
                            }
                            RowLayout {
                                spacing: 10
                                Label {
                                    id: labelhorizonTreadmillProfile2
                                    text: qsTr("User 2:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: horizonTreadmillProfile2TextField
                                    text: settings.horizon_treadmill_profile_user2
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onAccepted: settings.horizon_treadmill_profile_user2 = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    id: okhorizonTreadmillProfile2Button
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.horizon_treadmill_profile_user2 = horizonTreadmillProfile2TextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                                }
                            }
                            RowLayout {
                                spacing: 10
                                Label {
                                    id: labelhorizonTreadmillProfile3
                                    text: qsTr("User 3:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: horizonTreadmillProfile3TextField
                                    text: settings.horizon_treadmill_profile_user3
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onAccepted: settings.horizon_treadmill_profile_user3 = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    id: okhorizonTreadmillProfile3Button
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.horizon_treadmill_profile_user3 = horizonTreadmillProfile3TextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                                }
                            }
                            RowLayout {
                                spacing: 10
                                Label {
                                    id: labelhorizonTreadmillProfile4
                                    text: qsTr("User 4:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: horizonTreadmillProfile4TextField
                                    text: settings.horizon_treadmill_profile_user4
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onAccepted: settings.horizon_treadmill_profile_user4 = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    id: okhorizonTreadmillProfile4Button
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.horizon_treadmill_profile_user4 = horizonTreadmillProfile4TextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                                }
                            }
                            RowLayout {
                                spacing: 10
                                Label {
                                    id: labelhorizonTreadmillProfile5
                                    text: qsTr("User 5:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: horizonTreadmillProfile5TextField
                                    text: settings.horizon_treadmill_profile_user5
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onAccepted: settings.horizon_treadmill_profile_user5 = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    id: okhorizonTreadmillProfile5Button
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.horizon_treadmill_profile_user5 = horizonTreadmillProfile5TextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                                }
                            }
                        }
                    }

                    AccordionElement {
                        title: qsTr("Bodytone Treadmill Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            spacing: 0
                            IndicatorOnlySwitch {
                                text: qsTr("Force Using FTMS")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.horizon_treadmill_force_ftms
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.horizon_treadmill_force_ftms = checked; window.settings_restart_to_apply = true; }
                            }
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
                    IndicatorOnlySwitch {
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
                        onClicked:  { settings.trx_route_key = checked; window.settings_restart_to_apply = true; }
                    }
                    IndicatorOnlySwitch {
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
                        onClicked: { settings.toorx_65s_evo = checked; window.settings_restart_to_apply = true; }
                    }

                    IndicatorOnlySwitch {
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
                        onClicked: { settings.bh_spada_2 = checked; window.settings_restart_to_apply = true; }
                    }

                    IndicatorOnlySwitch {
                        text: qsTr("BH SPADA Wattage")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.bh_spada_2_watt
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.bh_spada_2_watt = checked; window.settings_restart_to_apply = true; }
                    }                    

                    IndicatorOnlySwitch {
                        text: qsTr("Toorx SRX 500")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.toorx_bike_srx_500
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.toorx_bike_srx_500 = checked; window.settings_restart_to_apply = true; }
                    }


                    IndicatorOnlySwitch {
                        text: qsTr("Toorx SRX 3500")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.toorx_srx_3500
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.toorx_srx_3500 = checked; window.settings_restart_to_apply = true; }
                    } 

                    IndicatorOnlySwitch {
                        text: qsTr("Enerfit SPX 9500 / Toorx SRX 500")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.enerfit_SPX_9500
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.enerfit_SPX_9500 = checked; window.settings_restart_to_apply = true; }
                    } 

                    IndicatorOnlySwitch {
                        text: qsTr("HOP-Sport HS-090h")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.hop_sport_hs_090h_bike
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.hop_sport_hs_090h_bike = checked; window.settings_restart_to_apply = true; }
                    }                    

                    IndicatorOnlySwitch {
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
                        onClicked: { settings.jtx_fitness_sprint_treadmill = checked; window.settings_restart_to_apply = true; }
                    }

                    IndicatorOnlySwitch {
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
                        onClicked: { settings.reebok_fr30_treadmill = checked; window.settings_restart_to_apply = true; }
                    }

                    IndicatorOnlySwitch {
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
                        onClicked: { settings.dkn_endurun_treadmill = checked; window.settings_restart_to_apply = true; }
                    }

                    IndicatorOnlySwitch {
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
                        onClicked: { settings.toorx_3_0 = checked; window.settings_restart_to_apply = true; }
                    }

                    IndicatorOnlySwitch {
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
                        onClicked: { settings.toorx_bike = checked; window.settings_restart_to_apply = true; }
                    }                    

                    IndicatorOnlySwitch {
                        id: toorxFTMSTreadmillDelegate
                        text: qsTr("Toorx FTMS Treadmill")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.toorx_ftms_treadmill
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.toorx_ftms_treadmill = checked; window.settings_restart_to_apply = true; }
                    }

                    IndicatorOnlySwitch {
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
                        onClicked: { settings.toorx_ftms = checked; window.settings_restart_to_apply = true; }
                    }

                    IndicatorOnlySwitch {
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
                        onClicked: { settings.jll_IC400_bike = checked; window.settings_restart_to_apply = true; }
                    }
                    IndicatorOnlySwitch {
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
                        onClicked: { settings.fytter_ri08_bike = checked; window.settings_restart_to_apply = true; }
                    }
                    IndicatorOnlySwitch {
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
                        onClicked: { settings.asviva_bike = checked; window.settings_restart_to_apply = true; }
                    }
                    IndicatorOnlySwitch {
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
                        onClicked: { settings.hertz_xr_770 = checked; window.settings_restart_to_apply = true; }
                    }
                    IndicatorOnlySwitch {
                        text: qsTr("iConsole Elliptical")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.iconsole_elliptical
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.iconsole_elliptical = checked; window.settings_restart_to_apply = true; }
                    }
                    IndicatorOnlySwitch {
                        text: qsTr("iConsole Rower")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.iconsole_rower
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.iconsole_rower = checked; window.settings_restart_to_apply = true; }
                    }
                }
            }

            AccordionElement {
                title: qsTr("Rower Options")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                accordionContent: ColumnLayout {
                    spacing: 0
                    AccordionElement {
                        title: qsTr("PM3, PM4 Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            RowLayout {
                                spacing: 10
                                Label {
                                    text: qsTr("Serial Port:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: csaferowerSerialPortTextField
                                    text: settings.csafe_rower
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    //inputMethodHints: Qt.ImhFormattedNumbersOnly
                                    onAccepted: settings.csafe_rower = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.csafe_rower = csaferowerSerialPortTextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                                }
                            }
                        }
                    }

                    Label {
                        text: qsTr("FTMS Rower:")
                        Layout.fillWidth: true
                    }
                    RowLayout {
                        spacing: 10
                        ComboBox {
                            id: ftmsRowerTextField
                            model: rootItem.bluetoothDevices
                            displayText: settings.ftms_rower
                            Layout.fillHeight: false
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onActivated: {
                                console.log("combomodel activated" + ftmsRowerTextField.currentIndex)
                                displayText = ftmsRowerTextField.currentValue
                             }

                        }
                        Button {
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.ftms_rower = ftmsRowerTextField.displayText; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                        }
                    }

                    Button {
                        text: "Refresh Devices List"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: refresh_bluetooth_devices_clicked();
                    }

                    Label {
                        text: qsTr("Allows you to force QZ to connect to your FTMS Rower. If you are in doubt, leave this Disabled and send an email to the QZ support. Default is “Disabled.”")
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

                    AccordionElement {
                        title: qsTr("Proform/Nordictrack Rower Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            IndicatorOnlySwitch {
                                text: qsTr("Proform Sport RL")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.proform_rower_sport_rl
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.proform_rower_sport_rl = checked; window.settings_restart_to_apply = true; }
                            }
                            RowLayout {
                                spacing: 10
                                Label {
                                    text: qsTr("ProForm Rower IP:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: proformRowerIPTextField
                                    text: settings.proform_rower_ip
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onAccepted: settings.proform_rower_ip = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.proform_rower_ip = proformRowerIPTextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                                }
                            }
                        }
                    }
                }
            }

            AccordionElement {
                id: ellipticalAccordion
                title: qsTr("Elliptical Options")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                accordionContent: ColumnLayout {
                    spacing: 0

                    AccordionElement {
                        id: domyosEllipticalAccordion
                        title: qsTr("Domyos Elliptical Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
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
                                onClicked: { settings.domyos_elliptical_speed_ratio = domyosEllipticalSpeedRatioTextField.text; toast.show("Setting saved!"); }
                            }
                        }
                        IndicatorOnlySwitch {
                            id: domyosEllipticalInclinationDelegate
                            text: qsTr("Inclination Supported")
                            spacing: 0
                            bottomPadding: 0
                            topPadding: 0
                            rightPadding: 0
                            leftPadding: 0
                            clip: false
                            checked: settings.domyos_elliptical_inclination
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Layout.fillWidth: true
                            onClicked: settings.domyos_elliptical_inclination = checked
                        }
                    }
                    AccordionElement {
                        title: qsTr("Life Fitness 95xi (CSAFE)")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            RowLayout {
                                spacing: 10
                                Label {
                                    text: qsTr("Serial Port:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: csafeellipticalSerialPortTextField
                                    text: settings.csafe_elliptical_port
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    //inputMethodHints: Qt.ImhFormattedNumbersOnly
                                    onAccepted: settings.csafe_elliptical_port = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.csafe_elliptical_port = csafeellipticalSerialPortTextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                                }
                            }
                        }
                    }

                    Label {
                        text: qsTr("FTMS Elliptical:")
                        Layout.fillWidth: true
                    }
                    RowLayout {
                        spacing: 10
                        ComboBox {
                            id: ftmsEllipticalTextField
                            model: rootItem.bluetoothDevices
                            displayText: settings.ftms_elliptical
                            Layout.fillHeight: false
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onActivated: {
                                console.log("combomodel activated" + ftmsEllipticalTextField.currentIndex)
                                displayText = ftmsEllipticalTextField.currentValue
                             }

                        }
                        Button {
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.ftms_elliptical = ftmsEllipticalTextField.displayText; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                        }
                    }

                    Button {
                        text: "Refresh Devices List"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: refresh_bluetooth_devices_clicked();
                    }

                    Label {
                        text: qsTr("Allows you to force QZ to connect to your FTMS Elliptical. If you are in doubt, leave this Disabled and send an email to the QZ support. Default is Disabled.")
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

                    AccordionElement {
                        id: proformEllipticalAccordion
                        title: qsTr("Proform/Nordictrack Elliptical Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            IndicatorOnlySwitch {
                                id: proformHybridDelegate
                                text: qsTr("Proform Hybrid Trainer XT")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.proform_hybrid_trainer_xt
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.proform_hybrid_trainer_xt = checked; window.settings_restart_to_apply = true; }
                            }
                            IndicatorOnlySwitch {
                                id: proformHybridPFEL03815Delegate
                                text: qsTr("Proform Hybrid Trainer PFEL03815")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.proform_hybrid_trainer_PFEL03815
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.proform_hybrid_trainer_PFEL03815 = checked; window.settings_restart_to_apply = true; }
                            }
                            IndicatorOnlySwitch {
                                text: qsTr("Nordictrack C7.5")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.nordictrack_elliptical_c7_5
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.nordictrack_elliptical_c7_5 = checked; window.settings_restart_to_apply = true; }
                            }
                            RowLayout {
                                spacing: 10
                                Label {
                                    text: qsTr("Companion IP:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: proformEllipticalCompanionIPTextField
                                    text: settings.proform_elliptical_ip
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    //inputMethodHints: Qt.ImhFormattedNumbersOnly
                                    onAccepted: settings.proform_elliptical_ip = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.proform_elliptical_ip = proformEllipticalCompanionIPTextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                                }
                            }
                            IndicatorOnlySwitch {
                                text: qsTr("ADB Remote")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.nordictrack_ifit_adb_remote
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.nordictrack_ifit_adb_remote = checked; window.settings_restart_to_apply = true; }
                            }
                        }
                    }                    

                    AccordionElement {
                        id: soleEllipticalAccordion
                        title: qsTr("Sole Elliptical Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            IndicatorOnlySwitch {
                                id: soleEllipticalInclinationDelegate
                                text: qsTr("Inclination Supported")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.sole_elliptical_inclination
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.sole_elliptical_inclination = checked; window.settings_restart_to_apply = true; }
                            }
                            IndicatorOnlySwitch {
                                id: soleEllipticalE55Delegate
                                text: qsTr("E55 elliptical")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.sole_elliptical_e55
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.sole_elliptical_e55 = checked; window.settings_restart_to_apply = true; }
                            }
                        }
                    }

                    AccordionElement {
                        title: qsTr("iConcept Elliptical Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            IndicatorOnlySwitch {
                                text: qsTr("iConcept elliptical")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.iconcept_elliptical
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.iconcept_elliptical = checked; window.settings_restart_to_apply = true; }
                            }
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
                    Label {
                        id: labelFilterDevice
                        text: qsTr("Manual Device:")
                        Layout.fillWidth: true
                    }
                    RowLayout {
                        spacing: 10
                        ComboBox {
                            id: filterDeviceTextField
                            model: rootItem.bluetoothDevices
                            displayText: settings.filter_device
                            Layout.fillHeight: false
                            Layout.fillWidth: true
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
                            onClicked: { settings.filter_device = filterDeviceTextField.displayText; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                        }
                    }

                    Button {
                        id: refreshFilterDeviceButton
                        text: "Refresh Devices List"
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: refresh_bluetooth_devices_clicked();
                    }

                    Label {
                        text: qsTr("Allows you to force QZ to connect to your equipment (see “Bluetooth Troubleshooting” below). Default is “Disabled.”")
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
                            id: labelwattOffset
                            text: qsTr("Watt Offset:")
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
                            onClicked: { settings.watt_offset = wattOffsetTextField.text; settings.treadmillDataPoints = ""; settings.ergDataPoints = ""; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("You can increase/decrease your watt output for moving your avatar faster/slower in Zwift or other similar apps as a way of calibrating your equipment. The number you enter as an Offset adds that amount to your watts.")
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
                            onClicked: { settings.watt_gain = wattGainTextField.text; settings.treadmillDataPoints = ""; settings.ergDataPoints = ""; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("You can increase/decrease your watt output for moving your avatar faster/slower in Zwift or other similar apps as a way of calibrating your equipment. For example, to use a rower to cycle in Zwift, you could double your watt output to better match your cycling speed by entering 2. The number you enter is a multiplier applied to your actual watts.")
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
                            //inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.speed_offset = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okspeedOffsetButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.speed_offset = speedOffsetTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("You can increase/decrease your speed for moving your avatar faster/slower in Zwift if your equipment outputs speed but not watts. The number you enter as an Offset adds that amount to your speed.")
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
                            onClicked: { settings.speed_gain = speedGainTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("You can increase/decrease your speed output for moving your avatar faster/slower in Zwift or other apps as a way of calibrating your equipment if your equipment outputs speed but not watts. For example, to use a rower to cycle in Zwift, you could double your speed output to better match your cycling speed. The number you enter is a multiplier applied to your actual speed.")
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
                            onClicked: { settings.cadence_offset = cadenceOffsetTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("You can increase/decrease your cadence output. The number you enter as an Offset adds that amount to your cadence.")
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
                            onClicked: { settings.cadence_gain = cadenceGainTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("You can increase/decrease your cadence output as a way of calibrating your equipment if your equipment outputs cadence but not watts. The number you enter is a multiplier applied to your actual cadence.")
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
                            text: qsTr("Strava Upload:")
                            Layout.fillWidth: true
                        }
                        ComboBox {
                            id: stravaUploadMode
                            model: [ "Always", "Request", "Disabled" ]
                            displayText: settings.strava_upload_mode
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onActivated: {
                                console.log("combomodel activated" + stravaUploadMode.currentIndex)
                                displayText = stravaUploadMode.currentValue
                             }

                        }
                        Button {
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.strava_upload_mode = stravaUploadMode.displayText; toast.show("Setting saved!"); }
                        }
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
                            onClicked: { settings.strava_suffix = stravaSuffixTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Default is “QZ.” Please leave this set to default so that other Strava users will see the QZ; a tiny bit of advertising that helps promote the app and support its development. If you choose to remove it, please consider contributing to the developer’s Patreon or Buy Me a Coffee accounts or just subscribe to the Swag bag in the left side bar to allow me to continue developing and supporting the app.")
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

                    IndicatorOnlySwitch {
                        text: qsTr("Strava External Browser Auth")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.strava_auth_external_webbrowser
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.strava_auth_external_webbrowser = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("QZ can open a external browser in order to auth strava to QZ. Default: disabled.")
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

                    IndicatorOnlySwitch {
                        text: qsTr("Use garmin device in the FIT file")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.fit_file_garmin_device_training_effect
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.fit_file_garmin_device_training_effect = checked; window.settings_restart_to_apply = false; }
                    }

                    Label {
                        text: qsTr("With this enabled, QZ will write the FIT file as a Garmin device so Garmin will consider this fit file for the training effect. Default: disabled.")
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

                    Label {
                        text: qsTr("Garmin device for FIT file")
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                    }

                    ComboBox {
                        id: garminDeviceComboBoxDelegate
                        model: [
                            "Edge 130 (2909)",
                            "Edge 200 (1325)", 
                            "Edge 520 (2067)",
                            "Edge 530 (3121)",
                            "Edge 810 (1567)",
                            "Edge 820 (2530)",
                            "Edge 830 (3122)",
                            "Edge 1000 (1836)",
                            "Edge 1030 (2713)",
                            "Edge Explore (3011)",
                            "Fenix (1551)",
                            "Fenix 2 (1967)",
                            "Fenix 3 (2050)",
                            "Fenix 3 HR (2413)",
                            "Fenix 3 Chronos (2432)",
                            "Fenix 5 (2697)",
                            "Fenix 5S (2544)",
                            "Fenix 5X (2604)",
                            "Fenix 5S Plus (2900)",
                            "Fenix 5 Plus (3110)",
                            "Fenix 5X Plus (3111)",
                            "Fenix 6 (3290)",
                            "Fenix 6S (3288)",
                            "Fenix 6X (3291)",
                            "Fenix 7 (3906)",
                            "Fenix 7S (3905)",
                            "Fenix 7X (3907)",
                            "Fenix 8 (4536)",
                            "Venu (3226)",
                            "Venu 2 (3703)",
                            "Venu 2S (3704)",
                            "Venu 3 (4260)",
                            "Venu 3S (4261)",
                            "VenuSq (3600)",
                            "VenuSq Music (3596)",
                            "Vivoactive 3 (2700)",
                            "Vivoactive 4 Small (3224)",
                            "Vivoactive 4 Large (3225)"
                        ]
                        currentIndex: {
                            if (settings.fit_file_garmin_device_training_effect_device === 2909) return 0;  // Edge 130
                            if (settings.fit_file_garmin_device_training_effect_device === 1325) return 1;  // Edge 200
                            if (settings.fit_file_garmin_device_training_effect_device === 2067) return 2;  // Edge 520
                            if (settings.fit_file_garmin_device_training_effect_device === 3121) return 3;  // Edge 530
                            if (settings.fit_file_garmin_device_training_effect_device === 1567) return 4;  // Edge 810
                            if (settings.fit_file_garmin_device_training_effect_device === 2530) return 5;  // Edge 820
                            if (settings.fit_file_garmin_device_training_effect_device === 3122) return 6;  // Edge 830
                            if (settings.fit_file_garmin_device_training_effect_device === 1836) return 7;  // Edge 1000
                            if (settings.fit_file_garmin_device_training_effect_device === 2713) return 8;  // Edge 1030
                            if (settings.fit_file_garmin_device_training_effect_device === 3011) return 9;  // Edge Explore
                            if (settings.fit_file_garmin_device_training_effect_device === 1551) return 10; // Fenix
                            if (settings.fit_file_garmin_device_training_effect_device === 1967) return 11; // Fenix 2
                            if (settings.fit_file_garmin_device_training_effect_device === 2050) return 12; // Fenix 3
                            if (settings.fit_file_garmin_device_training_effect_device === 2413) return 13; // Fenix 3 HR
                            if (settings.fit_file_garmin_device_training_effect_device === 2432) return 14; // Fenix 3 Chronos
                            if (settings.fit_file_garmin_device_training_effect_device === 2697) return 15; // Fenix 5
                            if (settings.fit_file_garmin_device_training_effect_device === 2544) return 16; // Fenix 5S
                            if (settings.fit_file_garmin_device_training_effect_device === 2604) return 17; // Fenix 5X
                            if (settings.fit_file_garmin_device_training_effect_device === 2900) return 18; // Fenix 5S Plus
                            if (settings.fit_file_garmin_device_training_effect_device === 3110) return 19; // Fenix 5 Plus
                            if (settings.fit_file_garmin_device_training_effect_device === 3111) return 20; // Fenix 5X Plus
                            if (settings.fit_file_garmin_device_training_effect_device === 3290) return 21; // Fenix 6
                            if (settings.fit_file_garmin_device_training_effect_device === 3288) return 22; // Fenix 6S
                            if (settings.fit_file_garmin_device_training_effect_device === 3291) return 23; // Fenix 6X
                            if (settings.fit_file_garmin_device_training_effect_device === 3906) return 24; // Fenix 7
                            if (settings.fit_file_garmin_device_training_effect_device === 3905) return 25; // Fenix 7S
                            if (settings.fit_file_garmin_device_training_effect_device === 3907) return 26; // Fenix 7X
                            if (settings.fit_file_garmin_device_training_effect_device === 4536) return 27; // Fenix 8
                            if (settings.fit_file_garmin_device_training_effect_device === 3226) return 28; // Venu
                            if (settings.fit_file_garmin_device_training_effect_device === 3703) return 29; // Venu 2
                            if (settings.fit_file_garmin_device_training_effect_device === 3704) return 30; // Venu 2S
                            if (settings.fit_file_garmin_device_training_effect_device === 4260) return 31; // Venu 3
                            if (settings.fit_file_garmin_device_training_effect_device === 4261) return 32; // Venu 3S
                            if (settings.fit_file_garmin_device_training_effect_device === 3600) return 33; // VenuSq
                            if (settings.fit_file_garmin_device_training_effect_device === 3596) return 34; // VenuSq Music
                            if (settings.fit_file_garmin_device_training_effect_device === 2700) return 35; // Vivoactive 3
                            if (settings.fit_file_garmin_device_training_effect_device === 3224) return 36; // Vivoactive 4 Small
                            if (settings.fit_file_garmin_device_training_effect_device === 3225) return 37; // Vivoactive 4 Large
                            return 6; // Default to Edge 830
                        }
                        onCurrentIndexChanged: {
                            switch(currentIndex) {
                                case 0: settings.fit_file_garmin_device_training_effect_device = 2909; break;  // Edge 130
                                case 1: settings.fit_file_garmin_device_training_effect_device = 1325; break;  // Edge 200
                                case 2: settings.fit_file_garmin_device_training_effect_device = 2067; break;  // Edge 520
                                case 3: settings.fit_file_garmin_device_training_effect_device = 3121; break;  // Edge 530
                                case 4: settings.fit_file_garmin_device_training_effect_device = 1567; break;  // Edge 810
                                case 5: settings.fit_file_garmin_device_training_effect_device = 2530; break;  // Edge 820
                                case 6: settings.fit_file_garmin_device_training_effect_device = 3122; break;  // Edge 830
                                case 7: settings.fit_file_garmin_device_training_effect_device = 1836; break;  // Edge 1000
                                case 8: settings.fit_file_garmin_device_training_effect_device = 2713; break;  // Edge 1030
                                case 9: settings.fit_file_garmin_device_training_effect_device = 3011; break;  // Edge Explore
                                case 10: settings.fit_file_garmin_device_training_effect_device = 1551; break; // Fenix
                                case 11: settings.fit_file_garmin_device_training_effect_device = 1967; break; // Fenix 2
                                case 12: settings.fit_file_garmin_device_training_effect_device = 2050; break; // Fenix 3
                                case 13: settings.fit_file_garmin_device_training_effect_device = 2413; break; // Fenix 3 HR
                                case 14: settings.fit_file_garmin_device_training_effect_device = 2432; break; // Fenix 3 Chronos
                                case 15: settings.fit_file_garmin_device_training_effect_device = 2697; break; // Fenix 5
                                case 16: settings.fit_file_garmin_device_training_effect_device = 2544; break; // Fenix 5S
                                case 17: settings.fit_file_garmin_device_training_effect_device = 2604; break; // Fenix 5X
                                case 18: settings.fit_file_garmin_device_training_effect_device = 2900; break; // Fenix 5S Plus
                                case 19: settings.fit_file_garmin_device_training_effect_device = 3110; break; // Fenix 5 Plus
                                case 20: settings.fit_file_garmin_device_training_effect_device = 3111; break; // Fenix 5X Plus
                                case 21: settings.fit_file_garmin_device_training_effect_device = 3290; break; // Fenix 6
                                case 22: settings.fit_file_garmin_device_training_effect_device = 3288; break; // Fenix 6S
                                case 23: settings.fit_file_garmin_device_training_effect_device = 3291; break; // Fenix 6X
                                case 24: settings.fit_file_garmin_device_training_effect_device = 3906; break; // Fenix 7
                                case 25: settings.fit_file_garmin_device_training_effect_device = 3905; break; // Fenix 7S
                                case 26: settings.fit_file_garmin_device_training_effect_device = 3907; break; // Fenix 7X
                                case 27: settings.fit_file_garmin_device_training_effect_device = 4536; break; // Fenix 8
                                case 28: settings.fit_file_garmin_device_training_effect_device = 3226; break; // Venu
                                case 29: settings.fit_file_garmin_device_training_effect_device = 3703; break; // Venu 2
                                case 30: settings.fit_file_garmin_device_training_effect_device = 3704; break; // Venu 2S
                                case 31: settings.fit_file_garmin_device_training_effect_device = 4260; break; // Venu 3
                                case 32: settings.fit_file_garmin_device_training_effect_device = 4261; break; // Venu 3S
                                case 33: settings.fit_file_garmin_device_training_effect_device = 3600; break; // VenuSq
                                case 34: settings.fit_file_garmin_device_training_effect_device = 3596; break; // VenuSq Music
                                case 35: settings.fit_file_garmin_device_training_effect_device = 2700; break; // Vivoactive 3
                                case 36: settings.fit_file_garmin_device_training_effect_device = 3224; break; // Vivoactive 4 Small
                                case 37: settings.fit_file_garmin_device_training_effect_device = 3225; break; // Vivoactive 4 Large
                            }
                        }
                        Layout.fillWidth: true
                    }

                    IndicatorOnlySwitch {
                        id: stravaVirtualActivityDelegate
                        text: qsTr("Strava Virtual Activity Tag")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.strava_virtual_activity
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.strava_virtual_activity = checked
                    }

                    Label {
                        text: qsTr("Append the Virtual Tag to the Strava Activity")
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

                    IndicatorOnlySwitch {
                        text: qsTr("Strava Treadmill Tag")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.strava_treadmill
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.strava_treadmill = checked
                    }

                    Label {
                        text: qsTr("Append the Treadmill Tag to the Strava Activity when you are using a treadmill. If you want to see the elevation on Strava, you need to disable this.")
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

                    IndicatorOnlySwitch {
                        text: qsTr("Date Prefix on Strava Workout")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.strava_date_prefix
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.strava_date_prefix = checked
                    }

                    Label {
                        text: qsTr("Append the Date to the Strava Activity as a prefix only for non-peloton workout")
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

                    IndicatorOnlySwitch {
                        id: volumeChangeGearsDelegate
                        text: qsTr("Volume buttons change gears")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.volume_change_gears
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.volume_change_gears = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("Allows you to change resistance during auto-follow mode using the volume buttons of the device running QZ, Bluetooth headphones or a Bluetooth remote. Changes made using these external controls will be visible in the Gears tile. This is a VERY USEFUL feature! Default is off.")
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

                    IndicatorOnlySwitch {
                        text: qsTr("Volume buttons debouncing")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.gears_volume_debouncing
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.gears_volume_debouncing = checked; }
                    }

                    Label {
                        text: qsTr("Debounce the volume buttons, so you will only see 1 gear step if there are 2 or more volume near steps.  Default is off.")
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

                    IndicatorOnlySwitch {
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

                    Label {
                        text: qsTr("If the power output/watts your equipment sends to QZ is quite variable, this setting will result in smoother Power Zone graphs. This is also helpful for use with Power Meter Pedals. Default is off.")
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

                    IndicatorOnlySwitch {
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

                    Label {
                        text: qsTr("Enables the calculation of watts, even while in Pause mode. Default is off.")
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

                    IndicatorOnlySwitch {
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

                    Label {
                        text: qsTr("Turn this on if you have a bike with inclination capabilities to fix Zwift’s bug that sends half-negative downhill inclination")
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
                            onClicked: { settings.zwift_inclination_offset = treadmillInclinationOffsetTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("Inclination Offset and Gain are used to adjust the incline set by Zwift instead of, or in addition to, using the QZ Zwift Gain setting. For example, when Zwift changes the incline to 1%, you can have your treadmill change to 2%. The number you enter as an offset adds to the inclination sent from Zwift or any other 3rd party app. Default is 0.")
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
                            onClicked: { settings.zwift_inclination_gain = treadmillInclinationGainTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("The number you enter as a Gain is a multiplier applied to the inclination sent from Zwift or any other 3rd party app. Default is 1.")
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
                            text: qsTr("Minimum Inclination:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: minInclinationTextField
                            text: settings.min_inclination
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            //inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onAccepted: settings.min_inclination = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.min_inclination = minInclinationTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("If you don't want to go below a certain inclination value for bikes and treadmill set the min. value here. Default: -999.")
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
                            id: labelInclinationStep
                            text: qsTr("Inclination Step:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: inclinationStepTextField
                            text: settings.treadmill_step_incline
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            //inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.treadmill_step_incline = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            id: okInclinationStepButton
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.treadmill_step_incline = inclinationStepTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("(Incline Tile) This controls the amount of the increase or decrease in the inclination when you press the plus or minus button in the Incline Tile for both treadmills and bikes. Default is 0.5.")
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

                    IndicatorOnlySwitch {
                        text: qsTr("Send real inclination to virtual bridge")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.real_inclination_to_virtual_treamill_bridge
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.real_inclination_to_virtual_treamill_bridge = checked
                    }

                    Label {
                        text: qsTr("By default QZ sends to the virtual bluetooth/dircon bridge the current inclination of the treadmill. Enabling this, it will send instead the one wihtout considering inclination gain or offset. Default: False.")
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


                    IndicatorOnlySwitch {
                        text: qsTr("Disable Wattage from Machinery")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.watt_ignore_builtin
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.watt_ignore_builtin = checked
                    }

                    Label {
                        text: qsTr("This prevents your fitness device from sending its wattage calculation to QZ and defaults to QZ’s more accurate calculation.")
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

                    IndicatorOnlySwitch {
                        text: qsTr("Use Resistance instead of Inclination")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.force_resistance_instead_inclination
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.force_resistance_instead_inclination = checked
                    }

                    Label {
                        text: qsTr("For the smart trainers, use resistance instead of inclination. This should help if you don't want to have the Wahoo Climb or similar to change inclination when you change gears. Default: disabled")
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
                            text: qsTr("AutoLap on Distance:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: autoLapOnDistanceTextField
                            text: (settings.miles_unit?settings.autolap_distance * 0.621371:settings.autolap_distance).toFixed(1)
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            //inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.autolap_distance = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.autolap_distance = (settings.miles_unit?autoLapOnDistanceTextField.text * 1.60934:autoLapOnDistanceTextField.text); toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("You can trigger auto laps in the FIT file based on distance. Unit: "+ (settings.miles_unit?"Mi":"KM") +" Default: 0 (disabled).")
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
                            text: qsTr("Inclination Delay:")
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: treadmillInclinationDelayTextField
                            text: settings.inclination_delay_seconds
                            horizontalAlignment: Text.AlignRight
                            Layout.fillHeight: false
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            //inputMethodHints: Qt.ImhDigitsOnly
                            onAccepted: settings.inclination_delay_seconds = text
                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                        }
                        Button {
                            text: "OK"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: { settings.inclination_delay_seconds = treadmillInclinationDelayTextField.text; toast.show("Setting saved!"); }
                        }
                    }

                    Label {
                        text: qsTr("This slow down the inclination changes adding a delay between each change. This is not applied to all the model of treadmill/bike. Default is 0.")
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

            AccordionElement {
                id: accesoriesAccordion
                title: qsTr("Accessories")
                indicatRectColor: Material.color(Material.Grey)
                textColor: Material.color(Material.Grey)
                color: Material.backgroundColor
                accordionContent: ColumnLayout {
                    spacing: 0

                    AccordionElement {
                        id: cadenceSensorOptionsAccordion
                        title: qsTr("Cadence Sensor Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            spacing: 0

                            Label {
                                id: cadenceSensorLabel
                                text: qsTr("Don't touch these settings if your bike works properly!")
                                font.bold: true
                                font.italic: true
                                font.pixelSize: Qt.application.font.pixelSize - 2
                                textFormat: Text.PlainText
                                wrapMode: Text.WordWrap
                                verticalAlignment: Text.AlignVCenter
                                color: Material.color(Material.Red)
                                Layout.fillWidth: true
                            }

                            IndicatorOnlySwitch {
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
                                onClicked: { settings.cadence_sensor_as_bike = checked; window.settings_restart_to_apply = true; }
                            }

                            Label {
                                text: qsTr("If your bike doesn’t have Bluetooth, this setting allows you to use a cadence sensor so your bike will work with QZ. Default is off.")
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

                            Label {
                                id: labelCadenceSensorName
                                text: qsTr("Cadence Sensor:")
                                Layout.fillWidth: true
                            }
                            RowLayout {
                                spacing: 10
                                ComboBox {
                                    id: cadenceSensorNameTextField
                                    model: rootItem.bluetoothDevices
                                    displayText: settings.cadence_sensor_name
                                    Layout.fillHeight: false
                                    Layout.fillWidth: true
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
                                    onClicked: { settings.cadence_sensor_name = cadenceSensorNameTextField.displayText; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                                }
                            }

                            Button {
                                id: refreshCadenceSensorNameButton
                                text: "Refresh Devices List"
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                onClicked: refresh_bluetooth_devices_clicked();
                            }

                            Label {
                                text: qsTr("Use this setting to connect QZ to your cadence sensor. Default is Disabled.")
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
                                    onClicked: { settings.cadence_sensor_speed_ratio = cadenceSpeedRatioTextField.text; toast.show("Setting saved!"); }
                                }
                            }

                            Label {
                                text: qsTr("Wheel ratio is the multiplier used by QZ to calculate your speed based on your cadence. For example, if you enter 1 for your wheel ratio and you are riding at a cadence of 30, QZ will display your speed as 30 km/h. The default of 0.33 is correct for most bikes.")
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

                            IndicatorOnlySwitch {
                                text: qsTr("Rogue Echo Bike")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.rogue_echo_bike
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.rogue_echo_bike = checked; window.settings_restart_to_apply = true; }
                            }

                            Label {
                                text: qsTr("Enable special wattage calculation for Rogue Echo Bike: m_watt = 0.000602337 * pow(rpm, 3.11762) + 32.6404. Default is off.")
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

                    AccordionElement {
                        id: powerSensorOptionsAccordion
                        title: qsTr("Power Sensor Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            spacing: 0
                            IndicatorOnlySwitch {
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
                                onClicked: { settings.power_sensor_as_bike = checked; window.settings_restart_to_apply = true; }
                            }

                            Label {
                                text: qsTr("If your bike doesn’t have Bluetooth, this setting allows you to use a power meter pedal sensor so your bike will work with QZ. Default is off.")
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

                            IndicatorOnlySwitch {
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
                                onClicked: { settings.power_sensor_as_treadmill = checked; window.settings_restart_to_apply = true; }
                            }

                            Label {
                                text: qsTr("If your treadmill doesn’t have Bluetooth, this setting allows you to use a Stryde sensor (or similar) so your treadmill will work with QZ. Default is off.")
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

                            IndicatorOnlySwitch {
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

                            Label {
                                text: qsTr("Some power sensors send cadence divided by 2. This setting will fix this behavior.")
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

                            IndicatorOnlySwitch {
                                id: powerSensorRunCadenceHalfStravaDelegate
                                text: qsTr("Half Cadence on Strava")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.powr_sensor_running_cadence_half_on_strava
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: settings.powr_sensor_running_cadence_half_on_strava = checked
                            }

                            Label {
                                text: qsTr("Divide the cadence sent to Strava by 2.")
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

                            IndicatorOnlySwitch {
                                text: qsTr("Use speed from the power sensor")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.stryd_speed_instead_treadmill
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: settings.stryd_speed_instead_treadmill = checked
                            }

                            Label {
                                text: qsTr("If you have a bluetooth treadmill and also a Stryd device connected to QZ and you want to use the speed from the stryd instead of the speed of the treadmill, enable this. Default: disabled.")
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

                            IndicatorOnlySwitch {
                                text: qsTr("Use inclination from the power sensor")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.stryd_inclination_instead_treadmill
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: settings.stryd_inclination_instead_treadmill = checked
                            }

                            Label {
                                text: qsTr("If you have a bluetooth treadmill and also a Runn device connected to QZ and you want to use the inclination from the RUNN instead of the inclination of the treadmill, enable this. Default: disabled.")
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

                            IndicatorOnlySwitch {
                                text: qsTr("Add inclination gain factor to the power")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.stryd_add_inclination_gain
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: settings.stryd_add_inclination_gain = checked
                            }

                            Label {
                                text: qsTr("If you have a bluetooth treadmill and also a Stryd device connected to QZ, by default Stryd can't get the inclination from the treadmill. Enabling this and QZ will add a inclination gain to the power read from the Stryd. Default: disabled.")
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

                            Label {
                                id: labelPowerSensorName
                                text: qsTr("Power Sensor:")
                                Layout.fillWidth: true
                            }
                            RowLayout {
                                spacing: 10
                                ComboBox {
                                    id: powerSensorNameTextField
                                    model: rootItem.bluetoothDevices
                                    displayText: settings.power_sensor_name
                                    Layout.fillHeight: false
                                    Layout.fillWidth: true
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
                                    onClicked: { settings.power_sensor_name = powerSensorNameTextField.displayText; settings.treadmillDataPoints = ""; settings.ergDataPoints = ""; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                                }
                            }

                            Button {
                                id: refreshPowerSensorNameButton
                                text: "Refresh Devices List"
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                onClicked: refresh_bluetooth_devices_clicked();
                            }

                            Label {
                                text: qsTr("Leave on Disabled or select from list of found Bluetooth devices.")
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

                    AccordionElement {
                        id: eliteAccesoriesAccordion
                        title: qsTr("Elite™ Products")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            spacing: 0
                            AccordionElement {
                                id: eliteRizerOptionsAccordion
                                title: qsTr("Elite Rizer Options")
                                indicatRectColor: Material.color(Material.Grey)
                                textColor: Material.color(Material.Blue)
                                color: Material.backgroundColor
                                accordionContent: ColumnLayout {
                                    spacing: 0
                                    Label {
                                        id: labelEliteRizerName
                                        text: qsTr("Elite Rizer:")
                                        Layout.fillWidth: true
                                    }
                                    RowLayout {
                                        spacing: 10
                                        ComboBox {
                                            id: eliteRizerNameTextField
                                            model: rootItem.bluetoothDevices
                                            displayText: settings.elite_rizer_name
                                            Layout.fillHeight: false
                                            Layout.fillWidth: true
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
                                            onClicked: { settings.elite_rizer_name = eliteRizerNameTextField.displayText;; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                                        }
                                    }

                                    Button {
                                        id: refreshEliteRizerNameButton
                                        text: "Refresh Devices List"
                                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                        onClicked: refresh_bluetooth_devices_clicked();
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
                                            onClicked: { settings.elite_rizer_gain = eliteRizerGainTextField.text; toast.show("Setting saved!"); }
                                        }
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
                                    spacing: 0
                                    Label {
                                        id: labelEliteSterzoSmartName
                                        text: qsTr("Elite Sterzo Smart:")
                                        Layout.fillWidth: true
                                    }
                                    RowLayout {
                                        spacing: 10
                                        ComboBox {
                                            id: eliteSterzoSmartNameTextField
                                            model: rootItem.bluetoothDevices
                                            displayText: settings.elite_sterzo_smart_name
                                            Layout.fillHeight: false
                                            Layout.fillWidth: true
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
                                            onClicked: { settings.elite_sterzo_smart_name = eliteSterzoSmartNameTextField.displayText; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
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
                            spacing: 0
                            Label {
                                id: labelFTMSAccessoryName
                                text: qsTr("SmartSpin2k device:")
                                Layout.fillWidth: true
                            }
                            RowLayout {
                                spacing: 10
                                ComboBox {
                                    id: ftmsAccessoryNameTextField
                                    model: rootItem.bluetoothDevices
                                    displayText: settings.ftms_accessory_name
                                    Layout.fillHeight: false
                                    Layout.fillWidth: true
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
                                    onClicked: { settings.ftms_accessory_name = ftmsAccessoryNameTextField.displayText; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                                }
                            }

                            Button {
                                id: refreshFTMSAccessoryNameButton
                                text: "Refresh Devices List"
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                onClicked: refresh_bluetooth_devices_clicked();
                            }                                                        

                            IndicatorOnlySwitch {
                                id: ss2kPelotonDelegate
                                text: qsTr("Peloton Bike")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.ss2k_peloton
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.ss2k_peloton = checked; window.settings_restart_to_apply = true; }
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
                                    onClicked: { settings.ss2k_shift_step = ss2kShiftStepTextField.text; toast.show("Setting saved!"); }
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
                                    onClicked: { settings.ss2k_max_resistance = ss2kMaxResistanceTextField.text; toast.show("Setting saved!"); }
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
                                    onClicked: { settings.ss2k_min_resistance = ss2kMinResistanceTextField.text; toast.show("Setting saved!"); }
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
                                            onClicked: { settings.ss2k_resistance_sample_1 = ss2kResistanceSample1TextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
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
                                            onClicked: { settings.ss2k_shift_step_sample_1 = ss2kShiftStepSample1TextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
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
                                            onClicked: { settings.ss2k_resistance_sample_2 = ss2kResistanceSample2TextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
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
                                            onClicked: { settings.ss2k_shift_step_sample_2 = ss2kShiftStepSample2TextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
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
                                            onClicked: { settings.ss2k_resistance_sample_3 = ss2kResistanceSample3TextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
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
                                            onClicked: { settings.ss2k_shift_step_sample_3 = ss2kShiftStepSample3TextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
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
                                            onClicked: { settings.ss2k_resistance_sample_4 = ss2kResistanceSample4TextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
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
                                            onClicked: { settings.ss2k_shift_step_sample_4 = ss2kShiftStepSample4TextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
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
                            IndicatorOnlySwitch {
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
                                onClicked: { settings.fitmetria_fanfit_enable = checked; window.settings_restart_to_apply = true; }
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
                                    onClicked: { settings.fitmetria_fanfit_mode = fitmetriaFanFitModeTextField.displayText; toast.show("Setting saved!"); }
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
                                    onClicked: { settings.fitmetria_fanfit_min = fitmetriaFanFitMinTextField.text; toast.show("Setting saved!"); }
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
                                    onClicked: { settings.fitmetria_fanfit_max = fitmetriaFanFitMaxTextField.text; toast.show("Setting saved!"); }
                                }
                            }
                        }
                    }

                    AccordionElement {
                        title: qsTr("Wahoo Kickr HeadWind Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor

                        accordionContent: ColumnLayout {
                            spacing: 0
                            IndicatorOnlySwitch {
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
                                onClicked: { settings.fitmetria_fanfit_enable = checked; window.settings_restart_to_apply = true; }
                            }

                            RowLayout {
                                spacing: 10
                                Label {
                                    text: qsTr("Mode:")
                                    Layout.fillWidth: true
                                }
                                ComboBox {
                                    id: headWindModeTextField
                                    model: [ "Heart", "Power", "Manual" ]
                                    displayText: settings.fitmetria_fanfit_mode
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onActivated: {
                                        console.log("combomodel activated" + headWindModeTextField.currentIndex)
                                        displayText = headWindModeTextField.currentValue
                                    }

                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.fitmetria_fanfit_mode = headWindModeTextField.displayText; toast.show("Setting saved!"); }
                                }
                            }
                            RowLayout {
                                spacing: 10
                                Label {
                                    text: qsTr("Min. value (0-100):")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: headWindMinTextField
                                    text: settings.fitmetria_fanfit_min
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    onAccepted: settings.fitmetria_fanfit_min = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.fitmetria_fanfit_min = headWindMinTextField.text; toast.show("Setting saved!"); }
                                }
                            }
                            RowLayout {
                                spacing: 10
                                Label {
                                    text: qsTr("Max value (0-100):")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: headWindMaxTextField
                                    text: settings.fitmetria_fanfit_max
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    onAccepted: settings.fitmetria_fanfit_max = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.fitmetria_fanfit_max = headWindMaxTextField.text; toast.show("Setting saved!"); }
                                }
                            }
                        }
                    }

                    AccordionElement {
                        title: qsTr("Elite Aria Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor

                        accordionContent: ColumnLayout {
                            spacing: 0
                            IndicatorOnlySwitch {
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
                                onClicked: { settings.fitmetria_fanfit_enable = checked; window.settings_restart_to_apply = true; }
                            }

                            RowLayout {
                                spacing: 10
                                Label {
                                    text: qsTr("Mode:")
                                    Layout.fillWidth: true
                                }
                                ComboBox {
                                    id: eliteAriaModeTextField
                                    model: [ "Heart", "Power", "Manual" ]
                                    displayText: settings.fitmetria_fanfit_mode
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onActivated: {
                                        console.log("combomodel activated" + eliteAriaModeTextField.currentIndex)
                                        displayText = eliteAriaModeTextField.currentValue
                                    }

                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.fitmetria_fanfit_mode = eliteAriaModeTextField.displayText; toast.show("Setting saved!"); }
                                }
                            }
                            RowLayout {
                                spacing: 10
                                Label {
                                    text: qsTr("Min. value (0-100):")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: eliteAriaMinTextField
                                    text: settings.fitmetria_fanfit_min
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    onAccepted: settings.fitmetria_fanfit_min = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.fitmetria_fanfit_min = eliteAriaMinTextField.text; toast.show("Setting saved!"); }
                                }
                            }
                            RowLayout {
                                spacing: 10
                                Label {
                                    text: qsTr("Max value (0-100):")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: eliteAriaMaxTextField
                                    text: settings.fitmetria_fanfit_max
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    onAccepted: settings.fitmetria_fanfit_max = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.fitmetria_fanfit_max = eliteAriaMaxTextField.text; toast.show("Setting saved!"); }
                                }
                            }
                        }
                    }

/*
                AccordionElement {
                        title: qsTr("SRAM Devices Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor

                        accordionContent: ColumnLayout {
                            spacing: 0
                            IndicatorOnlySwitch {
                                text: qsTr("SRAM AXS")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.sram_axs_controller
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.sram_axs_controller = checked; window.settings_restart_to_apply = true; }
                            }

                            Label {
                                text: qsTr("Use it to change the gears on QZ!")
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
                    }*/

                AccordionElement {
                        title: qsTr("Zwift Devices Options")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor

                        accordionContent: ColumnLayout {
                            spacing: 0
                            IndicatorOnlySwitch {
                                text: qsTr("Zwift Click")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.zwift_click
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.zwift_click = checked; window.settings_restart_to_apply = true; }
                            }

                            Label {
                                text: qsTr("Use it to change the gears on QZ!")
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

                            IndicatorOnlySwitch {
                                text: qsTr("Zwift Play")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.zwift_play
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.zwift_play = checked; window.settings_restart_to_apply = true; }
                            }

                            Label {
                                text: qsTr("Also for Elite Square. Use it to change the gears on QZ!")
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

                            IndicatorOnlySwitch {
                                text: qsTr("Buttons debouncing")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.gears_volume_debouncing
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.gears_volume_debouncing = checked; }
                            }

                            Label {
                                text: qsTr("Debounce the buttons, so you will only see 1 gear step even if you are keep pressing the buttons.  Default is off.")
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

                            IndicatorOnlySwitch {
                                text: qsTr("Swap sides")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.zwiftplay_swap
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.zwiftplay_swap = checked; }
                            }

                            Label {
                                text: qsTr("You can swap the left to the right controller and viceversa.  Default is off.")
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

                            IndicatorOnlySwitch {
                                text: qsTr("Use Zwift app ratio for gears (Experimental)")
                                spacing: 0
                                bottomPadding: 0
                                topPadding: 0
                                rightPadding: 0
                                leftPadding: 0
                                clip: false
                                checked: settings.gears_zwift_ratio
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                onClicked: { settings.gears_zwift_ratio = checked; }
                            }

                            Label {
                                text: qsTr("Use the zwift gears table instead of the QZ classic gears algorithm.  Default is off.")
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
                                    text: qsTr("Pool time (ms):")
                                    Layout.fillWidth: true
                                }
                                TextField {
																		id: zwiftDevPollTimeTextField
                                    text: settings.poll_device_time
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    onAccepted: settings.poll_device_time = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.poll_device_time = zwiftDevPollTimeTextField.text; toast.show("Setting saved!"); window.settings_restart_to_apply = true;}
                                }
                            }
                            Label {
                                text: qsTr("Default: 200ms. Lower it if you want to improve the gear reactivity. Warning: lowering this value will cause more power used on the QZ device")
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
                            onClicked: { settings.maps_type = mapsTypeTextField.displayText; toast.show("Setting saved!"); }
                        }
                    }
                    IndicatorOnlySwitch {
                        id: gpxLoopDelegate
                        text: qsTr("Loop Start-End-Start")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.gpx_loop
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: settings.gpx_loop = checked
                    }
                }
            }

            /*
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
            }*/

            // static in order to handle the AccordionCheckElement
            StaticAccordionElement {
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
                    IndicatorOnlySwitch {
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
                        onClicked: { settings.bluetooth_relaxed = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("Leave this setting off unless the Support staff asks you to turn it on during troubleshooting. Can improve the Android Bluetooth connection to Zwift. Default is off.")
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

                    IndicatorOnlySwitch {
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
                        onClicked: { settings.bluetooth_30m_hangs = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("Same as “Relaxed Bluetooth for mad devices”. Leave off unless the Support staff asks you to turn it on. Default is off.")
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

                    IndicatorOnlySwitch {
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
                        onClicked: { settings.battery_service = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("Leave this off unless the Support staff asks you to turn it on. Enables a new Bluetooth service, indicating the battery level of your device. Default is off.")
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
                    IndicatorOnlySwitch {
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
                                    IndicatorOnlySwitch {
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
                                        onClicked: { settings.virtual_device_onlyheart = checked; window.settings_restart_to_apply = true; }
                                    }

                                    Label {
                                        text: qsTr("Forces QZ to communicate ONLY the Heart Rate metric to third-party apps. Default is off.")
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

                                    IndicatorOnlySwitch {
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
                                        onClicked: { settings.virtual_device_echelon = checked; window.settings_restart_to_apply = true; }
                                    }

                                    Label {
                                        text: qsTr("Enables QZ to communicate with the Echelon app. This setting can only be used with iOS running QZ and iOS running the Echelon app. Default is off.")
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

                                    IndicatorOnlySwitch {
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
                                        onClicked: { settings.virtual_device_rower = checked; window.settings_restart_to_apply = true; }
                                    }

                                    Label {
                                        text: qsTr("Enables QZ to send a rower Bluetooth profile instead of a bike profile to third party apps that support rowing (examples: Kinomap and BitGym). This should be off for Zwift. Default is off.")
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

                                    IndicatorOnlySwitch {
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

                                    Label {
                                        text: qsTr("Enables third-party apps to change the resistance of your equipment. Default is on.")
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


                                    IndicatorOnlySwitch {
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
                                        onClicked: { settings.bike_power_sensor = checked; window.settings_restart_to_apply = true; }
                                    }

                                    Label {
                                        text: qsTr("This changes the virtual Bluetooth bridge from the standard FMTS to the Power Sensor interface. Default is off.")
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

                                    IndicatorOnlySwitch {
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
                                        onClicked: { settings.virtual_device_ifit = checked; window.settings_restart_to_apply = true; }
                                    }

                                    Label {
                                        text: qsTr("Enables a virtual bluetooth bridge to the iFit App. This setting requires that at least one device be Android. For example, this setting does NOT work with QZ on iOS and iFit to iOS, but DOES work with QZ on iOS and iFit to Android. On Android remember to rename your device into I_EL into the android settings and reboot your device.")
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
                            AccordionCheckElement {
                                id: dirconAccordion
                                title: qsTr("Wahoo direct connect")
                                linkedBoolSetting: "dircon_yes"
                                settings: settings
                                accordionContent: ColumnLayout {
                                    spacing: 0
                                    /*
                                    IndicatorOnlySwitch {
                                        id: wahooRGTDirconDelegate
                                        text: qsTr("MyWhoosh Compatibility")
                                        spacing: 0
                                        bottomPadding: 0
                                        topPadding: 0
                                        rightPadding: 0
                                        leftPadding: 0
                                        clip: false
                                        checked: settings.wahoo_rgt_dircon
                                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                        Layout.fillWidth: true
                                        onClicked: { settings.wahoo_rgt_dircon = checked; window.settings_restart_to_apply = true; }
                                    }*/

                                    Label {
                                        text: qsTr("Enables the compatibility of the Wahoo KICKR protocol to Wahoo RGT app. Leave the RGT compatibility disabled in order to use Zwift.")
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
                                            text: qsTr("ID:")
                                            Layout.fillWidth: true
                                        }
                                        TextField {
                                            id: dirconIdTextField
                                            text: settings.dircon_id
                                            horizontalAlignment: Text.AlignRight
                                            Layout.fillHeight: false
                                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                                            onAccepted: settings.dircon_id = text
                                            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                        }
                                        Button {
                                            text: "OK"
                                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                            onClicked: { settings.dircon_id = dirconIdTextField.text; toast.show("Setting saved!"); window.settings_restart_to_apply = true; }
                                        }
                                    }

                                    Label {
                                        text: qsTr("If you have multiple QZ instances, you can change the id of the virtual wahoo device. Default: 0")
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
                                            onClicked: { settings.dircon_server_base_port = dirconServerPortTextField.text; toast.show("Setting saved!"); }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    AccordionElement {
                        id: mqttAccordion
                        title: qsTr("MQTT Settings")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                            spacing: 0
                            
                            RowLayout {
                                spacing: 10
                                Label {
                                    text: qsTr("MQTT Host:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: mqttHostTextField
                                    text: settings.mqtt_host
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onAccepted: settings.mqtt_host = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.mqtt_host = mqttHostTextField.text; toast.show("Setting saved!"); }
                                }
                            }

                            Label {
                                text: qsTr("Enter the MQTT broker hostname or IP address")
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
                                    text: qsTr("MQTT Port:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: mqttPortTextField
                                    text: settings.mqtt_port
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    onAccepted: settings.mqtt_port = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.mqtt_port = mqttPortTextField.text; toast.show("Setting saved!"); }
                                }
                            }

                            Label {
                                text: qsTr("Enter the MQTT broker port (default: 1883)")
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

                            // Username field
                            RowLayout {
                                spacing: 10
                                Label {
                                    text: qsTr("Username:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: mqttUsernameTextField
                                    text: settings.mqtt_username
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onAccepted: settings.mqtt_username = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.mqtt_username = mqttUsernameTextField.text; toast.show("Setting saved!"); }
                                }
                            }

                            Label {
                                text: qsTr("Enter the MQTT broker username (if required)")
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

                            // Password field
                            RowLayout {
                                spacing: 10
                                Label {
                                    text: qsTr("Password:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: mqttPasswordTextField
                                    text: settings.mqtt_password
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    echoMode: TextInput.Password
                                    onAccepted: settings.mqtt_password = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.mqtt_password = mqttPasswordTextField.text; toast.show("Setting saved!"); }
                                }
                            }

                            Label {
                                text: qsTr("Enter the MQTT broker password (if required)")
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

                            // Device ID field
                            RowLayout {
                                spacing: 10
                                Label {
                                    text: qsTr("Device ID:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: mqttDeviceIdTextField
                                    text: settings.mqtt_deviceid
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onAccepted: settings.mqtt_deviceid = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.mqtt_deviceid = mqttDeviceIdTextField.text; toast.show("Setting saved!"); }
                                }
                            }

                            Label {
                                text: qsTr("Enter a unique device identifier for MQTT client")
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

                    AccordionElement {
                        id: oscAccordion
                        title: qsTr("OSC Settings")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: ColumnLayout {
                        spacing: 0

                            RowLayout {
                                spacing: 10
                                Label {
                                    text: qsTr("OSC IP:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: oscIPTextField
                                    text: settings.osc_ip
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    //inputMethodHints: Qt.ImhFormattedNumbersOnly
                                    onAccepted: settings.osc_ip = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.osc_ip = oscIPTextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                                }
                            }

                            RowLayout {
                                spacing: 10
                                Label {
                                    text: qsTr("OSC Port:")
                                    Layout.fillWidth: true
                                }
                                TextField {
                                    id: oscPortTextField
                                    text: settings.osc_port
                                    horizontalAlignment: Text.AlignRight
                                    Layout.fillHeight: false
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    //inputMethodHints: Qt.ImhFormattedNumbersOnly
                                    onAccepted: settings.osc_port = text
                                    onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                                }
                                Button {
                                    text: "OK"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    onClicked: { settings.osc_port = oscPortTextField.text; window.settings_restart_to_apply = true; toast.show("Setting saved!"); }
                                }
                            }
                        }
                    }

                    IndicatorOnlySwitch {
                        text: qsTr("Race Mode")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.race_mode
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.race_mode = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("By default QZ sends the info to Zwift or any other 3rd party apps with a 1000ms interval rate. Enabling the Race Mode setting will cause QZ to send them to 100ms (10hz). Of course the bottleneck will be always your bike/treadmill.")
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

                    IndicatorOnlySwitch {
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
                        onClicked: { settings.run_cadence_sensor = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("Forces the virtual Bluetooth bridge to send only the cadence information instead of the full FTMS metrics. Default is off.")
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

                    IndicatorOnlySwitch {
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
                        onClicked: { settings.android_wakelock = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("Forces Android devices to remain awake while QZ is running. Default is on.")
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

                    IndicatorOnlySwitch {
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
                        onClicked: { settings.ios_peloton_workaround = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("This MUST be always ON on an iOS device. Turning it OFF will lead to unexpected crashes of QZ. Default is on.")
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

                    IndicatorOnlySwitch {
                        text: qsTr("iOS Bluetooth Device Native")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.ios_btdevice_native
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.ios_btdevice_native = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("If you are experiencing crash on iOS midride, try to turn this on. Default is off.")
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

                    IndicatorOnlySwitch {
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
                        onClicked: { settings.applewatch_fakedevice = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("Simulates QZ being connected to a bike. When this is turned on QZ will calculate KCal based on your heart rate. Examples of when to use this setting: ○ To capture Peloton class data for classes without connected equipment (e.g., a strength or yoga workout).. ○ To arrange tiles on the QZ dashboard without connecting to your equipment. ○ To use the QZ Apple Watch app without connecting to your equipment.")
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

                    IndicatorOnlySwitch {
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
                        onClicked: { settings.fakedevice_treadmill = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("Same as Fake Device but instead of simulating a bike it simulates a treadmill.")
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

                    IndicatorOnlySwitch {
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
                        onClicked: { settings.fakedevice_elliptical = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("Same as Fake Device but instead of simulating a bike it simulates an elliptical.")
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

                    IndicatorOnlySwitch {
                        text: qsTr("Fake Rower")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.fakedevice_rower
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.fakedevice_rower = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("Same as Fake Device but instead of simulating a bike it simulates a rower.")
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

                    IndicatorOnlySwitch {
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
                        onClicked: { settings.ios_cache_heart_device = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("Leave this on unless you have issues connecting your Bluetooth HRM to QZ. If turning this off does not solve the connection issue, open a support ticket on GitHub. Default is on.")
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

                    IndicatorOnlySwitch {
                        id: androidNotificationDelegate
                        text: qsTr("Android Notification")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.android_notification
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.android_notification = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("Android Only: enable this to force Android to don't kill QZ when it's running on background")
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

                    IndicatorOnlySwitch {
                        text: qsTr("Android Force Documents/QZ Folder")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        clip: false
                        checked: settings.android_documents_folder
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.android_documents_folder = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("Android Only: force QZ to use the /Documents/QZ folder for debug log and fit files")
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

                    IndicatorOnlySwitch {
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
                        onClicked: { settings.log_debug = checked; window.settings_restart_to_apply = true; }
                    }

                    Label {
                        text: qsTr("Turn this on to save a debug log to your device for use when requesting help with a bug.")
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
                        Layout.fillWidth: true
                        
                        Button {
                            id: clearLogs
                            text: "Clear History"
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                            onClicked: rootItem.clearFiles();
                        }
                        
                        Button {
                            text: "Show Logs Folder"
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: {
                                toast.show(rootItem.getProfileDir())
                            }
                        }
                    }

                    Label {
                        text: qsTr("Clears all the QZ logs, QZ .fit files and QZ images (these files are saved by QZ for every session) from your device while maintaining your saved Profiles and Settings.")
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
        }
    }
/*##^##
Designer {
    D{i:0;formeditorZoom:0.6600000262260437}
}
##^##*/
