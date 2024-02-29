#ifndef SMARTSPIN2K_H
#define SMARTSPIN2K_H

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
#include <QUdpSocket>

#include "devices/bike.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class smartspin2k : public bike {

    Q_OBJECT
  public:
    smartspin2k(bool noWriteResistance, bool noHeartService, resistance_t max_resistance, bike *parentDevice);
    bool connected() override;

  private:
    #define max_calibration_samples  4
    void writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log = false,
                             bool wait_for_response = false);
    void writeCharacteristicFTMS(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log = false,
                                 bool wait_for_response = false);
    void startDiscover();
    uint16_t watts() override;
    void forceResistance(resistance_t requestResistance);
    void setShiftStep(uint16_t);
    void lowInit(resistance_t resistance);

    QTimer *refresh;

    QUdpSocket *udpSocket = new QUdpSocket();

    QLowEnergyService *gattCommunicationChannelService = nullptr;
    QLowEnergyService *gattCommunicationChannelServiceFTMS = nullptr;
    QLowEnergyCharacteristic gattWriteCharacteristic;
    QLowEnergyCharacteristic gattWriteCharControlPointId;

    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;

    bool initDone = false;
    bool initRequest = false;
    bool first = true;

    bool noWriteResistance = false;
    bool noHeartService = false;

    resistance_t startupResistance = -1;
    resistance_t lastResistance;
    resistance_t lastRequestResistance;

    resistance_t max_resistance;

    double slope = 0.0;
    double intercept = 0.0;

    bike *parentDevice = nullptr;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  signals:
    void disconnected();
    void debug(QString string);
    void gearUp();
    void gearDown();

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void resistanceReadFromTheBike(resistance_t resistance);
    void autoResistanceChanged(bool value);
    void calibrateShiftStep();

  private slots:

    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void stateChanged(QLowEnergyService::ServiceState state);
    void stateChangedFTMS(QLowEnergyService::ServiceState state);
    void controllerStateChanged(QLowEnergyController::ControllerState state);

    void readPendingDatagrams();

    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone(void);
    void update();
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError);
};

#endif // SMARTSPIN2K_H
