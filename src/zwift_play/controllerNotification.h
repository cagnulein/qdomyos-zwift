#ifndef CONTROLLERNOTIFICATION_H
#define CONTROLLERNOTIFICATION_H

#include <QByteArray>
#include <QString>
#include <QDebug>

class ControllerNotification {
public:
    static const char BTN_PRESSED = 0;

    ControllerNotification(const QByteArray& message) {
        // Replace with actual parsing logic according to your message format
        if (message.size() >= 9) {
            isRightController = message[0] == BTN_PRESSED;
            buttonYPressed = message[1] == BTN_PRESSED;
            buttonZPressed = message[2] == BTN_PRESSED;
            buttonAPressed = message[3] == BTN_PRESSED;
            buttonBPressed = message[4] == BTN_PRESSED;
            shoulderButtonPressed = message[5] == BTN_PRESSED;
            powerButtonPressed = message[6] == BTN_PRESSED;
            steerBrakeValue = static_cast<int>(message[7]); // Assuming single-byte value
            somethingValue = static_cast<int>(message[8]); // Assuming single-byte value
        }
    }

    QString diff(const ControllerNotification& previousNotification) const {
        QString diff;
        diff += diffField(nameY(), buttonYPressed, previousNotification.buttonYPressed);
        diff += diffField(nameZ(), buttonZPressed, previousNotification.buttonZPressed);
        diff += diffField(nameA(), buttonAPressed, previousNotification.buttonAPressed);
        diff += diffField(nameB(), buttonBPressed, previousNotification.buttonBPressed);
        diff += diffField("Shoulder", shoulderButtonPressed, previousNotification.shoulderButtonPressed);
        diff += diffField("Power", powerButtonPressed, previousNotification.powerButtonPressed);
        diff += diffField("Steer/Brake", steerBrakeValue, previousNotification.steerBrakeValue);
        diff += diffField("???", somethingValue, previousNotification.somethingValue);
        return diff;
    }

    QString toString() const {
        QString text = "ControllerNotification(";

        text += nameController() + " ";
        text += buttonYPressed ? nameY() + " " : "";
        text += buttonZPressed ? nameZ() + " " : "";
        text += buttonAPressed ? nameA() + " " : "";
        text += buttonBPressed ? nameB() + " " : "";
        text += shoulderButtonPressed ? "Shoulder " : "";
        text += powerButtonPressed ? "Power " : "";
        text += steerBrakeValue != 0 ? QString("Steer/Brake: %1 ").arg(steerBrakeValue) : "";
        text += somethingValue != 0 ? QString("???: %1 ").arg(somethingValue) : "";

        text += ")";
        return text;
    }

private:
    bool isRightController = false;
    bool buttonYPressed = false;
    bool buttonZPressed = false;
    bool buttonAPressed = false;
    bool buttonBPressed = false;
    bool shoulderButtonPressed = false;
    bool powerButtonPressed = false;
    int steerBrakeValue = 0;
    int somethingValue = 0;

    static QString diffField(const QString& title, bool newValue, bool oldValue) {
        return newValue != oldValue ? title + "=" + (newValue ? "Pressed " : "Released ") : "";
    }

    static QString diffField(const QString& title, int newValue, int oldValue) {
        return newValue != oldValue ? QString("%1=%2 ").arg(title).arg(newValue) : "";
    }

    QString nameController() const { return isRightController ? "Right" : "Left"; }
    QString nameY() const { return isRightController ? "Y" : "Up"; }
    QString nameZ() const { return isRightController ? "Z" : "Left"; }
    QString nameA() const { return isRightController ? "A" : "Right"; }
    QString nameB() const { return isRightController ? "B" : "Down"; }
};


#endif // CONTROLLERNOTIFICATION_H
