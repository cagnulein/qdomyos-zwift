#ifndef DIRCONMANAGER_H
#define DIRCONMANAGER_H

#include "devices/bluetoothdevice.h"
#include "characteristics/characteristicnotifier2a37.h"
#include "characteristics/characteristicnotifier2a53.h"
#include "characteristics/characteristicnotifier2a5b.h"
#include "characteristics/characteristicnotifier2a63.h"
#include "characteristics/characteristicnotifier2acc.h"
#include "characteristics/characteristicnotifier2acd.h"
#include "characteristics/characteristicnotifier2ad2.h"
#include "characteristics/characteristicnotifier2ad9.h"
#include "characteristics/characteristicwriteprocessor2ad9.h"
#include "characteristics/characteristicwriteprocessore005.h"
#include "characteristics/characteristicwriteprocessor0003.h"
#include "devices/dircon/dirconpacket.h"
#include "devices/dircon/dirconprocessor.h"
#include <QObject>

#define DM_CHAR_NOTIF_OP(OP, P1, P2, P3)                                                                               \
    OP(2AD2, P1, P2, P3)                                                                                               \
    OP(2A63, P1, P2, P3)                                                                                               \
    OP(2A37, P1, P2, P3) OP(2A5B, P1, P2, P3) OP(2A53, P1, P2, P3) OP(2ACD, P1, P2, P3) OP(2ACC, P1, P2, P3)           \
        OP(2AD9, P1, P2, P3)                                                                                           \
    OP(0002, P1, P2, P3) OP(0004, P1, P2, P3)

#define DM_CHAR_NOTIF_DEFINE_OP(UUID, P1, P2, P3) CharacteristicNotifier##UUID *notif##UUID = 0;

class DirconManager : public QObject {
    Q_OBJECT
    QTimer bikeTimer;
    CharacteristicWriteProcessor2AD9 *writeP2AD9 = 0;
    CharacteristicWriteProcessor0003 *writeP0003 = 0;
    CharacteristicWriteProcessorE005 *writePE005 = 0;
    DM_CHAR_NOTIF_OP(DM_CHAR_NOTIF_DEFINE_OP, 0, 0, 0)
    QList<DirconProcessor *> processors;
    static QString getMacAddress();
    bluetoothdevice* bt = 0;

  public:
    double currentGear();
    explicit DirconManager(bluetoothdevice *t, int8_t bikeResistanceOffset = 4, double bikeResistanceGain = 1.0,
                           QObject *parent = nullptr);

    /**
     * @brief Rebind every consumer of the bound device in one pass.
     *
     * The device pointer is copied into three places when the manager is built:
     * `bt`, the CharacteristicNotifier chain, and the write processors. Rebinding
     * only some of them leaves a live object dereferencing a freed device, so this
     * is the only supported way to change it.
     *
     * Neither the TCP listener nor the mDNS advertisement is touched: swapping the
     * device must stay invisible to a connected client, which is the whole point of
     * giving the endpoint a lifetime longer than the bike's.
     */
    void setDevice(bluetoothdevice *t);
    bluetoothdevice *device() const { return bt; }
  private slots:
    void bikeProvider();
  signals:
    void changeInclination(double grade, double percentage);
    void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
};

#endif // DIRCOMMANAGER_H
