#ifndef BLUETOOTHDEVICE_H
#define BLUETOOTHDEVICE_H

#include "metric.h"
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QDateTime>
#include <QGeoCoordinate>
#include <QObject>
#include <QTimer>

#include <QtBluetooth/qlowenergyadvertisingdata.h>
#include <QtBluetooth/qlowenergyadvertisingparameters.h>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtBluetooth/qlowenergycharacteristicdata.h>

#include <QtBluetooth/qlowenergycontroller.h>
#include <QtBluetooth/qlowenergydescriptordata.h>
#include <QtBluetooth/qlowenergyservice.h>
#include <QtBluetooth/qlowenergyservicedata.h>

#if defined(Q_OS_IOS)
#define SAME_BLUETOOTH_DEVICE(d1, d2) (d1.deviceUuid() == d2.deviceUuid())
#else
#define SAME_BLUETOOTH_DEVICE(d1, d2) (d1.address() == d2.address())
#endif

class bluetoothdevice : public QObject {

    Q_OBJECT
  public:
    bluetoothdevice();
    virtual metric currentHeart();
    virtual metric currentSpeed();
    virtual QTime currentPace();
    virtual metric currentInclination();
    void setInclination(double inclination);
    virtual QTime averagePace();
    virtual QTime maxPace();
    virtual double odometer();
    virtual metric calories();
    metric jouls();
    virtual uint8_t fanSpeed();
    virtual QTime elapsedTime();
    virtual void offsetElapsedTime(int offset);
    virtual QTime movingTime();
    virtual QTime lapElapsedTime();
    virtual bool connected();
    virtual metric currentResistance();
    virtual metric currentCadence();
    virtual double currentCrankRevolutions();
    virtual QGeoCoordinate currentCordinate();
    virtual uint16_t lastCrankEventTime();
    virtual void *VirtualDevice();
    uint16_t watts(double weight);
    metric wattsMetric();
    virtual bool changeFanSpeed(uint8_t speed);
    virtual metric elevationGain();
    virtual void clearStats();
    QBluetoothDeviceInfo bluetoothDevice;
    void disconnectBluetooth();
    virtual void setPaused(bool p);
    bool isPaused() { return paused; }
    virtual void setLap();
    void setAutoResistance(bool value) { autoResistanceEnable = value; }
    bool autoResistance() { return autoResistanceEnable; }
    void setDifficult(double d);
    double difficult();
    double weightLoss() { return WeightLoss.value(); }
    metric wattKg() { return WattKg; }
    metric currentMETS() { return METS; }
    metric currentHeartZone() { return HeartZone; }
    metric currentPowerZone() { return PowerZone; }

    // in the future these 2 should be calculated inside the update_metrics()
    void setHeartZone(double hz) { HeartZone = hz; }
    void setPowerZone(double pz) { PowerZone = pz; }

    enum BLUETOOTH_TYPE { UNKNOWN = 0, TREADMILL, BIKE, ROWING, ELLIPTICAL };
    enum WORKOUT_EVENT_STATE { STARTED = 0, PAUSED = 1, RESUMED = 2, STOPPED = 3 };

    virtual BLUETOOTH_TYPE deviceType();
    static QStringList metrics();
    virtual uint8_t metrics_override_heartrate();
    virtual uint8_t maxResistance();

  public Q_SLOTS:
    virtual void start();
    virtual void stop();
    virtual void heartRate(uint8_t heart);
    virtual void cadenceSensor(uint8_t cadence);
    virtual void powerSensor(uint16_t power);
    virtual void speedSensor(double speed);
    virtual void changeResistance(int8_t res);
    virtual void changePower(int32_t power);
    virtual void changeInclination(double grade, double percentage);
    virtual void changeGeoPosition(QGeoCoordinate p, double azimuth);
    virtual void workoutEventStateChanged(bluetoothdevice::WORKOUT_EVENT_STATE state);
    virtual bool supportStateMachine();

  Q_SIGNALS:
    void connectedAndDiscovered();
    void speedChanged(double speed);
    void cadenceChanged(uint8_t cadence);
    void powerChanged(uint16_t power);
    void inclinationChanged(double grade, double percentage);
    void fanSpeedChanged(uint8_t speed);
    void deviceStateChanged(uint8_t state);

  protected:
    QLowEnergyController *m_control = nullptr;

    metric elapsed;
    metric moving; // moving time
    metric Speed;
    metric KCal;
    metric Distance;
    uint8_t FanSpeed = 0;
    metric Heart;
    int8_t requestStart = -1;
    int8_t requestStop = -1;
    int8_t requestIncreaseFan = -1;
    int8_t requestDecreaseFan = -1;
    double requestFanSpeed = -1;
    double m_difficult = 1.0;
    metric m_jouls;
    metric elevationAcc;
    metric m_watt;
    metric WattKg;
    metric WeightLoss;
    metric Cadence;
    metric Resistance;
    metric METS;

    QGeoCoordinate coordinate;
    double azimuth;
    quint64 coordinateTS = 0;

    metric Inclination;
    metric HeartZone;
    metric PowerZone;

    bluetoothdevice::WORKOUT_EVENT_STATE lastState;
    bool paused = false;
    bool autoResistanceEnable = true;

    QDateTime _lastTimeUpdate;
    bool _firstUpdate = true;
    void update_metrics(bool watt_calc, const double watts);
    double calculateMETS();
};

#endif // BLUETOOTHDEVICE_H
