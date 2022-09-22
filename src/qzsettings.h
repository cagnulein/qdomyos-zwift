#ifndef QZSETTINGS_H
#define QZSETTINGS_H

#include <QString>

namespace QZSettings {

    //--------------------------------------------------------------------------------------------
    // These are not in settings.qml
    //--------------------------------------------------------------------------------------------
    /**
	 *@brief 
	*/
	static const QString bluetooth_no_reconnection = QStringLiteral("bluetooth_no_reconnection");
    static const bool default_bluetooth_no_reconnection = false;
    /**
	 *@brief 
	*/
	static const QString bike_wheel_revs = QStringLiteral("bike_wheel_revs");
    static const bool default_bike_wheel_revs = false;
    /**
	 *@brief 
	*/
	static const QString bluetooth_lastdevice_name = QStringLiteral("bluetooth_lastdevice_name");
    static const QString default_bluetooth_lastdevice_name = QStringLiteral("");
    /**
	 *@brief 
	*/
	static const QString bluetooth_lastdevice_address = QStringLiteral("bluetooth_lastdevice_address");
    static const QString default_bluetooth_lastdevice_address = QStringLiteral("");
    /**
	 *@brief 
	*/
	static const QString hrm_lastdevice_name = QStringLiteral("hrm_lastdevice_name");
	static const QString default_hrm_lastdevice_name = QStringLiteral("");
    /**
	 *@brief 
	*/
	static const QString hrm_lastdevice_address = QStringLiteral("hrm_lastdevice_address");
	static const QString default_hrm_lastdevice_address = QStringLiteral("");
    /**
	 *@brief 
	*/
	static const QString ftms_accessory_address = QStringLiteral("ftms_accessory_address");
    static const QString default_ftms_accessory_address = QStringLiteral("");
    /**
	 *@brief 
	*/
	static const QString ftms_accessory_lastdevice_name = QStringLiteral("ftms_accessory_lastdevice_name");
    static const QString default_ftms_accessory_lastdevice_name = QStringLiteral("");
    /**
	 *@brief 
	*/
	static const QString csc_sensor_address = QStringLiteral("csc_sensor_address");
	static const QString default_csc_sensor_address = QStringLiteral("");
    /**
	 *@brief 
	*/
	static const QString csc_sensor_lastdevice_name = QStringLiteral("csc_sensor_lastdevice_name");
	static const QString default_csc_sensor_lastdevice_name = QStringLiteral("");
    /**
	 *@brief 
	*/
	static const QString power_sensor_lastdevice_name = QStringLiteral("power_sensor_lastdevice_name");
	static const QString default_power_sensor_lastdevice_name = QStringLiteral("");
    /**
	 *@brief 
	*/
	static const QString power_sensor_address = QStringLiteral("power_sensor_address");
	static const QString default_power_sensor_address = QStringLiteral("");
    /**
	 *@brief 
	*/
	static const QString elite_rizer_lastdevice_name = QStringLiteral("elite_rizer_lastdevice_name");
	static const QString default_elite_rizer_lastdevice_name = QStringLiteral("");
    /**
	 *@brief 
	*/
	static const QString elite_rizer_address = QStringLiteral("elite_rizer_address");
	static const QString default_elite_rizer_address = QStringLiteral("");
    /**
	 *@brief 
	*/
	static const QString elite_sterzo_smart_lastdevice_name = QStringLiteral("elite_sterzo_smart_lastdevice_name");
	static const QString default_elite_sterzo_smart_lastdevice_name = QStringLiteral("");
    /**
	 *@brief 
	*/
	static const QString elite_sterzo_smart_address = QStringLiteral("elite_sterzo_smart_address");
	static const QString default_elite_sterzo_smart_address = QStringLiteral("");
    /**
	 *@brief 
	*/
	static const QString strava_accesstoken = QStringLiteral("strava_accesstoken");
    static const QString default_strava_accesstoken = QStringLiteral("");


    /**
	 *@brief 
	*/
	static const QString strava_refreshtoken = QStringLiteral("strava_refreshtoken");
    static const QString default_strava_refreshtoken = QStringLiteral("");
    /**
	 *@brief 
	*/
	static const QString strava_lastrefresh = QStringLiteral("strava_lastrefresh");
    static const QString default_strava_lastrefresh = QStringLiteral("");
    /**
	 *@brief 
	*/
	static const QString strava_expires = QStringLiteral("strava_expires");
    static const QString default_strava_expires = QStringLiteral("");

    /**
	 *@brief 
	*/
	static const QString virtualrower_forceresistance = QStringLiteral("virtualrower_forceresistance");
    static constexpr bool default_virtualrower_forceresistance = true;

    /**
     * @brief Strava code.
     */
    static const QString code = QStringLiteral("code");
    static const QString default_code = QStringLiteral("");

    //--------------------------------------------------------------------------------------------

    /**
	 *@brief 
	*/
	static const QString ui_zoom = QStringLiteral("ui_zoom");
	static constexpr float default_ui_zoom = 100.0;
	/**
	 *@brief 
	*/
	static const QString bike_heartrate_service = QStringLiteral("bike_heartrate_service");
	static constexpr bool default_bike_heartrate_service = false;
	/**
	 *@brief 
	*/
	static const QString bike_resistance_offset = QStringLiteral("bike_resistance_offset");
	static constexpr int default_bike_resistance_offset = 4;
    /**
	 *@brief 
	*/
	static const QString bike_resistance_gain_f = QStringLiteral("bike_resistance_gain_f");
	static constexpr float default_bike_resistance_gain_f = 1.0;
	/**
	 *@brief 
	*/
	static const QString zwift_erg = QStringLiteral("zwift_erg");
	static constexpr bool default_zwift_erg = false;
    /**
	 *@brief 
	*/
	static const QString zwift_erg_filter = QStringLiteral("zwift_erg_filter");
	static constexpr float default_zwift_erg_filter = 10.0;
    /**
	 *@brief 
	*/
	static const QString zwift_erg_filter_down = QStringLiteral("zwift_erg_filter_down");
	static constexpr float default_zwift_erg_filter_down = 10.0;
	/**
	 *@brief 
	*/
	static const QString zwift_negative_inclination_x2 = QStringLiteral("zwift_negative_inclination_x2");
	static constexpr bool default_zwift_negative_inclination_x2 = false;
    /**
	 *@brief 
	*/
	static const QString zwift_inclination_offset = QStringLiteral("zwift_inclination_offset");
	static constexpr float default_zwift_inclination_offset = 0;
    /**
	 *@brief 
	*/
	static const QString zwift_inclination_gain = QStringLiteral("zwift_inclination_gain");
	static constexpr float default_zwift_inclination_gain = 1.0;
    /**
	 *@brief 
	*/
	static const QString echelon_resistance_offset = QStringLiteral("echelon_resistance_offset");
	static constexpr float default_echelon_resistance_offset = 0;
    /**
	 *@brief 
	*/
	static const QString echelon_resistance_gain = QStringLiteral("echelon_resistance_gain");
	static constexpr float default_echelon_resistance_gain = 1.0;
	/**
	 *@brief 
	*/
	static const QString speed_power_based = QStringLiteral("speed_power_based");
	static constexpr bool default_speed_power_based = false;
	/**
	 *@brief 
	*/
	static const QString bike_resistance_start = QStringLiteral("bike_resistance_start");
	static constexpr int default_bike_resistance_start = 1;
	/**
	 *@brief 
	*/
	static const QString age = QStringLiteral("age");
	static constexpr int default_age = 35.0;
    /**
	 *@brief 
	*/
	static const QString weight = QStringLiteral("weight");
	static constexpr float default_weight = 75.0;
    /**
	 *@brief 
	*/
	static const QString ftp = QStringLiteral("ftp");
	static constexpr float default_ftp = 200.0;
    /**
	 *@brief 
	*/
	static const QString user_email = QStringLiteral("user_email");
	static const QString default_user_email = QStringLiteral("");
	/**
	 *@brief 
	*/
	static const QString user_nickname = QStringLiteral("user_nickname");
	static const QString default_user_nickname = QStringLiteral("");
	/**
	 *@brief 
	*/
	static const QString miles_unit = QStringLiteral("miles_unit");
	static constexpr bool default_miles_unit = false;
	/**
	 *@brief 
	*/
	static const QString pause_on_start = QStringLiteral("pause_on_start");
	static constexpr bool default_pause_on_start = false;
	/**
	 *@brief 
	*/
	static const QString treadmill_force_speed = QStringLiteral("treadmill_force_speed");
	static constexpr bool default_treadmill_force_speed = false;
	/**
	 *@brief 
	*/
	static const QString pause_on_start_treadmill = QStringLiteral("pause_on_start_treadmill");
	static constexpr bool default_pause_on_start_treadmill = false;
	/**
	 *@brief 
	*/
	static const QString continuous_moving = QStringLiteral("continuous_moving");
	static constexpr bool default_continuous_moving = false;
	/**
	 *@brief 
	*/
	static const QString bike_cadence_sensor = QStringLiteral("bike_cadence_sensor");
	static constexpr bool default_bike_cadence_sensor = false;
	/**
	 *@brief 
	*/
	static const QString run_cadence_sensor = QStringLiteral("run_cadence_sensor");
	static constexpr bool default_run_cadence_sensor = false;
	/**
	 *@brief 
	*/
	static const QString bike_power_sensor = QStringLiteral("bike_power_sensor");
	static constexpr bool default_bike_power_sensor = false;
	/**
	 *@brief 
	*/
	static const QString heart_rate_belt_name = QStringLiteral("heart_rate_belt_name");
	static const QString default_heart_rate_belt_name = QStringLiteral("Disabled");
	/**
	 *@brief 
	*/
	static const QString heart_ignore_builtin = QStringLiteral("heart_ignore_builtin");
	static constexpr bool default_heart_ignore_builtin = false;
	/**
	 *@brief 
	*/
	static const QString kcal_ignore_builtin = QStringLiteral("kcal_ignore_builtin");
	static constexpr bool default_kcal_ignore_builtin = false;
	/**
	 *@brief 
	*/
	static const QString ant_cadence = QStringLiteral("ant_cadence");
	static constexpr bool default_ant_cadence = false;
	/**
	 *@brief 
	*/
	static const QString ant_heart = QStringLiteral("ant_heart");
	static constexpr bool default_ant_heart = false;
	/**
	 *@brief 
	*/
	static const QString ant_garmin = QStringLiteral("ant_garmin");
	static constexpr bool default_ant_garmin = false;
	/**
	 *@brief 
	*/
	static const QString top_bar_enabled = QStringLiteral("top_bar_enabled");
	static constexpr bool default_top_bar_enabled = true;
	/**
	 *@brief 
	*/
	static const QString peloton_username = QStringLiteral("peloton_username");
	static const QString default_peloton_username = QStringLiteral("username");
	/**
	 *@brief 
	*/
	static const QString peloton_password = QStringLiteral("peloton_password");
	static const QString default_peloton_password = QStringLiteral("password");
	/**
	 *@brief 
	*/
	static const QString peloton_difficulty = QStringLiteral("peloton_difficulty");
	static const QString default_peloton_difficulty = QStringLiteral("lower");
	/**
	 *@brief 
	*/
	static const QString peloton_cadence_metric = QStringLiteral("peloton_cadence_metric");
	static const QString default_peloton_cadence_metric = QStringLiteral("Cadence");
	/**
	 *@brief 
	*/
	static const QString peloton_heartrate_metric = QStringLiteral("peloton_heartrate_metric");
	static const QString default_peloton_heartrate_metric = QStringLiteral("Heart Rate");
	/**
	 *@brief 
	*/
	static const QString peloton_date = QStringLiteral("peloton_date");
	static const QString default_peloton_date = QStringLiteral("Before Title");
	/**
	 *@brief 
	*/
	static const QString peloton_description_link = QStringLiteral("peloton_description_link");
	static constexpr bool default_peloton_description_link = true;
	/**
	 *@brief 
	*/
	static const QString pzp_username = QStringLiteral("pzp_username");
	static const QString default_pzp_username = QStringLiteral("username");
	/**
	 *@brief 
	*/
	static const QString pzp_password = QStringLiteral("pzp_password");
	static const QString default_pzp_password = QStringLiteral("username");
	/**
	 *@brief 
	*/
	static const QString tile_speed_enabled = QStringLiteral("tile_speed_enabled");
	static constexpr bool default_tile_speed_enabled = true;
	/**
	 *@brief 
	*/
	static const QString tile_speed_order = QStringLiteral("tile_speed_order");
	static constexpr int default_tile_speed_order = 0;
	/**
	 *@brief 
	*/
	static const QString tile_inclination_enabled = QStringLiteral("tile_inclination_enabled");
	static constexpr bool default_tile_inclination_enabled = true;
	/**
	 *@brief 
	*/
	static const QString tile_inclination_order = QStringLiteral("tile_inclination_order");
	static constexpr int default_tile_inclination_order = 1;
	/**
	 *@brief 
	*/
	static const QString tile_cadence_enabled = QStringLiteral("tile_cadence_enabled");
	static constexpr bool default_tile_cadence_enabled = true;
	/**
	 *@brief 
	*/
	static const QString tile_cadence_order = QStringLiteral("tile_cadence_order");
	static constexpr int default_tile_cadence_order = 2;
	/**
	 *@brief 
	*/
	static const QString tile_elevation_enabled = QStringLiteral("tile_elevation_enabled");
	static constexpr bool default_tile_elevation_enabled = true;
	/**
	 *@brief 
	*/
	static const QString tile_elevation_order = QStringLiteral("tile_elevation_order");
	static constexpr int default_tile_elevation_order = 3;
	/**
	 *@brief 
	*/
	static const QString tile_calories_enabled = QStringLiteral("tile_calories_enabled");
	static constexpr bool default_tile_calories_enabled = true;
	/**
	 *@brief 
	*/
	static const QString tile_calories_order = QStringLiteral("tile_calories_order");
	static constexpr int default_tile_calories_order = 4;
	/**
	 *@brief 
	*/
	static const QString tile_odometer_enabled = QStringLiteral("tile_odometer_enabled");
	static constexpr bool default_tile_odometer_enabled = true;
	/**
	 *@brief 
	*/
	static const QString tile_odometer_order = QStringLiteral("tile_odometer_order");
	static constexpr int default_tile_odometer_order = 5;
	/**
	 *@brief 
	*/
	static const QString tile_pace_enabled = QStringLiteral("tile_pace_enabled");
	static constexpr bool default_tile_pace_enabled = true;
	/**
	 *@brief 
	*/
	static const QString tile_pace_order = QStringLiteral("tile_pace_order");
	static constexpr int default_tile_pace_order = 6;
	/**
	 *@brief 
	*/
	static const QString tile_resistance_enabled = QStringLiteral("tile_resistance_enabled");
	static constexpr bool default_tile_resistance_enabled = true;
	/**
	 *@brief 
	*/
	static const QString tile_resistance_order = QStringLiteral("tile_resistance_order");
	static constexpr int default_tile_resistance_order = 7;
	/**
	 *@brief 
	*/
	static const QString tile_watt_enabled = QStringLiteral("tile_watt_enabled");
	static constexpr bool default_tile_watt_enabled = true;
	/**
	 *@brief 
	*/
	static const QString tile_watt_order = QStringLiteral("tile_watt_order");
	static constexpr int default_tile_watt_order = 8;
	/**
	 *@brief 
	*/
	static const QString tile_weight_loss_enabled = QStringLiteral("tile_weight_loss_enabled");
	static constexpr bool default_tile_weight_loss_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tile_weight_loss_order = QStringLiteral("tile_weight_loss_order");
	static constexpr int default_tile_weight_loss_order = 24;
	/**
	 *@brief 
	*/
	static const QString tile_avgwatt_enabled = QStringLiteral("tile_avgwatt_enabled");
	static constexpr bool default_tile_avgwatt_enabled = true;
	/**
	 *@brief 
	*/
	static const QString tile_avgwatt_order = QStringLiteral("tile_avgwatt_order");
	static constexpr int default_tile_avgwatt_order = 9;
	/**
	 *@brief 
	*/
	static const QString tile_ftp_enabled = QStringLiteral("tile_ftp_enabled");
	static constexpr bool default_tile_ftp_enabled = true;
	/**
	 *@brief 
	*/
	static const QString tile_ftp_order = QStringLiteral("tile_ftp_order");
	static constexpr int default_tile_ftp_order = 10;
	/**
	 *@brief 
	*/
	static const QString tile_heart_enabled = QStringLiteral("tile_heart_enabled");
	static constexpr bool default_tile_heart_enabled = true;
	/**
	 *@brief 
	*/
	static const QString tile_heart_order = QStringLiteral("tile_heart_order");
	static constexpr int default_tile_heart_order = 11;
	/**
	 *@brief 
	*/
	static const QString tile_fan_enabled = QStringLiteral("tile_fan_enabled");
	static constexpr bool default_tile_fan_enabled = true;
	/**
	 *@brief 
	*/
	static const QString tile_fan_order = QStringLiteral("tile_fan_order");
	static constexpr int default_tile_fan_order = 12;
	/**
	 *@brief 
	*/
	static const QString tile_jouls_enabled = QStringLiteral("tile_jouls_enabled");
	static constexpr bool default_tile_jouls_enabled = true;
	/**
	 *@brief 
	*/
	static const QString tile_jouls_order = QStringLiteral("tile_jouls_order");
	static constexpr int default_tile_jouls_order = 13;
	/**
	 *@brief 
	*/
	static const QString tile_elapsed_enabled = QStringLiteral("tile_elapsed_enabled");
	static constexpr bool default_tile_elapsed_enabled = true;
	/**
	 *@brief 
	*/
	static const QString tile_elapsed_order = QStringLiteral("tile_elapsed_order");
	static constexpr int default_tile_elapsed_order = 14;
	/**
	 *@brief 
	*/
	static const QString tile_lapelapsed_enabled = QStringLiteral("tile_lapelapsed_enabled");
	static constexpr bool default_tile_lapelapsed_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tile_lapelapsed_order = QStringLiteral("tile_lapelapsed_order");
	static constexpr int default_tile_lapelapsed_order = 17;
	/**
	 *@brief 
	*/
	static const QString tile_moving_time_enabled = QStringLiteral("tile_moving_time_enabled");
	static constexpr bool default_tile_moving_time_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tile_moving_time_order = QStringLiteral("tile_moving_time_order");
	static constexpr int default_tile_moving_time_order = 21;
	/**
	 *@brief 
	*/
	static const QString tile_peloton_offset_enabled = QStringLiteral("tile_peloton_offset_enabled");
	static constexpr bool default_tile_peloton_offset_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tile_peloton_offset_order = QStringLiteral("tile_peloton_offset_order");
	static constexpr int default_tile_peloton_offset_order = 22;
	/**
	 *@brief 
	*/
	static const QString tile_peloton_difficulty_enabled = QStringLiteral("tile_peloton_difficulty_enabled");
	static constexpr bool default_tile_peloton_difficulty_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tile_peloton_difficulty_order = QStringLiteral("tile_peloton_difficulty_order");
	static constexpr int default_tile_peloton_difficulty_order = 32;
	/**
	 *@brief 
	*/
	static const QString tile_peloton_resistance_enabled = QStringLiteral("tile_peloton_resistance_enabled");
	static constexpr bool default_tile_peloton_resistance_enabled = true;
	/**
	 *@brief 
	*/
	static const QString tile_peloton_resistance_order = QStringLiteral("tile_peloton_resistance_order");
	static constexpr int default_tile_peloton_resistance_order = 15;
	/**
	 *@brief 
	*/
	static const QString tile_datetime_enabled = QStringLiteral("tile_datetime_enabled");
	static constexpr bool default_tile_datetime_enabled = true;
	/**
	 *@brief 
	*/
	static const QString tile_datetime_order = QStringLiteral("tile_datetime_order");
	static constexpr int default_tile_datetime_order = 16;
	/**
	 *@brief 
	*/
	static const QString tile_target_resistance_enabled = QStringLiteral("tile_target_resistance_enabled");
	static constexpr bool default_tile_target_resistance_enabled = true;
	/**
	 *@brief 
	*/
	static const QString tile_target_resistance_order = QStringLiteral("tile_target_resistance_order");
	static constexpr int default_tile_target_resistance_order = 15;
	/**
	 *@brief 
	*/
	static const QString tile_target_peloton_resistance_enabled = QStringLiteral("tile_target_peloton_resistance_enabled");
	static constexpr bool default_tile_target_peloton_resistance_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tile_target_peloton_resistance_order = QStringLiteral("tile_target_peloton_resistance_order");
	static constexpr int default_tile_target_peloton_resistance_order = 21;
	/**
	 *@brief 
	*/
	static const QString tile_target_cadence_enabled = QStringLiteral("tile_target_cadence_enabled");
	static constexpr bool default_tile_target_cadence_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tile_target_cadence_order = QStringLiteral("tile_target_cadence_order");
	static constexpr int default_tile_target_cadence_order = 19;
	/**
	 *@brief 
	*/
	static const QString tile_target_power_enabled = QStringLiteral("tile_target_power_enabled");
	static constexpr bool default_tile_target_power_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tile_target_power_order = QStringLiteral("tile_target_power_order");
	static constexpr int default_tile_target_power_order = 20;
	/**
	 *@brief 
	*/
	static const QString tile_target_zone_enabled = QStringLiteral("tile_target_zone_enabled");
	static constexpr bool default_tile_target_zone_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tile_target_zone_order = QStringLiteral("tile_target_zone_order");
	static constexpr int default_tile_target_zone_order = 24;
	/**
	 *@brief 
	*/
	static const QString tile_target_speed_enabled = QStringLiteral("tile_target_speed_enabled");
	static constexpr bool default_tile_target_speed_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tile_target_speed_order = QStringLiteral("tile_target_speed_order");
	static constexpr int default_tile_target_speed_order = 27;
	/**
	 *@brief 
	*/
	static const QString tile_target_incline_enabled = QStringLiteral("tile_target_incline_enabled");
	static constexpr bool default_tile_target_incline_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tile_target_incline_order = QStringLiteral("tile_target_incline_order");
	static constexpr int default_tile_target_incline_order = 28;
	/**
	 *@brief 
	*/
	static const QString tile_strokes_count_enabled = QStringLiteral("tile_strokes_count_enabled");
	static constexpr bool default_tile_strokes_count_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tile_strokes_count_order = QStringLiteral("tile_strokes_count_order");
	static constexpr int default_tile_strokes_count_order = 22;
	/**
	 *@brief 
	*/
	static const QString tile_strokes_length_enabled = QStringLiteral("tile_strokes_length_enabled");
	static constexpr bool default_tile_strokes_length_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tile_strokes_length_order = QStringLiteral("tile_strokes_length_order");
	static constexpr int default_tile_strokes_length_order = 23;
	/**
	 *@brief 
	*/
	static const QString tile_watt_kg_enabled = QStringLiteral("tile_watt_kg_enabled");
	static constexpr bool default_tile_watt_kg_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tile_watt_kg_order = QStringLiteral("tile_watt_kg_order");
	static constexpr int default_tile_watt_kg_order = 25;
	/**
	 *@brief 
	*/
	static const QString tile_gears_enabled = QStringLiteral("tile_gears_enabled");
	static constexpr bool default_tile_gears_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tile_gears_order = QStringLiteral("tile_gears_order");
	static constexpr int default_tile_gears_order = 26;
	/**
	 *@brief 
	*/
	static const QString tile_remainingtimetrainprogramrow_enabled = QStringLiteral("tile_remainingtimetrainprogramrow_enabled");
	static constexpr bool default_tile_remainingtimetrainprogramrow_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tile_remainingtimetrainprogramrow_order = QStringLiteral("tile_remainingtimetrainprogramrow_order");
	static constexpr int default_tile_remainingtimetrainprogramrow_order = 27;
	/**
	 *@brief 
	*/
	static const QString tile_nextrowstrainprogram_enabled = QStringLiteral("tile_nextrowstrainprogram_enabled");
	static constexpr bool default_tile_nextrowstrainprogram_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tile_nextrowstrainprogram_order = QStringLiteral("tile_nextrowstrainprogram_order");
	static constexpr int default_tile_nextrowstrainprogram_order = 31;
	/**
	 *@brief 
	*/
	static const QString tile_mets_enabled = QStringLiteral("tile_mets_enabled");
	static constexpr bool default_tile_mets_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tile_mets_order = QStringLiteral("tile_mets_order");
	static constexpr int default_tile_mets_order = 28;
	/**
	 *@brief 
	*/
	static const QString tile_targetmets_enabled = QStringLiteral("tile_targetmets_enabled");
	static constexpr bool default_tile_targetmets_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tile_targetmets_order = QStringLiteral("tile_targetmets_order");
	static constexpr int default_tile_targetmets_order = 29;
	/**
	 *@brief 
	*/
	static const QString tile_steering_angle_enabled = QStringLiteral("tile_steering_angle_enabled");
	static constexpr bool default_tile_steering_angle_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tile_steering_angle_order = QStringLiteral("tile_steering_angle_order");
	static constexpr int default_tile_steering_angle_order = 30;
	/**
	 *@brief 
	*/
	static const QString tile_pid_hr_enabled = QStringLiteral("tile_pid_hr_enabled");
	static constexpr bool default_tile_pid_hr_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tile_pid_hr_order = QStringLiteral("tile_pid_hr_order");
	static constexpr int default_tile_pid_hr_order = 31;
    /**
	 *@brief 
	*/
	static const QString heart_rate_zone1 = QStringLiteral("heart_rate_zone1");
	static constexpr float default_heart_rate_zone1 = 70.0;
    /**
	 *@brief 
	*/
	static const QString heart_rate_zone2 = QStringLiteral("heart_rate_zone2");
	static constexpr float default_heart_rate_zone2 = 80.0;
    /**
	 *@brief 
	*/
	static const QString heart_rate_zone3 = QStringLiteral("heart_rate_zone3");
	static constexpr float default_heart_rate_zone3 = 90.0;
    /**
	 *@brief 
	*/
	static const QString heart_rate_zone4 = QStringLiteral("heart_rate_zone4");
	static constexpr float default_heart_rate_zone4 = 100.0;
	/**
	 *@brief 
	*/
	static const QString heart_max_override_enable = QStringLiteral("heart_max_override_enable");
	static constexpr bool default_heart_max_override_enable = false;
    /**
	 *@brief 
	*/
	static const QString heart_max_override_value = QStringLiteral("heart_max_override_value");
	static constexpr float default_heart_max_override_value = 195.0;
    /**
	 *@brief 
	*/
	static const QString peloton_gain = QStringLiteral("peloton_gain");
	static constexpr float default_peloton_gain = 1.0;
    /**
	 *@brief 
	*/
	static const QString peloton_offset = QStringLiteral("peloton_offset");
	static constexpr float default_peloton_offset = 0;
	/**
	 *@brief 
	*/
	static const QString treadmill_pid_heart_zone = QStringLiteral("treadmill_pid_heart_zone");
	static const QString default_treadmill_pid_heart_zone = QStringLiteral("Disabled");
    /**
	 *@brief 
	*/
	static const QString pacef_1mile = QStringLiteral("pacef_1mile");
	static constexpr float default_pacef_1mile = 250;
    /**
	 *@brief 
	*/
	static const QString pacef_5km = QStringLiteral("pacef_5km");
	static constexpr float default_pacef_5km = 300;
    /**
	 *@brief 
	*/
	static const QString pacef_10km = QStringLiteral("pacef_10km");
	static constexpr float default_pacef_10km = 320;
    /**
	 *@brief 
	*/
	static const QString pacef_halfmarathon = QStringLiteral("pacef_halfmarathon");
	static constexpr float default_pacef_halfmarathon = 340;
    /**
	 *@brief 
	*/
	static const QString pacef_marathon = QStringLiteral("pacef_marathon");
	static constexpr float default_pacef_marathon = 360;
	/**
	 *@brief 
	*/
	static const QString pace_default = QStringLiteral("pace_default");
	static const QString default_pace_default = QStringLiteral("Half Marathon");
	/**
	 *@brief 
	*/
	static const QString domyos_treadmill_buttons = QStringLiteral("domyos_treadmill_buttons");
	static constexpr bool default_domyos_treadmill_buttons = false;
	/**
	 *@brief 
	*/
	static const QString domyos_treadmill_distance_display = QStringLiteral("domyos_treadmill_distance_display");
	static constexpr bool default_domyos_treadmill_distance_display = true;
	/**
	 *@brief 
	*/
	static const QString domyos_treadmill_display_invert = QStringLiteral("domyos_treadmill_display_invert");
	static constexpr bool default_domyos_treadmill_display_invert = false;
    /**
	 *@brief 
	*/
	static const QString domyos_bike_cadence_filter = QStringLiteral("domyos_bike_cadence_filter");
	static constexpr float default_domyos_bike_cadence_filter = 0.0;
	/**
	 *@brief 
	*/
	static const QString domyos_bike_display_calories = QStringLiteral("domyos_bike_display_calories");
	static constexpr bool default_domyos_bike_display_calories = true;
    /**
	 *@brief 
	*/
	static const QString domyos_elliptical_speed_ratio = QStringLiteral("domyos_elliptical_speed_ratio");
	static constexpr float default_domyos_elliptical_speed_ratio = 1.0;
	/**
	 *@brief 
	*/
	static const QString eslinker_cadenza = QStringLiteral("eslinker_cadenza");
	static constexpr bool default_eslinker_cadenza = true;
	/**
	 *@brief 
	*/
	static const QString echelon_watttable = QStringLiteral("echelon_watttable");
	static const QString default_echelon_watttable = QStringLiteral("Echelon");
    /**
	 *@brief 
	*/
	static const QString proform_wheel_ratio = QStringLiteral("proform_wheel_ratio");
	static constexpr float default_proform_wheel_ratio = 0.33;
	/**
	 *@brief 
	*/
	static const QString proform_tour_de_france_clc = QStringLiteral("proform_tour_de_france_clc");
	static constexpr bool default_proform_tour_de_france_clc = false;
	/**
	 *@brief 
	*/
	static const QString proform_tdf_jonseed_watt = QStringLiteral("proform_tdf_jonseed_watt");
	static constexpr bool default_proform_tdf_jonseed_watt = false;
	/**
	 *@brief 
	*/
	static const QString proform_studio = QStringLiteral("proform_studio");
	static constexpr bool default_proform_studio = false;
	/**
	 *@brief 
	*/
	static const QString proform_tdf_10 = QStringLiteral("proform_tdf_10");
	static constexpr bool default_proform_tdf_10 = false;
	/**
	 *@brief 
	*/
	static const QString horizon_gr7_cadence_multiplier = QStringLiteral("horizon_gr7_cadence_multiplier");
	static constexpr double default_horizon_gr7_cadence_multiplier = 1.0;
	/**
	 *@brief 
	*/
	static const QString fitshow_user_id = QStringLiteral("fitshow_user_id");
	static constexpr int default_fitshow_user_id = 0x13AA;
	/**
	 *@brief 
	*/
	static const QString inspire_peloton_formula = QStringLiteral("inspire_peloton_formula");
	static constexpr bool default_inspire_peloton_formula = false;
	/**
	 *@brief 
	*/
	static const QString inspire_peloton_formula2 = QStringLiteral("inspire_peloton_formula2");
	static constexpr bool default_inspire_peloton_formula2 = false;
	/**
	 *@brief 
	*/
	static const QString hammer_racer_s = QStringLiteral("hammer_racer_s");
	static constexpr bool default_hammer_racer_s = false;
	/**
	 *@brief 
	*/
	static const QString pafers_treadmill = QStringLiteral("pafers_treadmill");
	static constexpr bool default_pafers_treadmill = false;
	/**
	 *@brief 
	*/
	static const QString yesoul_peloton_formula = QStringLiteral("yesoul_peloton_formula");
	static constexpr bool default_yesoul_peloton_formula = false;
	/**
	 *@brief 
	*/
	static const QString nordictrack_10_treadmill = QStringLiteral("nordictrack_10_treadmill");
	static constexpr bool default_nordictrack_10_treadmill = true;
	/**
	 *@brief 
	*/
	static const QString nordictrack_t65s_treadmill = QStringLiteral("nordictrack_t65s_treadmill");
	static constexpr bool default_nordictrack_t65s_treadmill = false;
   
	//static const QString proform_treadmill_995i = QStringLiteral("proform_treadmill_995i");
	//static constexpr bool default_proform_treadmill_995i = false;
	
	/**
	 *@brief 
	*/
	static const QString toorx_3_0 = QStringLiteral("toorx_3_0");
	static constexpr bool default_toorx_3_0 = false;
	/**
	 *@brief 
	*/
	static const QString toorx_65s_evo = QStringLiteral("toorx_65s_evo");
	static constexpr bool default_toorx_65s_evo = false;
	/**
	 *@brief 
	*/
	static const QString jtx_fitness_sprint_treadmill = QStringLiteral("jtx_fitness_sprint_treadmill");
	static constexpr bool default_jtx_fitness_sprint_treadmill = false;
	/**
	 *@brief 
	*/
	static const QString dkn_endurun_treadmill = QStringLiteral("dkn_endurun_treadmill");
	static constexpr bool default_dkn_endurun_treadmill = false;
	/**
	 *@brief 
	*/
	static const QString trx_route_key = QStringLiteral("trx_route_key");
	static constexpr bool default_trx_route_key = false;
	/**
	 *@brief 
	*/
	static const QString bh_spada_2 = QStringLiteral("bh_spada_2");
	static constexpr bool default_bh_spada_2 = false;
	/**
	 *@brief 
	*/
	static const QString toorx_bike = QStringLiteral("toorx_bike");
	static constexpr bool default_toorx_bike = false;
	/**
	 *@brief 
	*/
	static const QString toorx_ftms = QStringLiteral("toorx_ftms");
	static constexpr bool default_toorx_ftms = false;
	/**
	 *@brief 
	*/
	static const QString jll_IC400_bike = QStringLiteral("jll_IC400_bike");
	static constexpr bool default_jll_IC400_bike = false;
	/**
	 *@brief 
	*/
	static const QString fytter_ri08_bike = QStringLiteral("fytter_ri08_bike");
	static constexpr bool default_fytter_ri08_bike = false;
	/**
	 *@brief 
	*/
	static const QString asviva_bike = QStringLiteral("asviva_bike");
	static constexpr bool default_asviva_bike = false;
	/**
	 *@brief 
	*/
	static const QString hertz_xr_770 = QStringLiteral("hertz_xr_770");
	static constexpr bool default_hertz_xr_770 = false;
	/**
	 *@brief 
	*/
	static const QString m3i_bike_id = QStringLiteral("m3i_bike_id");
	static constexpr int default_m3i_bike_id = 256;
	/**
	 *@brief 
	*/
	static const QString m3i_bike_speed_buffsize = QStringLiteral("m3i_bike_speed_buffsize");
	static constexpr int default_m3i_bike_speed_buffsize = 90;
	/**
	 *@brief 
	*/
	static const QString m3i_bike_qt_search = QStringLiteral("m3i_bike_qt_search");
	static constexpr bool default_m3i_bike_qt_search = false;
	/**
	 *@brief 
	*/
	static const QString m3i_bike_kcal = QStringLiteral("m3i_bike_kcal");
	static constexpr bool default_m3i_bike_kcal = true;
	/**
	 *@brief 
	*/
	static const QString snode_bike = QStringLiteral("snode_bike");
	static constexpr bool default_snode_bike = false;
	/**
	 *@brief 
	*/
	static const QString fitplus_bike = QStringLiteral("fitplus_bike");
	static constexpr bool default_fitplus_bike = false;
	/**
	 *@brief 
	*/
	static const QString virtufit_etappe = QStringLiteral("virtufit_etappe");
	static constexpr bool default_virtufit_etappe = false;
	/**
	 *@brief 
	*/
	static const QString flywheel_filter = QStringLiteral("flywheel_filter");
	static constexpr int default_flywheel_filter = 2;
	/**
	 *@brief 
	*/
	static const QString flywheel_life_fitness_ic8 = QStringLiteral("flywheel_life_fitness_ic8");
	static constexpr bool default_flywheel_life_fitness_ic8 = false;
	/**
	 *@brief 
	*/
	static const QString sole_treadmill_inclination = QStringLiteral("sole_treadmill_inclination");
	static constexpr bool default_sole_treadmill_inclination = false;
	/**
	 *@brief 
	*/
	static const QString sole_treadmill_miles = QStringLiteral("sole_treadmill_miles");
	static constexpr bool default_sole_treadmill_miles = true;
	/**
	 *@brief 
	*/
	static const QString sole_treadmill_f65 = QStringLiteral("sole_treadmill_f65");
	static constexpr bool default_sole_treadmill_f65 = false;
	/**
	 *@brief 
	*/
	static const QString sole_treadmill_f63 = QStringLiteral("sole_treadmill_f63");
	static constexpr bool default_sole_treadmill_f63 = false;
	/**
	 *@brief 
	*/
	static const QString sole_treadmill_tt8 = QStringLiteral("sole_treadmill_tt8");
	static constexpr bool default_sole_treadmill_tt8 = false;
	/**
	 *@brief 
	*/
	static const QString schwinn_bike_resistance = QStringLiteral("schwinn_bike_resistance");
	static constexpr bool default_schwinn_bike_resistance = false;
	/**
	 *@brief 
	*/
	static const QString schwinn_bike_resistance_v2 = QStringLiteral("schwinn_bike_resistance_v2");
    static constexpr bool default_schwinn_bike_resistance_v2 = false;
	/**
	 *@brief 
	*/
	static const QString technogym_myrun_treadmill_experimental = QStringLiteral("technogym_myrun_treadmill_experimental");
	static constexpr bool default_technogym_myrun_treadmill_experimental = false;
	/**
	 *@brief 
	*/
	static const QString trainprogram_random = QStringLiteral("trainprogram_random");
	static constexpr bool default_trainprogram_random = false;
	/**
	 *@brief 
	*/
	static const QString trainprogram_total = QStringLiteral("trainprogram_total");
	static constexpr int default_trainprogram_total = 60;
    /**
	 *@brief 
	*/
	static const QString trainprogram_period_seconds = QStringLiteral("trainprogram_period_seconds");
	static constexpr float default_trainprogram_period_seconds = 60;
    /**
	 *@brief 
	*/
	static const QString trainprogram_speed_min = QStringLiteral("trainprogram_speed_min");
	static constexpr float default_trainprogram_speed_min = 8;
    /**
	 *@brief 
	*/
	static const QString trainprogram_speed_max = QStringLiteral("trainprogram_speed_max");
	static constexpr float default_trainprogram_speed_max = 16;
    /**
	 *@brief 
	*/
	static const QString trainprogram_incline_min = QStringLiteral("trainprogram_incline_min");
	static constexpr float default_trainprogram_incline_min = 0;
    /**
	 *@brief 
	*/
	static const QString trainprogram_incline_max = QStringLiteral("trainprogram_incline_max");
	static constexpr float default_trainprogram_incline_max = 15;
    /**
	 *@brief 
	*/
	static const QString trainprogram_resistance_min = QStringLiteral("trainprogram_resistance_min");
	static constexpr float default_trainprogram_resistance_min = 1;
    /**
	 *@brief 
	*/
	static const QString trainprogram_resistance_max = QStringLiteral("trainprogram_resistance_max");
	static constexpr float default_trainprogram_resistance_max = 32;
    /**
	 *@brief 
	*/
	static const QString watt_offset = QStringLiteral("watt_offset");
	static constexpr float default_watt_offset = 0;
    /**
	 *@brief 
	*/
	static const QString watt_gain = QStringLiteral("watt_gain");
	static constexpr float default_watt_gain = 1;
	/**
	 *@brief 
	*/
	static const QString power_avg_5s = QStringLiteral("power_avg_5s");
	static constexpr bool default_power_avg_5s = false;
	/**
	 *@brief 
	*/
	static const QString instant_power_on_pause = QStringLiteral("instant_power_on_pause");
	static constexpr bool default_instant_power_on_pause = false;
    /**
	 *@brief 
	*/
	static const QString speed_offset = QStringLiteral("speed_offset");
	static constexpr float default_speed_offset = 0;
    /**
	 *@brief 
	*/
	static const QString speed_gain = QStringLiteral("speed_gain");
	static constexpr float default_speed_gain = 1;
	/**
	 *@brief 
	*/
	static const QString filter_device = QStringLiteral("filter_device");
	static const QString default_filter_device = QStringLiteral("Disabled");
	/**
	 *@brief 
	*/
	static const QString strava_suffix = QStringLiteral("strava_suffix");
	static const QString default_strava_suffix = QStringLiteral("#QZ");
	/**
	 *@brief 
	*/
	static const QString cadence_sensor_name = QStringLiteral("cadence_sensor_name");
	static const QString default_cadence_sensor_name = QStringLiteral("Disabled");
	/**
	 *@brief 
	*/
	static const QString cadence_sensor_as_bike = QStringLiteral("cadence_sensor_as_bike");
	static constexpr bool default_cadence_sensor_as_bike = false;
    /**
	 *@brief 
	*/
	static const QString cadence_sensor_speed_ratio = QStringLiteral("cadence_sensor_speed_ratio");
	static constexpr float default_cadence_sensor_speed_ratio = 0.33;
    /**
	 *@brief 
	*/
	static const QString power_hr_pwr1 = QStringLiteral("power_hr_pwr1");
	static constexpr float default_power_hr_pwr1 = 200;
    /**
	 *@brief 
	*/
	static const QString power_hr_hr1 = QStringLiteral("power_hr_hr1");
	static constexpr float default_power_hr_hr1 = 150;
    /**
	 *@brief 
	*/
	static const QString power_hr_pwr2 = QStringLiteral("power_hr_pwr2");
	static constexpr float default_power_hr_pwr2 = 230;
    /**
	 *@brief 
	*/
	static const QString power_hr_hr2 = QStringLiteral("power_hr_hr2");
	static constexpr float default_power_hr_hr2 = 170;
	/**
	 *@brief 
	*/
	static const QString power_sensor_name = QStringLiteral("power_sensor_name");
	static const QString default_power_sensor_name = QStringLiteral("Disabled");
	/**
	 *@brief 
	*/
	static const QString power_sensor_as_bike = QStringLiteral("power_sensor_as_bike");
	static constexpr bool default_power_sensor_as_bike = false;
	/**
	 *@brief 
	*/
	static const QString power_sensor_as_treadmill = QStringLiteral("power_sensor_as_treadmill");
	static constexpr bool default_power_sensor_as_treadmill = false;
	/**
	 *@brief 
	*/
	static const QString powr_sensor_running_cadence_double = QStringLiteral("powr_sensor_running_cadence_double");
	static constexpr bool default_powr_sensor_running_cadence_double = false;
	/**
	 *@brief 
	*/
	static const QString elite_rizer_name = QStringLiteral("elite_rizer_name");
	static const QString default_elite_rizer_name = QStringLiteral("Disabled");
	/**
	 *@brief 
	*/
	static const QString elite_sterzo_smart_name = QStringLiteral("elite_sterzo_smart_name");
	static const QString default_elite_sterzo_smart_name = QStringLiteral("Disabled");
	/**
	 *@brief 
	*/
	static const QString ftms_accessory_name = QStringLiteral("ftms_accessory_name");
	static const QString default_ftms_accessory_name = QStringLiteral("Disabled");
    /**
	 *@brief 
	*/
	static const QString ss2k_shift_step = QStringLiteral("ss2k_shift_step");
	static constexpr float default_ss2k_shift_step = 900;
	/**
	 *@brief 
	*/
	static const QString fitmetria_fanfit_enable = QStringLiteral("fitmetria_fanfit_enable");
	static constexpr bool default_fitmetria_fanfit_enable = false;
	/**
	 *@brief 
	*/
	static const QString fitmetria_fanfit_mode = QStringLiteral("fitmetria_fanfit_mode");
	static const QString default_fitmetria_fanfit_mode = QStringLiteral("Heart");
    /**
	 *@brief 
	*/
	static const QString fitmetria_fanfit_min = QStringLiteral("fitmetria_fanfit_min");
	static constexpr float default_fitmetria_fanfit_min = 0;
    /**
	 *@brief 
	*/
	static const QString fitmetria_fanfit_max = QStringLiteral("fitmetria_fanfit_max");
	static constexpr float default_fitmetria_fanfit_max = 100;
	/**
	 *@brief 
	*/
	static const QString virtualbike_forceresistance = QStringLiteral("virtualbike_forceresistance");
	static constexpr bool default_virtualbike_forceresistance = true;
	/**
	 *@brief 
	*/
	static const QString bluetooth_relaxed = QStringLiteral("bluetooth_relaxed");
	static constexpr bool default_bluetooth_relaxed = false;
	/**
	 *@brief 
	*/
	static const QString bluetooth_30m_hangs = QStringLiteral("bluetooth_30m_hangs");
	static constexpr bool default_bluetooth_30m_hangs = false;
	/**
	 *@brief 
	*/
	static const QString battery_service = QStringLiteral("battery_service");
	static constexpr bool default_battery_service = false;
	/**
	 *@brief 
	*/
	static const QString service_changed = QStringLiteral("service_changed");
	static constexpr bool default_service_changed = false;
	/**
	 *@brief 
	*/
	static const QString virtual_device_enabled = QStringLiteral("virtual_device_enabled");
	static constexpr bool default_virtual_device_enabled = true;
	/**
	 *@brief 
	*/
	static const QString virtual_device_bluetooth = QStringLiteral("virtual_device_bluetooth");
	static constexpr bool default_virtual_device_bluetooth = true;
	/**
	 *@brief 
	*/
	static const QString ios_peloton_workaround = QStringLiteral("ios_peloton_workaround");
	static constexpr bool default_ios_peloton_workaround = true;
	/**
	 *@brief 
	*/
	static const QString android_wakelock = QStringLiteral("android_wakelock");
	static constexpr bool default_android_wakelock = true;
	/**
	 *@brief 
	*/
	static const QString log_debug = QStringLiteral("log_debug");
	static constexpr bool default_log_debug = false;
	/**
	 *@brief 
	*/
	static const QString virtual_device_onlyheart = QStringLiteral("virtual_device_onlyheart");
	static constexpr bool default_virtual_device_onlyheart = false;
	/**
	 *@brief 
	*/
	static const QString virtual_device_echelon = QStringLiteral("virtual_device_echelon");
	static constexpr bool default_virtual_device_echelon = false;
	/**
	 *@brief 
	*/
	static const QString virtual_device_ifit = QStringLiteral("virtual_device_ifit");
	static constexpr bool default_virtual_device_ifit = false;
	/**
	 *@brief 
	*/
	static const QString virtual_device_rower = QStringLiteral("virtual_device_rower");
	static constexpr bool default_virtual_device_rower = false;
	/**
	 *@brief 
	*/
	static const QString virtual_device_force_bike = QStringLiteral("virtual_device_force_bike");
	static constexpr bool default_virtual_device_force_bike = false;
	/**
	 *@brief 
	*/
	static const QString volume_change_gears = QStringLiteral("volume_change_gears");
	static constexpr bool default_volume_change_gears = false;
	/**
	 *@brief 
	*/
	static const QString applewatch_fakedevice = QStringLiteral("applewatch_fakedevice");
	static constexpr bool default_applewatch_fakedevice = false;
	// from version 2.10.15
    /**
	 *@brief 
	*/
	static const QString zwift_erg_resistance_down = QStringLiteral("zwift_erg_resistance_down");
	static constexpr float default_zwift_erg_resistance_down = 0.0;
    /**
	 *@brief 
	*/
	static const QString zwift_erg_resistance_up = QStringLiteral("zwift_erg_resistance_up");
	static constexpr float default_zwift_erg_resistance_up = 999.0;
	// from version 2.10.16
	/**
	 *@brief 
	*/
	static const QString horizon_paragon_x = QStringLiteral("horizon_paragon_x");
	static constexpr bool default_horizon_paragon_x = false;
	// from version 2.10.18
    /**
	 *@brief 
	*/
	static const QString treadmill_step_speed = QStringLiteral("treadmill_step_speed");
	static constexpr float default_treadmill_step_speed = 0.5;
    /**
	 *@brief 
	*/
	static const QString treadmill_step_incline = QStringLiteral("treadmill_step_incline");
	static constexpr float default_treadmill_step_incline = 0.5;
	// from version 2.10.19
	/**
	 *@brief 
	*/
	static const QString fitshow_anyrun = QStringLiteral("fitshow_anyrun");
	static constexpr bool default_fitshow_anyrun = false;
	// from version 2.10.21
	/**
	 *@brief 
	*/
	static const QString nordictrack_s30_treadmill = QStringLiteral("nordictrack_s30_treadmill");
	static constexpr bool default_nordictrack_s30_treadmill = false;
	// from version 2.10.23
	// not used anymore because it's an elliptical not a treadmill. Don't remove this
	// it will cause corruption in the settings
	/**
	 *@brief 
	*/
	static const QString nordictrack_fs5i_treadmill = QStringLiteral("nordictrack_fs5i_treadmill");
	static constexpr bool default_nordictrack_fs5i_treadmill = false;
	// from version 2.10.26
	/**
	 *@brief 
	*/
	static const QString renpho_peloton_conversion_v2 = QStringLiteral("renpho_peloton_conversion_v2");
	static constexpr bool default_renpho_peloton_conversion_v2 = false;
	// from version 2.10.27
    /**
	 *@brief 
	*/
	static const QString ss2k_resistance_sample_1 = QStringLiteral("ss2k_resistance_sample_1");
	static constexpr float default_ss2k_resistance_sample_1 = 20;
    /**
	 *@brief 
	*/
	static const QString ss2k_shift_step_sample_1 = QStringLiteral("ss2k_shift_step_sample_1");
	static constexpr float default_ss2k_shift_step_sample_1 = 0;
    /**
	 *@brief 
	*/
	static const QString ss2k_resistance_sample_2 = QStringLiteral("ss2k_resistance_sample_2");
	static constexpr float default_ss2k_resistance_sample_2 = 30;
    /**
	 *@brief 
	*/
	static const QString ss2k_shift_step_sample_2 = QStringLiteral("ss2k_shift_step_sample_2");
	static constexpr float default_ss2k_shift_step_sample_2 = 0;
    /**
	 *@brief 
	*/
	static const QString ss2k_resistance_sample_3 = QStringLiteral("ss2k_resistance_sample_3");
	static constexpr float default_ss2k_resistance_sample_3 = 40;
    /**
	 *@brief 
	*/
	static const QString ss2k_shift_step_sample_3 = QStringLiteral("ss2k_shift_step_sample_3");
	static constexpr float default_ss2k_shift_step_sample_3 = 0;
    /**
	 *@brief 
	*/
	static const QString ss2k_resistance_sample_4 = QStringLiteral("ss2k_resistance_sample_4");
	static constexpr float default_ss2k_resistance_sample_4 = 50;
    /**
	 *@brief 
	*/
	static const QString ss2k_shift_step_sample_4 = QStringLiteral("ss2k_shift_step_sample_4");
	static constexpr float default_ss2k_shift_step_sample_4 = 0;
	/**
	 *@brief 
	*/
	static const QString fitshow_truetimer = QStringLiteral("fitshow_truetimer");
	static constexpr bool default_fitshow_truetimer = false;
	// from version 2.10.28
    /**
	 *@brief 
	*/
	static const QString elite_rizer_gain = QStringLiteral("elite_rizer_gain");
	static constexpr float default_elite_rizer_gain = 1.0;
	/**
	 *@brief 
	*/
	static const QString tile_ext_incline_enabled = QStringLiteral("tile_ext_incline_enabled");
	static constexpr bool default_tile_ext_incline_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tile_ext_incline_order = QStringLiteral("tile_ext_incline_order");
	static constexpr int default_tile_ext_incline_order = 32;
	// from version 2.10.41
	/**
	 *@brief 
	*/
	static const QString reebok_fr30_treadmill = QStringLiteral("reebok_fr30_treadmill");
	static constexpr bool default_reebok_fr30_treadmill = false;
	// from version 2.10.44
	/**
	 *@brief 
	*/
	static const QString horizon_treadmill_7_8 = QStringLiteral("horizon_treadmill_7_8");
	static constexpr bool default_horizon_treadmill_7_8 = false;
	// from version 2.10.45
	/**
	 *@brief 
	*/
	static const QString profile_name = QStringLiteral("profile_name");
	static const QString default_profile_name = QStringLiteral("default");
	// from version 2.10.46
	/**
	 *@brief 
	*/
	static const QString tile_cadence_color_enabled = QStringLiteral("tile_cadence_color_enabled");
	static constexpr bool default_tile_cadence_color_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tile_peloton_remaining_enabled = QStringLiteral("tile_peloton_remaining_enabled");
	static constexpr bool default_tile_peloton_remaining_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tile_peloton_remaining_order = QStringLiteral("tile_peloton_remaining_order");
	static constexpr int default_tile_peloton_remaining_order = 22;
	/**
	 *@brief 
	*/
	static const QString tile_peloton_resistance_color_enabled = QStringLiteral("tile_peloton_resistance_color_enabled");
	static constexpr bool default_tile_peloton_resistance_color_enabled = false;
	// from version 2.10.49
	/**
	 *@brief 
	*/
	static const QString dircon_yes = QStringLiteral("dircon_yes");
	static constexpr bool default_dircon_yes = true;
	/**
	 *@brief 
	*/
	static const QString dircon_server_base_port = QStringLiteral("dircon_server_base_port");
	static constexpr int default_dircon_server_base_port = 36866;
	// from version 2.10.56
	/**
	 *@brief 
	*/
	static const QString ios_cache_heart_device = QStringLiteral("ios_cache_heart_device");
	static constexpr bool default_ios_cache_heart_device = true;
	// from version 2.10.57
	/**
	 *@brief 
	*/
	static const QString app_opening = QStringLiteral("app_opening");
	static constexpr int default_app_opening = 0;
	// from version 2.10.62
	/**
	 *@brief 
	*/
	static const QString proformtdf4ip = QStringLiteral("proformtdf4ip");
	static const QString default_proformtdf4ip = QStringLiteral("");
	// from version 2.10.72
	/**
	 *@brief 
	*/
	static const QString fitfiu_mc_v460 = QStringLiteral("fitfiu_mc_v460");
	static constexpr bool default_fitfiu_mc_v460 = false;
    /**
	 *@brief 
	*/
	static const QString bike_weight = QStringLiteral("bike_weight");
	static constexpr float default_bike_weight = 0;
	// from version 2.10.77
	/**
	 *@brief 
	*/
	static const QString kingsmith_encrypt_v2 = QStringLiteral("kingsmith_encrypt_v2");
	static constexpr bool default_kingsmith_encrypt_v2 = false;
	// from version 2.10.81
	/**
	 *@brief 
	*/
	static const QString proform_treadmill_9_0 = QStringLiteral("proform_treadmill_9_0");
	static constexpr bool default_proform_treadmill_9_0 = false;
	// from version 2.10.85
	/**
	 *@brief 
	*/
	static const QString proform_treadmill_1800i = QStringLiteral("proform_treadmill_1800i");
	static constexpr bool default_proform_treadmill_1800i = false;
	// from version 2.10.91
    /**
	 *@brief 
	*/
	static const QString cadence_offset = QStringLiteral("cadence_offset");
	static constexpr float default_cadence_offset = 0;
    /**
	 *@brief 
	*/
	static const QString cadence_gain = QStringLiteral("cadence_gain");
	static constexpr float default_cadence_gain = 1;
	/**
	 *@brief 
	*/
	static const QString sp_ht_9600ie = QStringLiteral("sp_ht_9600ie");
	static constexpr bool default_sp_ht_9600ie = false;
	// from version 2.10.92
	/**
	 *@brief 
	*/
	static const QString tts_enabled = QStringLiteral("tts_enabled");
	static constexpr bool default_tts_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tts_summary_sec = QStringLiteral("tts_summary_sec");
	static constexpr int default_tts_summary_sec = 120;
	/**
	 *@brief 
	*/
	static const QString tts_act_speed = QStringLiteral("tts_act_speed");
	static constexpr bool default_tts_act_speed = false;
	/**
	 *@brief 
	*/
	static const QString tts_avg_speed = QStringLiteral("tts_avg_speed");
	static constexpr bool default_tts_avg_speed = true;
	/**
	 *@brief 
	*/
	static const QString tts_max_speed = QStringLiteral("tts_max_speed");
	static constexpr bool default_tts_max_speed = false;
	/**
	 *@brief 
	*/
	static const QString tts_act_inclination = QStringLiteral("tts_act_inclination");
	static constexpr bool default_tts_act_inclination = false;
	/**
	 *@brief 
	*/
	static const QString tts_act_cadence = QStringLiteral("tts_act_cadence");
	static constexpr bool default_tts_act_cadence = false;
	/**
	 *@brief 
	*/
	static const QString tts_avg_cadence = QStringLiteral("tts_avg_cadence");
	static constexpr bool default_tts_avg_cadence = true;
	/**
	 *@brief 
	*/
	static const QString tts_max_cadence = QStringLiteral("tts_max_cadence");
	static constexpr bool default_tts_max_cadence = false;
	/**
	 *@brief 
	*/
	static const QString tts_act_elevation = QStringLiteral("tts_act_elevation");
	static constexpr bool default_tts_act_elevation = true;
	/**
	 *@brief 
	*/
	static const QString tts_act_calories = QStringLiteral("tts_act_calories");
	static constexpr bool default_tts_act_calories = true;
	/**
	 *@brief 
	*/
	static const QString tts_act_odometer = QStringLiteral("tts_act_odometer");
	static constexpr bool default_tts_act_odometer = true;
	/**
	 *@brief 
	*/
	static const QString tts_act_pace = QStringLiteral("tts_act_pace");
	static constexpr bool default_tts_act_pace = false;
	/**
	 *@brief 
	*/
	static const QString tts_avg_pace = QStringLiteral("tts_avg_pace");
	static constexpr bool default_tts_avg_pace = true;
	/**
	 *@brief 
	*/
	static const QString tts_max_pace = QStringLiteral("tts_max_pace");
	static constexpr bool default_tts_max_pace = false;
	/**
	 *@brief 
	*/
	static const QString tts_act_resistance = QStringLiteral("tts_act_resistance");
	static constexpr bool default_tts_act_resistance = true;
	/**
	 *@brief 
	*/
	static const QString tts_avg_resistance = QStringLiteral("tts_avg_resistance");
	static constexpr bool default_tts_avg_resistance = true;
	/**
	 *@brief 
	*/
	static const QString tts_max_resistance = QStringLiteral("tts_max_resistance");
	static constexpr bool default_tts_max_resistance = false;
	/**
	 *@brief 
	*/
	static const QString tts_act_watt = QStringLiteral("tts_act_watt");
	static constexpr bool default_tts_act_watt = false;
	/**
	 *@brief 
	*/
	static const QString tts_avg_watt = QStringLiteral("tts_avg_watt");
	static constexpr bool default_tts_avg_watt = true;
	/**
	 *@brief 
	*/
	static const QString tts_max_watt = QStringLiteral("tts_max_watt");
	static constexpr bool default_tts_max_watt = true;
	/**
	 *@brief 
	*/
	static const QString tts_act_ftp = QStringLiteral("tts_act_ftp");
	static constexpr bool default_tts_act_ftp = false;
	/**
	 *@brief 
	*/
	static const QString tts_avg_ftp = QStringLiteral("tts_avg_ftp");
	static constexpr bool default_tts_avg_ftp = true;
	/**
	 *@brief 
	*/
	static const QString tts_max_ftp = QStringLiteral("tts_max_ftp");
	static constexpr bool default_tts_max_ftp = false;
	/**
	 *@brief 
	*/
	static const QString tts_act_heart = QStringLiteral("tts_act_heart");
	static constexpr bool default_tts_act_heart = true;
	/**
	 *@brief 
	*/
	static const QString tts_avg_heart = QStringLiteral("tts_avg_heart");
	static constexpr bool default_tts_avg_heart = true;
	/**
	 *@brief 
	*/
	static const QString tts_max_heart = QStringLiteral("tts_max_heart");
	static constexpr bool default_tts_max_heart = false;
	/**
	 *@brief 
	*/
	static const QString tts_act_jouls = QStringLiteral("tts_act_jouls");
	static constexpr bool default_tts_act_jouls = true;
	/**
	 *@brief 
	*/
	static const QString tts_act_elapsed = QStringLiteral("tts_act_elapsed");
	static constexpr bool default_tts_act_elapsed = true;
	/**
	 *@brief 
	*/
	static const QString tts_act_peloton_resistance = QStringLiteral("tts_act_peloton_resistance");
	static constexpr bool default_tts_act_peloton_resistance = false;
	/**
	 *@brief 
	*/
	static const QString tts_avg_peloton_resistance = QStringLiteral("tts_avg_peloton_resistance");
	static constexpr bool default_tts_avg_peloton_resistance = false;
	/**
	 *@brief 
	*/
	static const QString tts_max_peloton_resistance = QStringLiteral("tts_max_peloton_resistance");
	static constexpr bool default_tts_max_peloton_resistance = false;
	/**
	 *@brief 
	*/
	static const QString tts_act_target_peloton_resistance = QStringLiteral("tts_act_target_peloton_resistance");
	static constexpr bool default_tts_act_target_peloton_resistance = true;
	/**
	 *@brief 
	*/
	static const QString tts_act_target_cadence = QStringLiteral("tts_act_target_cadence");
	static constexpr bool default_tts_act_target_cadence = true;
	/**
	 *@brief 
	*/
	static const QString tts_act_target_power = QStringLiteral("tts_act_target_power");
	static constexpr bool default_tts_act_target_power = true;
	/**
	 *@brief 
	*/
	static const QString tts_act_target_zone = QStringLiteral("tts_act_target_zone");
	static constexpr bool default_tts_act_target_zone = true;
	/**
	 *@brief 
	*/
	static const QString tts_act_target_speed = QStringLiteral("tts_act_target_speed");
	static constexpr bool default_tts_act_target_speed = true;
	/**
	 *@brief 
	*/
	static const QString tts_act_target_incline = QStringLiteral("tts_act_target_incline");
	static constexpr bool default_tts_act_target_incline = true;
	/**
	 *@brief 
	*/
	static const QString tts_act_watt_kg = QStringLiteral("tts_act_watt_kg");
	static constexpr bool default_tts_act_watt_kg = false;
	/**
	 *@brief 
	*/
	static const QString tts_avg_watt_kg = QStringLiteral("tts_avg_watt_kg");
	static constexpr bool default_tts_avg_watt_kg = false;
	/**
	 *@brief 
	*/
	static const QString tts_max_watt_kg = QStringLiteral("tts_max_watt_kg");
	static constexpr bool default_tts_max_watt_kg = false;
	// from version 2.10.96
	/**
	 *@brief 
	*/
	static const QString fakedevice_elliptical = QStringLiteral("fakedevice_elliptical");
	static constexpr bool default_fakedevice_elliptical = false;
	// from version 2.10.99
	/**
	 *@brief 
	*/
	static const QString nordictrack_2950_ip = QStringLiteral("nordictrack_2950_ip");
	static const QString default_nordictrack_2950_ip = QStringLiteral("");
	// from version 2.10.102
	/**
	 *@brief 
	*/
	static const QString tile_instantaneous_stride_length_enabled = QStringLiteral("tile_instantaneous_stride_length_enabled");
	static constexpr bool default_tile_instantaneous_stride_length_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tile_instantaneous_stride_length_order = QStringLiteral("tile_instantaneous_stride_length_order");
	static constexpr int default_tile_instantaneous_stride_length_order = 32;
	/**
	 *@brief 
	*/
	static const QString tile_ground_contact_enabled = QStringLiteral("tile_ground_contact_enabled");
	static constexpr bool default_tile_ground_contact_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tile_ground_contact_order = QStringLiteral("tile_ground_contact_order");
	static constexpr int default_tile_ground_contact_order = 33;
	/**
	 *@brief 
	*/
	static const QString tile_vertical_oscillation_enabled = QStringLiteral("tile_vertical_oscillation_enabled");
	static constexpr bool default_tile_vertical_oscillation_enabled = false;
	/**
	 *@brief 
	*/
	static const QString tile_vertical_oscillation_order = QStringLiteral("tile_vertical_oscillation_order");
	static constexpr int default_tile_vertical_oscillation_order = 34;
	/**
	 *@brief 
	*/
	static const QString sex = QStringLiteral("sex");
	static const QString default_sex = QStringLiteral("Male");
	// from version 2.10.111
	/**
	 *@brief 
	*/
	static const QString maps_type = QStringLiteral("maps_type");
	static const QString default_maps_type = QStringLiteral("3D");
	// from version 2.10.112
    /**
	 *@brief 
	*/
	static const QString ss2k_max_resistance = QStringLiteral("ss2k_max_resistance");
	static constexpr float default_ss2k_max_resistance = 100;
    /**
	 *@brief 
	*/
	static const QString ss2k_min_resistance = QStringLiteral("ss2k_min_resistance");
	static constexpr float default_ss2k_min_resistance = 0;
	// from version 2.11.10
	/**
	 *@brief 
	*/
	static const QString proform_treadmill_se = QStringLiteral("proform_treadmill_se");
	static constexpr bool default_proform_treadmill_se = false;
	// from version 2.11.14
	/**
	 *@brief 
	*/
	static const QString proformtreadmillip = QStringLiteral("proformtreadmillip");
	static const QString default_proformtreadmillip = QStringLiteral("");
	// from version 2.11.22
	/**
	 *@brief 
	*/
	static const QString kingsmith_encrypt_v3 = QStringLiteral("kingsmith_encrypt_v3");
	static constexpr bool default_kingsmith_encrypt_v3 = false;
	// from version 2.11.38
	/**
	 *@brief 
	*/
	static const QString tdf_10_ip = QStringLiteral("tdf_10_ip");
	static const QString default_tdf_10_ip = QStringLiteral("");
	// from version 2.11.41
	/**
	 *@brief 
	*/
	static const QString fakedevice_treadmill = QStringLiteral("fakedevice_treadmill");
	static constexpr bool default_fakedevice_treadmill = false;
	// from version 2.11.43
	/**
	 *@brief 
	*/
	static const QString video_playback_window_s = QStringLiteral("video_playback_window_s");
	static constexpr int default_video_playback_window_s = 12;

    /**
     * @brief Write the QSettings values using the constants from this namespace.
     */
    static void qDebugAllSettings();
}

#endif
