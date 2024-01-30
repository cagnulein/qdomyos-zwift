#ifndef ZWIFTPLAYDEVICE_H
#define ZWIFTPLAYDEVICE_H

#include <QByteArray>
#include <QDebug>
#include "abstractZapDevice.h"
#include "zapConstants.h"
#include "controllerNotification.h"

class ZwiftPlayDevice : public AbstractZapDevice {
public:
    ZwiftPlayDevice() : batteryLevel(0), lastButtonState(nullptr) {}

protected:
    void processEncryptedData(const QByteArray& bytes) override {
        try {
            qDebug() << "Decrypted:" << bytes.toHex();

            QByteArray counter = bytes.left(sizeof(int));
            const int MAC_LENGHT = 4;
            QByteArray payload = bytes.mid(sizeof(int), bytes.length() - MAC_LENGHT - sizeof(int));
            
            QByteArray data = zapEncryption.decrypt(counter, payload);
            qDebug() << data << counter << payload;
            char type = data[0];
            QByteArray message = data.mid(1);

            switch (type) {
                case ZapConstants::CONTROLLER_NOTIFICATION_MESSAGE_TYPE:
                    processButtonNotification(ControllerNotification(message));
                    break;
                case ZapConstants::EMPTY_MESSAGE_TYPE:
                    qDebug() << "Empty Message";
                    break;
                case ZapConstants::BATTERY_LEVEL_TYPE: {/*
                    BatteryStatus notification(message);
                    if (batteryLevel != notification.getLevel()) {
                        batteryLevel = notification.getLevel();
                        qDebug() << "Battery level update:" << batteryLevel;
                    }*/
                    break;
                }
                default:
                    qDebug() << "Unprocessed - Type:" << static_cast<unsigned char>(type) << "Data:" << data.toHex();
                    break;
            }
        } catch (std::exception& ex) {
            qDebug() << "Decrypt failed:" << ex.what();
        }
    }

private:
    void processButtonNotification(const ControllerNotification& notification) {
        if (!lastButtonState) {
            qDebug() << notification.toString();
        } else {
            QString diff = notification.diff(*lastButtonState);
            if (!diff.isEmpty()) // get repeats of the same state
                qDebug() << diff;
        }
        lastButtonState = std::make_unique<ControllerNotification>(notification);
    }

    int batteryLevel;
    std::unique_ptr<ControllerNotification> lastButtonState;
};


#endif // ZWIFTPLAYDEVICE_H
