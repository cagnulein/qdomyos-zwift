GOOGLETEST_DIR = $$PWD/googletest

include(../defaults.pri)
include(gtest_dependency.pri)

TEMPLATE = app

CONFIG += console c++11
CONFIG -= app_bundle
CONFIG += thread
CONFIG += androidextras

SOURCES += \
        Devices/FTMSBike/ftmsbiketestdata.cpp \
        Devices/FitPlusBike/fitplusbiketestdata.cpp \
        Devices/M3IBike/m3ibiketestdata.cpp \
        Devices/RenphoBike/renphobiketestdata.cpp \
        Devices/SnodeBike/snodebiketestdata.cpp \
        Devices/StagesBike/stagesbiketestdata.cpp \
        Devices/TrxAppGateUSBTreadmill/trxappgateusbtreadmilltestdata.cpp \
        Devices/bluetoothdevicetestdata.cpp \
        Devices/bluetoothdevicetestsuite.cpp \
        Devices/devicediscoveryinfo.cpp \
        ToolTests/testsettingstestsuite.cpp \
        Tools/testsettings.cpp \
        main.cpp

# Avoid the "File too big" error building in Windows. This has happened when a template class is used with Google Test / typed tests
# to produce a large number of classes.
win32:QMAKE_CXXFLAGS += -Wa,-mbig-obj

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../src/release/ -lqdomyos-zwift
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../src/debug/ -lqdomyos-zwift
else:unix: LIBS += -L$$OUT_PWD/../src/ -lqdomyos-zwift

INCLUDEPATH += $$PWD/../src
DEPENDPATH += $$PWD/../src

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../src/release/libqdomyos-zwift.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../src/debug/libqdomyos-zwift.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../src/release/qdomyos-zwift.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../src/debug/qdomyos-zwift.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../src/libqdomyos-zwift.a

HEADERS += \
    Devices/ActivioTreadmill/activiotreadmilltestdata.h \
    Devices/BHFitnessElliptical/bhfitnessellipticaltestdata.h \
    Devices/Bike/biketestdata.h \
    Devices/BowflexT216Treadmill/bowflext216treadmilltestdata.h \
    Devices/BowflexTreadmill/bowflextreadmilltestdata.h \
    Devices/CSCBike/cscbiketestdata.h \
    Devices/Chronobike/chronobiketestdata.h \
    Devices/CompuTrainer/computrainertestdata.h \
    Devices/Concept2SkiErg/concept2skiergtestdata.h \
    Devices/DomyosBike/domyosbiketestdata.h \
    Devices/DomyosElliptical/domyosellipticaltestdata.h \
    Devices/DomyosRower/domyosrowertestdata.h \
    Devices/DomyosTreadmill/domyostreadmilltestdata.h \
    Devices/ESLinkerTreadmill/eslinkertreadmilltestdata.h \
    Devices/EchelonConnectSportBike/echelonconnectsportbiketestdata.h \
    Devices/EchelonRower/echelonrowertestdata.h \
    Devices/EchelonStrideTreadmill/echelonstridetreadmilltestdata.h \
    Devices/EliteSterzoSmart/elitesterzosmarttestdata.h \
    Devices/Elliptical/ellipticaltestdata.h \
    Devices/FTMSBike/ftmsbiketestdata.h \
    Devices/FTMSRower/ftmsrowertestdata.h \
    Devices/FakeBike/fakebiketestdata.h \
    Devices/FakeElliptical/fakeellipticaltestdata.h \
    Devices/FakeTreadmill/faketreadmilltestdata.h \
    Devices/FitPlusBike/fitplusbiketestdata.h \
    Devices/FitshowTreadmill/fitshowtreadmilltestdata.h \
    Devices/FlywheelBike/flywheelbiketestdata.h \
    Devices/HorizonGR7Bike/horizongr7biketestdata.h \
    Devices/HorizonTreadmill/horizontreadmilltestdata.h \
    Devices/InspireBike/inspirebiketestdata.h \
    Devices/KeepBike/keepbiketestdata.h \
    Devices/KingsmithR1ProTreadmill/kingsmithr1protreadmilltestdata.h \
    Devices/KingsmithR2Treadmill/kingsmithr2treadmilltestdata.h \
    Devices/LifeFitnessTreadmill/lifefitnesstreadmilltestdata.h \
    Devices/M3IBike/m3ibiketestdata.h \
    Devices/MCFBike/mcfbiketestdata.h \
    Devices/MepanelBike/mepanelbiketestdata.h \
    Devices/NPECableBike/npecablebiketestdata.h \
    Devices/NautilusBike/nautilusbiketestdata.h \
    Devices/NautilusElliptical/nautilusellipticaltestdata.h \
    Devices/NautilusTreadmill/nautilustreadmilltestdata.h \
    Devices/NordicTrackElliptical/nordictrackellipticaltestdata.h \
    Devices/NordicTrackIFitADBTreadmill/nordictrackifitadbtreadmilltestdata.h \
    Devices/OctaneTreadmill/octanetreadmilltestdata.h \
    Devices/PafersBike/pafersbiketestdata.h \
    Devices/PafersTreadmill/paferstreadmilltestdata.h \
    Devices/ProFormBike/proformbiketestdata.h \
    Devices/ProFormElliptical/proformellipticaltestdata.h \
    Devices/ProFormEllipticalTrainer/proformellipticaltrainertestdata.h \
    Devices/ProFormRower/proformrowertestdata.h \
    Devices/ProFormTreadmill/proformtreadmilltestdata.h \
    Devices/ProFormWiFiBike/proformwifibiketestdata.h \
    Devices/ProFormWiFiTreadmill/proformwifitreadmilltestdata.h \
    Devices/RenphoBike/renphobiketestdata.h \
    Devices/Rower/rowertestdata.h \
    Devices/SchwinnIC4Bike/schwinnic4biketestdata.h \
    Devices/Shuaa5Treadmill/shuaa5treadmilltestdata.h \
    Devices/SkandikaWiryBike/skandikawirybiketestdata.h \
    Devices/SmartRowRower/smartrowrowertestdata.h \
    Devices/SnodeBike/snodebiketestdata.h \
    Devices/SoleBike/solebiketestdata.h \
    Devices/SoleElliptical/soleellipticaltestdata.h \
    Devices/SoleF80Treadmill/solef80treadmilltestdata.h \
    Devices/SpiritTreadmill/spirittreadmilltestdata.h \
    Devices/SportsPlusBike/sportsplusbiketestdata.h \
    Devices/SportsTechBike/sportstechbiketestdata.h \
    Devices/StagesBike/stagesbiketestdata.h \
    Devices/StrydeRunPowerSensor/stryderunpowersensortestdata.h \
    Devices/TacxNeo2/tacxneo2testdata.h \
    Devices/TechnoGymMyRunTreadmill/technogymmyruntreadmilltestdata.h \
    Devices/TechnogymMyRunTreadmillRFComm/technogymmyruntreadmillrfcommtestdata.h \
    Devices/ToorxTreadmill/toorxtreadmilltestdata.h \
    Devices/Treadmill/treadmilltestdata.h \
    Devices/TrueTreadmill/truetreadmilltestdata.h \
    Devices/TrxAppGateUSBBike/trxappgateusbbiketestdata.h \
    Devices/TrxAppGateUSBTreadmill/trxappgateusbtreadmilltestdata.h \
    Devices/UltrasportBike/ultrasportbiketestdata.h \
    Devices/WahooKickrSnapBike/wahookickrsnapbiketestdata.h \
    Devices/YesoulBike/yesoulbiketestdata.h \
    Devices/bluetoothdevicetestdata.h \
    Devices/bluetoothdevicetestsuite.h \
    Devices/devicediscoveryinfo.h \
    Devices/devices.h \
    Devices/iConceptBike/iconceptbiketestdata.h \
    ToolTests/testsettingstestsuite.h \
    Tools/testsettings.h
