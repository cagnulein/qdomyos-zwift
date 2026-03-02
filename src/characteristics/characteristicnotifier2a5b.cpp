#include "characteristicnotifier2a5b.h"
#include <QSettings>

CharacteristicNotifier2A5B::CharacteristicNotifier2A5B(bluetoothdevice *Bike, QObject *parent)
    : CharacteristicNotifier(0x2a5b, parent), Bike(Bike) {
    QSettings settings;
    bike_wheel_revs = settings.value(QZSettings::bike_wheel_revs, QZSettings::default_bike_wheel_revs).toBool();
}

int CharacteristicNotifier2A5B::notify(QByteArray &value) {
    if (!bike_wheel_revs) {
        value.append((char)0x02); // crank data present
    } else {

        value.append((char)0x03); // crank and wheel data present

        if (Bike->currentSpeed().value()) {

            const double wheelCircumference = 2000.0; // millimeters
            wheelRevs++;
            lastWheelTime +=
                (uint16_t)(1024.0 / ((Bike->currentSpeed().value() / 3.6) / (wheelCircumference / 1000.0)));
        }
        value.append((char)((wheelRevs & 0xFF)));        // wheel count
        value.append((char)((wheelRevs >> 8) & 0xFF));   // wheel count
        value.append((char)((wheelRevs >> 16) & 0xFF));  // wheel count
        value.append((char)((wheelRevs >> 24) & 0xFF));  // wheel count
        value.append((char)(lastWheelTime & 0xff));      // eventtime
        value.append((char)(lastWheelTime >> 8) & 0xFF); // eventtime
    }
    value.append((char)(((uint16_t)Bike->currentCrankRevolutions()) & 0xFF));      // revs count
    value.append((char)(((uint16_t)Bike->currentCrankRevolutions()) >> 8) & 0xFF); // revs count
    value.append((char)(Bike->lastCrankEventTime() & 0xff));                       // eventtime
    value.append((char)(Bike->lastCrankEventTime() >> 8) & 0xFF);                  // eventtime
    return CN_OK;
}
