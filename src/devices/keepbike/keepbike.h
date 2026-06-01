#ifndef KEEPBIKE_H
#define KEEPBIKE_H

#include <QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/qlowenergyadvertisingdata.h>
#include <QtBluetooth/qlowenergyadvertisingparameters.h>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtBluetooth/qlowenergycharacteristicdata.h>
#include <QtBluetooth/qlowenergycontroller.h>
#include <QtBluetooth/qlowenergydescriptordata.h>
#include <QtBluetooth/qlowenergyservice.h>
#include <QtBluetooth/qlowenergyservicedata.h>
#include <QtCore/qbytearray.h>

#ifndef Q_OS_ANDROID
#include <QtCore/qcoreapplication.h>
#else
#include <QtGui/qguiapplication.h>
#endif
#include <QtCore/qlist.h>
#include <QtCore/qmutex.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qtimer.h>

#include <QDateTime>
#include <QObject>
#include <QString>

#include "devices/bike.h"
#include "virtualdevices/virtualbike.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class keepbike : public bike {
    Q_OBJECT
  public:
    struct NewProtocolMetrics {
        bool valid = false;
        quint32 timestamp = 0;
        uint16_t elapsed = 0;
        uint16_t cadence = 0;
        uint16_t resistance = 0;
        uint16_t watt = 0;
        uint16_t status = 0;
    };

    static bool isNewProtocolFrame(const QByteArray &packet);
    static uint16_t newProtocolCrc(const QByteArray &packet);
    static QByteArray buildNewProtocolFrame(uint16_t sequence, quint32 session, const QByteArray &payload);
    static NewProtocolMetrics parseNewProtocolMetricsFrame(const QByteArray &packet);

    keepbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset, double bikeResistanceGain);
    resistance_t pelotonToBikeResistance(int pelotonResistance) override;
    resistance_t maxResistance() override { return max_resistance; }
    bool connected() override;

  private:
    const resistance_t max_resistance = 36;
    double bikeResistanceToPeloton(double resistance);
    double GetDistanceFromPacket(const QByteArray &packet);
    double GetSpeedFromPacket(const QByteArray &packet);
    double GetWattFromPacket(const QByteArray &packet);
    QTime GetElapsedFromPacket(const QByteArray &packet);
    void btinit();
    void btinitNewProtocol();
    void writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log = false,
                             bool wait_for_response = false);
    void writeNewProtocolCommand(const QByteArray &payload, const QString &info, bool wait_for_response = true);
    void startDiscover();
    void forceResistance(resistance_t requestResistance);
    void sendPoll();
    void sendPollNewProtocol();
    bool handleNewProtocolFrame(const QByteArray &newValue);
    uint16_t watts() override;

    QTimer *refresh;

    QLowEnergyService *gattCommunicationChannelService = nullptr;
    QLowEnergyCharacteristic gattWriteCharacteristic;
    QLowEnergyCharacteristic gattNotifyCharacteristic;

    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;
    uint8_t counterPoll = 1;
    uint16_t newProtocolSequence = 0x00a0;
    quint32 newProtocolSession = 0;
    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;
    resistance_t lastResistanceBeforeDisconnection = -1;

    bool initDone = false;
    bool initRequest = false;
    bool newProtocol = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  Q_SIGNALS:
    void disconnected();

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  private slots:

    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void stateChanged(QLowEnergyService::ServiceState state);
    void controllerStateChanged(QLowEnergyController::ControllerState state);

    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone(void);
    void update();
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError);
};

#endif // KEEPBIKE_H
