#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <QBluetoothDeviceDiscoveryAgent>
#include <QFile>
#include <QObject>
#include <QtBluetooth/qlowenergyadvertisingdata.h>
#include <QtBluetooth/qlowenergyadvertisingparameters.h>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtBluetooth/qlowenergycharacteristicdata.h>

#include <QtBluetooth/qlowenergycontroller.h>
#include <QtBluetooth/qlowenergydescriptordata.h>
#include <QtBluetooth/qlowenergyservice.h>
#include <QtBluetooth/qlowenergyservicedata.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qloggingcategory.h>

#include "bluetoothdevice.h"
#include "chronobike.h"
#include "cscbike.h"
#include "domyosbike.h"
#include "domyoselliptical.h"
#include "domyostreadmill.h"

#include "echelonconnectsport.h"
#include "echelonrower.h"
#include "eliterizer.h"
#include "eslinkertreadmill.h"
#include "fitplusbike.h"

#include "fitshowtreadmill.h"
#include "flywheelbike.h"
#include "ftmsbike.h"
#include "ftmsrower.h"
#include "heartratebelt.h"
#include "horizontreadmill.h"
#include "inspirebike.h"
#include "kingsmithr1protreadmill.h"
#include "kingsmithr2treadmill.h"
#include "m3ibike.h"
#include "npecablebike.h"
#include "proformbike.h"
#include "proformtreadmill.h"
#include "schwinnic4bike.h"
#include "signalhandler.h"
#include "skandikawiribike.h"
#include "smartrowrower.h"
#include "smartspin2k.h"
#include "snodebike.h"

#include "soleelliptical.h"
#include "solef80.h"

#include "spirittreadmill.h"
#include "sportstechbike.h"
#include "stagesbike.h"

#include "renphobike.h"
#include "tacxneo2.h"

#include "echelonstride.h"

#include "templateinfosenderbuilder.h"
#include "toorxtreadmill.h"
#include "treadmill.h"
#include "trxappgateusbbike.h"
#include "trxappgateusbtreadmill.h"
#include "yesoulbike.h"

class bluetooth : public QObject, public SignalHandler {

    Q_OBJECT
  public:
    explicit bluetooth(bool logs, const QString &deviceName = QLatin1String(""), bool noWriteResistance = false,
                       bool noHeartService = false, uint32_t pollDeviceTime = 200, bool noConsole = false,
                       bool testResistance = false, uint8_t bikeResistanceOffset = 4, double bikeResistanceGain = 1.0);
    ~bluetooth();
    bluetoothdevice *device();
    bluetoothdevice *heartRateDevice() { return heartRateBelt; }
    QList<QBluetoothDeviceInfo> devices;
    bool onlyDiscover = false;
    TemplateInfoSenderBuilder *getUserTemplateManager() const { return userTemplateManager; }
    TemplateInfoSenderBuilder *getInnerTemplateManager() const { return innerTemplateManager; }

  private:
    TemplateInfoSenderBuilder *userTemplateManager = nullptr;
    TemplateInfoSenderBuilder *innerTemplateManager = nullptr;
    QFile *debugCommsLog = nullptr;
    QBluetoothDeviceDiscoveryAgent *discoveryAgent;
    fitshowtreadmill *fitshowTreadmill = nullptr;
    domyostreadmill *domyos = nullptr;
    domyosbike *domyosBike = nullptr;
    domyoselliptical *domyosElliptical = nullptr;
    toorxtreadmill *toorx = nullptr;
    trxappgateusbtreadmill *trxappgateusb = nullptr;
    spirittreadmill *spiritTreadmill = nullptr;
    trxappgateusbbike *trxappgateusbBike = nullptr;
    echelonconnectsport *echelonConnectSport = nullptr;
    yesoulbike *yesoulBike = nullptr;
    flywheelbike *flywheelBike = nullptr;
    proformbike *proformBike = nullptr;
    proformtreadmill *proformTreadmill = nullptr;
    horizontreadmill *horizonTreadmill = nullptr;
    schwinnic4bike *schwinnIC4Bike = nullptr;
    sportstechbike *sportsTechBike = nullptr;
    inspirebike *inspireBike = nullptr;
    snodebike *snodeBike = nullptr;
    eslinkertreadmill *eslinkerTreadmill = nullptr;
    m3ibike *m3iBike = nullptr;
    skandikawiribike *skandikaWiriBike = nullptr;
    cscbike *cscBike = nullptr;
    npecablebike *npeCableBike = nullptr;
    stagesbike *stagesBike = nullptr;
    soleelliptical *soleElliptical = nullptr;
    solef80 *soleF80 = nullptr;
    chronobike *chronoBike = nullptr;
    fitplusbike *fitPlusBike = nullptr;
    echelonrower *echelonRower = nullptr;
    ftmsrower *ftmsRower = nullptr;
    smartrowrower *smartrowRower = nullptr;
    echelonstride *echelonStride = nullptr;
    kingsmithr1protreadmill *kingsmithR1ProTreadmill = nullptr;
    kingsmithr2treadmill *kingsmithR2Treadmill = nullptr;
    ftmsbike *ftmsBike = nullptr;
    tacxneo2 *tacxneo2Bike = nullptr;
    renphobike *renphoBike = nullptr;
    heartratebelt *heartRateBelt = nullptr;
    smartspin2k *ftmsAccessory = nullptr;
    cscbike *cadenceSensor = nullptr;
    stagesbike *powerSensor = nullptr;
    eliterizer *eliteRizer = nullptr;
    QString filterDevice = QLatin1String("");

    bool testResistance = false;
    bool noWriteResistance = false;
    bool noHeartService = false;
    bool noConsole = false;
    bool logs = true;
    uint32_t pollDeviceTime = 200;
    uint8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;
    bool forceHeartBeltOffForTimeout = false;

    bool handleSignal(int signal) override;
    void stateFileUpdate();
    void stateFileRead();
    bool heartRateBeltAvaiable();
    bool ftmsAccessoryAvaiable();
    bool cscSensorAvaiable();
    bool powerSensorAvaiable();
    bool eliteRizerAvaiable();

  signals:
    void deviceConnected();
    void deviceFound(QString name);
    void searchingStop();
    void ftmsAccessoryConnected(smartspin2k *d);

  public slots:
    void restart();
    void debug(const QString &string);
    void heartRate(uint8_t heart);

  private slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    void deviceUpdated(const QBluetoothDeviceInfo &device, QBluetoothDeviceInfo::Fields updateFields);
#endif
    void canceled();
    void finished();
    void speedChanged(double);
    void inclinationChanged(double, double);
    void connectedAndDiscovered();

  signals:
};

#endif // BLUETOOTH_H
