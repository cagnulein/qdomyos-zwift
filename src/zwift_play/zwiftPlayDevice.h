#ifndef ZWIFTPLAYDEVICE_H
#define ZWIFTPLAYDEVICE_H

#include <QByteArray>
#include <QDebug>
#include "abstractZapDevice.h"
#include "zapConstants.h"
#include "controllerNotification.h"

// ****************************************************************************************
// THIS CLASS IS USELESS SINCE I'M DOING EVERYTHING ON THE ANDROID, SWIFT NATIVE CODE
// ****************************************************************************************

class ZwiftPlayDevice : public AbstractZapDevice {
    Q_OBJECT
public:
    ZwiftPlayDevice() : batteryLevel(0) {}

protected:
    int processEncryptedData(const QByteArray& bytes) override {
        try {
            qDebug() << "Encrypted:" << bytes.toHex();

            QByteArray counter = bytes.left(sizeof(int));
            QByteArray payload = bytes.mid(sizeof(int), bytes.length() - sizeof(int));

            QByteArray data = zapEncryption.decrypt(counter, payload);
            if (data.isEmpty()) {
                return 0;
            }
            qDebug() << data.toHex(' ') << counter.toHex(' ') << payload.toHex(' ');
            char type = data[0];
            QByteArray message = data.mid(1);

            switch (type) {
                case ZapConstants::CONTROLLER_NOTIFICATION_MESSAGE_TYPE:
                    processButtonNotification(ControllerNotification(message));
                    break;
                case ZapConstants::EMPTY_MESSAGE_TYPE:
                    qDebug() << "Empty Message";
                    break;
                case ZapConstants::BATTERY_LEVEL_TYPE:
                    break;
                case ZapConstants::CLICK_TYPE:
                case 0x37:
                    return processClickButtonNotification(data);
                default:
                    qDebug() << "Unprocessed - Type:" << static_cast<unsigned char>(type) << "Data:" << data.toHex();
                    break;
            }
        } catch (std::exception& ex) {
            qDebug() << "Decrypt failed:" << ex.what();
        }
        return 0;
    }

private:    
    void processButtonNotification(const ControllerNotification& notification) {
        ControllerNotification &lastButtonState = notification.isRightController() ? lastRightButtonState : lastLeftButtonState;
        bool &lastButtonStateValid = notification.isRightController() ? lastRightButtonStateValid : lastLeftButtonStateValid;

        if (!lastButtonStateValid) {
            qDebug() << notification.toString();
        } else {
            QString diff = notification.diff(lastButtonState);
            if (!diff.isEmpty()) // get repeats of the same state
                qDebug() << diff;
        }

        const bool paddlePressed = qAbs(notification.steerBrakeValue()) >= 100;
        const bool previousPaddlePressed = lastButtonStateValid && qAbs(lastButtonState.steerBrakeValue()) >= 100;

        if (notification.isRightController()) {
            emitIfChanged(notification.buttonYPressed(), lastButtonStateValid ? lastButtonState.buttonYPressed() : false, &ZwiftPlayDevice::rightY);
            emitIfChanged(notification.buttonZPressed(), lastButtonStateValid ? lastButtonState.buttonZPressed() : false, &ZwiftPlayDevice::rightZ);
            emitIfChanged(notification.buttonAPressed(), lastButtonStateValid ? lastButtonState.buttonAPressed() : false, &ZwiftPlayDevice::rightA);
            emitIfChanged(notification.buttonBPressed(), lastButtonStateValid ? lastButtonState.buttonBPressed() : false, &ZwiftPlayDevice::rightB);
            emitIfChanged(notification.shoulderButtonPressed(), lastButtonStateValid ? lastButtonState.shoulderButtonPressed() : false, &ZwiftPlayDevice::rightShoulder);
            emitIfChanged(notification.powerButtonPressed(), lastButtonStateValid ? lastButtonState.powerButtonPressed() : false, &ZwiftPlayDevice::rightPower);
            if (paddlePressed != previousPaddlePressed) {
                emit rightPaddle(paddlePressed ? 100 : 0);
            }
        } else {
            emitIfChanged(notification.buttonYPressed(), lastButtonStateValid ? lastButtonState.buttonYPressed() : false, &ZwiftPlayDevice::leftUp);
            emitIfChanged(notification.buttonZPressed(), lastButtonStateValid ? lastButtonState.buttonZPressed() : false, &ZwiftPlayDevice::leftLeft);
            emitIfChanged(notification.buttonAPressed(), lastButtonStateValid ? lastButtonState.buttonAPressed() : false, &ZwiftPlayDevice::leftRight);
            emitIfChanged(notification.buttonBPressed(), lastButtonStateValid ? lastButtonState.buttonBPressed() : false, &ZwiftPlayDevice::leftDown);
            emitIfChanged(notification.shoulderButtonPressed(), lastButtonStateValid ? lastButtonState.shoulderButtonPressed() : false, &ZwiftPlayDevice::leftShoulder);
            emitIfChanged(notification.powerButtonPressed(), lastButtonStateValid ? lastButtonState.powerButtonPressed() : false, &ZwiftPlayDevice::leftPower);
            if (paddlePressed != previousPaddlePressed) {
                emit leftPaddle(paddlePressed ? -100 : 0);
            }
        }

        lastButtonState = notification;
        lastButtonStateValid = true;
    }

    int processClickButtonNotification(const QByteArray &data) {
        if (data.size() == 5) {
            if (data[4] == 0x00) {
                emit minus();
                return 2;
            }
            if (data[2] == 0x00) {
                emit plus();
                return 1;
            }
        }
        return 0;
    }

    bool emitIfChanged(bool current, bool previous, void (ZwiftPlayDevice::*signal)(bool)) {
        if (current == previous) {
            return false;
        }
        (this->*signal)(current);
        return true;
    }

    int batteryLevel;
    ControllerNotification lastLeftButtonState;
    ControllerNotification lastRightButtonState;
    bool lastLeftButtonStateValid = false;
    bool lastRightButtonStateValid = false;
};


#endif // ZWIFTPLAYDEVICE_H
