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

    // Function to send mouse input reports
    void sendMouseReport(int8_t x, int8_t y, uint8_t buttons = 0, int8_t wheel = 0) {
        if (!serviceHID || !hidEnabled || leController->state() != QLowEnergyController::ConnectedState) {
            qDebug() << "HID service not available or not connected";
            return;
        }

        QByteArray report;
        report.append(buttons);  // Button state
        report.append(x);        // X movement
        report.append(y);        // Y movement
        report.append(wheel);    // Wheel movement

               // Find the report characteristic
        QLowEnergyCharacteristic reportChar = serviceHID->characteristic(QBluetoothUuid(quint16(0x2A4D)));
        if (!reportChar.isValid()) {
            qDebug() << "HID report characteristic not found";
            return;
        }

        writeCharacteristic(serviceHID, reportChar, report);
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
    QLowEnergyAdvertisingData advertisingData;
    QLowEnergyServiceData serviceDataHR;
    QLowEnergyServiceData serviceDataBattery;
    QLowEnergyServiceData serviceDataFIT;
    QLowEnergyServiceData serviceData;
    QLowEnergyServiceData serviceDataChanged;
    QLowEnergyServiceData serviceEchelon;
    QLowEnergyServiceData serviceDataWattAtomBike;
    QLowEnergyServiceData serviceDataZwiftPlayBike;
    QTimer bikeTimer;
    bluetoothdevice *Bike;
    CharacteristicWriteProcessor2AD9 *writeP2AD9 = 0;
    CharacteristicNotifier2AD2 *notif2AD2 = 0;
    CharacteristicNotifier2AD9 *notif2AD9 = 0;
    CharacteristicNotifier2A63 *notif2A63 = 0;
    CharacteristicNotifier2A37 *notif2A37 = 0;
    CharacteristicNotifier2A5B *notif2A5B = 0;

    qint64 lastFTMSFrameReceived = 0;
    qint64 lastDirconFTMSFrameReceived = 0;

    bool noHeartService = false;
    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;
    DirconManager *dirconManager = 0;
    uint8_t CurrentZwiftGear = 8;
    int iFit_pelotonToBikeResistance(int pelotonResistance);
    int iFit_resistanceToIfit(int ifitResistance);
    qint64 iFit_timer = 0;
    qint64 iFit_TSLastFrame = 0;
    QByteArray iFit_LastFrameReceived;
    resistance_t iFit_LastResistanceRequested = 0;
    bool iFit_Stop = false;

    void handleZwiftGear(const QByteArray &array);

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

    QLowEnergyServiceData serviceDataHID;
    QLowEnergyService* serviceHID = nullptr;
    bool hidEnabled = false;

    // HID report descriptor for mouse
    QByteArray getHIDReportDescriptor() {
       static const char reportDesc[] = {
           0x05, 0x01,  // Usage Page (Generic Desktop)
           0x09, 0x02,  // Usage (Mouse)
           static_cast<char>(0xA1), 0x01,  // Collection (Application)
           0x09, 0x01,  //   Usage (Pointer)
           static_cast<char>(0xA1), 0x00,  //   Collection (Physical)
           0x05, 0x09,  //     Usage Page (Button)
           0x19, 0x01,  //     Usage Minimum (1)
           0x29, 0x03,  //     Usage Maximum (3)
           0x15, 0x00,  //     Logical Minimum (0)
           0x25, 0x01,  //     Logical Maximum (1)
           static_cast<char>(0x95), 0x03,  //     Report Count (3)
           0x75, 0x01,  //     Report Size (1)
           static_cast<char>(0x81), 0x02,  //     Input (Data, Variable, Absolute)
           static_cast<char>(0x95), 0x01,  //     Report Count (1)
           0x75, 0x05,  //     Report Size (5)
           static_cast<char>(0x81), 0x01,  //     Input (Constant)
           0x05, 0x01,  //     Usage Page (Generic Desktop)
           0x09, 0x30,  //     Usage (X)
           0x09, 0x31,  //     Usage (Y)
           0x09, 0x38,  //     Usage (Wheel)
           0x15, static_cast<char>(0x81),  //     Logical Minimum (-127)
           0x25, 0x7F,  //     Logical Maximum (127)
           0x75, 0x08,  //     Report Size (8)
           static_cast<char>(0x95), 0x03,  //     Report Count (3)
           static_cast<char>(0x81), 0x06,  //     Input (Data, Variable, Relative)
           static_cast<char>(0xC0),        //   End Collection
           static_cast<char>(0xC0)         // End Collection
       };
       return QByteArray(reportDesc, sizeof(reportDesc));
    }
    
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
