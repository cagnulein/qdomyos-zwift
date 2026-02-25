#ifndef VIRTUALBIKE_H
#define VIRTUALBIKE_H

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
#ifndef Q_OS_ANDROID
#include <QtCore/qcoreapplication.h>
#else
#include <QtGui/qguiapplication.h>
#endif
#include <QtCore/qlist.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qtimer.h>
#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif
#include "devices/dircon/dirconmanager.h"
#include "virtualdevices/virtualdevice.h"

class virtualbike : public virtualdevice {

    Q_OBJECT
  public:
    virtualbike(bluetoothdevice *t, bool noWriteResistance = false, bool noHeartService = false,
                int8_t bikeResistanceOffset = 4, double bikeResistanceGain = 1.0);
    bool connected() override;
    bool ftmsDeviceConnected() { return lastFTMSFrameReceived != 0 || lastDirconFTMSFrameReceived != 0; }
    qint64 whenLastFTMSFrameReceived() {
        if (lastFTMSFrameReceived != 0)
            return lastFTMSFrameReceived;
        else
            return lastDirconFTMSFrameReceived;
    }
    double currentGear() {
        if(dirconManager) {
            return dirconManager->currentGear();
        }
        return writeP0003->currentGear();
    }

  private:
    QLowEnergyController *leController = nullptr;
    QLowEnergyService *serviceHR = nullptr;
    QLowEnergyService *serviceBattery = nullptr;
    QLowEnergyService *serviceFIT = nullptr;
    QLowEnergyService *service = nullptr;
    QLowEnergyService *serviceChanged = nullptr;
    QLowEnergyService *serviceWattAtomBike = nullptr;
    QLowEnergyService *serviceZwiftPlayBike = nullptr;
    QLowEnergyService *serviceDIS = nullptr;
    QLowEnergyAdvertisingData advertisingData;
    QLowEnergyServiceData serviceDataHR;
    QLowEnergyServiceData serviceDataBattery;
    QLowEnergyServiceData serviceDataFIT;
    QLowEnergyServiceData serviceData;
    QLowEnergyServiceData serviceDataChanged;
    QLowEnergyServiceData serviceEchelon;
    QLowEnergyServiceData serviceDataWattAtomBike;
    QLowEnergyServiceData serviceDataZwiftPlayBike;
    QLowEnergyServiceData serviceDataDIS;

    QTimer bikeTimer;
    bluetoothdevice *Bike;
    CharacteristicWriteProcessor2AD9 *writeP2AD9 = 0;
    CharacteristicWriteProcessor0003 *writeP0003 = 0;
    CharacteristicNotifier2AD2 *notif2AD2 = 0;
    CharacteristicNotifier2AD9 *notif2AD9 = 0;
    CharacteristicNotifier2A63 *notif2A63 = 0;
    CharacteristicNotifier2A37 *notif2A37 = 0;
    CharacteristicNotifier2A5B *notif2A5B = 0;
    CharacteristicNotifier0002 *notif0002 = 0;
    CharacteristicNotifier0004 *notif0004 = 0;

    qint64 lastFTMSFrameReceived = 0;
    qint64 lastDirconFTMSFrameReceived = 0;

    bool noHeartService = false;
    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;
    DirconManager *dirconManager = 0;
    int iFit_pelotonToBikeResistance(int pelotonResistance);
    int iFit_resistanceToIfit(int ifitResistance);
    qint64 iFit_timer = 0;
    qint64 iFit_TSLastFrame = 0;
    QByteArray iFit_LastFrameReceived;
    resistance_t iFit_LastResistanceRequested = 0;
    bool iFit_Stop = false;

    bool echelonInitDone = false;
    void echelonWriteResistance();
    void echelonWriteStatus();

    void writeCharacteristic(QLowEnergyService *service, const QLowEnergyCharacteristic &characteristic,
                             const QByteArray &value);
    
#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif
    
    // Struct to hold varint decoding result
    struct VarintResult {
       qint64 value;
       int bytesRead;
    };
    VarintResult decodeVarint(const QByteArray& bytes, int startIndex);
    qint32 decodeSInt(const QByteArray& bytes);
    
  signals:
    void changeInclination(double grade, double percentage);

    void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);

  private slots:
    void dirconFtmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void bikeProvider();
    void reconnect();
    void error(QLowEnergyController::Error newError);
};

#endif // VIRTUALBIKE_H
