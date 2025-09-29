#ifndef KETTLERHANDSHAKEREADER_H
#define KETTLERHANDSHAKEREADER_H

#include <QObject>
#include <QByteArray>
#include <QString>

#ifdef Q_OS_ANDROID
#include <QtAndroidExtras/QAndroidJniObject>
#include <QtAndroidExtras/QtAndroid>
#include <QtAndroidExtras/QAndroidJniEnvironment>
#include <jni.h>
#endif

class KettlerHandshakeReader : public QObject
{
    Q_OBJECT

public:
    explicit KettlerHandshakeReader(QObject *parent = nullptr);
    static KettlerHandshakeReader* getInstance();

    void connectToDevice(const QString& deviceAddress);
    bool sendHandshakeResponse(const QByteArray& handshakeData);
    void setPower(int power);
    void disconnectDevice();

signals:
    void deviceConnected();
    void deviceDisconnected();
    void handshakeSeedReceived(const QByteArray& seedData);
    void handshakeReadError(const QString& error);
    void dataReceived(const QString& characteristicUuid, const QByteArray& data);

#ifdef Q_OS_ANDROID
private:
    static KettlerHandshakeReader* instance;

public slots:
    static void onDeviceConnected();
    static void onDeviceDisconnected();
    static void onHandshakeSeedReceived(const QByteArray& seedData);
    static void onHandshakeReadError(const QString& error);
    static void onDataReceived(const QString& characteristicUuid, const QByteArray& data);
#endif
};

#ifdef Q_OS_ANDROID
// JNI callback functions
extern "C" {
    JNIEXPORT void JNICALL Java_org_cagnulen_qdomyoszwift_KettlerHandshakeReader_onDeviceConnected(JNIEnv *env, jclass clazz);
    JNIEXPORT void JNICALL Java_org_cagnulen_qdomyoszwift_KettlerHandshakeReader_onDeviceDisconnected(JNIEnv *env, jclass clazz);
    JNIEXPORT void JNICALL Java_org_cagnulen_qdomyoszwift_KettlerHandshakeReader_onHandshakeSeedReceived(JNIEnv *env, jclass clazz, jbyteArray seedData);
    JNIEXPORT void JNICALL Java_org_cagnulen_qdomyoszwift_KettlerHandshakeReader_onHandshakeReadError(JNIEnv *env, jclass clazz, jstring error);
    JNIEXPORT void JNICALL Java_org_cagnulen_qdomyoszwift_KettlerHandshakeReader_onDataReceived(JNIEnv *env, jclass clazz, jstring characteristicUuid, jbyteArray data);
}
#endif

#endif // KETTLERHANDSHAKEREADER_H
