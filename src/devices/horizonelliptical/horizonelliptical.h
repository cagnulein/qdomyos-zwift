#ifndef HORIZONELLIPTICAL_H
#define HORIZONELLIPTICAL_H

#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtBluetooth/qlowenergycontroller.h>
#include <QtBluetooth/qlowenergyservice.h>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QtCore/qbytearray.h>

#ifndef Q_OS_ANDROID
#include <QtCore/qcoreapplication.h>
#else
#include <QtGui/qguiapplication.h>
#endif
#include <QtCore/qlist.h>
#include <QtCore/qmutex.h>
#include <QtCore/qtimer.h>

#include <QDateTime>
#include <QObject>
#include <QString>

#include "devices/elliptical.h"
#include "devices/horizonprotocol/horizonprofile.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

/**
 * Horizon ellipticals (e.g. 7.0 AE) on the custom FFF0 protocol.
 *
 * They share the wire protocol with the Horizon treadmills, but the machines are not treadmills:
 * inclination is a 1-20 level rather than a percentage, they report a real cadence, and they report
 * a resistance level the treadmills do not have. Driving them through horizontreadmill therefore
 * produced a half-wrong picture and recorded the sessions as runs.
 *
 * The profile-upload handshake is shared with the treadmill via horizonprofile rather than
 * duplicated - treadmill and elliptical are sibling classes, so it cannot be shared by inheritance.
 */
class horizonelliptical : public elliptical {
    Q_OBJECT
  public:
    horizonelliptical(bool noWriteResistance = false, bool noHeartService = false);
    bool connected() override;
    double minStepInclination() override { return 1.0; }   // whole console levels
    bool inclinationAvailableByHardware() override { return true; }
    bool inclinationSeparatedFromResistance() override { return true; }

  private:
    void writeCharacteristic(QLowEnergyService *service, QLowEnergyCharacteristic characteristic, uint8_t *data,
                             uint8_t data_len, const QString &info, bool disable_log = false,
                             bool wait_for_response = false);
    void waitForAPacket();
    void startDiscover();
    void btinit();
    void parseCustomFrame(const QByteArray &frame);

    QTimer *refresh;
    horizonprofile profile;

    QList<QLowEnergyService *> gattCommunicationChannelService;
    QLowEnergyService *gattCustomService = nullptr;
    QLowEnergyCharacteristic gattWriteCharCustomService;
    volatile int notificationSubscribed = 0;

    QByteArray lastPacket;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    bool firstDistanceCalculated = false;
    uint8_t firstStateChanged = 0;
    bool initDone = false;
    bool initRequest = false;
    bool initPacketRecv = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  signals:
    void disconnected();
    void debug(QString string);
    void packetReceived();

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  private slots:
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void stateChanged(QLowEnergyService::ServiceState state);
    void controllerStateChanged(QLowEnergyController::ControllerState state);
    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone(void);
    void update();
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError);
};

#endif // HORIZONELLIPTICAL_H
