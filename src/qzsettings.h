#ifndef QZSETTINGS_H
#define QZSETTINGS_H

#include <QString>
#include "fit-sdk/fit_profile.hpp"

class QZSettings {
  private:
    QZSettings() {}

  public:
    //--------------------------------------------------------------------------------------------
    // These are not in settings.qml
    //--------------------------------------------------------------------------------------------

    static const QString cryptoKeySettingsProfiles;
    static constexpr int default_cryptoKeySettingsProfiles = 0;
    /**
     *@brief Disable (true) reconnection when the device disconnects from Bluetooth.
     */
    static const QString bluetooth_no_reconnection;
    static constexpr bool default_bluetooth_no_reconnection = false;

    /**
     *@brief Choose between wheel revolutions (true) and wheel and crank revolutions (false)
     *when configuring the CSC feature BLE characteristic.
     */
    static const QString bike_wheel_revs;
    static constexpr bool default_bike_wheel_revs = false;

    static const QString bluetooth_lastdevice_name;
    static const QString default_bluetooth_lastdevice_name;

    static const QString bluetooth_lastdevice_address;
    static const QString default_bluetooth_lastdevice_address;

    static const QString hrm_lastdevice_name;
    static const QString default_hrm_lastdevice_name;

    static const QString hrm_lastdevice_address;
    static const QString default_hrm_lastdevice_address;

    static const QString ftms_accessory_address;
    static const QString default_ftms_accessory_address;

    static const QString ftms_accessory_lastdevice_name;
    static const QString default_ftms_accessory_lastdevice_name;

    static const QString csc_sensor_address;
    static const QString default_csc_sensor_address;

    static const QString csc_sensor_lastdevice_name;
    static const QString default_csc_sensor_lastdevice_name;

    static const QString power_sensor_lastdevice_name;
    static const QString default_power_sensor_lastdevice_name;

    static const QString power_sensor_address;
    static const QString default_power_sensor_address;

    static const QString elite_rizer_lastdevice_name;
    static const QString default_elite_rizer_lastdevice_name;

    static const QString elite_rizer_address;
    static const QString default_elite_rizer_address;

    static const QString elite_sterzo_smart_lastdevice_name;
    static const QString default_elite_sterzo_smart_lastdevice_name;

    static const QString elite_sterzo_smart_address;
    static const QString default_elite_sterzo_smart_address;

    static const QString strava_accesstoken;
    static const QString default_strava_accesstoken;

    static const QString strava_refreshtoken;
    static const QString default_strava_refreshtoken;

    static const QString strava_lastrefresh;
    static const QString default_strava_lastrefresh;

    static const QString strava_expires;
    static const QString default_strava_expires;

    static const QString code;
    static const QString default_code;

    //--------------------------------------------------------------------------------------------

    /**
     *@brief Zoom percentage for the user interface.
     */
    static const QString ui_zoom;
    static constexpr float default_ui_zoom = 100.0;

    /**
     *@brief Disable (true) or use (false) the device's heart rate service.
     */
    static const QString bike_heartrate_service;
    static constexpr bool default_bike_heartrate_service = false;

    /**
     *@brief An offset that can be applied to the resistance from the device.
     * calculated_resistance = raw_resistance * bike_resistance_gain_f + bike_resistance_offset
     */
    static const QString bike_resistance_offset;
    static constexpr int default_bike_resistance_offset = 4;

    /**
     *@brief A gain that can be applied to the resistance from the device.
     * calculated_resistance = raw_resistance * bike_resistance_gain_f + bike_resistance_offset
     */
    static const QString bike_resistance_gain_f;
    static constexpr float default_bike_resistance_gain_f = 1.0;

    /**
     *@brief Used to specify of QZ is using Zwift in ERG (workout) Mode.
     * When supporting this, QZ should communicate the target resistance
     * (or automatically adjust the device's resistance if it has this capability) to match the target
     * watts based on the cadence (RPM). In ERG Mode, the changes in inclination should not affect target resistance,
     * as is the case in Simulation Mode. Default is false.
     *
     */
    static const QString zwift_erg;
    static constexpr bool default_zwift_erg = false;

    /**
     *@brief In ERG Mode, Zwift sends a “target output” request. If the output requested doesn’t match the current
     *output (calculated using cadence and resistance level), the target resistance should change to help the user get
     *closer to the target output. If the filter is set to higher values, there should be less adjustment of the target
     *resistance and cadence would need to be increased to match the target output. The zwift_erg_filter and
     *zwift_erg_filter_down settings are the upper and lower margin before the adjustment of resistance is communicated.
     *Example: if the zwift_erg_filter and zwift_erg_filter_down filters are set to 10 and the target output is 100
     *watts, a change of resistance will only be communicated if the device produces less than 90 Watts or more than 110
     *Watts.
     */
    static const QString zwift_erg_filter;
    static constexpr float default_zwift_erg_filter = 10.0;

    /**
     *@brief In ERG Mode, Zwift sends a “target output” request. If the output requested doesn’t match the current
     *output (calculated using cadence and resistance level), the target resistance should change to help the user get
     *closer to the target output. If the filter is set to higher values, there should be less adjustment of the target
     *resistance and cadence would need to be increased to match the target output. The zwift_erg_filter and
     *zwift_erg_filter_down settings are the upper and lower margin before the adjustment of resistance is communicated.
     *Example: if the zwift_erg_filter and zwift_erg_filter_down filters are set to 10 and the target output is 100
     *watts, a change of resistance will only be communicated if the device produces less than 90 Watts or more than 110
     *Watts.
     */
    static const QString zwift_erg_filter_down;
    static constexpr float default_zwift_erg_filter_down = 10.0;

    /**
     *@brief Used to invoke a workaround whereby negative inclination is multiplied by 2.
     */
    static const QString zwift_negative_inclination_x2;
    static constexpr bool default_zwift_negative_inclination_x2 = false;

    /**
     *@brief An offset that will be applied to the inclination received from the client application.
     * calculated_inclination = raw_inclination * zwift_inclination_gain + zwift_inclination_offset
     */
    static const QString zwift_inclination_offset;
    static constexpr float default_zwift_inclination_offset = 0;

    /**
     *@brief A gain that will be applied to the inclination received from the client application.
     * calculated_inclination = raw_inclination * zwift_inclination_gain + zwift_inclination_offset
     */
    static const QString zwift_inclination_gain;
    static constexpr float default_zwift_inclination_gain = 1.0;

    static const QString echelon_resistance_offset;
    static constexpr float default_echelon_resistance_offset = 0;

    static const QString echelon_resistance_gain;
    static constexpr float default_echelon_resistance_gain = 1.0;

    /**
     *@brief Used for some devices to specify that speed should be calculated from power.
     */
    static const QString speed_power_based;
    static constexpr bool default_speed_power_based = false;

    /**
     *@brief The resistance to be set when a bike or elliptical trainer first connects.
     */
    static const QString bike_resistance_start;
    static constexpr int default_bike_resistance_start = 1;

    /**
     *@brief The age of the user in years.
     */
    static const QString age;
    static constexpr int default_age = 35.0;

    /**
     *@brief The mass of the user in kilograms. Used for power calculations.
     */
    static const QString weight;
    static constexpr float default_weight = 75.0;

    /**
     *@brief The user's Functional Threshold Power in watts per kilogram. This is a measure of the best average power
     *output the user could sustain for 1 hour in a time-trial scenario.
     */
    static const QString ftp;
    static constexpr float default_ftp = 200.0;

    /**
     *@brief Email address of the user.
     */
    static const QString user_email;

    /**
     * @brief Default email address of user. Empty Latin1 string.
     * This is a Latin1 string because of where it could be used.
     */
    static const QString default_user_email;

    static const QString user_nickname;
    static const QString default_user_nickname;

    /**
     *@brief Specifies whether or not to use miles (false) or kilometers (true) as the unit of distance.
     */
    static const QString miles_unit;
    static constexpr bool default_miles_unit = false;

    static const QString pause_on_start;
    static constexpr bool default_pause_on_start = false;

    /**
     *@brief value for peloton trainrow.forcespeed.
     */
    static const QString treadmill_force_speed;
    static constexpr bool default_treadmill_force_speed = false;

    static const QString pause_on_start_treadmill;
    static constexpr bool default_pause_on_start_treadmill = false;

    /**
     *@brief Flag to indicate if it should be ignored (true) that the user has stopped doing work.
     */
    static const QString continuous_moving;
    static constexpr bool default_continuous_moving = false;

    static const QString bike_cadence_sensor;
    static constexpr bool default_bike_cadence_sensor = false;

    static const QString run_cadence_sensor;
    static constexpr bool default_run_cadence_sensor = false;

    static const QString rogue_echo_bike;
    static constexpr bool default_rogue_echo_bike = false;

    static const QString bike_power_sensor;
    static constexpr bool default_bike_power_sensor = false;

    static const QString bike_power_offset;
    static constexpr int default_bike_power_offset = 0;

    static const QString heart_rate_belt_name;
    static const QString default_heart_rate_belt_name;

    /**
     *@brief Used to ignore the heart rate from some devices.
     */
    static const QString heart_ignore_builtin;
    static constexpr bool default_heart_ignore_builtin = false;

    /**
     * @brief Used to ignore an energy reading from some devices.
     */
    static const QString kcal_ignore_builtin;
    static constexpr bool default_kcal_ignore_builtin = false;

    static const QString ant_cadence;
    static constexpr bool default_ant_cadence = false;

    static const QString ant_heart;
    static constexpr bool default_ant_heart = false;

    static const QString ant_garmin;
    static constexpr bool default_ant_garmin = false;

    static const QString top_bar_enabled;
    static constexpr bool default_top_bar_enabled = true;
    /**
     *@brief The username for logging in to Peloton.
     */
    static const QString peloton_username;
    static const QString default_peloton_username;

    /**
     *@brief The password for logging in to Peloton.
     */
    static const QString peloton_password;
    static const QString default_peloton_password;

    static const QString peloton_difficulty;
    static const QString default_peloton_difficulty;

    static const QString peloton_cadence_metric;
    static const QString default_peloton_cadence_metric;

    static const QString peloton_heartrate_metric;
    static const QString default_peloton_heartrate_metric;

    static const QString peloton_date;
    static const QString default_peloton_date;

    static const QString peloton_description_link;
    static constexpr bool default_peloton_description_link = true;

    static const QString pzp_username;
    static const QString default_pzp_username;

    static const QString pzp_password;
    static const QString default_pzp_password;

    static const QString tile_speed_enabled;
    static constexpr bool default_tile_speed_enabled = true;

    static const QString tile_speed_order;
    static constexpr int default_tile_speed_order = 0;

    static const QString tile_inclination_enabled;
    static constexpr bool default_tile_inclination_enabled = true;

    static const QString tile_inclination_order;
    static constexpr int default_tile_inclination_order = 1;

    static const QString tile_cadence_enabled;
    static constexpr bool default_tile_cadence_enabled = true;

    static const QString tile_cadence_order;
    static constexpr int default_tile_cadence_order = 2;

    static const QString tile_elevation_enabled;
    static constexpr bool default_tile_elevation_enabled = true;

    static const QString tile_elevation_order;
    static constexpr int default_tile_elevation_order = 3;

    static const QString tile_calories_enabled;
    static constexpr bool default_tile_calories_enabled = true;

    static const QString tile_calories_order;
    static constexpr int default_tile_calories_order = 4;

    static const QString tile_odometer_enabled;
    static constexpr bool default_tile_odometer_enabled = true;

    static const QString tile_odometer_order;
    static constexpr int default_tile_odometer_order = 5;

    static const QString tile_pace_enabled;
    static constexpr bool default_tile_pace_enabled = true;

    static const QString tile_pace_order;
    static constexpr int default_tile_pace_order = 6;

    static const QString tile_resistance_enabled;
    static constexpr bool default_tile_resistance_enabled = true;

    static const QString tile_resistance_order;
    static constexpr int default_tile_resistance_order = 7;

    static const QString tile_watt_enabled;
    static constexpr bool default_tile_watt_enabled = true;

    static const QString tile_watt_order;
    static constexpr int default_tile_watt_order = 8;

    static const QString tile_weight_loss_enabled;
    static constexpr bool default_tile_weight_loss_enabled = false;

    static const QString tile_weight_loss_order;
    static constexpr int default_tile_weight_loss_order = 24;

    static const QString tile_avgwatt_enabled;
    static constexpr bool default_tile_avgwatt_enabled = true;

    static const QString tile_avgwatt_order;
    static constexpr int default_tile_avgwatt_order = 9;

    static const QString tile_ftp_enabled;
    static constexpr bool default_tile_ftp_enabled = true;

    static const QString tile_ftp_order;
    static constexpr int default_tile_ftp_order = 10;

    static const QString tile_heart_enabled;
    static constexpr bool default_tile_heart_enabled = true;

    static const QString tile_heart_order;
    static constexpr int default_tile_heart_order = 11;

    static const QString tile_fan_enabled;
    static constexpr bool default_tile_fan_enabled = true;

    static const QString tile_fan_order;
    static constexpr int default_tile_fan_order = 12;

    static const QString tile_jouls_enabled;
    static constexpr bool default_tile_jouls_enabled = true;

    static const QString tile_jouls_order;
    static constexpr int default_tile_jouls_order = 13;

    static const QString tile_elapsed_enabled;
    static constexpr bool default_tile_elapsed_enabled = true;

    static const QString tile_elapsed_order;
    static constexpr int default_tile_elapsed_order = 14;

    static const QString tile_lapelapsed_enabled;
    static constexpr bool default_tile_lapelapsed_enabled = false;

    static const QString tile_lapelapsed_order;
    static constexpr int default_tile_lapelapsed_order = 17;

    static const QString tile_moving_time_enabled;
    static constexpr bool default_tile_moving_time_enabled = false;

    static const QString tile_moving_time_order;
    static constexpr int default_tile_moving_time_order = 21;

    static const QString tile_peloton_offset_enabled;
    static constexpr bool default_tile_peloton_offset_enabled = false;

    static const QString tile_peloton_offset_order;
    static constexpr int default_tile_peloton_offset_order = 22;

    static const QString tile_peloton_difficulty_enabled;
    static constexpr bool default_tile_peloton_difficulty_enabled = false;

    static const QString tile_peloton_difficulty_order;
    static constexpr int default_tile_peloton_difficulty_order = 32;

    static const QString tile_peloton_resistance_enabled;
    static constexpr bool default_tile_peloton_resistance_enabled = true;

    static const QString tile_peloton_resistance_order;
    static constexpr int default_tile_peloton_resistance_order = 15;

    static const QString tile_datetime_enabled;
    static constexpr bool default_tile_datetime_enabled = true;

    static const QString tile_datetime_order;
    static constexpr int default_tile_datetime_order = 16;

    static const QString tile_target_resistance_enabled;
    static constexpr bool default_tile_target_resistance_enabled = true;

    static const QString tile_target_resistance_order;
    static constexpr int default_tile_target_resistance_order = 15;

    static const QString tile_target_peloton_resistance_enabled;
    static constexpr bool default_tile_target_peloton_resistance_enabled = false;

    static const QString tile_target_peloton_resistance_order;
    static constexpr int default_tile_target_peloton_resistance_order = 21;

    static const QString tile_target_cadence_enabled;
    static constexpr bool default_tile_target_cadence_enabled = false;

    static const QString tile_target_cadence_order;
    static constexpr int default_tile_target_cadence_order = 19;

    static const QString tile_target_power_enabled;
    static constexpr bool default_tile_target_power_enabled = false;

    static const QString tile_target_power_order;
    static constexpr int default_tile_target_power_order = 20;

    static const QString tile_target_zone_enabled;
    static constexpr bool default_tile_target_zone_enabled = false;

    static const QString tile_target_zone_order;
    static constexpr int default_tile_target_zone_order = 24;

    static const QString tile_target_speed_enabled;
    static constexpr bool default_tile_target_speed_enabled = false;

    static const QString tile_target_speed_order;
    static constexpr int default_tile_target_speed_order = 27;

    static const QString tile_target_incline_enabled;
    static constexpr bool default_tile_target_incline_enabled = false;

    static const QString tile_target_incline_order;
    static constexpr int default_tile_target_incline_order = 28;

    static const QString tile_strokes_count_enabled;
    static constexpr bool default_tile_strokes_count_enabled = false;

    static const QString tile_strokes_count_order;
    static constexpr int default_tile_strokes_count_order = 22;

    static const QString tile_strokes_length_enabled;
    static constexpr bool default_tile_strokes_length_enabled = false;

    static const QString tile_strokes_length_order;
    static constexpr int default_tile_strokes_length_order = 23;

    static const QString tile_watt_kg_enabled;
    static constexpr bool default_tile_watt_kg_enabled = false;

    static const QString tile_watt_kg_order;
    static constexpr int default_tile_watt_kg_order = 25;

    static const QString tile_gears_enabled;
    static constexpr bool default_tile_gears_enabled = false;

    static const QString tile_gears_order;
    static constexpr int default_tile_gears_order = 26;

    static const QString tile_remainingtimetrainprogramrow_enabled;
    static constexpr bool default_tile_remainingtimetrainprogramrow_enabled = false;

    static const QString tile_remainingtimetrainprogramrow_order;
    static constexpr int default_tile_remainingtimetrainprogramrow_order = 27;

    static const QString tile_nextrowstrainprogram_enabled;
    static constexpr bool default_tile_nextrowstrainprogram_enabled = false;

    static const QString tile_nextrowstrainprogram_order;
    static constexpr int default_tile_nextrowstrainprogram_order = 31;

    static const QString tile_mets_enabled;
    static constexpr bool default_tile_mets_enabled = false;

    static const QString tile_mets_order;
    static constexpr int default_tile_mets_order = 28;

    static const QString tile_targetmets_enabled;
    static constexpr bool default_tile_targetmets_enabled = false;

    static const QString tile_targetmets_order;
    static constexpr int default_tile_targetmets_order = 29;

    static const QString tile_steering_angle_enabled;
    static constexpr bool default_tile_steering_angle_enabled = false;

    static const QString tile_steering_angle_order;
    static constexpr int default_tile_steering_angle_order = 30;

    static const QString tile_pid_hr_enabled;
    static constexpr bool default_tile_pid_hr_enabled = false;

    static const QString tile_pid_hr_order;
    static constexpr int default_tile_pid_hr_order = 31;

    static const QString heart_rate_zone1;
    static constexpr float default_heart_rate_zone1 = 70.0;

    static const QString heart_rate_zone2;
    static constexpr float default_heart_rate_zone2 = 80.0;

    static const QString heart_rate_zone3;
    static constexpr float default_heart_rate_zone3 = 90.0;

    static const QString heart_rate_zone4;
    static constexpr float default_heart_rate_zone4 = 100.0;

    static const QString heart_max_override_enable;
    static constexpr bool default_heart_max_override_enable = false;

    static const QString heart_max_override_value;
    static constexpr float default_heart_max_override_value = 195.0;

    static const QString peloton_gain;
    static constexpr float default_peloton_gain = 1.0;

    static const QString peloton_offset;
    static constexpr float default_peloton_offset = 0;

    static const QString treadmill_pid_heart_zone;
    static const QString default_treadmill_pid_heart_zone;
    /**
     *@brief 1 mile time goal, for a training program with the speed control.
     */
    static const QString pacef_1mile;
    static constexpr float default_pacef_1mile = 250;
    /**
     *@brief 5 km time goal, for a training program with the speed control.
     */
    static const QString pacef_5km;
    static constexpr float default_pacef_5km = 300;
    /**
     *@brief 10 km time goal, for a training program with the speed control.
     */
    static const QString pacef_10km;
    static constexpr float default_pacef_10km = 320;
    /**
     *@brief  pacef_1mile, but for half-marathon distance, for a training program with the speed control.
     */
    static const QString pacef_halfmarathon;
    static constexpr float default_pacef_halfmarathon = 340;
    /**
     *@brief  pacef_1mile, but for marathon distance, for a training program with the speed control.
     */
    static const QString pacef_marathon;
    static constexpr float default_pacef_marathon = 360;

    /**
     *@brief default pace to be used when the ZWO file does not indicate a precise pace.
     *Text values, i.e. "1 mile", "5 km", "10 km", "Half Marathon"
     */
    static const QString pace_default;
    static const QString default_pace_default;

    static const QString domyos_treadmill_buttons;
    static constexpr bool default_domyos_treadmill_buttons = false;

    static const QString domyos_treadmill_distance_display;
    static constexpr bool default_domyos_treadmill_distance_display = true;

    static const QString domyos_treadmill_display_invert;
    static constexpr bool default_domyos_treadmill_display_invert = false;

    static const QString domyos_bike_cadence_filter;
    static constexpr float default_domyos_bike_cadence_filter = 0.0;

    static const QString domyos_bike_display_calories;
    static constexpr bool default_domyos_bike_display_calories = true;

    static const QString domyos_elliptical_speed_ratio;
    static constexpr float default_domyos_elliptical_speed_ratio = 1.0;

    static const QString eslinker_cadenza;
    static constexpr bool default_eslinker_cadenza = true;
    static const QString eslinker_ypoo;
    static constexpr bool default_eslinker_ypoo = false;
    /**
     *@brief Choose between the standard and MGARCEA watt table.
     */
    static const QString echelon_watttable;
    static const QString default_echelon_watttable;

    static const QString proform_wheel_ratio;
    static constexpr float default_proform_wheel_ratio = 0.33;

    static const QString proform_tour_de_france_clc;
    static constexpr bool default_proform_tour_de_france_clc = false;

    static const QString proform_tdf_jonseed_watt;
    static constexpr bool default_proform_tdf_jonseed_watt = false;

    static const QString proform_studio;
    static constexpr bool default_proform_studio = false;

    static const QString proform_tdf_10;
    static constexpr bool default_proform_tdf_10 = false;

    static const QString horizon_gr7_cadence_multiplier;
    static constexpr double default_horizon_gr7_cadence_multiplier = 1.0;

    static const QString fitshow_user_id;
    static constexpr int default_fitshow_user_id = 0x13AA;

    static const QString inspire_peloton_formula;
    static constexpr bool default_inspire_peloton_formula = false;

    static const QString inspire_peloton_formula2;
    static constexpr bool default_inspire_peloton_formula2 = false;

    static const QString hammer_racer_s;
    static constexpr bool default_hammer_racer_s = false;

    static const QString pafers_treadmill;
    static constexpr bool default_pafers_treadmill = false;

    static const QString yesoul_peloton_formula;
    static constexpr bool default_yesoul_peloton_formula = false;

    static const QString nordictrack_10_treadmill;
    static constexpr bool default_nordictrack_10_treadmill = true;

    static const QString nordictrack_t65s_treadmill;
    static constexpr bool default_nordictrack_t65s_treadmill = false;

    static const QString nordictrack_treadmill_ultra_le;
    static constexpr bool default_nordictrack_treadmill_ultra_le = false;

    static const QString proform_treadmill_carbon_tls;
    static constexpr bool default_proform_treadmill_carbon_tls = false;

    static const QString proform_treadmill_995i;
    static constexpr bool default_proform_treadmill_995i = false;

    static const QString toorx_3_0;
    static constexpr bool default_toorx_3_0 = false;

    static const QString toorx_65s_evo;
    static constexpr bool default_toorx_65s_evo = false;

    static const QString jtx_fitness_sprint_treadmill;
    static constexpr bool default_jtx_fitness_sprint_treadmill = false;

    static const QString dkn_endurun_treadmill;
    static constexpr bool default_dkn_endurun_treadmill = false;

    static const QString trx_route_key;
    static constexpr bool default_trx_route_key = false;

    static const QString bh_spada_2;
    static constexpr bool default_bh_spada_2 = false;

    static const QString toorx_bike;
    static constexpr bool default_toorx_bike = false;

    static const QString toorx_ftms;
    static constexpr bool default_toorx_ftms = false;

    static const QString jll_IC400_bike;
    static constexpr bool default_jll_IC400_bike = false;

    static const QString fytter_ri08_bike;
    static constexpr bool default_fytter_ri08_bike = false;

    static const QString asviva_bike;
    static constexpr bool default_asviva_bike = false;

    static const QString hertz_xr_770;
    static constexpr bool default_hertz_xr_770 = false;

    static const QString m3i_bike_id;
    static constexpr int default_m3i_bike_id = 256;

    static const QString m3i_bike_speed_buffsize;
    static constexpr int default_m3i_bike_speed_buffsize = 90;

    static const QString m3i_bike_qt_search;
    static constexpr bool default_m3i_bike_qt_search = false;

    static const QString m3i_bike_kcal;
    static constexpr bool default_m3i_bike_kcal = true;

    static const QString snode_bike;
    static constexpr bool default_snode_bike = false;

    static const QString fitplus_bike;
    static constexpr bool default_fitplus_bike = false;

    static const QString virtufit_etappe;
    static constexpr bool default_virtufit_etappe = false;

    static const QString flywheel_filter;
    static constexpr int default_flywheel_filter = 2;

    static const QString flywheel_life_fitness_ic8;
    static constexpr bool default_flywheel_life_fitness_ic8 = false;

    static const QString sole_treadmill_inclination;
    static constexpr bool default_sole_treadmill_inclination = false;

    static const QString sole_treadmill_miles;
    static constexpr bool default_sole_treadmill_miles = true;

    static const QString sole_treadmill_f65;
    static constexpr bool default_sole_treadmill_f65 = false;

    static const QString sole_treadmill_f63;
    static constexpr bool default_sole_treadmill_f63 = false;

    static const QString sole_treadmill_tt8;
    static constexpr bool default_sole_treadmill_tt8 = false;

    static const QString schwinn_bike_resistance;
    static constexpr bool default_schwinn_bike_resistance = false;

    static const QString schwinn_bike_resistance_v2;
    static constexpr bool default_schwinn_bike_resistance_v2 = false;

    static const QString technogym_myrun_treadmill_experimental;
    static constexpr bool default_technogym_myrun_treadmill_experimental = false;

    static const QString technogym_group_cycle;
    static constexpr bool default_technogym_group_cycle = false;

    static const QString ant_bike_device_number;
    static constexpr int default_ant_bike_device_number = 0;

    static const QString ant_heart_device_number;
    static constexpr int default_ant_heart_device_number = 0;

    static const QString trainprogram_random;
    static constexpr bool default_trainprogram_random = false;

    static const QString trainprogram_total;
    static constexpr int default_trainprogram_total = 60;

    static const QString trainprogram_period_seconds;
    static constexpr float default_trainprogram_period_seconds = 60;

    static const QString trainprogram_speed_min;
    static constexpr float default_trainprogram_speed_min = 8;

    static const QString trainprogram_speed_max;
    static constexpr float default_trainprogram_speed_max = 16;

    static const QString trainprogram_incline_min;
    static constexpr float default_trainprogram_incline_min = 0;

    static const QString trainprogram_incline_max;
    static constexpr float default_trainprogram_incline_max = 15;

    static const QString trainprogram_resistance_min;
    static constexpr float default_trainprogram_resistance_min = 1;

    static const QString trainprogram_resistance_max;
    static constexpr float default_trainprogram_resistance_max = 32;

    /**
     * @brief Adjusts value in a metric object that's configured specifically for measuring WATTS.
     */
    static const QString watt_offset;
    static constexpr float default_watt_offset = 0;

    /**
     * @brief Adjusts value in a metric object that's configured specifically for measuring WATTS.
     */
    static const QString watt_gain;
    static constexpr float default_watt_gain = 1;

    static const QString power_avg_5s;
    static constexpr bool default_power_avg_5s = false;

    static const QString instant_power_on_pause;
    static constexpr bool default_instant_power_on_pause = false;

    /**
     * @brief Adjusts value in a metric object that's configured specifically for measuring SPEED.
     */
    static const QString speed_offset;
    static constexpr float default_speed_offset = 0;

    /**
     * @brief Adjusts value in a metric object that's configured specifically for measuring SPEED.
     */
    static const QString speed_gain;
    static constexpr float default_speed_gain = 1;

    static const QString filter_device;
    static const QString default_filter_device;

    static const QString strava_suffix;
    static const QString default_strava_suffix;

    static const QString cadence_sensor_name;
    static const QString default_cadence_sensor_name;

    static const QString cadence_sensor_as_bike;
    static constexpr bool default_cadence_sensor_as_bike = false;

    static const QString cadence_sensor_speed_ratio;
    static constexpr float default_cadence_sensor_speed_ratio = 0.33;

    static const QString power_hr_pwr1;
    static constexpr float default_power_hr_pwr1 = 200;

    static const QString power_hr_hr1;
    static constexpr float default_power_hr_hr1 = 150;

    static const QString power_hr_pwr2;
    static constexpr float default_power_hr_pwr2 = 230;

    static const QString power_hr_hr2;
    static constexpr float default_power_hr_hr2 = 170;

    static const QString power_sensor_name;
    static const QString default_power_sensor_name;

    static const QString power_sensor_as_bike;
    static constexpr bool default_power_sensor_as_bike = false;

    static const QString power_sensor_as_treadmill;
    static constexpr bool default_power_sensor_as_treadmill = false;

    static const QString powr_sensor_running_cadence_double;
    static constexpr bool default_powr_sensor_running_cadence_double = false;

    static const QString elite_rizer_name;
    static const QString default_elite_rizer_name;

    static const QString elite_sterzo_smart_name;
    static const QString default_elite_sterzo_smart_name;

    static const QString ftms_accessory_name;
    static const QString default_ftms_accessory_name;

    static const QString ss2k_shift_step;
    static constexpr float default_ss2k_shift_step = 900;

    static const QString fitmetria_fanfit_enable;
    static constexpr bool default_fitmetria_fanfit_enable = false;

    static const QString fitmetria_fanfit_mode;
    static const QString default_fitmetria_fanfit_mode;

    static const QString fitmetria_fanfit_min;
    static constexpr float default_fitmetria_fanfit_min = 0;

    static const QString fitmetria_fanfit_max;
    static constexpr float default_fitmetria_fanfit_max = 100;
    /**
     *@brief Indicates if the virtual device should send resistance requests to the bike.
     */
    static const QString virtualbike_forceresistance;
    static constexpr bool default_virtualbike_forceresistance = true;
    /**
     *@brief Troubleshooting setting. Should be false unless advised by QZ tech support.
     */
    static const QString bluetooth_relaxed;
    static constexpr bool default_bluetooth_relaxed = false;
    /**
     *@brief Troubleshooting setting. Should be false unless advised by QZ tech support.
     */
    static const QString bluetooth_30m_hangs;
    static constexpr bool default_bluetooth_30m_hangs = false;

    static const QString battery_service;
    static constexpr bool default_battery_service = false;

    /**
     *@brief Experimental feature. Not recommended to use.
     */
    static const QString service_changed;
    static constexpr bool default_service_changed = false;

    /**
     *@brief Enable/disable the virtual device that connects QZ to the client app.
     */
    static const QString virtual_device_enabled;
    static constexpr bool default_virtual_device_enabled = true;
    /**
     *@brief Enable/disable the Bluetooth connectivity of the virtual device that connects QZ to the client app.
     */
    static const QString virtual_device_bluetooth;
    static constexpr bool default_virtual_device_bluetooth = true;

    static const QString ios_peloton_workaround;
    static constexpr bool default_ios_peloton_workaround = true;

    static const QString android_wakelock;
    static constexpr bool default_android_wakelock = true;
    /**
     *@brief Specifies if the debug log file will be written.
     */
    static const QString log_debug;
    static constexpr bool default_log_debug = false;
    /**
     *@brief Force QZ to communicate ONLY the Heart Rate metric to third-party apps.
     */
    static const QString virtual_device_onlyheart;
    static constexpr bool default_virtual_device_onlyheart = false;
    /**
     *@brief Enables QZ to communicate with the Echelon app.
     *This setting can only be used with iOS running QZ and iOS running the Echelon app.
     */
    static const QString virtual_device_echelon;
    static constexpr bool default_virtual_device_echelon = false;
    /**
     *@brief Enables a virtual bluetooth bridge to the iFit App.
     */
    static const QString virtual_device_ifit;
    static constexpr bool default_virtual_device_ifit = false;
    /**
     *@brief Instructs QZ to send a rower Bluetooth profile instead of a bike profile to third party apps that support
     *rowing (examples: Kinomap and BitGym). This should be off for Zwift.
     */
    static const QString virtual_device_rower;
    static constexpr bool default_virtual_device_rower = false;
    /**
     *@brief Used to force a non-bike device to be presented to client apps as a bike.
     */
    static const QString virtual_device_force_bike;
    static constexpr bool default_virtual_device_force_bike = false;

    static const QString volume_change_gears;
    static constexpr bool default_volume_change_gears = false;

    static const QString applewatch_fakedevice;
    static constexpr bool default_applewatch_fakedevice = false;

    /**
     *@brief Minimum target resistance for ERG mode.
     */
    static const QString zwift_erg_resistance_down;
    static constexpr float default_zwift_erg_resistance_down = 0.0;

    /**
     *@brief Maximum targe resistance for ERG mode.
     */
    static const QString zwift_erg_resistance_up;
    static constexpr float default_zwift_erg_resistance_up = 999.0;

    static const QString horizon_paragon_x;
    static constexpr bool default_horizon_paragon_x = false;

    static const QString treadmill_step_speed;
    static constexpr float default_treadmill_step_speed = 0.5;

    static const QString treadmill_step_incline;
    static constexpr float default_treadmill_step_incline = 0.5;

    static const QString fitshow_anyrun;
    static constexpr bool default_fitshow_anyrun = false;

    static const QString nordictrack_s30_treadmill;
    static constexpr bool default_nordictrack_s30_treadmill = false;

    // from version 2.10.23
    // not used anymore because it's an elliptical not a treadmill. Don't remove this
    // it will cause corruption in the settings
    // static const QString nordictrack_fs5i_treadmill;
    // static constexpr bool default_nordictrack_fs5i_treadmill = false;

    static const QString renpho_peloton_conversion_v2;
    static constexpr bool default_renpho_peloton_conversion_v2 = false;

    static const QString ss2k_resistance_sample_1;
    static constexpr float default_ss2k_resistance_sample_1 = 20;

    static const QString ss2k_shift_step_sample_1;
    static constexpr float default_ss2k_shift_step_sample_1 = 0;

    static const QString ss2k_resistance_sample_2;
    static constexpr float default_ss2k_resistance_sample_2 = 30;

    static const QString ss2k_shift_step_sample_2;
    static constexpr float default_ss2k_shift_step_sample_2 = 0;

    static const QString ss2k_resistance_sample_3;
    static constexpr float default_ss2k_resistance_sample_3 = 40;

    static const QString ss2k_shift_step_sample_3;
    static constexpr float default_ss2k_shift_step_sample_3 = 0;

    static const QString ss2k_resistance_sample_4;
    static constexpr float default_ss2k_resistance_sample_4 = 50;

    static const QString ss2k_shift_step_sample_4;
    static constexpr float default_ss2k_shift_step_sample_4 = 0;

    static const QString fitshow_truetimer;
    static constexpr bool default_fitshow_truetimer = false;

    static const QString elite_rizer_gain;
    static constexpr float default_elite_rizer_gain = 1.0;

    static const QString tile_ext_incline_enabled;
    static constexpr bool default_tile_ext_incline_enabled = false;

    static const QString tile_ext_incline_order;
    static constexpr int default_tile_ext_incline_order = 32;

    static const QString reebok_fr30_treadmill;
    static constexpr bool default_reebok_fr30_treadmill = false;

    static const QString horizon_treadmill_7_8;
    static constexpr bool default_horizon_treadmill_7_8 = false;

    /**
     *@brief The name of the profile for this settings file.
     */
    static const QString profile_name;
    static const QString default_profile_name;

    static const QString tile_cadence_color_enabled;
    static constexpr bool default_tile_cadence_color_enabled = false;

    static const QString tile_peloton_remaining_enabled;
    static constexpr bool default_tile_peloton_remaining_enabled = false;

    static const QString tile_peloton_remaining_order;
    static constexpr int default_tile_peloton_remaining_order = 22;

    static const QString tile_peloton_resistance_color_enabled;
    static constexpr bool default_tile_peloton_resistance_color_enabled = false;

    /**
     *@brief Enable the Wahoo Dircon device.
     */
    static const QString dircon_yes;
    static constexpr bool default_dircon_yes = true;

    static const QString dircon_server_base_port;
    static constexpr int default_dircon_server_base_port = 36866;

    static const QString ios_cache_heart_device;
    static constexpr bool default_ios_cache_heart_device = true;

    /**
     *@brief Count of the number of times the app has been opened.
     */
    static const QString app_opening;
    static constexpr int default_app_opening = 0;

    static const QString proformtdf4ip;
    static const QString default_proformtdf4ip;

    static const QString fitfiu_mc_v460;
    static constexpr bool default_fitfiu_mc_v460 = false;

    /**
     *@brief The mass of the bike in kilograms.
     */
    static const QString bike_weight;
    static constexpr float default_bike_weight = 0;

    static const QString kingsmith_encrypt_v2;
    static constexpr bool default_kingsmith_encrypt_v2 = false;

    static const QString proform_treadmill_9_0;
    static constexpr bool default_proform_treadmill_9_0 = false;

    static const QString proform_treadmill_1800i;
    static constexpr bool default_proform_treadmill_1800i = false;

    static const QString cadence_offset;
    static constexpr float default_cadence_offset = 0;

    static const QString cadence_gain;
    static constexpr float default_cadence_gain = 1;

    static const QString sp_ht_9600ie;
    static constexpr bool default_sp_ht_9600ie = false;

    /**
     * @brief Enable text to speech.
     */
    static const QString tts_enabled;
    static constexpr bool default_tts_enabled = false;

    static const QString tts_summary_sec;
    static constexpr int default_tts_summary_sec = 120;

    static const QString tts_act_speed;
    static constexpr bool default_tts_act_speed = false;

    static const QString tts_avg_speed;
    static constexpr bool default_tts_avg_speed = true;

    static const QString tts_max_speed;
    static constexpr bool default_tts_max_speed = false;

    static const QString tts_act_inclination;
    static constexpr bool default_tts_act_inclination = false;

    static const QString tts_act_cadence;
    static constexpr bool default_tts_act_cadence = false;

    static const QString tts_avg_cadence;
    static constexpr bool default_tts_avg_cadence = true;

    static const QString tts_max_cadence;
    static constexpr bool default_tts_max_cadence = false;

    static const QString tts_act_elevation;
    static constexpr bool default_tts_act_elevation = true;

    static const QString tts_act_calories;
    static constexpr bool default_tts_act_calories = true;

    static const QString tts_act_odometer;
    static constexpr bool default_tts_act_odometer = true;

    static const QString tts_act_pace;
    static constexpr bool default_tts_act_pace = false;

    static const QString tts_avg_pace;
    static constexpr bool default_tts_avg_pace = true;

    static const QString tts_max_pace;
    static constexpr bool default_tts_max_pace = false;

    static const QString tts_act_resistance;
    static constexpr bool default_tts_act_resistance = true;

    static const QString tts_avg_resistance;
    static constexpr bool default_tts_avg_resistance = true;

    static const QString tts_max_resistance;
    static constexpr bool default_tts_max_resistance = false;

    static const QString tts_act_watt;
    static constexpr bool default_tts_act_watt = false;

    static const QString tts_avg_watt;
    static constexpr bool default_tts_avg_watt = true;

    static const QString tts_max_watt;
    static constexpr bool default_tts_max_watt = true;

    static const QString tts_act_ftp;
    static constexpr bool default_tts_act_ftp = false;

    static const QString tts_avg_ftp;
    static constexpr bool default_tts_avg_ftp = true;

    static const QString tts_max_ftp;
    static constexpr bool default_tts_max_ftp = false;

    static const QString tts_act_heart;
    static constexpr bool default_tts_act_heart = true;

    static const QString tts_avg_heart;
    static constexpr bool default_tts_avg_heart = true;

    static const QString tts_max_heart;
    static constexpr bool default_tts_max_heart = false;

    static const QString tts_act_jouls;
    static constexpr bool default_tts_act_jouls = true;

    static const QString tts_act_elapsed;
    static constexpr bool default_tts_act_elapsed = true;

    static const QString tts_act_peloton_resistance;
    static constexpr bool default_tts_act_peloton_resistance = false;

    static const QString tts_avg_peloton_resistance;
    static constexpr bool default_tts_avg_peloton_resistance = false;

    static const QString tts_max_peloton_resistance;
    static constexpr bool default_tts_max_peloton_resistance = false;

    static const QString tts_act_target_peloton_resistance;
    static constexpr bool default_tts_act_target_peloton_resistance = true;

    static const QString tts_act_target_cadence;
    static constexpr bool default_tts_act_target_cadence = true;

    static const QString tts_act_target_power;
    static constexpr bool default_tts_act_target_power = true;

    static const QString tts_act_target_zone;
    static constexpr bool default_tts_act_target_zone = true;

    static const QString tts_act_target_speed;
    static constexpr bool default_tts_act_target_speed = true;

    static const QString tts_act_target_incline;
    static constexpr bool default_tts_act_target_incline = true;

    static const QString tts_act_watt_kg;
    static constexpr bool default_tts_act_watt_kg = false;

    static const QString tts_avg_watt_kg;
    static constexpr bool default_tts_avg_watt_kg = false;

    static const QString tts_max_watt_kg;
    static constexpr bool default_tts_max_watt_kg = false;

    /**
     *@brief Enable the fake device, emulating an elliptical trainer.
     */
    static const QString fakedevice_elliptical;
    static constexpr bool default_fakedevice_elliptical = false;

    static const QString nordictrack_2950_ip;
    static const QString default_nordictrack_2950_ip;

    static const QString tile_instantaneous_stride_length_enabled;
    static constexpr bool default_tile_instantaneous_stride_length_enabled = false;

    static const QString tile_instantaneous_stride_length_order;
    static constexpr int default_tile_instantaneous_stride_length_order = 32;

    static const QString tile_ground_contact_enabled;
    static constexpr bool default_tile_ground_contact_enabled = false;

    static const QString tile_ground_contact_order;
    static constexpr int default_tile_ground_contact_order = 33;

    static const QString tile_vertical_oscillation_enabled;
    static constexpr bool default_tile_vertical_oscillation_enabled = false;

    static const QString tile_vertical_oscillation_order;
    static constexpr int default_tile_vertical_oscillation_order = 34;

    /**
     *@brief The gender of the user.
     */
    static const QString sex;
    static const QString default_sex;

    static const QString maps_type;
    static const QString default_maps_type;

    static const QString ss2k_max_resistance;
    static constexpr float default_ss2k_max_resistance = 100;

    static const QString ss2k_min_resistance;
    static constexpr float default_ss2k_min_resistance = 0;

    static const QString proform_treadmill_se;
    static constexpr bool default_proform_treadmill_se = false;

    /**
     *@brief The IP address for the Proform Treadmill.
     */
    static const QString proformtreadmillip;
    static const QString default_proformtreadmillip;
    // from version 2.11.22
    /**
     *@brief
     */
    static const QString kingsmith_encrypt_v3;
    static constexpr bool default_kingsmith_encrypt_v3 = false;

    /**
     *@brief IP address for the TDF 10.
     */
    static const QString tdf_10_ip;
    static const QString default_tdf_10_ip;

    /**
     *@brief
     */
    static const QString fakedevice_treadmill;
    static constexpr bool default_fakedevice_treadmill = false;

    /**
     *@brief The number of seconds to add to the video timestamp.
     */
    static const QString video_playback_window_s;
    static constexpr int default_video_playback_window_s = 12;

    static const QString horizon_treadmill_profile_user1;
    static const QString default_horizon_treadmill_profile_user1;

    static const QString horizon_treadmill_profile_user2;
    static const QString default_horizon_treadmill_profile_user2;

    static const QString horizon_treadmill_profile_user3;
    static const QString default_horizon_treadmill_profile_user3;

    static const QString horizon_treadmill_profile_user4;
    static const QString default_horizon_treadmill_profile_user4;

    static const QString horizon_treadmill_profile_user5;
    static const QString default_horizon_treadmill_profile_user5;

    static const QString nordictrack_gx_2_7;
    static const bool default_nordictrack_gx_2_7 = false;

    static const QString rolling_resistance;
    static constexpr double default_rolling_resistance = 0.005;

    static const QString wahoo_rgt_dircon;
    static constexpr bool default_wahoo_rgt_dircon = false;

    static const QString wahoo_without_wheel_diameter;
    static constexpr bool default_wahoo_without_wheel_diameter = false;

    static const QString tts_description_enabled;
    static constexpr bool default_tts_description_enabled = true;

    static const QString tile_preset_resistance_1_enabled;
    static constexpr bool default_tile_preset_resistance_1_enabled = false;

    static const QString tile_preset_resistance_1_order;
    static constexpr int default_tile_preset_resistance_1_order = 33;

    static const QString tile_preset_resistance_1_value;
    static constexpr double default_tile_preset_resistance_1_value = 1;

    static const QString tile_preset_resistance_1_label;
    static const QString default_tile_preset_resistance_1_label;

    static const QString tile_preset_resistance_2_enabled;
    static constexpr bool default_tile_preset_resistance_2_enabled = false;

    static const QString tile_preset_resistance_2_order;
    static constexpr int default_tile_preset_resistance_2_order = 34;

    static const QString tile_preset_resistance_2_value;
    static constexpr double default_tile_preset_resistance_2_value = 10;

    static const QString tile_preset_resistance_2_label;
    static const QString default_tile_preset_resistance_2_label;

    static const QString tile_preset_resistance_3_enabled;
    static constexpr bool default_tile_preset_resistance_3_enabled = false;

    static const QString tile_preset_resistance_3_order;
    static constexpr int default_tile_preset_resistance_3_order = 35;

    static const QString tile_preset_resistance_3_value;
    static constexpr double default_tile_preset_resistance_3_value = 20;

    static const QString tile_preset_resistance_3_label;
    static const QString default_tile_preset_resistance_3_label;

    static const QString tile_preset_resistance_4_enabled;
    static constexpr bool default_tile_preset_resistance_4_enabled = false;

    static const QString tile_preset_resistance_4_order;
    static constexpr int default_tile_preset_resistance_4_order = 36;

    static const QString tile_preset_resistance_4_value;
    static constexpr double default_tile_preset_resistance_4_value = 25;

    static const QString tile_preset_resistance_4_label;
    static const QString default_tile_preset_resistance_4_label;

    static const QString tile_preset_resistance_5_enabled;
    static constexpr bool default_tile_preset_resistance_5_enabled = false;

    static const QString tile_preset_resistance_5_order;
    static constexpr int default_tile_preset_resistance_5_order = 37;

    static const QString tile_preset_resistance_5_value;
    static constexpr double default_tile_preset_resistance_5_value = 30;

    static const QString tile_preset_resistance_5_label;
    static const QString default_tile_preset_resistance_5_label;

    static const QString tile_preset_speed_1_enabled;
    static constexpr bool default_tile_preset_speed_1_enabled = false;

    static const QString tile_preset_speed_1_order;
    static constexpr int default_tile_preset_speed_1_order = 38;

    static const QString tile_preset_speed_1_value;
    static constexpr double default_tile_preset_speed_1_value = 5;

    static const QString tile_preset_speed_1_label;
    static const QString default_tile_preset_speed_1_label;

    static const QString tile_preset_speed_2_enabled;
    static constexpr bool default_tile_preset_speed_2_enabled = false;

    static const QString tile_preset_speed_2_order;
    static constexpr int default_tile_preset_speed_2_order = 39;

    static const QString tile_preset_speed_2_value;
    static constexpr double default_tile_preset_speed_2_value = 7;

    static const QString tile_preset_speed_2_label;
    static const QString default_tile_preset_speed_2_label;

    static const QString tile_preset_speed_3_enabled;
    static constexpr bool default_tile_preset_speed_3_enabled = false;

    static const QString tile_preset_speed_3_order;
    static constexpr int default_tile_preset_speed_3_order = 40;

    static const QString tile_preset_speed_3_value;
    static constexpr double default_tile_preset_speed_3_value = 10;

    static const QString tile_preset_speed_3_label;
    static const QString default_tile_preset_speed_3_label;

    static const QString tile_preset_speed_4_enabled;
    static constexpr bool default_tile_preset_speed_4_enabled = false;

    static const QString tile_preset_speed_4_order;
    static constexpr int default_tile_preset_speed_4_order = 41;

    static const QString tile_preset_speed_4_value;
    static constexpr double default_tile_preset_speed_4_value = 11;

    static const QString tile_preset_speed_4_label;
    static const QString default_tile_preset_speed_4_label;

    static const QString tile_preset_speed_5_enabled;
    static constexpr bool default_tile_preset_speed_5_enabled = false;

    static const QString tile_preset_speed_5_order;
    static constexpr int default_tile_preset_speed_5_order = 42;

    static const QString tile_preset_speed_5_value;
    static constexpr double default_tile_preset_speed_5_value = 12;

    static const QString tile_preset_speed_5_label;
    static const QString default_tile_preset_speed_5_label;

    static const QString tile_preset_inclination_1_enabled;
    static constexpr bool default_tile_preset_inclination_1_enabled = false;

    static const QString tile_preset_inclination_1_order;
    static constexpr int default_tile_preset_inclination_1_order = 43;

    static const QString tile_preset_inclination_1_value;
    static constexpr double default_tile_preset_inclination_1_value = 0;

    static const QString tile_preset_inclination_1_label;
    static const QString default_tile_preset_inclination_1_label;

    static const QString tile_preset_inclination_2_enabled;
    static constexpr bool default_tile_preset_inclination_2_enabled = false;

    static const QString tile_preset_inclination_2_order;
    static constexpr int default_tile_preset_inclination_2_order = 44;

    static const QString tile_preset_inclination_2_value;
    static constexpr double default_tile_preset_inclination_2_value = 1;

    static const QString tile_preset_inclination_2_label;
    static const QString default_tile_preset_inclination_2_label;

    static const QString tile_preset_inclination_3_enabled;
    static constexpr bool default_tile_preset_inclination_3_enabled = false;

    static const QString tile_preset_inclination_3_order;
    static constexpr int default_tile_preset_inclination_3_order = 45;

    static const QString tile_preset_inclination_3_value;
    static constexpr double default_tile_preset_inclination_3_value = 2;

    static const QString tile_preset_inclination_3_label;
    static const QString default_tile_preset_inclination_3_label;

    static const QString tile_preset_inclination_4_enabled;
    static constexpr bool default_tile_preset_inclination_4_enabled = false;

    static const QString tile_preset_inclination_4_order;
    static constexpr int default_tile_preset_inclination_4_order = 46;

    static const QString tile_preset_inclination_4_value;
    static constexpr double default_tile_preset_inclination_4_value = 3;

    static const QString tile_preset_inclination_4_label;
    static const QString default_tile_preset_inclination_4_label;

    static const QString tile_preset_inclination_5_enabled;
    static constexpr bool default_tile_preset_inclination_5_enabled = false;

    static const QString tile_preset_inclination_5_order;
    static constexpr int default_tile_preset_inclination_5_order = 47;

    static const QString tile_preset_inclination_5_value;
    static constexpr double default_tile_preset_inclination_5_value = 4;

    static const QString tile_preset_inclination_5_label;
    static const QString default_tile_preset_inclination_5_label;

    static const QString tile_preset_resistance_1_color;
    static const QString default_tile_preset_resistance_1_color;

    static const QString tile_preset_resistance_2_color;
    static const QString default_tile_preset_resistance_2_color;

    static const QString tile_preset_resistance_3_color;
    static const QString default_tile_preset_resistance_3_color;

    static const QString tile_preset_resistance_4_color;
    static const QString default_tile_preset_resistance_4_color;

    static const QString tile_preset_resistance_5_color;
    static const QString default_tile_preset_resistance_5_color;

    static const QString tile_preset_speed_1_color;
    static const QString default_tile_preset_speed_1_color;

    static const QString tile_preset_speed_2_color;
    static const QString default_tile_preset_speed_2_color;

    static const QString tile_preset_speed_3_color;
    static const QString default_tile_preset_speed_3_color;

    static const QString tile_preset_speed_4_color;
    static const QString default_tile_preset_speed_4_color;

    static const QString tile_preset_speed_5_color;
    static const QString default_tile_preset_speed_5_color;

    static const QString tile_preset_inclination_1_color;
    static const QString default_tile_preset_inclination_1_color;

    static const QString tile_preset_inclination_2_color;
    static const QString default_tile_preset_inclination_2_color;

    static const QString tile_preset_inclination_3_color;
    static const QString default_tile_preset_inclination_3_color;

    static const QString tile_preset_inclination_4_color;
    static const QString default_tile_preset_inclination_4_color;

    static const QString tile_preset_inclination_5_color;
    static const QString default_tile_preset_inclination_5_color;

    static const QString tile_avg_watt_lap_enabled;
    static constexpr bool default_tile_avg_watt_lap_enabled = false;

    static const QString tile_avg_watt_lap_order;
    static constexpr int default_tile_avg_watt_lap_order = 48;

    static const QString nordictrack_t70_treadmill;
    static constexpr bool default_nordictrack_t70_treadmill = false;

    static const QString CRRGain;
    static constexpr double default_CRRGain = 0;

    static const QString CWGain;
    static constexpr double default_CWGain = 0;

    static const QString proform_treadmill_cadence_lt;
    static constexpr bool default_proform_treadmill_cadence_lt = false;

    static const QString trainprogram_stop_at_end;
    static constexpr bool default_trainprogram_stop_at_end = false;

    static const QString domyos_elliptical_inclination;
    static constexpr bool default_domyos_elliptical_inclination = true;

    static const QString gpx_loop;
    static constexpr bool default_gpx_loop = false;

    static const QString android_notification;
    static constexpr bool default_android_notification = false;

    static const QString kingsmith_encrypt_v4;
    static constexpr bool default_kingsmith_encrypt_v4 = false;

    static const QString horizon_treadmill_disable_pause;
    static constexpr bool default_horizon_treadmill_disable_pause = false;

    static const QString domyos_bike_500_profile_v1;
    static constexpr bool default_domyos_bike_500_profile_v1 = false;

    static const QString ss2k_peloton;
    static constexpr bool default_ss2k_peloton = false;

    static const QString computrainer_serialport;
    static const QString default_computrainer_serialport;

    static const QString strava_virtual_activity;
    static constexpr bool default_strava_virtual_activity = true;

    static const QString powr_sensor_running_cadence_half_on_strava;
    static constexpr bool default_powr_sensor_running_cadence_half_on_strava = false;

    static const QString nordictrack_ifit_adb_remote;
    static constexpr bool default_nordictrack_ifit_adb_remote = false;

    static const QString floating_width;
    static constexpr int default_floating_width = 370;

    static const QString floating_height;
    static constexpr int default_floating_height = 210;

    static const QString floating_transparency;
    static constexpr int default_floating_transparency = 80;

    static const QString floating_startup;
    static constexpr int default_floating_startup = false;

    static const QString norditrack_s25i_treadmill;
    static constexpr int default_norditrack_s25i_treadmill = false;

    static const QString toorx_ftms_treadmill;
    static constexpr bool default_toorx_ftms_treadmill = false;

    static const QString nordictrack_t65s_83_treadmill;
    static constexpr bool default_nordictrack_t65s_83_treadmill = false;

    static const QString horizon_treadmill_suspend_stats_pause;
    static constexpr bool default_horizon_treadmill_suspend_stats_pause = false;

    static const QString sportstech_sx600;
    static constexpr bool default_sportstech_sx600 = false;

    static const QString sole_elliptical_inclination;
    static constexpr bool default_sole_elliptical_inclination = false;

    static const QString proform_hybrid_trainer_xt;
    static constexpr bool default_proform_hybrid_trainer_xt = false;

    static const QString gears_restore_value;
    static constexpr bool default_gears_restore_value = false;

    static const QString gears_current_value;
    static constexpr double default_gears_current_value = 0;

    static const QString tile_pace_last500m_enabled;
    static constexpr bool default_tile_pace_last500m_enabled = true;

    static const QString tile_pace_last500m_order;
    static constexpr int default_tile_pace_last500m_order = 49;

    static const QString treadmill_difficulty_gain_or_offset;
    static constexpr bool default_treadmill_difficulty_gain_or_offset = false;

    static const QString pafers_treadmill_bh_iboxster_plus;
    static constexpr bool default_pafers_treadmill_bh_iboxster_plus = false;

    static const QString proform_cycle_trainer_400;
    static constexpr bool default_proform_cycle_trainer_400 = false;

    static const QString peloton_workout_ocr;
    static constexpr bool default_peloton_workout_ocr = false;

    static const QString peloton_bike_ocr;
    static constexpr bool default_peloton_bike_ocr = false;
    static const QString fitshow_treadmill_miles;
    static constexpr bool default_fitshow_treadmill_miles = false;

    static const QString proform_hybrid_trainer_PFEL03815;
    static constexpr bool default_proform_hybrid_trainer_PFEL03815 = false;

    static const QString schwinn_resistance_smooth;
    static constexpr int default_schwinn_resistance_smooth = 0;

    static const QString treadmill_inclination_override_0;
    static constexpr double default_treadmill_inclination_override_0 = 0;
    static const QString treadmill_inclination_override_05;
    static constexpr double default_treadmill_inclination_override_05 = 0.5;
    static const QString treadmill_inclination_override_10;
    static constexpr double default_treadmill_inclination_override_10 = 1.0;
    static const QString treadmill_inclination_override_15;
    static constexpr double default_treadmill_inclination_override_15 = 1.5;
    static const QString treadmill_inclination_override_20;
    static constexpr double default_treadmill_inclination_override_20 = 2.0;
    static const QString treadmill_inclination_override_25;
    static constexpr double default_treadmill_inclination_override_25 = 2.5;
    static const QString treadmill_inclination_override_30;
    static constexpr double default_treadmill_inclination_override_30 = 3.0;
    static const QString treadmill_inclination_override_35;
    static constexpr double default_treadmill_inclination_override_35 = 3.5;
    static const QString treadmill_inclination_override_40;
    static constexpr double default_treadmill_inclination_override_40 = 4.0;
    static const QString treadmill_inclination_override_45;
    static constexpr double default_treadmill_inclination_override_45 = 4.5;
    static const QString treadmill_inclination_override_50;
    static constexpr double default_treadmill_inclination_override_50 = 5.0;
    static const QString treadmill_inclination_override_55;
    static constexpr double default_treadmill_inclination_override_55 = 5.5;
    static const QString treadmill_inclination_override_60;
    static constexpr double default_treadmill_inclination_override_60 = 6.0;
    static const QString treadmill_inclination_override_65;
    static constexpr double default_treadmill_inclination_override_65 = 6.5;
    static const QString treadmill_inclination_override_70;
    static constexpr double default_treadmill_inclination_override_70 = 7.0;
    static const QString treadmill_inclination_override_75;
    static constexpr double default_treadmill_inclination_override_75 = 7.5;
    static const QString treadmill_inclination_override_80;
    static constexpr double default_treadmill_inclination_override_80 = 8.0;
    static const QString treadmill_inclination_override_85;
    static constexpr double default_treadmill_inclination_override_85 = 8.5;
    static const QString treadmill_inclination_override_90;
    static constexpr double default_treadmill_inclination_override_90 = 9.0;
    static const QString treadmill_inclination_override_95;
    static constexpr double default_treadmill_inclination_override_95 = 9.5;
    static const QString treadmill_inclination_override_100;
    static constexpr double default_treadmill_inclination_override_100 = 10.0;
    static const QString treadmill_inclination_override_105;
    static constexpr double default_treadmill_inclination_override_105 = 10.5;
    static const QString treadmill_inclination_override_110;
    static constexpr double default_treadmill_inclination_override_110 = 11.0;
    static const QString treadmill_inclination_override_115;
    static constexpr double default_treadmill_inclination_override_115 = 11.5;
    static const QString treadmill_inclination_override_120;
    static constexpr double default_treadmill_inclination_override_120 = 12.0;
    static const QString treadmill_inclination_override_125;
    static constexpr double default_treadmill_inclination_override_125 = 12.5;
    static const QString treadmill_inclination_override_130;
    static constexpr double default_treadmill_inclination_override_130 = 13.0;
    static const QString treadmill_inclination_override_135;
    static constexpr double default_treadmill_inclination_override_135 = 13.5;
    static const QString treadmill_inclination_override_140;
    static constexpr double default_treadmill_inclination_override_140 = 14.0;
    static const QString treadmill_inclination_override_145;
    static constexpr double default_treadmill_inclination_override_145 = 14.5;
    static const QString treadmill_inclination_override_150;
    static constexpr double default_treadmill_inclination_override_150 = 15.0;

    static const QString sole_elliptical_e55;
    static constexpr bool default_sole_elliptical_e55 = false;

    static const QString horizon_treadmill_force_ftms;
    static constexpr bool default_horizon_treadmill_force_ftms = false;

    static const QString horizon_treadmill_7_0_at_24;
    static constexpr bool default_horizon_treadmill_7_0_at_24 = false;

    static const QString treadmill_pid_heart_min;
    static constexpr int default_treadmill_pid_heart_min = 0;

    static const QString treadmill_pid_heart_max;
    static constexpr int default_treadmill_pid_heart_max = 0;

    static const QString nordictrack_elliptical_c7_5;
    static constexpr bool default_nordictrack_elliptical_c7_5 = false;

    static const QString renpho_bike_double_resistance;
    static constexpr bool default_renpho_bike_double_resistance = false;

    static const QString nordictrack_incline_trainer_x7i;
    static constexpr bool default_nordictrack_incline_trainer_x7i = false;

    static const QString strava_auth_external_webbrowser;
    static constexpr bool default_strava_auth_external_webbrowser = false;

    static const QString gears_from_bike;
    static constexpr bool default_gears_from_bike = false;

    static const QString peloton_spinups_autoresistance;
    static constexpr bool default_peloton_spinups_autoresistance = true;

    static const QString eslinker_costaway;
    static constexpr bool default_eslinker_costaway = false;

    static const QString treadmill_inclination_ovveride_gain;
    static constexpr double default_treadmill_inclination_ovveride_gain = 1.0;

    static const QString treadmill_inclination_ovveride_offset;
    static constexpr double default_treadmill_inclination_ovveride_offset = 0.0;

    static const QString bh_spada_2_watt;
    static constexpr bool default_bh_spada_2_watt = false;

    static const QString tacx_neo2_peloton;
    static constexpr bool default_tacx_neo2_peloton = false;

    static const QString sole_treadmill_inclination_fast;
    static constexpr bool default_sole_treadmill_inclination_fast = false;

    static const QString zwift_ocr;
    static constexpr bool default_zwift_ocr = false;

    static const QString fit_file_saved_on_quit;
    static constexpr bool default_fit_file_saved_on_quit = false;

    static const QString gem_module_inclination;
    static constexpr bool default_gem_module_inclination = false;

    static const QString treadmill_simulate_inclination_with_speed;
    static constexpr bool default_treadmill_simulate_inclination_with_speed = false;

    static const QString garmin_companion;
    static constexpr bool default_garmin_companion = false;

    static const QString peloton_companion_workout_ocr;
    static constexpr bool default_companion_peloton_workout_ocr = false;

    static const QString iconcept_elliptical;
    static constexpr bool default_iconcept_elliptical = false;

    static const QString gears_gain;
    static constexpr double default_gears_gain = 1.0;

    static const QString proform_treadmill_8_0;
    static constexpr bool default_proform_treadmill_8_0 = false;

    static const QString zero_zt2500_treadmill;
    static constexpr bool default_zero_zt2500_treadmill = false;

    static const QString kingsmith_encrypt_v5;
    static constexpr bool default_kingsmith_encrypt_v5 = false;

    static const QString peloton_rower_level;
    static constexpr int default_peloton_rower_level = 1;

    static const QString tile_target_pace_enabled;
    static constexpr bool default_tile_target_pace_enabled = false;

    static const QString tile_target_pace_order;
    static constexpr int default_tile_target_pace_order = 50;

    static const QString tts_act_target_pace;
    static constexpr bool default_tts_act_target_pace = false;

    static const QString csafe_rower;
    static const QString default_csafe_rower;

    static const QString csafe_elliptical_port;
    static const QString default_csafe_elliptical_port;

    static const QString ftms_rower;
    static const QString default_ftms_rower;

    static const QString ftms_elliptical;
    static const QString default_ftms_elliptical;

    static const QString zwift_workout_ocr;
    static constexpr bool default_zwift_workout_ocr = false;

    static const QString proform_bike_sb;
    static constexpr bool default_proform_bike_sb = false;

    static const QString fakedevice_rower;
    static constexpr bool default_fakedevice_rower = false;

    static const QString zwift_ocr_climb_portal;
    static constexpr bool default_zwift_ocr_climb_portal = false;

    static const QString poll_device_time;
    static constexpr int default_poll_device_time = 200;

    static const QString proform_bike_PFEVEX71316_1;
    static constexpr bool default_proform_bike_PFEVEX71316_1 = false;

    static const QString schwinn_bike_resistance_v3;
    static constexpr bool default_schwinn_bike_resistance_v3 = false;

    static const QString watt_ignore_builtin;
    static constexpr bool default_watt_ignore_builtin = true;

    static const QString proform_treadmill_z1300i;
    static constexpr bool default_proform_treadmill_z1300i = false;

    static const QString ftms_bike;
    static const QString default_ftms_bike;

    static const QString ftms_treadmill;
    static const QString default_ftms_treadmill;

    static const QString proform_rower_sport_rl;
    static constexpr bool default_proform_rower_sport_rl = false;
    static const QString proform_rower_750r;
    static constexpr bool default_proform_rower_750r = false;

    static const QString strava_date_prefix;
    static constexpr bool default_strava_date_prefix = false;

    /**
     * @brief Adjusts value in a metric object that's configured specifically for measuring SPEED on ANT+.
     */
    static const QString ant_speed_offset;
    static constexpr float default_ant_speed_offset = 0;

    /**
     * @brief Adjusts value in a metric object that's configured specifically for measuring SPEED on ANT+.
     */
    static const QString ant_speed_gain;
    static constexpr float default_ant_speed_gain = 1;

    static const QString race_mode;
    static constexpr bool default_race_mode = false;

    static const QString proform_pro_1000_treadmill;
    static constexpr bool default_proform_pro_1000_treadmill = false;

    static const QString saris_trainer;
    static constexpr bool default_saris_trainer = false;

    static const QString proform_studio_NTEX71021;
    static constexpr bool default_proform_studio_NTEX71021 = false;

    static const QString nordictrack_x22i;
    static constexpr bool default_nordictrack_x22i = false;

    static const QString iconsole_elliptical;
    static constexpr bool default_iconsole_elliptical = false;    

    static const QString autolap_distance;
    static constexpr float default_autolap_distance = 0;

    static const QString nordictrack_s20_treadmill;
    static constexpr bool default_nordictrack_s20_treadmill = false;    

    static const QString freemotion_coachbike_b22_7;
    static constexpr bool default_freemotion_coachbike_b22_7 = false;

    static const QString proform_cycle_trainer_300_ci;
    static constexpr bool default_proform_cycle_trainer_300_ci = false;

    static const QString kingsmith_encrypt_g1_walking_pad;
    static constexpr bool default_kingsmith_encrypt_g1_walking_pad = false;

    static const QString proform_bike_225_csx;
    static constexpr bool default_proform_bike_225_csx = false;

    static const QString proform_treadmill_l6_0s;
    static constexpr bool default_proform_treadmill_l6_0s = false;

    static const QString proformtdf1ip;
    static const QString default_proformtdf1ip;

    static const QString zwift_username;
    static const QString default_zwift_username;

    static const QString zwift_password;
    static const QString default_zwift_password;

    static const QString garmin_bluetooth_compatibility;
    static constexpr bool default_garmin_bluetooth_compatibility = false;

    static const QString norditrack_s25_treadmill;
    static constexpr int default_norditrack_s25_treadmill = false;

    static const QString proform_8_5_treadmill;
    static constexpr int default_proform_8_5_treadmill = false;

    static const QString treadmill_incline_min;
    static constexpr float default_treadmill_incline_min = -100.0;

    static const QString treadmill_incline_max;
    static constexpr float default_treadmill_incline_max = 100.0;

    static const QString treadmill_speed_max;
    static constexpr float default_treadmill_speed_max = 100.0;

    static const QString proform_2000_treadmill;
    static constexpr bool default_proform_2000_treadmill = false;

    static const QString android_documents_folder;
    static constexpr bool default_android_documents_folder = false;

    static const QString zwift_api_autoinclination;
    static constexpr bool default_zwift_api_autoinclination = true;

    static const QString domyos_treadmill_button_5kmh;
    static constexpr float default_domyos_treadmill_button_5kmh = 5.0;

    static const QString domyos_treadmill_button_10kmh;
    static constexpr float default_domyos_treadmill_button_10kmh = 10.0;

    static const QString domyos_treadmill_button_16kmh;
    static constexpr float default_domyos_treadmill_button_16kmh = 16.0;

    static const QString domyos_treadmill_button_22kmh;
    static constexpr float default_domyos_treadmill_button_22kmh = 22.0;

    static const QString proform_treadmill_sport_8_5;
    static constexpr bool default_proform_treadmill_sport_8_5 = false;

    static const QString domyos_treadmill_t900a;
    static constexpr bool default_domyos_treadmill_t900a = false;

    static const QString enerfit_SPX_9500;
    static constexpr bool default_enerfit_SPX_9500 = false;

    static const QString proform_treadmill_505_cst;
    static constexpr bool default_proform_treadmill_505_cst = false;

    static const QString nordictrack_treadmill_t8_5s;
    static constexpr bool default_nordictrack_treadmill_t8_5s = false;
    
    static const QString zwift_click;
    static constexpr bool default_zwift_click = false;

    static const QString proform_treadmill_705_cst;
    static constexpr bool default_proform_treadmill_705_cst = false;

    static const QString hop_sport_hs_090h_bike;
    static constexpr bool default_hop_sport_hs_090h_bike = false;

    static const QString zwift_play;
    static constexpr bool default_zwift_play = false;

    static const QString zwift_play_vibration;
    static constexpr bool default_zwift_play_vibration = true;

    static const QString nordictrack_treadmill_x14i;
    static constexpr bool default_nordictrack_treadmill_x14i = false;

    static const QString zwift_api_poll;
    static constexpr int default_zwift_api_poll = 5;

    static const QString tile_step_count_enabled;
    static constexpr bool default_tile_step_count_enabled = false;

    static const QString tile_step_count_order;
    static constexpr int default_tile_step_count_order = 51;

    static const QString tile_erg_mode_enabled;
    static constexpr bool default_tile_erg_mode_enabled = false;

    static const QString tile_erg_mode_order;
    static constexpr int default_tile_erg_mode_order = 52;

    static const QString toorx_srx_3500;
    static constexpr bool default_toorx_srx_3500 = false;

    static const QString stryd_speed_instead_treadmill;
    static constexpr bool default_stryd_speed_instead_treadmill = false;
    static const QString inclination_delay_seconds;
    static constexpr float default_inclination_delay_seconds = 0.0;

    static const QString ergDataPoints;
    static const QString default_ergDataPoints;

    static const QString proform_carbon_tl;
    static constexpr bool default_proform_carbon_tl = false;    

    static const QString proform_proshox2;
    static constexpr bool default_proform_proshox2 = false;    

    static const QString proform_tdf_10_0;
    static constexpr bool default_proform_tdf_10_0 = false;

    static const QString nordictrack_GX4_5_bike;
    static constexpr bool default_nordictrack_GX4_5_bike = false;

    static const QString ftp_run;
    static constexpr float default_ftp_run = 200.0;

    static const QString tile_rss_enabled;
    static constexpr bool default_tile_rss_enabled = false;

    static const QString tile_rss_order;
    static constexpr int default_tile_rss_order = 53;

    static const QString treadmillDataPoints;
    static const QString default_treadmillDataPoints;

    static const QString nordictrack_s20i_treadmill;
    static constexpr bool default_nordictrack_s20i_treadmill = false;

    static const QString proform_595i_proshox2;
    static constexpr bool default_proform_595i_proshox2 = false;

    static const QString proform_treadmill_8_7;
    static constexpr bool default_proform_treadmill_8_7 = false;

    static const QString proform_bike_325_csx;
    static constexpr bool default_proform_bike_325_csx = false;

    static const QString strava_upload_mode;
    static const QString default_strava_upload_mode;

    static const QString proform_treadmill_705_cst_V78_239;
    static constexpr bool default_proform_treadmill_705_cst_V78_239 = false;

    static const QString stryd_add_inclination_gain;
    static constexpr bool default_stryd_add_inclination_gain = false;

    static const QString toorx_bike_srx_500;
    static constexpr bool default_toorx_bike_srx_500 = false;

    static const QString toorxtreadmill_discovery_completed;
    static constexpr bool default_toorxtreadmill_discovery_completed = false;

    static const QString atletica_lightspeed_treadmill;
    static constexpr bool default_atletica_lightspeed_treadmill = false;

    static const QString peloton_treadmill_level;
    static constexpr int default_peloton_treadmill_level = 1;

    static const QString peloton_treadmill_walk_level;
    static constexpr int default_peloton_treadmill_walk_level = 1;

    static const QString nordictrackadbbike_resistance;
    static constexpr bool default_nordictrackadbbike_resistance = false;

    static const QString proform_treadmill_carbon_t7;
    static constexpr bool default_proform_treadmill_carbon_t7 = false;

    static const QString nordictrack_treadmill_exp_5i;
    static constexpr bool default_nordictrack_treadmill_exp_5i = false;

    static const QString dircon_id;
    static constexpr int default_dircon_id = 0;

    static const QString proform_elliptical_ip;
    static const QString default_proform_elliptical_ip;

    static const QString proform_rower_ip;
    static const QString default_proform_rower_ip;


    static const QString antbike;
    static constexpr bool default_antbike = false;

    static const QString domyosbike_notfmts;
    static constexpr bool default_domyosbike_notfmts = false;

    static const QString gears_volume_debouncing;
    static constexpr bool default_gears_volume_debouncing = false;

    static const QString tile_biggears_enabled;
    static constexpr bool default_tile_biggears_enabled = false;

    static const QString tile_biggears_order;
    static constexpr int default_tile_biggears_order = 54;

    static const QString domyostreadmill_notfmts;
    static constexpr bool default_domyostreadmill_notfmts = false;

    static const QString zwiftplay_swap;
    static constexpr bool default_zwiftplay_swap = false;

    static const QString gears_zwift_ratio;
    static constexpr bool default_gears_zwift_ratio = false;

    static const QString domyos_bike_500_profile_v2;
    static constexpr bool default_domyos_bike_500_profile_v2 = false;

    static const QString gears_offset;
    static constexpr double default_gears_offset = 0.0;

    static const QString peloton_accesstoken;
    static const QString default_peloton_accesstoken;

    static const QString peloton_refreshtoken;
    static const QString default_peloton_refreshtoken;

    static const QString peloton_lastrefresh;
    static const QString default_peloton_lastrefresh;

    static const QString peloton_expires;
    static const QString default_peloton_expires;

    static const QString peloton_code;
    static const QString default_peloton_code;
    
    static const QString proform_carbon_tl_PFTL59720;
    static constexpr bool default_proform_carbon_tl_PFTL59720 = false;    

    static const QString proform_treadmill_sport_70;
    static constexpr bool default_proform_treadmill_sport_70 = false;

    static const QString peloton_date_format;
    static const QString default_peloton_date_format;

    static const QString force_resistance_instead_inclination;
    static constexpr bool default_force_resistance_instead_inclination = false;

    static const QString proform_treadmill_575i;
    static constexpr bool default_proform_treadmill_575i = false;

    static const QString zwift_play_emulator;
    static constexpr bool default_zwift_play_emulator = false;

    static const QString gear_configuration;
    static const QString default_gear_configuration;

    static const QString gear_crankset_size;
    static constexpr int default_gear_crankset_size = 42;

    static const QString gear_cog_size;
    static constexpr int default_gear_cog_size = 14;

    static const QString gear_wheel_size;
    static const QString default_gear_wheel_size;

    static const QString gear_circumference;
    static constexpr double default_gear_circumference = 2070.0;

    static const QString watt_bike_emulator;
    static constexpr bool default_watt_bike_emulator = false;

    static const QString restore_specific_gear;
    static constexpr bool default_restore_specific_gear = false;

    static const QString skipLocationServicesDialog;
    static constexpr bool default_skipLocationServicesDialog = false;

    static const QString trainprogram_pid_pushy;
    static constexpr bool default_trainprogram_pid_pushy = true;

    static const QString min_inclination;
    static constexpr double default_min_inclination = -999.0;

    static const QString proform_performance_300i;
    static constexpr bool default_proform_performance_300i = false;

    static const QString proform_performance_400i;
    static constexpr bool default_proform_performance_400i = false;

    static const QString proform_treadmill_c700;
    static constexpr bool default_proform_treadmill_c700 = false;

    static const QString sram_axs_controller;
    static constexpr bool default_sram_axs_controller = false;

    static const QString proform_treadmill_c960i;
    static constexpr bool default_proform_treadmill_c960i = false;

    static const QString mqtt_host;
    static const QString default_mqtt_host;

    static const QString mqtt_port;
    static constexpr int default_mqtt_port = 1883;

    static const QString mqtt_username;
    static const QString default_mqtt_username;

    static const QString mqtt_password;
    static const QString default_mqtt_password;

    static const QString mqtt_deviceid;
    static const QString default_mqtt_deviceid;

    static const QString peloton_auto_start_with_intro;
    static constexpr bool default_peloton_auto_start_with_intro = false;

    static const QString peloton_auto_start_without_intro;
    static constexpr bool default_peloton_auto_start_without_intro = false;

    static const QString nordictrack_tseries5_treadmill;
    static constexpr bool default_nordictrack_tseries5_treadmill = false;

    static const QString proform_carbon_tl_PFTL59722c;
    static constexpr bool default_proform_carbon_tl_PFTL59722c = false;

    static const QString nordictrack_gx_44_pro;
    static constexpr bool default_nordictrack_gx_44_pro = false;

    static const QString OSC_ip;
    static const QString default_OSC_ip;

    static const QString OSC_port;
    static constexpr int default_OSC_port = 9000;

    static const QString strava_treadmill;
    static constexpr bool default_strava_treadmill = true;

    static const QString iconsole_rower;
    static constexpr bool default_iconsole_rower = false;

    static const QString proform_treadmill_1500_pro;
    static constexpr bool default_proform_treadmill_1500_pro = false;

    static const QString proform_505_cst_80_44;
    static constexpr bool default_proform_505_cst_80_44 = false;

    static const QString proform_trainer_8_0;
    static constexpr bool default_proform_trainer_8_0 = false;

    static const QString tile_biggears_swap;
    static constexpr bool default_tile_biggears_swap = false;

    static const QString treadmill_follow_wattage;
    static constexpr bool default_treadmill_follow_wattage = false;

    static const QString fit_file_garmin_device_training_effect;
    static constexpr bool default_fit_file_garmin_device_training_effect = false;

    static const QString fit_file_garmin_device_training_effect_device;
    static constexpr int default_fit_file_garmin_device_training_effect_device = FIT_GARMIN_PRODUCT_EDGE_830;

    static const QString proform_treadmill_705_cst_V80_44;
    static constexpr bool default_proform_treadmill_705_cst_V80_44 = false;

    static const QString nordictrack_treadmill_1750_adb;
    static constexpr bool default_nordictrack_treadmill_1750_adb = false;

    static const QString tile_preset_powerzone_1_enabled;
    static constexpr bool default_tile_preset_powerzone_1_enabled = false;

    static const QString tile_preset_powerzone_1_order;
    static constexpr int default_tile_preset_powerzone_1_order = 55;

    static const QString tile_preset_powerzone_1_value;
    static constexpr double default_tile_preset_powerzone_1_value = 1.0;

    static const QString tile_preset_powerzone_1_label;
    static const QString default_tile_preset_powerzone_1_label;

    static const QString tile_preset_powerzone_1_color;
    static const QString default_tile_preset_powerzone_1_color;

    static const QString tile_preset_powerzone_2_enabled;
    static constexpr bool default_tile_preset_powerzone_2_enabled = false;

    static const QString tile_preset_powerzone_2_order;
    static constexpr int default_tile_preset_powerzone_2_order = 56;

    static const QString tile_preset_powerzone_2_value;
    static constexpr double default_tile_preset_powerzone_2_value = 2.0;

    static const QString tile_preset_powerzone_2_label;
    static const QString default_tile_preset_powerzone_2_label;

    static const QString tile_preset_powerzone_2_color;
    static const QString default_tile_preset_powerzone_2_color;

    static const QString tile_preset_powerzone_3_enabled;
    static constexpr bool default_tile_preset_powerzone_3_enabled = false;

    static const QString tile_preset_powerzone_3_order;
    static constexpr int default_tile_preset_powerzone_3_order = 57;

    static const QString tile_preset_powerzone_3_value;
    static constexpr double default_tile_preset_powerzone_3_value = 3.0;

    static const QString tile_preset_powerzone_3_label;
    static const QString default_tile_preset_powerzone_3_label;

    static const QString tile_preset_powerzone_3_color;
    static const QString default_tile_preset_powerzone_3_color;

    static const QString tile_preset_powerzone_4_enabled;
    static constexpr bool default_tile_preset_powerzone_4_enabled = false;

    static const QString tile_preset_powerzone_4_order;
    static constexpr int default_tile_preset_powerzone_4_order = 58;

    static const QString tile_preset_powerzone_4_value;
    static constexpr double default_tile_preset_powerzone_4_value = 4.0;

    static const QString tile_preset_powerzone_4_label;
    static const QString default_tile_preset_powerzone_4_label;

    static const QString tile_preset_powerzone_4_color;
    static const QString default_tile_preset_powerzone_4_color;

    static const QString tile_preset_powerzone_5_enabled;
    static constexpr bool default_tile_preset_powerzone_5_enabled = false;

    static const QString tile_preset_powerzone_5_order;
    static constexpr int default_tile_preset_powerzone_5_order = 59;

    static const QString tile_preset_powerzone_5_value;
    static constexpr double default_tile_preset_powerzone_5_value = 5.0;

    static const QString tile_preset_powerzone_5_label;
    static const QString default_tile_preset_powerzone_5_label;

    static const QString tile_preset_powerzone_5_color;
    static const QString default_tile_preset_powerzone_5_color;

    static const QString tile_preset_powerzone_6_enabled;
    static constexpr bool default_tile_preset_powerzone_6_enabled = false;

    static const QString tile_preset_powerzone_6_order;
    static constexpr int default_tile_preset_powerzone_6_order = 60;

    static const QString tile_preset_powerzone_6_value;
    static constexpr double default_tile_preset_powerzone_6_value = 6.0;

    static const QString tile_preset_powerzone_6_label;
    static const QString default_tile_preset_powerzone_6_label;

    static const QString tile_preset_powerzone_6_color;
    static const QString default_tile_preset_powerzone_6_color;

    static const QString tile_preset_powerzone_7_enabled;
    static constexpr bool default_tile_preset_powerzone_7_enabled = false;

    static const QString tile_preset_powerzone_7_order;
    static constexpr int default_tile_preset_powerzone_7_order = 61;

    static const QString tile_preset_powerzone_7_value;
    static constexpr double default_tile_preset_powerzone_7_value = 7.0;

    static const QString tile_preset_powerzone_7_label;
    static const QString default_tile_preset_powerzone_7_label;

    static const QString tile_preset_powerzone_7_color;
    static const QString default_tile_preset_powerzone_7_color;    

    static const QString proform_bike_PFEVEX71316_0;
    static constexpr bool default_proform_bike_PFEVEX71316_0 = false;

    static const QString real_inclination_to_virtual_treamill_bridge;
    static constexpr bool default_real_inclination_to_virtual_treamill_bridge = false;

    static const QString stryd_inclination_instead_treadmill;
    static constexpr bool default_stryd_inclination_instead_treadmill = false;

    static const QString domyos_elliptical_fmts;
    static constexpr bool default_domyos_elliptical_fmts = false;

    static const QString proform_xbike;
    static constexpr bool default_proform_xbike = false;

    static const QString peloton_current_user_id;
    static const QString default_peloton_current_user_id;
    static const QString proform_225_csx_PFEX32925_INT_0;
    static constexpr bool default_proform_225_csx_PFEX32925_INT_0 = false;
    static const QString trainprogram_pid_ignore_inclination;
    static constexpr bool default_trainprogram_pid_ignore_inclination = false;
    static const QString android_antbike;
    static constexpr bool default_android_antbike = false;

    static const QString tile_hr_time_in_zone_1_enabled;
    static constexpr bool default_tile_hr_time_in_zone_1_enabled = false;

    static const QString tile_hr_time_in_zone_1_order;
    static constexpr int default_tile_hr_time_in_zone_1_order = 62;

    static const QString tile_hr_time_in_zone_2_enabled;
    static constexpr bool default_tile_hr_time_in_zone_2_enabled = false;

    static const QString tile_hr_time_in_zone_2_order;
    static constexpr int default_tile_hr_time_in_zone_2_order = 63;

    static const QString tile_hr_time_in_zone_3_enabled;
    static constexpr bool default_tile_hr_time_in_zone_3_enabled = false;

    static const QString tile_hr_time_in_zone_3_order;
    static constexpr int default_tile_hr_time_in_zone_3_order = 64;

    static const QString tile_hr_time_in_zone_4_enabled;
    static constexpr bool default_tile_hr_time_in_zone_4_enabled = false;

    static const QString tile_hr_time_in_zone_4_order;
    static constexpr int default_tile_hr_time_in_zone_4_order = 65;

    static const QString tile_hr_time_in_zone_5_enabled;
    static constexpr bool default_tile_hr_time_in_zone_5_enabled = false;

    static const QString tile_hr_time_in_zone_5_order;
    static constexpr int default_tile_hr_time_in_zone_5_order = 66;

    static const QString tile_heat_time_in_zone_1_enabled;
    static constexpr bool default_tile_heat_time_in_zone_1_enabled = false;

    static const QString tile_heat_time_in_zone_1_order;
    static constexpr int default_tile_heat_time_in_zone_1_order = 68;

    static const QString tile_heat_time_in_zone_2_enabled;
    static constexpr bool default_tile_heat_time_in_zone_2_enabled = false;

    static const QString tile_heat_time_in_zone_2_order;
    static constexpr int default_tile_heat_time_in_zone_2_order = 69;

    static const QString tile_heat_time_in_zone_3_enabled;
    static constexpr bool default_tile_heat_time_in_zone_3_enabled = false;

    static const QString tile_heat_time_in_zone_3_order;
    static constexpr int default_tile_heat_time_in_zone_3_order = 70;

    static const QString tile_heat_time_in_zone_4_enabled;
    static constexpr bool default_tile_heat_time_in_zone_4_enabled = false;

    static const QString tile_heat_time_in_zone_4_order;
    static constexpr int default_tile_heat_time_in_zone_4_order = 71;

    static const QString zwift_gear_ui_aligned;
    static constexpr bool default_zwift_gear_ui_aligned = false;

    static const QString tacxneo2_disable_negative_inclination;
    static constexpr bool default_tacxneo2_disable_negative_inclination = false;

    static const QString tile_coretemperature_enabled;
    static constexpr bool default_tile_coretemperature_enabled = false;

    static const QString tile_coretemperature_order;
    static constexpr int default_tile_coretemperature_order = 67;

    static const QString tile_hr_time_in_zone_individual_mode;
    static constexpr bool default_tile_hr_time_in_zone_individual_mode = false;

    static const QString nordictrack_t65s_treadmill_81_miles;
    static constexpr bool default_nordictrack_t65s_treadmill_81_miles = false;

    static const QString nordictrack_elite_800;
    static constexpr bool default_nordictrack_elite_800 = false;
    static const QString ios_btdevice_native;
    static constexpr bool default_ios_btdevice_native = false;

    static const QString inclinationResistancePoints;
    static const QString default_inclinationResistancePoints;

    /**
     * @brief Enable automatic virtual shifting based on cadence thresholds
     */
    static const QString automatic_virtual_shifting_enabled;
    static constexpr bool default_automatic_virtual_shifting_enabled = false;

    /**
     * @brief Selected profile for automatic virtual shifting (0=cruise, 1=climb, 2=sprint)
     */
    static const QString automatic_virtual_shifting_profile;
    static constexpr int default_automatic_virtual_shifting_profile = 0;

    /**
     * @brief Cadence threshold for gear up - Cruise Profile (RPM)
     */
    static const QString automatic_virtual_shifting_gear_up_cadence;
    static constexpr int default_automatic_virtual_shifting_gear_up_cadence = 95;

    /**
     * @brief Time above cadence threshold before gear up - Cruise Profile (seconds)
     */
    static const QString automatic_virtual_shifting_gear_up_time;
    static constexpr float default_automatic_virtual_shifting_gear_up_time = 2.0;

    /**
     * @brief Cadence threshold for gear down - Cruise Profile (RPM)
     */
    static const QString automatic_virtual_shifting_gear_down_cadence;
    static constexpr int default_automatic_virtual_shifting_gear_down_cadence = 65;

    /**
     * @brief Time below cadence threshold before gear down - Cruise Profile (seconds)
     */
    static const QString automatic_virtual_shifting_gear_down_time;
    static constexpr float default_automatic_virtual_shifting_gear_down_time = 2.0;

    // Climb Profile Settings
    /**
     * @brief Cadence threshold for gear up - Climb Profile (RPM)
     */
    static const QString automatic_virtual_shifting_climb_gear_up_cadence;
    static constexpr int default_automatic_virtual_shifting_climb_gear_up_cadence = 95;

    /**
     * @brief Time above cadence threshold before gear up - Climb Profile (seconds)
     */
    static const QString automatic_virtual_shifting_climb_gear_up_time;
    static constexpr float default_automatic_virtual_shifting_climb_gear_up_time = 2.0;

    /**
     * @brief Cadence threshold for gear down - Climb Profile (RPM)
     */
    static const QString automatic_virtual_shifting_climb_gear_down_cadence;
    static constexpr int default_automatic_virtual_shifting_climb_gear_down_cadence = 65;

    /**
     * @brief Time below cadence threshold before gear down - Climb Profile (seconds)
     */
    static const QString automatic_virtual_shifting_climb_gear_down_time;
    static constexpr float default_automatic_virtual_shifting_climb_gear_down_time = 2.0;

    // Sprint Profile Settings
    /**
     * @brief Cadence threshold for gear up - Sprint Profile (RPM)
     */
    static const QString automatic_virtual_shifting_sprint_gear_up_cadence;
    static constexpr int default_automatic_virtual_shifting_sprint_gear_up_cadence = 95;

    /**
     * @brief Time above cadence threshold before gear up - Sprint Profile (seconds)
     */
    static const QString automatic_virtual_shifting_sprint_gear_up_time;
    static constexpr float default_automatic_virtual_shifting_sprint_gear_up_time = 2.0;

    /**
     * @brief Cadence threshold for gear down - Sprint Profile (RPM)
     */
    static const QString automatic_virtual_shifting_sprint_gear_down_cadence;
    static constexpr int default_automatic_virtual_shifting_sprint_gear_down_cadence = 65;

    /**
     * @brief Time below cadence threshold before gear down - Sprint Profile (seconds)
     */
    static const QString automatic_virtual_shifting_sprint_gear_down_time;
    static constexpr float default_automatic_virtual_shifting_sprint_gear_down_time = 2.0;

    /**
     * @brief Type of floating window to use. 0 = classic, 1 = horizontal
     */
    static const QString floatingwindow_type;
    static constexpr int default_floatingwindow_type = 0;

    static const QString pid_heart_zone_erg_mode_watt_step;
    static constexpr int default_pid_heart_zone_erg_mode_watt_step = 5;

    /**
     * @brief Enable auto virtual shifting cruise tile
     */
    static const QString tile_auto_virtual_shifting_cruise_enabled;
    static constexpr bool default_tile_auto_virtual_shifting_cruise_enabled = false;

    /**
     * @brief Order of auto virtual shifting cruise tile
     */
    static const QString tile_auto_virtual_shifting_cruise_order;
    static constexpr int default_tile_auto_virtual_shifting_cruise_order = 55;

    /**
     * @brief Enable auto virtual shifting climb tile
     */
    static const QString tile_auto_virtual_shifting_climb_enabled;
    static constexpr bool default_tile_auto_virtual_shifting_climb_enabled = false;

    /**
     * @brief Order of auto virtual shifting climb tile
     */
    static const QString tile_auto_virtual_shifting_climb_order;
    static constexpr int default_tile_auto_virtual_shifting_climb_order = 56;

    /**
     * @brief Enable auto virtual shifting sprint tile
     */
    static const QString tile_auto_virtual_shifting_sprint_enabled;
    static constexpr bool default_tile_auto_virtual_shifting_sprint_enabled = false;

    /**
     * @brief Order of auto virtual shifting sprint tile
     */
    static const QString tile_auto_virtual_shifting_sprint_order;
    static constexpr int default_tile_auto_virtual_shifting_sprint_order = 57;

    /**
     * @brief Chart display mode: 0 = both charts, 1 = heart rate only, 2 = power only
     */
    static const QString chart_display_mode;
    static constexpr int default_chart_display_mode = 0;

   /**
     * @brief Calculate only active calories (exclude basal metabolic rate)
     */
    static const QString calories_active_only;
    static constexpr bool default_calories_active_only = false;

    /**
     * @brief Calculate calories from heart rate instead of power
     */
    static const QString calories_from_hr;
    static constexpr bool default_calories_from_hr = false;

    /**
     * @brief Show a confirmation dialog before stopping a workout from the UI.
     */
    static const QString confirm_stop_workout;
    static constexpr bool default_confirm_stop_workout = false;

    /**
     * @brief User height in centimeters for BMR calculation
     */
    static const QString height;
    static constexpr double default_height = 175.0;

    static const QString taurua_ic90;
    static constexpr bool default_taurua_ic90 = false;

    static const QString proform_csx210;
    static constexpr bool default_proform_csx210 = false;

    /**
     * @brief Write the QSettings values using the constants from this namespace.
     * @param showDefaults Optionally indicates if the default should be shown with the key.
     */
    static void qDebugAllSettings(bool showDefaults = false);

    /**
     * @brief Restore the default value to all the settings
     */
    static void restoreAll();
};

#endif
