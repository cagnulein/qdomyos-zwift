#ifndef CHARACTERISTICWRITEPROCESSOR0003_H
#define CHARACTERISTICWRITEPROCESSOR0003_H

#include "characteristicnotifier0002.h"
#include "characteristicnotifier0004.h"
#include "characteristicwriteprocessor.h"
#include <QElapsedTimer>

class CharacteristicWriteProcessor0003 : public CharacteristicWriteProcessor {
    Q_OBJECT
    CharacteristicNotifier0002 *notifier0002 = nullptr;
    CharacteristicNotifier0004 *notifier0004 = nullptr;

public:
    explicit CharacteristicWriteProcessor0003(double bikeResistanceGain, int8_t bikeResistanceOffset,
                                            bluetoothdevice *bike, CharacteristicNotifier0002 *notifier0002,
                                            CharacteristicNotifier0004 *notifier0004,
                                            QObject *parent = nullptr);
    int writeProcess(quint16 uuid, const QByteArray &data, QByteArray &out) override;
    static QByteArray encodeHubRidingData(uint32_t power,
                                    uint32_t cadence,
                                    uint32_t speedX100,
                                    uint32_t hr,
                                    uint32_t unknown1,
                                    uint32_t unknown2);
    static uint32_t calculateUnknown1(uint16_t power);
    void handleZwiftGear(const QByteArray &array);
    double currentGear();
    qint64 hubRidingDataIdleMs() const;
    QByteArray buildCurrentHubRidingData();


private:
    struct VarintResult {
        qint64 value;
        int bytesRead;
    };

    VarintResult decodeVarint(const QByteArray& bytes, int startIndex);
    qint32 decodeSInt(const QByteArray& bytes);    
    int currentZwiftGear = 8;
    bool zwiftGearReceived = false;
    QElapsedTimer lastHubRidingDataTimer;

signals:
    void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
};

#endif // CHARACTERISTICWRITEPROCESSOR0003_H
