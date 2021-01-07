#ifndef BLUETOOTHDEVICE_H
#define BLUETOOTHDEVICE_H

#include <QObject>
#include <QTimer>
#include <QBluetoothDeviceInfo>
#include <QtBluetooth/qlowenergyadvertisingdata.h>
#include <QtBluetooth/qlowenergyadvertisingparameters.h>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtBluetooth/qlowenergycharacteristicdata.h>
#include <QtBluetooth/qlowenergydescriptordata.h>
#include <QtBluetooth/qlowenergycontroller.h>
#include <QtBluetooth/qlowenergyservice.h>
#include <QtBluetooth/qlowenergyservicedata.h>
#include <QBluetoothDeviceDiscoveryAgent>

class bluetoothdevice : public QObject
{
    Q_OBJECT
public:
    bluetoothdevice();
    virtual unsigned char currentHeart();
    virtual double currentSpeed();
    virtual QTime currentPace();
    virtual double odometer();
    virtual double calories();
    double jouls();
    virtual uint8_t fanSpeed();
    virtual QTime elapsedTime();
    virtual bool connected();
    virtual void* VirtualDevice();
    uint16_t watts(double weight);
    virtual bool changeFanSpeed(uint8_t speed);
    virtual uint8_t pelotonResistance();
    virtual uint8_t avgPelotonResistance();
    virtual double elevationGain();
    QBluetoothDeviceInfo bluetoothDevice;
    double avgWatt();
    void disconnect();

    uint8_t maxPelotonResistance = 0;

    enum BLUETOOTH_TYPE {
        UNKNOWN = 0,
        TREADMILL,
        BIKE,
        ROWING,
        ELLIPTICAL
    };

    virtual BLUETOOTH_TYPE deviceType();

public slots:
    virtual void start();
    virtual void stop();
    virtual void heartRate(uint8_t heart);

signals:
    void connectedAndDiscovered();

protected:
    QLowEnergyController* m_control = 0;

    double elapsed = 0;
    double Speed = 0;
    double KCal = 0;
    double Distance = 0;
    uint8_t FanSpeed = 0;
    uint8_t Heart = 0;
    int8_t requestStart = -1;
    int8_t requestStop = -1;
    int8_t requestIncreaseFan = -1;
    int8_t requestDecreaseFan = -1;
    double m_jouls = 0;
    uint8_t m_pelotonResistance = 0;
    double elevationAcc = 0;

    uint64_t totPower = 0;
    uint32_t countPower = 0;

    uint64_t totPelotonResistance = 0;
    uint32_t countPelotonResistance = 0;
};

#endif // BLUETOOTHDEVICE_H
