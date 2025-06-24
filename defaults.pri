QT += gui bluetooth widgets xml positioning quick networkauth websockets texttospeech location multimedia
QTPLUGIN += qavfmediaplayer
QT+= charts

unix:android: QT += androidextras gui-private

android: include(android_openssl/openssl.pri)

INCLUDEPATH += $$PWD/src/qmdnsengine/src/include

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/src/android

ANDROID_ABIS = armeabi-v7a arm64-v8a

#QMAKE_CXXFLAGS += -Werror=suggest-override
