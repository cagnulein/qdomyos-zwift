#include "characteristicnotifier2acd.h"
#include "devices/treadmill.h"
#include <qmath.h>
#include <QTime> // Include QTime for Bike->elapsedTime()

CharacteristicNotifier2ACD::CharacteristicNotifier2ACD(bluetoothdevice *Bike, QObject *parent)
    : CharacteristicNotifier(0x2acd, parent), Bike(Bike) {}

int CharacteristicNotifier2ACD::notify(QByteArray &value) {
    bluetoothdevice::BLUETOOTH_TYPE dt = Bike->deviceType();
    if (dt == bluetoothdevice::TREADMILL || dt == bluetoothdevice::ELLIPTICAL) {
        value.append(0x0C);       // Inclination available and distance for peloton
        //value.append((char)0x01); // heart rate available
        value.append((char)0x05); // HeartRate(8) | ElapsedTime(10)

        uint16_t normalizeSpeed = (uint16_t)qRound(Bike->currentSpeed().value() * 100);
        char a = (normalizeSpeed >> 8) & 0XFF;
        char b = normalizeSpeed & 0XFF;
        QByteArray speedBytes;
        speedBytes.append(b);
        speedBytes.append(a);
        
        // peloton wants the distance from the qz startup to handle stacked classes
        // https://github.com/cagnulein/qdomyos-zwift/issues/2018
        uint32_t normalizeDistance = (uint32_t)qRound(Bike->odometerFromStartup() * 1000);
        a = (normalizeDistance >> 16) & 0XFF;
        b = (normalizeDistance >> 8) & 0XFF;
        char c = normalizeDistance & 0XFF;
        QByteArray distanceBytes;
        distanceBytes.append(c);
        distanceBytes.append(b);
        distanceBytes.append(a);

        
        uint16_t normalizeIncline = 0;

        QSettings settings;
        bool real_inclination_to_virtual_treamill_bridge = settings.value(QZSettings::real_inclination_to_virtual_treamill_bridge, QZSettings::default_real_inclination_to_virtual_treamill_bridge).toBool();
        double inclination = ((treadmill *)Bike)->currentInclination().value();
        if(real_inclination_to_virtual_treamill_bridge) {
            double offset = settings.value(QZSettings::zwift_inclination_offset,
                                           QZSettings::default_zwift_inclination_offset).toDouble();
            double gain = settings.value(QZSettings::zwift_inclination_gain,
                                         QZSettings::default_zwift_inclination_gain).toDouble();
            inclination -= offset;
            inclination /= gain;
        }

        if (dt == bluetoothdevice::TREADMILL)
            normalizeIncline = (uint32_t)qRound(inclination * 10);
        a = (normalizeIncline >> 8) & 0XFF;
        b = normalizeIncline & 0XFF;
        QByteArray inclineBytes;
        inclineBytes.append(b);
        inclineBytes.append(a);
        double ramp = 0;
        if (dt == bluetoothdevice::TREADMILL)
            ramp = qRadiansToDegrees(qAtan(inclination / 100));
        int16_t normalizeRamp = (int32_t)qRound(ramp * 10);
        a = (normalizeRamp >> 8) & 0XFF;
        b = normalizeRamp & 0XFF;
        QByteArray rampBytes;
        rampBytes.append(b);
        rampBytes.append(a);

        // Get session elapsed time - makes Runna calculations work
        QTime sessionElapsedTime = Bike->elapsedTime();
        double elapsed_time_seconds =
            (double)sessionElapsedTime.hour() * 3600.0 +
            (double)sessionElapsedTime.minute() * 60.0 +
            (double)sessionElapsedTime.second() +
            (double)sessionElapsedTime.msec() / 1000.0;
        uint16_t ftms_elapsed_time_field = (uint16_t)qRound(elapsed_time_seconds);
        QByteArray elapsedBytes;
        elapsedBytes.append(static_cast<char>(ftms_elapsed_time_field & 0xFF));
        elapsedBytes.append(static_cast<char>((ftms_elapsed_time_field >> 8) & 0xFF));

        value.append(speedBytes); // Actual value.
        
        value.append(distanceBytes); // Actual value.

        value.append(inclineBytes); // incline

        value.append(rampBytes); // ramp angle

        value.append(Bike->currentHeart().value()); // current heart rate

        value.append(elapsedBytes); // Elapsed Time

        return CN_OK;
    } else
        return CN_INVALID;
}
