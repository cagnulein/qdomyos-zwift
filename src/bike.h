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
    virtual resistance_t pelotonToBikeResistance(int pelotonResistance);
    virtual resistance_t resistanceFromPowerRequest(uint16_t power);
    virtual uint16_t powerFromResistanceRequest(resistance_t requestResistance);
    virtual bool ergManagedBySS2K() { return false; }
    bluetoothdevice::BLUETOOTH_TYPE deviceType();
    metric pelotonResistance();
    void clearStats();
    void setLap();
    void setPaused(bool p);
    uint8_t metrics_override_heartrate();
    void setGears(int8_t d);
    int8_t gears();
    void setSpeedLimit(double speed) {m_speedLimit = speed;}
    double speedLimit() {return m_speedLimit;}


    /**
     * @brief currentSteeringAngle Gets a metric object to get or set the current steering angle
     * for the Elite Sterzo or emulating device. Expected range -45 to +45 degrees.
     * @return A metric object.
     */
    metric currentSteeringAngle() { return m_steeringAngle; }
    virtual bool inclinationAvailableByHardware();

  public Q_SLOTS:
    virtual void changeResistance(resistance_t res);
    virtual void changeCadence(int16_t cad);
    virtual void changePower(int32_t power);
    virtual void changeRequestedPelotonResistance(int8_t resistance);
    virtual void cadenceSensor(uint8_t cadence);
    virtual void powerSensor(uint16_t power);
    virtual void changeInclination(double grade, double percentage);
    virtual void changeSteeringAngle(double angle) { m_steeringAngle = angle; }
    virtual void resistanceFromFTMSAccessory(resistance_t res) { Q_UNUSED(res); }

  Q_SIGNALS:
    void bikeStarted();
    void resistanceChanged(resistance_t resistance);
    void resistanceRead(resistance_t resistance);
    void steeringAngleChanged(double angle);

  protected:
    metric RequestedResistance;
    metric RequestedPelotonResistance;
    metric RequestedCadence;
    metric RequestedPower;

    resistance_t requestResistance = -1;
    double requestInclination = -100;
    int16_t requestPower = -1;

    bool ergModeSupported = false; // if a bike has this mode supported, when from the virtual bike there is a power
                                   // request there is no need to translate in resistance levels

    int8_t m_gears = 0;
    resistance_t lastRawRequestedResistanceValue = -1;
    uint16_t LastCrankEventTime = 0;
    double CrankRevs = 0;

    metric m_pelotonResistance;

    metric m_steeringAngle;

    double m_speedLimit = 0;
};

#endif // BIKE_H
