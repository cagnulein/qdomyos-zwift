#ifndef ELLIPTICAL_H
#define ELLIPTICAL_H
#include "bluetoothdevice.h"
#include <QObject>

class elliptical : public bluetoothdevice {
    Q_OBJECT

  public:
    elliptical();
    void update_metrics(bool watt_calc, const double watts);
    metric lastRequestedResistance();
    virtual metric currentInclination();
    virtual metric currentResistance();
    virtual uint8_t fanSpeed();
    virtual double currentCrankRevolutions();
    virtual uint16_t lastCrankEventTime();
    virtual bool connected();
    virtual int pelotonToEllipticalResistance(int pelotonResistance);
    bluetoothdevice::BLUETOOTH_TYPE deviceType();
    void clearStats();
    void setPaused(bool p);
    void setLap();
    uint16_t watts();

  public Q_SLOTS:
    virtual void changeResistance(int8_t res);
    virtual void changeInclination(double grade, double inclination);
    virtual void changeCadence(int16_t cad);
    virtual void changeRequestedPelotonResistance(int8_t resistance);

  signals:
    void bikeStarted();

  protected:
    metric RequestedResistance;
    metric RequestedCadence;
    metric RequestedPelotonResistance;
    metric Resistance;
    uint16_t LastCrankEventTime = 0;
    int8_t requestResistance = -1;
    double requestInclination = -100;
    double CrankRevs = 0;
};

#endif // ELLIPTICAL_H
