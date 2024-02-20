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
    enum ZWIFT_PLAY_TYPE {
        NONE,
        LEFT,
        RIGHT
    };

    QByteArray RIDE_ON;
    QByteArray REQUEST_START;
    QByteArray RESPONSE_START;

    //ZapCrypto zapEncryption;
    AbstractZapDevice() /*: localKeyProvider(), zapEncryption(localKeyProvider)*/ {
        RIDE_ON = QByteArray::fromRawData("\x52\x69\x64\x65\x4F\x6E", 6);  // "RideOn"
        REQUEST_START = QByteArray::fromRawData("\x00\x09", 2);  // {0, 9}
        RESPONSE_START = QByteArray::fromRawData("\x01\x03", 2);  // {1, 3}
    }

    int processCharacteristic(const QString& characteristicName, const QByteArray& bytes, ZWIFT_PLAY_TYPE zapType) {
        if (bytes.isEmpty()) return 0;

        qDebug() << zapType << characteristicName << bytes.toHex() ;

#ifdef Q_OS_ANDROID_ENCRYPTION
        QAndroidJniEnvironment env;
        jbyteArray d = env->NewByteArray(bytes.length());
        jbyte *b = env->GetByteArrayElements(d, 0);
        for (int i = 0; i < bytes.length(); i++)
            b[i] = bytes[i];
        env->SetByteArrayRegion(d, 0, bytes.length(), b);

        int button = QAndroidJniObject::callStaticMethod<int>(
            "org/cagnulen/qdomyoszwift/ZapClickLayer", "processCharacteristic", "([B)I", d);
        env->DeleteLocalRef(d);
        if(button == 1)
            emit plus();
        else if(button == 2)
            emit minus();
        return button;
#else
        switch(bytes[0]) {
            case 0x37:
                if(bytes.length() == 5) {
                    if(bytes[2] == 0) {
                        emit plus();
                    } else if(bytes[4] == 0) {
                        emit minus();
                    }
                }
                break;
            case 0x07:
                if(bytes.length() > 3 && bytes[bytes.length() - 5] == 0x40 && (
                        (((uint8_t)bytes[bytes.length() - 4]) == 0xc7 && zapType == RIGHT) ||
                        (((uint8_t)bytes[bytes.length() - 4]) == 0xc8 && zapType == LEFT)
                    ) && bytes[bytes.length() - 3] == 0x01) {
                    if(zapType == LEFT) {
                        emit plus();
                    } else {
                        emit minus();
                    }
                }
                break;
        }
        return 1;
#endif

    }

    QByteArray buildHandshakeStart() {
#ifdef Q_OS_ANDROID_ENCRYPTION
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
        QByteArray a;
        a.append(0x52);
        a.append(0x69);
        a.append(0x64);
        a.append(0x65);
        a.append(0x4f);
        a.append(0x6e);
        return a;
    }

protected:
    virtual void processEncryptedData(const QByteArray& bytes) = 0;

private:
    QByteArray devicePublicKeyBytes;

signals:
    void plus();
    void minus();
};

#endif // ABSTRACTZAPDEVICE_H
