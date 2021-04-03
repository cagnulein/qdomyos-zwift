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
#include "metric.h"

#if defined(Q_OS_IOS)
#define SAME_BLUETOOTH_DEVICE(d1, d2) (d1.deviceUuid() == d2.deviceUuid())
#else
#define SAME_BLUETOOTH_DEVICE(d1, d2) (d1.address() == d2.address())
#endif

class bluetoothdevice : public QObject
{
    Q_OBJECT
public:
    bluetoothdevice();
    virtual metric currentHeart();
    virtual metric currentSpeed();
    virtual QTime currentPace();
    virtual double odometer();
    virtual double calories();
    metric jouls();
    virtual uint8_t fanSpeed();
    virtual QTime elapsedTime();
    virtual QTime lapElapsedTime();
    virtual bool connected();
    virtual void* VirtualDevice();
    uint16_t watts(double weight);
    metric wattsMetric();
    virtual bool changeFanSpeed(uint8_t speed);
    virtual double elevationGain();
    virtual void clearStats();
    QBluetoothDeviceInfo bluetoothDevice;
    void disconnectBluetooth();
    virtual void setPaused(bool p);
    bool isPaused() {return paused;}
    virtual void setLap();
    void setAutoResistance(bool value) {autoResistanceEnable = value;}
    bool autoResistance() {return autoResistanceEnable;}
    void setDifficult(double d);
    double difficult();

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

    metric elapsed;
    metric Speed;
    metric KCal;
    metric Distance;
    uint8_t FanSpeed = 0;
    metric Heart;
    int8_t requestStart = -1;
    int8_t requestStop = -1;
    int8_t requestIncreaseFan = -1;
    int8_t requestDecreaseFan = -1;
    double m_difficult = 1.0;
    metric m_jouls;
    double elevationAcc = 0;
    metric m_watt;

    bool paused = false;
    bool autoResistanceEnable = true;
};

#endif // BLUETOOTHDEVICE_H
