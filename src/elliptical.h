#ifndef ELLIPTICAL_H
#define ELLIPTICAL_H
#include "bluetoothdevice.h"
#include <QObject>

class elliptical : public bluetoothdevice {
    Q_OBJECT

  public:
    elliptical();
    metric lastRequestedPelotonResistance();
    void update_metrics(bool watt_calc, const double watts);
    metric lastRequestedCadence();
    metric lastRequestedResistance();
    metric lastRequestedSpeed() { return RequestedSpeed; }
    virtual metric currentInclination();
    virtual metric currentResistance();
    virtual double requestedSpeed();
    virtual uint8_t fanSpeed();
    virtual double currentCrankRevolutions();
    virtual uint16_t lastCrankEventTime();
    virtual bool connected();
    metric pelotonResistance();
    virtual int pelotonToEllipticalResistance(int pelotonResistance);
    virtual bool inclinationAvailableByHardware();
    bluetoothdevice::BLUETOOTH_TYPE deviceType();
    void clearStats();
    void setPaused(bool p);
    void setLap();
    uint16_t watts();
    void setGears(int8_t d);
    int8_t gears();

  public Q_SLOTS:
    virtual void changeSpeed(double speed);
    virtual void changeResistance(resistance_t res);
    virtual void changeInclination(double grade, double inclination);
    virtual void changeCadence(int16_t cad);
    virtual void changeRequestedPelotonResistance(int8_t resistance);

  signals:
    void bikeStarted();

  protected:
    metric RequestedResistance;
    metric RequestedCadence;
    metric RequestedSpeed;
    metric RequestedPelotonResistance;
    metric Resistance;
    metric m_pelotonResistance;
    uint16_t LastCrankEventTime = 0;
    resistance_t requestResistance = -1;
    volatile double requestSpeed = -1;
    double requestInclination = -100;
    double CrankRevs = 0;
    int8_t m_gears = 0;
    resistance_t lastRawRequestedResistanceValue = -1;
};

#endif // ELLIPTICAL_H
