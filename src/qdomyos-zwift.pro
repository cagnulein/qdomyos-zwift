QT += bluetooth widgets xml positioning quick

!ios: QT+= charts
unix:android: QT += androidextras

CONFIG += c++11 console debug app_bundle
macx: CONFIG += static

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
        domyostreadmill.cpp \
   echelonconnectsport.cpp \
        gpx.cpp \
   homeform.cpp \
   keepawakehelper.cpp \
        main.cpp \
   sessionline.cpp \
   signalhandler.cpp \
        toorxtreadmill.cpp \
        treadmill.cpp \        
        trainprogram.cpp \
   trxappgateusbtreadmill.cpp \
    virtualbike.cpp \
        virtualtreadmill.cpp \
    domyosbike.cpp

macx: SOURCES += macos/lockscreen.mm
!ios: SOURCES += mainwindow.cpp charts.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
   bike.h \
   bluetooth.h \
   bluetoothdevice.h \
   domyostreadmill.h \
   echelonconnectsport.h \
   homeform.h \
   keepawakehelper.h \
   macos/lockscreen.h \
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

!ios: HEADERS += charts.h

!ios: FORMS += \
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

ios {
    ios_icon.files = $$files($$PWD/icons/ios/*.png)
	 QMAKE_BUNDLE_DATA += ios_icon
}

ios {
    QMAKE_INFO_PLIST = ios/Info.plist
	 QMAKE_ASSET_CATALOGS = $$PWD/ios/Images.xcassets
	 QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"
}

TARGET = qdomyoszwift
QMAKE_TARGET_BUNDLE_PREFIX = org.cagnulein
VERSION = 1.4.1
