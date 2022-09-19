include(../defaults.pri)
QT += bluetooth widgets xml positioning quick networkauth websockets texttospeech location multimedia
QTPLUGIN += qavfmediaplayer
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

CONFIG += qmltypes
QML_IMPORT_NAME = org.cagnulein.qdomyoszwift
QML_IMPORT_MAJOR_VERSION = 1
# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

win32:QMAKE_LFLAGS_DEBUG += -static-libstdc++ -static-libgcc
QMAKE_LFLAGS_RELEASE += -s
QMAKE_CXXFLAGS += -fno-sized-deallocation
unix:android: {
    CONFIG -= optimize_size
    QMAKE_CFLAGS_OPTIMIZE_FULL -= -Oz
    QMAKE_CFLAGS_OPTIMIZE_FULL += -O3
}
macx: CONFIG += debug
win32: CONFIG += debug
macx: CONFIG += static
macx {
    QMAKE_INFO_PLIST = macx/Info.plist
}
INCLUDEPATH += qmdnsengine/src/include

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
# include(../qtzeroconf/qtzeroconf.pri)

SOURCES += \
   PathController.cpp \
    characteristicnotifier2a53.cpp \
    characteristicnotifier2a5b.cpp \
    characteristicnotifier2acc.cpp \
    characteristicnotifier2acd.cpp \
    characteristicnotifier2ad9.cpp \
    fakeelliptical.cpp \
   faketreadmill.cpp \
   kmlworkout.cpp \
   nautilusbike.cpp \
    nordictrackelliptical.cpp \
    nordictrackifitadbbike.cpp \
   nordictrackifitadbtreadmill.cpp \
   octanetreadmill.cpp \
   proformellipticaltrainer.cpp \
   proformrower.cpp \
   proformwifibike.cpp \
   proformwifitreadmill.cpp \
    qmdnsengine/src/src/abstractserver.cpp \
    qmdnsengine/src/src/bitmap.cpp \
    qmdnsengine/src/src/browser.cpp \
    qmdnsengine/src/src/cache.cpp \
    qmdnsengine/src/src/dns.cpp \
    qmdnsengine/src/src/hostname.cpp \
    qmdnsengine/src/src/mdns.cpp \
    qmdnsengine/src/src/message.cpp \
    qmdnsengine/src/src/prober.cpp \
    qmdnsengine/src/src/provider.cpp \
    qmdnsengine/src/src/query.cpp \
    qmdnsengine/src/src/record.cpp \
    qmdnsengine/src/src/resolver.cpp \
    qmdnsengine/src/src/server.cpp \
    qmdnsengine/src/src/service.cpp \
    activiotreadmill.cpp \
   bhfitnesselliptical.cpp \
   bike.cpp \
	     bluetooth.cpp \
		bluetoothdevice.cpp \
    characteristicnotifier2a37.cpp \
    characteristicnotifier2a63.cpp \
    characteristicnotifier2ad2.cpp \
    characteristicwriteprocessor2ad9.cpp \
   bowflext216treadmill.cpp \
    bowflextreadmill.cpp \
   chronobike.cpp \
    concept2skierg.cpp \
   cscbike.cpp \
    dirconmanager.cpp \
    dirconpacket.cpp \
    dirconprocessor.cpp \
	 domyoselliptical.cpp \
   domyosrower.cpp \
	     domyostreadmill.cpp \
		echelonconnectsport.cpp \
   echelonrower.cpp \
   echelonstride.cpp \
   eliterizer.cpp \
   elitesterzosmart.cpp \
	 elliptical.cpp \
	eslinkertreadmill.cpp \
    fakebike.cpp \
   filedownloader.cpp \
    fitmetria_fanfit.cpp \
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
    horizongr7bike.cpp \
   horizontreadmill.cpp \
   iconceptbike.cpp \
	inspirebike.cpp \
	keepawakehelper.cpp \
   keepbike.cpp \
   kingsmithr1protreadmill.cpp \
   kingsmithr2treadmill.cpp \
	     main.cpp \
   mcfbike.cpp \
		metric.cpp \
   nautiluselliptical.cpp \
    nautilustreadmill.cpp \
    npecablebike.cpp \
   pafersbike.cpp \
   paferstreadmill.cpp \
   peloton.cpp \
   powerzonepack.cpp \
	proformbike.cpp \
   proformelliptical.cpp \
	proformtreadmill.cpp \
	qfit.cpp \
   renphobike.cpp \
   rower.cpp \
	schwinnic4bike.cpp \
   screencapture.cpp \
	sessionline.cpp \
   shuaa5treadmill.cpp \
	signalhandler.cpp \
   simplecrypt.cpp \
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
   solebike.cpp \
   soleelliptical.cpp \
   solef80treadmill.cpp \
   spirittreadmill.cpp \
   sportsplusbike.cpp \
   sportstechbike.cpp \
   strydrunpowersensor.cpp \
   tacxneo2.cpp \
    tcpclientinfosender.cpp \
   technogymmyruntreadmill.cpp \
    technogymmyruntreadmillrfcomm.cpp \
    templateinfosender.cpp \
    templateinfosenderbuilder.cpp \
   stagesbike.cpp \
	     toorxtreadmill.cpp \
		  treadmill.cpp \
   truetreadmill.cpp \
   trxappgateusbbike.cpp \
   ultrasportbike.cpp \
   virtualrower.cpp \
   wahookickrsnapbike.cpp \
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
   PathController.h \
    characteristicnotifier2a53.h \
    characteristicnotifier2a5b.h \
    characteristicnotifier2acc.h \
    characteristicnotifier2acd.h \
    characteristicnotifier2ad9.h \
    definitions.h \
    fakeelliptical.h \
   faketreadmill.h \
   kmlworkout.h \
   nautilusbike.h \
    nordictrackelliptical.h \
    nordictrackifitadbbike.h \
   nordictrackifitadbtreadmill.h \
   octanetreadmill.h \
   proformellipticaltrainer.h \
   proformrower.h \
   proformwifibike.h \
   proformwifitreadmill.h \
    qmdnsengine/src/include/qmdnsengine/abstractserver.h \
    qmdnsengine/src/include/qmdnsengine/bitmap.h \
    qmdnsengine/src/include/qmdnsengine/browser.h \
    qmdnsengine/src/include/qmdnsengine/cache.h \
    qmdnsengine/src/include/qmdnsengine/dns.h \
    qmdnsengine/src/include/qmdnsengine/hostname.h \
    qmdnsengine/src/include/qmdnsengine/mdns.h \
    qmdnsengine/src/include/qmdnsengine/message.h \
    qmdnsengine/src/include/qmdnsengine/prober.h \
    qmdnsengine/src/include/qmdnsengine/provider.h \
    qmdnsengine/src/include/qmdnsengine/query.h \
    qmdnsengine/src/include/qmdnsengine/record.h \
    qmdnsengine/src/include/qmdnsengine/resolver.h \
    qmdnsengine/src/include/qmdnsengine/server.h \
    qmdnsengine/src/include/qmdnsengine/service.h \
    qmdnsengine/src/src/bitmap_p.h \
    qmdnsengine/src/src/browser_p.h \
    qmdnsengine/src/src/cache_p.h \
    qmdnsengine/src/src/hostname_p.h \
    qmdnsengine/src/src/message_p.h \
    qmdnsengine/src/src/prober_p.h \
    qmdnsengine/src/src/provider_p.h \
    qmdnsengine/src/src/query_p.h \
    qmdnsengine/src/src/record_p.h \
    qmdnsengine/src/src/resolver_p.h \
    qmdnsengine/src/src/server_p.h \
    qmdnsengine/src/src/service_p.h \
    activiotreadmill.h \
   bhfitnesselliptical.h \
   bike.h \
	bluetooth.h \
	bluetoothdevice.h \
    characteristicnotifier.h \
    characteristicnotifier2a37.h \
    characteristicnotifier2a63.h \
    characteristicnotifier2ad2.h \
    characteristicwriteprocessor.h \
    characteristicwriteprocessor2ad9.h \
   bowflext216treadmill.h \
    bowflextreadmill.h \
   chronobike.h \
    concept2skierg.h \
   cscbike.h \
    dirconmanager.h \
    dirconpacket.h \
    dirconprocessor.h \
	 domyoselliptical.h \
   domyosrower.h \
	domyostreadmill.h \
	echelonconnectsport.h \
   echelonrower.h \
   echelonstride.h \
   eliterizer.h \
   elitesterzosmart.h \
	 elliptical.h \
   eslinkertreadmill.h \
    fakebike.h \
   filedownloader.h \
    fitmetria_fanfit.h \
   fitplusbike.h \
    ftmsrower.h \
   homefitnessbuddy.h \
    horizongr7bike.h \
   iconceptbike.h \
   keepbike.h \
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
   mcfbike.h \
	metric.h \
   nautiluselliptical.h \
    nautilustreadmill.h \
    npecablebike.h \
   pafersbike.h \
   paferstreadmill.h \
   peloton.h \
   powerzonepack.h \
	proformbike.h \
   proformelliptical.h \
	proformtreadmill.h \
    qdebugfixup.h \
	qfit.h \
    qmdnsengine_export.h \
   renphobike.h \
   rower.h \
	schwinnic4bike.h \
   screencapture.h \
	sessionline.h \
   shuaa5treadmill.h \
	signalhandler.h \
   simplecrypt.h \
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
   solebike.h \
   soleelliptical.h \
   solef80treadmill.h \
   spirittreadmill.h \
   sportsplusbike.h \
   sportstechbike.h \
   strydrunpowersensor.h \
   tacxneo2.h \
    tcpclientinfosender.h \
   technogymmyruntreadmill.h \
    technogymmyruntreadmillrfcomm.h \
    templateinfosender.h \
    templateinfosenderbuilder.h \
   stagesbike.h \
	toorxtreadmill.h \
	gpx.h \
	treadmill.h \
	mainwindow.h \
	trainprogram.h \
   truetreadmill.h \
   trxappgateusbbike.h \
	trxappgateusbtreadmill.h \
   ultrasportbike.h \
	 virtualbike.h \
   virtualrower.h \
	virtualtreadmill.h \
	 domyosbike.h \
   wahookickrsnapbike.h \
   wobjectdefs.h \
   wobjectimpl.h \
        yesoulbike.h \
        scanrecordresult.h \
   zwiftworkout.h

exists(secret.h): HEADERS += secret.h

!ios: HEADERS += charts.h

!ios: FORMS += \
   charts.ui \
	mainwindow.ui

RESOURCES += \
   icons.qrc \
	qml.qrc

DISTFILES += \
    .clang-format \
   AppxManifest.xml \
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
	android/src/org/qtproject/qt/android/purchasing/Security.java \
	android/src/org/qtproject/qt/android/purchasing/InAppPurchase.java \
	android/src/org/qtproject/qt/android/purchasing/Base64.java \
	android/src/org/qtproject/qt/android/purchasing/Base64DecoderException.java \
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
    DEFINES+=_Nullable_result=_Nullable NS_FORMAT_ARGUMENT\\(A\\)=
}

include($$PWD/purchasing/purchasing.pri)
INCLUDEPATH += purchasing/qmltypes
INCLUDEPATH += purchasing/inapp

WINRT_MANIFEST = AppxManifest.xml

VERSION = 2.11.51