#ifndef ZAPCONSTANTS_H
#define ZAPCONSTANTS_H

#include <QByteArray>

class ZapConstants {
public:
    static constexpr int ZWIFT_MANUFACTURER_ID = 2378; // Zwift, Inc
    static constexpr char RC1_LEFT_SIDE = 3;
    static constexpr char RC1_RIGHT_SIDE = 2;

    static const QByteArray RIDE_ON;
    static const QByteArray REQUEST_START;
    static const QByteArray RESPONSE_START;

    static constexpr char CONTROLLER_NOTIFICATION_MESSAGE_TYPE = 7;
    static constexpr char EMPTY_MESSAGE_TYPE = 21;
    static constexpr char BATTERY_LEVEL_TYPE = 25;
};

// Definition of the static members outside the class
const QByteArray ZapConstants::RIDE_ON = QByteArray::fromRawData("\x52\x69\x64\x65\x4F\x6E", 6);  // "RideOn"
const QByteArray ZapConstants::REQUEST_START = QByteArray::fromRawData("\x00\x09", 2);  // {0, 9}
const QByteArray ZapConstants::RESPONSE_START = QByteArray::fromRawData("\x01\x03", 2);  // {1, 3}


#endif // ZAPCONSTANTS_H
