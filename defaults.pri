QT += gui bluetooth widgets xml positioning quick networkauth websockets texttospeech location multimedia
QTPLUGIN += qavfmediaplayer
QT+= charts

qtHaveModule(httpserver) {
    QT += httpserver
    DEFINES += Q_HTTPSERVER

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

unix:android: QT += androidextras gui-private

android: include(android_openssl/openssl.pri)

INCLUDEPATH += $$PWD/src/qmdnsengine/src/include

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/src/android

ANDROID_ABIS = armeabi-v7a arm64-v8a x86 x86_64

#QMAKE_CXXFLAGS += -Werror=suggest-override
