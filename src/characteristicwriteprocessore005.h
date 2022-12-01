#ifndef CHARACTERISTICWRITEPROCESSORE005_H
#define CHARACTERISTICWRITEPROCESSORE005_H

#include "bluetoothdevice.h"
#include "characteristicnotifier2ad9.h"
#include "characteristicwriteprocessor.h"

class CharacteristicWriteProcessorE005 : public CharacteristicWriteProcessor {
    Q_OBJECT
    uint8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;
    bluetoothdevice *Bike;

  public:
    explicit CharacteristicWriteProcessorE005(double bikeResistanceGain, uint8_t bikeResistanceOffset,
                                              bluetoothdevice *bike, // CharacteristicNotifier2AD9 *notifier,
                                              QObject *parent = nullptr);
    virtual int writeProcess(quint16 uuid, const QByteArray &data, QByteArray &out);
    void changeSlope(int16_t slope, uint8_t crr, uint8_t cw);
    void changePower(uint16_t power);
  signals:
    void changeInclination(double grade, double percentage);
    void slopeChanged();
    void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
};

#endif // CHARACTERISTICWRITEPROCESSORE005_H
