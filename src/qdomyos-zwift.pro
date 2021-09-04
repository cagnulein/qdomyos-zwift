QT += bluetooth widgets xml positioning quick networkauth websockets

QT+= charts

unix:android: QT += androidextras gui-private
qtHaveModule(httpserver) {
    QT += httpserver
    DEFINES += Q_HTTPSERVER
    SOURCES += webserverinfosender.cpp
    HEADERS += webserverinfosender.h

    # android and iOS are using ChartJS
    unix:android: {
        QT+= webview
        DEFINES += CHARTJS
    }
    ios: {
        QT+= webview
        DEFINES += CHARTJS
    }
}

CONFIG += c++17 console app_bundle optimize_full ltcg
QMAKE_LFLAGS_RELEASE += -s
QMAKE_CXXFLAGS += -fno-sized-deallocation
unix:android: {
    CONFIG -= optimize_size
    QMAKE_CFLAGS_OPTIMIZE_FULL -= -Oz
    QMAKE_CFLAGS_OPTIMIZE_FULL += -O3
}
macx: CONFIG += static

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS IO_UNDER_QT SMTP_BUILD

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

android: include(../android_openssl/openssl.pri)

SOURCES += \
   bike.cpp \
	     bluetooth.cpp \
		bluetoothdevice.cpp \
   chronobike.cpp \
   cscbike.cpp \
	 domyoselliptical.cpp \
	     domyostreadmill.cpp \
		echelonconnectsport.cpp \
   echelonrower.cpp \
   echelonstride.cpp \
   eliterizer.cpp \
	 elliptical.cpp \
	eslinkertreadmill.cpp \
   fitplusbike.cpp \
	fitshowtreadmill.cpp \
	fit-sdk/fit.cpp \
	fit-sdk/fit_accumulated_field.cpp \
	fit-sdk/fit_accumulator.cpp \
	fit-sdk/fit_buffer_encode.cpp \
	fit-sdk/fit_buffered_mesg_broadcaster.cpp \
	fit-sdk/fit_buffered_record_mesg_broadcaster.cpp \
	fit-sdk/fit_crc.cpp \
	fit-sdk/fit_date_time.cpp \
	fit-sdk/fit_decode.cpp \
	fit-sdk/fit_developer_field.cpp \
	fit-sdk/fit_developer_field_definition.cpp \
	fit-sdk/fit_developer_field_description.cpp \
	fit-sdk/fit_encode.cpp \
	fit-sdk/fit_factory.cpp \
	fit-sdk/fit_field.cpp \
	fit-sdk/fit_field_base.cpp \
	fit-sdk/fit_field_definition.cpp \
	fit-sdk/fit_mesg.cpp \
	fit-sdk/fit_mesg_broadcaster.cpp \
	fit-sdk/fit_mesg_definition.cpp \
	fit-sdk/fit_mesg_with_event_broadcaster.cpp \
	fit-sdk/fit_profile.cpp \
	fit-sdk/fit_protocol_validator.cpp \
	fit-sdk/fit_unicode.cpp \
	flywheelbike.cpp \
	ftmsbike.cpp \
    ftmsrower.cpp \
	     gpx.cpp \
		heartratebelt.cpp \
   homefitnessbuddy.cpp \
	homeform.cpp \
   horizontreadmill.cpp \
	inspirebike.cpp \
	keepawakehelper.cpp \
   kingsmithr1protreadmill.cpp \
   kingsmithr2treadmill.cpp \
	     main.cpp \
		metric.cpp \
    npecablebike.cpp \
   peloton.cpp \
   powerzonepack.cpp \
	proformbike.cpp \
	proformtreadmill.cpp \
	qfit.cpp \
   renphobike.cpp \
   rower.cpp \
	schwinnic4bike.cpp \
   screencapture.cpp \
	sessionline.cpp \
	signalhandler.cpp \
    skandikawiribike.cpp \
   smartrowrower.cpp \
   smartspin2k.cpp \
    smtpclient/src/emailaddress.cpp \
    smtpclient/src/mimeattachment.cpp \
    smtpclient/src/mimecontentformatter.cpp \
    smtpclient/src/mimefile.cpp \
    smtpclient/src/mimehtml.cpp \
    smtpclient/src/mimeinlinefile.cpp \
    smtpclient/src/mimemessage.cpp \
    smtpclient/src/mimemultipart.cpp \
    smtpclient/src/mimepart.cpp \
    smtpclient/src/mimetext.cpp \
    smtpclient/src/quotedprintable.cpp \
    smtpclient/src/smtpclient.cpp \
   snodebike.cpp \
   soleelliptical.cpp \
    solef80.cpp \
   spirittreadmill.cpp \
   sportstechbike.cpp \
   tacxneo2.cpp \
    tcpclientinfosender.cpp \
    templateinfosender.cpp \
    templateinfosenderbuilder.cpp \
   stagesbike.cpp \
	     toorxtreadmill.cpp \
		  treadmill.cpp \
   trxappgateusbbike.cpp \
		yesoulbike.cpp \
		  trainprogram.cpp \
		trxappgateusbtreadmill.cpp \
	 virtualbike.cpp \
	     virtualtreadmill.cpp \
             m3ibike.cpp \
                domyosbike.cpp \
               scanrecordresult.cpp \
   zwiftworkout.cpp
macx: SOURCES += macos/lockscreen.mm
!ios: SOURCES += mainwindow.cpp charts.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += fit-sdk/

HEADERS += \
   bike.h \
	bluetooth.h \
	bluetoothdevice.h \
   chronobike.h \
   cscbike.h \
	 domyoselliptical.h \
	domyostreadmill.h \
	echelonconnectsport.h \
   echelonrower.h \
   echelonstride.h \
   eliterizer.h \
	 elliptical.h \
   eslinkertreadmill.h \
   fitplusbike.h \
    ftmsrower.h \
   homefitnessbuddy.h \
   kingsmithr1protreadmill.h \
   kingsmithr2treadmill.h \
   m3ibike.h \
        fitshowtreadmill.h \
	fit-sdk/FitDecode.h \
	fit-sdk/FitDeveloperField.h \
	fit-sdk/FitEncode.h \
	fit-sdk/FitField.h \
	fit-sdk/FitFieldDefinition.h \
	fit-sdk/FitMesg.h \
	fit-sdk/FitMesgDefinition.h \
	fit-sdk/fit.hpp \
	fit-sdk/fit_accelerometer_data_mesg.hpp \
	fit-sdk/fit_accelerometer_data_mesg_listener.hpp \
	fit-sdk/fit_accumulated_field.hpp \
	fit-sdk/fit_accumulator.hpp \
	fit-sdk/fit_activity_mesg.hpp \
	fit-sdk/fit_activity_mesg_listener.hpp \
	fit-sdk/fit_ant_channel_id_mesg.hpp \
	fit-sdk/fit_ant_channel_id_mesg_listener.hpp \
	fit-sdk/fit_ant_rx_mesg.hpp \
	fit-sdk/fit_ant_rx_mesg_listener.hpp \
	fit-sdk/fit_ant_tx_mesg.hpp \
	fit-sdk/fit_ant_tx_mesg_listener.hpp \
	fit-sdk/fit_aviation_attitude_mesg.hpp \
	fit-sdk/fit_aviation_attitude_mesg_listener.hpp \
	fit-sdk/fit_barometer_data_mesg.hpp \
	fit-sdk/fit_barometer_data_mesg_listener.hpp \
	fit-sdk/fit_bike_profile_mesg.hpp \
	fit-sdk/fit_bike_profile_mesg_listener.hpp \
	fit-sdk/fit_blood_pressure_mesg.hpp \
	fit-sdk/fit_blood_pressure_mesg_listener.hpp \
	fit-sdk/fit_buffer_encode.hpp \
	fit-sdk/fit_buffered_mesg_broadcaster.hpp \
	fit-sdk/fit_buffered_record_mesg.hpp \
	fit-sdk/fit_buffered_record_mesg_broadcaster.hpp \
	fit-sdk/fit_buffered_record_mesg_listener.hpp \
	fit-sdk/fit_cadence_zone_mesg.hpp \
	fit-sdk/fit_cadence_zone_mesg_listener.hpp \
	fit-sdk/fit_camera_event_mesg.hpp \
	fit-sdk/fit_camera_event_mesg_listener.hpp \
	fit-sdk/fit_capabilities_mesg.hpp \
	fit-sdk/fit_capabilities_mesg_listener.hpp \
	fit-sdk/fit_climb_pro_mesg.hpp \
	fit-sdk/fit_climb_pro_mesg_listener.hpp \
	fit-sdk/fit_config.hpp \
	fit-sdk/fit_connectivity_mesg.hpp \
	fit-sdk/fit_connectivity_mesg_listener.hpp \
	fit-sdk/fit_course_mesg.hpp \
	fit-sdk/fit_course_mesg_listener.hpp \
	fit-sdk/fit_course_point_mesg.hpp \
	fit-sdk/fit_course_point_mesg_listener.hpp \
	fit-sdk/fit_crc.hpp \
	fit-sdk/fit_date_time.hpp \
	fit-sdk/fit_decode.hpp \
	fit-sdk/fit_developer_data_id_mesg.hpp \
	fit-sdk/fit_developer_data_id_mesg_listener.hpp \
	fit-sdk/fit_developer_field.hpp \
	fit-sdk/fit_developer_field_definition.hpp \
	fit-sdk/fit_developer_field_description.hpp \
	fit-sdk/fit_developer_field_description_listener.hpp \
	fit-sdk/fit_device_info_mesg.hpp \
	fit-sdk/fit_device_info_mesg_listener.hpp \
	fit-sdk/fit_device_settings_mesg.hpp \
	fit-sdk/fit_device_settings_mesg_listener.hpp \
	fit-sdk/fit_dive_alarm_mesg.hpp \
	fit-sdk/fit_dive_alarm_mesg_listener.hpp \
	fit-sdk/fit_dive_gas_mesg.hpp \
	fit-sdk/fit_dive_gas_mesg_listener.hpp \
	fit-sdk/fit_dive_settings_mesg.hpp \
	fit-sdk/fit_dive_settings_mesg_listener.hpp \
	fit-sdk/fit_dive_summary_mesg.hpp \
	fit-sdk/fit_dive_summary_mesg_listener.hpp \
	fit-sdk/fit_encode.hpp \
	fit-sdk/fit_event_mesg.hpp \
	fit-sdk/fit_event_mesg_listener.hpp \
	fit-sdk/fit_exd_data_concept_configuration_mesg.hpp \
	fit-sdk/fit_exd_data_concept_configuration_mesg_listener.hpp \
	fit-sdk/fit_exd_data_field_configuration_mesg.hpp \
	fit-sdk/fit_exd_data_field_configuration_mesg_listener.hpp \
	fit-sdk/fit_exd_screen_configuration_mesg.hpp \
	fit-sdk/fit_exd_screen_configuration_mesg_listener.hpp \
	fit-sdk/fit_exercise_title_mesg.hpp \
	fit-sdk/fit_exercise_title_mesg_listener.hpp \
	fit-sdk/fit_factory.hpp \
	fit-sdk/fit_field.hpp \
	fit-sdk/fit_field_base.hpp \
	fit-sdk/fit_field_capabilities_mesg.hpp \
	fit-sdk/fit_field_capabilities_mesg_listener.hpp \
	fit-sdk/fit_field_definition.hpp \
	fit-sdk/fit_field_description_mesg.hpp \
	fit-sdk/fit_field_description_mesg_listener.hpp \
	fit-sdk/fit_file_capabilities_mesg.hpp \
	fit-sdk/fit_file_capabilities_mesg_listener.hpp \
	fit-sdk/fit_file_creator_mesg.hpp \
	fit-sdk/fit_file_creator_mesg_listener.hpp \
	fit-sdk/fit_file_id_mesg.hpp \
	fit-sdk/fit_file_id_mesg_listener.hpp \
	fit-sdk/fit_goal_mesg.hpp \
	fit-sdk/fit_goal_mesg_listener.hpp \
	fit-sdk/fit_gps_metadata_mesg.hpp \
	fit-sdk/fit_gps_metadata_mesg_listener.hpp \
	fit-sdk/fit_gyroscope_data_mesg.hpp \
	fit-sdk/fit_gyroscope_data_mesg_listener.hpp \
	fit-sdk/fit_hr_mesg.hpp \
	fit-sdk/fit_hr_mesg_listener.hpp \
	fit-sdk/fit_hr_zone_mesg.hpp \
	fit-sdk/fit_hr_zone_mesg_listener.hpp \
	fit-sdk/fit_hrm_profile_mesg.hpp \
	fit-sdk/fit_hrm_profile_mesg_listener.hpp \
	fit-sdk/fit_hrv_mesg.hpp \
	fit-sdk/fit_hrv_mesg_listener.hpp \
	fit-sdk/fit_jump_mesg.hpp \
	fit-sdk/fit_jump_mesg_listener.hpp \
	fit-sdk/fit_lap_mesg.hpp \
	fit-sdk/fit_lap_mesg_listener.hpp \
	fit-sdk/fit_length_mesg.hpp \
	fit-sdk/fit_length_mesg_listener.hpp \
	fit-sdk/fit_magnetometer_data_mesg.hpp \
	fit-sdk/fit_magnetometer_data_mesg_listener.hpp \
	fit-sdk/fit_memo_glob_mesg.hpp \
	fit-sdk/fit_memo_glob_mesg_listener.hpp \
	fit-sdk/fit_mesg.hpp \
	fit-sdk/fit_mesg_broadcast_plugin.hpp \
	fit-sdk/fit_mesg_broadcaster.hpp \
	fit-sdk/fit_mesg_capabilities_mesg.hpp \
	fit-sdk/fit_mesg_capabilities_mesg_listener.hpp \
	fit-sdk/fit_mesg_definition.hpp \
	fit-sdk/fit_mesg_definition_listener.hpp \
	fit-sdk/fit_mesg_listener.hpp \
	fit-sdk/fit_mesg_with_event.hpp \
	fit-sdk/fit_mesg_with_event_broadcaster.hpp \
	fit-sdk/fit_mesg_with_event_listener.hpp \
	fit-sdk/fit_met_zone_mesg.hpp \
	fit-sdk/fit_met_zone_mesg_listener.hpp \
	fit-sdk/fit_monitoring_info_mesg.hpp \
	fit-sdk/fit_monitoring_info_mesg_listener.hpp \
	fit-sdk/fit_monitoring_mesg.hpp \
	fit-sdk/fit_monitoring_mesg_listener.hpp \
	fit-sdk/fit_nmea_sentence_mesg.hpp \
	fit-sdk/fit_nmea_sentence_mesg_listener.hpp \
	fit-sdk/fit_obdii_data_mesg.hpp \
	fit-sdk/fit_obdii_data_mesg_listener.hpp \
	fit-sdk/fit_ohr_settings_mesg.hpp \
	fit-sdk/fit_ohr_settings_mesg_listener.hpp \
	fit-sdk/fit_one_d_sensor_calibration_mesg.hpp \
	fit-sdk/fit_one_d_sensor_calibration_mesg_listener.hpp \
	fit-sdk/fit_pad_mesg.hpp \
	fit-sdk/fit_pad_mesg_listener.hpp \
	fit-sdk/fit_power_zone_mesg.hpp \
	fit-sdk/fit_power_zone_mesg_listener.hpp \
	fit-sdk/fit_profile.hpp \
	fit-sdk/fit_protocol_validator.hpp \
	fit-sdk/fit_record_mesg.hpp \
	fit-sdk/fit_record_mesg_listener.hpp \
	fit-sdk/fit_runtime_exception.hpp \
	fit-sdk/fit_schedule_mesg.hpp \
	fit-sdk/fit_schedule_mesg_listener.hpp \
	fit-sdk/fit_sdm_profile_mesg.hpp \
	fit-sdk/fit_sdm_profile_mesg_listener.hpp \
	fit-sdk/fit_segment_file_mesg.hpp \
	fit-sdk/fit_segment_file_mesg_listener.hpp \
	fit-sdk/fit_segment_id_mesg.hpp \
	fit-sdk/fit_segment_id_mesg_listener.hpp \
	fit-sdk/fit_segment_lap_mesg.hpp \
	fit-sdk/fit_segment_lap_mesg_listener.hpp \
	fit-sdk/fit_segment_leaderboard_entry_mesg.hpp \
	fit-sdk/fit_segment_leaderboard_entry_mesg_listener.hpp \
	fit-sdk/fit_segment_point_mesg.hpp \
	fit-sdk/fit_segment_point_mesg_listener.hpp \
	fit-sdk/fit_session_mesg.hpp \
	fit-sdk/fit_session_mesg_listener.hpp \
	fit-sdk/fit_set_mesg.hpp \
	fit-sdk/fit_set_mesg_listener.hpp \
	fit-sdk/fit_slave_device_mesg.hpp \
	fit-sdk/fit_slave_device_mesg_listener.hpp \
	fit-sdk/fit_software_mesg.hpp \
	fit-sdk/fit_software_mesg_listener.hpp \
	fit-sdk/fit_speed_zone_mesg.hpp \
	fit-sdk/fit_speed_zone_mesg_listener.hpp \
	fit-sdk/fit_sport_mesg.hpp \
	fit-sdk/fit_sport_mesg_listener.hpp \
	fit-sdk/fit_stress_level_mesg.hpp \
	fit-sdk/fit_stress_level_mesg_listener.hpp \
	fit-sdk/fit_three_d_sensor_calibration_mesg.hpp \
	fit-sdk/fit_three_d_sensor_calibration_mesg_listener.hpp \
	fit-sdk/fit_timestamp_correlation_mesg.hpp \
	fit-sdk/fit_timestamp_correlation_mesg_listener.hpp \
	fit-sdk/fit_totals_mesg.hpp \
	fit-sdk/fit_totals_mesg_listener.hpp \
	fit-sdk/fit_training_file_mesg.hpp \
	fit-sdk/fit_training_file_mesg_listener.hpp \
	fit-sdk/fit_unicode.hpp \
	fit-sdk/fit_user_profile_mesg.hpp \
	fit-sdk/fit_user_profile_mesg_listener.hpp \
	fit-sdk/fit_video_clip_mesg.hpp \
	fit-sdk/fit_video_clip_mesg_listener.hpp \
	fit-sdk/fit_video_description_mesg.hpp \
	fit-sdk/fit_video_description_mesg_listener.hpp \
	fit-sdk/fit_video_frame_mesg.hpp \
	fit-sdk/fit_video_frame_mesg_listener.hpp \
	fit-sdk/fit_video_mesg.hpp \
	fit-sdk/fit_video_mesg_listener.hpp \
	fit-sdk/fit_video_title_mesg.hpp \
	fit-sdk/fit_video_title_mesg_listener.hpp \
	fit-sdk/fit_watchface_settings_mesg.hpp \
	fit-sdk/fit_watchface_settings_mesg_listener.hpp \
	fit-sdk/fit_weather_alert_mesg.hpp \
	fit-sdk/fit_weather_alert_mesg_listener.hpp \
	fit-sdk/fit_weather_conditions_mesg.hpp \
	fit-sdk/fit_weather_conditions_mesg_listener.hpp \
	fit-sdk/fit_weight_scale_mesg.hpp \
	fit-sdk/fit_weight_scale_mesg_listener.hpp \
	fit-sdk/fit_workout_mesg.hpp \
	fit-sdk/fit_workout_mesg_listener.hpp \
	fit-sdk/fit_workout_session_mesg.hpp \
	fit-sdk/fit_workout_session_mesg_listener.hpp \
	fit-sdk/fit_workout_step_mesg.hpp \
	fit-sdk/fit_workout_step_mesg_listener.hpp \
	fit-sdk/fit_zones_target_mesg.hpp \
	fit-sdk/fit_zones_target_mesg_listener.hpp \
	flywheelbike.h \
	ftmsbike.h \
	 heartratebelt.h \
	homeform.h \
   horizontreadmill.h \
	inspirebike.h \
	ios/lockscreen.h \
	keepawakehelper.h \
	macos/lockscreen.h \
        ios/M3iIOS-Interface.h \
	material.h \
	metric.h \
    npecablebike.h \
   peloton.h \
   powerzonepack.h \
	proformbike.h \
	proformtreadmill.h \
    qdebugfixup.h \
	qfit.h \
   renphobike.h \
   rower.h \
	schwinnic4bike.h \
   screencapture.h \
	sessionline.h \
	signalhandler.h \
    skandikawiribike.h \
   smartrowrower.h \
   smartspin2k.h \
    smtpclient/src/SmtpMime \
    smtpclient/src/emailaddress.h \
    smtpclient/src/mimeattachment.h \
    smtpclient/src/mimecontentformatter.h \
    smtpclient/src/mimefile.h \
    smtpclient/src/mimehtml.h \
    smtpclient/src/mimeinlinefile.h \
    smtpclient/src/mimemessage.h \
    smtpclient/src/mimemultipart.h \
    smtpclient/src/mimepart.h \
    smtpclient/src/mimetext.h \
    smtpclient/src/quotedprintable.h \
    smtpclient/src/smtpclient.h \
    smtpclient/src/smtpexports.h \
   snodebike.h \
   soleelliptical.h \
    solef80.h \
   spirittreadmill.h \
   sportstechbike.h \
   tacxneo2.h \
    tcpclientinfosender.h \
    templateinfosender.h \
    templateinfosenderbuilder.h \
   stagesbike.h \
	toorxtreadmill.h \
	gpx.h \
	treadmill.h \
	mainwindow.h \
	trainprogram.h \
   trxappgateusbbike.h \
	trxappgateusbtreadmill.h \
	 virtualbike.h \
	virtualtreadmill.h \
	 domyosbike.h \
        yesoulbike.h \
        scanrecordresult.h \
   zwiftworkout.h

!ios: HEADERS += charts.h

!ios: FORMS += \
   charts.ui \
	mainwindow.ui

RESOURCES += \
   icons.qrc \
	qml.qrc

DISTFILES += \
    .clang-format \
   android/AndroidManifest.xml \
	android/build.gradle \
	android/gradle/wrapper/gradle-wrapper.jar \
	android/gradle/wrapper/gradle-wrapper.properties \
	android/gradlew \
	android/gradlew.bat \
	android/libs/android_antlib_4-14-0.jar \
	android/res/values/libs.xml \
	android/src/Ant.java \
	android/src/ChannelService.java \
        android/src/ScanRecordResult.java \
        android/src/NativeScanCallback.java \
   android/src/HeartChannelController.java \
	android/src/MyActivity.java \
	android/src/PowerChannelController.java \
	android/src/SpeedChannelController.java \
	android/src/com/dsi/ant/channel/PredefinedNetwork.java \
	ios/AppDelegate.swift \
	ios/BLEPeripheralManager.swift

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

ANDROID_ABIS = armeabi-v7a arm64-v8a x86 x86_64
#ANDROID_ABIS = arm64-v8a

ios {
    ios_icon.files = $$files($$PWD/icons/ios/*.png)
	 QMAKE_BUNDLE_DATA += ios_icon
}

ios {
    OBJECTIVE_SOURCES += ios/lockscreen.mm \
	 fit-sdk/FitDecode.mm \
	 fit-sdk/FitDeveloperField.mm \
	 fit-sdk/FitEncode.mm \
	 fit-sdk/FitField.mm \
	 fit-sdk/FitFieldDefinition.mm \
	 fit-sdk/FitMesg.mm \
         fit-sdk/FitMesgDefinition.mm \
         ios/M3iNS.mm \

    SOURCES += ios/M3iNSQT.cpp

    OBJECTIVE_HEADERS += ios/M3iNS.h

    QMAKE_INFO_PLIST = ios/Info.plist
	 QMAKE_ASSET_CATALOGS = $$PWD/ios/Images.xcassets
	 QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"

    TARGET = qdomyoszwift
	 QMAKE_TARGET_BUNDLE_PREFIX = org.cagnulein
}

VERSION = 2.8.53

