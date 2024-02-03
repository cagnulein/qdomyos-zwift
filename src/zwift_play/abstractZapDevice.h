#ifndef ABSTRACTZAPDEVICE_H
#define ABSTRACTZAPDEVICE_H

#include <QByteArray>
#include <QString>
#include <QDebug>
//#include "localKeyProvider.h"
//#include "zapCrypto.h"
#include "zapConstants.h"
#ifdef Q_OS_ANDROID
#include <QAndroidJniObject>
#include <QAndroidJniEnvironment>
#endif

class AbstractZapDevice: public QObject {
    Q_OBJECT
public:
    QByteArray RIDE_ON;
    QByteArray REQUEST_START;
    QByteArray RESPONSE_START;

    //ZapCrypto zapEncryption;
    AbstractZapDevice() /*: localKeyProvider(), zapEncryption(localKeyProvider)*/ {
        RIDE_ON = QByteArray::fromRawData("\x52\x69\x64\x65\x4F\x6E", 6);  // "RideOn"
        REQUEST_START = QByteArray::fromRawData("\x00\x09", 2);  // {0, 9}
        RESPONSE_START = QByteArray::fromRawData("\x01\x03", 2);  // {1, 3}
    }

    void processCharacteristic(const QString& characteristicName, const QByteArray& bytes) {
        if (bytes.isEmpty()) return;

        qDebug() << characteristicName << bytes.toHex();

#ifdef Q_OS_ANDROID
        QAndroidJniEnvironment env;
        jbyteArray d = env->NewByteArray(bytes.length());
        jbyte *b = env->GetByteArrayElements(d, 0);
        for (int i = 0; i < bytes.length(); i++)
            b[i] = bytes[i];
        env->SetByteArrayRegion(d, 0, bytes.length(), b);

        QAndroidJniObject::callStaticMethod<void>(
            "org/cagnulen/qdomyoszwift/ZapClickLayer", "processCharacteristic", "([B)V", d);
        env->DeleteLocalRef(d);
#endif

    }

    QByteArray buildHandshakeStart() {
#ifdef Q_OS_ANDROID
    QAndroidJniObject result =
        QAndroidJniObject::callStaticObjectMethod("org/cagnulen/qdomyoszwift/ZapClickLayer", "buildHandshakeStart", "()[B");
    if (result.isValid()) {
        // Ottiene la lunghezza dell'array di byte
        jsize length = QAndroidJniEnvironment()->GetArrayLength(result.object<jbyteArray>());

        // Allocare memoria per i byte nativi
        jbyte* bytes = QAndroidJniEnvironment()->GetByteArrayElements(result.object<jbyteArray>(), nullptr);

        // Costruire un QByteArray dal buffer di byte nativi
        QByteArray byteArray(reinterpret_cast<char*>(bytes), length);

        // Rilasciare la memoria dell'array di byte JNI
        QAndroidJniEnvironment()->ReleaseByteArrayElements(result.object<jbyteArray>(), bytes, JNI_ABORT);

        // Ora puoi usare byteArray come necessario
        return byteArray;
    }
#endif
        //return RIDE_ON + REQUEST_START + localKeyProvider.getPublicKeyBytes();
    }

protected:
    virtual void processEncryptedData(const QByteArray& bytes) = 0;

private:
    QByteArray devicePublicKeyBytes;
};

#endif // ABSTRACTZAPDEVICE_H
