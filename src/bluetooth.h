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

#include "activiotreadmill.h"
#include "bhfitnesselliptical.h"
#include "bluetoothdevice.h"
#include "bowflext216treadmill.h"
#include "bowflextreadmill.h"
#include "chronobike.h"
#include "concept2skierg.h"
#include "cscbike.h"
#include "domyosbike.h"
#include "domyoselliptical.h"
#include "domyosrower.h"
#include "domyostreadmill.h"

#include "echelonconnectsport.h"
#include "echelonrower.h"
#include "eliterizer.h"
#include "elitesterzosmart.h"
#include "eslinkertreadmill.h"
#include "fakebike.h"
#include "fakeelliptical.h"
#include "fitmetria_fanfit.h"
#include "fitplusbike.h"

#include "fitshowtreadmill.h"
#include "flywheelbike.h"
#include "ftmsbike.h"
#include "ftmsrower.h"
#include "heartratebelt.h"
#include "horizongr7bike.h"
#include "horizontreadmill.h"
#include "iconceptbike.h"
#include "inspirebike.h"
#include "keepbike.h"
#include "kingsmithr1protreadmill.h"
#include "kingsmithr2treadmill.h"
#include "m3ibike.h"
#include "mcfbike.h"
#include "nautilusbike.h"
#include "nautiluselliptical.h"
#include "nautilustreadmill.h"
#include "nordictrackelliptical.h"
#include "nordictrackifitadbtreadmill.h"
#include "npecablebike.h"
#include "octanetreadmill.h"
#include "pafersbike.h"
#include "paferstreadmill.h"
#include "proformbike.h"
#include "proformelliptical.h"
#include "proformellipticaltrainer.h"
#include "proformrower.h"
#include "proformtreadmill.h"
#include "proformwifibike.h"
#include "proformwifitreadmill.h"
#include "schwinnic4bike.h"
#include "signalhandler.h"
#include "skandikawiribike.h"
#include "smartrowrower.h"
#include "smartspin2k.h"
#include "snodebike.h"
#include "strydrunpowersensor.h"

#include "shuaa5treadmill.h"
#include "solebike.h"
#include "soleelliptical.h"
#include "solef80treadmill.h"

#include "spirittreadmill.h"
#include "sportsplusbike.h"
#include "sportstechbike.h"
#include "stagesbike.h"

#include "renphobike.h"
#include "tacxneo2.h"
#include "technogymmyruntreadmill.h"
#include "technogymmyruntreadmillrfcomm.h"

#include "echelonstride.h"

#include "templateinfosenderbuilder.h"
#include "toorxtreadmill.h"
#include "treadmill.h"
#include "truetreadmill.h"
#include "trxappgateusbbike.h"
#include "trxappgateusbtreadmill.h"
#include "ultrasportbike.h"
#include "wahookickrsnapbike.h"
#include "yesoulbike.h"

class bluetooth : public QObject, public SignalHandler {

    Q_OBJECT
  public:
    explicit bluetooth(bool logs, const QString &deviceName = QLatin1String(""), bool noWriteResistance = false,
                       bool noHeartService = false, uint32_t pollDeviceTime = 200, bool noConsole = false,
                       bool testResistance = false, uint8_t bikeResistanceOffset = 4, double bikeResistanceGain = 1.0);
    ~bluetooth();
    bluetoothdevice *device();
    bluetoothdevice *externalInclination() { return eliteRizer; }
    bluetoothdevice *heartRateDevice() { return this->heartRateBelt; }
    QList<QBluetoothDeviceInfo> devices;
    bool onlyDiscover = false;
    TemplateInfoSenderBuilder *getUserTemplateManager() const { return userTemplateManager; }
    TemplateInfoSenderBuilder *getInnerTemplateManager() const { return innerTemplateManager; }

  private:
    TemplateInfoSenderBuilder *userTemplateManager = nullptr;
    TemplateInfoSenderBuilder *innerTemplateManager = nullptr;
    QFile *debugCommsLog = nullptr;
    QBluetoothDeviceDiscoveryAgent *discoveryAgent;
    bluetoothdevice *bluetoothDevice = nullptr;


    heartratebelt *heartRateBelt = nullptr;
    smartspin2k *ftmsAccessory = nullptr;
    eliterizer *eliteRizer = nullptr;
    elitesterzosmart *eliteSterzoSmart = nullptr;
    cscbike *cadenceSensor = nullptr;
    bluetoothdevice *powerSensor = nullptr;

    QList<fitmetria_fanfit *> fitmetriaFanfit;
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
    bool heartRateBeltAvailable();
    bool ftmsAccessoryAvailable();
    bool cscSensorAvailable();
    bool powerSensorAvailable();
    bool eliteRizerAvailable();
    bool eliteSterzoSmartAvailable();
    bool fitmetria_fanfit_isconnected(QString name);

#ifdef Q_OS_WIN
    QTimer discoveryTimeout;
#endif

    // ------------------------------------------------------------------------------------------------------
    // Exercise machine detectors
    bluetoothdevice *detect_m3iBike(const QBluetoothDeviceInfo &b, bool filter, QSettings& settings);
    bluetoothdevice *detect_fakeBike(const QBluetoothDeviceInfo &b, bool filter, QSettings& settings);
    bluetoothdevice *detect_fakeElliptical(const QBluetoothDeviceInfo &b, bool filter, QSettings& settings);
    bluetoothdevice *detect_proformWifiBike(const QBluetoothDeviceInfo &b, bool filter, QSettings& settings);
    bluetoothdevice *detect_bhfitnesselliptical(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_bowflextreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_bowflext216treadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_fitshowtreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_concept2skierg(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_domyostreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_domyosbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_domyosrower(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_domyoselliptical(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_toorxtreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_iconceptbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_trxappgateusbtreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_spirittreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_activiotreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_nautilusbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_nautiluselliptical(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_nautilustreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_trxappgateusbbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_echelonconnectsport(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_yesoulbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_flywheelbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_nordictrackelliptical(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_nordictrackifitadbtreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_octanetreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_proformrower(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_proformbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_proformwifibike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_proformwifitreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_proformelliptical(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_proformellipticaltrainer(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_proformtreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_horizontreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_technogymmyruntreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_technogymmyruntreadmillrfcomm(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_truetreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_horizongr7bike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_schwinnic4bike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_sportstechbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_sportsplusbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_inspirebike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_snodebike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_eslinkertreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_m3ibike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_skandikawiribike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_cscbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_mcfbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_npecablebike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_stagesbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_solebike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_soleelliptical(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_solef80treadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_chronobike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_fitplusbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_echelonrower(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_ftmsrower(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_smartrowrower(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_echelonstride(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_keepbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_kingsmithr1protreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_kingsmithr2treadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_ftmsbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_pafersbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_paferstreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_tacxneo2(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_renphobike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_shuaa5treadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_ultrasportbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_wahookickrsnapbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_strydrunpowersensor(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_fakebike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_fakeelliptical(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings);
    bluetoothdevice *detect_powerBike(const QBluetoothDeviceInfo &b, bool filter, QSettings& settings);
    bluetoothdevice *detect_powerTreadmill(const QBluetoothDeviceInfo &b, bool filter, QSettings& settings);
    // ------------------------------------------------------------------------------------------------------

    // ------------------------------------------------------------------------------------------------------
    // special device detectors
    elitesterzosmart *detect_elitesterzosmart(const QBluetoothDeviceInfo& b, QSettings& settings);
    eliterizer *detect_eliterizer(const QBluetoothDeviceInfo& b, QSettings& settings);
    smartspin2k *detect_smartspin2k(const QBluetoothDeviceInfo& b,  QSettings& settings);
    heartratebelt *detect_heartratebelt(const QBluetoothDeviceInfo& b, QSettings& settings);
    cscbike *detect_cadenceSensor(const QBluetoothDeviceInfo &b, QSettings &settings);
    bluetoothdevice *detect_powerSensor(const QBluetoothDeviceInfo &b, QSettings &settings);
    // ------------------------------------------------------------------------------------------------------

    bool toorxBikeAvaiable(const QSettings &settings);


signals:
    void deviceConnected(QBluetoothDeviceInfo b);
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
