GOOGLETEST_DIR = $$PWD/googletest

include(../defaults.pri)
include(gtest_dependency.pri)

TEMPLATE = app

CONFIG += console c++11
CONFIG -= app_bundle
CONFIG += thread
CONFIG += androidextras

SOURCES += \
        Devices/ActivioTreadmill/activiotreadmilltestdata.cpp \
        Devices/BHFitnessElliptical/bhfitnessellipticaltestdata.cpp \
        Devices/Bike/biketestdata.cpp \
        Devices/BowflexT216Treadmill/bowflext216treadmilltestdata.cpp \
        Devices/BowflexTreadmill/bowflextreadmilltestdata.cpp \
        Devices/CSCBike/cscbiketestdata.cpp \
        Devices/Chronobike/chronobiketestdata.cpp \
        Devices/CompuTrainer/computrainertestdata.cpp \
        Devices/Concept2SkiErg/concept2skiergtestdata.cpp \
        Devices/DomyosBike/domyosbiketestdata.cpp \
        Devices/DomyosElliptical/domyosellipticaltestdata.cpp \
        Devices/DomyosRower/domyosrowertestdata.cpp \
        Devices/DomyosTreadmill/domyostreadmilltestdata.cpp \
        Devices/ESLinkerTreadmill/eslinkertreadmilltestdata.cpp \
        Devices/EchelonConnectSportBike/echelonconnectsportbiketestdata.cpp \
        Devices/EchelonRower/echelonrowertestdata.cpp \
        Devices/EchelonStrideTreadmill/echelonstridetreadmilltestdata.cpp \
        Devices/Elliptical/ellipticaltestdata.cpp \
        Devices/FTMSBike/ftmsbiketestdata.cpp \
        Devices/FTMSRower/ftmsrowertestdata.cpp \
        Devices/FakeBike/fakebiketestdata.cpp \
        Devices/FakeElliptical/fakeellipticaltestdata.cpp \
        Devices/FakeTreadmill/faketreadmilltestdata.cpp \
        Devices/FitPlusBike/fitplusbiketestdata.cpp \
        Devices/FitshowTreadmill/fitshowtreadmilltestdata.cpp \
        Devices/FlywheelBike/flywheelbiketestdata.cpp \
        Devices/HorizonGR7Bike/horizongr7biketestdata.cpp \
        Devices/HorizonTreadmill/horizontreadmilltestdata.cpp \
        Devices/InspireBike/inspirebiketestdata.cpp \
        Devices/KeepBike/keepbiketestdata.cpp \
        Devices/KingsmithR1ProTreadmill/kingsmithr1protreadmilltestdata.cpp \
        Devices/KingsmithR2Treadmill/kingsmithr2treadmilltestdata.cpp \
        Devices/LifeFitnessTreadmill/lifefitnesstreadmilltestdata.cpp \
        Devices/M3IBike/m3ibiketestdata.cpp \
        Devices/MCFBike/mcfbiketestdata.cpp \
        Devices/MepanelBike/mepanelbiketestdata.cpp \
        Devices/NPECableBike/npecablebiketestdata.cpp \
        Devices/NautilusBike/nautilusbiketestdata.cpp \
        Devices/NautilusElliptical/nautilusellipticaltestdata.cpp \
        Devices/NautilusTreadmill/nautilustreadmilltestdata.cpp \
        Devices/NordicTrackElliptical/nordictrackellipticaltestdata.cpp \
        Devices/NordicTrackIFitADBTreadmill/nordictrackifitadbtreadmilltestdata.cpp \
        Devices/OctaneElliptical/octaneellipticaltestdata.cpp \
        Devices/OctaneTreadmill/octanetreadmilltestdata.cpp \
        Devices/PafersBike/pafersbiketestdata.cpp \
        Devices/PafersTreadmill/paferstreadmilltestdata.cpp \
        Devices/ProFormBike/proformbiketestdata.cpp \
        Devices/ProFormElliptical/proformellipticaltestdata.cpp \
        Devices/ProFormEllipticalTrainer/proformellipticaltrainertestdata.cpp \
        Devices/ProFormRower/proformrowertestdata.cpp \
        Devices/ProFormTreadmill/proformtreadmilltestdata.cpp \
        Devices/ProFormWiFiBike/proformwifibiketestdata.cpp \
        Devices/ProFormWiFiTreadmill/proformwifitreadmilltestdata.cpp \
        Devices/RenphoBike/renphobiketestdata.cpp \
        Devices/Rower/rowertestdata.cpp \
        Devices/SchwinnIC4Bike/schwinnic4biketestdata.cpp \
        Devices/Shuaa5Treadmill/shuaa5treadmilltestdata.cpp \
        Devices/SkandikaWiryBike/skandikawirybiketestdata.cpp \
        Devices/SmartRowRower/smartrowrowertestdata.cpp \
        Devices/SnodeBike/snodebiketestdata.cpp \
        Devices/SoleBike/solebiketestdata.cpp \
        Devices/SoleElliptical/soleellipticaltestdata.cpp \
        Devices/SoleF80Treadmill/solef80treadmilltestdata.cpp \
        Devices/SpiritTreadmill/spirittreadmilltestdata.cpp \
        Devices/SportsPlusBike/sportsplusbiketestdata.cpp \
        Devices/SportsTechBike/sportstechbiketestdata.cpp \
        Devices/StagesBike/stagesbiketestdata.cpp \
        Devices/StrydeRunPowerSensor/stryderunpowersensortestdata.cpp \
        Devices/TacxNeo2/tacxneo2testdata.cpp \
        Devices/TechnoGymMyRunTreadmill/technogymmyruntreadmilltestdata.cpp \
        Devices/TechnogymMyRunTreadmillRFComm/technogymmyruntreadmillrfcommtestdata.cpp \
        Devices/ToorxTreadmill/toorxtreadmilltestdata.cpp \
        Devices/Treadmill/treadmilltestdata.cpp \
        Devices/TrueTreadmill/truetreadmilltestdata.cpp \
        Devices/TrxAppGateUSBBike/trxappgateusbbiketestdata.cpp \
        Devices/TrxAppGateUSBTreadmill/trxappgateusbtreadmilltestdata.cpp \
        Devices/UltrasportBike/ultrasportbiketestdata.cpp \
        Devices/WahooKickrSnapBike/wahookickrsnapbiketestdata.cpp \
        Devices/YesoulBike/yesoulbiketestdata.cpp \
        Devices/ZiproTreadmill/ziprotreadmilltestdata.cpp \
        Devices/bluetoothdevicetestdata.cpp \
        Devices/bluetoothdevicetestsuite.cpp \
        Devices/bluetoothsignalreceiver.cpp \
        Devices/devicediscoveryinfo.cpp \
        Devices/iConceptBike/iconceptbiketestdata.cpp \
        Devices/lockscreenfunctionstestdata.cpp \
        ToolTests/testsettingstestsuite.cpp \
        Tools/testlockscreen.cpp \
        Tools/testsettings.cpp \
        VirtualDevices/Bike/virtualbiketestdata.cpp \
        VirtualDevices/Rower/virtualrowertestdata.cpp \
        VirtualDevices/Treadmill/virtualtreadmilltestdata.cpp \
        VirtualDevices/virtualdevicetestdata.cpp \
        VirtualDevices/virtualdevicetestsuite.cpp \
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
    Devices/OctaneElliptical/octaneellipticaltestdata.h \
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
    Devices/Schwinn170Bike/schwinn170biketestdata.h \
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
    Devices/ZiproTreadmill/ziprotreadmilltestdata.h \
    Devices/bluetoothdevicetestdata.h \
    Devices/bluetoothdevicetestsuite.h \
    Devices/bluetoothsignalreceiver.h \
    Devices/devicediscoveryinfo.h \
    Devices/devices.h \
    Devices/iConceptBike/iconceptbiketestdata.h \
    Devices/lockscreenfunctionstestdata.h \
    ToolTests/testsettingstestsuite.h \
    Tools/testlockscreen.h \
    Tools/testsettings.h \
    VirtualDevices/Bike/virtualbiketestdata.h \
    VirtualDevices/Rower/virtualrowertestdata.h \
    VirtualDevices/Treadmill/virtualtreadmilltestdata.h \
    VirtualDevices/virtualdevicetestdata.h \
    VirtualDevices/virtualdevicetestsuite.h
