#pragma once

#include <stdint.h>
#include <functional>

/**
 * @brief An interface for interacting with a smartwatch lockscreen API.
 */
class QZLockscreen {
protected:
    explicit QZLockscreen() {}
    QZLockscreen(const QZLockscreen&) {}
public:
    virtual ~QZLockscreen() {};

  virtual void setTimerDisabled() = 0;
  virtual void request() = 0;
  virtual long heartRate() = 0;
  virtual long stepCadence() = 0;
  virtual void setKcal(double kcal) = 0;
  virtual void setDistance(double distance) = 0;

  // virtualbike
  virtual void virtualbike_ios() = 0;
  virtual void virtualbike_setHeartRate(unsigned char heartRate) = 0;
  virtual void virtualbike_setCadence(unsigned short crankRevolutions, unsigned short lastCrankEventTime) = 0;

  virtual void virtualbike_zwift_ios(bool disableHeartRate) = 0;
  virtual double virtualbike_getCurrentSlope() = 0;
  virtual double virtualbike_getCurrentCRR() = 0;
  virtual double virtualbike_getCurrentCW() = 0;
  virtual double virtualbike_getPowerRequested() = 0;
  virtual bool virtualbike_updateFTMS(unsigned short normalizeSpeed, unsigned char currentResistance,
                              unsigned short currentCadence, unsigned short currentWatt,
                              unsigned short CrankRevolutions, unsigned short LastCrankEventTime) = 0;
  virtual int virtualbike_getLastFTMSMessage(unsigned char *message) = 0;

  // virtualrower
  virtual void virtualrower_ios() = 0;
  virtual void virtualrower_setHeartRate(unsigned char heartRate) = 0;
  virtual bool virtualrower_updateFTMS(unsigned short normalizeSpeed, unsigned char currentResistance,
                               unsigned short currentCadence, unsigned short currentWatt,
                               unsigned short CrankRevolutions, unsigned short LastCrankEventTime,
                               unsigned short StrokesCount, unsigned int Distance, unsigned short KCal,
                               unsigned short Pace) = 0;
  virtual int virtualrower_getLastFTMSMessage(unsigned char *message) = 0;

  // virtualtreadmill
  virtual void virtualtreadmill_zwift_ios() = 0;
  virtual void virtualtreadmill_setHeartRate(unsigned char heartRate) = 0;
  virtual double virtualtreadmill_getCurrentSlope() = 0;
  virtual uint64_t virtualtreadmill_lastChangeCurrentSlope() = 0;
  virtual double virtualtreadmill_getPowerRequested() = 0;
  virtual bool virtualtreadmill_updateFTMS(unsigned short normalizeSpeed, unsigned char currentResistance,
                                   unsigned short currentCadence, unsigned short currentWatt,
                                   unsigned short currentInclination) = 0;

  // volume
  virtual double getVolume() = 0;

  // garmin
  virtual bool urlParser(const char *url) =0;
  virtual void garminconnect_init() =0;
  virtual int getHR() =0;
  virtual int getFootCad() =0;
};

