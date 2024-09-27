#ifndef ABSTRACTZAPDEVICE_H
#define ABSTRACTZAPDEVICE_H

#include <QByteArray>
#include <QString>
#include <QDebug>
#include <QSettings>
//#include "localKeyProvider.h"
//#include "zapCrypto.h"
#include "zapConstants.h"
#include "qzsettings.h"
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
        risingEdge = false;
    }

    int processCharacteristic(const QString& characteristicName, const QByteArray& bytes, ZWIFT_PLAY_TYPE zapType) {
        if (bytes.isEmpty()) return 0;

        QSettings settings;
        bool gears_volume_debouncing = settings.value(QZSettings::gears_volume_debouncing, QZSettings::default_gears_volume_debouncing).toBool();
        bool zwiftplay_swap = settings.value(QZSettings::zwiftplay_swap, QZSettings::default_zwiftplay_swap).toBool();

        qDebug() << zapType << characteristicName << bytes.toHex() << zwiftplay_swap << gears_volume_debouncing << risingEdge;

#define DEBOUNCE (!gears_volume_debouncing || !risingEdge)

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
                        if(DEBOUNCE) {
                            risingEdge = true;
                            if(!zwiftplay_swap)
                                emit plus();
                            else
                                emit minus();
                        }
                    } else if(bytes[4] == 0) {
                        if(DEBOUNCE) {
                            risingEdge = true;
                            if(!zwiftplay_swap)
                                emit minus();
                            else
                                emit plus();
                        }
                    } else {
                        risingEdge = false;
                    }
                }
                break;
            case 0x07: // zwift play
                if(bytes.length() > 5 && bytes[bytes.length() - 5] == 0x40 && (
                        (((uint8_t)bytes[bytes.length() - 4]) == 0xc7 && zapType == RIGHT) ||
                        (((uint8_t)bytes[bytes.length() - 4]) == 0xc8 && zapType == LEFT)
                    ) && bytes[bytes.length() - 3] == 0x01) {
                    if(zapType == LEFT) {
                        if(DEBOUNCE) {
                            risingEdge = true;
                            if(!zwiftplay_swap)
                                emit plus();
                            else
                                emit minus();
                        }
                    } else {
                        if(DEBOUNCE) {
                            risingEdge = true;
                            if(!zwiftplay_swap)
                                emit minus();
                            else
                                emit plus();
                        }
                    }
                } else if(bytes.length() > 14 && bytes[11] == 0x30 && bytes[12] == 0x00) {
                    if(zapType == LEFT) {
                        if(DEBOUNCE) {
                            risingEdge = true;
                            if(!zwiftplay_swap)
                                emit plus();
                            else
                                emit minus();
                        }
                    } else {
                        if(DEBOUNCE) {
                            risingEdge = true;
                            if(!zwiftplay_swap)
                                emit minus();
                            else
                                emit plus();
                        }
                    }
                } else {
                    risingEdge = false;
                }
                break;
            case 0x15: // empty data
                qDebug() << "ignoring this frame";
                return 1;
            case 0x23: // zwift ride
                if(bytes.length() > 12 &&
                        ((((uint8_t)bytes[12]) == 0xc7 && zapType == RIGHT) ||
                        (((uint8_t)bytes[12]) == 0xc8 && zapType == LEFT))
                    ) {
                    if(zapType == LEFT) {
                        if(DEBOUNCE) {
                            risingEdge = true;
                            if(!zwiftplay_swap)
                                emit plus();
                            else
                                emit minus();
                        }
                    } else {
                        if(DEBOUNCE) {
                            risingEdge = true;
                            if(!zwiftplay_swap)
                                emit minus();
                            else
                                emit plus();
                        }
                    }
                } else if(bytes.length() > 19 && ((uint8_t)bytes[18]) == 0xc8) {
                    if(DEBOUNCE) {
                        risingEdge = true;
                        if(!zwiftplay_swap)
                            emit plus();
                        else
                            emit minus();
                    }
                } else if(bytes.length() > 3 &&
                          ((((uint8_t)bytes[3]) == 0xdf) || // right top button
                          (((uint8_t)bytes[3]) == 0xbf))) { // right bottom button
                    if(DEBOUNCE) {
                        risingEdge = true;
                        if(!zwiftplay_swap)
                            emit plus();
                        else
                            emit minus();
                    }
                } else if(bytes.length() > 3 &&
                          ((((uint8_t)bytes[3]) == 0xfd) || // left top button
                          (((uint8_t)bytes[3]) == 0xfb))) { // left bottom button
                    if(DEBOUNCE) {
                        risingEdge = true;
                        if(!zwiftplay_swap)
                            emit minus();
                        else
                            emit plus();
                    }
                } else if(bytes.length() > 5 &&
                          ((((uint8_t)bytes[4]) == 0xfd) || // left top button
                          (((uint8_t)bytes[4]) == 0xfb))) { // left bottom button
                    if(DEBOUNCE) {
                        risingEdge = true;
                        if(!zwiftplay_swap)
                            emit minus();
                        else
                            emit plus();
                    }   
                } else {
                    risingEdge = false;
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
    static volatile bool risingEdge;

signals:
    void plus();
    void minus();
};

#endif // ABSTRACTZAPDEVICE_H
