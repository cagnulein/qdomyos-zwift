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

#include "devices/discoveryoptions.h"
#include "qzsettings.h"

#ifndef NO_NORDICTRACK
#include "devices/activiotreadmill/activiotreadmill.h"
#include "devices/speraxtreadmill/speraxtreadmill.h"
#include "devices/antbike/antbike.h"
#include "devices/android_antbike/android_antbike.h"
#include "devices/apexbike/apexbike.h"
#include "devices/bhfitnesselliptical/bhfitnesselliptical.h"
#include "devices/bkoolbike/bkoolbike.h"
#endif // NO_NORDICTRACK
#include "devices/bluetoothdevice.h"
#ifndef NO_NORDICTRACK
#include "devices/bowflext216treadmill/bowflext216treadmill.h"
#include "devices/bowflextreadmill/bowflextreadmill.h"
#include "devices/chronobike/chronobike.h"
#include "devices/coresensor/coresensor.h"
#endif // NO_NORDICTRACK
#ifndef Q_OS_IOS
#include "devices/computrainerbike/computrainerbike.h"
#include "devices/csaferower/csaferower.h"
#include "devices/csafeelliptical/csafeelliptical.h"
#endif
#include "devices/cscbike/cscbike.h"
#ifndef NO_NORDICTRACK
#include "devices/concept2skierg/concept2skierg.h"
#include "devices/crossrope/crossrope.h"
#include "devices/cycleopsphantombike/cycleopsphantombike.h"
#include "devices/deeruntreadmill/deerruntreadmill.h"
#include "devices/domyosbike/domyosbike.h"
#include "devices/domyoselliptical/domyoselliptical.h"
#include "devices/domyosrower/domyosrower.h"
#include "devices/domyostreadmill/domyostreadmill.h"
#endif // NO_NORDICTRACK

#ifndef NO_NORDICTRACK
#include "devices/echelonconnectsport/echelonconnectsport.h"
#include "devices/echelonrower/echelonrower.h"
#include "devices/echelonstairclimber/echelonstairclimber.h"
#include "devices/eslinkertreadmill/eslinkertreadmill.h"
#include "devices/fakebike/fakebike.h"
#include "devices/fakeelliptical/fakeelliptical.h"
#include "devices/fakerower/fakerower.h"
#include "devices/faketreadmill/faketreadmill.h"
#include "devices/fitplusbike/fitplusbike.h"
#endif // NO_NORDICTRACK
#include "devices/eliteariafan/eliteariafan.h"
#include "devices/eliterizer/eliterizer.h"
#include "devices/elitesquarecontroller/elitesquarecontroller.h"
#include "devices/elitesterzosmart/elitesterzosmart.h"

#include "devices/fitmetria_fanfit/fitmetria_fanfit.h"
#include "devices/heartratebelt/heartratebelt.h"
#ifndef NO_NORDICTRACK
#include "devices/fitshowtreadmill/fitshowtreadmill.h"
#include "devices/flywheelbike/flywheelbike.h"
#include "devices/ftmsbike/ftmsbike.h"
#include "devices/ftmsrower/ftmsrower.h"
#include "devices/focustreadmill/focustreadmill.h"
#include "devices/horizongr7bike/horizongr7bike.h"
#include "devices/horizontreadmill/horizontreadmill.h"
#include "devices/iconceptbike/iconceptbike.h"
#include "devices/iconceptelliptical/iconceptelliptical.h"
#include "devices/inspirebike/inspirebike.h"
#include "devices/keepbike/keepbike.h"
#include "devices/kineticinroadbike/kineticinroadbike.h"
#include "devices/kingsmithr1protreadmill/kingsmithr1protreadmill.h"
#include "devices/kingsmithr2treadmill/kingsmithr2treadmill.h"
#include "devices/lifefitnesstreadmill/lifefitnesstreadmill.h"
#include "devices/lifespantreadmill/lifespantreadmill.h"
#include "devices/m3ibike/m3ibike.h"
#include "devices/mcfbike/mcfbike.h"
#include "devices/mepanelbike/mepanelbike.h"
#include "devices/moxy5sensor/moxy5sensor.h"
#include "devices/nautilusbike/nautilusbike.h"
#include "devices/nautiluselliptical/nautiluselliptical.h"
#include "devices/nautilustreadmill/nautilustreadmill.h"
#include "devices/nordictrackelliptical/nordictrackelliptical.h"
#endif // NO_NORDICTRACK
#include "devices/nordictrackifitadbbike/nordictrackifitadbbike.h"
#ifndef NO_NORDICTRACK  
#include "devices/nordictrackifitadbelliptical/nordictrackifitadbelliptical.h"
#endif // NO_NORDICTRACK
#include "devices/nordictrackifitadbtreadmill/nordictrackifitadbtreadmill.h"
#ifndef NO_NORDICTRACK
#include "devices/npecablebike/npecablebike.h"
#include "devices/octaneelliptical/octaneelliptical.h"
#include "devices/octanetreadmill/octanetreadmill.h"
#include "devices/pafersbike/pafersbike.h"
#include "devices/paferstreadmill/paferstreadmill.h"
#include "devices/pelotonbike/pelotonbike.h"
#include "devices/pitpatbike/pitpatbike.h"
#include "devices/proformbike/proformbike.h"
#include "devices/proformelliptical/proformelliptical.h"
#include "devices/proformellipticaltrainer/proformellipticaltrainer.h"
#include "devices/proformrower/proformrower.h"
#include "devices/proformtreadmill/proformtreadmill.h"
#include "devices/proformtelnetbike/proformtelnetbike.h"
#include "devices/proformwifibike/proformwifibike.h"
#include "devices/proformwifitreadmill/proformwifitreadmill.h"
#include "devices/schwinn170bike/schwinn170bike.h"
#include "devices/schwinnic4bike/schwinnic4bike.h"
#endif // NO_NORDICTRACK
#include "signalhandler.h"
#include "devices/smartspin2k/smartspin2k.h"
#ifndef NO_NORDICTRACK
#include "devices/skandikawiribike/skandikawiribike.h"
#include "devices/smartrowrower/smartrowrower.h"

#include "devices/snodebike/snodebike.h"
#endif // NO_NORDICTRACK
#include "devices/strydrunpowersensor/strydrunpowersensor.h"
#ifndef NO_NORDICTRACK
#include "devices/shuaa5treadmill/shuaa5treadmill.h"
#include "devices/solebike/solebike.h"
#include "devices/soleelliptical/soleelliptical.h"
#include "devices/solef80treadmill/solef80treadmill.h"
#endif // NO_NORDICTRACK

#ifndef NO_NORDICTRACK
#include "devices/spirittreadmill/spirittreadmill.h"
#include "devices/sportsplusbike/sportsplusbike.h"
#include "devices/sportsplusrower/sportsplusrower.h"
#include "devices/sportstechbike/sportstechbike.h"
#include "devices/sportstechelliptical/sportstechelliptical.h"
#endif // NO_NORDICTRACK
#include "devices/sramAXSController/sramAXSController.h"
#include "devices/stagesbike/stagesbike.h"
#ifndef NO_NORDICTRACK
#include "devices/renphobike/renphobike.h"
#include "devices/tacxneo2/tacxneo2.h"
#include "devices/technogymmyruntreadmill/technogymmyruntreadmill.h"
#include "devices/technogymmyruntreadmillrfcomm/technogymmyruntreadmillrfcomm.h"
#endif // NO_NORDICTRACK

#ifndef NO_NORDICTRACK
#include "devices/echelonstride/echelonstride.h"
#endif // NO_NORDICTRACK

#include "templateinfosenderbuilder.h"
#ifndef NO_NORDICTRACK
#include "technogymbike/technogymbike.h"
#include "devices/toorxtreadmill/toorxtreadmill.h"
#endif // NO_NORDICTRACK
#include "devices/treadmill.h"
#ifndef NO_NORDICTRACK
#include "devices/truetreadmill/truetreadmill.h"
#include "devices/trxappgateusbbike/trxappgateusbbike.h"
#include "devices/trxappgateusbelliptical/trxappgateusbelliptical.h"
#include "devices/trxappgateusbrower/trxappgateusbrower.h"
#include "devices/trxappgateusbtreadmill/trxappgateusbtreadmill.h"
#include "devices/ultrasportbike/ultrasportbike.h"

#include "devices/wahookickrsnapbike/wahookickrsnapbike.h"
#include "devices/yesoulbike/yesoulbike.h"
#endif // NO_NORDICTRACK
#ifndef NO_NORDICTRACK
#include "devices/ypooelliptical/ypooelliptical.h"
#include "devices/ziprotreadmill/ziprotreadmill.h"
#endif // NO_NORDICTRACK
#include "devices/wahookickrheadwind/wahookickrheadwind.h"
#include "zwift_play/zwiftPlayDevice.h"
#include "zwift_play/zwiftclickremote.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class bluetooth : public QObject, public SignalHandler {

    Q_OBJECT
  public:
    bluetooth(const discoveryoptions &options);
    explicit bluetooth(bool logs, const QString &deviceName = QLatin1String(""), bool noWriteResistance = false,
                       bool noHeartService = false, uint32_t pollDeviceTime = 200, bool noConsole = false,
                       bool testResistance = false, int8_t bikeResistanceOffset = 4, double bikeResistanceGain = 1.0,
                       bool startDiscovery = true);
    ~bluetooth();
    bluetoothdevice *device();
    bluetoothdevice *externalInclination() { return eliteRizer; }
    bluetoothdevice *heartRateDevice() { return heartRateBelt; }
    QList<QBluetoothDeviceInfo> devices;
    bool onlyDiscover = false;
    volatile bool homeformLoaded = false;

  private:
    bool useDiscovery = false;
    QFile *debugCommsLog = nullptr;
    QBluetoothDeviceDiscoveryAgent *discoveryAgent = nullptr;
    nordictrackifitadbtreadmill *nordictrackifitadbTreadmill = nullptr;
    nordictrackifitadbbike *nordictrackifitadbBike = nullptr;

    #ifndef NO_NORDICTRACK
    antbike *antBike = nullptr;
    android_antbike *android_antBike = nullptr;
    apexbike *apexBike = nullptr;
    bkoolbike *bkoolBike = nullptr;
    bhfitnesselliptical *bhFitnessElliptical = nullptr;
    bowflextreadmill *bowflexTreadmill = nullptr;
    bowflext216treadmill *bowflexT216Treadmill = nullptr;
    coresensor* coreSensor = nullptr;
    crossrope *crossRope = nullptr;
    fitshowtreadmill *fitshowTreadmill = nullptr;
    focustreadmill *focusTreadmill = nullptr;
#ifndef Q_OS_IOS
    computrainerbike *computrainerBike = nullptr;
    csaferower *csafeRower = nullptr;
    csafeelliptical *csafeElliptical = nullptr;
#endif
    concept2skierg *concept2Skierg = nullptr;
    cycleopsphantombike *cycleopsphantomBike = nullptr;
    deerruntreadmill *deerrunTreadmill = nullptr;
    domyostreadmill *domyos = nullptr;
    domyosbike *domyosBike = nullptr;
    domyosrower *domyosRower = nullptr;
    domyoselliptical *domyosElliptical = nullptr;
    toorxtreadmill *toorx = nullptr;
    iconceptbike *iConceptBike = nullptr;
    iconceptelliptical *iConceptElliptical = nullptr;
    trxappgateusbtreadmill *trxappgateusb = nullptr;
    spirittreadmill *spiritTreadmill = nullptr;
    activiotreadmill *activioTreadmill = nullptr;
    speraxtreadmill *speraXTreadmill = nullptr;
    nautilusbike *nautilusBike = nullptr;
    nautiluselliptical *nautilusElliptical = nullptr;
    nautilustreadmill *nautilusTreadmill = nullptr;
    trxappgateusbbike *trxappgateusbBike = nullptr;
    trxappgateusbrower *trxappgateusbRower = nullptr;
    trxappgateusbelliptical *trxappgateusbElliptical = nullptr;
    echelonconnectsport *echelonConnectSport = nullptr;
    yesoulbike *yesoulBike = nullptr;
    flywheelbike *flywheelBike = nullptr;
    moxy5sensor *moxy5Sensor = nullptr;
    nordictrackelliptical *nordictrackElliptical = nullptr;
    nordictrackifitadbelliptical *nordictrackifitadbElliptical = nullptr;
    octaneelliptical *octaneElliptical = nullptr;
    octanetreadmill *octaneTreadmill = nullptr;
    pelotonbike *pelotonBike = nullptr;
    proformrower *proformRower = nullptr;
    proformbike *proformBike = nullptr;
    proformtelnetbike *proformTelnetBike = nullptr;
    proformwifibike *proformWifiBike = nullptr;
    proformwifitreadmill *proformWifiTreadmill = nullptr;
    proformelliptical *proformElliptical = nullptr;
    proformellipticaltrainer *proformEllipticalTrainer = nullptr;
    proformtreadmill *proformTreadmill = nullptr;
    horizontreadmill *horizonTreadmill = nullptr;
    technogymmyruntreadmill *technogymmyrunTreadmill = nullptr;
#ifndef Q_OS_IOS
    technogymmyruntreadmillrfcomm *technogymmyrunrfcommTreadmill = nullptr;
#endif
    truetreadmill *trueTreadmill = nullptr;
    horizongr7bike *horizonGr7Bike = nullptr;
    schwinnic4bike *schwinnIC4Bike = nullptr;
    technogymbike* technogymBike = nullptr;
    sportstechbike *sportsTechBike = nullptr;
    sportstechelliptical *sportsTechElliptical = nullptr;
    sportsplusbike *sportsPlusBike = nullptr;
    sportsplusrower *sportsPlusRower = nullptr;
    inspirebike *inspireBike = nullptr;
    snodebike *snodeBike = nullptr;
    eslinkertreadmill *eslinkerTreadmill = nullptr;
    m3ibike *m3iBike = nullptr;
    mepanelbike *mepanelBike = nullptr;
    skandikawiribike *skandikaWiriBike = nullptr;
#endif
    cscbike *cscBike = nullptr;
#ifndef NO_NORDICTRACK
    mcfbike *mcfBike = nullptr;
    npecablebike *npeCableBike = nullptr;
#endif
    stagesbike *stagesBike = nullptr;
#ifndef NO_NORDICTRACK
    solebike *soleBike = nullptr;
    soleelliptical *soleElliptical = nullptr;
    solef80treadmill *soleF80 = nullptr;
    schwinn170bike *schwinn170Bike = nullptr;
    chronobike *chronoBike = nullptr;
    fitplusbike *fitPlusBike = nullptr;
    echelonrower *echelonRower = nullptr;
    ftmsrower *ftmsRower = nullptr;
    smartrowrower *smartrowRower = nullptr;
    echelonstride *echelonStride = nullptr;
    echelonstairclimber *echelonStairclimber = nullptr;
    lifefitnesstreadmill *lifefitnessTreadmill = nullptr;
    lifespantreadmill *lifespanTreadmill = nullptr;
    keepbike *keepBike = nullptr;
    kingsmithr1protreadmill *kingsmithR1ProTreadmill = nullptr;
    kingsmithr2treadmill *kingsmithR2Treadmill = nullptr;
    ftmsbike *ftmsBike = nullptr;
    pafersbike *pafersBike = nullptr;
    paferstreadmill *pafersTreadmill = nullptr;
    tacxneo2 *tacxneo2Bike = nullptr;
    pitpatbike *pitpatBike = nullptr;
    renphobike *renphoBike = nullptr;
    shuaa5treadmill *shuaA5Treadmill = nullptr;
#endif
    heartratebelt *heartRateBelt = nullptr;
    smartspin2k *ftmsAccessory = nullptr;
    cscbike *cadenceSensor = nullptr;
    stagesbike *powerSensor = nullptr;
    strydrunpowersensor *powerSensorRun = nullptr;
    stagesbike *powerBike = nullptr;
#ifndef NO_NORDICTRACK
    ultrasportbike *ultraSportBike = nullptr;
    wahookickrsnapbike *wahooKickrSnapBike = nullptr;
    ypooelliptical *ypooElliptical = nullptr;
    ziprotreadmill *ziproTreadmill = nullptr;
    kineticinroadbike *kineticInroadBike = nullptr;
#endif
    strydrunpowersensor *powerTreadmill = nullptr;
    eliterizer *eliteRizer = nullptr;
    elitesterzosmart *eliteSterzoSmart = nullptr;
#ifndef NO_NORDICTRACK
    fakebike *fakeBike = nullptr;
    fakeelliptical *fakeElliptical = nullptr;
    fakerower *fakeRower = nullptr;
    faketreadmill *fakeTreadmill = nullptr;
#endif
    QList<fitmetria_fanfit *> fitmetriaFanfit;
    QList<wahookickrheadwind *> wahookickrHeadWind;
    QList<eliteariafan *> eliteAriaFan;
    QList<zwiftclickremote* > zwiftPlayDevice;
    zwiftclickremote* zwiftClickRemote = nullptr;
    sramaxscontroller* sramAXSController = nullptr;
    elitesquarecontroller* eliteSquareController = nullptr;
    QString filterDevice = QLatin1String("");

    bool testResistance = false;
    bool noWriteResistance = false;
    bool noHeartService = false;
    bool noConsole = false;
    bool logs = true;
    uint32_t pollDeviceTime = 200;
    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;
    bool forceHeartBeltOffForTimeout = false;

    /**
     * @brief Start the Bluetooth discovery agent.
     */
    void startDiscovery();

    /**
     * @brief Stop the Bluetooth discovery agent.
     */
    void stopDiscovery();

    bool handleSignal(int signal) override;
    bool deviceHasService(const QBluetoothDeviceInfo &device, QBluetoothUuid service);
    void stateFileUpdate();
    void stateFileRead();
    bool heartRateBeltAvaiable();
    bool ftmsAccessoryAvaiable();
    bool cscSensorAvaiable();
    bool powerSensorAvaiable();
    bool eliteRizerAvaiable();
    bool eliteSterzoSmartAvaiable();
    bool fitmetriaFanfitAvaiable();
    bool zwiftDeviceAvaiable();
    bool sramDeviceAvaiable();
    bool fitmetria_fanfit_isconnected(QString name);

#ifdef Q_OS_WIN
    QTimer discoveryTimeout;
#endif

#ifdef Q_OS_IOS
    lockscreen *h = nullptr;
#endif

    /**
     * @brief Store the name and other info in the settings.
     * @param b The bluetooth device info.
     */
    void setLastBluetoothDevice(const QBluetoothDeviceInfo &b);
    void signalBluetoothDeviceConnected(bluetoothdevice *b);
  signals:
    void deviceConnected(QBluetoothDeviceInfo b);
    void deviceFound(QString name);
    void searchingStop();
    void ftmsAccessoryConnected(smartspin2k *d);

    void bluetoothDeviceConnected(bluetoothdevice *b);
    void bluetoothDeviceDisconnected();
  public slots:
    void restart();
    void debug(const QString &string);
    void heartRate(uint8_t heart);
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
  private slots:
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    void deviceUpdated(const QBluetoothDeviceInfo &device, QBluetoothDeviceInfo::Fields updateFields);
#endif
    void canceled();
    void finished();
    void speedChanged(double);
    void inclinationChanged(double, double);
    void connectedAndDiscovered();
    void gearDown();
    void gearUp();
    void gearFailedDown();
    void gearFailedUp();

  signals:
};

#endif // BLUETOOTH_H
