#include "characteristicnotifier2ad2.h"
#include "devices/elliptical.h"
#include "devices/rower.h"
#include "devices/treadmill.h"
#include <QSettings>

CharacteristicNotifier2AD2::CharacteristicNotifier2AD2(bluetoothdevice *Bike, QObject *parent)
    : CharacteristicNotifier(0x2ad2, parent), Bike(Bike) {}

int CharacteristicNotifier2AD2::notify(QByteArray &value) {
    BLUETOOTH_TYPE dt = Bike->deviceType();

    QSettings settings;
    bool virtual_device_rower =
        settings.value(QZSettings::virtual_device_rower, QZSettings::default_virtual_device_rower).toBool();
    bool rowerAsABike = !virtual_device_rower && dt == ROWING;
    bool double_cadence = settings.value(QZSettings::powr_sensor_running_cadence_double, QZSettings::default_powr_sensor_running_cadence_double).toBool();
    double cadence_multiplier = 2.0;
    if (double_cadence)
        cadence_multiplier = 1.0;


    double normalizeWattage = Bike->wattsMetricforUI();
    if (normalizeWattage < 0)
        normalizeWattage = 0;

    if (dt == BIKE || rowerAsABike) {
        uint16_t normalizeSpeed = (uint16_t)qRound(Bike->currentSpeed().value() * 100);
        value.append((char)0x64); // speed, inst. cadence, resistance lvl, instant power
        value.append((char)0x02); // heart rate

        value.append((char)(normalizeSpeed & 0xFF));      // speed
        value.append((char)(normalizeSpeed >> 8) & 0xFF); // speed

        value.append((char)((uint16_t)(Bike->currentCadence().value() * cadence_multiplier) & 0xFF));        // cadence
        value.append((char)(((uint16_t)(Bike->currentCadence().value() * cadence_multiplier) >> 8) & 0xFF)); // cadence

        value.append((char)Bike->currentResistance().value()); // resistance
        value.append((char)(0));                               // resistance

        value.append((char)(((uint16_t)normalizeWattage) & 0xFF));      // watts
        value.append((char)(((uint16_t)normalizeWattage) >> 8) & 0xFF); // watts

        value.append(char(Bike->currentHeart().value())); // Actual value.
        value.append((char)0);                            // Bkool FTMS protocol HRM offset 1280 fix
        return CN_OK;
    } else if (dt == TREADMILL || dt == ELLIPTICAL || dt == ROWING) {
        uint16_t normalizeSpeed = (uint16_t)qRound(Bike->currentSpeed().value() * 100);
        value.append((char)0x64); // speed, inst. cadence, resistance lvl, instant power
        value.append((char)0x02); // heart rate

        value.append((char)(normalizeSpeed & 0xFF));      // speed
        value.append((char)(normalizeSpeed >> 8) & 0xFF); // speed

        uint16_t cadence = 0;
        if (dt == ELLIPTICAL)
            cadence = ((elliptical *)Bike)->currentCadence().value();
        else if (dt == TREADMILL)
            cadence = ((treadmill *)Bike)->currentCadence().value();
        else if (dt == ROWING)
            cadence = ((rower *)Bike)->currentCadence().value();

        value.append((char)((uint16_t)(cadence * cadence_multiplier) & 0xFF));        // cadence
        value.append((char)(((uint16_t)(cadence * cadence_multiplier) >> 8) & 0xFF)); // cadence

        value.append((char)(0)); // resistance
        value.append((char)(0)); // resistance

        value.append((char)(((uint16_t)normalizeWattage) & 0xFF));      // watts
        value.append((char)(((uint16_t)normalizeWattage) >> 8) & 0xFF); // watts

        value.append(char(Bike->currentHeart().value())); // Actual value.
        value.append((char)0);
        return CN_OK;
    } else
        return CN_INVALID;
}
