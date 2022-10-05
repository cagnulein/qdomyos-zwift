#ifndef QZSETTINGS_H
#define QZSETTINGS_H

#include <QString>

namespace QZSettings {

//--------------------------------------------------------------------------------------------
// These are not in settings.qml
//--------------------------------------------------------------------------------------------

static const char *cryptoKeySettingsProfiles = "cryptoKeySettingsProfiles";
static const int default_cryptoKeySettingsProfiles = 0;
/**
 *@brief Disable (true) reconnection when the device disconnects from Bluetooth.
 */
static const char *bluetooth_no_reconnection = "bluetooth_no_reconnection";
static const bool default_bluetooth_no_reconnection = false;

/**
 *@brief Choose between wheel revolutions (true) and wheel and crank revolutions (false)
 *when configuring the CSC feature BLE characteristic.
 */
static const char *bike_wheel_revs = "bike_wheel_revs";
static const bool default_bike_wheel_revs = false;

static const char *bluetooth_lastdevice_name = "bluetooth_lastdevice_name";
static const char *default_bluetooth_lastdevice_name = "";

static const char *bluetooth_lastdevice_address = "bluetooth_lastdevice_address";
static const char *default_bluetooth_lastdevice_address = "";

static const char *hrm_lastdevice_name = "hrm_lastdevice_name";
static const char *default_hrm_lastdevice_name = "";

static const char *hrm_lastdevice_address = "hrm_lastdevice_address";
static const char *default_hrm_lastdevice_address = "";

static const char *ftms_accessory_address = "ftms_accessory_address";
static const char *default_ftms_accessory_address = "";

static const char *ftms_accessory_lastdevice_name = "ftms_accessory_lastdevice_name";
static const char *default_ftms_accessory_lastdevice_name = "";

static const char *csc_sensor_address = "csc_sensor_address";
static const char *default_csc_sensor_address = "";

static const char *csc_sensor_lastdevice_name = "csc_sensor_lastdevice_name";
static const char *default_csc_sensor_lastdevice_name = "";

static const char *power_sensor_lastdevice_name = "power_sensor_lastdevice_name";
static const char *default_power_sensor_lastdevice_name = "";

static const char *power_sensor_address = "power_sensor_address";
static const char *default_power_sensor_address = "";

static const char *elite_rizer_lastdevice_name = "elite_rizer_lastdevice_name";
static const char *default_elite_rizer_lastdevice_name = "";

static const char *elite_rizer_address = "elite_rizer_address";
static const char *default_elite_rizer_address = "";

static const char *elite_sterzo_smart_lastdevice_name = "elite_sterzo_smart_lastdevice_name";
static const char *default_elite_sterzo_smart_lastdevice_name = "";

static const char *elite_sterzo_smart_address = "elite_sterzo_smart_address";
static const char *default_elite_sterzo_smart_address = "";

static const char *strava_accesstoken = "strava_accesstoken";
static const char *default_strava_accesstoken = "";

static const char *strava_refreshtoken = "strava_refreshtoken";
static const char *default_strava_refreshtoken = "";

static const char *strava_lastrefresh = "strava_lastrefresh";
static const char *default_strava_lastrefresh = "";

static const char *strava_expires = "strava_expires";
static const char *default_strava_expires = "";

static const char *code = "code";
static const char *default_code = "";

//--------------------------------------------------------------------------------------------

/**
 *@brief Zoom percentage for the user interface.
 */
static const char *ui_zoom = "ui_zoom";
static constexpr float default_ui_zoom = 100.0;

/**
 *@brief Disable (true) or use (false) the device's heart rate service.
 */
static const char *bike_heartrate_service = "bike_heartrate_service";
static constexpr bool default_bike_heartrate_service = false;

/**
 *@brief An offset that can be applied to the resistance from the device.
 * calculated_resistance = raw_resitance * bike_resistance_gain_f + bike_resistance_offset
 */
static const char *bike_resistance_offset = "bike_resistance_offset";
static constexpr int default_bike_resistance_offset = 4;

/**
 *@brief A gain that can be applied to the resistance from the device.
 * calculated_resistance = raw_resitance * bike_resistance_gain_f + bike_resistance_offset
 */
static const char *bike_resistance_gain_f = "bike_resistance_gain_f";
static constexpr float default_bike_resistance_gain_f = 1.0;

/**
 *@brief Used to specify of QZ is using Zwift in ERG (workout) Mode.
 * When supporting this, QZ should communicate the target resistance
 * (or automatically adjust the device's resistance if it has this capability) to match the target
 * watts based on the cadence (RPM). In ERG Mode, the changes in inclination should not affect target resistance,
 * as is the case in Simulation Mode. Default is false.
 *
 */
static const char *zwift_erg = "zwift_erg";
static constexpr bool default_zwift_erg = false;

/**
 *@brief In ERG Mode, Zwift sends a “target output” request. If the output requested doesn’t match the current output
 *(calculated using cadence and resistance level), the target resistance should change to help the user get closer
 *to the target output. If the filter is set to higher values, there should be less adjustment of the target resistance
 *and cadence would need to be increased to match the target output.
 * The zwift_erg_filter and zwift_erg_filter_down settings are the upper and lower margin before the adjustment of
 *resistance is communicated. Example: if the zwift_erg_filter and zwift_erg_filter_down filters are set to 10 and the
 *target output is 100 watts, a change of resistance will only be communicated if the device produces less than 90 Watts
 *or more than 110 Watts.
 */
static const char *zwift_erg_filter = "zwift_erg_filter";
static constexpr float default_zwift_erg_filter = 10.0;

/**
 *@brief In ERG Mode, Zwift sends a “target output” request. If the output requested doesn’t match the current output
 *(calculated using cadence and resistance level), the target resistance should change to help the user get closer
 *to the target output. If the filter is set to higher values, there should be less adjustment of the target resistance
 *and cadence would need to be increased to match the target output.
 *The zwift_erg_filter and zwift_erg_filter_down settings are the upper and lower margin before the adjustment of
 *resistance is communicated. Example: if the zwift_erg_filter and zwift_erg_filter_down filters are set to 10 and the
 *target output is 100 watts, a change of resistance will only be communicated if the device produces less than 90 Watts
 *or more than 110 Watts.
 */
static const char *zwift_erg_filter_down = "zwift_erg_filter_down";
static constexpr float default_zwift_erg_filter_down = 10.0;

/**
 *@brief Used to invoke a workaround whereby negative inclination is multiplied by 2.
 */
static const char *zwift_negative_inclination_x2 = "zwift_negative_inclination_x2";
static constexpr bool default_zwift_negative_inclination_x2 = false;

/**
 *@brief An offset that will be applied to the inclination received from the client application.
 * calculated_inclination = raw_inclination * zwift_inclination_gain + zwift_inclination_offset
 */
static const char *zwift_inclination_offset = "zwift_inclination_offset";
static constexpr float default_zwift_inclination_offset = 0;

/**
 *@brief A gain that will be applied to the inclination received from the client application.
 * calculated_inclination = raw_inclination * zwift_inclination_gain + zwift_inclination_offset
 */
static const char *zwift_inclination_gain = "zwift_inclination_gain";
static constexpr float default_zwift_inclination_gain = 1.0;

static const char *echelon_resistance_offset = "echelon_resistance_offset";
static constexpr float default_echelon_resistance_offset = 0;

static const char *echelon_resistance_gain = "echelon_resistance_gain";
static constexpr float default_echelon_resistance_gain = 1.0;

/**
 *@brief Used for some devices to specify that speed should be calculated from power.
 */
static const char *speed_power_based = "speed_power_based";
static constexpr bool default_speed_power_based = false;

/**
 *@brief The resistance to be set when a bike or elliptical trainer first connects.
 */
static const char *bike_resistance_start = "bike_resistance_start";
static constexpr int default_bike_resistance_start = 1;

/**
 *@brief The age of the user in years.
 */
static const char *age = "age";
static constexpr int default_age = 35.0;

/**
 *@brief The mass of the user in kilograms. Used for power calculations.
 */
static const char *weight = "weight";
static constexpr float default_weight = 75.0;

/**
 *@brief The user's Functional Threshold Power in watts per kilogram. This is a measure of the best average power output
 *the user could sustain for 1 hour in a time-trial scenario.
 */
static const char *ftp = "ftp";
static constexpr float default_ftp = 200.0;

/**
 *@brief Email address of the user.
 */
static const char *user_email = "user_email";

/**
 * @brief Default email address of user. Empty Latin1 string.
 * This is a Latin1 string because of where it could be used.
 */
static const char *default_user_email = "";

static const char *user_nickname = "user_nickname";
static const char *default_user_nickname = "";

/**
 *@brief Specifies whether or not to use miles (false) or kilometers (true) as the unit of distance.
 */
static const char *miles_unit = "miles_unit";
static constexpr bool default_miles_unit = false;

static const char *pause_on_start = "pause_on_start";
static constexpr bool default_pause_on_start = false;

/**
 *@brief value for peloton trainrow.forcespeed.
 */
static const char *treadmill_force_speed = "treadmill_force_speed";
static constexpr bool default_treadmill_force_speed = false;

static const char *pause_on_start_treadmill = "pause_on_start_treadmill";
static constexpr bool default_pause_on_start_treadmill = false;

/**
 *@brief Flag to indicate if it should be ignored (true) that the user has stopped doing work.
 */
static const char *continuous_moving = "continuous_moving";
static constexpr bool default_continuous_moving = false;

static const char *bike_cadence_sensor = "bike_cadence_sensor";
static constexpr bool default_bike_cadence_sensor = false;

static const char *run_cadence_sensor = "run_cadence_sensor";
static constexpr bool default_run_cadence_sensor = false;

static const char *bike_power_sensor = "bike_power_sensor";
static constexpr bool default_bike_power_sensor = false;

static const char *heart_rate_belt_name = "heart_rate_belt_name";
static const char *default_heart_rate_belt_name = "Disabled";

/**
 *@brief Used to ignore the heart rate from some devices.
 */
static const char *heart_ignore_builtin = "heart_ignore_builtin";
static constexpr bool default_heart_ignore_builtin = false;

/**
 * @brief Used to ignore an energy reading from some devices.
 */
static const char *kcal_ignore_builtin = "kcal_ignore_builtin";
static constexpr bool default_kcal_ignore_builtin = false;

static const char *ant_cadence = "ant_cadence";
static constexpr bool default_ant_cadence = false;

static const char *ant_heart = "ant_heart";
static constexpr bool default_ant_heart = false;

static const char *ant_garmin = "ant_garmin";
static constexpr bool default_ant_garmin = false;

static const char *top_bar_enabled = "top_bar_enabled";
static constexpr bool default_top_bar_enabled = true;
/**
 *@brief The username for logging in to Peloton.
 */
static const char *peloton_username = "peloton_username";
static const char *default_peloton_username = "username";

/**
 *@brief The password for logging in to Peloton.
 */
static const char *peloton_password = "peloton_password";
static const char *default_peloton_password = "password";

static const char *peloton_difficulty = "peloton_difficulty";
static const char *default_peloton_difficulty = "lower";

static const char *peloton_cadence_metric = "peloton_cadence_metric";
static const char *default_peloton_cadence_metric = "Cadence";

static const char *peloton_heartrate_metric = "peloton_heartrate_metric";
static const char *default_peloton_heartrate_metric = "Heart Rate";

static const char *peloton_date = "peloton_date";
static const char *default_peloton_date = "Before Title";

static const char *peloton_description_link = "peloton_description_link";
static constexpr bool default_peloton_description_link = true;

static const char *pzp_username = "pzp_username";
static const char *default_pzp_username = "username";

static const char *pzp_password = "pzp_password";
static const char *default_pzp_password = "username";

static const char *tile_speed_enabled = "tile_speed_enabled";
static constexpr bool default_tile_speed_enabled = true;

static const char *tile_speed_order = "tile_speed_order";
static constexpr int default_tile_speed_order = 0;

static const char *tile_inclination_enabled = "tile_inclination_enabled";
static constexpr bool default_tile_inclination_enabled = true;

static const char *tile_inclination_order = "tile_inclination_order";
static constexpr int default_tile_inclination_order = 1;

static const char *tile_cadence_enabled = "tile_cadence_enabled";
static constexpr bool default_tile_cadence_enabled = true;

static const char *tile_cadence_order = "tile_cadence_order";
static constexpr int default_tile_cadence_order = 2;

static const char *tile_elevation_enabled = "tile_elevation_enabled";
static constexpr bool default_tile_elevation_enabled = true;

static const char *tile_elevation_order = "tile_elevation_order";
static constexpr int default_tile_elevation_order = 3;

static const char *tile_calories_enabled = "tile_calories_enabled";
static constexpr bool default_tile_calories_enabled = true;

static const char *tile_calories_order = "tile_calories_order";
static constexpr int default_tile_calories_order = 4;

static const char *tile_odometer_enabled = "tile_odometer_enabled";
static constexpr bool default_tile_odometer_enabled = true;

static const char *tile_odometer_order = "tile_odometer_order";
static constexpr int default_tile_odometer_order = 5;

static const char *tile_pace_enabled = "tile_pace_enabled";
static constexpr bool default_tile_pace_enabled = true;

static const char *tile_pace_order = "tile_pace_order";
static constexpr int default_tile_pace_order = 6;

static const char *tile_resistance_enabled = "tile_resistance_enabled";
static constexpr bool default_tile_resistance_enabled = true;

static const char *tile_resistance_order = "tile_resistance_order";
static constexpr int default_tile_resistance_order = 7;

static const char *tile_watt_enabled = "tile_watt_enabled";
static constexpr bool default_tile_watt_enabled = true;

static const char *tile_watt_order = "tile_watt_order";
static constexpr int default_tile_watt_order = 8;

static const char *tile_weight_loss_enabled = "tile_weight_loss_enabled";
static constexpr bool default_tile_weight_loss_enabled = false;

static const char *tile_weight_loss_order = "tile_weight_loss_order";
static constexpr int default_tile_weight_loss_order = 24;

static const char *tile_avgwatt_enabled = "tile_avgwatt_enabled";
static constexpr bool default_tile_avgwatt_enabled = true;

static const char *tile_avgwatt_order = "tile_avgwatt_order";
static constexpr int default_tile_avgwatt_order = 9;

static const char *tile_ftp_enabled = "tile_ftp_enabled";
static constexpr bool default_tile_ftp_enabled = true;

static const char *tile_ftp_order = "tile_ftp_order";
static constexpr int default_tile_ftp_order = 10;

static const char *tile_heart_enabled = "tile_heart_enabled";
static constexpr bool default_tile_heart_enabled = true;

static const char *tile_heart_order = "tile_heart_order";
static constexpr int default_tile_heart_order = 11;

static const char *tile_fan_enabled = "tile_fan_enabled";
static constexpr bool default_tile_fan_enabled = true;

static const char *tile_fan_order = "tile_fan_order";
static constexpr int default_tile_fan_order = 12;

static const char *tile_jouls_enabled = "tile_jouls_enabled";
static constexpr bool default_tile_jouls_enabled = true;

static const char *tile_jouls_order = "tile_jouls_order";
static constexpr int default_tile_jouls_order = 13;

static const char *tile_elapsed_enabled = "tile_elapsed_enabled";
static constexpr bool default_tile_elapsed_enabled = true;

static const char *tile_elapsed_order = "tile_elapsed_order";
static constexpr int default_tile_elapsed_order = 14;

static const char *tile_lapelapsed_enabled = "tile_lapelapsed_enabled";
static constexpr bool default_tile_lapelapsed_enabled = false;

static const char *tile_lapelapsed_order = "tile_lapelapsed_order";
static constexpr int default_tile_lapelapsed_order = 17;

static const char *tile_moving_time_enabled = "tile_moving_time_enabled";
static constexpr bool default_tile_moving_time_enabled = false;

static const char *tile_moving_time_order = "tile_moving_time_order";
static constexpr int default_tile_moving_time_order = 21;

static const char *tile_peloton_offset_enabled = "tile_peloton_offset_enabled";
static constexpr bool default_tile_peloton_offset_enabled = false;

static const char *tile_peloton_offset_order = "tile_peloton_offset_order";
static constexpr int default_tile_peloton_offset_order = 22;

static const char *tile_peloton_difficulty_enabled = "tile_peloton_difficulty_enabled";
static constexpr bool default_tile_peloton_difficulty_enabled = false;

static const char *tile_peloton_difficulty_order = "tile_peloton_difficulty_order";
static constexpr int default_tile_peloton_difficulty_order = 32;

static const char *tile_peloton_resistance_enabled = "tile_peloton_resistance_enabled";
static constexpr bool default_tile_peloton_resistance_enabled = true;

static const char *tile_peloton_resistance_order = "tile_peloton_resistance_order";
static constexpr int default_tile_peloton_resistance_order = 15;

static const char *tile_datetime_enabled = "tile_datetime_enabled";
static constexpr bool default_tile_datetime_enabled = true;

static const char *tile_datetime_order = "tile_datetime_order";
static constexpr int default_tile_datetime_order = 16;

static const char *tile_target_resistance_enabled = "tile_target_resistance_enabled";
static constexpr bool default_tile_target_resistance_enabled = true;

static const char *tile_target_resistance_order = "tile_target_resistance_order";
static constexpr int default_tile_target_resistance_order = 15;

static const char *tile_target_peloton_resistance_enabled = "tile_target_peloton_resistance_enabled";
static constexpr bool default_tile_target_peloton_resistance_enabled = false;

static const char *tile_target_peloton_resistance_order = "tile_target_peloton_resistance_order";
static constexpr int default_tile_target_peloton_resistance_order = 21;

static const char *tile_target_cadence_enabled = "tile_target_cadence_enabled";
static constexpr bool default_tile_target_cadence_enabled = false;

static const char *tile_target_cadence_order = "tile_target_cadence_order";
static constexpr int default_tile_target_cadence_order = 19;

static const char *tile_target_power_enabled = "tile_target_power_enabled";
static constexpr bool default_tile_target_power_enabled = false;

static const char *tile_target_power_order = "tile_target_power_order";
static constexpr int default_tile_target_power_order = 20;

static const char *tile_target_zone_enabled = "tile_target_zone_enabled";
static constexpr bool default_tile_target_zone_enabled = false;

static const char *tile_target_zone_order = "tile_target_zone_order";
static constexpr int default_tile_target_zone_order = 24;

static const char *tile_target_speed_enabled = "tile_target_speed_enabled";
static constexpr bool default_tile_target_speed_enabled = false;

static const char *tile_target_speed_order = "tile_target_speed_order";
static constexpr int default_tile_target_speed_order = 27;

static const char *tile_target_incline_enabled = "tile_target_incline_enabled";
static constexpr bool default_tile_target_incline_enabled = false;

static const char *tile_target_incline_order = "tile_target_incline_order";
static constexpr int default_tile_target_incline_order = 28;

static const char *tile_strokes_count_enabled = "tile_strokes_count_enabled";
static constexpr bool default_tile_strokes_count_enabled = false;

static const char *tile_strokes_count_order = "tile_strokes_count_order";
static constexpr int default_tile_strokes_count_order = 22;

static const char *tile_strokes_length_enabled = "tile_strokes_length_enabled";
static constexpr bool default_tile_strokes_length_enabled = false;

static const char *tile_strokes_length_order = "tile_strokes_length_order";
static constexpr int default_tile_strokes_length_order = 23;

static const char *tile_watt_kg_enabled = "tile_watt_kg_enabled";
static constexpr bool default_tile_watt_kg_enabled = false;

static const char *tile_watt_kg_order = "tile_watt_kg_order";
static constexpr int default_tile_watt_kg_order = 25;

static const char *tile_gears_enabled = "tile_gears_enabled";
static constexpr bool default_tile_gears_enabled = false;

static const char *tile_gears_order = "tile_gears_order";
static constexpr int default_tile_gears_order = 26;

static const char *tile_remainingtimetrainprogramrow_enabled = "tile_remainingtimetrainprogramrow_enabled";
static constexpr bool default_tile_remainingtimetrainprogramrow_enabled = false;

static const char *tile_remainingtimetrainprogramrow_order = "tile_remainingtimetrainprogramrow_order";
static constexpr int default_tile_remainingtimetrainprogramrow_order = 27;

static const char *tile_nextrowstrainprogram_enabled = "tile_nextrowstrainprogram_enabled";
static constexpr bool default_tile_nextrowstrainprogram_enabled = false;

static const char *tile_nextrowstrainprogram_order = "tile_nextrowstrainprogram_order";
static constexpr int default_tile_nextrowstrainprogram_order = 31;

static const char *tile_mets_enabled = "tile_mets_enabled";
static constexpr bool default_tile_mets_enabled = false;

static const char *tile_mets_order = "tile_mets_order";
static constexpr int default_tile_mets_order = 28;

static const char *tile_targetmets_enabled = "tile_targetmets_enabled";
static constexpr bool default_tile_targetmets_enabled = false;

static const char *tile_targetmets_order = "tile_targetmets_order";
static constexpr int default_tile_targetmets_order = 29;

static const char *tile_steering_angle_enabled = "tile_steering_angle_enabled";
static constexpr bool default_tile_steering_angle_enabled = false;

static const char *tile_steering_angle_order = "tile_steering_angle_order";
static constexpr int default_tile_steering_angle_order = 30;

static const char *tile_pid_hr_enabled = "tile_pid_hr_enabled";
static constexpr bool default_tile_pid_hr_enabled = false;

static const char *tile_pid_hr_order = "tile_pid_hr_order";
static constexpr int default_tile_pid_hr_order = 31;

static const char *heart_rate_zone1 = "heart_rate_zone1";
static constexpr float default_heart_rate_zone1 = 70.0;

static const char *heart_rate_zone2 = "heart_rate_zone2";
static constexpr float default_heart_rate_zone2 = 80.0;

static const char *heart_rate_zone3 = "heart_rate_zone3";
static constexpr float default_heart_rate_zone3 = 90.0;

static const char *heart_rate_zone4 = "heart_rate_zone4";
static constexpr float default_heart_rate_zone4 = 100.0;

static const char *heart_max_override_enable = "heart_max_override_enable";
static constexpr bool default_heart_max_override_enable = false;

static const char *heart_max_override_value = "heart_max_override_value";
static constexpr float default_heart_max_override_value = 195.0;

static const char *peloton_gain = "peloton_gain";
static constexpr float default_peloton_gain = 1.0;

static const char *peloton_offset = "peloton_offset";
static constexpr float default_peloton_offset = 0;

static const char *treadmill_pid_heart_zone = "treadmill_pid_heart_zone";
static const char *default_treadmill_pid_heart_zone = "Disabled";
/**
 *@brief 1 mile time goal, for a training program with the speed control.
 */
static const char *pacef_1mile = "pacef_1mile";
static constexpr float default_pacef_1mile = 250;
/**
 *@brief 5 km time goal, for a training program with the speed control.
 */
static const char *pacef_5km = "pacef_5km";
static constexpr float default_pacef_5km = 300;
/**
 *@brief 10 km time goal, for a training program with the speed control.
 */
static const char *pacef_10km = "pacef_10km";
static constexpr float default_pacef_10km = 320;
/**
 *@brief  pacef_1mile, but for half-marathon distance, for a training program with the speed control.
 */
static const char *pacef_halfmarathon = "pacef_halfmarathon";
static constexpr float default_pacef_halfmarathon = 340;
/**
 *@brief  pacef_1mile, but for marathon distance, for a training program with the speed control.
 */
static const char *pacef_marathon = "pacef_marathon";
static constexpr float default_pacef_marathon = 360;

/**
 *@brief default pace to be used when the ZWO file does not indicate a precise pace.
 *Text values, i.e. "1 mile", "5 km", "10 km", "Half Marathon"
 */
static const char *pace_default = "pace_default";
static const char *default_pace_default = "Half Marathon";

static const char *domyos_treadmill_buttons = "domyos_treadmill_buttons";
static constexpr bool default_domyos_treadmill_buttons = false;

static const char *domyos_treadmill_distance_display = "domyos_treadmill_distance_display";
static constexpr bool default_domyos_treadmill_distance_display = true;

static const char *domyos_treadmill_display_invert = "domyos_treadmill_display_invert";
static constexpr bool default_domyos_treadmill_display_invert = false;

static const char *domyos_bike_cadence_filter = "domyos_bike_cadence_filter";
static constexpr float default_domyos_bike_cadence_filter = 0.0;

static const char *domyos_bike_display_calories = "domyos_bike_display_calories";
static constexpr bool default_domyos_bike_display_calories = true;

static const char *domyos_elliptical_speed_ratio = "domyos_elliptical_speed_ratio";
static constexpr float default_domyos_elliptical_speed_ratio = 1.0;

static const char *eslinker_cadenza = "eslinker_cadenza";
static constexpr bool default_eslinker_cadenza = true;
/**
 *@brief Choose between the standard and MGARCEA watt table.
 */
static const char *echelon_watttable = "echelon_watttable";
static const char *default_echelon_watttable = "Echelon";

static const char *proform_wheel_ratio = "proform_wheel_ratio";
static constexpr float default_proform_wheel_ratio = 0.33;

static const char *proform_tour_de_france_clc = "proform_tour_de_france_clc";
static constexpr bool default_proform_tour_de_france_clc = false;

static const char *proform_tdf_jonseed_watt = "proform_tdf_jonseed_watt";
static constexpr bool default_proform_tdf_jonseed_watt = false;

static const char *proform_studio = "proform_studio";
static constexpr bool default_proform_studio = false;

static const char *proform_tdf_10 = "proform_tdf_10";
static constexpr bool default_proform_tdf_10 = false;

static const char *horizon_gr7_cadence_multiplier = "horizon_gr7_cadence_multiplier";
static constexpr double default_horizon_gr7_cadence_multiplier = 1.0;

static const char *fitshow_user_id = "fitshow_user_id";
static constexpr int default_fitshow_user_id = 0x13AA;

static const char *inspire_peloton_formula = "inspire_peloton_formula";
static constexpr bool default_inspire_peloton_formula = false;

static const char *inspire_peloton_formula2 = "inspire_peloton_formula2";
static constexpr bool default_inspire_peloton_formula2 = false;

static const char *hammer_racer_s = "hammer_racer_s";
static constexpr bool default_hammer_racer_s = false;

static const char *pafers_treadmill = "pafers_treadmill";
static constexpr bool default_pafers_treadmill = false;

static const char *yesoul_peloton_formula = "yesoul_peloton_formula";
static constexpr bool default_yesoul_peloton_formula = false;

static const char *nordictrack_10_treadmill = "nordictrack_10_treadmill";
static constexpr bool default_nordictrack_10_treadmill = true;

static const char *nordictrack_t65s_treadmill = "nordictrack_t65s_treadmill";
static constexpr bool default_nordictrack_t65s_treadmill = false;

// static const char* proform_treadmill_995i = "proform_treadmill_995i";
// static constexpr bool default_proform_treadmill_995i = false;

static const char *toorx_3_0 = "toorx_3_0";
static constexpr bool default_toorx_3_0 = false;

static const char *toorx_65s_evo = "toorx_65s_evo";
static constexpr bool default_toorx_65s_evo = false;

static const char *jtx_fitness_sprint_treadmill = "jtx_fitness_sprint_treadmill";
static constexpr bool default_jtx_fitness_sprint_treadmill = false;

static const char *dkn_endurun_treadmill = "dkn_endurun_treadmill";
static constexpr bool default_dkn_endurun_treadmill = false;

static const char *trx_route_key = "trx_route_key";
static constexpr bool default_trx_route_key = false;

static const char *bh_spada_2 = "bh_spada_2";
static constexpr bool default_bh_spada_2 = false;

static const char *toorx_bike = "toorx_bike";
static constexpr bool default_toorx_bike = false;

static const char *toorx_ftms = "toorx_ftms";
static constexpr bool default_toorx_ftms = false;

static const char *jll_IC400_bike = "jll_IC400_bike";
static constexpr bool default_jll_IC400_bike = false;

static const char *fytter_ri08_bike = "fytter_ri08_bike";
static constexpr bool default_fytter_ri08_bike = false;

static const char *asviva_bike = "asviva_bike";
static constexpr bool default_asviva_bike = false;

static const char *hertz_xr_770 = "hertz_xr_770";
static constexpr bool default_hertz_xr_770 = false;

static const char *m3i_bike_id = "m3i_bike_id";
static constexpr int default_m3i_bike_id = 256;

static const char *m3i_bike_speed_buffsize = "m3i_bike_speed_buffsize";
static constexpr int default_m3i_bike_speed_buffsize = 90;

static const char *m3i_bike_qt_search = "m3i_bike_qt_search";
static constexpr bool default_m3i_bike_qt_search = false;

static const char *m3i_bike_kcal = "m3i_bike_kcal";
static constexpr bool default_m3i_bike_kcal = true;

static const char *snode_bike = "snode_bike";
static constexpr bool default_snode_bike = false;

static const char *fitplus_bike = "fitplus_bike";
static constexpr bool default_fitplus_bike = false;

static const char *virtufit_etappe = "virtufit_etappe";
static constexpr bool default_virtufit_etappe = false;

static const char *flywheel_filter = "flywheel_filter";
static constexpr int default_flywheel_filter = 2;

static const char *flywheel_life_fitness_ic8 = "flywheel_life_fitness_ic8";
static constexpr bool default_flywheel_life_fitness_ic8 = false;

static const char *sole_treadmill_inclination = "sole_treadmill_inclination";
static constexpr bool default_sole_treadmill_inclination = false;

static const char *sole_treadmill_miles = "sole_treadmill_miles";
static constexpr bool default_sole_treadmill_miles = true;

static const char *sole_treadmill_f65 = "sole_treadmill_f65";
static constexpr bool default_sole_treadmill_f65 = false;

static const char *sole_treadmill_f63 = "sole_treadmill_f63";
static constexpr bool default_sole_treadmill_f63 = false;

static const char *sole_treadmill_tt8 = "sole_treadmill_tt8";
static constexpr bool default_sole_treadmill_tt8 = false;

static const char *schwinn_bike_resistance = "schwinn_bike_resistance";
static constexpr bool default_schwinn_bike_resistance = false;

static const char *schwinn_bike_resistance_v2 = "schwinn_bike_resistance_v2";
static constexpr bool default_schwinn_bike_resistance_v2 = false;

static const char *technogym_myrun_treadmill_experimental = "technogym_myrun_treadmill_experimental";
static constexpr bool default_technogym_myrun_treadmill_experimental = false;

static const char *trainprogram_random = "trainprogram_random";
static constexpr bool default_trainprogram_random = false;

static const char *trainprogram_total = "trainprogram_total";
static constexpr int default_trainprogram_total = 60;

static const char *trainprogram_period_seconds = "trainprogram_period_seconds";
static constexpr float default_trainprogram_period_seconds = 60;

static const char *trainprogram_speed_min = "trainprogram_speed_min";
static constexpr float default_trainprogram_speed_min = 8;

static const char *trainprogram_speed_max = "trainprogram_speed_max";
static constexpr float default_trainprogram_speed_max = 16;

static const char *trainprogram_incline_min = "trainprogram_incline_min";
static constexpr float default_trainprogram_incline_min = 0;

static const char *trainprogram_incline_max = "trainprogram_incline_max";
static constexpr float default_trainprogram_incline_max = 15;

static const char *trainprogram_resistance_min = "trainprogram_resistance_min";
static constexpr float default_trainprogram_resistance_min = 1;

static const char *trainprogram_resistance_max = "trainprogram_resistance_max";
static constexpr float default_trainprogram_resistance_max = 32;

/**
 * @brief Adjusts value in a metric object that's configured specifically for measuring WATTS.
 */
static const char *watt_offset = "watt_offset";
static constexpr float default_watt_offset = 0;

/**
 * @brief Adjusts value in a metric object that's configured specifically for measuring WATTS.
 */
static const char *watt_gain = "watt_gain";
static constexpr float default_watt_gain = 1;

static const char *power_avg_5s = "power_avg_5s";
static constexpr bool default_power_avg_5s = false;

static const char *instant_power_on_pause = "instant_power_on_pause";
static constexpr bool default_instant_power_on_pause = false;

/**
 * @brief Adjusts value in a metric object that's configured specifically for measuring SPEED.
 */
static const char *speed_offset = "speed_offset";
static constexpr float default_speed_offset = 0;

/**
 * @brief Adjusts value in a metric object that's configured specifically for measuring SPEED.
 */
static const char *speed_gain = "speed_gain";
static constexpr float default_speed_gain = 1;

static const char *filter_device = "filter_device";
static const char *default_filter_device = "Disabled";

static const char *strava_suffix = "strava_suffix";
static const char *default_strava_suffix = "#QZ";

static const char *cadence_sensor_name = "cadence_sensor_name";
static const char *default_cadence_sensor_name = "Disabled";

static const char *cadence_sensor_as_bike = "cadence_sensor_as_bike";
static constexpr bool default_cadence_sensor_as_bike = false;

static const char *cadence_sensor_speed_ratio = "cadence_sensor_speed_ratio";
static constexpr float default_cadence_sensor_speed_ratio = 0.33;

static const char *power_hr_pwr1 = "power_hr_pwr1";
static constexpr float default_power_hr_pwr1 = 200;

static const char *power_hr_hr1 = "power_hr_hr1";
static constexpr float default_power_hr_hr1 = 150;

static const char *power_hr_pwr2 = "power_hr_pwr2";
static constexpr float default_power_hr_pwr2 = 230;

static const char *power_hr_hr2 = "power_hr_hr2";
static constexpr float default_power_hr_hr2 = 170;

static const char *power_sensor_name = "power_sensor_name";
static const char *default_power_sensor_name = "Disabled";

static const char *power_sensor_as_bike = "power_sensor_as_bike";
static constexpr bool default_power_sensor_as_bike = false;

static const char *power_sensor_as_treadmill = "power_sensor_as_treadmill";
static constexpr bool default_power_sensor_as_treadmill = false;

static const char *powr_sensor_running_cadence_double = "powr_sensor_running_cadence_double";
static constexpr bool default_powr_sensor_running_cadence_double = false;

static const char *elite_rizer_name = "elite_rizer_name";
static const char *default_elite_rizer_name = "Disabled";

static const char *elite_sterzo_smart_name = "elite_sterzo_smart_name";
static const char *default_elite_sterzo_smart_name = "Disabled";

static const char *ftms_accessory_name = "ftms_accessory_name";
static const char *default_ftms_accessory_name = "Disabled";

static const char *ss2k_shift_step = "ss2k_shift_step";
static constexpr float default_ss2k_shift_step = 900;

static const char *fitmetria_fanfit_enable = "fitmetria_fanfit_enable";
static constexpr bool default_fitmetria_fanfit_enable = false;

static const char *fitmetria_fanfit_mode = "fitmetria_fanfit_mode";
static const char *default_fitmetria_fanfit_mode = "Heart";

static const char *fitmetria_fanfit_min = "fitmetria_fanfit_min";
static constexpr float default_fitmetria_fanfit_min = 0;

static const char *fitmetria_fanfit_max = "fitmetria_fanfit_max";
static constexpr float default_fitmetria_fanfit_max = 100;
/**
 *@brief Indicates if the virtual device should send resistance requests to the bike.
 */
static const char *virtualbike_forceresistance = "virtualbike_forceresistance";
static constexpr bool default_virtualbike_forceresistance = true;
/**
 *@brief Troubleshooting setting. Should be false unless advised by QZ tech support.
 */
static const char *bluetooth_relaxed = "bluetooth_relaxed";
static constexpr bool default_bluetooth_relaxed = false;
/**
 *@brief Troubleshooting setting. Should be false unless advised by QZ tech support.
 */
static const char *bluetooth_30m_hangs = "bluetooth_30m_hangs";
static constexpr bool default_bluetooth_30m_hangs = false;

static const char *battery_service = "battery_service";
static constexpr bool default_battery_service = false;

/**
 *@brief Experimental feature. Not recommended to use.
 */
static const char *service_changed = "service_changed";
static constexpr bool default_service_changed = false;

/**
 *@brief Enable/disable the virtual device that connects QZ to the client app.
 */
static const char *virtual_device_enabled = "virtual_device_enabled";
static constexpr bool default_virtual_device_enabled = true;
/**
 *@brief Enable/disable the Bluetooth connectivity of the virtual device that connects QZ to the client app.
 */
static const char *virtual_device_bluetooth = "virtual_device_bluetooth";
static constexpr bool default_virtual_device_bluetooth = true;

static const char *ios_peloton_workaround = "ios_peloton_workaround";
static constexpr bool default_ios_peloton_workaround = true;

static const char *android_wakelock = "android_wakelock";
static constexpr bool default_android_wakelock = true;
/**
 *@brief Specifies if the debug log file will be written.
 */
static const char *log_debug = "log_debug";
static constexpr bool default_log_debug = false;
/**
 *@brief Force QZ to communicate ONLY the Heart Rate metric to third-party apps.
 */
static const char *virtual_device_onlyheart = "virtual_device_onlyheart";
static constexpr bool default_virtual_device_onlyheart = false;
/**
 *@brief Enables QZ to communicate with the Echelon app.
 *This setting can only be used with iOS running QZ and iOS running the Echelon app.
 */
static const char *virtual_device_echelon = "virtual_device_echelon";
static constexpr bool default_virtual_device_echelon = false;
/**
 *@brief Enables a virtual bluetooth bridge to the iFit App.
 */
static const char *virtual_device_ifit = "virtual_device_ifit";
static constexpr bool default_virtual_device_ifit = false;
/**
 *@brief Instructs QZ to send a rower Bluetooth profile instead of a bike profile to third party apps that support
 *rowing (examples: Kinomap and BitGym). This should be off for Zwift.
 */
static const char *virtual_device_rower = "virtual_device_rower";
static constexpr bool default_virtual_device_rower = false;
/**
 *@brief Used to force a non-bike device to be presented to client apps as a bike.
 */
static const char *virtual_device_force_bike = "virtual_device_force_bike";
static constexpr bool default_virtual_device_force_bike = false;

static const char *volume_change_gears = "volume_change_gears";
static constexpr bool default_volume_change_gears = false;

static const char *applewatch_fakedevice = "applewatch_fakedevice";
static constexpr bool default_applewatch_fakedevice = false;

/**
 *@brief Minimum target resistance for ERG mode.
 */
static const char *zwift_erg_resistance_down = "zwift_erg_resistance_down";
static constexpr float default_zwift_erg_resistance_down = 0.0;

/**
 *@brief Maximum targe resistance for ERG mode.
 */
static const char *zwift_erg_resistance_up = "zwift_erg_resistance_up";
static constexpr float default_zwift_erg_resistance_up = 999.0;

static const char *horizon_paragon_x = "horizon_paragon_x";
static constexpr bool default_horizon_paragon_x = false;

static const char *treadmill_step_speed = "treadmill_step_speed";
static constexpr float default_treadmill_step_speed = 0.5;

static const char *treadmill_step_incline = "treadmill_step_incline";
static constexpr float default_treadmill_step_incline = 0.5;

static const char *fitshow_anyrun = "fitshow_anyrun";
static constexpr bool default_fitshow_anyrun = false;

static const char *nordictrack_s30_treadmill = "nordictrack_s30_treadmill";
static constexpr bool default_nordictrack_s30_treadmill = false;

// from version 2.10.23
// not used anymore because it's an elliptical not a treadmill. Don't remove this
// it will cause corruption in the settings
// static const char* nordictrack_fs5i_treadmill = "nordictrack_fs5i_treadmill";
// static constexpr bool default_nordictrack_fs5i_treadmill = false;

static const char *renpho_peloton_conversion_v2 = "renpho_peloton_conversion_v2";
static constexpr bool default_renpho_peloton_conversion_v2 = false;

static const char *ss2k_resistance_sample_1 = "ss2k_resistance_sample_1";
static constexpr float default_ss2k_resistance_sample_1 = 20;

static const char *ss2k_shift_step_sample_1 = "ss2k_shift_step_sample_1";
static constexpr float default_ss2k_shift_step_sample_1 = 0;

static const char *ss2k_resistance_sample_2 = "ss2k_resistance_sample_2";
static constexpr float default_ss2k_resistance_sample_2 = 30;

static const char *ss2k_shift_step_sample_2 = "ss2k_shift_step_sample_2";
static constexpr float default_ss2k_shift_step_sample_2 = 0;

static const char *ss2k_resistance_sample_3 = "ss2k_resistance_sample_3";
static constexpr float default_ss2k_resistance_sample_3 = 40;

static const char *ss2k_shift_step_sample_3 = "ss2k_shift_step_sample_3";
static constexpr float default_ss2k_shift_step_sample_3 = 0;

static const char *ss2k_resistance_sample_4 = "ss2k_resistance_sample_4";
static constexpr float default_ss2k_resistance_sample_4 = 50;

static const char *ss2k_shift_step_sample_4 = "ss2k_shift_step_sample_4";
static constexpr float default_ss2k_shift_step_sample_4 = 0;

static const char *fitshow_truetimer = "fitshow_truetimer";
static constexpr bool default_fitshow_truetimer = false;

static const char *elite_rizer_gain = "elite_rizer_gain";
static constexpr float default_elite_rizer_gain = 1.0;

static const char *tile_ext_incline_enabled = "tile_ext_incline_enabled";
static constexpr bool default_tile_ext_incline_enabled = false;

static const char *tile_ext_incline_order = "tile_ext_incline_order";
static constexpr int default_tile_ext_incline_order = 32;

static const char *reebok_fr30_treadmill = "reebok_fr30_treadmill";
static constexpr bool default_reebok_fr30_treadmill = false;

static const char *horizon_treadmill_7_8 = "horizon_treadmill_7_8";
static constexpr bool default_horizon_treadmill_7_8 = false;

/**
 *@brief The name of the profile for this settings file.
 */
static const char *profile_name = "profile_name";
static const char *default_profile_name = "default";

static const char *tile_cadence_color_enabled = "tile_cadence_color_enabled";
static constexpr bool default_tile_cadence_color_enabled = false;

static const char *tile_peloton_remaining_enabled = "tile_peloton_remaining_enabled";
static constexpr bool default_tile_peloton_remaining_enabled = false;

static const char *tile_peloton_remaining_order = "tile_peloton_remaining_order";
static constexpr int default_tile_peloton_remaining_order = 22;

static const char *tile_peloton_resistance_color_enabled = "tile_peloton_resistance_color_enabled";
static constexpr bool default_tile_peloton_resistance_color_enabled = false;

/**
 *@brief Enable the Wahoo Dircon device.
 */
static const char *dircon_yes = "dircon_yes";
static constexpr bool default_dircon_yes = true;

static const char *dircon_server_base_port = "dircon_server_base_port";
static constexpr int default_dircon_server_base_port = 36866;

static const char *ios_cache_heart_device = "ios_cache_heart_device";
static constexpr bool default_ios_cache_heart_device = true;

/**
 *@brief Count of the number of times the app has been opened.
 */
static const char *app_opening = "app_opening";
static constexpr int default_app_opening = 0;

static const char *proformtdf4ip = "proformtdf4ip";
static const char *default_proformtdf4ip = "";

static const char *fitfiu_mc_v460 = "fitfiu_mc_v460";
static constexpr bool default_fitfiu_mc_v460 = false;

/**
 *@brief The mass of the bike in kilograms.
 */
static const char *bike_weight = "bike_weight";
static constexpr float default_bike_weight = 0;

static const char *kingsmith_encrypt_v2 = "kingsmith_encrypt_v2";
static constexpr bool default_kingsmith_encrypt_v2 = false;

static const char *proform_treadmill_9_0 = "proform_treadmill_9_0";
static constexpr bool default_proform_treadmill_9_0 = false;

static const char *proform_treadmill_1800i = "proform_treadmill_1800i";
static constexpr bool default_proform_treadmill_1800i = false;

static const char *cadence_offset = "cadence_offset";
static constexpr float default_cadence_offset = 0;

static const char *cadence_gain = "cadence_gain";
static constexpr float default_cadence_gain = 1;

static const char *sp_ht_9600ie = "sp_ht_9600ie";
static constexpr bool default_sp_ht_9600ie = false;

/**
 * @brief Enable text to speech.
 */
static const char *tts_enabled = "tts_enabled";
static constexpr bool default_tts_enabled = false;

static const char *tts_summary_sec = "tts_summary_sec";
static constexpr int default_tts_summary_sec = 120;

static const char *tts_act_speed = "tts_act_speed";
static constexpr bool default_tts_act_speed = false;

static const char *tts_avg_speed = "tts_avg_speed";
static constexpr bool default_tts_avg_speed = true;

static const char *tts_max_speed = "tts_max_speed";
static constexpr bool default_tts_max_speed = false;

static const char *tts_act_inclination = "tts_act_inclination";
static constexpr bool default_tts_act_inclination = false;

static const char *tts_act_cadence = "tts_act_cadence";
static constexpr bool default_tts_act_cadence = false;

static const char *tts_avg_cadence = "tts_avg_cadence";
static constexpr bool default_tts_avg_cadence = true;

static const char *tts_max_cadence = "tts_max_cadence";
static constexpr bool default_tts_max_cadence = false;

static const char *tts_act_elevation = "tts_act_elevation";
static constexpr bool default_tts_act_elevation = true;

static const char *tts_act_calories = "tts_act_calories";
static constexpr bool default_tts_act_calories = true;

static const char *tts_act_odometer = "tts_act_odometer";
static constexpr bool default_tts_act_odometer = true;

static const char *tts_act_pace = "tts_act_pace";
static constexpr bool default_tts_act_pace = false;

static const char *tts_avg_pace = "tts_avg_pace";
static constexpr bool default_tts_avg_pace = true;

static const char *tts_max_pace = "tts_max_pace";
static constexpr bool default_tts_max_pace = false;

static const char *tts_act_resistance = "tts_act_resistance";
static constexpr bool default_tts_act_resistance = true;

static const char *tts_avg_resistance = "tts_avg_resistance";
static constexpr bool default_tts_avg_resistance = true;

static const char *tts_max_resistance = "tts_max_resistance";
static constexpr bool default_tts_max_resistance = false;

static const char *tts_act_watt = "tts_act_watt";
static constexpr bool default_tts_act_watt = false;

static const char *tts_avg_watt = "tts_avg_watt";
static constexpr bool default_tts_avg_watt = true;

static const char *tts_max_watt = "tts_max_watt";
static constexpr bool default_tts_max_watt = true;

static const char *tts_act_ftp = "tts_act_ftp";
static constexpr bool default_tts_act_ftp = false;

static const char *tts_avg_ftp = "tts_avg_ftp";
static constexpr bool default_tts_avg_ftp = true;

static const char *tts_max_ftp = "tts_max_ftp";
static constexpr bool default_tts_max_ftp = false;

static const char *tts_act_heart = "tts_act_heart";
static constexpr bool default_tts_act_heart = true;

static const char *tts_avg_heart = "tts_avg_heart";
static constexpr bool default_tts_avg_heart = true;

static const char *tts_max_heart = "tts_max_heart";
static constexpr bool default_tts_max_heart = false;

static const char *tts_act_jouls = "tts_act_jouls";
static constexpr bool default_tts_act_jouls = true;

static const char *tts_act_elapsed = "tts_act_elapsed";
static constexpr bool default_tts_act_elapsed = true;

static const char *tts_act_peloton_resistance = "tts_act_peloton_resistance";
static constexpr bool default_tts_act_peloton_resistance = false;

static const char *tts_avg_peloton_resistance = "tts_avg_peloton_resistance";
static constexpr bool default_tts_avg_peloton_resistance = false;

static const char *tts_max_peloton_resistance = "tts_max_peloton_resistance";
static constexpr bool default_tts_max_peloton_resistance = false;

static const char *tts_act_target_peloton_resistance = "tts_act_target_peloton_resistance";
static constexpr bool default_tts_act_target_peloton_resistance = true;

static const char *tts_act_target_cadence = "tts_act_target_cadence";
static constexpr bool default_tts_act_target_cadence = true;

static const char *tts_act_target_power = "tts_act_target_power";
static constexpr bool default_tts_act_target_power = true;

static const char *tts_act_target_zone = "tts_act_target_zone";
static constexpr bool default_tts_act_target_zone = true;

static const char *tts_act_target_speed = "tts_act_target_speed";
static constexpr bool default_tts_act_target_speed = true;

static const char *tts_act_target_incline = "tts_act_target_incline";
static constexpr bool default_tts_act_target_incline = true;

static const char *tts_act_watt_kg = "tts_act_watt_kg";
static constexpr bool default_tts_act_watt_kg = false;

static const char *tts_avg_watt_kg = "tts_avg_watt_kg";
static constexpr bool default_tts_avg_watt_kg = false;

static const char *tts_max_watt_kg = "tts_max_watt_kg";
static constexpr bool default_tts_max_watt_kg = false;

/**
 *@brief Enable the fake device, emulating an elliptical trainer.
 */
static const char *fakedevice_elliptical = "fakedevice_elliptical";
static constexpr bool default_fakedevice_elliptical = false;

static const char *nordictrack_2950_ip = "nordictrack_2950_ip";
static const char *default_nordictrack_2950_ip = "";

static const char *tile_instantaneous_stride_length_enabled = "tile_instantaneous_stride_length_enabled";
static constexpr bool default_tile_instantaneous_stride_length_enabled = false;

static const char *tile_instantaneous_stride_length_order = "tile_instantaneous_stride_length_order";
static constexpr int default_tile_instantaneous_stride_length_order = 32;

static const char *tile_ground_contact_enabled = "tile_ground_contact_enabled";
static constexpr bool default_tile_ground_contact_enabled = false;

static const char *tile_ground_contact_order = "tile_ground_contact_order";
static constexpr int default_tile_ground_contact_order = 33;

static const char *tile_vertical_oscillation_enabled = "tile_vertical_oscillation_enabled";
static constexpr bool default_tile_vertical_oscillation_enabled = false;

static const char *tile_vertical_oscillation_order = "tile_vertical_oscillation_order";
static constexpr int default_tile_vertical_oscillation_order = 34;

/**
 *@brief The gender of the user.
 */
static const char *sex = "sex";
static const char *default_sex = "Male";

static const char *maps_type = "maps_type";
static const char *default_maps_type = "3D";

static const char *ss2k_max_resistance = "ss2k_max_resistance";
static constexpr float default_ss2k_max_resistance = 100;

static const char *ss2k_min_resistance = "ss2k_min_resistance";
static constexpr float default_ss2k_min_resistance = 0;

static const char *proform_treadmill_se = "proform_treadmill_se";
static constexpr bool default_proform_treadmill_se = false;

/**
 *@brief The IP address for the Proform Treadmill.
 */
static const char *proformtreadmillip = "proformtreadmillip";
static const char *default_proformtreadmillip = "";
// from version 2.11.22
/**
 *@brief
 */
static const char *kingsmith_encrypt_v3 = "kingsmith_encrypt_v3";
static constexpr bool default_kingsmith_encrypt_v3 = false;

/**
 *@brief IP address for the TDF 10.
 */
static const char *tdf_10_ip = "tdf_10_ip";
static const char *default_tdf_10_ip = "";

/**
 *@brief
 */
static const char *fakedevice_treadmill = "fakedevice_treadmill";
static constexpr bool default_fakedevice_treadmill = false;

/**
 *@brief The number of seconds to add to the video timestamp.
 */
static const char *video_playback_window_s = "video_playback_window_s";
static constexpr int default_video_playback_window_s = 12;

static const char *horizon_treadmill_profile_user1 = "horizon_treadmill_profile_user1";
static const char *default_horizon_treadmill_profile_user1 = "user1";

static const char *horizon_treadmill_profile_user2 = "horizon_treadmill_profile_user2";
static const char *default_horizon_treadmill_profile_user2 = "user2";

static const char *horizon_treadmill_profile_user3 = "horizon_treadmill_profile_user3";
static const char *default_horizon_treadmill_profile_user3 = "user3";

static const char *horizon_treadmill_profile_user4 = "horizon_treadmill_profile_user4";
static const char *default_horizon_treadmill_profile_user4 = "user4";

static const char *horizon_treadmill_profile_user5 = "horizon_treadmill_profile_user5";
static const char *default_horizon_treadmill_profile_user5 = "user5";

static const char *nordictrack_gx_2_7 = "nordictrack_gx_2_7";
static const bool default_nordictrack_gx_2_7 = false;

/**
 * @brief Write the QSettings values using the constants from this namespace.
 * @param showDefaults Optionally indicates if the default should be shown with the key.
 */
void qDebugAllSettings(bool showDefaults = false);
} // namespace QZSettings

#endif
