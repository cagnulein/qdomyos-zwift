#include "characteristicnotifier2a53.h"
#include "devices/treadmill.h"
#include <QSettings>


CharacteristicNotifier2A53::CharacteristicNotifier2A53(bluetoothdevice *Bike, QObject *parent)
    : CharacteristicNotifier(0x2a53, parent), Bike(Bike) {}

int CharacteristicNotifier2A53::notify(QByteArray &value) {
    QSettings settings;
    bool double_cadence = settings.value(QZSettings::powr_sensor_running_cadence_double, QZSettings::default_powr_sensor_running_cadence_double).toBool();
    double cadence_multiplier = 1.0;
    if (double_cadence)
        cadence_multiplier = 2.0;    
    bluetoothdevice::BLUETOOTH_TYPE dt = Bike->deviceType();
    value.append(0x02); // total distance
    uint16_t speed = Bike->currentSpeed().value() / 3.6 * 256;
    uint32_t distance = Bike->odometer() * 10000.0;
    value.append((char)((speed & 0xFF)));
    value.append((char)((speed >> 8) & 0xFF));
    value.append((char)(Bike->currentCadence().value() / cadence_multiplier));
    value.append((char)((distance & 0xFF)));
    value.append((char)((distance >> 8) & 0xFF));
    value.append((char)((distance >> 16) & 0xFF));
    value.append((char)((distance >> 24) & 0xFF));
    return CN_OK;
}
