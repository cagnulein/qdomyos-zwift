#ifndef ZAPCONSTANTS_H
#define ZAPCONSTANTS_H

#include <QByteArray>

class ZapConstants {
public:
    static constexpr int ZWIFT_MANUFACTURER_ID = 2378; // Zwift, Inc
    static constexpr char RC1_LEFT_SIDE = 3;
    static constexpr char RC1_RIGHT_SIDE = 2;

    static constexpr char CONTROLLER_NOTIFICATION_MESSAGE_TYPE = 7;
    static constexpr char EMPTY_MESSAGE_TYPE = 21;
    static constexpr char BATTERY_LEVEL_TYPE = 25;
};

#endif // ZAPCONSTANTS_H
