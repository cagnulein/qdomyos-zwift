#ifndef DIRCONMANAGER_H
#define DIRCONMANAGER_H

#include "bluetoothdevice.h"
#include "characteristicnotifier2a37.h"
#include "characteristicnotifier2a5b.h"
#include "characteristicnotifier2a63.h"
#include "characteristicnotifier2ad2.h"
#include "characteristicwriteprocessor2ad9.h"
#include "dirconpacket.h"
#include "dirconprocessor.h"
#include <QObject>

class DirconManager : public QObject {
    Q_OBJECT
    QTimer bikeTimer;
    CharacteristicWriteProcessor2AD9 *writeP2AD9 = 0;
    CharacteristicNotifier2AD2 *notif2AD2 = 0;
    CharacteristicNotifier2A63 *notif2A63 = 0;
    CharacteristicNotifier2A37 *notif2A37 = 0;
    CharacteristicNotifier2A5B *notif2A5B = 0;
    QList<DirconProcessor *> processors;
    static QString getMacAddress();

  public:
    explicit DirconManager(bluetoothdevice *t, uint8_t bikeResistanceOffset = 4, double bikeResistanceGain = 1.0,
                           QObject *parent = nullptr);
  private slots:
    void bikeProvider();
  signals:
    void changeInclination(double grade, double percentage);
    void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
};

#endif // DIRCOMMANAGER_H
