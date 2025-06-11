#include "qzsettings.h"
#include <QDebug>
#include <QSettings>
const QString QZSettings::cryptoKeySettingsProfiles = QStringLiteral("cryptoKeySettingsProfiles");
const QString QZSettings::bluetooth_no_reconnection = QStringLiteral("bluetooth_no_reconnection");
const QString QZSettings::bike_wheel_revs = QStringLiteral("bike_wheel_revs");
const QString QZSettings::bluetooth_lastdevice_name = QStringLiteral("bluetooth_lastdevice_name");
const QString QZSettings::default_bluetooth_lastdevice_name = QStringLiteral("");
const QString QZSettings::bluetooth_lastdevice_address = QStringLiteral("bluetooth_lastdevice_address");
const QString QZSettings::default_bluetooth_lastdevice_address = QStringLiteral("");
const QString QZSettings::hrm_lastdevice_name = QStringLiteral("hrm_lastdevice_name");
const QString QZSettings::default_hrm_lastdevice_name = QStringLiteral("");
const QString QZSettings::hrm_lastdevice_address = QStringLiteral("hrm_lastdevice_address");
const QString QZSettings::default_hrm_lastdevice_address = QStringLiteral("");
const QString QZSettings::ftms_accessory_address = QStringLiteral("ftms_accessory_address");
const QString QZSettings::default_ftms_accessory_address = QStringLiteral("");
const QString QZSettings::ftms_accessory_lastdevice_name = QStringLiteral("ftms_accessory_lastdevice_name");
const QString QZSettings::default_ftms_accessory_lastdevice_name = QStringLiteral("");
const QString QZSettings::csc_sensor_address = QStringLiteral("csc_sensor_address");
const QString QZSettings::default_csc_sensor_address = QStringLiteral("");
const QString QZSettings::csc_sensor_lastdevice_name = QStringLiteral("csc_sensor_lastdevice_name");
const QString QZSettings::default_csc_sensor_lastdevice_name = QStringLiteral("");
const QString QZSettings::power_sensor_lastdevice_name = QStringLiteral("power_sensor_lastdevice_name");
const QString QZSettings::default_power_sensor_lastdevice_name = QStringLiteral("");
const QString QZSettings::power_sensor_address = QStringLiteral("power_sensor_address");
const QString QZSettings::default_power_sensor_address = QStringLiteral("");
const QString QZSettings::elite_rizer_lastdevice_name = QStringLiteral("elite_rizer_lastdevice_name");
const QString QZSettings::default_elite_rizer_lastdevice_name = QStringLiteral("");
const QString QZSettings::elite_rizer_address = QStringLiteral("elite_rizer_address");
const QString QZSettings::default_elite_rizer_address = QStringLiteral("");
const QString QZSettings::elite_sterzo_smart_lastdevice_name = QStringLiteral("elite_sterzo_smart_lastdevice_name");
const QString QZSettings::default_elite_sterzo_smart_lastdevice_name = QStringLiteral("");
const QString QZSettings::elite_sterzo_smart_address = QStringLiteral("elite_sterzo_smart_address");
const QString QZSettings::default_elite_sterzo_smart_address = QStringLiteral("");
const QString QZSettings::strava_accesstoken = QStringLiteral("strava_accesstoken");
const QString QZSettings::default_strava_accesstoken = QStringLiteral("");
const QString QZSettings::strava_refreshtoken = QStringLiteral("strava_refreshtoken");
const QString QZSettings::default_strava_refreshtoken = QStringLiteral("");
const QString QZSettings::strava_lastrefresh = QStringLiteral("strava_lastrefresh");
const QString QZSettings::default_strava_lastrefresh = QStringLiteral("");
const QString QZSettings::strava_expires = QStringLiteral("strava_expires");
const QString QZSettings::default_strava_expires = QStringLiteral("");
const QString QZSettings::code = QStringLiteral("code");
const QString QZSettings::default_code = QStringLiteral("");
//--------------------------------------------------------------------------------------------
const QString QZSettings::ui_zoom = QStringLiteral("ui_zoom");
const QString QZSettings::bike_heartrate_service = QStringLiteral("bike_heartrate_service");
const QString QZSettings::bike_resistance_offset = QStringLiteral("bike_resistance_offset");
const QString QZSettings::bike_resistance_gain_f = QStringLiteral("bike_resistance_gain_f");
const QString QZSettings::zwift_erg = QStringLiteral("zwift_erg");
const QString QZSettings::zwift_erg_filter = QStringLiteral("zwift_erg_filter");
const QString QZSettings::zwift_erg_filter_down = QStringLiteral("zwift_erg_filter_down");
const QString QZSettings::zwift_negative_inclination_x2 = QStringLiteral("zwift_negative_inclination_x2");
const QString QZSettings::zwift_inclination_offset = QStringLiteral("zwift_inclination_offset");
const QString QZSettings::zwift_inclination_gain = QStringLiteral("zwift_inclination_gain");
const QString QZSettings::echelon_resistance_offset = QStringLiteral("echelon_resistance_offset");
const QString QZSettings::echelon_resistance_gain = QStringLiteral("echelon_resistance_gain");
const QString QZSettings::speed_power_based = QStringLiteral("speed_power_based");
const QString QZSettings::bike_resistance_start = QStringLiteral("bike_resistance_start");
const QString QZSettings::age = QStringLiteral("age");
const QString QZSettings::weight = QStringLiteral("weight");
const QString QZSettings::ftp = QStringLiteral("ftp");
const QString QZSettings::user_email = QStringLiteral("user_email");
const QString QZSettings::default_user_email = QLatin1String("");
const QString QZSettings::user_nickname = QStringLiteral("user_nickname");
const QString QZSettings::default_user_nickname = QStringLiteral("");
const QString QZSettings::miles_unit = QStringLiteral("miles_unit");
const QString QZSettings::pause_on_start = QStringLiteral("pause_on_start");
const QString QZSettings::treadmill_force_speed = QStringLiteral("treadmill_force_speed");
const QString QZSettings::pause_on_start_treadmill = QStringLiteral("pause_on_start_treadmill");
const QString QZSettings::continuous_moving = QStringLiteral("continuous_moving");
const QString QZSettings::bike_cadence_sensor = QStringLiteral("bike_cadence_sensor");
const QString QZSettings::run_cadence_sensor = QStringLiteral("run_cadence_sensor");
const QString QZSettings::bike_power_sensor = QStringLiteral("bike_power_sensor");
const QString QZSettings::heart_rate_belt_name = QStringLiteral("heart_rate_belt_name");
const QString QZSettings::default_heart_rate_belt_name = QStringLiteral("Disabled");
const QString QZSettings::heart_ignore_builtin = QStringLiteral("heart_ignore_builtin");
const QString QZSettings::kcal_ignore_builtin = QStringLiteral("kcal_ignore_builtin");
const QString QZSettings::ant_cadence = QStringLiteral("ant_cadence");
const QString QZSettings::ant_heart = QStringLiteral("ant_heart");
const QString QZSettings::ant_garmin = QStringLiteral("ant_garmin");
const QString QZSettings::top_bar_enabled = QStringLiteral("top_bar_enabled");
const QString QZSettings::peloton_username = QStringLiteral("peloton_username");
const QString QZSettings::default_peloton_username = QStringLiteral("username");
const QString QZSettings::peloton_password = QStringLiteral("peloton_password");
const QString QZSettings::default_peloton_password = QStringLiteral("password");
const QString QZSettings::peloton_difficulty = QStringLiteral("peloton_difficulty");
const QString QZSettings::default_peloton_difficulty = QStringLiteral("lower");
const QString QZSettings::peloton_cadence_metric = QStringLiteral("peloton_cadence_metric");
const QString QZSettings::default_peloton_cadence_metric = QStringLiteral("Cadence");
const QString QZSettings::peloton_heartrate_metric = QStringLiteral("peloton_heartrate_metric");
const QString QZSettings::default_peloton_heartrate_metric = QStringLiteral("Heart Rate");
const QString QZSettings::peloton_date = QStringLiteral("peloton_date");
const QString QZSettings::default_peloton_date = QStringLiteral("Before Title");
const QString QZSettings::peloton_description_link = QStringLiteral("peloton_description_link");
const QString QZSettings::pzp_username = QStringLiteral("pzp_username");
const QString QZSettings::default_pzp_username = QStringLiteral("username");
const QString QZSettings::pzp_password = QStringLiteral("pzp_password");
const QString QZSettings::default_pzp_password = QStringLiteral("username");
const QString QZSettings::tile_speed_enabled = QStringLiteral("tile_speed_enabled");
const QString QZSettings::tile_speed_order = QStringLiteral("tile_speed_order");
const QString QZSettings::tile_inclination_enabled = QStringLiteral("tile_inclination_enabled");
const QString QZSettings::tile_inclination_order = QStringLiteral("tile_inclination_order");
const QString QZSettings::tile_cadence_enabled = QStringLiteral("tile_cadence_enabled");
const QString QZSettings::tile_cadence_order = QStringLiteral("tile_cadence_order");
const QString QZSettings::tile_elevation_enabled = QStringLiteral("tile_elevation_enabled");
const QString QZSettings::tile_elevation_order = QStringLiteral("tile_elevation_order");
const QString QZSettings::tile_calories_enabled = QStringLiteral("tile_calories_enabled");
const QString QZSettings::tile_calories_order = QStringLiteral("tile_calories_order");
const QString QZSettings::tile_odometer_enabled = QStringLiteral("tile_odometer_enabled");
const QString QZSettings::tile_odometer_order = QStringLiteral("tile_odometer_order");
const QString QZSettings::tile_pace_enabled = QStringLiteral("tile_pace_enabled");
const QString QZSettings::tile_pace_order = QStringLiteral("tile_pace_order");
const QString QZSettings::tile_resistance_enabled = QStringLiteral("tile_resistance_enabled");
const QString QZSettings::tile_resistance_order = QStringLiteral("tile_resistance_order");
const QString QZSettings::tile_watt_enabled = QStringLiteral("tile_watt_enabled");
const QString QZSettings::tile_watt_order = QStringLiteral("tile_watt_order");
const QString QZSettings::tile_weight_loss_enabled = QStringLiteral("tile_weight_loss_enabled");
const QString QZSettings::tile_weight_loss_order = QStringLiteral("tile_weight_loss_order");
const QString QZSettings::tile_avgwatt_enabled = QStringLiteral("tile_avgwatt_enabled");
const QString QZSettings::tile_avgwatt_order = QStringLiteral("tile_avgwatt_order");
const QString QZSettings::tile_ftp_enabled = QStringLiteral("tile_ftp_enabled");
const QString QZSettings::tile_ftp_order = QStringLiteral("tile_ftp_order");
const QString QZSettings::tile_heart_enabled = QStringLiteral("tile_heart_enabled");
const QString QZSettings::tile_heart_order = QStringLiteral("tile_heart_order");
const QString QZSettings::tile_fan_enabled = QStringLiteral("tile_fan_enabled");
const QString QZSettings::tile_fan_order = QStringLiteral("tile_fan_order");
const QString QZSettings::tile_jouls_enabled = QStringLiteral("tile_jouls_enabled");
const QString QZSettings::tile_jouls_order = QStringLiteral("tile_jouls_order");
const QString QZSettings::tile_elapsed_enabled = QStringLiteral("tile_elapsed_enabled");
const QString QZSettings::tile_elapsed_order = QStringLiteral("tile_elapsed_order");
const QString QZSettings::tile_lapelapsed_enabled = QStringLiteral("tile_lapelapsed_enabled");
const QString QZSettings::tile_lapelapsed_order = QStringLiteral("tile_lapelapsed_order");
const QString QZSettings::tile_moving_time_enabled = QStringLiteral("tile_moving_time_enabled");
const QString QZSettings::tile_moving_time_order = QStringLiteral("tile_moving_time_order");
const QString QZSettings::tile_peloton_offset_enabled = QStringLiteral("tile_peloton_offset_enabled");
const QString QZSettings::tile_peloton_offset_order = QStringLiteral("tile_peloton_offset_order");
const QString QZSettings::tile_peloton_difficulty_enabled = QStringLiteral("tile_peloton_difficulty_enabled");
const QString QZSettings::tile_peloton_difficulty_order = QStringLiteral("tile_peloton_difficulty_order");
const QString QZSettings::tile_peloton_resistance_enabled = QStringLiteral("tile_peloton_resistance_enabled");
const QString QZSettings::tile_peloton_resistance_order = QStringLiteral("tile_peloton_resistance_order");
const QString QZSettings::tile_datetime_enabled = QStringLiteral("tile_datetime_enabled");
const QString QZSettings::tile_datetime_order = QStringLiteral("tile_datetime_order");
const QString QZSettings::tile_target_resistance_enabled = QStringLiteral("tile_target_resistance_enabled");
const QString QZSettings::tile_target_resistance_order = QStringLiteral("tile_target_resistance_order");
const QString QZSettings::tile_target_peloton_resistance_enabled =
    QStringLiteral("tile_target_peloton_resistance_enabled");
const QString QZSettings::tile_target_peloton_resistance_order = QStringLiteral("tile_target_peloton_resistance_order");
const QString QZSettings::tile_target_cadence_enabled = QStringLiteral("tile_target_cadence_enabled");
const QString QZSettings::tile_target_cadence_order = QStringLiteral("tile_target_cadence_order");
const QString QZSettings::tile_target_power_enabled = QStringLiteral("tile_target_power_enabled");
const QString QZSettings::tile_target_power_order = QStringLiteral("tile_target_power_order");
const QString QZSettings::tile_target_zone_enabled = QStringLiteral("tile_target_zone_enabled");
const QString QZSettings::tile_target_zone_order = QStringLiteral("tile_target_zone_order");
const QString QZSettings::tile_target_speed_enabled = QStringLiteral("tile_target_speed_enabled");
const QString QZSettings::tile_target_speed_order = QStringLiteral("tile_target_speed_order");
const QString QZSettings::tile_target_incline_enabled = QStringLiteral("tile_target_incline_enabled");
const QString QZSettings::tile_target_incline_order = QStringLiteral("tile_target_incline_order");
const QString QZSettings::tile_strokes_count_enabled = QStringLiteral("tile_strokes_count_enabled");
const QString QZSettings::tile_strokes_count_order = QStringLiteral("tile_strokes_count_order");
const QString QZSettings::tile_strokes_length_enabled = QStringLiteral("tile_strokes_length_enabled");
const QString QZSettings::tile_strokes_length_order = QStringLiteral("tile_strokes_length_order");
const QString QZSettings::tile_watt_kg_enabled = QStringLiteral("tile_watt_kg_enabled");
const QString QZSettings::tile_watt_kg_order = QStringLiteral("tile_watt_kg_order");
const QString QZSettings::tile_gears_enabled = QStringLiteral("tile_gears_enabled");
const QString QZSettings::tile_gears_order = QStringLiteral("tile_gears_order");
const QString QZSettings::tile_remainingtimetrainprogramrow_enabled =
    QStringLiteral("tile_remainingtimetrainprogramrow_enabled");
const QString QZSettings::tile_remainingtimetrainprogramrow_order =
    QStringLiteral("tile_remainingtimetrainprogramrow_order");
const QString QZSettings::tile_nextrowstrainprogram_enabled = QStringLiteral("tile_nextrowstrainprogram_enabled");
const QString QZSettings::tile_nextrowstrainprogram_order = QStringLiteral("tile_nextrowstrainprogram_order");
const QString QZSettings::tile_mets_enabled = QStringLiteral("tile_mets_enabled");
const QString QZSettings::tile_mets_order = QStringLiteral("tile_mets_order");
const QString QZSettings::tile_targetmets_enabled = QStringLiteral("tile_targetmets_enabled");
const QString QZSettings::tile_targetmets_order = QStringLiteral("tile_targetmets_order");
const QString QZSettings::tile_steering_angle_enabled = QStringLiteral("tile_steering_angle_enabled");
const QString QZSettings::tile_steering_angle_order = QStringLiteral("tile_steering_angle_order");
const QString QZSettings::tile_pid_hr_enabled = QStringLiteral("tile_pid_hr_enabled");
const QString QZSettings::tile_pid_hr_order = QStringLiteral("tile_pid_hr_order");
const QString QZSettings::heart_rate_zone1 = QStringLiteral("heart_rate_zone1");
const QString QZSettings::heart_rate_zone2 = QStringLiteral("heart_rate_zone2");
const QString QZSettings::heart_rate_zone3 = QStringLiteral("heart_rate_zone3");
const QString QZSettings::heart_rate_zone4 = QStringLiteral("heart_rate_zone4");
const QString QZSettings::heart_max_override_enable = QStringLiteral("heart_max_override_enable");
const QString QZSettings::heart_max_override_value = QStringLiteral("heart_max_override_value");
const QString QZSettings::peloton_gain = QStringLiteral("peloton_gain");
const QString QZSettings::peloton_offset = QStringLiteral("peloton_offset");
const QString QZSettings::treadmill_pid_heart_zone = QStringLiteral("treadmill_pid_heart_zone");
const QString QZSettings::default_treadmill_pid_heart_zone = QStringLiteral("Disabled");
const QString QZSettings::pacef_1mile = QStringLiteral("pacef_1mile");
const QString QZSettings::pacef_5km = QStringLiteral("pacef_5km");
const QString QZSettings::pacef_10km = QStringLiteral("pacef_10km");
const QString QZSettings::pacef_halfmarathon = QStringLiteral("pacef_halfmarathon");
const QString QZSettings::pacef_marathon = QStringLiteral("pacef_marathon");
const QString QZSettings::pace_default = QStringLiteral("pace_default");
const QString QZSettings::default_pace_default = QStringLiteral("Half Marathon");
const QString QZSettings::domyos_treadmill_buttons = QStringLiteral("domyos_treadmill_buttons");
const QString QZSettings::domyos_treadmill_distance_display = QStringLiteral("domyos_treadmill_distance_display");
const QString QZSettings::domyos_treadmill_display_invert = QStringLiteral("domyos_treadmill_display_invert");
const QString QZSettings::domyos_bike_cadence_filter = QStringLiteral("domyos_bike_cadence_filter");
const QString QZSettings::domyos_bike_display_calories = QStringLiteral("domyos_bike_display_calories");
const QString QZSettings::domyos_elliptical_speed_ratio = QStringLiteral("domyos_elliptical_speed_ratio");
const QString QZSettings::eslinker_cadenza = QStringLiteral("eslinker_cadenza");
const QString QZSettings::eslinker_ypoo = QStringLiteral("eslinker_ypoo");
const QString QZSettings::echelon_watttable = QStringLiteral("echelon_watttable");
const QString QZSettings::default_echelon_watttable = QStringLiteral("Echelon");
const QString QZSettings::proform_wheel_ratio = QStringLiteral("proform_wheel_ratio");
const QString QZSettings::proform_tour_de_france_clc = QStringLiteral("proform_tour_de_france_clc");
const QString QZSettings::proform_tdf_jonseed_watt = QStringLiteral("proform_tdf_jonseed_watt");
const QString QZSettings::proform_studio = QStringLiteral("proform_studio");
const QString QZSettings::proform_tdf_10 = QStringLiteral("proform_tdf_10");
const QString QZSettings::horizon_gr7_cadence_multiplier = QStringLiteral("horizon_gr7_cadence_multiplier");
const QString QZSettings::fitshow_user_id = QStringLiteral("fitshow_user_id");
const QString QZSettings::inspire_peloton_formula = QStringLiteral("inspire_peloton_formula");
const QString QZSettings::inspire_peloton_formula2 = QStringLiteral("inspire_peloton_formula2");
const QString QZSettings::hammer_racer_s = QStringLiteral("hammer_racer_s");
const QString QZSettings::pafers_treadmill = QStringLiteral("pafers_treadmill");
const QString QZSettings::yesoul_peloton_formula = QStringLiteral("yesoul_peloton_formula");
const QString QZSettings::nordictrack_10_treadmill = QStringLiteral("nordictrack_10_treadmill");
const QString QZSettings::nordictrack_t65s_treadmill = QStringLiteral("nordictrack_t65s_treadmill");
const QString QZSettings::nordictrack_treadmill_ultra_le = QStringLiteral("nordictrack_treadmill_ultra_le");
// const QString QZSettings:: proform_treadmill_995i = QStringLiteral("proform_treadmill_995i");
//
const QString QZSettings::toorx_3_0 = QStringLiteral("toorx_3_0");
const QString QZSettings::toorx_65s_evo = QStringLiteral("toorx_65s_evo");
const QString QZSettings::jtx_fitness_sprint_treadmill = QStringLiteral("jtx_fitness_sprint_treadmill");
const QString QZSettings::dkn_endurun_treadmill = QStringLiteral("dkn_endurun_treadmill");
const QString QZSettings::trx_route_key = QStringLiteral("trx_route_key");
const QString QZSettings::bh_spada_2 = QStringLiteral("bh_spada_2");
const QString QZSettings::toorx_bike = QStringLiteral("toorx_bike");
const QString QZSettings::toorx_ftms = QStringLiteral("toorx_ftms");
const QString QZSettings::jll_IC400_bike = QStringLiteral("jll_IC400_bike");
const QString QZSettings::fytter_ri08_bike = QStringLiteral("fytter_ri08_bike");
const QString QZSettings::asviva_bike = QStringLiteral("asviva_bike");
const QString QZSettings::hertz_xr_770 = QStringLiteral("hertz_xr_770");
const QString QZSettings::m3i_bike_id = QStringLiteral("m3i_bike_id");
const QString QZSettings::m3i_bike_speed_buffsize = QStringLiteral("m3i_bike_speed_buffsize");
const QString QZSettings::m3i_bike_qt_search = QStringLiteral("m3i_bike_qt_search");
const QString QZSettings::m3i_bike_kcal = QStringLiteral("m3i_bike_kcal");
const QString QZSettings::snode_bike = QStringLiteral("snode_bike");
const QString QZSettings::fitplus_bike = QStringLiteral("fitplus_bike");
const QString QZSettings::virtufit_etappe = QStringLiteral("virtufit_etappe");
const QString QZSettings::flywheel_filter = QStringLiteral("flywheel_filter");
const QString QZSettings::flywheel_life_fitness_ic8 = QStringLiteral("flywheel_life_fitness_ic8");
const QString QZSettings::sole_treadmill_inclination = QStringLiteral("sole_treadmill_inclination");
const QString QZSettings::sole_treadmill_miles = QStringLiteral("sole_treadmill_miles");
const QString QZSettings::sole_treadmill_f65 = QStringLiteral("sole_treadmill_f65");
const QString QZSettings::sole_treadmill_f63 = QStringLiteral("sole_treadmill_f63");
const QString QZSettings::sole_treadmill_tt8 = QStringLiteral("sole_treadmill_tt8");
const QString QZSettings::schwinn_bike_resistance = QStringLiteral("schwinn_bike_resistance");
const QString QZSettings::schwinn_bike_resistance_v2 = QStringLiteral("schwinn_bike_resistance_v2");
const QString QZSettings::technogym_myrun_treadmill_experimental =
    QStringLiteral("technogym_myrun_treadmill_experimental");
const QString QZSettings::trainprogram_random = QStringLiteral("trainprogram_random");
const QString QZSettings::trainprogram_total = QStringLiteral("trainprogram_total");
const QString QZSettings::trainprogram_period_seconds = QStringLiteral("trainprogram_period_seconds");
const QString QZSettings::trainprogram_speed_min = QStringLiteral("trainprogram_speed_min");
const QString QZSettings::trainprogram_speed_max = QStringLiteral("trainprogram_speed_max");
const QString QZSettings::trainprogram_incline_min = QStringLiteral("trainprogram_incline_min");
const QString QZSettings::trainprogram_incline_max = QStringLiteral("trainprogram_incline_max");
const QString QZSettings::trainprogram_resistance_min = QStringLiteral("trainprogram_resistance_min");
const QString QZSettings::trainprogram_resistance_max = QStringLiteral("trainprogram_resistance_max");
const QString QZSettings::watt_offset = QStringLiteral("watt_offset");
const QString QZSettings::watt_gain = QStringLiteral("watt_gain");
const QString QZSettings::power_avg_5s = QStringLiteral("power_avg_5s");
const QString QZSettings::instant_power_on_pause = QStringLiteral("instant_power_on_pause");
const QString QZSettings::speed_offset = QStringLiteral("speed_offset");
const QString QZSettings::speed_gain = QStringLiteral("speed_gain");
const QString QZSettings::filter_device = QStringLiteral("filter_device");
const QString QZSettings::default_filter_device = QStringLiteral("Disabled");
const QString QZSettings::strava_suffix = QStringLiteral("strava_suffix");
const QString QZSettings::default_strava_suffix = QStringLiteral("#QZ");
const QString QZSettings::cadence_sensor_name = QStringLiteral("cadence_sensor_name");
const QString QZSettings::default_cadence_sensor_name = QStringLiteral("Disabled");
const QString QZSettings::cadence_sensor_as_bike = QStringLiteral("cadence_sensor_as_bike");
const QString QZSettings::cadence_sensor_speed_ratio = QStringLiteral("cadence_sensor_speed_ratio");
const QString QZSettings::power_hr_pwr1 = QStringLiteral("power_hr_pwr1");
const QString QZSettings::power_hr_hr1 = QStringLiteral("power_hr_hr1");
const QString QZSettings::power_hr_pwr2 = QStringLiteral("power_hr_pwr2");
const QString QZSettings::power_hr_hr2 = QStringLiteral("power_hr_hr2");
const QString QZSettings::power_sensor_name = QStringLiteral("power_sensor_name");
const QString QZSettings::default_power_sensor_name = QStringLiteral("Disabled");
const QString QZSettings::power_sensor_as_bike = QStringLiteral("power_sensor_as_bike");
const QString QZSettings::power_sensor_as_treadmill = QStringLiteral("power_sensor_as_treadmill");
const QString QZSettings::powr_sensor_running_cadence_double = QStringLiteral("powr_sensor_running_cadence_double");
const QString QZSettings::elite_rizer_name = QStringLiteral("elite_rizer_name");
const QString QZSettings::default_elite_rizer_name = QStringLiteral("Disabled");
const QString QZSettings::elite_sterzo_smart_name = QStringLiteral("elite_sterzo_smart_name");
const QString QZSettings::default_elite_sterzo_smart_name = QStringLiteral("Disabled");
const QString QZSettings::ftms_accessory_name = QStringLiteral("ftms_accessory_name");
const QString QZSettings::default_ftms_accessory_name = QStringLiteral("Disabled");
const QString QZSettings::ss2k_shift_step = QStringLiteral("ss2k_shift_step");
const QString QZSettings::fitmetria_fanfit_enable = QStringLiteral("fitmetria_fanfit_enable");
const QString QZSettings::fitmetria_fanfit_mode = QStringLiteral("fitmetria_fanfit_mode");
const QString QZSettings::default_fitmetria_fanfit_mode = QStringLiteral("Heart");
const QString QZSettings::fitmetria_fanfit_min = QStringLiteral("fitmetria_fanfit_min");
const QString QZSettings::fitmetria_fanfit_max = QStringLiteral("fitmetria_fanfit_max");
const QString QZSettings::virtualbike_forceresistance = QStringLiteral("virtualbike_forceresistance");
const QString QZSettings::bluetooth_relaxed = QStringLiteral("bluetooth_relaxed");
const QString QZSettings::bluetooth_30m_hangs = QStringLiteral("bluetooth_30m_hangs");
const QString QZSettings::battery_service = QStringLiteral("battery_service");
const QString QZSettings::service_changed = QStringLiteral("service_changed");
const QString QZSettings::virtual_device_enabled = QStringLiteral("virtual_device_enabled");
const QString QZSettings::virtual_device_bluetooth = QStringLiteral("virtual_device_bluetooth");
const QString QZSettings::ios_peloton_workaround = QStringLiteral("ios_peloton_workaround");
const QString QZSettings::android_wakelock = QStringLiteral("android_wakelock");
const QString QZSettings::log_debug = QStringLiteral("log_debug");
const QString QZSettings::virtual_device_onlyheart = QStringLiteral("virtual_device_onlyheart");
const QString QZSettings::virtual_device_echelon = QStringLiteral("virtual_device_echelon");
const QString QZSettings::virtual_device_ifit = QStringLiteral("virtual_device_ifit");
const QString QZSettings::virtual_device_rower = QStringLiteral("virtual_device_rower");
const QString QZSettings::virtual_device_force_bike = QStringLiteral("virtual_device_force_bike");
const QString QZSettings::volume_change_gears = QStringLiteral("volume_change_gears");
const QString QZSettings::applewatch_fakedevice = QStringLiteral("applewatch_fakedevice");
const QString QZSettings::zwift_erg_resistance_down = QStringLiteral("zwift_erg_resistance_down");
const QString QZSettings::zwift_erg_resistance_up = QStringLiteral("zwift_erg_resistance_up");
const QString QZSettings::horizon_paragon_x = QStringLiteral("horizon_paragon_x");
const QString QZSettings::treadmill_step_speed = QStringLiteral("treadmill_step_speed");
const QString QZSettings::treadmill_step_incline = QStringLiteral("treadmill_step_incline");
const QString QZSettings::fitshow_anyrun = QStringLiteral("fitshow_anyrun");
const QString QZSettings::nordictrack_s30_treadmill = QStringLiteral("nordictrack_s30_treadmill");
// from version 2.10.23
// not used anymore because it's an elliptical not a treadmill. Don't remove this
// it will cause corruption in the settings
// const QString QZSettings:: nordictrack_fs5i_treadmill = QStringLiteral("nordictrack_fs5i_treadmill");
//
const QString QZSettings::renpho_peloton_conversion_v2 = QStringLiteral("renpho_peloton_conversion_v2");
const QString QZSettings::ss2k_resistance_sample_1 = QStringLiteral("ss2k_resistance_sample_1");
const QString QZSettings::ss2k_shift_step_sample_1 = QStringLiteral("ss2k_shift_step_sample_1");
const QString QZSettings::ss2k_resistance_sample_2 = QStringLiteral("ss2k_resistance_sample_2");
const QString QZSettings::ss2k_shift_step_sample_2 = QStringLiteral("ss2k_shift_step_sample_2");
const QString QZSettings::ss2k_resistance_sample_3 = QStringLiteral("ss2k_resistance_sample_3");
const QString QZSettings::ss2k_shift_step_sample_3 = QStringLiteral("ss2k_shift_step_sample_3");
const QString QZSettings::ss2k_resistance_sample_4 = QStringLiteral("ss2k_resistance_sample_4");
const QString QZSettings::ss2k_shift_step_sample_4 = QStringLiteral("ss2k_shift_step_sample_4");
const QString QZSettings::fitshow_truetimer = QStringLiteral("fitshow_truetimer");
const QString QZSettings::elite_rizer_gain = QStringLiteral("elite_rizer_gain");
const QString QZSettings::tile_ext_incline_enabled = QStringLiteral("tile_ext_incline_enabled");
const QString QZSettings::tile_ext_incline_order = QStringLiteral("tile_ext_incline_order");
const QString QZSettings::reebok_fr30_treadmill = QStringLiteral("reebok_fr30_treadmill");
const QString QZSettings::horizon_treadmill_7_8 = QStringLiteral("horizon_treadmill_7_8");
const QString QZSettings::profile_name = QStringLiteral("profile_name");
const QString QZSettings::default_profile_name = QStringLiteral("default");
const QString QZSettings::tile_cadence_color_enabled = QStringLiteral("tile_cadence_color_enabled");
const QString QZSettings::tile_peloton_remaining_enabled = QStringLiteral("tile_peloton_remaining_enabled");
const QString QZSettings::tile_peloton_remaining_order = QStringLiteral("tile_peloton_remaining_order");
const QString QZSettings::tile_peloton_resistance_color_enabled =
    QStringLiteral("tile_peloton_resistance_color_enabled");
const QString QZSettings::dircon_yes = QStringLiteral("dircon_yes");
const QString QZSettings::dircon_server_base_port = QStringLiteral("dircon_server_base_port");
const QString QZSettings::ios_cache_heart_device = QStringLiteral("ios_cache_heart_device");
const QString QZSettings::app_opening = QStringLiteral("app_opening");
const QString QZSettings::proformtdf4ip = QStringLiteral("proformtdf4ip");
const QString QZSettings::default_proformtdf4ip = QStringLiteral("");
const QString QZSettings::fitfiu_mc_v460 = QStringLiteral("fitfiu_mc_v460");
const QString QZSettings::bike_weight = QStringLiteral("bike_weight");
const QString QZSettings::kingsmith_encrypt_v2 = QStringLiteral("kingsmith_encrypt_v2");
const QString QZSettings::proform_treadmill_9_0 = QStringLiteral("proform_treadmill_9_0");
const QString QZSettings::proform_treadmill_1800i = QStringLiteral("proform_treadmill_1800i");
const QString QZSettings::cadence_offset = QStringLiteral("cadence_offset");
const QString QZSettings::cadence_gain = QStringLiteral("cadence_gain");
const QString QZSettings::sp_ht_9600ie = QStringLiteral("sp_ht_9600ie");
const QString QZSettings::tts_enabled = QStringLiteral("tts_enabled");
const QString QZSettings::tts_summary_sec = QStringLiteral("tts_summary_sec");
const QString QZSettings::tts_act_speed = QStringLiteral("tts_act_speed");
const QString QZSettings::tts_avg_speed = QStringLiteral("tts_avg_speed");
const QString QZSettings::tts_max_speed = QStringLiteral("tts_max_speed");
const QString QZSettings::tts_act_inclination = QStringLiteral("tts_act_inclination");
const QString QZSettings::tts_act_cadence = QStringLiteral("tts_act_cadence");
const QString QZSettings::tts_avg_cadence = QStringLiteral("tts_avg_cadence");
const QString QZSettings::tts_max_cadence = QStringLiteral("tts_max_cadence");
const QString QZSettings::tts_act_elevation = QStringLiteral("tts_act_elevation");
const QString QZSettings::tts_act_calories = QStringLiteral("tts_act_calories");
const QString QZSettings::tts_act_odometer = QStringLiteral("tts_act_odometer");
const QString QZSettings::tts_act_pace = QStringLiteral("tts_act_pace");
const QString QZSettings::tts_avg_pace = QStringLiteral("tts_avg_pace");
const QString QZSettings::tts_max_pace = QStringLiteral("tts_max_pace");
const QString QZSettings::tts_act_resistance = QStringLiteral("tts_act_resistance");
const QString QZSettings::tts_avg_resistance = QStringLiteral("tts_avg_resistance");
const QString QZSettings::tts_max_resistance = QStringLiteral("tts_max_resistance");
const QString QZSettings::tts_act_watt = QStringLiteral("tts_act_watt");
const QString QZSettings::tts_avg_watt = QStringLiteral("tts_avg_watt");
const QString QZSettings::tts_max_watt = QStringLiteral("tts_max_watt");
const QString QZSettings::tts_act_ftp = QStringLiteral("tts_act_ftp");
const QString QZSettings::tts_avg_ftp = QStringLiteral("tts_avg_ftp");
const QString QZSettings::tts_max_ftp = QStringLiteral("tts_max_ftp");
const QString QZSettings::tts_act_heart = QStringLiteral("tts_act_heart");
const QString QZSettings::tts_avg_heart = QStringLiteral("tts_avg_heart");
const QString QZSettings::tts_max_heart = QStringLiteral("tts_max_heart");
const QString QZSettings::tts_act_jouls = QStringLiteral("tts_act_jouls");
const QString QZSettings::tts_act_elapsed = QStringLiteral("tts_act_elapsed");
const QString QZSettings::tts_act_peloton_resistance = QStringLiteral("tts_act_peloton_resistance");
const QString QZSettings::tts_avg_peloton_resistance = QStringLiteral("tts_avg_peloton_resistance");
const QString QZSettings::tts_max_peloton_resistance = QStringLiteral("tts_max_peloton_resistance");
const QString QZSettings::tts_act_target_peloton_resistance = QStringLiteral("tts_act_target_peloton_resistance");
const QString QZSettings::tts_act_target_cadence = QStringLiteral("tts_act_target_cadence");
const QString QZSettings::tts_act_target_power = QStringLiteral("tts_act_target_power");
const QString QZSettings::tts_act_target_zone = QStringLiteral("tts_act_target_zone");
const QString QZSettings::tts_act_target_speed = QStringLiteral("tts_act_target_speed");
const QString QZSettings::tts_act_target_incline = QStringLiteral("tts_act_target_incline");
const QString QZSettings::tts_act_watt_kg = QStringLiteral("tts_act_watt_kg");
const QString QZSettings::tts_avg_watt_kg = QStringLiteral("tts_avg_watt_kg");
const QString QZSettings::tts_max_watt_kg = QStringLiteral("tts_max_watt_kg");
const QString QZSettings::fakedevice_elliptical = QStringLiteral("fakedevice_elliptical");
const QString QZSettings::nordictrack_2950_ip = QStringLiteral("nordictrack_2950_ip");
const QString QZSettings::default_nordictrack_2950_ip = QStringLiteral("");
const QString QZSettings::tile_instantaneous_stride_length_enabled =
    QStringLiteral("tile_instantaneous_stride_length_enabled");
const QString QZSettings::tile_instantaneous_stride_length_order =
    QStringLiteral("tile_instantaneous_stride_length_order");
const QString QZSettings::tile_ground_contact_enabled = QStringLiteral("tile_ground_contact_enabled");
const QString QZSettings::tile_ground_contact_order = QStringLiteral("tile_ground_contact_order");
const QString QZSettings::tile_vertical_oscillation_enabled = QStringLiteral("tile_vertical_oscillation_enabled");
const QString QZSettings::tile_vertical_oscillation_order = QStringLiteral("tile_vertical_oscillation_order");
const QString QZSettings::sex = QStringLiteral("sex");
const QString QZSettings::default_sex = QStringLiteral("Male");
const QString QZSettings::maps_type = QStringLiteral("maps_type");
const QString QZSettings::default_maps_type = QStringLiteral("3D");
const QString QZSettings::ss2k_max_resistance = QStringLiteral("ss2k_max_resistance");
const QString QZSettings::ss2k_min_resistance = QStringLiteral("ss2k_min_resistance");
const QString QZSettings::proform_treadmill_se = QStringLiteral("proform_treadmill_se");
const QString QZSettings::proformtreadmillip = QStringLiteral("proformtreadmillip");
const QString QZSettings::default_proformtreadmillip = QStringLiteral("");
// from version 2.11.22
const QString QZSettings::kingsmith_encrypt_v3 = QStringLiteral("kingsmith_encrypt_v3");
const QString QZSettings::tdf_10_ip = QStringLiteral("tdf_10_ip");
const QString QZSettings::default_tdf_10_ip = QStringLiteral("");
const QString QZSettings::fakedevice_treadmill = QStringLiteral("fakedevice_treadmill");
const QString QZSettings::video_playback_window_s = QStringLiteral("video_playback_window_s");
const QString QZSettings::horizon_treadmill_profile_user1 = QStringLiteral("horizon_treadmill_profile_user1");
const QString QZSettings::default_horizon_treadmill_profile_user1 = QStringLiteral("user1");
const QString QZSettings::horizon_treadmill_profile_user2 = QStringLiteral("horizon_treadmill_profile_user2");
const QString QZSettings::default_horizon_treadmill_profile_user2 = QStringLiteral("user2");
const QString QZSettings::horizon_treadmill_profile_user3 = QStringLiteral("horizon_treadmill_profile_user3");
const QString QZSettings::default_horizon_treadmill_profile_user3 = QStringLiteral("user3");
const QString QZSettings::horizon_treadmill_profile_user4 = QStringLiteral("horizon_treadmill_profile_user4");
const QString QZSettings::default_horizon_treadmill_profile_user4 = QStringLiteral("user4");
const QString QZSettings::horizon_treadmill_profile_user5 = QStringLiteral("horizon_treadmill_profile_user5");
const QString QZSettings::default_horizon_treadmill_profile_user5 = QStringLiteral("user5");
const QString QZSettings::nordictrack_gx_2_7 = QStringLiteral("nordictrack_gx_2_7");
const QString QZSettings::rolling_resistance = QStringLiteral("rolling_resistance");
const QString QZSettings::wahoo_rgt_dircon = QStringLiteral("wahoo_rgt_dircon");
const QString QZSettings::tts_description_enabled = QStringLiteral("tts_description_enabled");
const QString QZSettings::tile_preset_resistance_1_enabled = QStringLiteral("tile_preset_resistance_1_enabled");
const QString QZSettings::tile_preset_resistance_1_order = QStringLiteral("tile_preset_resistance_1_order");
const QString QZSettings::tile_preset_resistance_1_value = QStringLiteral("tile_preset_resistance_1_value");
const QString QZSettings::tile_preset_resistance_1_label = QStringLiteral("tile_preset_resistance_1_label");
const QString QZSettings::default_tile_preset_resistance_1_label = QStringLiteral("Res. 1");
const QString QZSettings::tile_preset_resistance_2_enabled = QStringLiteral("tile_preset_resistance_2_enabled");
const QString QZSettings::tile_preset_resistance_2_order = QStringLiteral("tile_preset_resistance_2_order");
const QString QZSettings::tile_preset_resistance_2_value = QStringLiteral("tile_preset_resistance_2_value");
const QString QZSettings::tile_preset_resistance_2_label = QStringLiteral("tile_preset_resistance_2_label");
const QString QZSettings::default_tile_preset_resistance_2_label = QStringLiteral("Res. 10");
const QString QZSettings::tile_preset_resistance_3_enabled = QStringLiteral("tile_preset_resistance_3_enabled");
const QString QZSettings::tile_preset_resistance_3_order = QStringLiteral("tile_preset_resistance_3_order");
const QString QZSettings::tile_preset_resistance_3_value = QStringLiteral("tile_preset_resistance_3_value");
const QString QZSettings::tile_preset_resistance_3_label = QStringLiteral("tile_preset_resistance_3_label");
const QString QZSettings::default_tile_preset_resistance_3_label = QStringLiteral("Res. 20");
const QString QZSettings::tile_preset_resistance_4_enabled = QStringLiteral("tile_preset_resistance_4_enabled");
const QString QZSettings::tile_preset_resistance_4_order = QStringLiteral("tile_preset_resistance_4_order");
const QString QZSettings::tile_preset_resistance_4_value = QStringLiteral("tile_preset_resistance_4_value");
const QString QZSettings::tile_preset_resistance_4_label = QStringLiteral("tile_preset_resistance_4_label");
const QString QZSettings::default_tile_preset_resistance_4_label = QStringLiteral("Res. 25");
const QString QZSettings::tile_preset_resistance_5_enabled = QStringLiteral("tile_preset_resistance_5_enabled");
const QString QZSettings::tile_preset_resistance_5_order = QStringLiteral("tile_preset_resistance_5_order");
const QString QZSettings::tile_preset_resistance_5_value = QStringLiteral("tile_preset_resistance_5_value");
const QString QZSettings::tile_preset_resistance_5_label = QStringLiteral("tile_preset_resistance_5_label");
const QString QZSettings::default_tile_preset_resistance_5_label = QStringLiteral("Res. 30");
const QString QZSettings::tile_preset_speed_1_enabled = QStringLiteral("tile_preset_speed_1_enabled");
const QString QZSettings::tile_preset_speed_1_order = QStringLiteral("tile_preset_speed_1_order");
const QString QZSettings::tile_preset_speed_1_value = QStringLiteral("tile_preset_speed_1_value");
const QString QZSettings::tile_preset_speed_1_label = QStringLiteral("tile_preset_speed_1_label");
const QString QZSettings::default_tile_preset_speed_1_label = QStringLiteral("5km/h");
const QString QZSettings::tile_preset_speed_2_enabled = QStringLiteral("tile_preset_speed_2_enabled");
const QString QZSettings::tile_preset_speed_2_order = QStringLiteral("tile_preset_speed_2_order");
const QString QZSettings::tile_preset_speed_2_value = QStringLiteral("tile_preset_speed_2_value");
const QString QZSettings::tile_preset_speed_2_label = QStringLiteral("tile_preset_speed_2_label");
const QString QZSettings::default_tile_preset_speed_2_label = QStringLiteral("7 km/h");
const QString QZSettings::tile_preset_speed_3_enabled = QStringLiteral("tile_preset_speed_3_enabled");
const QString QZSettings::tile_preset_speed_3_order = QStringLiteral("tile_preset_speed_3_order");
const QString QZSettings::tile_preset_speed_3_value = QStringLiteral("tile_preset_speed_3_value");
const QString QZSettings::tile_preset_speed_3_label = QStringLiteral("tile_preset_speed_3_label");
const QString QZSettings::default_tile_preset_speed_3_label = QStringLiteral("10 km/h");
const QString QZSettings::tile_preset_speed_4_enabled = QStringLiteral("tile_preset_speed_4_enabled");
const QString QZSettings::tile_preset_speed_4_order = QStringLiteral("tile_preset_speed_4_order");
const QString QZSettings::tile_preset_speed_4_value = QStringLiteral("tile_preset_speed_4_value");
const QString QZSettings::tile_preset_speed_4_label = QStringLiteral("tile_preset_speed_4_label");
const QString QZSettings::default_tile_preset_speed_4_label = QStringLiteral("11 km/h");
const QString QZSettings::tile_preset_speed_5_enabled = QStringLiteral("tile_preset_speed_5_enabled");
const QString QZSettings::tile_preset_speed_5_order = QStringLiteral("tile_preset_speed_5_order");
const QString QZSettings::tile_preset_speed_5_value = QStringLiteral("tile_preset_speed_5_value");
const QString QZSettings::tile_preset_speed_5_label = QStringLiteral("tile_preset_speed_5_label");
const QString QZSettings::default_tile_preset_speed_5_label = QStringLiteral("12 km/h");
const QString QZSettings::tile_preset_inclination_1_enabled = QStringLiteral("tile_preset_inclination_1_enabled");
const QString QZSettings::tile_preset_inclination_1_order = QStringLiteral("tile_preset_inclination_1_order");
const QString QZSettings::tile_preset_inclination_1_value = QStringLiteral("tile_preset_inclination_1_value");
const QString QZSettings::tile_preset_inclination_1_label = QStringLiteral("tile_preset_inclination_1_label");
const QString QZSettings::default_tile_preset_inclination_1_label = QStringLiteral("0%");
const QString QZSettings::tile_preset_inclination_2_enabled = QStringLiteral("tile_preset_inclination_2_enabled");
const QString QZSettings::tile_preset_inclination_2_order = QStringLiteral("tile_preset_inclination_2_order");
const QString QZSettings::tile_preset_inclination_2_value = QStringLiteral("tile_preset_inclination_2_value");
const QString QZSettings::tile_preset_inclination_2_label = QStringLiteral("tile_preset_inclination_2_label");
const QString QZSettings::default_tile_preset_inclination_2_label = QStringLiteral("1%");
const QString QZSettings::tile_preset_inclination_3_enabled = QStringLiteral("tile_preset_inclination_3_enabled");
const QString QZSettings::tile_preset_inclination_3_order = QStringLiteral("tile_preset_inclination_3_order");
const QString QZSettings::tile_preset_inclination_3_value = QStringLiteral("tile_preset_inclination_3_value");
const QString QZSettings::tile_preset_inclination_3_label = QStringLiteral("tile_preset_inclination_3_label");
const QString QZSettings::default_tile_preset_inclination_3_label = QStringLiteral("2%");
const QString QZSettings::tile_preset_inclination_4_enabled = QStringLiteral("tile_preset_inclination_4_enabled");
const QString QZSettings::tile_preset_inclination_4_order = QStringLiteral("tile_preset_inclination_4_order");
const QString QZSettings::tile_preset_inclination_4_value = QStringLiteral("tile_preset_inclination_4_value");
const QString QZSettings::tile_preset_inclination_4_label = QStringLiteral("tile_preset_inclination_4_label");
const QString QZSettings::default_tile_preset_inclination_4_label = QStringLiteral("3%");
const QString QZSettings::tile_preset_inclination_5_enabled = QStringLiteral("tile_preset_inclination_5_enabled");
const QString QZSettings::tile_preset_inclination_5_order = QStringLiteral("tile_preset_inclination_5_order");
const QString QZSettings::tile_preset_inclination_5_value = QStringLiteral("tile_preset_inclination_5_value");
const QString QZSettings::tile_preset_inclination_5_label = QStringLiteral("tile_preset_inclination_5_label");
const QString QZSettings::default_tile_preset_inclination_5_label = QStringLiteral("4%");
const QString QZSettings::tile_preset_resistance_1_color = QStringLiteral("tile_preset_resistance_1_color");
const QString QZSettings::default_tile_preset_resistance_1_color = QStringLiteral("grey");
const QString QZSettings::tile_preset_resistance_2_color = QStringLiteral("tile_preset_resistance_2_color");
const QString QZSettings::default_tile_preset_resistance_2_color = QStringLiteral("grey");
const QString QZSettings::tile_preset_resistance_3_color = QStringLiteral("tile_preset_resistance_3_color");
const QString QZSettings::default_tile_preset_resistance_3_color = QStringLiteral("grey");
const QString QZSettings::tile_preset_resistance_4_color = QStringLiteral("tile_preset_resistance_4_color");
const QString QZSettings::default_tile_preset_resistance_4_color = QStringLiteral("grey");
const QString QZSettings::tile_preset_resistance_5_color = QStringLiteral("tile_preset_resistance_5_color");
const QString QZSettings::default_tile_preset_resistance_5_color = QStringLiteral("grey");
const QString QZSettings::tile_preset_speed_1_color = QStringLiteral("tile_preset_speed_1_color");
const QString QZSettings::default_tile_preset_speed_1_color = QStringLiteral("grey");
const QString QZSettings::tile_preset_speed_2_color = QStringLiteral("tile_preset_speed_2_color");
const QString QZSettings::default_tile_preset_speed_2_color = QStringLiteral("grey");
const QString QZSettings::tile_preset_speed_3_color = QStringLiteral("tile_preset_speed_3_color");
const QString QZSettings::default_tile_preset_speed_3_color = QStringLiteral("grey");
const QString QZSettings::tile_preset_speed_4_color = QStringLiteral("tile_preset_speed_4_color");
const QString QZSettings::default_tile_preset_speed_4_color = QStringLiteral("grey");
const QString QZSettings::tile_preset_speed_5_color = QStringLiteral("tile_preset_speed_5_color");
const QString QZSettings::default_tile_preset_speed_5_color = QStringLiteral("grey");
const QString QZSettings::tile_preset_inclination_1_color = QStringLiteral("tile_preset_inclination_1_color");
const QString QZSettings::default_tile_preset_inclination_1_color = QStringLiteral("grey");
const QString QZSettings::tile_preset_inclination_2_color = QStringLiteral("tile_preset_inclination_2_color");
const QString QZSettings::default_tile_preset_inclination_2_color = QStringLiteral("grey");
const QString QZSettings::tile_preset_inclination_3_color = QStringLiteral("tile_preset_inclination_3_color");
const QString QZSettings::default_tile_preset_inclination_3_color = QStringLiteral("grey");
const QString QZSettings::tile_preset_inclination_4_color = QStringLiteral("tile_preset_inclination_4_color");
const QString QZSettings::default_tile_preset_inclination_4_color = QStringLiteral("grey");
const QString QZSettings::tile_preset_inclination_5_color = QStringLiteral("tile_preset_inclination_5_color");
const QString QZSettings::default_tile_preset_inclination_5_color = QStringLiteral("grey");
const QString QZSettings::tile_avg_watt_lap_enabled = QStringLiteral("tile_avg_watt_lap_enabled");
const QString QZSettings::tile_avg_watt_lap_order = QStringLiteral("tile_avg_watt_lap_order");
const QString QZSettings::nordictrack_t70_treadmill = QStringLiteral("nordictrack_t70_treadmill");
const QString QZSettings::CRRGain = QStringLiteral("crrGain");
const QString QZSettings::CWGain = QStringLiteral("cwGain");
const QString QZSettings::proform_treadmill_cadence_lt = QStringLiteral("proform_treadmill_cadence_lt");
const QString QZSettings::trainprogram_stop_at_end = QStringLiteral("trainprogram_stop_at_end");
const QString QZSettings::domyos_elliptical_inclination = QStringLiteral("domyos_elliptical_inclination");
const QString QZSettings::gpx_loop = QStringLiteral("gpx_loop");
const QString QZSettings::android_notification = QStringLiteral("android_notification");
const QString QZSettings::kingsmith_encrypt_v4 = QStringLiteral("kingsmith_encrypt_v4");
const QString QZSettings::horizon_treadmill_disable_pause = QStringLiteral("horizon_treadmill_disable_pause");
const QString QZSettings::domyos_bike_500_profile_v1 = QStringLiteral("domyos_bike_500_profile_v1");
const QString QZSettings::ss2k_peloton = QStringLiteral("ss2k_peloton");
const QString QZSettings::computrainer_serialport = QStringLiteral("computrainer_serialport");
const QString QZSettings::default_computrainer_serialport = QStringLiteral("");
const QString QZSettings::strava_virtual_activity = QStringLiteral("strava_virtual_activity");
const QString QZSettings::powr_sensor_running_cadence_half_on_strava =
    QStringLiteral("powr_sensor_running_cadence_half_on_strava");
const QString QZSettings::nordictrack_ifit_adb_remote = QStringLiteral("nordictrack_ifit_adb_remote");
const QString QZSettings::floating_height = QStringLiteral("floating_height");
const QString QZSettings::floating_width = QStringLiteral("floating_width");
const QString QZSettings::floating_transparency = QStringLiteral("floating_transparency");
const QString QZSettings::floating_startup = QStringLiteral("floating_startup");
const QString QZSettings::norditrack_s25i_treadmill = QStringLiteral("norditrack_s25i_treadmill");
const QString QZSettings::toorx_ftms_treadmill = QStringLiteral("toorx_ftms_treadmill");
const QString QZSettings::nordictrack_t65s_83_treadmill = QStringLiteral("nordictrack_t65s_83_treadmill");
const QString QZSettings::horizon_treadmill_suspend_stats_pause =
    QStringLiteral("horizon_treadmill_suspend_stats_pause");
const QString QZSettings::sportstech_sx600 = QStringLiteral("sportstech_sx600");
const QString QZSettings::sole_elliptical_inclination = QStringLiteral("sole_elliptical_inclination");
const QString QZSettings::proform_hybrid_trainer_xt = QStringLiteral("proform_hybrid_trainer_xt");
const QString QZSettings::gears_restore_value = QStringLiteral("gears_restore_value");
const QString QZSettings::gears_current_value = QStringLiteral("gears_current_value_f");
const QString QZSettings::tile_pace_last500m_enabled = QStringLiteral("tile_pace_last500m_enabled");
const QString QZSettings::tile_pace_last500m_order = QStringLiteral("tile_pace_last500m_order");
const QString QZSettings::treadmill_difficulty_gain_or_offset = QStringLiteral("treadmill_difficulty_gain_or_offset");
const QString QZSettings::pafers_treadmill_bh_iboxster_plus = QStringLiteral("pafers_treadmill_bh_iboxster_plus");
const QString QZSettings::proform_cycle_trainer_400 = QStringLiteral("proform_cycle_trainer_400");
const QString QZSettings::peloton_workout_ocr = QStringLiteral("peloton_workout_ocr");
const QString QZSettings::peloton_companion_workout_ocr = QStringLiteral("peloton_companion_workout_ocr");
const QString QZSettings::peloton_bike_ocr = QStringLiteral("peloton_bike_ocr");
const QString QZSettings::fitshow_treadmill_miles = QStringLiteral("fitshow_treadmill_miles");
const QString QZSettings::proform_hybrid_trainer_PFEL03815 = QStringLiteral("proform_hybrid_trainer_PFEL03815");
const QString QZSettings::schwinn_resistance_smooth = QStringLiteral("schwinn_resistance_smooth");
const QString QZSettings::treadmill_inclination_override_0 = QStringLiteral("treadmill_inclination_override_0");
const QString QZSettings::treadmill_inclination_override_05 = QStringLiteral("treadmill_inclination_override_05");
const QString QZSettings::treadmill_inclination_override_10 = QStringLiteral("treadmill_inclination_override_10");
const QString QZSettings::treadmill_inclination_override_15 = QStringLiteral("treadmill_inclination_override_15");
const QString QZSettings::treadmill_inclination_override_20 = QStringLiteral("treadmill_inclination_override_20");
const QString QZSettings::treadmill_inclination_override_25 = QStringLiteral("treadmill_inclination_override_25");
const QString QZSettings::treadmill_inclination_override_30 = QStringLiteral("treadmill_inclination_override_30");
const QString QZSettings::treadmill_inclination_override_35 = QStringLiteral("treadmill_inclination_override_35");
const QString QZSettings::treadmill_inclination_override_40 = QStringLiteral("treadmill_inclination_override_40");
const QString QZSettings::treadmill_inclination_override_45 = QStringLiteral("treadmill_inclination_override_45");
const QString QZSettings::treadmill_inclination_override_50 = QStringLiteral("treadmill_inclination_override_50");
const QString QZSettings::treadmill_inclination_override_55 = QStringLiteral("treadmill_inclination_override_55");
const QString QZSettings::treadmill_inclination_override_60 = QStringLiteral("treadmill_inclination_override_60");
const QString QZSettings::treadmill_inclination_override_65 = QStringLiteral("treadmill_inclination_override_65");
const QString QZSettings::treadmill_inclination_override_70 = QStringLiteral("treadmill_inclination_override_70");
const QString QZSettings::treadmill_inclination_override_75 = QStringLiteral("treadmill_inclination_override_75");
const QString QZSettings::treadmill_inclination_override_80 = QStringLiteral("treadmill_inclination_override_80");
const QString QZSettings::treadmill_inclination_override_85 = QStringLiteral("treadmill_inclination_override_85");
const QString QZSettings::treadmill_inclination_override_90 = QStringLiteral("treadmill_inclination_override_90");
const QString QZSettings::treadmill_inclination_override_95 = QStringLiteral("treadmill_inclination_override_95");
const QString QZSettings::treadmill_inclination_override_100 = QStringLiteral("treadmill_inclination_override_100");
const QString QZSettings::treadmill_inclination_override_105 = QStringLiteral("treadmill_inclination_override_105");
const QString QZSettings::treadmill_inclination_override_110 = QStringLiteral("treadmill_inclination_override_110");
const QString QZSettings::treadmill_inclination_override_115 = QStringLiteral("treadmill_inclination_override_115");
const QString QZSettings::treadmill_inclination_override_120 = QStringLiteral("treadmill_inclination_override_120");
const QString QZSettings::treadmill_inclination_override_125 = QStringLiteral("treadmill_inclination_override_125");
const QString QZSettings::treadmill_inclination_override_130 = QStringLiteral("treadmill_inclination_override_130");
const QString QZSettings::treadmill_inclination_override_135 = QStringLiteral("treadmill_inclination_override_135");
const QString QZSettings::treadmill_inclination_override_140 = QStringLiteral("treadmill_inclination_override_140");
const QString QZSettings::treadmill_inclination_override_145 = QStringLiteral("treadmill_inclination_override_145");
const QString QZSettings::treadmill_inclination_override_150 = QStringLiteral("treadmill_inclination_override_150");
const QString QZSettings::sole_elliptical_e55 = QStringLiteral("sole_elliptical_e55");
const QString QZSettings::horizon_treadmill_force_ftms = QStringLiteral("horizon_treadmill_force_ftms");
const QString QZSettings::horizon_treadmill_7_0_at_24 = QStringLiteral("horizon_treadmill_7_0_at_24");
const QString QZSettings::treadmill_pid_heart_min = QStringLiteral("treadmill_pid_heart_min");
const QString QZSettings::treadmill_pid_heart_max = QStringLiteral("treadmill_pid_heart_max");
const QString QZSettings::nordictrack_elliptical_c7_5 = QStringLiteral("nordictrack_elliptical_c7_5");
const QString QZSettings::renpho_bike_double_resistance = QStringLiteral("renpho_bike_double_resistance");
const QString QZSettings::nordictrack_incline_trainer_x7i = QStringLiteral("nordictrack_incline_trainer_x7i");
const QString QZSettings::strava_auth_external_webbrowser = QStringLiteral("strava_auth_external_webbrowser");
const QString QZSettings::gears_from_bike = QStringLiteral("gears_from_bike");
const QString QZSettings::peloton_spinups_autoresistance = QStringLiteral("peloton_spinups_autoresistance");
const QString QZSettings::eslinker_costaway = QStringLiteral("eslinker_costaway");
const QString QZSettings::treadmill_inclination_ovveride_gain = QStringLiteral("treadmill_inclination_ovveride_gain");
const QString QZSettings::treadmill_inclination_ovveride_offset =
    QStringLiteral("treadmill_inclination_ovveride_offset");
const QString QZSettings::bh_spada_2_watt = QStringLiteral("bh_spada_2_watt");
const QString QZSettings::tacx_neo2_peloton = QStringLiteral("tacx_neo2_peloton");
const QString QZSettings::sole_treadmill_inclination_fast = QStringLiteral("sole_treadmill_inclination_fast");
const QString QZSettings::zwift_ocr = QStringLiteral("zwift_ocr");
const QString QZSettings::fit_file_saved_on_quit = QStringLiteral("fit_file_saved_on_quit");
const QString QZSettings::gem_module_inclination = QStringLiteral("gem_module_inclination");
const QString QZSettings::treadmill_simulate_inclination_with_speed =
    QStringLiteral("treadmill_simulate_inclination_with_speed");
const QString QZSettings::garmin_companion = QStringLiteral("garmin_companion");
const QString QZSettings::iconcept_elliptical = QStringLiteral("iconcept_elliptical");
const QString QZSettings::gears_gain = QStringLiteral("gears_gain");
const QString QZSettings::proform_treadmill_8_0 = QStringLiteral("proform_treadmill_8_0");
const QString QZSettings::zero_zt2500_treadmill = QStringLiteral("zero_zt2500_treadmill");
const QString QZSettings::kingsmith_encrypt_v5 = QStringLiteral("kingsmith_encrypt_v5");
const QString QZSettings::peloton_rower_level = QStringLiteral("peloton_rower_level");
const QString QZSettings::tile_target_pace_enabled = QStringLiteral("tile_target_pace_enabled");
const QString QZSettings::tile_target_pace_order = QStringLiteral("tile_target_pace_order");
const QString QZSettings::tts_act_target_pace = QStringLiteral("tts_act_target_pace");
const QString QZSettings::csafe_rower = QStringLiteral("csafe_rower");
const QString QZSettings::default_csafe_rower = QStringLiteral("");
const QString QZSettings::csafe_elliptical_port = QStringLiteral("csafe_elliptical_port");
const QString QZSettings::default_csafe_elliptical_port = QStringLiteral("");
const QString QZSettings::ftms_rower = QStringLiteral("ftms_rower");
const QString QZSettings::default_ftms_rower = QStringLiteral("Disabled");
const QString QZSettings::zwift_workout_ocr = QStringLiteral("zwift_workout_ocr");
const QString QZSettings::proform_bike_sb = QStringLiteral("proform_bike_sb");
const QString QZSettings::fakedevice_rower = QStringLiteral("fakedevice_rower");
const QString QZSettings::zwift_ocr_climb_portal = QStringLiteral("zwift_ocr_climb_portal");
const QString QZSettings::poll_device_time = QStringLiteral("poll_device_time");
const QString QZSettings::proform_bike_PFEVEX71316_1 = QStringLiteral("proform_bike_PFEVEX71316_1");
const QString QZSettings::schwinn_bike_resistance_v3 = QStringLiteral("schwinn_bike_resistance_v3");
const QString QZSettings::watt_ignore_builtin = QStringLiteral("watt_ignore_builtin");
const QString QZSettings::proform_treadmill_z1300i = QStringLiteral("proform_treadmill_z1300i");
const QString QZSettings::ftms_bike = QStringLiteral("ftms_bike");
const QString QZSettings::default_ftms_bike = QStringLiteral("Disabled");
const QString QZSettings::ftms_treadmill = QStringLiteral("ftms_treadmill");
const QString QZSettings::default_ftms_treadmill = QStringLiteral("Disabled");
const QString QZSettings::ant_speed_offset = QStringLiteral("ant_speed_offset");
const QString QZSettings::ant_speed_gain = QStringLiteral("ant_speed_gain");
const QString QZSettings::proform_rower_sport_rl = QStringLiteral("proform_rower_sport_rl");
const QString QZSettings::strava_date_prefix = QStringLiteral("strava_date_prefix");
const QString QZSettings::race_mode = QStringLiteral("race_mode");
const QString QZSettings::proform_pro_1000_treadmill = QStringLiteral("proform_pro_1000_treadmill");
const QString QZSettings::saris_trainer = QStringLiteral("saris_trainer");
const QString QZSettings::proform_studio_NTEX71021 = QStringLiteral("proform_studio_NTEX71021");
const QString QZSettings::nordictrack_x22i = QStringLiteral("nordictrack_x22i");
const QString QZSettings::iconsole_elliptical = QStringLiteral("iconsole_elliptical");
const QString QZSettings::autolap_distance = QStringLiteral("autolap_distance");
const QString QZSettings::nordictrack_s20_treadmill = QStringLiteral("nordictrack_s20_treadmill");
const QString QZSettings::freemotion_coachbike_b22_7 = QStringLiteral("freemotion_coachbike_b22_7");
const QString QZSettings::proform_cycle_trainer_300_ci = QStringLiteral("proform_cycle_trainer_300_ci");
const QString QZSettings::kingsmith_encrypt_g1_walking_pad = QStringLiteral("kingsmith_encrypt_g1_walking_pad");
const QString QZSettings::proformtdf1ip = QStringLiteral("proformtdf1ip");
const QString QZSettings::default_proformtdf1ip = QStringLiteral("");
const QString QZSettings::proform_bike_225_csx = QStringLiteral("proform_bike_225_csx");
const QString QZSettings::proform_treadmill_l6_0s = QStringLiteral("proform_treadmill_l6_0s");
const QString QZSettings::zwift_username = QStringLiteral("zwift_username");
const QString QZSettings::default_zwift_username = QStringLiteral("");
const QString QZSettings::zwift_password = QStringLiteral("zwift_password");
const QString QZSettings::default_zwift_password = QStringLiteral("");
const QString QZSettings::garmin_bluetooth_compatibility = QStringLiteral("garmin_bluetooth_compatibility");
const QString QZSettings::norditrack_s25_treadmill = QStringLiteral("norditrack_s25_treadmill");
const QString QZSettings::proform_8_5_treadmill = QStringLiteral("proform_8_5_treadmill");
const QString QZSettings::treadmill_incline_min = QStringLiteral("treadmill_incline_min");
const QString QZSettings::treadmill_incline_max = QStringLiteral("treadmill_incline_max");
const QString QZSettings::proform_2000_treadmill = QStringLiteral("proform_2000_treadmill");
const QString QZSettings::android_documents_folder = QStringLiteral("android_documents_folder");
const QString QZSettings::zwift_api_autoinclination = QStringLiteral("zwift_api_autoinclination");
const QString QZSettings::domyos_treadmill_button_5kmh = QStringLiteral("domyos_treadmill_button_5kmh");
const QString QZSettings::domyos_treadmill_button_10kmh = QStringLiteral("domyos_treadmill_button_10kmh");
const QString QZSettings::domyos_treadmill_button_16kmh = QStringLiteral("domyos_treadmill_button_16kmh");
const QString QZSettings::domyos_treadmill_button_22kmh = QStringLiteral("domyos_treadmill_button_22kmh");
const QString QZSettings::proform_treadmill_sport_8_5 = QStringLiteral("proform_treadmill_sport_8_5");
const QString QZSettings::domyos_treadmill_t900a = QStringLiteral("domyos_treadmill_t900a");
const QString QZSettings::enerfit_SPX_9500 = QStringLiteral("enerfit_SPX_9500");
const QString QZSettings::proform_treadmill_505_cst = QStringLiteral("proform_treadmill_505_cst");
const QString QZSettings::nordictrack_treadmill_t8_5s = QStringLiteral("nordictrack_treadmill_t8_5s");
const QString QZSettings::proform_treadmill_705_cst = QStringLiteral("proform_treadmill_705_cst");
const QString QZSettings::zwift_click = QStringLiteral("zwift_click");
const QString QZSettings::hop_sport_hs_090h_bike = QStringLiteral("hop_sport_hs_090h_bike");
const QString QZSettings::zwift_play = QStringLiteral("zwift_play");
const QString QZSettings::nordictrack_treadmill_x14i = QStringLiteral("nordictrack_treadmill_x14i");
const QString QZSettings::zwift_api_poll = QStringLiteral("zwift_api_poll");
const QString QZSettings::tile_step_count_enabled = QStringLiteral("tile_step_count_enabled");
const QString QZSettings::tile_step_count_order = QStringLiteral("tile_step_count_order");
const QString QZSettings::tile_erg_mode_enabled = QStringLiteral("tile_erg_mode_enabled");
const QString QZSettings::tile_erg_mode_order = QStringLiteral("tile_erg_mode_order");
const QString QZSettings::toorx_srx_3500 = QStringLiteral("toorx_srx_3500");
const QString QZSettings::stryd_speed_instead_treadmill = QStringLiteral("stryd_speed_instead_treadmill");
const QString QZSettings::inclination_delay_seconds = QStringLiteral("inclination_delay_seconds");
const QString QZSettings::ergDataPoints = QStringLiteral("ergDataPoints");
const QString QZSettings::default_ergDataPoints = QStringLiteral("");
const QString QZSettings::proform_carbon_tl = QStringLiteral("proform_carbon_tl");
const QString QZSettings::proform_proshox2 = QStringLiteral("proform_proshox2");
const QString QZSettings::proform_tdf_10_0 = QStringLiteral("proform_tdf_10_0");
const QString QZSettings::nordictrack_GX4_5_bike = QStringLiteral("nordictrack_GX4_5_bike");
const QString QZSettings::ftp_run = QStringLiteral("ftp_run");
const QString QZSettings::tile_rss_enabled = QStringLiteral("tile_rss_enabled");
const QString QZSettings::tile_rss_order = QStringLiteral("tile_rss_order");
const QString QZSettings::treadmillDataPoints = QStringLiteral("treadmillDataPoints");
const QString QZSettings::default_treadmillDataPoints = QStringLiteral("");
const QString QZSettings::nordictrack_s20i_treadmill = QStringLiteral("nordictrack_s20i_treadmill");
const QString QZSettings::proform_595i_proshox2 = QStringLiteral("proform_595i_proshox2");
const QString QZSettings::proform_treadmill_8_7 = QStringLiteral("proform_treadmill_8_7");
const QString QZSettings::proform_bike_325_csx = QStringLiteral("proform_bike_325_csx");
const QString QZSettings::strava_upload_mode = QStringLiteral("strava_upload_mode");
const QString QZSettings::default_strava_upload_mode = QStringLiteral("Always");
const QString QZSettings::proform_treadmill_705_cst_V78_239 = QStringLiteral("proform_treadmill_705_cst_V78_239");
const QString QZSettings::stryd_add_inclination_gain = QStringLiteral("stryd_add_inclination_gain");
const QString QZSettings::toorx_bike_srx_500 = QStringLiteral("toorx_bike_srx_500");
const QString QZSettings::atletica_lightspeed_treadmill = QStringLiteral("atletica_lightspeed_treadmill");
const QString QZSettings::peloton_treadmill_level = QStringLiteral("peloton_treadmill_level");
const QString QZSettings::nordictrackadbbike_resistance = QStringLiteral("nordictrackadbbike_resistance");
const QString QZSettings::proform_treadmill_carbon_t7 = QStringLiteral("proform_treadmill_carbon_t7");
const QString QZSettings::nordictrack_treadmill_exp_5i = QStringLiteral("nordictrack_treadmill_exp_5i");
const QString QZSettings::dircon_id = QStringLiteral("dircon_id");
const QString QZSettings::proform_elliptical_ip = QStringLiteral("proform_elliptical_ip");
const QString QZSettings::default_proform_elliptical_ip = QStringLiteral("");
const QString QZSettings::antbike = QStringLiteral("antbike");
const QString QZSettings::domyosbike_notfmts = QStringLiteral("domyosbike_notfmts");
const QString QZSettings::gears_volume_debouncing = QStringLiteral("gears_volume_debouncing");
const QString QZSettings::tile_biggears_enabled = QStringLiteral("tile_biggears_enabled");
const QString QZSettings::tile_biggears_order = QStringLiteral("tile_biggears_order");
const QString QZSettings::domyostreadmill_notfmts = QStringLiteral("domyostreadmill_notfmts");
const QString QZSettings::zwiftplay_swap = QStringLiteral("zwiftplay_swap");
const QString QZSettings::gears_zwift_ratio = QStringLiteral("gears_zwift_ratio");
const QString QZSettings::domyos_bike_500_profile_v2 = QStringLiteral("domyos_bike_500_profile_v2");
const QString QZSettings::gears_offset = QStringLiteral("gears_offset");
const QString QZSettings::peloton_accesstoken = QStringLiteral("peloton_accesstoken");
const QString QZSettings::default_peloton_accesstoken = QStringLiteral("");
const QString QZSettings::peloton_refreshtoken = QStringLiteral("peloton_refreshtoken");
const QString QZSettings::default_peloton_refreshtoken = QStringLiteral("");
const QString QZSettings::peloton_lastrefresh = QStringLiteral("peloton_lastrefresh");
const QString QZSettings::default_peloton_lastrefresh = QStringLiteral("");
const QString QZSettings::peloton_expires = QStringLiteral("peloton_expires");
const QString QZSettings::default_peloton_expires = QStringLiteral("");
const QString QZSettings::peloton_code = QStringLiteral("peloton_code");
const QString QZSettings::default_peloton_code = QStringLiteral("");
const QString QZSettings::proform_carbon_tl_PFTL59720 = QStringLiteral("proform_carbon_tl_PFTL59720");
const QString QZSettings::proform_treadmill_sport_70 = QStringLiteral("proform_treadmill_sport_70");
const QString QZSettings::peloton_date_format = QStringLiteral("peloton_date_format");
const QString QZSettings::default_peloton_date_format = QStringLiteral("MM/dd/yy");
const QString QZSettings::force_resistance_instead_inclination = QStringLiteral("force_resistance_instead_inclination");
const QString QZSettings::proform_treadmill_575i = QStringLiteral("proform_treadmill_575i");
const QString QZSettings::zwift_play_emulator = QStringLiteral("zwift_play_emulator");
const QString QZSettings::gear_configuration = QStringLiteral("gear_configuration");
const QString QZSettings::default_gear_configuration = QStringLiteral("1|38|44|true\n2|38|38|true\n3|38|32|true\n4|38|28|true\n5|38|24|true\n6|38|21|true\n7|38|19|true\n8|38|17|true\n9|38|15|true\n10|38|13|true\n11|38|11|true\n12|38|10|true");
const QString QZSettings::gear_crankset_size = QStringLiteral("gear_crankset_size");
const QString QZSettings::gear_cog_size = QStringLiteral("gear_cog_size");
const QString QZSettings::gear_wheel_size = QStringLiteral("gear_wheel_size");
const QString QZSettings::default_gear_wheel_size = QStringLiteral("700 x 18C");
const QString QZSettings::gear_circumference = QStringLiteral("gear_circumference");
const QString QZSettings::watt_bike_emulator = QStringLiteral("watt_bike_emulator");
const QString QZSettings::restore_specific_gear = QStringLiteral("restore_specific_gear");
const QString QZSettings::skipLocationServicesDialog = QStringLiteral("skipLocationServicesDialog");
const QString QZSettings::trainprogram_pid_pushy = QStringLiteral("trainprogram_pid_pushy");
const QString QZSettings::min_inclination = QStringLiteral("min_inclination");
const QString QZSettings::proform_performance_300i = QStringLiteral("proform_performance_300i");
const QString QZSettings::proform_performance_400i = QStringLiteral("proform_performance_400i");
const QString QZSettings::proform_treadmill_c700 = QStringLiteral("proform_treadmill_c700");
const QString QZSettings::sram_axs_controller = QStringLiteral("sram_axs_controller");
const QString QZSettings::proform_treadmill_c960i = QStringLiteral("proform_treadmill_c960i");
const QString QZSettings::mqtt_host = QStringLiteral("mqtt_host");
const QString QZSettings::default_mqtt_host = QStringLiteral("");
const QString QZSettings::mqtt_port = QStringLiteral("mqtt_port");
const QString QZSettings::mqtt_username = QStringLiteral("mqtt_username");
const QString QZSettings::mqtt_password = QStringLiteral("mqtt_password");
const QString QZSettings::mqtt_deviceid = QStringLiteral("mqtt_deviceid");
const QString QZSettings::default_mqtt_username = QStringLiteral("");
const QString QZSettings::default_mqtt_password = QStringLiteral("");
const QString QZSettings::default_mqtt_deviceid = QStringLiteral("default");
const QString QZSettings::peloton_auto_start_with_intro = QStringLiteral("peloton_auto_start_with_intro");
const QString QZSettings::peloton_auto_start_without_intro = QStringLiteral("peloton_auto_start_without_intro");
const QString QZSettings::nordictrack_tseries5_treadmill = QStringLiteral("nordictrack_tseries5_treadmill");
const QString QZSettings::proform_carbon_tl_PFTL59722c = QStringLiteral("proform_carbon_tl_PFTL59722c");
const QString QZSettings::nordictrack_gx_44_pro = QStringLiteral("nordictrack_gx_44_pro");
const QString QZSettings::OSC_ip = QStringLiteral("osc_ip");
const QString QZSettings::default_OSC_ip = QStringLiteral("");
const QString QZSettings::OSC_port = QStringLiteral("osc_port");
const QString QZSettings::strava_treadmill = QStringLiteral("strava_treadmill");
const QString QZSettings::iconsole_rower = QStringLiteral("iconsole_rower");
const QString QZSettings::proform_treadmill_1500_pro = QStringLiteral("proform_treadmill_1500_pro");
const QString QZSettings::proform_505_cst_80_44 = QStringLiteral("proform_505_cst_80_44");
const QString QZSettings::proform_trainer_8_0 = QStringLiteral("proform_trainer_8_0");
const QString QZSettings::tile_biggears_swap = QStringLiteral("tile_biggears_swap");
const QString QZSettings::treadmill_follow_wattage = QStringLiteral("treadmill_follow_wattage");
const QString QZSettings::fit_file_garmin_device_training_effect = QStringLiteral("fit_file_garmin_device_training_effect");
const QString QZSettings::proform_treadmill_705_cst_V80_44 = QStringLiteral("proform_treadmill_705_cst_V80_44");
const QString QZSettings::nordictrack_treadmill_1750_adb = QStringLiteral("nordictrack_treadmill_1750_adb");

const QString QZSettings::tile_preset_powerzone_1_enabled = QStringLiteral("tile_preset_powerzone_1_enabled");
const QString QZSettings::tile_preset_powerzone_1_order = QStringLiteral("tile_preset_powerzone_1_order");
const QString QZSettings::tile_preset_powerzone_1_value = QStringLiteral("tile_preset_powerzone_1_value");
const QString QZSettings::tile_preset_powerzone_1_label = QStringLiteral("tile_preset_powerzone_1_label");
const QString QZSettings::default_tile_preset_powerzone_1_label = QStringLiteral("Zone 1");
const QString QZSettings::tile_preset_powerzone_1_color = QStringLiteral("tile_preset_powerzone_1_color");
const QString QZSettings::default_tile_preset_powerzone_1_color = QStringLiteral("white");

const QString QZSettings::tile_preset_powerzone_2_enabled = QStringLiteral("tile_preset_powerzone_2_enabled");
const QString QZSettings::tile_preset_powerzone_2_order = QStringLiteral("tile_preset_powerzone_2_order");
const QString QZSettings::tile_preset_powerzone_2_value = QStringLiteral("tile_preset_powerzone_2_value");
const QString QZSettings::tile_preset_powerzone_2_label = QStringLiteral("tile_preset_powerzone_2_label");
const QString QZSettings::default_tile_preset_powerzone_2_label = QStringLiteral("Zone 2");
const QString QZSettings::tile_preset_powerzone_2_color = QStringLiteral("tile_preset_powerzone_2_color");
const QString QZSettings::default_tile_preset_powerzone_2_color = QStringLiteral("limegreen");

const QString QZSettings::tile_preset_powerzone_3_enabled = QStringLiteral("tile_preset_powerzone_3_enabled");
const QString QZSettings::tile_preset_powerzone_3_order = QStringLiteral("tile_preset_powerzone_3_order");
const QString QZSettings::tile_preset_powerzone_3_value = QStringLiteral("tile_preset_powerzone_3_value");
const QString QZSettings::tile_preset_powerzone_3_label = QStringLiteral("tile_preset_powerzone_3_label");
const QString QZSettings::default_tile_preset_powerzone_3_label = QStringLiteral("Zone 3");
const QString QZSettings::tile_preset_powerzone_3_color = QStringLiteral("tile_preset_powerzone_3_color");
const QString QZSettings::default_tile_preset_powerzone_3_color = QStringLiteral("gold");

const QString QZSettings::tile_preset_powerzone_4_enabled = QStringLiteral("tile_preset_powerzone_4_enabled");
const QString QZSettings::tile_preset_powerzone_4_order = QStringLiteral("tile_preset_powerzone_4_order");
const QString QZSettings::tile_preset_powerzone_4_value = QStringLiteral("tile_preset_powerzone_4_value");
const QString QZSettings::tile_preset_powerzone_4_label = QStringLiteral("tile_preset_powerzone_4_label");
const QString QZSettings::default_tile_preset_powerzone_4_label = QStringLiteral("Zone 4");
const QString QZSettings::tile_preset_powerzone_4_color = QStringLiteral("tile_preset_powerzone_4_color");
const QString QZSettings::default_tile_preset_powerzone_4_color = QStringLiteral("orange");

const QString QZSettings::tile_preset_powerzone_5_enabled = QStringLiteral("tile_preset_powerzone_5_enabled");
const QString QZSettings::tile_preset_powerzone_5_order = QStringLiteral("tile_preset_powerzone_5_order");
const QString QZSettings::tile_preset_powerzone_5_value = QStringLiteral("tile_preset_powerzone_5_value");
const QString QZSettings::tile_preset_powerzone_5_label = QStringLiteral("tile_preset_powerzone_5_label");
const QString QZSettings::default_tile_preset_powerzone_5_label = QStringLiteral("Zone 5");
const QString QZSettings::tile_preset_powerzone_5_color = QStringLiteral("tile_preset_powerzone_5_color");
const QString QZSettings::default_tile_preset_powerzone_5_color = QStringLiteral("darkorange");

const QString QZSettings::tile_preset_powerzone_6_enabled = QStringLiteral("tile_preset_powerzone_6_enabled");
const QString QZSettings::tile_preset_powerzone_6_order = QStringLiteral("tile_preset_powerzone_6_order");
const QString QZSettings::tile_preset_powerzone_6_value = QStringLiteral("tile_preset_powerzone_6_value");
const QString QZSettings::tile_preset_powerzone_6_label = QStringLiteral("tile_preset_powerzone_6_label");
const QString QZSettings::default_tile_preset_powerzone_6_label = QStringLiteral("Zone 6");
const QString QZSettings::tile_preset_powerzone_6_color = QStringLiteral("tile_preset_powerzone_6_color");
const QString QZSettings::default_tile_preset_powerzone_6_color = QStringLiteral("orangered");

const QString QZSettings::tile_preset_powerzone_7_enabled = QStringLiteral("tile_preset_powerzone_7_enabled");
const QString QZSettings::tile_preset_powerzone_7_order = QStringLiteral("tile_preset_powerzone_7_order");
const QString QZSettings::tile_preset_powerzone_7_value = QStringLiteral("tile_preset_powerzone_7_value");
const QString QZSettings::tile_preset_powerzone_7_label = QStringLiteral("tile_preset_powerzone_7_label");
const QString QZSettings::default_tile_preset_powerzone_7_label = QStringLiteral("Zone 7");
const QString QZSettings::tile_preset_powerzone_7_color = QStringLiteral("tile_preset_powerzone_7_color");
const QString QZSettings::default_tile_preset_powerzone_7_color = QStringLiteral("red");

const QString QZSettings::proform_bike_PFEVEX71316_0 = QStringLiteral("proform_bike_PFEVEX71316_0");

const QString QZSettings::real_inclination_to_virtual_treamill_bridge = QStringLiteral("real_inclination_to_virtual_treamill_bridge");
const QString QZSettings::stryd_inclination_instead_treadmill = QStringLiteral("stryd_inclination_instead_treadmill");
const QString QZSettings::domyos_elliptical_fmts = QStringLiteral("domyos_elliptical_fmts");
const QString QZSettings::proform_xbike = QStringLiteral("proform_xbike");
const QString QZSettings::peloton_current_user_id = QStringLiteral("peloton_current_user_id");
const QString QZSettings::default_peloton_current_user_id = QStringLiteral("");
const QString QZSettings::proform_225_csx_PFEX32925_INT_0 = QStringLiteral("proform_225_csx_PFEX32925_INT_0");
const QString QZSettings::trainprogram_pid_ignore_inclination = QStringLiteral("trainprogram_pid_ignore_inclination");
const QString QZSettings::android_antbike = QStringLiteral("android_antbike");

const QString QZSettings::tile_hr_time_in_zone_1_enabled = QStringLiteral("tile_hr_time_in_zone_1_enabled");
const QString QZSettings::tile_hr_time_in_zone_1_order = QStringLiteral("tile_hr_time_in_zone_1_order");

const QString QZSettings::tile_hr_time_in_zone_2_enabled = QStringLiteral("tile_hr_time_in_zone_2_enabled");
const QString QZSettings::tile_hr_time_in_zone_2_order = QStringLiteral("tile_hr_time_in_zone_2_order");

const QString QZSettings::tile_hr_time_in_zone_3_enabled = QStringLiteral("tile_hr_time_in_zone_3_enabled");
const QString QZSettings::tile_hr_time_in_zone_3_order = QStringLiteral("tile_hr_time_in_zone_3_order");

const QString QZSettings::tile_hr_time_in_zone_4_enabled = QStringLiteral("tile_hr_time_in_zone_4_enabled");
const QString QZSettings::tile_hr_time_in_zone_4_order = QStringLiteral("tile_hr_time_in_zone_4_order");

const QString QZSettings::tile_hr_time_in_zone_5_enabled = QStringLiteral("tile_hr_time_in_zone_5_enabled");
const QString QZSettings::tile_hr_time_in_zone_5_order = QStringLiteral("tile_hr_time_in_zone_5_order");

const QString QZSettings::zwift_gear_ui_aligned = QStringLiteral("zwift_gear_ui_aligned");
const QString QZSettings::tacxneo2_disable_negative_inclination = QStringLiteral("tacxneo2_disable_negative_inclination");

const QString QZSettings::tile_coretemperature_enabled = QStringLiteral("tile_coretemperature_enabled");
const QString QZSettings::tile_coretemperature_order = QStringLiteral("tile_coretemperature_order");

const QString QZSettings::nordictrack_t65s_treadmill_81_miles = QStringLiteral("nordictrack_t65s_treadmill_81_miles");
const QString QZSettings::nordictrack_elite_800 = QStringLiteral("nordictrack_elite_800");

const QString QZSettings::ios_btdevice_native = QStringLiteral("ios_btdevice_native");

const QString QZSettings::inclinationResistancePoints = QStringLiteral("inclinationResistancePoints");
const QString QZSettings::default_inclinationResistancePoints = QStringLiteral("");

const QString QZSettings::floatingwindow_type = QStringLiteral("floatingwindow_type");


const uint32_t allSettingsCount = 759;

QVariant allSettings[allSettingsCount][2] = {
    {QZSettings::cryptoKeySettingsProfiles, QZSettings::default_cryptoKeySettingsProfiles},
    {QZSettings::bluetooth_no_reconnection, QZSettings::default_bluetooth_no_reconnection},
    {QZSettings::bike_wheel_revs, QZSettings::default_bike_wheel_revs},
    {QZSettings::bluetooth_lastdevice_name, QZSettings::default_bluetooth_lastdevice_name},
    {QZSettings::bluetooth_lastdevice_address, QZSettings::default_bluetooth_lastdevice_address},
    {QZSettings::hrm_lastdevice_name, QZSettings::default_hrm_lastdevice_name},
    {QZSettings::hrm_lastdevice_address, QZSettings::default_hrm_lastdevice_address},
    {QZSettings::ftms_accessory_address, QZSettings::default_ftms_accessory_address},
    {QZSettings::ftms_accessory_lastdevice_name, QZSettings::default_ftms_accessory_lastdevice_name},
    {QZSettings::csc_sensor_address, QZSettings::default_csc_sensor_address},
    {QZSettings::csc_sensor_lastdevice_name, QZSettings::default_csc_sensor_lastdevice_name},
    {QZSettings::power_sensor_lastdevice_name, QZSettings::default_power_sensor_lastdevice_name},
    {QZSettings::power_sensor_address, QZSettings::default_power_sensor_address},
    {QZSettings::elite_rizer_lastdevice_name, QZSettings::default_elite_rizer_lastdevice_name},
    {QZSettings::elite_rizer_address, QZSettings::default_elite_rizer_address},
    {QZSettings::elite_sterzo_smart_lastdevice_name, QZSettings::default_elite_sterzo_smart_lastdevice_name},
    {QZSettings::elite_sterzo_smart_address, QZSettings::default_elite_sterzo_smart_address},
    {QZSettings::strava_accesstoken, QZSettings::default_strava_accesstoken},
    {QZSettings::strava_refreshtoken, QZSettings::default_strava_refreshtoken},
    {QZSettings::strava_lastrefresh, QZSettings::default_strava_lastrefresh},
    {QZSettings::strava_expires, QZSettings::default_strava_expires},
    {QZSettings::ui_zoom, QZSettings::default_ui_zoom},
    {QZSettings::bike_heartrate_service, QZSettings::default_bike_heartrate_service},
    {QZSettings::bike_resistance_offset, QZSettings::default_bike_resistance_offset},
    {QZSettings::bike_resistance_gain_f, QZSettings::default_bike_resistance_gain_f},
    {QZSettings::zwift_erg, QZSettings::default_zwift_erg},
    {QZSettings::zwift_erg_filter, QZSettings::default_zwift_erg_filter},
    {QZSettings::zwift_erg_filter_down, QZSettings::default_zwift_erg_filter_down},
    {QZSettings::zwift_negative_inclination_x2, QZSettings::default_zwift_negative_inclination_x2},
    {QZSettings::zwift_inclination_offset, QZSettings::default_zwift_inclination_offset},
    {QZSettings::zwift_inclination_gain, QZSettings::default_zwift_inclination_gain},
    {QZSettings::echelon_resistance_offset, QZSettings::default_echelon_resistance_offset},
    {QZSettings::echelon_resistance_gain, QZSettings::default_echelon_resistance_gain},
    {QZSettings::speed_power_based, QZSettings::default_speed_power_based},
    {QZSettings::bike_resistance_start, QZSettings::default_bike_resistance_start},
    {QZSettings::age, QZSettings::default_age},
    {QZSettings::weight, QZSettings::default_weight},
    {QZSettings::ftp, QZSettings::default_ftp},
    {QZSettings::user_email, QZSettings::default_user_email},
    {QZSettings::user_nickname, QZSettings::default_user_nickname},
    {QZSettings::miles_unit, QZSettings::default_miles_unit},
    {QZSettings::pause_on_start, QZSettings::default_pause_on_start},
    {QZSettings::treadmill_force_speed, QZSettings::default_treadmill_force_speed},
    {QZSettings::pause_on_start_treadmill, QZSettings::default_pause_on_start_treadmill},
    {QZSettings::continuous_moving, QZSettings::default_continuous_moving},
    {QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor},
    {QZSettings::run_cadence_sensor, QZSettings::default_run_cadence_sensor},
    {QZSettings::bike_power_sensor, QZSettings::default_bike_power_sensor},
    {QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name},
    {QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin},
    {QZSettings::kcal_ignore_builtin, QZSettings::default_kcal_ignore_builtin},
    {QZSettings::ant_cadence, QZSettings::default_ant_cadence},
    {QZSettings::ant_heart, QZSettings::default_ant_heart},
    {QZSettings::ant_garmin, QZSettings::default_ant_garmin},
    {QZSettings::top_bar_enabled, QZSettings::default_top_bar_enabled},
    {QZSettings::peloton_username, QZSettings::default_peloton_username},
    {QZSettings::peloton_password, QZSettings::default_peloton_password},
    {QZSettings::peloton_difficulty, QZSettings::default_peloton_difficulty},
    {QZSettings::peloton_cadence_metric, QZSettings::default_peloton_cadence_metric},
    {QZSettings::peloton_heartrate_metric, QZSettings::default_peloton_heartrate_metric},
    {QZSettings::peloton_date, QZSettings::default_peloton_date},
    {QZSettings::peloton_description_link, QZSettings::default_peloton_description_link},
    {QZSettings::pzp_username, QZSettings::default_pzp_username},
    {QZSettings::pzp_password, QZSettings::default_pzp_password},
    {QZSettings::tile_speed_enabled, QZSettings::default_tile_speed_enabled},
    {QZSettings::tile_speed_order, QZSettings::default_tile_speed_order},
    {QZSettings::tile_inclination_enabled, QZSettings::default_tile_inclination_enabled},
    {QZSettings::tile_inclination_order, QZSettings::default_tile_inclination_order},
    {QZSettings::tile_cadence_enabled, QZSettings::default_tile_cadence_enabled},
    {QZSettings::tile_cadence_order, QZSettings::default_tile_cadence_order},
    {QZSettings::tile_elevation_enabled, QZSettings::default_tile_elevation_enabled},
    {QZSettings::tile_elevation_order, QZSettings::default_tile_elevation_order},
    {QZSettings::tile_calories_enabled, QZSettings::default_tile_calories_enabled},
    {QZSettings::tile_calories_order, QZSettings::default_tile_calories_order},
    {QZSettings::tile_odometer_enabled, QZSettings::default_tile_odometer_enabled},
    {QZSettings::tile_odometer_order, QZSettings::default_tile_odometer_order},
    {QZSettings::tile_pace_enabled, QZSettings::default_tile_pace_enabled},
    {QZSettings::tile_pace_order, QZSettings::default_tile_pace_order},
    {QZSettings::tile_resistance_enabled, QZSettings::default_tile_resistance_enabled},
    {QZSettings::tile_resistance_order, QZSettings::default_tile_resistance_order},
    {QZSettings::tile_watt_enabled, QZSettings::default_tile_watt_enabled},
    {QZSettings::tile_watt_order, QZSettings::default_tile_watt_order},
    {QZSettings::tile_weight_loss_enabled, QZSettings::default_tile_weight_loss_enabled},
    {QZSettings::tile_weight_loss_order, QZSettings::default_tile_weight_loss_order},
    {QZSettings::tile_avgwatt_enabled, QZSettings::default_tile_avgwatt_enabled},
    {QZSettings::tile_avgwatt_order, QZSettings::default_tile_avgwatt_order},
    {QZSettings::tile_ftp_enabled, QZSettings::default_tile_ftp_enabled},
    {QZSettings::tile_ftp_order, QZSettings::default_tile_ftp_order},
    {QZSettings::tile_heart_enabled, QZSettings::default_tile_heart_enabled},
    {QZSettings::tile_heart_order, QZSettings::default_tile_heart_order},
    {QZSettings::tile_fan_enabled, QZSettings::default_tile_fan_enabled},
    {QZSettings::tile_fan_order, QZSettings::default_tile_fan_order},
    {QZSettings::tile_jouls_enabled, QZSettings::default_tile_jouls_enabled},
    {QZSettings::tile_jouls_order, QZSettings::default_tile_jouls_order},
    {QZSettings::tile_elapsed_enabled, QZSettings::default_tile_elapsed_enabled},
    {QZSettings::tile_elapsed_order, QZSettings::default_tile_elapsed_order},
    {QZSettings::tile_lapelapsed_enabled, QZSettings::default_tile_lapelapsed_enabled},
    {QZSettings::tile_lapelapsed_order, QZSettings::default_tile_lapelapsed_order},
    {QZSettings::tile_moving_time_enabled, QZSettings::default_tile_moving_time_enabled},
    {QZSettings::tile_moving_time_order, QZSettings::default_tile_moving_time_order},
    {QZSettings::tile_peloton_offset_enabled, QZSettings::default_tile_peloton_offset_enabled},
    {QZSettings::tile_peloton_offset_order, QZSettings::default_tile_peloton_offset_order},
    {QZSettings::tile_peloton_difficulty_enabled, QZSettings::default_tile_peloton_difficulty_enabled},
    {QZSettings::tile_peloton_difficulty_order, QZSettings::default_tile_peloton_difficulty_order},
    {QZSettings::tile_peloton_resistance_enabled, QZSettings::default_tile_peloton_resistance_enabled},
    {QZSettings::tile_peloton_resistance_order, QZSettings::default_tile_peloton_resistance_order},
    {QZSettings::tile_datetime_enabled, QZSettings::default_tile_datetime_enabled},
    {QZSettings::tile_datetime_order, QZSettings::default_tile_datetime_order},
    {QZSettings::tile_target_resistance_enabled, QZSettings::default_tile_target_resistance_enabled},
    {QZSettings::tile_target_resistance_order, QZSettings::default_tile_target_resistance_order},
    {QZSettings::tile_target_peloton_resistance_enabled, QZSettings::default_tile_target_peloton_resistance_enabled},
    {QZSettings::tile_target_peloton_resistance_order, QZSettings::default_tile_target_peloton_resistance_order},
    {QZSettings::tile_target_cadence_enabled, QZSettings::default_tile_target_cadence_enabled},
    {QZSettings::tile_target_cadence_order, QZSettings::default_tile_target_cadence_order},
    {QZSettings::tile_target_power_enabled, QZSettings::default_tile_target_power_enabled},
    {QZSettings::tile_target_power_order, QZSettings::default_tile_target_power_order},
    {QZSettings::tile_target_zone_enabled, QZSettings::default_tile_target_zone_enabled},
    {QZSettings::tile_target_zone_order, QZSettings::default_tile_target_zone_order},
    {QZSettings::tile_target_speed_enabled, QZSettings::default_tile_target_speed_enabled},
    {QZSettings::tile_target_speed_order, QZSettings::default_tile_target_speed_order},
    {QZSettings::tile_target_incline_enabled, QZSettings::default_tile_target_incline_enabled},
    {QZSettings::tile_target_incline_order, QZSettings::default_tile_target_incline_order},
    {QZSettings::tile_strokes_count_enabled, QZSettings::default_tile_strokes_count_enabled},
    {QZSettings::tile_strokes_count_order, QZSettings::default_tile_strokes_count_order},
    {QZSettings::tile_strokes_length_enabled, QZSettings::default_tile_strokes_length_enabled},
    {QZSettings::tile_strokes_length_order, QZSettings::default_tile_strokes_length_order},
    {QZSettings::tile_watt_kg_enabled, QZSettings::default_tile_watt_kg_enabled},
    {QZSettings::tile_watt_kg_order, QZSettings::default_tile_watt_kg_order},
    {QZSettings::tile_gears_enabled, QZSettings::default_tile_gears_enabled},
    {QZSettings::tile_gears_order, QZSettings::default_tile_gears_order},
    {QZSettings::tile_remainingtimetrainprogramrow_enabled,
     QZSettings::default_tile_remainingtimetrainprogramrow_enabled},
    {QZSettings::tile_remainingtimetrainprogramrow_order, QZSettings::default_tile_remainingtimetrainprogramrow_order},
    {QZSettings::tile_nextrowstrainprogram_enabled, QZSettings::default_tile_nextrowstrainprogram_enabled},
    {QZSettings::tile_nextrowstrainprogram_order, QZSettings::default_tile_nextrowstrainprogram_order},
    {QZSettings::tile_mets_enabled, QZSettings::default_tile_mets_enabled},
    {QZSettings::tile_mets_order, QZSettings::default_tile_mets_order},
    {QZSettings::tile_targetmets_enabled, QZSettings::default_tile_targetmets_enabled},
    {QZSettings::tile_targetmets_order, QZSettings::default_tile_targetmets_order},
    {QZSettings::tile_steering_angle_enabled, QZSettings::default_tile_steering_angle_enabled},
    {QZSettings::tile_steering_angle_order, QZSettings::default_tile_steering_angle_order},
    {QZSettings::tile_pid_hr_enabled, QZSettings::default_tile_pid_hr_enabled},
    {QZSettings::tile_pid_hr_order, QZSettings::default_tile_pid_hr_order},
    {QZSettings::heart_rate_zone1, QZSettings::default_heart_rate_zone1},
    {QZSettings::heart_rate_zone2, QZSettings::default_heart_rate_zone2},
    {QZSettings::heart_rate_zone3, QZSettings::default_heart_rate_zone3},
    {QZSettings::heart_rate_zone4, QZSettings::default_heart_rate_zone4},
    {QZSettings::heart_max_override_enable, QZSettings::default_heart_max_override_enable},
    {QZSettings::heart_max_override_value, QZSettings::default_heart_max_override_value},
    {QZSettings::peloton_gain, QZSettings::default_peloton_gain},
    {QZSettings::peloton_offset, QZSettings::default_peloton_offset},
    {QZSettings::treadmill_pid_heart_zone, QZSettings::default_treadmill_pid_heart_zone},
    {QZSettings::pacef_1mile, QZSettings::default_pacef_1mile},
    {QZSettings::pacef_5km, QZSettings::default_pacef_5km},
    {QZSettings::pacef_10km, QZSettings::default_pacef_10km},
    {QZSettings::pacef_halfmarathon, QZSettings::default_pacef_halfmarathon},
    {QZSettings::pacef_marathon, QZSettings::default_pacef_marathon},
    {QZSettings::pace_default, QZSettings::default_pace_default},
    {QZSettings::domyos_treadmill_buttons, QZSettings::default_domyos_treadmill_buttons},
    {QZSettings::domyos_treadmill_distance_display, QZSettings::default_domyos_treadmill_distance_display},
    {QZSettings::domyos_treadmill_display_invert, QZSettings::default_domyos_treadmill_display_invert},
    {QZSettings::domyos_bike_cadence_filter, QZSettings::default_domyos_bike_cadence_filter},
    {QZSettings::domyos_bike_display_calories, QZSettings::default_domyos_bike_display_calories},
    {QZSettings::domyos_elliptical_speed_ratio, QZSettings::default_domyos_elliptical_speed_ratio},
    {QZSettings::eslinker_cadenza, QZSettings::default_eslinker_cadenza},
    {QZSettings::eslinker_ypoo, QZSettings::default_eslinker_ypoo},
    {QZSettings::echelon_watttable, QZSettings::default_echelon_watttable},
    {QZSettings::proform_wheel_ratio, QZSettings::default_proform_wheel_ratio},
    {QZSettings::proform_tour_de_france_clc, QZSettings::default_proform_tour_de_france_clc},
    {QZSettings::proform_tdf_jonseed_watt, QZSettings::default_proform_tdf_jonseed_watt},
    {QZSettings::proform_studio, QZSettings::default_proform_studio},
    {QZSettings::proform_tdf_10, QZSettings::default_proform_tdf_10},
    {QZSettings::horizon_gr7_cadence_multiplier, QZSettings::default_horizon_gr7_cadence_multiplier},
    {QZSettings::fitshow_user_id, QZSettings::default_fitshow_user_id},
    {QZSettings::inspire_peloton_formula, QZSettings::default_inspire_peloton_formula},
    {QZSettings::inspire_peloton_formula2, QZSettings::default_inspire_peloton_formula2},
    {QZSettings::hammer_racer_s, QZSettings::default_hammer_racer_s},
    {QZSettings::pafers_treadmill, QZSettings::default_pafers_treadmill},
    {QZSettings::yesoul_peloton_formula, QZSettings::default_yesoul_peloton_formula},
    {QZSettings::nordictrack_10_treadmill, QZSettings::default_nordictrack_10_treadmill},
    {QZSettings::nordictrack_t65s_treadmill, QZSettings::default_nordictrack_t65s_treadmill},
    {QZSettings::nordictrack_treadmill_ultra_le, QZSettings::default_nordictrack_treadmill_ultra_le},
    {QZSettings::toorx_3_0, QZSettings::default_toorx_3_0},
    {QZSettings::toorx_65s_evo, QZSettings::default_toorx_65s_evo},
    {QZSettings::jtx_fitness_sprint_treadmill, QZSettings::default_jtx_fitness_sprint_treadmill},
    {QZSettings::dkn_endurun_treadmill, QZSettings::default_dkn_endurun_treadmill},
    {QZSettings::trx_route_key, QZSettings::default_trx_route_key},
    {QZSettings::bh_spada_2, QZSettings::default_bh_spada_2},
    {QZSettings::toorx_bike, QZSettings::default_toorx_bike},
    {QZSettings::toorx_ftms, QZSettings::default_toorx_ftms},
    {QZSettings::jll_IC400_bike, QZSettings::default_jll_IC400_bike},
    {QZSettings::fytter_ri08_bike, QZSettings::default_fytter_ri08_bike},
    {QZSettings::asviva_bike, QZSettings::default_asviva_bike},
    {QZSettings::hertz_xr_770, QZSettings::default_hertz_xr_770},
    {QZSettings::m3i_bike_id, QZSettings::default_m3i_bike_id},
    {QZSettings::m3i_bike_speed_buffsize, QZSettings::default_m3i_bike_speed_buffsize},
    {QZSettings::m3i_bike_qt_search, QZSettings::default_m3i_bike_qt_search},
    {QZSettings::m3i_bike_kcal, QZSettings::default_m3i_bike_kcal},
    {QZSettings::snode_bike, QZSettings::default_snode_bike},
    {QZSettings::fitplus_bike, QZSettings::default_fitplus_bike},
    {QZSettings::virtufit_etappe, QZSettings::default_virtufit_etappe},
    {QZSettings::flywheel_filter, QZSettings::default_flywheel_filter},
    {QZSettings::flywheel_life_fitness_ic8, QZSettings::default_flywheel_life_fitness_ic8},
    {QZSettings::sole_treadmill_inclination, QZSettings::default_sole_treadmill_inclination},
    {QZSettings::sole_treadmill_miles, QZSettings::default_sole_treadmill_miles},
    {QZSettings::sole_treadmill_f65, QZSettings::default_sole_treadmill_f65},
    {QZSettings::sole_treadmill_f63, QZSettings::default_sole_treadmill_f63},
    {QZSettings::sole_treadmill_tt8, QZSettings::default_sole_treadmill_tt8},
    {QZSettings::schwinn_bike_resistance, QZSettings::default_schwinn_bike_resistance},
    {QZSettings::schwinn_bike_resistance_v2, QZSettings::default_schwinn_bike_resistance_v2},
    {QZSettings::technogym_myrun_treadmill_experimental, QZSettings::default_technogym_myrun_treadmill_experimental},
    {QZSettings::trainprogram_random, QZSettings::default_trainprogram_random},
    {QZSettings::trainprogram_total, QZSettings::default_trainprogram_total},
    {QZSettings::trainprogram_period_seconds, QZSettings::default_trainprogram_period_seconds},
    {QZSettings::trainprogram_speed_min, QZSettings::default_trainprogram_speed_min},
    {QZSettings::trainprogram_speed_max, QZSettings::default_trainprogram_speed_max},
    {QZSettings::trainprogram_incline_min, QZSettings::default_trainprogram_incline_min},
    {QZSettings::trainprogram_incline_max, QZSettings::default_trainprogram_incline_max},
    {QZSettings::trainprogram_resistance_min, QZSettings::default_trainprogram_resistance_min},
    {QZSettings::trainprogram_resistance_max, QZSettings::default_trainprogram_resistance_max},
    {QZSettings::watt_offset, QZSettings::default_watt_offset},
    {QZSettings::watt_gain, QZSettings::default_watt_gain},
    {QZSettings::power_avg_5s, QZSettings::default_power_avg_5s},
    {QZSettings::instant_power_on_pause, QZSettings::default_instant_power_on_pause},
    {QZSettings::speed_offset, QZSettings::default_speed_offset},
    {QZSettings::speed_gain, QZSettings::default_speed_gain},
    {QZSettings::filter_device, QZSettings::default_filter_device},
    {QZSettings::strava_suffix, QZSettings::default_strava_suffix},
    {QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name},
    {QZSettings::cadence_sensor_as_bike, QZSettings::default_cadence_sensor_as_bike},
    {QZSettings::cadence_sensor_speed_ratio, QZSettings::default_cadence_sensor_speed_ratio},
    {QZSettings::power_hr_pwr1, QZSettings::default_power_hr_pwr1},
    {QZSettings::power_hr_hr1, QZSettings::default_power_hr_hr1},
    {QZSettings::power_hr_pwr2, QZSettings::default_power_hr_pwr2},
    {QZSettings::power_hr_hr2, QZSettings::default_power_hr_hr2},
    {QZSettings::power_sensor_name, QZSettings::default_power_sensor_name},
    {QZSettings::power_sensor_as_bike, QZSettings::default_power_sensor_as_bike},
    {QZSettings::power_sensor_as_treadmill, QZSettings::default_power_sensor_as_treadmill},
    {QZSettings::powr_sensor_running_cadence_double, QZSettings::default_powr_sensor_running_cadence_double},
    {QZSettings::elite_rizer_name, QZSettings::default_elite_rizer_name},
    {QZSettings::elite_sterzo_smart_name, QZSettings::default_elite_sterzo_smart_name},
    {QZSettings::ftms_accessory_name, QZSettings::default_ftms_accessory_name},
    {QZSettings::ss2k_shift_step, QZSettings::default_ss2k_shift_step},
    {QZSettings::fitmetria_fanfit_enable, QZSettings::default_fitmetria_fanfit_enable},
    {QZSettings::fitmetria_fanfit_mode, QZSettings::default_fitmetria_fanfit_mode},
    {QZSettings::fitmetria_fanfit_min, QZSettings::default_fitmetria_fanfit_min},
    {QZSettings::fitmetria_fanfit_max, QZSettings::default_fitmetria_fanfit_max},
    {QZSettings::virtualbike_forceresistance, QZSettings::default_virtualbike_forceresistance},
    {QZSettings::bluetooth_relaxed, QZSettings::default_bluetooth_relaxed},
    {QZSettings::bluetooth_30m_hangs, QZSettings::default_bluetooth_30m_hangs},
    {QZSettings::battery_service, QZSettings::default_battery_service},
    {QZSettings::service_changed, QZSettings::default_service_changed},
    {QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled},
    {QZSettings::virtual_device_bluetooth, QZSettings::default_virtual_device_bluetooth},
    {QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround},
    {QZSettings::android_wakelock, QZSettings::default_android_wakelock},
    {QZSettings::log_debug, QZSettings::default_log_debug},
    {QZSettings::virtual_device_onlyheart, QZSettings::default_virtual_device_onlyheart},
    {QZSettings::virtual_device_echelon, QZSettings::default_virtual_device_echelon},
    {QZSettings::virtual_device_ifit, QZSettings::default_virtual_device_ifit},
    {QZSettings::virtual_device_rower, QZSettings::default_virtual_device_rower},
    {QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike},
    {QZSettings::volume_change_gears, QZSettings::default_volume_change_gears},
    {QZSettings::applewatch_fakedevice, QZSettings::default_applewatch_fakedevice},
    {QZSettings::zwift_erg_resistance_down, QZSettings::default_zwift_erg_resistance_down},
    {QZSettings::zwift_erg_resistance_up, QZSettings::default_zwift_erg_resistance_up},
    {QZSettings::horizon_paragon_x, QZSettings::default_horizon_paragon_x},
    {QZSettings::treadmill_step_speed, QZSettings::default_treadmill_step_speed},
    {QZSettings::treadmill_step_incline, QZSettings::default_treadmill_step_incline},
    {QZSettings::fitshow_anyrun, QZSettings::default_fitshow_anyrun},
    {QZSettings::nordictrack_s30_treadmill, QZSettings::default_nordictrack_s30_treadmill},
    {QZSettings::renpho_peloton_conversion_v2, QZSettings::default_renpho_peloton_conversion_v2},
    {QZSettings::ss2k_resistance_sample_1, QZSettings::default_ss2k_resistance_sample_1},
    {QZSettings::ss2k_shift_step_sample_1, QZSettings::default_ss2k_shift_step_sample_1},
    {QZSettings::ss2k_resistance_sample_2, QZSettings::default_ss2k_resistance_sample_2},
    {QZSettings::ss2k_shift_step_sample_2, QZSettings::default_ss2k_shift_step_sample_2},
    {QZSettings::ss2k_resistance_sample_3, QZSettings::default_ss2k_resistance_sample_3},
    {QZSettings::ss2k_shift_step_sample_3, QZSettings::default_ss2k_shift_step_sample_3},
    {QZSettings::ss2k_resistance_sample_4, QZSettings::default_ss2k_resistance_sample_4},
    {QZSettings::ss2k_shift_step_sample_4, QZSettings::default_ss2k_shift_step_sample_4},
    {QZSettings::fitshow_truetimer, QZSettings::default_fitshow_truetimer},
    {QZSettings::elite_rizer_gain, QZSettings::default_elite_rizer_gain},
    {QZSettings::tile_ext_incline_enabled, QZSettings::default_tile_ext_incline_enabled},
    {QZSettings::tile_ext_incline_order, QZSettings::default_tile_ext_incline_order},
    {QZSettings::reebok_fr30_treadmill, QZSettings::default_reebok_fr30_treadmill},
    {QZSettings::horizon_treadmill_7_8, QZSettings::default_horizon_treadmill_7_8},
    {QZSettings::profile_name, QZSettings::default_profile_name},
    {QZSettings::tile_cadence_color_enabled, QZSettings::default_tile_cadence_color_enabled},
    {QZSettings::tile_peloton_remaining_enabled, QZSettings::default_tile_peloton_remaining_enabled},
    {QZSettings::tile_peloton_remaining_order, QZSettings::default_tile_peloton_remaining_order},
    {QZSettings::tile_peloton_resistance_color_enabled, QZSettings::default_tile_peloton_resistance_color_enabled},
    {QZSettings::dircon_yes, QZSettings::default_dircon_yes},
    {QZSettings::dircon_server_base_port, QZSettings::default_dircon_server_base_port},
    {QZSettings::ios_cache_heart_device, QZSettings::default_ios_cache_heart_device},
    {QZSettings::app_opening, QZSettings::default_app_opening},
    {QZSettings::proformtdf4ip, QZSettings::default_proformtdf4ip},
    {QZSettings::fitfiu_mc_v460, QZSettings::default_fitfiu_mc_v460},
    {QZSettings::bike_weight, QZSettings::default_bike_weight},
    {QZSettings::kingsmith_encrypt_v2, QZSettings::default_kingsmith_encrypt_v2},
    {QZSettings::proform_treadmill_9_0, QZSettings::default_proform_treadmill_9_0},
    {QZSettings::proform_treadmill_1800i, QZSettings::default_proform_treadmill_1800i},
    {QZSettings::cadence_offset, QZSettings::default_cadence_offset},
    {QZSettings::cadence_gain, QZSettings::default_cadence_gain},
    {QZSettings::sp_ht_9600ie, QZSettings::default_sp_ht_9600ie},
    {QZSettings::tts_enabled, QZSettings::default_tts_enabled},
    {QZSettings::tts_summary_sec, QZSettings::default_tts_summary_sec},
    {QZSettings::tts_act_speed, QZSettings::default_tts_act_speed},
    {QZSettings::tts_avg_speed, QZSettings::default_tts_avg_speed},
    {QZSettings::tts_max_speed, QZSettings::default_tts_max_speed},
    {QZSettings::tts_act_inclination, QZSettings::default_tts_act_inclination},
    {QZSettings::tts_act_cadence, QZSettings::default_tts_act_cadence},
    {QZSettings::tts_avg_cadence, QZSettings::default_tts_avg_cadence},
    {QZSettings::tts_max_cadence, QZSettings::default_tts_max_cadence},
    {QZSettings::tts_act_elevation, QZSettings::default_tts_act_elevation},
    {QZSettings::tts_act_calories, QZSettings::default_tts_act_calories},
    {QZSettings::tts_act_odometer, QZSettings::default_tts_act_odometer},
    {QZSettings::tts_act_pace, QZSettings::default_tts_act_pace},
    {QZSettings::tts_avg_pace, QZSettings::default_tts_avg_pace},
    {QZSettings::tts_max_pace, QZSettings::default_tts_max_pace},
    {QZSettings::tts_act_resistance, QZSettings::default_tts_act_resistance},
    {QZSettings::tts_avg_resistance, QZSettings::default_tts_avg_resistance},
    {QZSettings::tts_max_resistance, QZSettings::default_tts_max_resistance},
    {QZSettings::tts_act_watt, QZSettings::default_tts_act_watt},
    {QZSettings::tts_avg_watt, QZSettings::default_tts_avg_watt},
    {QZSettings::tts_max_watt, QZSettings::default_tts_max_watt},
    {QZSettings::tts_act_ftp, QZSettings::default_tts_act_ftp},
    {QZSettings::tts_avg_ftp, QZSettings::default_tts_avg_ftp},
    {QZSettings::tts_max_ftp, QZSettings::default_tts_max_ftp},
    {QZSettings::tts_act_heart, QZSettings::default_tts_act_heart},
    {QZSettings::tts_avg_heart, QZSettings::default_tts_avg_heart},
    {QZSettings::tts_max_heart, QZSettings::default_tts_max_heart},
    {QZSettings::tts_act_jouls, QZSettings::default_tts_act_jouls},
    {QZSettings::tts_act_elapsed, QZSettings::default_tts_act_elapsed},
    {QZSettings::tts_act_peloton_resistance, QZSettings::default_tts_act_peloton_resistance},
    {QZSettings::tts_avg_peloton_resistance, QZSettings::default_tts_avg_peloton_resistance},
    {QZSettings::tts_max_peloton_resistance, QZSettings::default_tts_max_peloton_resistance},
    {QZSettings::tts_act_target_peloton_resistance, QZSettings::default_tts_act_target_peloton_resistance},
    {QZSettings::tts_act_target_cadence, QZSettings::default_tts_act_target_cadence},
    {QZSettings::tts_act_target_power, QZSettings::default_tts_act_target_power},
    {QZSettings::tts_act_target_zone, QZSettings::default_tts_act_target_zone},
    {QZSettings::tts_act_target_speed, QZSettings::default_tts_act_target_speed},
    {QZSettings::tts_act_target_incline, QZSettings::default_tts_act_target_incline},
    {QZSettings::tts_act_watt_kg, QZSettings::default_tts_act_watt_kg},
    {QZSettings::tts_avg_watt_kg, QZSettings::default_tts_avg_watt_kg},
    {QZSettings::tts_max_watt_kg, QZSettings::default_tts_max_watt_kg},
    {QZSettings::fakedevice_elliptical, QZSettings::default_fakedevice_elliptical},
    {QZSettings::nordictrack_2950_ip, QZSettings::default_nordictrack_2950_ip},
    {QZSettings::tile_instantaneous_stride_length_enabled,
     QZSettings::default_tile_instantaneous_stride_length_enabled},
    {QZSettings::tile_instantaneous_stride_length_order, QZSettings::default_tile_instantaneous_stride_length_order},
    {QZSettings::tile_ground_contact_enabled, QZSettings::default_tile_ground_contact_enabled},
    {QZSettings::tile_ground_contact_order, QZSettings::default_tile_ground_contact_order},
    {QZSettings::tile_vertical_oscillation_enabled, QZSettings::default_tile_vertical_oscillation_enabled},
    {QZSettings::tile_vertical_oscillation_order, QZSettings::default_tile_vertical_oscillation_order},
    {QZSettings::sex, QZSettings::default_sex},
    {QZSettings::maps_type, QZSettings::default_maps_type},
    {QZSettings::ss2k_max_resistance, QZSettings::default_ss2k_max_resistance},
    {QZSettings::ss2k_min_resistance, QZSettings::default_ss2k_min_resistance},
    {QZSettings::proform_treadmill_se, QZSettings::default_proform_treadmill_se},
    {QZSettings::proformtreadmillip, QZSettings::default_proformtreadmillip},
    {QZSettings::kingsmith_encrypt_v3, QZSettings::default_kingsmith_encrypt_v3},
    {QZSettings::tdf_10_ip, QZSettings::default_tdf_10_ip},
    {QZSettings::fakedevice_treadmill, QZSettings::default_fakedevice_treadmill},
    {QZSettings::video_playback_window_s, QZSettings::default_video_playback_window_s},
    {QZSettings::horizon_treadmill_profile_user1, QZSettings::default_horizon_treadmill_profile_user1},
    {QZSettings::horizon_treadmill_profile_user2, QZSettings::default_horizon_treadmill_profile_user2},
    {QZSettings::horizon_treadmill_profile_user3, QZSettings::default_horizon_treadmill_profile_user3},
    {QZSettings::horizon_treadmill_profile_user4, QZSettings::default_horizon_treadmill_profile_user4},
    {QZSettings::horizon_treadmill_profile_user5, QZSettings::default_horizon_treadmill_profile_user5},
    {QZSettings::nordictrack_gx_2_7, QZSettings::default_nordictrack_gx_2_7},
    {QZSettings::rolling_resistance, QZSettings::default_rolling_resistance},
    {QZSettings::wahoo_rgt_dircon, QZSettings::default_wahoo_rgt_dircon},
    {QZSettings::tts_description_enabled, QZSettings::default_tts_description_enabled},
    {QZSettings::tile_preset_resistance_1_enabled, QZSettings::default_tile_preset_resistance_1_enabled},
    {QZSettings::tile_preset_resistance_1_order, QZSettings::default_tile_preset_resistance_1_order},
    {QZSettings::tile_preset_resistance_1_value, QZSettings::default_tile_preset_resistance_1_value},
    {QZSettings::tile_preset_resistance_1_label, QZSettings::default_tile_preset_resistance_1_label},
    {QZSettings::tile_preset_resistance_2_enabled, QZSettings::default_tile_preset_resistance_2_enabled},
    {QZSettings::tile_preset_resistance_2_order, QZSettings::default_tile_preset_resistance_2_order},
    {QZSettings::tile_preset_resistance_2_value, QZSettings::default_tile_preset_resistance_2_value},
    {QZSettings::tile_preset_resistance_2_label, QZSettings::default_tile_preset_resistance_2_label},
    {QZSettings::tile_preset_resistance_3_enabled, QZSettings::default_tile_preset_resistance_3_enabled},
    {QZSettings::tile_preset_resistance_3_order, QZSettings::default_tile_preset_resistance_3_order},
    {QZSettings::tile_preset_resistance_3_value, QZSettings::default_tile_preset_resistance_3_value},
    {QZSettings::tile_preset_resistance_3_label, QZSettings::default_tile_preset_resistance_3_label},
    {QZSettings::tile_preset_resistance_4_enabled, QZSettings::default_tile_preset_resistance_4_enabled},
    {QZSettings::tile_preset_resistance_4_order, QZSettings::default_tile_preset_resistance_4_order},
    {QZSettings::tile_preset_resistance_4_value, QZSettings::default_tile_preset_resistance_4_value},
    {QZSettings::tile_preset_resistance_4_label, QZSettings::default_tile_preset_resistance_4_label},
    {QZSettings::tile_preset_resistance_5_enabled, QZSettings::default_tile_preset_resistance_5_enabled},
    {QZSettings::tile_preset_resistance_5_order, QZSettings::default_tile_preset_resistance_5_order},
    {QZSettings::tile_preset_resistance_5_value, QZSettings::default_tile_preset_resistance_5_value},
    {QZSettings::tile_preset_resistance_5_label, QZSettings::default_tile_preset_resistance_5_label},
    {QZSettings::tile_preset_speed_1_enabled, QZSettings::default_tile_preset_speed_1_enabled},
    {QZSettings::tile_preset_speed_1_order, QZSettings::default_tile_preset_speed_1_order},
    {QZSettings::tile_preset_speed_1_value, QZSettings::default_tile_preset_speed_1_value},
    {QZSettings::tile_preset_speed_1_label, QZSettings::default_tile_preset_speed_1_label},
    {QZSettings::tile_preset_speed_2_enabled, QZSettings::default_tile_preset_speed_2_enabled},
    {QZSettings::tile_preset_speed_2_order, QZSettings::default_tile_preset_speed_2_order},
    {QZSettings::tile_preset_speed_2_value, QZSettings::default_tile_preset_speed_2_value},
    {QZSettings::tile_preset_speed_2_label, QZSettings::default_tile_preset_speed_2_label},
    {QZSettings::tile_preset_speed_3_enabled, QZSettings::default_tile_preset_speed_3_enabled},
    {QZSettings::tile_preset_speed_3_order, QZSettings::default_tile_preset_speed_3_order},
    {QZSettings::tile_preset_speed_3_value, QZSettings::default_tile_preset_speed_3_value},
    {QZSettings::tile_preset_speed_3_label, QZSettings::default_tile_preset_speed_3_label},
    {QZSettings::tile_preset_speed_4_enabled, QZSettings::default_tile_preset_speed_4_enabled},
    {QZSettings::tile_preset_speed_4_order, QZSettings::default_tile_preset_speed_4_order},
    {QZSettings::tile_preset_speed_4_value, QZSettings::default_tile_preset_speed_4_value},
    {QZSettings::tile_preset_speed_4_label, QZSettings::default_tile_preset_speed_4_label},
    {QZSettings::tile_preset_speed_5_enabled, QZSettings::default_tile_preset_speed_5_enabled},
    {QZSettings::tile_preset_speed_5_order, QZSettings::default_tile_preset_speed_5_order},
    {QZSettings::tile_preset_speed_5_value, QZSettings::default_tile_preset_speed_5_value},
    {QZSettings::tile_preset_speed_5_label, QZSettings::default_tile_preset_speed_5_label},
    {QZSettings::tile_preset_inclination_1_enabled, QZSettings::default_tile_preset_inclination_1_enabled},
    {QZSettings::tile_preset_inclination_1_order, QZSettings::default_tile_preset_inclination_1_order},
    {QZSettings::tile_preset_inclination_1_value, QZSettings::default_tile_preset_inclination_1_value},
    {QZSettings::tile_preset_inclination_1_label, QZSettings::default_tile_preset_inclination_1_label},
    {QZSettings::tile_preset_inclination_2_enabled, QZSettings::default_tile_preset_inclination_2_enabled},
    {QZSettings::tile_preset_inclination_2_order, QZSettings::default_tile_preset_inclination_2_order},
    {QZSettings::tile_preset_inclination_2_value, QZSettings::default_tile_preset_inclination_2_value},
    {QZSettings::tile_preset_inclination_2_label, QZSettings::default_tile_preset_inclination_2_label},
    {QZSettings::tile_preset_inclination_3_enabled, QZSettings::default_tile_preset_inclination_3_enabled},
    {QZSettings::tile_preset_inclination_3_order, QZSettings::default_tile_preset_inclination_3_order},
    {QZSettings::tile_preset_inclination_3_value, QZSettings::default_tile_preset_inclination_3_value},
    {QZSettings::tile_preset_inclination_3_label, QZSettings::default_tile_preset_inclination_3_label},
    {QZSettings::tile_preset_inclination_4_enabled, QZSettings::default_tile_preset_inclination_4_enabled},
    {QZSettings::tile_preset_inclination_4_order, QZSettings::default_tile_preset_inclination_4_order},
    {QZSettings::tile_preset_inclination_4_value, QZSettings::default_tile_preset_inclination_4_value},
    {QZSettings::tile_preset_inclination_4_label, QZSettings::default_tile_preset_inclination_4_label},
    {QZSettings::tile_preset_inclination_5_enabled, QZSettings::default_tile_preset_inclination_5_enabled},
    {QZSettings::tile_preset_inclination_5_order, QZSettings::default_tile_preset_inclination_5_order},
    {QZSettings::tile_preset_inclination_5_value, QZSettings::default_tile_preset_inclination_5_value},
    {QZSettings::tile_preset_inclination_5_label, QZSettings::default_tile_preset_inclination_5_label},
    {QZSettings::tile_preset_resistance_1_color, QZSettings::default_tile_preset_resistance_1_color},
    {QZSettings::tile_preset_resistance_2_color, QZSettings::default_tile_preset_resistance_2_color},
    {QZSettings::tile_preset_resistance_3_color, QZSettings::default_tile_preset_resistance_3_color},
    {QZSettings::tile_preset_resistance_4_color, QZSettings::default_tile_preset_resistance_4_color},
    {QZSettings::tile_preset_resistance_5_color, QZSettings::default_tile_preset_resistance_5_color},
    {QZSettings::tile_preset_speed_1_color, QZSettings::default_tile_preset_speed_1_color},
    {QZSettings::tile_preset_speed_2_color, QZSettings::default_tile_preset_speed_2_color},
    {QZSettings::tile_preset_speed_3_color, QZSettings::default_tile_preset_speed_3_color},
    {QZSettings::tile_preset_speed_4_color, QZSettings::default_tile_preset_speed_4_color},
    {QZSettings::tile_preset_speed_5_color, QZSettings::default_tile_preset_speed_5_color},
    {QZSettings::tile_preset_inclination_1_color, QZSettings::default_tile_preset_inclination_1_color},
    {QZSettings::tile_preset_inclination_2_color, QZSettings::default_tile_preset_inclination_2_color},
    {QZSettings::tile_preset_inclination_3_color, QZSettings::default_tile_preset_inclination_3_color},
    {QZSettings::tile_preset_inclination_4_color, QZSettings::default_tile_preset_inclination_4_color},
    {QZSettings::tile_preset_inclination_5_color, QZSettings::default_tile_preset_inclination_5_color},
    {QZSettings::tile_avg_watt_lap_enabled, QZSettings::default_tile_avg_watt_lap_enabled},
    {QZSettings::tile_avg_watt_lap_order, QZSettings::default_tile_avg_watt_lap_order},
    {QZSettings::nordictrack_t70_treadmill, QZSettings::default_nordictrack_t70_treadmill},
    {QZSettings::CRRGain, QZSettings::default_CRRGain},
    {QZSettings::CWGain, QZSettings::default_CWGain},
    {QZSettings::proform_treadmill_cadence_lt, QZSettings::default_proform_treadmill_cadence_lt},
    {QZSettings::trainprogram_stop_at_end, QZSettings::default_trainprogram_stop_at_end},
    {QZSettings::domyos_elliptical_inclination, QZSettings::default_domyos_elliptical_inclination},
    {QZSettings::gpx_loop, QZSettings::default_gpx_loop},
    {QZSettings::android_notification, QZSettings::default_android_notification},
    {QZSettings::kingsmith_encrypt_v4, QZSettings::default_kingsmith_encrypt_v4},
    {QZSettings::horizon_treadmill_disable_pause, QZSettings::default_horizon_treadmill_disable_pause},
    {QZSettings::domyos_bike_500_profile_v1, QZSettings::domyos_bike_500_profile_v1},
    {QZSettings::ss2k_peloton, QZSettings::default_ss2k_peloton},
    {QZSettings::computrainer_serialport, QZSettings::default_computrainer_serialport},
    {QZSettings::strava_virtual_activity, QZSettings::default_strava_virtual_activity},
    {QZSettings::powr_sensor_running_cadence_half_on_strava,
     QZSettings::default_powr_sensor_running_cadence_half_on_strava},
    {QZSettings::nordictrack_ifit_adb_remote, QZSettings::default_nordictrack_ifit_adb_remote},
    {QZSettings::floating_height, QZSettings::default_floating_height},
    {QZSettings::floating_width, QZSettings::default_floating_width},
    {QZSettings::floating_transparency, QZSettings::default_floating_transparency},
    {QZSettings::floating_startup, QZSettings::default_floating_startup},
    {QZSettings::norditrack_s25i_treadmill, QZSettings::default_norditrack_s25i_treadmill},
    {QZSettings::toorx_ftms_treadmill, QZSettings::default_toorx_ftms_treadmill},
    {QZSettings::nordictrack_t65s_83_treadmill, QZSettings::default_nordictrack_t65s_83_treadmill},
    {QZSettings::horizon_treadmill_suspend_stats_pause, QZSettings::default_horizon_treadmill_suspend_stats_pause},
    {QZSettings::sportstech_sx600, QZSettings::default_sportstech_sx600},
    {QZSettings::sole_elliptical_inclination, QZSettings::default_sole_elliptical_inclination},
    {QZSettings::proform_hybrid_trainer_xt, QZSettings::default_proform_hybrid_trainer_xt},
    {QZSettings::gears_restore_value, QZSettings::default_gears_restore_value},
    {QZSettings::gears_current_value, QZSettings::default_gears_current_value},
    {QZSettings::tile_pace_last500m_enabled, QZSettings::default_tile_pace_last500m_enabled},
    {QZSettings::tile_pace_last500m_order, QZSettings::default_tile_pace_last500m_order},
    {QZSettings::treadmill_difficulty_gain_or_offset, QZSettings::default_treadmill_difficulty_gain_or_offset},
    {QZSettings::pafers_treadmill_bh_iboxster_plus, QZSettings::default_pafers_treadmill_bh_iboxster_plus},
    {QZSettings::proform_cycle_trainer_400, QZSettings::default_proform_cycle_trainer_400},
    {QZSettings::peloton_workout_ocr, QZSettings::default_peloton_workout_ocr},
    {QZSettings::peloton_bike_ocr, QZSettings::default_peloton_bike_ocr},
    {QZSettings::fitshow_treadmill_miles, QZSettings::default_fitshow_treadmill_miles},
    {QZSettings::proform_hybrid_trainer_PFEL03815, QZSettings::default_proform_hybrid_trainer_PFEL03815},
    {QZSettings::schwinn_resistance_smooth, QZSettings::default_schwinn_resistance_smooth},
    {QZSettings::treadmill_inclination_override_0, QZSettings::default_treadmill_inclination_override_0},
    {QZSettings::treadmill_inclination_override_05, QZSettings::default_treadmill_inclination_override_05},
    {QZSettings::treadmill_inclination_override_10, QZSettings::default_treadmill_inclination_override_10},
    {QZSettings::treadmill_inclination_override_15, QZSettings::default_treadmill_inclination_override_15},
    {QZSettings::treadmill_inclination_override_20, QZSettings::default_treadmill_inclination_override_20},
    {QZSettings::treadmill_inclination_override_25, QZSettings::default_treadmill_inclination_override_25},
    {QZSettings::treadmill_inclination_override_30, QZSettings::default_treadmill_inclination_override_30},
    {QZSettings::treadmill_inclination_override_35, QZSettings::default_treadmill_inclination_override_35},
    {QZSettings::treadmill_inclination_override_40, QZSettings::default_treadmill_inclination_override_40},
    {QZSettings::treadmill_inclination_override_45, QZSettings::default_treadmill_inclination_override_45},
    {QZSettings::treadmill_inclination_override_50, QZSettings::default_treadmill_inclination_override_50},
    {QZSettings::treadmill_inclination_override_55, QZSettings::default_treadmill_inclination_override_55},
    {QZSettings::treadmill_inclination_override_60, QZSettings::default_treadmill_inclination_override_60},
    {QZSettings::treadmill_inclination_override_65, QZSettings::default_treadmill_inclination_override_65},
    {QZSettings::treadmill_inclination_override_70, QZSettings::default_treadmill_inclination_override_70},
    {QZSettings::treadmill_inclination_override_75, QZSettings::default_treadmill_inclination_override_75},
    {QZSettings::treadmill_inclination_override_80, QZSettings::default_treadmill_inclination_override_80},
    {QZSettings::treadmill_inclination_override_85, QZSettings::default_treadmill_inclination_override_85},
    {QZSettings::treadmill_inclination_override_90, QZSettings::default_treadmill_inclination_override_90},
    {QZSettings::treadmill_inclination_override_95, QZSettings::default_treadmill_inclination_override_95},
    {QZSettings::treadmill_inclination_override_100, QZSettings::default_treadmill_inclination_override_100},
    {QZSettings::treadmill_inclination_override_105, QZSettings::default_treadmill_inclination_override_105},
    {QZSettings::treadmill_inclination_override_110, QZSettings::default_treadmill_inclination_override_110},
    {QZSettings::treadmill_inclination_override_115, QZSettings::default_treadmill_inclination_override_115},
    {QZSettings::treadmill_inclination_override_120, QZSettings::default_treadmill_inclination_override_120},
    {QZSettings::treadmill_inclination_override_125, QZSettings::default_treadmill_inclination_override_125},
    {QZSettings::treadmill_inclination_override_130, QZSettings::default_treadmill_inclination_override_130},
    {QZSettings::treadmill_inclination_override_135, QZSettings::default_treadmill_inclination_override_135},
    {QZSettings::treadmill_inclination_override_140, QZSettings::default_treadmill_inclination_override_140},
    {QZSettings::treadmill_inclination_override_145, QZSettings::default_treadmill_inclination_override_145},
    {QZSettings::treadmill_inclination_override_150, QZSettings::default_treadmill_inclination_override_150},
    {QZSettings::sole_elliptical_e55, QZSettings::default_sole_elliptical_e55},
    {QZSettings::horizon_treadmill_force_ftms, QZSettings::default_horizon_treadmill_force_ftms},
    {QZSettings::horizon_treadmill_7_0_at_24, QZSettings::default_horizon_treadmill_7_0_at_24},
    {QZSettings::treadmill_pid_heart_min, QZSettings::default_treadmill_pid_heart_min},
    {QZSettings::treadmill_pid_heart_max, QZSettings::default_treadmill_pid_heart_max},
    {QZSettings::nordictrack_elliptical_c7_5, QZSettings::default_nordictrack_elliptical_c7_5},
    {QZSettings::renpho_bike_double_resistance, QZSettings::default_renpho_bike_double_resistance},
    {QZSettings::nordictrack_incline_trainer_x7i, QZSettings::default_nordictrack_incline_trainer_x7i},
    {QZSettings::strava_auth_external_webbrowser, QZSettings::default_strava_auth_external_webbrowser},
    {QZSettings::gears_from_bike, QZSettings::default_gears_from_bike},
    {QZSettings::peloton_spinups_autoresistance, QZSettings::default_peloton_spinups_autoresistance},
    {QZSettings::eslinker_costaway, QZSettings::default_eslinker_costaway},
    {QZSettings::treadmill_inclination_ovveride_gain, QZSettings::default_treadmill_inclination_ovveride_gain},
    {QZSettings::treadmill_inclination_ovveride_offset, QZSettings::default_treadmill_inclination_ovveride_offset},
    {QZSettings::bh_spada_2_watt, QZSettings::default_bh_spada_2_watt},
    {QZSettings::tacx_neo2_peloton, QZSettings::default_tacx_neo2_peloton},
    {QZSettings::sole_treadmill_inclination_fast, QZSettings::default_sole_treadmill_inclination_fast},
    {QZSettings::zwift_ocr, QZSettings::default_zwift_ocr},
    {QZSettings::fit_file_saved_on_quit, QZSettings::default_fit_file_saved_on_quit},
    {QZSettings::gem_module_inclination, QZSettings::default_gem_module_inclination},
    {QZSettings::treadmill_simulate_inclination_with_speed,
     QZSettings::default_treadmill_simulate_inclination_with_speed},
    {QZSettings::garmin_companion, QZSettings::default_garmin_companion},
    {QZSettings::peloton_companion_workout_ocr, QZSettings::default_companion_peloton_workout_ocr},
    {QZSettings::iconcept_elliptical, QZSettings::default_iconcept_elliptical},
    {QZSettings::gears_gain, QZSettings::default_gears_gain},
    {QZSettings::proform_treadmill_8_0, QZSettings::default_proform_treadmill_8_0},
    {QZSettings::zero_zt2500_treadmill, QZSettings::default_zero_zt2500_treadmill},
    {QZSettings::kingsmith_encrypt_v5, QZSettings::default_kingsmith_encrypt_v5},
    {QZSettings::peloton_rower_level, QZSettings::default_peloton_rower_level},
    {QZSettings::tile_target_pace_enabled, QZSettings::default_tile_target_pace_enabled},
    {QZSettings::tile_target_pace_order, QZSettings::default_tile_target_pace_order},
    {QZSettings::tts_act_target_pace, QZSettings::default_tts_act_target_pace},
    {QZSettings::csafe_rower, QZSettings::default_csafe_rower},
    {QZSettings::csafe_elliptical_port, QZSettings::default_csafe_elliptical_port},
    {QZSettings::ftms_rower, QZSettings::default_ftms_rower},
    {QZSettings::zwift_workout_ocr, QZSettings::default_zwift_workout_ocr},
    {QZSettings::proform_bike_sb, QZSettings::default_proform_bike_sb},
    {QZSettings::fakedevice_rower, QZSettings::default_fakedevice_rower},
    {QZSettings::zwift_ocr_climb_portal, QZSettings::default_zwift_ocr_climb_portal},
    {QZSettings::poll_device_time, QZSettings::default_poll_device_time},
    {QZSettings::proform_bike_PFEVEX71316_1, QZSettings::default_proform_bike_PFEVEX71316_1},
    {QZSettings::schwinn_bike_resistance_v3, QZSettings::default_schwinn_bike_resistance_v3},
    {QZSettings::watt_ignore_builtin, QZSettings::default_watt_ignore_builtin},
    {QZSettings::proform_treadmill_z1300i, QZSettings::default_proform_treadmill_z1300i},
    {QZSettings::ftms_bike, QZSettings::default_ftms_bike},
    {QZSettings::ftms_treadmill, QZSettings::default_ftms_treadmill},
    {QZSettings::ant_speed_offset, QZSettings::default_ant_speed_offset},
    {QZSettings::ant_speed_gain, QZSettings::default_ant_speed_gain},
    {QZSettings::proform_rower_sport_rl, QZSettings::default_proform_rower_sport_rl},
    {QZSettings::strava_date_prefix, QZSettings::default_strava_date_prefix},
    {QZSettings::race_mode, QZSettings::default_race_mode},
    {QZSettings::proform_pro_1000_treadmill, QZSettings::default_proform_pro_1000_treadmill},
    {QZSettings::saris_trainer, QZSettings::default_saris_trainer},
    {QZSettings::proform_studio_NTEX71021, QZSettings::default_proform_studio_NTEX71021},
    {QZSettings::nordictrack_x22i, QZSettings::default_nordictrack_x22i},
    {QZSettings::iconsole_elliptical, QZSettings::default_iconsole_elliptical},    
    {QZSettings::autolap_distance, QZSettings::default_autolap_distance},
    {QZSettings::nordictrack_s20_treadmill, QZSettings::default_nordictrack_s20_treadmill},
    {QZSettings::freemotion_coachbike_b22_7, QZSettings::default_freemotion_coachbike_b22_7},
    {QZSettings::proform_cycle_trainer_300_ci, QZSettings::default_proform_cycle_trainer_300_ci},
    {QZSettings::kingsmith_encrypt_g1_walking_pad, QZSettings::default_kingsmith_encrypt_g1_walking_pad},    
    {QZSettings::proform_bike_225_csx, QZSettings::default_proform_bike_225_csx},
    {QZSettings::proform_treadmill_l6_0s, QZSettings::default_proform_treadmill_l6_0s},
    {QZSettings::proformtdf1ip, QZSettings::default_proformtdf1ip},
    {QZSettings::zwift_username, QZSettings::default_zwift_username},
    {QZSettings::zwift_password, QZSettings::default_zwift_password},
    {QZSettings::garmin_bluetooth_compatibility, QZSettings::default_garmin_bluetooth_compatibility},
    {QZSettings::norditrack_s25_treadmill, QZSettings::default_norditrack_s25_treadmill},
    {QZSettings::proform_8_5_treadmill, QZSettings::default_proform_8_5_treadmill},
    {QZSettings::treadmill_incline_min, QZSettings::default_treadmill_incline_min},
    {QZSettings::treadmill_incline_max, QZSettings::default_treadmill_incline_max},
    {QZSettings::proform_2000_treadmill, QZSettings::default_proform_2000_treadmill},
    {QZSettings::android_documents_folder, QZSettings::default_android_documents_folder},
    {QZSettings::zwift_api_autoinclination, QZSettings::default_zwift_api_autoinclination},
    {QZSettings::domyos_treadmill_button_5kmh, QZSettings::default_domyos_treadmill_button_5kmh},
    {QZSettings::domyos_treadmill_button_10kmh, QZSettings::default_domyos_treadmill_button_10kmh},
    {QZSettings::domyos_treadmill_button_16kmh, QZSettings::default_domyos_treadmill_button_16kmh},
    {QZSettings::domyos_treadmill_button_22kmh, QZSettings::default_domyos_treadmill_button_22kmh},
    {QZSettings::proform_treadmill_sport_8_5, QZSettings::default_proform_treadmill_sport_8_5},
    {QZSettings::domyos_treadmill_t900a, QZSettings::default_domyos_treadmill_t900a},
    {QZSettings::enerfit_SPX_9500, QZSettings::default_enerfit_SPX_9500},
    {QZSettings::proform_treadmill_505_cst, QZSettings::default_proform_treadmill_505_cst},
    {QZSettings::nordictrack_treadmill_t8_5s, QZSettings::default_nordictrack_treadmill_t8_5s},    
    {QZSettings::proform_treadmill_705_cst, QZSettings::default_proform_treadmill_705_cst},
    {QZSettings::zwift_click, QZSettings::default_zwift_click},
    {QZSettings::hop_sport_hs_090h_bike, QZSettings::default_hop_sport_hs_090h_bike},
    {QZSettings::zwift_play, QZSettings::default_zwift_play},
    {QZSettings::nordictrack_treadmill_x14i, QZSettings::default_nordictrack_treadmill_x14i},
    {QZSettings::zwift_api_poll, QZSettings::default_zwift_api_poll},
    {QZSettings::tile_step_count_enabled, QZSettings::default_tile_step_count_enabled},
    {QZSettings::tile_step_count_order, QZSettings::default_tile_step_count_order},
    {QZSettings::tile_erg_mode_enabled, QZSettings::default_tile_erg_mode_enabled},
    {QZSettings::tile_erg_mode_order, QZSettings::default_tile_erg_mode_order},
    {QZSettings::toorx_srx_3500, QZSettings::default_toorx_srx_3500},
    {QZSettings::stryd_speed_instead_treadmill, QZSettings::default_stryd_speed_instead_treadmill},
    {QZSettings::inclination_delay_seconds, QZSettings::default_inclination_delay_seconds},
    {QZSettings::ergDataPoints, QZSettings::default_ergDataPoints},
    {QZSettings::proform_carbon_tl, QZSettings::default_proform_carbon_tl},
    {QZSettings::proform_proshox2, QZSettings::default_proform_proshox2},
    {QZSettings::proform_tdf_10_0, QZSettings::default_proform_tdf_10_0},
    {QZSettings::nordictrack_GX4_5_bike, QZSettings::default_nordictrack_GX4_5_bike},
    {QZSettings::ftp_run, QZSettings::default_ftp_run},
    {QZSettings::tile_rss_enabled, QZSettings::default_tile_rss_enabled},
    {QZSettings::tile_rss_order, QZSettings::default_tile_rss_order},
    {QZSettings::treadmillDataPoints, QZSettings::default_treadmillDataPoints},
    {QZSettings::nordictrack_s20i_treadmill, QZSettings::default_nordictrack_s20i_treadmill},
    {QZSettings::proform_595i_proshox2, QZSettings::default_proform_595i_proshox2},
    {QZSettings::proform_treadmill_8_7, QZSettings::default_proform_treadmill_8_7},
    {QZSettings::proform_bike_325_csx, QZSettings::default_proform_bike_325_csx},
    {QZSettings::strava_upload_mode, QZSettings::default_strava_upload_mode},
    {QZSettings::proform_treadmill_705_cst_V78_239, QZSettings::default_proform_treadmill_705_cst_V78_239},
    {QZSettings::stryd_add_inclination_gain, QZSettings::default_stryd_add_inclination_gain},
    {QZSettings::toorx_bike_srx_500, QZSettings::default_toorx_bike_srx_500},
    {QZSettings::atletica_lightspeed_treadmill, QZSettings::default_atletica_lightspeed_treadmill},
    {QZSettings::peloton_treadmill_level, QZSettings::default_peloton_treadmill_level},
    {QZSettings::nordictrackadbbike_resistance, QZSettings::default_nordictrackadbbike_resistance},
    {QZSettings::proform_treadmill_carbon_t7, QZSettings::default_proform_treadmill_carbon_t7},
    {QZSettings::nordictrack_treadmill_exp_5i, QZSettings::default_nordictrack_treadmill_exp_5i},
    {QZSettings::dircon_id, QZSettings::default_dircon_id},
    {QZSettings::proform_elliptical_ip, QZSettings::default_proform_elliptical_ip},
    {QZSettings::antbike, QZSettings::default_antbike},
    {QZSettings::domyosbike_notfmts, QZSettings::default_domyosbike_notfmts},
    {QZSettings::gears_volume_debouncing, QZSettings::default_gears_volume_debouncing},
    {QZSettings::tile_biggears_enabled, QZSettings::default_tile_biggears_enabled},
    {QZSettings::tile_biggears_order, QZSettings::default_tile_biggears_order},
    {QZSettings::domyostreadmill_notfmts, QZSettings::default_domyostreadmill_notfmts},
    {QZSettings::zwiftplay_swap, QZSettings::default_zwiftplay_swap},
    {QZSettings::gears_zwift_ratio, QZSettings::default_gears_zwift_ratio},
    {QZSettings::domyos_bike_500_profile_v2, QZSettings::default_domyos_bike_500_profile_v2},
    {QZSettings::gears_offset, QZSettings::default_gears_offset},
    {QZSettings::peloton_accesstoken, QZSettings::default_peloton_accesstoken},
    {QZSettings::peloton_refreshtoken, QZSettings::default_peloton_refreshtoken},
    {QZSettings::peloton_lastrefresh, QZSettings::default_peloton_lastrefresh},
    {QZSettings::peloton_expires, QZSettings::default_peloton_expires},
    {QZSettings::peloton_code, QZSettings::default_peloton_code},
    {QZSettings::proform_carbon_tl_PFTL59720, QZSettings::default_proform_carbon_tl_PFTL59720},
    {QZSettings::proform_treadmill_sport_70, QZSettings::default_proform_treadmill_sport_70},
    {QZSettings::peloton_date_format, QZSettings::default_peloton_date_format},
    {QZSettings::force_resistance_instead_inclination, QZSettings::default_force_resistance_instead_inclination},
    {QZSettings::proform_treadmill_575i, QZSettings::default_proform_treadmill_575i},
    {QZSettings::zwift_play_emulator, QZSettings::default_zwift_play_emulator},
    {QZSettings::gear_configuration, QZSettings::default_gear_configuration},
    {QZSettings::gear_crankset_size, QZSettings::default_gear_crankset_size},
    {QZSettings::gear_cog_size, QZSettings::default_gear_cog_size},
    {QZSettings::gear_wheel_size, QZSettings::default_gear_wheel_size},
    {QZSettings::gear_circumference, QZSettings::default_gear_circumference},
    {QZSettings::watt_bike_emulator, QZSettings::default_watt_bike_emulator},
    {QZSettings::restore_specific_gear, QZSettings::default_restore_specific_gear},
    {QZSettings::skipLocationServicesDialog, QZSettings::default_skipLocationServicesDialog},
    {QZSettings::trainprogram_pid_pushy, QZSettings::default_trainprogram_pid_pushy},
    {QZSettings::min_inclination, QZSettings::default_min_inclination},
    {QZSettings::proform_performance_300i, QZSettings::default_proform_performance_300i},
    {QZSettings::proform_performance_400i, QZSettings::default_proform_performance_400i},
    {QZSettings::proform_treadmill_c700, QZSettings::default_proform_treadmill_c700},
    {QZSettings::sram_axs_controller, QZSettings::default_sram_axs_controller},
    {QZSettings::proform_treadmill_c960i, QZSettings::default_proform_treadmill_c960i},
    {QZSettings::mqtt_host, QZSettings::default_mqtt_host},
    {QZSettings::mqtt_port, QZSettings::default_mqtt_port},
    {QZSettings::mqtt_username, QZSettings::default_mqtt_username},
    {QZSettings::mqtt_password, QZSettings::default_mqtt_password},
    {QZSettings::mqtt_deviceid, QZSettings::default_mqtt_deviceid},
    {QZSettings::peloton_auto_start_with_intro, QZSettings::default_peloton_auto_start_with_intro},
    {QZSettings::peloton_auto_start_without_intro, QZSettings::default_peloton_auto_start_without_intro},
    {QZSettings::nordictrack_tseries5_treadmill, QZSettings::default_nordictrack_tseries5_treadmill},
    {QZSettings::proform_carbon_tl_PFTL59722c, QZSettings::default_proform_carbon_tl_PFTL59722c},
    {QZSettings::nordictrack_gx_44_pro, QZSettings::default_nordictrack_gx_44_pro},
    {QZSettings::OSC_ip, QZSettings::default_OSC_ip},
    {QZSettings::OSC_port, QZSettings::default_OSC_port},
    {QZSettings::strava_treadmill, QZSettings::default_strava_treadmill},
    {QZSettings::iconsole_rower, QZSettings::default_iconsole_rower},
    {QZSettings::proform_treadmill_1500_pro, QZSettings::default_proform_treadmill_1500_pro},
    {QZSettings::proform_505_cst_80_44, QZSettings::default_proform_505_cst_80_44},
    {QZSettings::proform_trainer_8_0, QZSettings::default_proform_trainer_8_0},
    {QZSettings::tile_biggears_swap, QZSettings::default_tile_biggears_swap},
    {QZSettings::treadmill_follow_wattage, QZSettings::default_treadmill_follow_wattage},
    {QZSettings::fit_file_garmin_device_training_effect, QZSettings::default_fit_file_garmin_device_training_effect},
    {QZSettings::proform_treadmill_705_cst_V80_44, QZSettings::default_proform_treadmill_705_cst_V80_44},
    {QZSettings::nordictrack_treadmill_1750_adb, QZSettings::default_nordictrack_treadmill_1750_adb},
    
    {QZSettings::tile_preset_powerzone_1_enabled, QZSettings::default_tile_preset_powerzone_1_enabled},
    {QZSettings::tile_preset_powerzone_1_order, QZSettings::default_tile_preset_powerzone_1_order},
    {QZSettings::tile_preset_powerzone_1_value, QZSettings::default_tile_preset_powerzone_1_value},
    {QZSettings::tile_preset_powerzone_1_label, QZSettings::default_tile_preset_powerzone_1_label},
    {QZSettings::tile_preset_powerzone_1_color, QZSettings::default_tile_preset_powerzone_1_color},

    {QZSettings::tile_preset_powerzone_2_enabled, QZSettings::default_tile_preset_powerzone_2_enabled},
    {QZSettings::tile_preset_powerzone_2_order, QZSettings::default_tile_preset_powerzone_2_order},
    {QZSettings::tile_preset_powerzone_2_value, QZSettings::default_tile_preset_powerzone_2_value},
    {QZSettings::tile_preset_powerzone_2_label, QZSettings::default_tile_preset_powerzone_2_label},
    {QZSettings::tile_preset_powerzone_2_color, QZSettings::default_tile_preset_powerzone_2_color},

    {QZSettings::tile_preset_powerzone_3_enabled, QZSettings::default_tile_preset_powerzone_3_enabled},
    {QZSettings::tile_preset_powerzone_3_order, QZSettings::default_tile_preset_powerzone_3_order},
    {QZSettings::tile_preset_powerzone_3_value, QZSettings::default_tile_preset_powerzone_3_value}, 
    {QZSettings::tile_preset_powerzone_3_label, QZSettings::default_tile_preset_powerzone_3_label},
    {QZSettings::tile_preset_powerzone_3_color, QZSettings::default_tile_preset_powerzone_3_color},

    {QZSettings::tile_preset_powerzone_4_enabled, QZSettings::default_tile_preset_powerzone_4_enabled},
    {QZSettings::tile_preset_powerzone_4_order, QZSettings::default_tile_preset_powerzone_4_order},
    {QZSettings::tile_preset_powerzone_4_value, QZSettings::default_tile_preset_powerzone_4_value},
    {QZSettings::tile_preset_powerzone_4_label, QZSettings::default_tile_preset_powerzone_4_label},
    {QZSettings::tile_preset_powerzone_4_color, QZSettings::default_tile_preset_powerzone_4_color},

    {QZSettings::tile_preset_powerzone_5_enabled, QZSettings::default_tile_preset_powerzone_5_enabled},
    {QZSettings::tile_preset_powerzone_5_order, QZSettings::default_tile_preset_powerzone_5_order},
    {QZSettings::tile_preset_powerzone_5_value, QZSettings::default_tile_preset_powerzone_5_value},
    {QZSettings::tile_preset_powerzone_5_label, QZSettings::default_tile_preset_powerzone_5_label},
    {QZSettings::tile_preset_powerzone_5_color, QZSettings::default_tile_preset_powerzone_5_color},

    {QZSettings::tile_preset_powerzone_6_enabled, QZSettings::default_tile_preset_powerzone_6_enabled},
    {QZSettings::tile_preset_powerzone_6_order, QZSettings::default_tile_preset_powerzone_6_order},
    {QZSettings::tile_preset_powerzone_6_value, QZSettings::default_tile_preset_powerzone_6_value},
    {QZSettings::tile_preset_powerzone_6_label, QZSettings::default_tile_preset_powerzone_6_label}, 
    {QZSettings::tile_preset_powerzone_6_color, QZSettings::default_tile_preset_powerzone_6_color},

    {QZSettings::tile_preset_powerzone_7_enabled, QZSettings::default_tile_preset_powerzone_7_enabled},
    {QZSettings::tile_preset_powerzone_7_order, QZSettings::default_tile_preset_powerzone_7_order},
    {QZSettings::tile_preset_powerzone_7_value, QZSettings::default_tile_preset_powerzone_7_value},
    {QZSettings::tile_preset_powerzone_7_label, QZSettings::default_tile_preset_powerzone_7_label},
    {QZSettings::tile_preset_powerzone_7_color, QZSettings::default_tile_preset_powerzone_7_color},    

    {QZSettings::proform_bike_PFEVEX71316_0, QZSettings::default_proform_bike_PFEVEX71316_0},
    {QZSettings::real_inclination_to_virtual_treamill_bridge, QZSettings::default_real_inclination_to_virtual_treamill_bridge},
    {QZSettings::stryd_inclination_instead_treadmill, QZSettings::default_stryd_inclination_instead_treadmill},
    {QZSettings::domyos_elliptical_fmts, QZSettings::default_domyos_elliptical_fmts},
    {QZSettings::proform_xbike, QZSettings::default_proform_xbike},
    {QZSettings::peloton_current_user_id, QZSettings::default_peloton_current_user_id},
    {QZSettings::proform_225_csx_PFEX32925_INT_0, QZSettings::proform_225_csx_PFEX32925_INT_0},
    {QZSettings::trainprogram_pid_ignore_inclination, QZSettings::default_trainprogram_pid_ignore_inclination},

    {QZSettings::android_antbike, QZSettings::default_android_antbike},

    {QZSettings::tile_hr_time_in_zone_1_enabled, QZSettings::default_tile_hr_time_in_zone_1_enabled},
    {QZSettings::tile_hr_time_in_zone_1_order, QZSettings::default_tile_hr_time_in_zone_1_order},
    {QZSettings::tile_hr_time_in_zone_2_enabled, QZSettings::default_tile_hr_time_in_zone_2_enabled},
    {QZSettings::tile_hr_time_in_zone_2_order, QZSettings::default_tile_hr_time_in_zone_2_order},
    {QZSettings::tile_hr_time_in_zone_3_enabled, QZSettings::default_tile_hr_time_in_zone_3_enabled},
    {QZSettings::tile_hr_time_in_zone_3_order, QZSettings::default_tile_hr_time_in_zone_3_order},
    {QZSettings::tile_hr_time_in_zone_4_enabled, QZSettings::default_tile_hr_time_in_zone_4_enabled},
    {QZSettings::tile_hr_time_in_zone_4_order, QZSettings::default_tile_hr_time_in_zone_4_order},
    {QZSettings::tile_hr_time_in_zone_5_enabled, QZSettings::default_tile_hr_time_in_zone_5_enabled},
    {QZSettings::tile_hr_time_in_zone_5_order, QZSettings::default_tile_hr_time_in_zone_5_order},
    {QZSettings::ios_btdevice_native, QZSettings::default_ios_btdevice_native},

    {QZSettings::zwift_gear_ui_aligned, QZSettings::default_zwift_gear_ui_aligned},
    {QZSettings::tacxneo2_disable_negative_inclination, QZSettings::default_tacxneo2_disable_negative_inclination},

    {QZSettings::tile_coretemperature_enabled, QZSettings::default_tile_coretemperature_enabled},
    {QZSettings::tile_coretemperature_order, QZSettings::default_tile_coretemperature_order},

    {QZSettings::nordictrack_t65s_treadmill_81_miles, QZSettings::default_nordictrack_t65s_treadmill_81_miles},
    {QZSettings::nordictrack_elite_800, QZSettings::default_nordictrack_elite_800},
    {QZSettings::inclinationResistancePoints, QZSettings::default_inclinationResistancePoints},
    {QZSettings::floatingwindow_type, QZSettings::default_floatingwindow_type},
};

void QZSettings::qDebugAllSettings(bool showDefaults) {
    QSettings settings;
    // make a copy of the settings for sorting
    std::vector<QVariant *> sorted;
    for (uint32_t i = 0; i < allSettingsCount; i++) {
        sorted.push_back(allSettings[i]);
    }
    // sort the settings alphabetically
    struct {
        bool operator()(QVariant *a, QVariant *b) { return a[0].toString() < b[0].toString(); }
    } comparer;
    std::sort(sorted.begin(), sorted.end(), comparer);
    for (uint32_t i = 0; i < sorted.size(); i++) {
        QVariant *item = sorted[i];
        QString key = item[0].toString();
        QVariant defaultValue = item[1];
        if (!showDefaults) {
            qDebug() << key << settings.value(key, defaultValue);
        } else {
            qDebug() << "(" << key << ", " << defaultValue << ") = " << settings.value(key, defaultValue);
        }
    }
}

void QZSettings::restoreAll() {
    qDebug() << QStringLiteral("RESTORING SETTINGS!");
    QSettings settings;
    for (uint32_t i = 0; i < allSettingsCount; i++) {
        settings.setValue(allSettings[i][0].toString(), allSettings[i][1]);
    }
}
