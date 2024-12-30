#ifndef CHARACTERISTICWRITEPROCESSOR0003_H
#define CHARACTERISTICWRITEPROCESSOR0003_H

#include "characteristicnotifier0002.h"
#include "characteristicwriteprocessor.h"

class CharacteristicWriteProcessor0003 : public CharacteristicWriteProcessor {
    Q_OBJECT
    CharacteristicNotifier0002 *notifier = nullptr;

public:
    explicit CharacteristicWriteProcessor0003(double bikeResistanceGain, int8_t bikeResistanceOffset,
                                            bluetoothdevice *bike, CharacteristicNotifier0002 *notifier,
                                            QObject *parent = nullptr);
    int writeProcess(quint16 uuid, const QByteArray &data, QByteArray &out) override;

private:
    struct VarintResult {
        qint64 value;
        int bytesRead;
    };

    VarintResult decodeVarint(const QByteArray& bytes, int startIndex);
    qint32 decodeSInt(const QByteArray& bytes);
    void handleZwiftGear(const QByteArray &array);
    int currentZwiftGear = 0;

signals:
    void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
};

#endif // CHARACTERISTICWRITEPROCESSOR0003_H
