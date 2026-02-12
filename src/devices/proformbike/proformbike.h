#ifndef PROFORMBIKE_H
#define PROFORMBIKE_H

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

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class proformbike : public bike {
    Q_OBJECT
  public:
    proformbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset, double bikeResistanceGain);
    resistance_t pelotonToBikeResistance(int pelotonResistance) override;
    resistance_t resistanceFromPowerRequest(uint16_t power) override;
    resistance_t maxResistance() override { return max_resistance; }
    bool inclinationAvailableByHardware() override;
    bool connected() override;

  private:
    resistance_t max_resistance = 16;
    uint16_t wattsFromResistance(resistance_t resistance);
    double GetDistanceFromPacket(QByteArray packet);
    QTime GetElapsedFromPacket(QByteArray packet);
    void btinit();
    void writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log = false,
                             bool wait_for_response = false);
    void startDiscover();
    void sendPoll();
    uint16_t watts() override;
    void forceResistance(resistance_t requestResistance);
    void forceIncline(double incline);
    bool innerWriteResistance();

    QTimer *refresh;
    uint8_t counterPoll = 0;
    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;

    QLowEnergyService *gattCommunicationChannelService = nullptr;
    QLowEnergyCharacteristic gattWriteCharacteristic;
    QLowEnergyCharacteristic gattNotify1Characteristic;

    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;
    uint16_t m_watts = 0;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

    bool proform_studio = false;
    bool proform_tdf_10 = false;
    bool nordictrack_GX4_5_bike = false;
    bool nordictrack_gx_2_7 = false;
    bool proform_hybrid_trainer_PFEL03815 = false;
    bool proform_bike_sb = false;
    bool proform_cycle_trainer_300_ci =false;
    bool proform_bike_225_csx = false;
    bool proform_bike_325_csx = false;
    bool proform_tour_de_france_clc = false;
    bool proform_studio_NTEX71021 = false;
    bool freemotion_coachbike_b22_7 = false;
    bool proform_cycle_trainer_400 = false;
    bool proform_bike_PFEVEX71316_1 = false;
    bool nordictrack_gx_44_pro = false;
    bool proform_bike_PFEVEX71316_0 = false;
    bool proform_xbike = false;
    bool proform_225_csx_PFEX32925_INT_0 = false;
    bool proform_csx210 = false;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  signals:
    void disconnected();
    void debug(QString string);

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

#endif // PROFORMBIKE_H
