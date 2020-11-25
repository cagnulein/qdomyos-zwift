QT += bluetooth widgets xml positioning charts quick
unix:android: QT += androidextras

CONFIG += c++11 console debug app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
   bike.cpp \
        bluetooth.cpp \
   bluetoothdevice.cpp \
   charts.cpp \
        domyostreadmill.cpp \
        gpx.cpp \
   homeform.cpp \
        main.cpp \
   sessionline.cpp \
   signalhandler.cpp \
        toorxtreadmill.cpp \
        treadmill.cpp \
        mainwindow.cpp \
        trainprogram.cpp \
   trxappgateusbtreadmill.cpp \
    virtualbike.cpp \
        virtualtreadmill.cpp \
    domyosbike.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
   bike.h \
   bluetooth.h \
   bluetoothdevice.h \
   charts.h \
   domyostreadmill.h \
   homeform.h \
   sessionline.h \
   signalhandler.h \
   toorxtreadmill.h \
   gpx.h \
   treadmill.h \
   mainwindow.h \
   trainprogram.h \
   trxappgateusbtreadmill.h \
    virtualbike.h \
   virtualtreadmill.h \
    domyosbike.h

FORMS += \
   charts.ui \
   mainwindow.ui

RESOURCES += \
   icons.qrc \
   qml.qrc

DISTFILES += \
   android/AndroidManifest.xml \
   android/build.gradle \
   android/gradle/wrapper/gradle-wrapper.jar \
   android/gradle/wrapper/gradle-wrapper.properties \
   android/gradlew \
   android/gradlew.bat \
	android/res/values/libs.xml \
   android/src/MyActivity.java

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

ANDROID_ABIS = armeabi-v7a
