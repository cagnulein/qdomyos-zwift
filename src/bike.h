#ifndef BIKE_H
#define BIKE_H

#include "bluetoothdevice.h"
#include <QObject>

class bike : public bluetoothdevice {

    Q_OBJECT

  public:
    bike();
    metric lastRequestedResistance();
    metric lastRequestedPelotonResistance();
    metric lastRequestedCadence();
    metric lastRequestedPower();
    virtual metric currentResistance();
    virtual uint8_t fanSpeed();
    virtual double currentCrankRevolutions();
    virtual uint16_t lastCrankEventTime();
    virtual bool connected();
    virtual uint16_t watts();
    virtual int pelotonToBikeResistance(int pelotonResistance);
    virtual uint8_t resistanceFromPowerRequest(uint16_t power);
    virtual uint16_t powerFromResistanceRequest(int8_t requestResistance);
    virtual bool ergManagedBySS2K() { return false; }
    bluetoothdevice::BLUETOOTH_TYPE deviceType();
    metric pelotonResistance();
    void clearStats();
    void setLap();
    void setPaused(bool p);
    uint8_t metrics_override_heartrate();
    void setGears(int8_t d);
    int8_t gears();
    metric currentSteeringAngle() { return m_steeringAngle; }
    virtual bool inclinationAvailableByHardware();

  public Q_SLOTS:
    virtual void changeResistanceRange(int8_t lower, int8_t upper);
    virtual void changeResistance(int8_t res);
    virtual void changeCadence(int16_t cad);
    virtual void changePower(int32_t power);
    virtual void changeRequestedPelotonResistance(int8_t resistance);
    virtual void cadenceSensor(uint8_t cadence);
    virtual void powerSensor(uint16_t power);
    virtual void changeInclination(double grade, double percentage);
    virtual void changeSteeringAngle(double angle) { m_steeringAngle = angle; }
    virtual void resistanceFromFTMSAccessory(int8_t res) { Q_UNUSED(res); }

  Q_SIGNALS:
    void bikeStarted();
    void resistanceChanged(int8_t resistance);
    void resistanceRead(int8_t resistance);
    void steeringAngleChanged(double angle);

  protected:
    int16_t RequestedLowerResistance = -1;
    int16_t RequestedUpperResistance = -1;
    metric RequestedResistance;
    metric RequestedPelotonResistance;
    metric RequestedCadence;
    metric RequestedPower;

    int8_t requestResistance = -1;
    double requestInclination = -1;
    int16_t requestPower = -1;

    bool ergModeSupported = false; // if a bike has this mode supported, when from the virtual bike there is a power
                                   // request there is no need to translate in resistance levels

    int8_t m_gears = 0;
    int8_t lastRawRequestedResistanceValue = -1;
    uint16_t LastCrankEventTime = 0;
    double CrankRevs = 0;

    metric m_pelotonResistance;

    metric m_steeringAngle;
};

#endif // BIKE_H
