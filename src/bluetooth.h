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
#include "cscbike.h"
#include "eliterizer.h"
#include "elitesterzosmart.h"
#include "fitmetria_fanfit.h"
#include "heartratebelt.h"
#include "strydrunpowersensor.h"
#include "signalhandler.h"
#include "smartspin2k.h"
#include "stagesbike.h"
#include "templateinfosenderbuilder.h"


class bluetooth : public QObject, public SignalHandler {

    Q_OBJECT
  public:
    explicit bluetooth(bool logs, const QString &deviceName = QLatin1String(""), bool noWriteResistance = false,
                       bool noHeartService = false, uint32_t pollDeviceTime = 200, bool noConsole = false,
                       bool testResistance = false, uint8_t bikeResistanceOffset = 4, double bikeResistanceGain = 1.0);
    ~bluetooth();

    /**
     * @brief device Gets the currently active device.
     * @return
     */
    bluetoothdevice *device();

    /**
     * @brief device Gets the currently active device object dynamically cast to the specified type.
     * @return Null if the device is not of the specified type, otherwise the current device object cast to the specified type.
     */
    template <typename T> T *device() { return dynamic_cast<T*>(this->device()); }

    bluetoothdevice *externalInclination() { return eliteRizer; }
    bluetoothdevice *heartRateDevice() { return heartRateBelt; }
    QList<QBluetoothDeviceInfo> devices;
    bool onlyDiscover = false;
    TemplateInfoSenderBuilder *getUserTemplateManager() const { return userTemplateManager; }
    TemplateInfoSenderBuilder *getInnerTemplateManager() const { return innerTemplateManager; }

  private:
    TemplateInfoSenderBuilder *userTemplateManager = nullptr;
    TemplateInfoSenderBuilder *innerTemplateManager = nullptr;
    QFile *debugCommsLog = nullptr;
    QBluetoothDeviceDiscoveryAgent *discoveryAgent = nullptr;

    bluetoothdevice * bluetoothDevice = nullptr;
    cscbike *cadenceSensor = nullptr;
    stagesbike *powerSensor = nullptr;
    strydrunpowersensor *powerSensorRun = nullptr;
    smartspin2k *ftmsAccessory = nullptr;
    heartratebelt *heartRateBelt = nullptr;
    eliterizer *eliteRizer = nullptr;
    elitesterzosmart *eliteSterzoSmart = nullptr;

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

    /**
     * @brief Start the Bluetooth discovery agent.
     */
    void startDiscovery();

    /**
     * @brief Stop the Bluetooth discovery agent.
     */
    void stopDiscovery();

    bool handleSignal(int signal) override;
    void stateFileUpdate();
    void stateFileRead();
    bool heartRateBeltAvaiable();
    bool ftmsAccessoryAvaiable();
    bool cscSensorAvaiable();
    bool powerSensorAvaiable();
    bool eliteRizerAvaiable();
    bool eliteSterzoSmartAvaiable();
    bool fitmetria_fanfit_isconnected(QString name);

#ifdef Q_OS_WIN
    QTimer discoveryTimeout;
#endif

    /**
     * @brief Store the name and other info in the settings.
     * @param b The bluetooth device info.
     */
    void setLastBluetoothDevice(const QBluetoothDeviceInfo &b);
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
