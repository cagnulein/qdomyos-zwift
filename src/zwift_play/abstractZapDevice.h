#ifndef ABSTRACTZAPDEVICE_H
#define ABSTRACTZAPDEVICE_H

#include <QByteArray>
#include <QString>
#include <QDebug>
#include <QSettings>
#include <QTimer>
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

    // Constructor now initializes the auto-repeat timer
    AbstractZapDevice() : autoRepeatTimer(new QTimer(this)) {
        RIDE_ON = QByteArray::fromRawData("\x52\x69\x64\x65\x4F\x6E", 6);  // "RideOn"
        REQUEST_START = QByteArray::fromRawData("\x00\x09", 2);  // {0, 9}
        RESPONSE_START = QByteArray::fromRawData("\x01\x03", 2);  // {1, 3}

        // Setup auto-repeat timer with 500ms interval
        autoRepeatTimer->setInterval(500);
        connect(autoRepeatTimer, &QTimer::timeout, this, &AbstractZapDevice::handleAutoRepeat);
    }

    int processCharacteristic(const QString& characteristicName, const QByteArray& bytes, ZWIFT_PLAY_TYPE zapType) {
        if (bytes.isEmpty()) return 0;

        QSettings settings;
        bool zwiftplay_swap = settings.value(QZSettings::zwiftplay_swap, QZSettings::default_zwiftplay_swap).toBool();

        qDebug() << zapType << characteristicName << bytes.toHex() << zwiftplay_swap << risingEdge;

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
        // Process different button press patterns
        switch(bytes[0]) {
        case 0x37:
            if(bytes.length() == 5) {
                if(bytes[2] == 0) {
                    // Button press detected, handle with auto-repeat
                    handleButtonPress(!zwiftplay_swap);
                } else if(bytes[4] == 0) {
                    handleButtonPress(zwiftplay_swap);
                } else {
                    // Button release detected, stop auto-repeat
                    handleButtonRelease();
                }
            }
            break;
        case 0x07: // zwift play
            if(bytes.length() > 5 && bytes[bytes.length() - 5] == 0x40 && (
                    (((uint8_t)bytes[bytes.length() - 4]) == 0xc7 && zapType == RIGHT) ||
                    (((uint8_t)bytes[bytes.length() - 4]) == 0xc8 && zapType == LEFT)
                    ) && bytes[bytes.length() - 3] == 0x01) {
                if(zapType == LEFT) {
                    handleButtonPress(!zwiftplay_swap);
                } else {
                    handleButtonPress(zwiftplay_swap);
                }
            } else if(bytes.length() > 14 && bytes[11] == 0x30 && bytes[12] == 0x00) {
                if(zapType == LEFT) {
                    handleButtonPress(!zwiftplay_swap);
                } else {
                    handleButtonPress(zwiftplay_swap);
                }
            } else {
                // No button press detected, handle as release
                handleButtonRelease();
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
                    handleButtonPress(!zwiftplay_swap);
                } else {
                    handleButtonPress(zwiftplay_swap);
                }
            } else if(bytes.length() > 19 && ((uint8_t)bytes[18]) == 0xc8) {
                handleButtonPress(!zwiftplay_swap);
            } else if(bytes.length() > 3 &&
                       ((((uint8_t)bytes[3]) == 0xdf) || // right top button
                        (((uint8_t)bytes[3]) == 0xbf))) { // right bottom button
                handleButtonPress(!zwiftplay_swap);
            } else if(bytes.length() > 3 &&
                       ((((uint8_t)bytes[3]) == 0xfd) || // left top button
                        (((uint8_t)bytes[3]) == 0xfb))) { // left bottom button
                handleButtonPress(zwiftplay_swap);
            } else if(bytes.length() > 5 &&
                       ((((uint8_t)bytes[4]) == 0xfd) || // left top button
                        (((uint8_t)bytes[4]) == 0xfb))) { // left bottom button
                handleButtonPress(zwiftplay_swap);
            } else {
                // No button press detected, handle as release
                handleButtonRelease();
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
            jsize length = QAndroidJniEnvironment()->GetArrayLength(result.object<jbyteArray>());
            jbyte* bytes = QAndroidJniEnvironment()->GetByteArrayElements(result.object<jbyteArray>(), nullptr);
            QByteArray byteArray(reinterpret_cast<char*>(bytes), length);
            QAndroidJniEnvironment()->ReleaseByteArrayElements(result.object<jbyteArray>(), bytes, JNI_ABORT);
            return byteArray;
        }
#endif
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
    static volatile int8_t risingEdge;

    // Timer for auto-repeat functionality
    QTimer* autoRepeatTimer;
    // Tracks which button is currently being held
    bool isPlus = false;

    // Handles initial button press and starts auto-repeat timer
    void handleButtonPress(bool plus) {
        if (!autoRepeatTimer->isActive()) {
            // Store which button was pressed
            isPlus = plus;
            // Emit initial button press event
            if (isPlus) {
                emit this->plus();
            } else {
                emit this->minus();
            }
            // Start auto-repeat timer
            autoRepeatTimer->start();
        }
    }

    // Handles button release by stopping auto-repeat
    void handleButtonRelease() {
        autoRepeatTimer->stop();
    }

  private slots:
    // Handles auto-repeat events when button is held
    void handleAutoRepeat() {
        // Emit appropriate signal based on which button is held
        if (isPlus) {
            emit plus();
        } else {
            emit minus();
        }
    }

  signals:
    void plus();
    void minus();
};

#endif // ABSTRACTZAPDEVICE_H
