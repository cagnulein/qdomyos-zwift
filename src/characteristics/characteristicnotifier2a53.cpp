#include "characteristicnotifier2a53.h"
#include "devices/treadmill.h"

CharacteristicNotifier2A53::CharacteristicNotifier2A53(bluetoothdevice *Bike, QObject *parent, bool garminFootPod)
    : CharacteristicNotifier(0x2a53, parent), Bike(Bike), garminFootPod(garminFootPod) {}

int CharacteristicNotifier2A53::notify(QByteArray &value) {
    BLUETOOTH_TYPE dt = Bike->deviceType();
    const double speedKmh = Bike->currentSpeed().value();
    const bool running = speedKmh >= 7.0;
    uint8_t cadence = Bike->currentCadence().value();
    if (garminFootPod) {
        // A Garmin watch takes the cadence only from the foot pod and doubles it: 170 sent is shown as 340.
        // Without a cadence source it would show 0, so estimate it from the speed.
        double stepsPerMinute = Bike->currentCadence().value();
        if (stepsPerMinute <= 0 && speedKmh >= 0.5) {
            stepsPerMinute = running ? 150.0 + 2.5 * speedKmh : 80.0 + 6.0 * speedKmh;
        } else if (stepsPerMinute > 0 && stepsPerMinute < (speedKmh > 6.0 ? 120.0 : 60.0)) {
            // Per-leg cadence, as in treadmill::evaluateStepCount()
            stepsPerMinute *= 2;
        }
        cadence = qMin(stepsPerMinute, 220.0) / 2.0 + 0.5;
    }
    value.append(0x02 | (garminFootPod && running ? 0x04 : 0x00)); // total distance, walking or running status
    uint16_t speed = speedKmh / 3.6 * 256;
    uint32_t distance = Bike->odometer() * 10000.0;
    value.append((char)((speed & 0xFF)));
    value.append((char)((speed >> 8) & 0xFF));
    value.append((char)(cadence));
    value.append((char)((distance & 0xFF)));
    value.append((char)((distance >> 8) & 0xFF));
    value.append((char)((distance >> 16) & 0xFF));
    value.append((char)((distance >> 24) & 0xFF));
    return CN_OK;
}
