#ifndef CONTROLLERNOTIFICATION_H
#define CONTROLLERNOTIFICATION_H

#include <QByteArray>
#include <QString>
#include <QDebug>

class ControllerNotification {
public:
    static const char BTN_PRESSED = 0;

    ControllerNotification() = default;

    ControllerNotification(const QByteArray& message) {
        int index = 0;
        while (index < message.size()) {
            quint64 tag = 0;
            if (!readVarInt(message, index, tag)) {
                break;
            }
            const int wireType = static_cast<int>(tag & 0x7);
            if (tag == 0 || wireType == 4) {
                break;
            }
            const int fieldNumber = static_cast<int>(tag >> 3);
            if (wireType != 0) {
                break;
            }

            quint64 rawValue = 0;
            if (!readVarInt(message, index, rawValue)) {
                break;
            }

            const int value = static_cast<int>(rawValue);
            switch (fieldNumber) {
                case 1:
                    ++m_fieldsSeen;
                    m_isRightController = value == BTN_PRESSED;
                    break;
                case 2:
                    ++m_fieldsSeen;
                    m_buttonYPressed = value == BTN_PRESSED;
                    break;
                case 3:
                    ++m_fieldsSeen;
                    m_buttonZPressed = value == BTN_PRESSED;
                    break;
                case 4:
                    ++m_fieldsSeen;
                    m_buttonAPressed = value == BTN_PRESSED;
                    break;
                case 5:
                    ++m_fieldsSeen;
                    m_buttonBPressed = value == BTN_PRESSED;
                    break;
                case 6:
                    ++m_fieldsSeen;
                    m_shoulderButtonPressed = value == BTN_PRESSED;
                    break;
                case 7:
                    ++m_fieldsSeen;
                    m_powerButtonPressed = value == BTN_PRESSED;
                    break;
                case 8:
                    ++m_fieldsSeen;
                    m_steerBrakeValue = decodeSignedValue(value);
                    break;
                case 9:
                    ++m_fieldsSeen;
                    m_somethingValue = value;
                    break;
                default:
                    break;
            }
        }
    }

    QString diff(const ControllerNotification& previousNotification) const {
        QString diff;
        diff += diffField(nameY(), m_buttonYPressed, previousNotification.m_buttonYPressed);
        diff += diffField(nameZ(), m_buttonZPressed, previousNotification.m_buttonZPressed);
        diff += diffField(nameA(), m_buttonAPressed, previousNotification.m_buttonAPressed);
        diff += diffField(nameB(), m_buttonBPressed, previousNotification.m_buttonBPressed);
        diff += diffField("Shoulder", m_shoulderButtonPressed, previousNotification.m_shoulderButtonPressed);
        diff += diffField("Power", m_powerButtonPressed, previousNotification.m_powerButtonPressed);
        diff += diffField("Steer/Brake", m_steerBrakeValue, previousNotification.m_steerBrakeValue);
        diff += diffField("???", m_somethingValue, previousNotification.m_somethingValue);
        return diff;
    }

    QString toString() const {
        QString text = "ControllerNotification(";

        text += nameController() + " ";
        text += m_buttonYPressed ? nameY() + " " : "";
        text += m_buttonZPressed ? nameZ() + " " : "";
        text += m_buttonAPressed ? nameA() + " " : "";
        text += m_buttonBPressed ? nameB() + " " : "";
        text += m_shoulderButtonPressed ? "Shoulder " : "";
        text += m_powerButtonPressed ? "Power " : "";
        text += m_steerBrakeValue != 0 ? QString("Steer/Brake: %1 ").arg(m_steerBrakeValue) : "";
        text += m_somethingValue != 0 ? QString("???: %1 ").arg(m_somethingValue) : "";

        text += ")";
        return text;
    }

    bool isRightController() const { return m_isRightController; }
    bool buttonYPressed() const { return m_buttonYPressed; }
    bool buttonZPressed() const { return m_buttonZPressed; }
    bool buttonAPressed() const { return m_buttonAPressed; }
    bool buttonBPressed() const { return m_buttonBPressed; }
    bool shoulderButtonPressed() const { return m_shoulderButtonPressed; }
    bool powerButtonPressed() const { return m_powerButtonPressed; }
    int steerBrakeValue() const { return m_steerBrakeValue; }
    int somethingValue() const { return m_somethingValue; }
    int fieldsSeen() const { return m_fieldsSeen; }
    bool hasParsedFields() const { return m_fieldsSeen > 0; }
    bool hasActiveInput() const {
        return m_buttonYPressed || m_buttonZPressed || m_buttonAPressed || m_buttonBPressed ||
               m_shoulderButtonPressed || m_powerButtonPressed || m_steerBrakeValue != 0 || m_somethingValue != 0;
    }

private:
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

    static int decodeSignedValue(int value) {
        const int negativeBit = value & 0x1;
        const int num = value >> 1;
        return negativeBit == 1 ? -num : num;
    }

    bool m_isRightController = false;
    bool m_buttonYPressed = false;
    bool m_buttonZPressed = false;
    bool m_buttonAPressed = false;
    bool m_buttonBPressed = false;
    bool m_shoulderButtonPressed = false;
    bool m_powerButtonPressed = false;
    int m_steerBrakeValue = 0;
    int m_somethingValue = 0;
    int m_fieldsSeen = 0;

    static QString diffField(const QString& title, bool newValue, bool oldValue) {
        return newValue != oldValue ? title + "=" + (newValue ? "Pressed " : "Released ") : "";
    }

    static QString diffField(const QString& title, int newValue, int oldValue) {
        return newValue != oldValue ? QString("%1=%2 ").arg(title).arg(newValue) : "";
    }

    QString nameController() const { return m_isRightController ? "Right" : "Left"; }
    QString nameY() const { return m_isRightController ? "Y" : "Up"; }
    QString nameZ() const { return m_isRightController ? "Z" : "Left"; }
    QString nameA() const { return m_isRightController ? "A" : "Right"; }
    QString nameB() const { return m_isRightController ? "B" : "Down"; }
};


#endif // CONTROLLERNOTIFICATION_H
