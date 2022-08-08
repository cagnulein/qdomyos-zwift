#ifndef ROWER_H
#define ROWER_H

#include "bike.h"
#include "bluetoothdevice.h"
#include <QObject>

class rower : public bluetoothdevice {
    Q_OBJECT

  public:
    rower();
    metric lastRequestedResistance();
    metric lastRequestedPelotonResistance();
    metric lastRequestedCadence();
    metric lastRequestedPower();
    virtual metric currentResistance() override;
    virtual metric currentStrokesCount();
    virtual metric currentStrokesLength();
    virtual QTime currentPace() override;
    virtual uint8_t fanSpeed() override;
    virtual double currentCrankRevolutions() override;
    virtual uint16_t lastCrankEventTime() override;
    virtual bool connected() override;
    virtual uint16_t watts();
    virtual int pelotonToBikeResistance(int pelotonResistance);
    virtual uint8_t resistanceFromPowerRequest(uint16_t power);
    bluetoothdevice::BLUETOOTH_TYPE deviceType() override;
    metric pelotonResistance();
    void clearStats() override;
    void setLap() override;
    void setPaused(bool p) override;

  public slots:
    virtual void changeResistance(int8_t res) override;
    virtual void changeCadence(int16_t cad);
    virtual void changePower(int32_t power) override;
    virtual void changeRequestedPelotonResistance(int8_t resistance);
    virtual void cadenceSensor(uint8_t cadence) override;
    virtual void powerSensor(uint16_t power) override;

  signals:
    void bikeStarted();
    void resistanceChanged(int8_t resistance);
    void resistanceRead(int8_t resistance);

  protected:
    metric Resistance;
    metric RequestedResistance;
    metric RequestedPelotonResistance;
    double requestInclination = -100;
    metric RequestedCadence;
    metric RequestedPower;
    metric StrokesLength;
    metric StrokesCount;
    uint16_t LastCrankEventTime = 0;
    int8_t requestResistance = -1;
    double CrankRevs = 0;

    metric m_pelotonResistance;
};

#endif // ROWER_H
