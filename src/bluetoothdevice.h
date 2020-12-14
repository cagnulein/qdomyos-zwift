#ifndef BLUETOOTHDEVICE_H
#define BLUETOOTHDEVICE_H

#include <QObject>
#include <QTimer>
#include <QBluetoothDeviceInfo>

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
    QBluetoothDeviceInfo bluetoothDevice;

    enum BLUETOOTH_TYPE {
        UNKNOWN = 0,
        TREADMILL,
        BIKE,
        ROWING
    };

    virtual BLUETOOTH_TYPE deviceType();

public slots:
    virtual void start();
    virtual void stop();

protected:
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
};

#endif // BLUETOOTHDEVICE_H
