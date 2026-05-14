#ifndef ABSTRACTZAPDEVICE_H
#define ABSTRACTZAPDEVICE_H

#include <QByteArray>
#include <QString>
#include <QDebug>
#include <QSettings>
#include <QTimer>
#include <QDateTime>
#include <QtGlobal>
#if 0
#include "zapConstants.h"
#include "localKeyProvider.h"
#include "zapCrypto.h"
#include "encryptionUtils.h"
#endif
#include "zapConstants.h"
#include "controllerNotification.h"
#include "qzsettings.h"

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

        qDebug() << zapType << characteristicName << bytes.toHex() << zwiftplay_swap << gears_volume_debouncing << risingEdge << lastFrame;

#define DEBOUNCE (!gears_volume_debouncing || risingEdge <= 0)

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
                            autoRepeatTimer->start();
                        }
                        else {
                            emit minus();
                            lastButtonPlus = false;
                            autoRepeatTimer->start();
                        }
                    }
                } else if(bytes[4] == 0) {
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
            processPlainControllerNotification(bytes);
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
            } else if(bytes.length() > 14 && bytes[11] == 0x30 && bytes[12] == 0x00) {
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
            if (processRideControllerNotification(bytes, zwiftplay_swap, DEBOUNCE)) {
                risingEdge--;
                if(risingEdge < 0)
                    risingEdge = 0;
                if(risingEdge == 0)
                    autoRepeatTimer->stop();
                break;
            }
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
            } else if(bytes.length() > 3 &&
                       ((((uint8_t)bytes[3]) == 0xdf) || // right top button
                        (((uint8_t)bytes[3]) == 0xbf))) { // right bottom button
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
            } else if(bytes.length() > 3 &&
                       ((((uint8_t)bytes[3]) == 0xfd) || // left top button
                        (((uint8_t)bytes[3]) == 0xfb))) { // left bottom button
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
            } else if(bytes.length() > 5 &&
                       ((((uint8_t)bytes[4]) == 0xfd) || // left top button
                        (((uint8_t)bytes[4]) == 0xfb))) { // left bottom button
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
    }

    QByteArray buildHandshakeStart() {
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
    virtual int processEncryptedData(const QByteArray& bytes) = 0;

  private:
    struct RideButtonState {
        bool leftUp = false;
        bool leftDown = false;
        bool leftLeft = false;
        bool leftRight = false;
        bool rightY = false;
        bool rightA = false;
        bool rightB = false;
        bool rightZAlt = false;
        bool rightZ = false;
        bool leftShiftUp = false;
        bool leftShiftDown = false;
        bool leftPower = false;
        bool leftPowerUp = false;
        bool leftOnOff = false;
        bool rightShiftUp = false;
        bool rightShiftDown = false;
        bool rightPower = false;
        bool rightPowerUp = false;
        bool rightOnOff = false;
    };

    struct RideAnalogState {
        bool leftPaddle = false;
        bool rightPaddle = false;
    };

    bool processRideControllerNotification(const QByteArray &bytes, bool zwiftplay_swap, bool debounce) {
        quint32 buttonMap = 0xffffffff;
        bool buttonMapFound = false;
        RideAnalogState analogState;
        int index = 1; // Skip the 0x23 command byte.

        while (index < bytes.size()) {
            quint64 tag = 0;
            if (!readVarInt(bytes, index, tag)) {
                break;
            }

            const int wireType = static_cast<int>(tag & 0x7);
            const int fieldNumber = static_cast<int>(tag >> 3);

            if (wireType == 0) {
                quint64 rawValue = 0;
                if (!readVarInt(bytes, index, rawValue)) {
                    break;
                }
                if (fieldNumber == 1) {
                    buttonMap = static_cast<quint32>(rawValue);
                    buttonMapFound = true;
                }
            } else if (wireType == 2) {
                quint64 length = 0;
                if (!readVarInt(bytes, index, length)) {
                    break;
                }
                if (length > static_cast<quint64>(bytes.size() - index)) {
                    break;
                }
                const int groupStart = index;
                const int groupLength = static_cast<int>(length);
                if (fieldNumber == 2 || fieldNumber == 3) {
                    processRideAnalogKeyGroup(bytes.mid(groupStart, groupLength), analogState);
                }
                index += groupLength;
            } else {
                break;
            }
        }

        if (!buttonMapFound) {
            return false;
        }

        const auto isPressed = [buttonMap](quint32 mask) {
            return (buttonMap & mask) == 0;
        };

        RideButtonState current;
        current.leftLeft = isPressed(0x00001);
        current.leftUp = isPressed(0x00002);
        current.leftRight = isPressed(0x00004);
        current.leftDown = isPressed(0x00008);
        current.rightA = isPressed(0x00010);
        current.rightB = isPressed(0x00020);
        current.rightY = isPressed(0x00040);
        current.rightZ = isPressed(0x00080);
        current.leftShiftUp = isPressed(0x00100);
        current.leftShiftDown = isPressed(0x00200);
        current.leftPowerUp = isPressed(0x00400);
        current.leftPower = isPressed(0x00800);
        current.rightShiftUp = isPressed(0x01000);
        current.rightShiftDown = isPressed(0x02000);
        current.rightPowerUp = isPressed(0x04000);
        current.rightPower = isPressed(0x08000);
        current.rightOnOff = isPressed(0x20000);

        if (!lastRideButtonStateValid) {
            qDebug() << rideButtonStateToString(current);
        } else {
            const QString diff = rideButtonStateDiff(current, lastRideButtonState);
            if (!diff.isEmpty()) {
                qDebug() << diff;
            }
        }

        emitIfChanged(current.leftUp, lastRideButtonStateValid ? lastRideButtonState.leftUp : false, &AbstractZapDevice::leftUp);
        emitIfChanged(current.leftDown, lastRideButtonStateValid ? lastRideButtonState.leftDown : false, &AbstractZapDevice::leftDown);
        emitIfChanged(current.leftLeft, lastRideButtonStateValid ? lastRideButtonState.leftLeft : false, &AbstractZapDevice::leftLeft);
        emitIfChanged(current.leftRight, lastRideButtonStateValid ? lastRideButtonState.leftRight : false, &AbstractZapDevice::leftRight);
        emitIfChanged(current.leftShiftUp, lastRideButtonStateValid ? lastRideButtonState.leftShiftUp : false, &AbstractZapDevice::rideLeftShiftUp);
        emitIfChanged(current.leftShiftDown, lastRideButtonStateValid ? lastRideButtonState.leftShiftDown : false, &AbstractZapDevice::rideLeftShiftDown);
        emitIfChanged(current.leftPower, lastRideButtonStateValid ? lastRideButtonState.leftPower : false, &AbstractZapDevice::rideLeftPower);
        emitIfChanged(current.leftPowerUp, lastRideButtonStateValid ? lastRideButtonState.leftPowerUp : false, &AbstractZapDevice::rideLeftPowerUp);
        emitIfChanged(current.leftOnOff, lastRideButtonStateValid ? lastRideButtonState.leftOnOff : false, &AbstractZapDevice::rideLeftOnOff);
        emitIfChanged(current.rightY, lastRideButtonStateValid ? lastRideButtonState.rightY : false, &AbstractZapDevice::rightY);
        emitIfChanged(current.rightZAlt, lastRideButtonStateValid ? lastRideButtonState.rightZAlt : false, &AbstractZapDevice::rideRightZAlt);
        emitIfChanged(current.rightZ, lastRideButtonStateValid ? lastRideButtonState.rightZ : false, &AbstractZapDevice::rightZ);
        emitIfChanged(current.rightA, lastRideButtonStateValid ? lastRideButtonState.rightA : false, &AbstractZapDevice::rightA);
        emitIfChanged(current.rightB, lastRideButtonStateValid ? lastRideButtonState.rightB : false, &AbstractZapDevice::rightB);
        emitIfChanged(current.rightShiftUp, lastRideButtonStateValid ? lastRideButtonState.rightShiftUp : false, &AbstractZapDevice::rideRightShiftUp);
        emitIfChanged(current.rightShiftDown, lastRideButtonStateValid ? lastRideButtonState.rightShiftDown : false, &AbstractZapDevice::rideRightShiftDown);
        emitIfChanged(current.rightPower, lastRideButtonStateValid ? lastRideButtonState.rightPower : false, &AbstractZapDevice::rideRightPower);
        emitIfChanged(current.rightPowerUp, lastRideButtonStateValid ? lastRideButtonState.rightPowerUp : false, &AbstractZapDevice::rideRightPowerUp);
        emitIfChanged(current.rightOnOff, lastRideButtonStateValid ? lastRideButtonState.rightOnOff : false, &AbstractZapDevice::rideRightOnOff);

        processRideShiftGear(current, zwiftplay_swap, debounce);
        processRideAnalogState(analogState, zwiftplay_swap, debounce);

        lastRideButtonState = current;
        lastRideButtonStateValid = true;
        return true;
    }

    void processRideAnalogKeyGroup(const QByteArray &message, RideAnalogState &state) {
        int index = 0;
        int location = -1;
        qint32 analogValue = 0;
        bool hasLocation = false;
        bool hasAnalogValue = false;

        while (index < message.size()) {
            quint64 tag = 0;
            if (!readVarInt(message, index, tag)) {
                return;
            }

            const int wireType = static_cast<int>(tag & 0x7);
            const int fieldNumber = static_cast<int>(tag >> 3);

            if (wireType == 0) {
                quint64 rawValue = 0;
                if (!readVarInt(message, index, rawValue)) {
                    return;
                }
                if (fieldNumber == 1) {
                    location = static_cast<int>(rawValue);
                    hasLocation = true;
                } else if (fieldNumber == 2) {
                    analogValue = decodeZigZag32(rawValue);
                    hasAnalogValue = true;
                }
            } else if (wireType == 2) {
                quint64 length = 0;
                if (!readVarInt(message, index, length) || length > static_cast<quint64>(message.size() - index)) {
                    return;
                }
                index += static_cast<int>(length);
            } else {
                return;
            }
        }

        if (!hasLocation || !hasAnalogValue) {
            return;
        }

        const bool pressed = qAbs(analogValue) >= 90;
        if (location == 0) {
            state.leftPaddle = pressed;
        } else if (location == 1) {
            state.rightPaddle = pressed;
        }
    }

    void processRideAnalogState(const RideAnalogState &state, bool zwiftplay_swap, bool debounce) {
        const bool previousLeftPaddle = lastRideAnalogStateValid ? lastRideAnalogState.leftPaddle : false;
        const bool previousRightPaddle = lastRideAnalogStateValid ? lastRideAnalogState.rightPaddle : false;

        if (state.leftPaddle != previousLeftPaddle) {
            emit leftPaddle(state.leftPaddle ? -100 : 0);
        }
        if (state.rightPaddle != previousRightPaddle) {
            emit rightPaddle(state.rightPaddle ? 100 : 0);
        }

        if (debounce && state.leftPaddle && !previousLeftPaddle) {
            emitRidePaddleGear(false, zwiftplay_swap);
        }
        if (debounce && state.rightPaddle && !previousRightPaddle) {
            emitRidePaddleGear(true, zwiftplay_swap);
        }

        lastRideAnalogState = state;
        lastRideAnalogStateValid = true;
    }

    void processRideShiftGear(const RideButtonState &state, bool zwiftplay_swap, bool debounce) {
        if (!debounce) {
            return;
        }

        const bool previousLeftShiftUp = lastRideButtonStateValid ? lastRideButtonState.leftShiftUp : false;
        const bool previousLeftShiftDown = lastRideButtonStateValid ? lastRideButtonState.leftShiftDown : false;
        const bool previousRightShiftUp = lastRideButtonStateValid ? lastRideButtonState.rightShiftUp : false;
        const bool previousRightShiftDown = lastRideButtonStateValid ? lastRideButtonState.rightShiftDown : false;

        if ((state.leftShiftUp && !previousLeftShiftUp) || (state.leftShiftDown && !previousLeftShiftDown)) {
            emitRidePaddleGear(false, zwiftplay_swap);
        }
        if ((state.rightShiftUp && !previousRightShiftUp) || (state.rightShiftDown && !previousRightShiftDown)) {
            emitRidePaddleGear(true, zwiftplay_swap);
        }
    }

    void emitRidePaddleGear(bool plusAction, bool zwiftplay_swap) {
        risingEdge = 2;
        const bool emitPlus = zwiftplay_swap ? !plusAction : plusAction;
        if (emitPlus) {
            emit plus();
        } else {
            emit minus();
        }
        lastButtonPlus = emitPlus;
        autoRepeatTimer->start();
    }

    bool processPlainControllerNotification(const QByteArray &bytes) {
        ControllerNotification bestCandidate;
        int bestScore = 0;

        for (int offset = 1; offset < bytes.size() && offset < 20; ++offset) {
            ControllerNotification candidate(bytes.mid(offset));
            int score = candidate.fieldsSeen();
            if (candidate.hasActiveInput()) {
                score += 10;
            }
            if (score > bestScore) {
                bestCandidate = candidate;
                bestScore = score;
            }
        }

        if (bestScore == 0 || !bestCandidate.hasParsedFields()) {
            return false;
        }

        processButtonNotification(bestCandidate);
        return true;
    }

    static bool readVarInt(const QByteArray &message, int &index, quint64 &value) {
        value = 0;
        int shift = 0;
        while (index < message.size() && shift < 64) {
            const quint8 byte = static_cast<quint8>(message.at(index++));
            value |= static_cast<quint64>(byte & 0x7F) << shift;
            if ((byte & 0x80) == 0) {
                return true;
            }
            shift += 7;
        }
        return false;
    }

    static qint32 decodeZigZag32(quint64 value) {
        return static_cast<qint32>((value >> 1) ^ -static_cast<qint64>(value & 1));
    }

    static QString rideButtonStateToString(const RideButtonState &state) {
        QString text = QStringLiteral("RideButtonState(");
        text += state.leftUp ? QStringLiteral("Left Up ") : QString();
        text += state.leftDown ? QStringLiteral("Left Down ") : QString();
        text += state.leftLeft ? QStringLiteral("Left Left ") : QString();
        text += state.leftRight ? QStringLiteral("Left Right ") : QString();
        text += state.rightY ? QStringLiteral("Right Y ") : QString();
        text += state.rightZAlt ? QStringLiteral("Right Z Alt ") : QString();
        text += state.rightZ ? QStringLiteral("Right Z ") : QString();
        text += state.rightA ? QStringLiteral("Right A ") : QString();
        text += state.rightB ? QStringLiteral("Right B ") : QString();
        text += state.leftShiftUp ? QStringLiteral("Left Shift Up ") : QString();
        text += state.leftShiftDown ? QStringLiteral("Left Shift Down ") : QString();
        text += state.leftPower ? QStringLiteral("Left Power ") : QString();
        text += state.leftPowerUp ? QStringLiteral("Left Power Up ") : QString();
        text += state.leftOnOff ? QStringLiteral("Left On/Off ") : QString();
        text += state.rightShiftUp ? QStringLiteral("Right Shift Up ") : QString();
        text += state.rightShiftDown ? QStringLiteral("Right Shift Down ") : QString();
        text += state.rightPower ? QStringLiteral("Right Power ") : QString();
        text += state.rightPowerUp ? QStringLiteral("Right Power Up ") : QString();
        text += state.rightOnOff ? QStringLiteral("Right On/Off ") : QString();
        text += QStringLiteral(")");
        return text;
    }

    static QString rideButtonStateDiff(const RideButtonState &current, const RideButtonState &previous) {
        QString diff;
        diff += diffField(QStringLiteral("Left Up"), current.leftUp, previous.leftUp);
        diff += diffField(QStringLiteral("Left Down"), current.leftDown, previous.leftDown);
        diff += diffField(QStringLiteral("Left Left"), current.leftLeft, previous.leftLeft);
        diff += diffField(QStringLiteral("Left Right"), current.leftRight, previous.leftRight);
        diff += diffField(QStringLiteral("Right Y"), current.rightY, previous.rightY);
        diff += diffField(QStringLiteral("Right Z Alt"), current.rightZAlt, previous.rightZAlt);
        diff += diffField(QStringLiteral("Right Z"), current.rightZ, previous.rightZ);
        diff += diffField(QStringLiteral("Right A"), current.rightA, previous.rightA);
        diff += diffField(QStringLiteral("Right B"), current.rightB, previous.rightB);
        diff += diffField(QStringLiteral("Left Shift Up"), current.leftShiftUp, previous.leftShiftUp);
        diff += diffField(QStringLiteral("Left Shift Down"), current.leftShiftDown, previous.leftShiftDown);
        diff += diffField(QStringLiteral("Left Power"), current.leftPower, previous.leftPower);
        diff += diffField(QStringLiteral("Left Power Up"), current.leftPowerUp, previous.leftPowerUp);
        diff += diffField(QStringLiteral("Left On/Off"), current.leftOnOff, previous.leftOnOff);
        diff += diffField(QStringLiteral("Right Shift Up"), current.rightShiftUp, previous.rightShiftUp);
        diff += diffField(QStringLiteral("Right Shift Down"), current.rightShiftDown, previous.rightShiftDown);
        diff += diffField(QStringLiteral("Right Power"), current.rightPower, previous.rightPower);
        diff += diffField(QStringLiteral("Right Power Up"), current.rightPowerUp, previous.rightPowerUp);
        diff += diffField(QStringLiteral("Right On/Off"), current.rightOnOff, previous.rightOnOff);
        return diff;
    }

    static QString diffField(const QString &title, bool current, bool previous) {
        return current != previous ? title + QStringLiteral("=") + (current ? QStringLiteral("Pressed ") : QStringLiteral("Released ")) : QString();
    }

    void processButtonNotification(const ControllerNotification& notification) {
        ControllerNotification &lastButtonState = notification.isRightController() ? lastRightButtonState : lastLeftButtonState;
        bool &lastButtonStateValid = notification.isRightController() ? lastRightButtonStateValid : lastLeftButtonStateValid;

        if (!lastButtonStateValid) {
            qDebug() << notification.toString();
        } else {
            QString diff = notification.diff(lastButtonState);
            if (!diff.isEmpty())
                qDebug() << diff;
        }

        const bool paddlePressed = qAbs(notification.steerBrakeValue()) >= 100;
        const bool previousPaddlePressed = lastButtonStateValid && qAbs(lastButtonState.steerBrakeValue()) >= 100;

        if (notification.isRightController()) {
            emitIfChanged(notification.buttonYPressed(), lastButtonStateValid ? lastButtonState.buttonYPressed() : false, &AbstractZapDevice::rightY);
            emitIfChanged(notification.buttonZPressed(), lastButtonStateValid ? lastButtonState.buttonZPressed() : false, &AbstractZapDevice::rightZ);
            emitIfChanged(notification.buttonAPressed(), lastButtonStateValid ? lastButtonState.buttonAPressed() : false, &AbstractZapDevice::rightA);
            emitIfChanged(notification.buttonBPressed(), lastButtonStateValid ? lastButtonState.buttonBPressed() : false, &AbstractZapDevice::rightB);
            emitIfChanged(notification.shoulderButtonPressed(), lastButtonStateValid ? lastButtonState.shoulderButtonPressed() : false, &AbstractZapDevice::rightShoulder);
            emitIfChanged(notification.powerButtonPressed(), lastButtonStateValid ? lastButtonState.powerButtonPressed() : false, &AbstractZapDevice::rightPower);
            if (paddlePressed != previousPaddlePressed) {
                emit rightPaddle(paddlePressed ? 100 : 0);
            }
        } else {
            emitIfChanged(notification.buttonYPressed(), lastButtonStateValid ? lastButtonState.buttonYPressed() : false, &AbstractZapDevice::leftUp);
            emitIfChanged(notification.buttonZPressed(), lastButtonStateValid ? lastButtonState.buttonZPressed() : false, &AbstractZapDevice::leftLeft);
            emitIfChanged(notification.buttonAPressed(), lastButtonStateValid ? lastButtonState.buttonAPressed() : false, &AbstractZapDevice::leftRight);
            emitIfChanged(notification.buttonBPressed(), lastButtonStateValid ? lastButtonState.buttonBPressed() : false, &AbstractZapDevice::leftDown);
            emitIfChanged(notification.shoulderButtonPressed(), lastButtonStateValid ? lastButtonState.shoulderButtonPressed() : false, &AbstractZapDevice::leftShoulder);
            emitIfChanged(notification.powerButtonPressed(), lastButtonStateValid ? lastButtonState.powerButtonPressed() : false, &AbstractZapDevice::leftPower);
            if (paddlePressed != previousPaddlePressed) {
                emit leftPaddle(paddlePressed ? -100 : 0);
            }
        }

        lastButtonState = notification;
        lastButtonStateValid = true;
    }

    bool emitIfChanged(bool current, bool previous, void (AbstractZapDevice::*signal)(bool)) {
        if (current == previous) {
            return false;
        }
        (this->*signal)(current);
        return true;
    }

    static volatile int8_t risingEdge;
    static QTimer* autoRepeatTimer;    // Static timer for auto-repeat
    static bool lastButtonPlus;  // Static track of which button was last pressed
    static QDateTime lastFrame;
    ControllerNotification lastLeftButtonState;
    ControllerNotification lastRightButtonState;
    bool lastLeftButtonStateValid = false;
    bool lastRightButtonStateValid = false;
    RideButtonState lastRideButtonState;
    bool lastRideButtonStateValid = false;
    RideAnalogState lastRideAnalogState;
    bool lastRideAnalogStateValid = false;

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
    void leftUp(bool pressed);
    void leftDown(bool pressed);
    void leftLeft(bool pressed);
    void leftRight(bool pressed);
    void leftShoulder(bool pressed);
    void leftPower(bool pressed);
    void leftPaddle(int value);
    void rideLeftShiftUp(bool pressed);
    void rideLeftShiftDown(bool pressed);
    void rideLeftPower(bool pressed);
    void rideLeftPowerUp(bool pressed);
    void rideLeftOnOff(bool pressed);
    void rightY(bool pressed);
    void rightZ(bool pressed);
    void rightA(bool pressed);
    void rightB(bool pressed);
    void rightShoulder(bool pressed);
    void rightPower(bool pressed);
    void rightPaddle(int value);
    void rideRightZAlt(bool pressed);
    void rideRightShiftUp(bool pressed);
    void rideRightShiftDown(bool pressed);
    void rideRightPower(bool pressed);
    void rideRightPowerUp(bool pressed);
    void rideRightOnOff(bool pressed);
};

#endif // ABSTRACTZAPDEVICE_H
