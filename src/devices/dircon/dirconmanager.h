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
    // Milliseconds, decided at construction from zwift_play_emulator / race_mode.
    // Kept so the timer can be stopped while idle and restarted on setDevice().
    int bikeTimerInterval = 1000;
    CharacteristicWriteProcessor2AD9 *writeP2AD9 = 0;
    CharacteristicWriteProcessor0003 *writeP0003 = 0;
    CharacteristicWriteProcessorE005 *writePE005 = 0;
    DM_CHAR_NOTIF_OP(DM_CHAR_NOTIF_DEFINE_OP, 0, 0, 0)
    QList<DirconProcessor *> processors;
    static QString getMacAddress();
    bluetoothdevice* bt = 0;
    // Which DM_MACHINE_TYPE_* this endpoint was built for. It fixes both the advertised
    // service set and the listening port, so it cannot be changed by rebinding.
    uint8_t machineType = 0;

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

    /**
     * @brief Keep the resistance conversion in step with the device.
     *
     * Previously a fresh manager was built per virtual device, so these came along
     * with the constructor. The shared manager is built once, so a later device
     * carrying different settings has to push them in.
     */
    void setResistanceParameters(int8_t bikeResistanceOffset, double bikeResistanceGain);

    /**
     * @brief The process-lifetime DIRCON endpoint, created on first use.
     *
     * The TCP listener and the mDNS advertisement must outlive any single virtual
     * device: destroying and rebuilding them mid-session leaves clients that cache
     * discovery results (Rouvy) holding a record for a port nobody is listening on.
     * The instance is parented to the application object, so no virtual device owns
     * it and `bluetoothdevice::setVirtualDevice()` cannot take it down.
     *
     * Subsequent calls rebind the existing manager to @p t rather than building a
     * second one, which would collide on the listening port.
     */
    static DirconManager *shared(bluetoothdevice *t, int8_t bikeResistanceOffset = 4,
                                 double bikeResistanceGain = 1.0);
    static DirconManager *sharedIfAny();

    /**
     * @brief Bring the endpoint up at launch, before any device is connected.
     *
     * This is the point of the whole refactor. A client that caches discovery results
     * and does not retry a failed connect - Rouvy does both - will otherwise latch on
     * to a record for a port nobody is listening on, and stay broken until its cache is
     * cleared by hand. Listening from launch means a cached record is always
     * connectable.
     *
     * Guarded by `dircon_yes` and idempotent. The endpoint serves the bike profile;
     * see machineTypeFor().
     */
    static void startIdleEndpoint();

    /**
     * @brief The DM_MACHINE_TYPE_* an endpoint for @p t has to be built as.
     * Returns the bike profile for a null device.
     */
    static uint8_t machineTypeFor(bluetoothdevice *t);

    /**
     * @brief Tear the shared endpoint down, freeing its ports and withdrawing its
     * advertisement.
     *
     * Only for callers that need to build a DIRCON endpoint with a different service
     * profile - today that means `virtualtreadmill`, whose machine type maps to the
     * same base port. Destroying the manager sends the mDNS goodbye through
     * `~ProviderPrivate()`, so clients are told before the socket disappears.
     */
    static void releaseShared();
  private slots:
    void bikeProvider();
  signals:
    void changeInclination(double grade, double percentage);
    void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
};

#endif // DIRCOMMANAGER_H
