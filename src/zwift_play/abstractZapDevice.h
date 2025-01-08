#ifndef ABSTRACTZAPDEVICE_H
#define ABSTRACTZAPDEVICE_H

#include <QByteArray>
#include <QString>
#include <QDebug>
#include <QSettings>
#include <QTimer>
#include <QDateTime>
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

    AbstractZapDevice() {
        RIDE_ON = QByteArray::fromRawData("\x52\x69\x64\x65\x4F\x6E", 6);  // "RideOn"
        REQUEST_START = QByteArray::fromRawData("\x00\x09", 2);  // {0, 9}
        RESPONSE_START = QByteArray::fromRawData("\x01\x03", 2);  // {1, 3}

               // Setup auto-repeat
        autoRepeatTimer = new QTimer();
        autoRepeatTimer->setInterval(500);
        connect(autoRepeatTimer, &QTimer::timeout, this, &AbstractZapDevice::handleAutoRepeat);
    }

    ~AbstractZapDevice() {
        if (autoRepeatTimer) {
            autoRepeatTimer->stop();
        }
    }

    int processCharacteristic(const QString& characteristicName, const QByteArray& bytes, ZWIFT_PLAY_TYPE zapType) {
        if (bytes.isEmpty()) return 0;

        QSettings settings;
        bool gears_volume_debouncing = settings.value(QZSettings::gears_volume_debouncing, QZSettings::default_gears_volume_debouncing).toBool();
        bool zwiftplay_swap = settings.value(QZSettings::zwiftplay_swap, QZSettings::default_zwiftplay_swap).toBool();
        int shift_style = settings.value(QZSettings::shift_style, QZSettings::default_shift_style).toInt();

        qDebug() << zapType << characteristicName << bytes.toHex() << zwiftplay_swap << gears_volume_debouncing << risingEdge << lastFrame;

#define DEBOUNCE (!gears_volume_debouncing || risingEdge <= 0)

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
            lastFrame = QDateTime::currentDateTime();
            if(bytes.length() == 5) {
                if(bytes[2] == 0) {
                    if(DEBOUNCE) {
                        risingEdge = 2;
                        if(!zwiftplay_swap) {
                            emit plus();
                            lastButtonPlus = true;
                        }
                        else {
                            emit minus();
                            lastButtonPlus = false;
                        }
                        autoRepeatTimer->start();
                    }
                } else if(bytes[4] == 0) {
                    if(DEBOUNCE) {
                        risingEdge = 2;
                        if(!zwiftplay_swap) {
                            emit minus();
                            lastButtonPlus = false;
                        }
                        else {
                            emit plus();
                            lastButtonPlus = true;
                        }
                        autoRepeatTimer->start();
                    }
                } else {
                    risingEdge--;
                    if(risingEdge < 0)
                        risingEdge = 0;
                    if(risingEdge == 0)
                        autoRepeatTimer->stop();
                }
            }
            break;

        case 0x07: // zwift play
            lastFrame = QDateTime::currentDateTime();
            if(bytes.length() > 5 && bytes[bytes.length() - 5] == 0x40 && (
                                                                               (((uint8_t)bytes[bytes.length() - 4]) == 0xc7 && zapType == RIGHT) ||
                                                                               (((uint8_t)bytes[bytes.length() - 4]) == 0xc8 && zapType == LEFT)
                                                                               ) && bytes[bytes.length() - 3] == 0x01) {
                if(zapType == LEFT) {
                    if(DEBOUNCE) {
                        risingEdge = 2;
                        if(!zwiftplay_swap) {
                            emit plus();
                            lastButtonPlus = true;
                        }
                        else {
                            emit minus();
                            lastButtonPlus = false;
                        }
                        autoRepeatTimer->start();
                    }
                } else {
                    if(DEBOUNCE) {
                        risingEdge = 2;
                        if(!zwiftplay_swap) {
                            emit minus();
                            lastButtonPlus = false;
                        }
                        else {
                            emit plus();
                            lastButtonPlus = true;
                        }
                        autoRepeatTimer->start();
                    }
                }
            } else if(bytes.length() > 14 && bytes[11] == 0x30 && bytes[12] == 0x00) {
                if(zapType == LEFT) {
                    if(DEBOUNCE) {
                        risingEdge = 2;
                        if(!zwiftplay_swap) {
                            emit plus();
                            lastButtonPlus = true;
                        }
                        else {
                            emit minus();
                            lastButtonPlus = false;
                        }
                        autoRepeatTimer->start();
                    }
                } else {
                    if(DEBOUNCE) {
                        risingEdge = 2;
                        if(!zwiftplay_swap) {
                            emit minus();
                            lastButtonPlus = false;
                        }
                        else {
                            emit plus();
                            lastButtonPlus = true;
                        }
                        autoRepeatTimer->start();
                    }
                }
            } else {
                risingEdge--;
                if(risingEdge < 0)
                    risingEdge = 0;
                if(risingEdge == 0)
                    autoRepeatTimer->stop();
            }
            break;
        case 0x15: // empty data
            qDebug() << "ignoring this frame";
            return 1;
        case 0x23: // zwift ride
            lastFrame = QDateTime::currentDateTime();
            if(bytes.length() > 12 &&
                ((((uint8_t)bytes[12]) == 0xc7 && zapType == RIGHT) ||
                 (((uint8_t)bytes[12]) == 0xc8 && zapType == LEFT))
                ) {
                if(zapType == LEFT) {
                    if(DEBOUNCE) {
                        risingEdge = 2;
                        if(!zwiftplay_swap) {
                            emit plus();
                            lastButtonPlus = true;
                            autoRepeatTimer->start();
                        }
                        else {
                            emit minus();
                            lastButtonPlus = false;
                            autoRepeatTimer->start();
                        }
                    }
                } else {
                    if(DEBOUNCE) {
                        risingEdge = 2;
                        if(!zwiftplay_swap) {
                            emit minus();
                            lastButtonPlus = false;
                            autoRepeatTimer->start();
                        }
                        else {
                            emit plus();
                            lastButtonPlus = true;
                            autoRepeatTimer->start();
                        }
                    }
                }
            } else if(bytes.length() > 19 && ((uint8_t)bytes[18]) == 0xc8) {
                if(DEBOUNCE) {
                    risingEdge = 2;
                    if(!zwiftplay_swap) {
                        emit plus();
                        lastButtonPlus = true;
                        autoRepeatTimer->start();
                    }
                    else {
                        emit minus();
                        lastButtonPlus = false;
                        autoRepeatTimer->start();
                    }
                }
            } else if(bytes.length() > 3) {
                bool isRightTop = ((uint8_t)bytes[3]) == 0xdf;
                bool isRightBottom = ((uint8_t)bytes[3]) == 0xbf;
                bool isLeftTop = ((uint8_t)bytes[3]) == 0xfd;
                bool isLeftBottom = ((uint8_t)bytes[3]) == 0xfb;

                if(DEBOUNCE && (isRightTop || isRightBottom || isLeftTop || isLeftBottom)) {
                    risingEdge = 2;
                    switch(shift_style) {
                    case QZSettings::SHIFT_STYLE_SEQUENTIAL:
                        if(isRightTop || isRightBottom) {
                            if(!zwiftplay_swap) {
                                emit plus();
                                lastButtonPlus = true;
                            } else {
                                emit minus();
                                lastButtonPlus = false;
                            }
                        } else {
                            if(!zwiftplay_swap) {
                                emit minus();
                                lastButtonPlus = false;
                            } else {
                                emit plus();
                                lastButtonPlus = true;
                            }
                        }
                        break;

                    case QZSettings::SHIFT_STYLE_SHIMANO_A:
                        if(isLeftTop || isLeftBottom)
                            emit isLeftTop ? chainRingUp() : chainRingDown();
                        else
                            emit isRightTop ? cassetteUp() : cassetteDown();
                        lastButtonPlus = isRightTop || isLeftTop;
                        break;

                    case QZSettings::SHIFT_STYLE_SHIMANO_B:
                        if(isLeftTop || isLeftBottom)
                            emit isLeftBottom ? chainRingUp() : chainRingDown();
                        else
                            emit isRightBottom ? cassetteUp() : cassetteDown();
                        lastButtonPlus = isRightBottom || isLeftBottom;
                        break;

                    case QZSettings::SHIFT_STYLE_SRAM:
                        if(isRightTop || isRightBottom)
                            emit cassetteUp();
                        else
                            emit cassetteDown();
                        lastButtonPlus = isRightTop || isRightBottom;
                        break;
                    }
                    autoRepeatTimer->start();
                }
            } else {
                risingEdge--;
                if(risingEdge < 0)
                    risingEdge = 0;
                if(risingEdge == 0)
                    autoRepeatTimer->stop();
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
    static QTimer* autoRepeatTimer;
    static bool lastButtonPlus;
    static QDateTime lastFrame;

  private slots:
    void handleAutoRepeat() {
        uint64_t delta = lastFrame.msecsTo(QDateTime::currentDateTime());
        qDebug() << "gear auto repeat" << lastButtonPlus << lastFrame << delta;
        if(delta > 400) {
            qDebug() << "stopping repeat timer";
            autoRepeatTimer->stop();
            return;
        }
        if(lastButtonPlus)
            emit plus();
        else
            emit minus();
    }

  signals:
    void plus();
    void minus();
    void chainRingUp();
    void chainRingDown();
    void cassetteUp();
    void cassetteDown();
};

#endif // ABSTRACTZAPDEVICE_H
