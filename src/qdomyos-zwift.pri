include(../defaults.pri)
QT += bluetooth widgets xml positioning quick networkauth websockets texttospeech location multimedia
QTPLUGIN += qavfmediaplayer
QT+= charts core-private sql concurrent

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
#	 win32: {
#	     DEFINES += CHARTJS
#		}
}

CONFIG += c++17 console app_bundle optimize_full ltcg

CONFIG += qmltypes

#win32: CONFIG += webengine
#unix:!android: CONFIG += webengine

win32:DEFINES += _ITERATOR_DEBUG_LEVEL=0
win32:!mingw:LIBS += -llibprotobuf -llibprotoc -labseil_dll -llibprotobuf-lite -ldbghelp -L$$PWD 

QML_IMPORT_NAME = org.cagnulein.qdomyoszwift
QML_IMPORT_MAJOR_VERSION = 1
# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

win32:QMAKE_LFLAGS_DEBUG += -static-libstdc++ -static-libgcc -llibcrypto-1_1-x64 -llibssl-1_1-x64 -L$$PWD/../windows_openssl
win32:QMAKE_LFLAGS_RELEASE += -static-libstdc++ -static-libgcc -llibcrypto-1_1-x64 -llibssl-1_1-x64 -L$$PWD/../windows_openssl

QMAKE_LFLAGS_RELEASE += -s
QMAKE_CXXFLAGS += -fno-sized-deallocation
mingw: QMAKE_CXXFLAGS += -Wa,-mbig-obj
msvc {
   win32:QMAKE_CXXFLAGS_DEBUG += /RTC1
}
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
DEFINES += QT_DEPRECATED_WARNINGS IO_UNDER_QT SMTP_BUILD NOMINMAX


# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


# include(../qtzeroconf/qtzeroconf.pri)

SOURCES += \
    $$PWD/characteristics/characteristicnotifier0002.cpp \
    $$PWD/characteristics/characteristicnotifier0004.cpp \
    $$PWD/characteristics/characteristicwriteprocessor0003.cpp \
    $$PWD/devices/android_antbike/android_antbike.cpp \
    $$PWD/androidqlog.cpp \
    $$PWD/devices/antbike/antbike.cpp \
    $$PWD/devices/coresensor/coresensor.cpp \
    $$PWD/devices/crossrope/crossrope.cpp \
    $$PWD/devices/cycleopsphantombike/cycleopsphantombike.cpp \
    $$PWD/devices/deeruntreadmill/deerruntreadmill.cpp \
    $$PWD/devices/elitesquarecontroller/elitesquarecontroller.cpp \
    $$PWD/devices/focustreadmill/focustreadmill.cpp \
    $$PWD/devices/jumprope.cpp \
    $$PWD/devices/kineticinroadbike/SmartControl.cpp \
    $$PWD/devices/kineticinroadbike/kineticinroadbike.cpp \
    $$PWD/devices/lifespantreadmill/lifespantreadmill.cpp \
    $$PWD/devices/moxy5sensor/moxy5sensor.cpp \
    $$PWD/devices/nordictrackifitadbelliptical/nordictrackifitadbelliptical.cpp \
    $$PWD/devices/nordictrackifitadbrower/nordictrackifitadbrower.cpp \
    $$PWD/devices/pitpatbike/pitpatbike.cpp \
    $$PWD/devices/speraxtreadmill/speraxtreadmill.cpp \
    $$PWD/devices/sportsplusrower/sportsplusrower.cpp \
    $$PWD/devices/sportstechelliptical/sportstechelliptical.cpp \
    $$PWD/devices/sramAXSController/sramAXSController.cpp \
    $$PWD/devices/stairclimber.cpp \
    $$PWD/devices/echelonstairclimber/echelonstairclimber.cpp \
    $$PWD/devices/technogymbike/technogymbike.cpp \
    $$PWD/devices/trxappgateusbelliptical/trxappgateusbelliptical.cpp \
    $$PWD/fitdatabaseprocessor.cpp \
    $$PWD/devices/trxappgateusbrower/trxappgateusbrower.cpp \
    $$PWD/logwriter.cpp \
    $$PWD/fitbackupwriter.cpp \
    $$PWD/mqtt/qmqttauthenticationproperties.cpp \
    $$PWD/mqtt/qmqttclient.cpp \
    $$PWD/mqtt/qmqttconnection.cpp \
    $$PWD/mqtt/qmqttconnectionproperties.cpp \
    $$PWD/mqtt/qmqttcontrolpacket.cpp \
    $$PWD/mqtt/qmqttmessage.cpp \
    $$PWD/mqtt/qmqttpublishproperties.cpp \
    $$PWD/mqtt/qmqttsubscription.cpp \
    $$PWD/mqtt/qmqttsubscriptionproperties.cpp \
    $$PWD/mqtt/qmqtttopicfilter.cpp \
    $$PWD/mqtt/qmqtttopicname.cpp \
    $$PWD/mqtt/qmqtttype.cpp \
    $$PWD/osc.cpp \
    $$PWD/workoutloaderworker.cpp \
    $$PWD/workoutmodel.cpp \
QTelnet.cpp \
devices/bkoolbike/bkoolbike.cpp \
devices/csafe/csafe.cpp \
devices/csafe/csaferunner.cpp \
devices/csafe/csafeutility.cpp \
devices/csafe/serialhandler.cpp \
devices/csafe/serialport.cpp \
devices/csafe/netserial.cpp \
devices/csafe/kalmanfilter.cpp \
devices/csafeelliptical/csafeelliptical.cpp \
devices/csaferower/csaferower.cpp \
devices/eliteariafan/eliteariafan.cpp \
devices/fakerower/fakerower.cpp \
devices/proformtelnetbike/proformtelnetbike.cpp \
virtualdevices/virtualdevice.cpp \
androidactivityresultreceiver.cpp \
androidadblog.cpp \
devices/apexbike/apexbike.cpp \
handleurl.cpp \
devices/iconceptelliptical/iconceptelliptical.cpp \
localipaddress.cpp \
devices/pelotonbike/pelotonbike.cpp \
devices/schwinn170bike/schwinn170bike.cpp \
devices/wahookickrheadwind/wahookickrheadwind.cpp \
windows_zwift_workout_paddleocr_thread.cpp \
devices/ypooelliptical/ypooelliptical.cpp \
devices/ziprotreadmill/ziprotreadmill.cpp \
zwift_play/zwiftclickremote.cpp \
devices/computrainerbike/Computrainer.cpp \
devices/kettlerusbbike/KettlerUSB.cpp \
PathController.cpp \
characteristics/characteristicnotifier2a53.cpp \
characteristics/characteristicnotifier2a5b.cpp \
characteristics/characteristicnotifier2acc.cpp \
characteristics/characteristicnotifier2acd.cpp \
characteristics/characteristicnotifier2ad9.cpp \
characteristics/characteristicwriteprocessor.cpp \
characteristics/characteristicwriteprocessore005.cpp \
devices/computrainerbike/computrainerbike.cpp \
devices/kettlerusbbike/kettlerusbbike.cpp \
devices/fakeelliptical/fakeelliptical.cpp \
devices/faketreadmill/faketreadmill.cpp \
devices/lifefitnesstreadmill/lifefitnesstreadmill.cpp \
devices/mepanelbike/mepanelbike.cpp \
devices/nautilusbike/nautilusbike.cpp \
devices/nordictrackelliptical/nordictrackelliptical.cpp \
devices/nordictrackifitadbbike/nordictrackifitadbbike.cpp \
devices/nordictrackifitadbtreadmill/nordictrackifitadbtreadmill.cpp \
devices/octaneelliptical/octaneelliptical.cpp \
devices/octanetreadmill/octanetreadmill.cpp \
devices/proformellipticaltrainer/proformellipticaltrainer.cpp \
devices/proformrower/proformrower.cpp \
devices/proformwifibike/proformwifibike.cpp \
devices/proformwifitreadmill/proformwifitreadmill.cpp \
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
devices/activiotreadmill/activiotreadmill.cpp \
devices/bhfitnesselliptical/bhfitnesselliptical.cpp \
devices/bike.cpp \
devices/bluetooth.cpp \
devices/bluetoothdevice.cpp \
characteristics/characteristicnotifier2a37.cpp \
characteristics/characteristicnotifier2a63.cpp \
characteristics/characteristicnotifier2ad2.cpp \
characteristics/characteristicwriteprocessor2ad9.cpp \
devices/bowflext216treadmill/bowflext216treadmill.cpp \
devices/bowflextreadmill/bowflextreadmill.cpp \
devices/chronobike/chronobike.cpp \
devices/concept2skierg/concept2skierg.cpp \
devices/cscbike/cscbike.cpp \
devices/dircon/dirconmanager.cpp \
devices/dircon/dirconpacket.cpp \
devices/dircon/dirconprocessor.cpp \
devices/domyoselliptical/domyoselliptical.cpp \
devices/domyosrower/domyosrower.cpp \
devices/domyostreadmill/domyostreadmill.cpp \
devices/echelonconnectsport/echelonconnectsport.cpp \
devices/echelonrower/echelonrower.cpp \
devices/echelonstride/echelonstride.cpp \
devices/eliterizer/eliterizer.cpp \
devices/elitesterzosmart/elitesterzosmart.cpp \
devices/elliptical.cpp \
devices/eslinkertreadmill/eslinkertreadmill.cpp \
devices/fakebike/fakebike.cpp \
filedownloader.cpp \
devices/fitmetria_fanfit/fitmetria_fanfit.cpp \
devices/fitplusbike/fitplusbike.cpp \
devices/fitshowtreadmill/fitshowtreadmill.cpp \
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
devices/flywheelbike/flywheelbike.cpp \
devices/ftmsbike/ftmsbike.cpp \
devices/ftmsrower/ftmsrower.cpp \
gpx.cpp \
devices/heartratebelt/heartratebelt.cpp \
homefitnessbuddy.cpp \
homeform.cpp \
garminconnect.cpp \
devices/horizongr7bike/horizongr7bike.cpp \
devices/horizontreadmill/horizontreadmill.cpp \
devices/iconceptbike/iconceptbike.cpp \
devices/inspirebike/inspirebike.cpp \
keepawakehelper.cpp \
devices/keepbike/keepbike.cpp \
devices/kingsmithr1protreadmill/kingsmithr1protreadmill.cpp \
devices/kingsmithr2treadmill/kingsmithr2treadmill.cpp \
main.cpp \
devices/mcfbike/mcfbike.cpp \
metric.cpp \
devices/nautiluselliptical/nautiluselliptical.cpp \
devices/nautilustreadmill/nautilustreadmill.cpp \
devices/npecablebike/npecablebike.cpp \
devices/pafersbike/pafersbike.cpp \
devices/paferstreadmill/paferstreadmill.cpp \
peloton.cpp \
powerzonepack.cpp \
devices/proformbike/proformbike.cpp \
devices/proformelliptical/proformelliptical.cpp \
devices/proformtreadmill/proformtreadmill.cpp \
qfit.cpp \
qzsettings.cpp \
devices/renphobike/renphobike.cpp \
devices/rower.cpp \
devices/schwinnic4bike/schwinnic4bike.cpp \
screencapture.cpp \
sessionline.cpp \
devices/shuaa5treadmill/shuaa5treadmill.cpp \
signalhandler.cpp \
simplecrypt.cpp \
devices/skandikawiribike/skandikawiribike.cpp \
devices/smartrowrower/smartrowrower.cpp \
devices/smartspin2k/smartspin2k.cpp \
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
devices/snodebike/snodebike.cpp \
devices/solebike/solebike.cpp \
devices/soleelliptical/soleelliptical.cpp \
devices/solef80treadmill/solef80treadmill.cpp \
devices/spirittreadmill/spirittreadmill.cpp \
devices/sportsplusbike/sportsplusbike.cpp \
devices/sportstechbike/sportstechbike.cpp \
devices/strydrunpowersensor/strydrunpowersensor.cpp \
devices/tacxneo2/tacxneo2.cpp \
tcpclientinfosender.cpp \
devices/technogymmyruntreadmill/technogymmyruntreadmill.cpp \
devices/technogymmyruntreadmillrfcomm/technogymmyruntreadmillrfcomm.cpp \
templateinfosender.cpp \
templateinfosenderbuilder.cpp \
devices/stagesbike/stagesbike.cpp \
devices/toorxtreadmill/toorxtreadmill.cpp \
devices/treadmill.cpp \
devices/truetreadmill/truetreadmill.cpp \
devices/trxappgateusbbike/trxappgateusbbike.cpp \
devices/ultrasportbike/ultrasportbike.cpp \
virtualdevices/virtualrower.cpp \
devices/wahookickrsnapbike/wahookickrsnapbike.cpp \
devices/yesoulbike/yesoulbike.cpp \
trainprogram.cpp \
devices/trxappgateusbtreadmill/trxappgateusbtreadmill.cpp \
virtualdevices/virtualbike.cpp \
virtualdevices/virtualtreadmill.cpp \
devices/m3ibike/m3ibike.cpp \
devices/domyosbike/domyosbike.cpp \
scanrecordresult.cpp \
windows_zwift_incline_paddleocr_thread.cpp \
zwiftworkout.cpp
   
macx: SOURCES += macos/lockscreen.mm
!ios: SOURCES += mainwindow.cpp charts.cpp

#zwift api
msvc {
    SOURCES += zwift-api/zwift_messages.pb.cc
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += fit-sdk/ devices/

HEADERS += \
    $$PWD/EventHandler.h \
    $$PWD/characteristics/characteristicnotifier0002.h \
    $$PWD/characteristics/characteristicnotifier0004.h \
    $$PWD/characteristics/characteristicwriteprocessor0003.h \
    $$PWD/OAuth2.h \
    $$PWD/devices/android_antbike/android_antbike.h \
    $$PWD/devices/antbike/antbike.h \
    $$PWD/devices/coresensor/coresensor.h \
    $$PWD/devices/crossrope/crossrope.h \
    $$PWD/devices/cycleopsphantombike/cycleopsphantombike.h \
    $$PWD/devices/deeruntreadmill/deerruntreadmill.h \
    $$PWD/devices/echelonstairclimber/echelonstairclimber.h \
    $$PWD/devices/elitesquarecontroller/elitesquarecontroller.h \
    $$PWD/devices/focustreadmill/focustreadmill.h \
    $$PWD/devices/jumprope.h \
    $$PWD/devices/kineticinroadbike/SmartControl.h \
    $$PWD/devices/kineticinroadbike/kineticinroadbike.h \
    $$PWD/devices/lifespantreadmill/lifespantreadmill.h \
    $$PWD/devices/moxy5sensor/moxy5sensor.h \
    $$PWD/devices/nordictrackifitadbelliptical/nordictrackifitadbelliptical.h \
    $$PWD/devices/nordictrackifitadbrower/nordictrackifitadbrower.h \
    $$PWD/devices/pitpatbike/pitpatbike.h \
    $$PWD/devices/speraxtreadmill/speraxtreadmill.h \
    $$PWD/devices/sportsplusrower/sportsplusrower.h \
    $$PWD/devices/sportstechelliptical/sportstechelliptical.h \
    $$PWD/devices/sramAXSController/sramAXSController.h \
    $$PWD/devices/stairclimber.h \
    $$PWD/devices/technogymbike/technogymbike.h \
    $$PWD/devices/trxappgateusbelliptical/trxappgateusbelliptical.h \
    $$PWD/devices/trxappgateusbrower/trxappgateusbrower.h \
    $$PWD/ergtable.h \
    $$PWD/fitdatabaseprocessor.h \
    $$PWD/inclinationresistancetable.h \
    $$PWD/logwriter.h \
    $$PWD/fitbackupwriter.h \
    $$PWD/osc.h \
    $$PWD/oscpp/client.hpp \
    $$PWD/oscpp/detail/endian.hpp \
    $$PWD/oscpp/detail/host.hpp \
    $$PWD/oscpp/detail/stream.hpp \
    $$PWD/oscpp/error.hpp \
    $$PWD/oscpp/print.hpp \
    $$PWD/oscpp/server.hpp \
    $$PWD/oscpp/types.hpp \
    $$PWD/oscpp/util.hpp \
    $$PWD/mqtt/qmqttauthenticationproperties.h \
    $$PWD/mqtt/qmqttclient.h \
    $$PWD/mqtt/qmqttclient_p.h \
    $$PWD/mqtt/qmqttconnection_p.h \
    $$PWD/mqtt/qmqttconnectionproperties.h \
    $$PWD/mqtt/qmqttconnectionproperties_p.h \
    $$PWD/mqtt/qmqttcontrolpacket_p.h \
    $$PWD/mqtt/qmqttglobal.h \
    $$PWD/mqtt/qmqttmessage.h \
    $$PWD/mqtt/qmqttmessage_p.h \
    $$PWD/mqtt/qmqttpublishproperties.h \
    $$PWD/mqtt/qmqttpublishproperties_p.h \
    $$PWD/mqtt/qmqttsubscription.h \
    $$PWD/mqtt/qmqttsubscription_p.h \
    $$PWD/mqtt/qmqttsubscriptionproperties.h \
    $$PWD/mqtt/qmqtttopicfilter.h \
    $$PWD/mqtt/qmqtttopicname.h \
    $$PWD/mqtt/qmqtttype.h \
    $$PWD/treadmillErgTable.h \
    $$PWD/wheelcircumference.h \
    $$PWD/workoutloaderworker.h \
    $$PWD/workoutmodel.h \
QTelnet.h \
devices/bkoolbike/bkoolbike.h \
devices/csafe/csafe.h \
devices/csafe/csaferunner.h \
devices/csafe/csafeutility.h \
devices/csafe/serialhandler.h \
devices/csafe/serialport.h \
devices/csafe/netserial.h \
devices/csafe/kalmanfilter.h \
devices/csafeelliptical/csafeelliptical.h \
devices/csaferower/csaferower.h \
devices/eliteariafan/eliteariafan.h \
devices/proformtelnetbike/proformtelnetbike.h \
windows_zwift_workout_paddleocr_thread.h \
devices/fakerower/fakerower.h \
zwift-api/PlayerStateWrapper.h \
zwift-api/zwift_client_auth.h \
zwift_play/abstractZapDevice.h \
zwift_play/zapBleUuids.h \
zwift_play/zapConstants.h \
zwift_play/zwiftPlayDevice.h \
zwift_play/zwiftclickremote.h \
virtualdevices/virtualdevice.h \
androidactivityresultreceiver.h \
androidadblog.h \
devices/apexbike/apexbike.h \
devices/discoveryoptions.h \
handleurl.h \
devices/iconceptelliptical/iconceptelliptical.h \
localipaddress.h \
devices/pelotonbike/pelotonbike.h \
devices/schwinn170bike/schwinn170bike.h \
devices/wahookickrheadwind/wahookickrheadwind.h \
devices/ypooelliptical/ypooelliptical.h \
devices/ziprotreadmill/ziprotreadmill.h \
devices/computrainerbike/Computrainer.h \
devices/kettlerusbbike/KettlerUSB.h \
PathController.h \
characteristics/characteristicnotifier2a53.h \
characteristics/characteristicnotifier2a5b.h \
characteristics/characteristicnotifier2acc.h \
characteristics/characteristicnotifier2acd.h \
characteristics/characteristicnotifier2ad9.h \
characteristics/characteristicwriteprocessore005.h \
devices/computrainerbike/computrainerbike.h \
devices/kettlerusbbike/kettlerusbbike.h \
definitions.h \
devices/fakeelliptical/fakeelliptical.h \
devices/faketreadmill/faketreadmill.h \
devices/lifefitnesstreadmill/lifefitnesstreadmill.h \
devices/mepanelbike/mepanelbike.h \
devices/nautilusbike/nautilusbike.h \
devices/nordictrackelliptical/nordictrackelliptical.h \
devices/nordictrackifitadbbike/nordictrackifitadbbike.h \
devices/nordictrackifitadbtreadmill/nordictrackifitadbtreadmill.h \
devices/octaneelliptical/octaneelliptical.h \
devices/octanetreadmill/octanetreadmill.h \
devices/proformellipticaltrainer/proformellipticaltrainer.h \
devices/proformrower/proformrower.h \
devices/proformwifibike/proformwifibike.h \
devices/proformwifitreadmill/proformwifitreadmill.h \
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
devices/activiotreadmill/activiotreadmill.h \
devices/bhfitnesselliptical/bhfitnesselliptical.h \
devices/bike.h \
devices/bluetooth.h \
devices/bluetoothdevice.h \
characteristics/characteristicnotifier.h \
characteristics/characteristicnotifier2a37.h \
characteristics/characteristicnotifier2a63.h \
characteristics/characteristicnotifier2ad2.h \
characteristics/characteristicwriteprocessor.h \
characteristics/characteristicwriteprocessor2ad9.h \
devices/bowflext216treadmill/bowflext216treadmill.h \
devices/bowflextreadmill/bowflextreadmill.h \
devices/chronobike/chronobike.h \
devices/concept2skierg/concept2skierg.h \
devices/cscbike/cscbike.h \
devices/dircon/dirconmanager.h \
devices/dircon/dirconpacket.h \
devices/dircon/dirconprocessor.h \
devices/domyoselliptical/domyoselliptical.h \
devices/domyosrower/domyosrower.h \
devices/domyostreadmill/domyostreadmill.h \
devices/echelonconnectsport/echelonconnectsport.h \
devices/echelonrower/echelonrower.h \
devices/echelonstride/echelonstride.h \
devices/eliterizer/eliterizer.h \
devices/elitesterzosmart/elitesterzosmart.h \
devices/elliptical.h \
devices/eslinkertreadmill/eslinkertreadmill.h \
devices/fakebike/fakebike.h \
filedownloader.h \
devices/fitmetria_fanfit/fitmetria_fanfit.h \
devices/fitplusbike/fitplusbike.h \
devices/ftmsrower/ftmsrower.h \
homefitnessbuddy.h \
devices/horizongr7bike/horizongr7bike.h \
devices/iconceptbike/iconceptbike.h \
devices/keepbike/keepbike.h \
devices/kingsmithr1protreadmill/kingsmithr1protreadmill.h \
devices/kingsmithr2treadmill/kingsmithr2treadmill.h \
devices/m3ibike/m3ibike.h \
devices/fitshowtreadmill/fitshowtreadmill.h \
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
devices/flywheelbike/flywheelbike.h \
devices/ftmsbike/ftmsbike.h \
devices/heartratebelt/heartratebelt.h \
homeform.h \
garminconnect.h \
devices/horizontreadmill/horizontreadmill.h \
devices/inspirebike/inspirebike.h \
ios/lockscreen.h \
keepawakehelper.h \
macos/lockscreen.h \
ios/M3iIOS-Interface.h \
material.h \
devices/mcfbike/mcfbike.h \
metric.h \
devices/nautiluselliptical/nautiluselliptical.h \
devices/nautilustreadmill/nautilustreadmill.h \
devices/npecablebike/npecablebike.h \
devices/pafersbike/pafersbike.h \
devices/paferstreadmill/paferstreadmill.h \
peloton.h \
powerzonepack.h \
devices/proformbike/proformbike.h \
devices/proformelliptical/proformelliptical.h \
devices/proformtreadmill/proformtreadmill.h \
qdebugfixup.h \
qfit.h \
qmdnsengine_export.h \
qzsettings.h \
devices/renphobike/renphobike.h \
devices/rower.h \
devices/schwinnic4bike/schwinnic4bike.h \
screencapture.h \
sessionline.h \
devices/shuaa5treadmill/shuaa5treadmill.h \
signalhandler.h \
simplecrypt.h \
devices/skandikawiribike/skandikawiribike.h \
devices/smartrowrower/smartrowrower.h \
devices/smartspin2k/smartspin2k.h \
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
devices/snodebike/snodebike.h \
devices/solebike/solebike.h \
devices/soleelliptical/soleelliptical.h \
devices/solef80treadmill/solef80treadmill.h \
devices/spirittreadmill/spirittreadmill.h \
devices/sportsplusbike/sportsplusbike.h \
devices/sportstechbike/sportstechbike.h \
devices/strydrunpowersensor/strydrunpowersensor.h \
devices/tacxneo2/tacxneo2.h \
tcpclientinfosender.h \
devices/technogymmyruntreadmill/technogymmyruntreadmill.h \
devices/technogymmyruntreadmillrfcomm/technogymmyruntreadmillrfcomm.h \
templateinfosender.h \
templateinfosenderbuilder.h \
devices/stagesbike/stagesbike.h \
devices/toorxtreadmill/toorxtreadmill.h \
gpx.h \
devices/treadmill.h \
mainwindow.h \
trainprogram.h \
devices/truetreadmill/truetreadmill.h \
devices/trxappgateusbbike/trxappgateusbbike.h \
devices/trxappgateusbtreadmill/trxappgateusbtreadmill.h \
devices/ultrasportbike/ultrasportbike.h \
virtualdevices/virtualbike.h \
virtualdevices/virtualrower.h \
virtualdevices/virtualtreadmill.h \
devices/domyosbike/domyosbike.h \
devices/wahookickrsnapbike/wahookickrsnapbike.h \
wobjectdefs.h \
wobjectimpl.h \
devices/yesoulbike/yesoulbike.h \
scanrecordresult.h \
windows_zwift_incline_paddleocr_thread.h \
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
    $$PWD/android/libs/android_antlib_4-16-0.aar \
    $$PWD/android/libs/ciq-companion-app-sdk-2.0.3.aar \
    $$PWD/android/libs/zaplibrary-debug.aar \
    $$PWD/android/res/xml/device_filter.xml \
    $$PWD/android/src/BikeChannelController.java \
    $$PWD/android/src/BleAdvertiser.java \
   $$PWD/android/src/CSafeRowerUSBHID.java \
    $$PWD/android/src/ContentHelper.java \
    $$PWD/android/src/CustomQtActivity.java \
    $$PWD/android/src/Garmin.java \
   $$PWD/android/src/HidBridge.java \
    $$PWD/android/src/IQMessageReceiverWrapper.java \
    $$PWD/android/src/LocationHelper.java \
    $$PWD/android/src/MediaButtonReceiver.java \
    $$PWD/android/src/MediaProjection.java \
    $$PWD/android/src/NotificationUtils.java \
    $$PWD/android/src/QLog.java \
    $$PWD/android/src/ScreenCaptureService.java \
    $$PWD/android/src/Shortcuts.java \
    $$PWD/android/src/WearableController.java \
    $$PWD/android/src/WearableMessageListenerService.java \
    $$PWD/android/src/ZapClickLayer.java \
    $$PWD/android/src/ZwiftAPI.java \
    $$PWD/android/src/ZwiftHubBike.java \
    $$PWD/android/src/main/proto/zwift_hub.proto \
    $$PWD/android/src/main/proto/zwift_messages.proto \
    .clang-format \
   AppxManifest.xml \
   android/AndroidManifest.xml \
	android/build.gradle \
	android/gradle/wrapper/gradle-wrapper.jar \
	android/gradle/wrapper/gradle-wrapper.properties \
	android/gradlew \
	android/gradlew.bat \
   android/res/layout/floating_layout.xml \
	android/res/values/libs.xml \
	android/src/Ant.java \
	android/src/ChannelService.java \
   android/src/FloatingHandler.java \
   android/src/FloatingWindowGFG.java \
   android/src/ForegroundService.java \
   android/src/NotificationClient.java \
   android/src/QZAdbRemote.java \
        android/src/ScanRecordResult.java \
        android/src/NativeScanCallback.java \
   android/src/HeartChannelController.java \
	android/src/MyActivity.java \
	android/src/PowerChannelController.java \
	android/src/SpeedChannelController.java \
   android/src/SDMChannelController.java \
    android/src/Usbserial.java \
   android/src/com/cgutman/adblib/AdbBase64.java \
   android/src/com/cgutman/adblib/AdbConnection.java \
   android/src/com/cgutman/adblib/AdbCrypto.java \
   android/src/com/cgutman/adblib/AdbProtocol.java \
   android/src/com/cgutman/adblib/AdbStream.java \
   android/src/com/cgutman/adblib/package-info.java \
   android/src/com/cgutman/androidremotedebugger/AdbUtils.java \
   android/src/com/cgutman/androidremotedebugger/adblib/AndroidBase64.java \
   android/src/com/cgutman/androidremotedebugger/console/CommandHistory.java \
   android/src/com/cgutman/androidremotedebugger/console/ConsoleBuffer.java \
   android/src/com/cgutman/androidremotedebugger/devconn/DeviceConnection.java \
   android/src/com/cgutman/androidremotedebugger/devconn/DeviceConnectionListener.java \
   android/src/com/cgutman/androidremotedebugger/service/ShellListener.java \
   android/src/com/cgutman/androidremotedebugger/service/ShellService.java \
   android/src/com/cgutman/androidremotedebugger/ui/Dialog.java \
   android/src/com/cgutman/androidremotedebugger/ui/SpinnerDialog.java \
	android/src/com/dsi/ant/channel/PredefinedNetwork.java \
    android/gradle.properties \
	android/src/org/qtproject/qt/android/purchasing/Security.java \
	android/src/org/qtproject/qt/android/purchasing/InAppPurchase.java \
	android/src/org/qtproject/qt/android/purchasing/Base64.java \
	android/src/org/qtproject/qt/android/purchasing/Base64DecoderException.java \
	ios/AppDelegate.swift \
	ios/BLEPeripheralManager.swift \
	ios/LiveActivityManager.swift

win32: DISTFILES += \
   $$PWD/adb/AdbWinApi.dll \
	$$PWD/adb/AdbWinUsbApi.dll \
	$$PWD/adb/adb.exe \


ios {
    ios_icon.files = $$files($$PWD/icons/ios/*.png)
	 QMAKE_BUNDLE_DATA += ios_icon
}

ios {
    OBJECTIVE_SOURCES += ios/lockscreen.mm \
    ios/ios_eliteariafan.mm \
    ios/ios_app_delegate.mm \
    ios/ios_liveactivity.mm \
	 fit-sdk/FitDecode.mm \
	 fit-sdk/FitDeveloperField.mm \
	 fit-sdk/FitEncode.mm \
	 fit-sdk/FitField.mm \
	 fit-sdk/FitFieldDefinition.mm \
	 fit-sdk/FitMesg.mm \
         fit-sdk/FitMesgDefinition.mm \
         ios/M3iNS.mm \

    SOURCES += ios/M3iNSQT.cpp

    OBJECTIVE_HEADERS += ios/M3iNS.h \
    ios/ios_liveactivity.h

    QMAKE_INFO_PLIST = ios/Info.plist
	 QMAKE_ASSET_CATALOGS = $$PWD/ios/Images.xcassets
	 QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"
	 QMAKE_ASSET_CATALOGS_BUILD_PATH = $$PWD/ios/ 

    TARGET = qdomyoszwift
	 QMAKE_TARGET_BUNDLE_PREFIX = org.cagnulein
    DEFINES+=_Nullable_result=_Nullable NS_FORMAT_ARGUMENT\\(A\\)=
}

HEADERS += \
    mqttpublisher.h \
    androidstatusbar.h \
    fontmanager.h

SOURCES += \
    mqttpublisher.cpp \
    androidstatusbar.cpp \
    fontmanager.cpp

include($$PWD/purchasing/purchasing.pri)
INCLUDEPATH += purchasing/qmltypes
INCLUDEPATH += purchasing/inapp

WINRT_MANIFEST = AppxManifest.xml

VERSION = 2.20.22
