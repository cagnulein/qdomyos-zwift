cmake_minimum_required(VERSION 3.23)

# Add purchasing module
add_subdirectory(purchasing)

# Set target name
set(TARGET_NAME qdomyos-zwift)

# Core source files
set(CORE_SOURCES
    # Characteristics
    characteristics/characteristicnotifier.cpp
    characteristics/characteristicnotifier0002.cpp
    characteristics/characteristicnotifier0004.cpp
    characteristics/characteristicwriteprocessor0003.cpp
    characteristics/characteristicnotifier2a53.cpp
    characteristics/characteristicnotifier2a5b.cpp
    characteristics/characteristicnotifier2acc.cpp
    characteristics/characteristicnotifier2acd.cpp
    characteristics/characteristicnotifier2ad9.cpp
    characteristics/characteristicwriteprocessor.cpp
    characteristics/characteristicwriteprocessore005.cpp
    characteristics/characteristicnotifier2a37.cpp
    characteristics/characteristicnotifier2a63.cpp
    characteristics/characteristicnotifier2ad2.cpp
    characteristics/characteristicwriteprocessor2ad9.cpp
    
    # Core files
    androidqlog.cpp
    logwriter.cpp
    osc.cpp
    QTelnet.cpp
    PathController.cpp
    filedownloader.cpp
    androidactivityresultreceiver.cpp
    androidadblog.cpp
    handleurl.cpp
    localipaddress.cpp
    homefitnessbuddy.cpp
    homeform.cpp
    keepawakehelper.cpp
    main.cpp
    metric.cpp
    peloton.cpp
    powerzonepack.cpp
    qfit.cpp
    qzsettings.cpp
    screencapture.cpp
    sessionline.cpp
    signalhandler.cpp
    simplecrypt.cpp
    tcpclientinfosender.cpp
    templateinfosender.cpp
    templateinfosenderbuilder.cpp
    gpx.cpp
    trainprogram.cpp
    scanrecordresult.cpp
    zwiftworkout.cpp
    mqttpublisher.cpp
    androidstatusbar.cpp
    inclinationresistancetable.cpp
    treadmillErgTable.cpp
    ergtable.cpp
    fitdatabaseprocessor.cpp
    workoutmodel.cpp
    fontmanager.cpp
    workoutloaderworker.cpp
    fitbackupwriter.cpp
)

# Device sources
set(DEVICE_SOURCES
    devices/android_antbike/android_antbike.cpp
    devices/antbike/antbike.cpp
    devices/coresensor/coresensor.cpp
    devices/crossrope/crossrope.cpp
    devices/cycleopsphantombike/cycleopsphantombike.cpp
    devices/deeruntreadmill/deerruntreadmill.cpp
    devices/elitesquarecontroller/elitesquarecontroller.cpp
    devices/focustreadmill/focustreadmill.cpp
    devices/jumprope.cpp
    devices/kineticinroadbike/SmartControl.cpp
    devices/kineticinroadbike/kineticinroadbike.cpp
    devices/lifespantreadmill/lifespantreadmill.cpp
    devices/moxy5sensor/moxy5sensor.cpp
    devices/nordictrackifitadbelliptical/nordictrackifitadbelliptical.cpp
    devices/nordictrackifitadbrower/nordictrackifitadbrower.cpp
    devices/pitpatbike/pitpatbike.cpp
    devices/speraxtreadmill/speraxtreadmill.cpp
    devices/sportsplusrower/sportsplusrower.cpp
    devices/sportstechelliptical/sportstechelliptical.cpp
    devices/sramAXSController/sramAXSController.cpp
    devices/stairclimber.cpp
    devices/echelonstairclimber/echelonstairclimber.cpp
    devices/technogymbike/technogymbike.cpp
    devices/trxappgateusbelliptical/trxappgateusbelliptical.cpp
    devices/trxappgateusbrower/trxappgateusbrower.cpp
    devices/bkoolbike/bkoolbike.cpp
    devices/apexbike/apexbike.cpp
    devices/iconceptelliptical/iconceptelliptical.cpp
    devices/pelotonbike/pelotonbike.cpp
    devices/schwinn170bike/schwinn170bike.cpp
    devices/wahookickrheadwind/wahookickrheadwind.cpp
    devices/ypooelliptical/ypooelliptical.cpp
    devices/ziprotreadmill/ziprotreadmill.cpp
    devices/computrainerbike/Computrainer.cpp
    devices/computrainerbike/computrainerbike.cpp
    devices/fakeelliptical/fakeelliptical.cpp
    devices/faketreadmill/faketreadmill.cpp
    devices/lifefitnesstreadmill/lifefitnesstreadmill.cpp
    devices/mepanelbike/mepanelbike.cpp
    devices/nautilusbike/nautilusbike.cpp
    devices/nordictrackelliptical/nordictrackelliptical.cpp
    devices/nordictrackifitadbbike/nordictrackifitadbbike.cpp
    devices/nordictrackifitadbtreadmill/nordictrackifitadbtreadmill.cpp
    devices/octaneelliptical/octaneelliptical.cpp
    devices/octanetreadmill/octanetreadmill.cpp
    devices/proformellipticaltrainer/proformellipticaltrainer.cpp
    devices/proformrower/proformrower.cpp
    devices/proformwifibike/proformwifibike.cpp
    devices/proformwifitreadmill/proformwifitreadmill.cpp
    devices/activiotreadmill/activiotreadmill.cpp
    devices/bhfitnesselliptical/bhfitnesselliptical.cpp
    devices/bike.cpp
    devices/bluetooth.cpp
    devices/bluetoothdevice.cpp
    devices/bowflext216treadmill/bowflext216treadmill.cpp
    devices/bowflextreadmill/bowflextreadmill.cpp
    devices/chronobike/chronobike.cpp
    devices/concept2skierg/concept2skierg.cpp
    devices/cscbike/cscbike.cpp
    devices/dircon/dirconmanager.cpp
    devices/dircon/dirconpacket.cpp
    devices/dircon/dirconprocessor.cpp
    devices/domyoselliptical/domyoselliptical.cpp
    devices/domyosrower/domyosrower.cpp
    devices/domyostreadmill/domyostreadmill.cpp
    devices/echelonconnectsport/echelonconnectsport.cpp
    devices/echelonrower/echelonrower.cpp
    devices/echelonstride/echelonstride.cpp
    devices/eliterizer/eliterizer.cpp
    devices/elitesterzosmart/elitesterzosmart.cpp
    devices/elliptical.cpp
    devices/eslinkertreadmill/eslinkertreadmill.cpp
    devices/fakebike/fakebike.cpp
    devices/fitmetria_fanfit/fitmetria_fanfit.cpp
    devices/fitplusbike/fitplusbike.cpp
    devices/fitshowtreadmill/fitshowtreadmill.cpp
    devices/flywheelbike/flywheelbike.cpp
    devices/ftmsbike/ftmsbike.cpp
    devices/ftmsrower/ftmsrower.cpp
    devices/heartratebelt/heartratebelt.cpp
    devices/horizongr7bike/horizongr7bike.cpp
    devices/horizontreadmill/horizontreadmill.cpp
    devices/iconceptbike/iconceptbike.cpp
    devices/inspirebike/inspirebike.cpp
    devices/keepbike/keepbike.cpp
    devices/kingsmithr1protreadmill/kingsmithr1protreadmill.cpp
    devices/kingsmithr2treadmill/kingsmithr2treadmill.cpp
    devices/mcfbike/mcfbike.cpp
    devices/nautiluselliptical/nautiluselliptical.cpp
    devices/nautilustreadmill/nautilustreadmill.cpp
    devices/npecablebike/npecablebike.cpp
    devices/pafersbike/pafersbike.cpp
    devices/paferstreadmill/paferstreadmill.cpp
    devices/proformbike/proformbike.cpp
    devices/proformelliptical/proformelliptical.cpp
    devices/proformtreadmill/proformtreadmill.cpp
    devices/renphobike/renphobike.cpp
    devices/rower.cpp
    devices/schwinnic4bike/schwinnic4bike.cpp
    devices/shuaa5treadmill/shuaa5treadmill.cpp
    devices/skandikawiribike/skandikawiribike.cpp
    devices/smartrowrower/smartrowrower.cpp
    devices/smartspin2k/smartspin2k.cpp
    devices/snodebike/snodebike.cpp
    devices/solebike/solebike.cpp
    devices/soleelliptical/soleelliptical.cpp
    devices/solef80treadmill/solef80treadmill.cpp
    devices/spirittreadmill/spirittreadmill.cpp
    devices/sportsplusbike/sportsplusbike.cpp
    devices/sportstechbike/sportstechbike.cpp
    devices/strydrunpowersensor/strydrunpowersensor.cpp
    devices/tacxneo2/tacxneo2.cpp
    devices/technogymmyruntreadmill/technogymmyruntreadmill.cpp
    devices/technogymmyruntreadmillrfcomm/technogymmyruntreadmillrfcomm.cpp
    devices/stagesbike/stagesbike.cpp
    devices/toorxtreadmill/toorxtreadmill.cpp
    devices/treadmill.cpp
    devices/truetreadmill/truetreadmill.cpp
    devices/trxappgateusbbike/trxappgateusbbike.cpp
    devices/ultrasportbike/ultrasportbike.cpp
    devices/wahookickrsnapbike/wahookickrsnapbike.cpp
    devices/yesoulbike/yesoulbike.cpp
    devices/trxappgateusbtreadmill/trxappgateusbtreadmill.cpp
    devices/m3ibike/m3ibike.cpp
    devices/domyosbike/domyosbike.cpp
)

# CSAFE sources
set(CSAFE_SOURCES
    devices/csafe/csafe.cpp
    devices/csafe/csaferunner.cpp
    devices/csafe/csafeutility.cpp
    devices/csafe/serialhandler.cpp
    devices/csafe/serialport.cpp
    devices/csafe/netserial.cpp
    devices/csafe/kalmanfilter.cpp
    devices/csafeelliptical/csafeelliptical.cpp
    devices/csaferower/csaferower.cpp
)

# Fan/Controller sources
set(FAN_CONTROLLER_SOURCES
    devices/eliteariafan/eliteariafan.cpp
    devices/fakerower/fakerower.cpp
    devices/proformtelnetbike/proformtelnetbike.cpp
)

# Virtual device sources
set(VIRTUAL_DEVICE_SOURCES
    virtualdevices/virtualdevice.cpp
    virtualdevices/virtualbike.cpp
    virtualdevices/virtualrower.cpp
    virtualdevices/virtualtreadmill.cpp
)

# Zwift Play sources
set(ZWIFT_PLAY_SOURCES
    zwift_play/zwiftclickremote.cpp
    zwift_play/zwiftPlayDevice.cpp
    zwift_play/abstractZapDevice.cpp
)

# MQTT sources
set(MQTT_SOURCES
    mqtt/qmqttauthenticationproperties.cpp
    mqtt/qmqttclient.cpp
    mqtt/qmqttconnection.cpp
    mqtt/qmqttconnectionproperties.cpp
    mqtt/qmqttcontrolpacket.cpp
    mqtt/qmqttmessage.cpp
    mqtt/qmqttpublishproperties.cpp
    mqtt/qmqttsubscription.cpp
    mqtt/qmqttsubscriptionproperties.cpp
    mqtt/qmqtttopicfilter.cpp
    mqtt/qmqtttopicname.cpp
    mqtt/qmqtttype.cpp
)

# QmDNSEngine sources
set(QMDNSENGINE_SOURCES
    qmdnsengine/src/src/abstractserver.cpp
    qmdnsengine/src/src/bitmap.cpp
    qmdnsengine/src/src/browser.cpp
    qmdnsengine/src/src/cache.cpp
    qmdnsengine/src/src/dns.cpp
    qmdnsengine/src/src/hostname.cpp
    qmdnsengine/src/src/mdns.cpp
    qmdnsengine/src/src/message.cpp
    qmdnsengine/src/src/prober.cpp
    qmdnsengine/src/src/provider.cpp
    qmdnsengine/src/src/query.cpp
    qmdnsengine/src/src/record.cpp
    qmdnsengine/src/src/resolver.cpp
    qmdnsengine/src/src/server.cpp
    qmdnsengine/src/src/service.cpp
    # Headers with Q_OBJECT for AUTOMOC
    qmdnsengine/src/include/qmdnsengine/abstractserver.h
    qmdnsengine/src/include/qmdnsengine/browser.h
    qmdnsengine/src/include/qmdnsengine/cache.h
    qmdnsengine/src/include/qmdnsengine/hostname.h
    qmdnsengine/src/include/qmdnsengine/prober.h
    qmdnsengine/src/include/qmdnsengine/provider.h
    qmdnsengine/src/include/qmdnsengine/resolver.h
    qmdnsengine/src/include/qmdnsengine/server.h
)

# FIT SDK sources
set(FIT_SDK_SOURCES
    fit-sdk/fit.cpp
    fit-sdk/fit_accumulated_field.cpp
    fit-sdk/fit_accumulator.cpp
    fit-sdk/fit_buffer_encode.cpp
    fit-sdk/fit_buffered_mesg_broadcaster.cpp
    fit-sdk/fit_buffered_record_mesg_broadcaster.cpp
    fit-sdk/fit_crc.cpp
    fit-sdk/fit_date_time.cpp
    fit-sdk/fit_decode.cpp
    fit-sdk/fit_developer_field.cpp
    fit-sdk/fit_developer_field_definition.cpp
    fit-sdk/fit_developer_field_description.cpp
    fit-sdk/fit_encode.cpp
    fit-sdk/fit_factory.cpp
    fit-sdk/fit_field.cpp
    fit-sdk/fit_field_base.cpp
    fit-sdk/fit_field_definition.cpp
    fit-sdk/fit_mesg.cpp
    fit-sdk/fit_mesg_broadcaster.cpp
    fit-sdk/fit_mesg_definition.cpp
    fit-sdk/fit_mesg_with_event_broadcaster.cpp
    fit-sdk/fit_profile.cpp
    fit-sdk/fit_protocol_validator.cpp
    fit-sdk/fit_unicode.cpp
)

# SMTP Client sources
set(SMTP_CLIENT_SOURCES
    smtpclient/src/emailaddress.cpp
    smtpclient/src/mimeattachment.cpp
    smtpclient/src/mimecontentformatter.cpp
    smtpclient/src/mimefile.cpp
    smtpclient/src/mimehtml.cpp
    smtpclient/src/mimeinlinefile.cpp
    smtpclient/src/mimemessage.cpp
    smtpclient/src/mimemultipart.cpp
    smtpclient/src/mimepart.cpp
    smtpclient/src/mimetext.cpp
    smtpclient/src/quotedprintable.cpp
    smtpclient/src/smtpclient.cpp
    smtpclient/src/mimebase64encoder.cpp
    smtpclient/src/mimebase64formatter.cpp
    smtpclient/src/mimebytearrayattachment.cpp
    smtpclient/src/mimecontentencoder.cpp
    smtpclient/src/mimeqpencoder.cpp
    smtpclient/src/mimeqpformatter.cpp
)

# Resource files
set(RESOURCE_SOURCES
    icons.qrc
    qml.qrc
)

# Combine all sources
set(ALL_SOURCES
    ${CORE_SOURCES}
    ${DEVICE_SOURCES}
    ${CSAFE_SOURCES}
    ${FAN_CONTROLLER_SOURCES}
    ${VIRTUAL_DEVICE_SOURCES}
    ${ZWIFT_PLAY_SOURCES}
    ${MQTT_SOURCES}
    ${QMDNSENGINE_SOURCES}
    ${FIT_SDK_SOURCES}
    ${SMTP_CLIENT_SOURCES}
    ${RESOURCE_SOURCES}
)

# Platform-specific sources
if(WIN32)
    list(APPEND ALL_SOURCES
        windows_zwift_workout_paddleocr_thread.cpp
        windows_zwift_incline_paddleocr_thread.cpp
    )
endif()

if(APPLE)
    if(IOS)
        # iOS-specific sources will be added here
    else()
        # macOS-specific sources
        list(APPEND ALL_SOURCES macos/lockscreen.mm)
    endif()
endif()

# Desktop-only sources (not iOS)
if(NOT IOS)
    list(APPEND ALL_SOURCES
        mainwindow.cpp
        charts.cpp
    )
endif()

# MSVC-specific sources
if(MSVC)
    list(APPEND ALL_SOURCES zwift-api/zwift_messages.pb.cc)
endif()

# Conditional sources
qt6_standard_project_setup()

# Check for httpserver module
if(TARGET Qt6::HttpServer)
    list(APPEND ALL_SOURCES webserverinfosender.cpp)
    set(HAVE_HTTPSERVER TRUE)
endif()

# Create the executable
qt6_add_executable(${TARGET_NAME} ${ALL_SOURCES})

# Set target properties
set_target_properties(${TARGET_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

# Link Qt6 libraries
if(ANDROID)
    # Android specific libraries
    target_link_libraries(${TARGET_NAME} PRIVATE
        Qt6::Core
        Qt6::Quick
        Qt6::Xml
        Qt6::Positioning
        Qt6::NetworkAuth
        Qt6::WebSockets
        Qt6::Location
        Qt6::Multimedia
        Qt6::Network
        Qt6::Gui
        Qt6::Charts
        Qt6::Sql
    )
    # Link optional Android modules
    if(TARGET Qt6::Bluetooth)
        target_link_libraries(${TARGET_NAME} PRIVATE Qt6::Bluetooth)
        target_compile_definitions(${TARGET_NAME} PRIVATE HAVE_BLUETOOTH)
    endif()
    if(TARGET Qt6::Core5Compat)
        target_link_libraries(${TARGET_NAME} PRIVATE Qt6::Core5Compat)
        target_compile_definitions(${TARGET_NAME} PRIVATE HAVE_CORE5COMPAT)
    endif()
    if(TARGET Qt6::TextToSpeech)
        target_link_libraries(${TARGET_NAME} PRIVATE Qt6::TextToSpeech)
        target_compile_definitions(${TARGET_NAME} PRIVATE HAVE_TEXTTOSPEECH)
    endif()
else()
    # Desktop/full libraries
    target_link_libraries(${TARGET_NAME} PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Bluetooth
        Qt6::Xml
        Qt6::Positioning
        Qt6::Quick
        Qt6::NetworkAuth
        Qt6::WebSockets
        Qt6::TextToSpeech
        Qt6::Location
        Qt6::Multimedia
        Qt6::MultimediaWidgets
        Qt6::Network
        Qt6::Gui
        Qt6::WebView
        Qt6::Charts
        Qt6::Core5Compat
    )
    target_compile_definitions(${TARGET_NAME} PRIVATE HAVE_BLUETOOTH HAVE_CORE5COMPAT)
endif()

# Link purchasing library
target_link_libraries(${TARGET_NAME} PRIVATE purchasing)

# Link httpserver if available
if(HAVE_HTTPSERVER)
    target_link_libraries(${TARGET_NAME} PRIVATE Qt6::HttpServer)
    target_compile_definitions(${TARGET_NAME} PRIVATE HAVE_HTTPSERVER)
endif()

# Include directories
target_include_directories(${TARGET_NAME} PRIVATE
    .
    fit-sdk/
    devices/
    qmdnsengine/src/include
    purchasing/qmltypes
    purchasing/inapp
)

# Resources are handled automatically by AUTORCC
# The .qrc files are already included in the source list

# Platform-specific settings
if(ANDROID)
    # Android-specific settings
    set_target_properties(${TARGET_NAME} PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
        QT_ANDROID_MIN_SDK_VERSION 23
        QT_ANDROID_TARGET_SDK_VERSION 36
    )
    
    # Enable multi-ABI
    set_target_properties(${TARGET_NAME} PROPERTIES
        QT_ANDROID_BUILD_ALL_ABIS TRUE
        QT_ANDROID_ABIS "arm64-v8a"
    )
endif()

if(WIN32)
    # Windows-specific libraries
    target_link_libraries(${TARGET_NAME} PRIVATE
        dbghelp
        user32
    )
endif()

# Compiler-specific settings
if(MINGW)
    target_compile_options(${TARGET_NAME} PRIVATE -Wa,-mbig-obj)
endif()

target_compile_options(${TARGET_NAME} PRIVATE -fno-sized-deallocation)

# Definitions
target_compile_definitions(${TARGET_NAME} PRIVATE
    QT_DEPRECATED_WARNINGS
    IO_UNDER_QT
    SMTP_BUILD
    NOMINMAX
    QML_IMPORT_NAME=org.cagnulein.qdomyoszwift
    QML_IMPORT_MAJOR_VERSION=1
)

# Release optimizations
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${TARGET_NAME} PRIVATE -s)
endif()
