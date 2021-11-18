#ifndef CHARACTERISTICWRITEPROCESSOR2AD9_H
#define CHARACTERISTICWRITEPROCESSOR2AD9_H

#include "bluetoothdevice.h"
#include "characteristicwriteprocessor.h"

class CharacteristicWriteProcessor2AD9 : public CharacteristicWriteProcessor {
    Q_OBJECT
    uint8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;
    bluetoothdevice *Bike;

  public:
    void slopeChanged(int16_t slope);
    void powerChanged(uint16_t power);
    explicit CharacteristicWriteProcessor2AD9(double bikeResistanceGain, uint8_t bikeResistanceOffset,
                                              bluetoothdevice *bike, QObject *parent = nullptr);
    virtual int writeProcess(quint16 uuid, const QByteArray &data, QByteArray &out);
  signals:
    void changeInclination(double grade, double percentage);
    void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
};

#endif // CHARACTERISTICWRITEPROCESSOR2AD9_H
