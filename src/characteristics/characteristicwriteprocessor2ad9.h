#ifndef CHARACTERISTICWRITEPROCESSOR2AD9_H
#define CHARACTERISTICWRITEPROCESSOR2AD9_H

#include "characteristicnotifier2ad9.h"
#include "characteristicwriteprocessor.h"

class CharacteristicWriteProcessor2AD9 : public CharacteristicWriteProcessor {
    Q_OBJECT
    CharacteristicNotifier2AD9 *notifier = nullptr;

  public:
    explicit CharacteristicWriteProcessor2AD9(double bikeResistanceGain, int8_t bikeResistanceOffset,
                                              bluetoothdevice *bike, CharacteristicNotifier2AD9 *notifier,
                                              QObject *parent = nullptr);
    int writeProcess(quint16 uuid, const QByteArray &data, QByteArray &out) override;
  signals:
    void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
};

#endif // CHARACTERISTICWRITEPROCESSOR2AD9_H
