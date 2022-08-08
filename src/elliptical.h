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
    virtual metric currentInclination() override;
    virtual metric currentResistance() override;
    virtual double requestedSpeed();
    virtual uint8_t fanSpeed() override;
    virtual double currentCrankRevolutions() override;
    virtual uint16_t lastCrankEventTime() override;
    virtual bool connected() override;
    metric pelotonResistance();
    virtual int pelotonToEllipticalResistance(int pelotonResistance);
    bluetoothdevice::BLUETOOTH_TYPE deviceType() override;
    void clearStats() override;
    void setPaused(bool p) override;
    void setLap() override;
    uint16_t watts();

  public Q_SLOTS:
    virtual void changeSpeed(double speed);
    virtual void changeResistance(int8_t res) override;
    virtual void changeInclination(double grade, double inclination) override;
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
    int8_t requestResistance = -1;
    volatile double requestSpeed = -1;
    double requestInclination = -100;
    double CrankRevs = 0;
};

#endif // ELLIPTICAL_H
