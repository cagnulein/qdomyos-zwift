#ifndef LOCKSCREEN_H
#define LOCKSCREEN_H

#include "lockscreen/qzlockscreen.h"

class lockscreen : public QZLockscreen {
public:
    explicit lockscreen() : QZLockscreen() {}

    void setTimerDisabled() override;
    void request() override;
    long heartRate() override;
    long stepCadence() override;
    void setKcal(double kcal) override;
    void setDistance(double distance) override;

    // virtualbike
    void virtualbike_ios() override;
    void virtualbike_setHeartRate(unsigned char heartRate) override;
    void virtualbike_setCadence(unsigned short crankRevolutions, unsigned short lastCrankEventTime) override;

    void virtualbike_zwift_ios(bool disableHeartRate) override;
    double virtualbike_getCurrentSlope() override;
    double virtualbike_getCurrentCRR() override;
    double virtualbike_getCurrentCW() override;
    double virtualbike_getPowerRequested() override;
    bool virtualbike_updateFTMS(unsigned short normalizeSpeed, unsigned char currentResistance,
                                unsigned short currentCadence, unsigned short currentWatt,
                                unsigned short CrankRevolutions, unsigned short LastCrankEventTime) override;
    int virtualbike_getLastFTMSMessage(unsigned char *message) override;

    // virtualrower
    void virtualrower_ios() override;
    void virtualrower_setHeartRate(unsigned char heartRate) override;
    bool virtualrower_updateFTMS(unsigned short normalizeSpeed, unsigned char currentResistance,
                                 unsigned short currentCadence, unsigned short currentWatt,
                                 unsigned short CrankRevolutions, unsigned short LastCrankEventTime,
                                 unsigned short StrokesCount, unsigned int Distance, unsigned short KCal,
                                 unsigned short Pace) override;
    int virtualrower_getLastFTMSMessage(unsigned char *message) override;

    // virtualtreadmill
    void virtualtreadmill_zwift_ios() override;
    void virtualtreadmill_setHeartRate(unsigned char heartRate) override;
    double virtualtreadmill_getCurrentSlope() override;
    uint64_t virtualtreadmill_lastChangeCurrentSlope() override;
    double virtualtreadmill_getPowerRequested() override;
    bool virtualtreadmill_updateFTMS(unsigned short normalizeSpeed, unsigned char currentResistance,
                                     unsigned short currentCadence, unsigned short currentWatt,
                                     unsigned short currentInclination) override;

    // volume
    double getVolume() override;

    // garmin
    bool urlParser(const char *url);
    void garminconnect_init();
    int getHR();
    int getFootCad();
};

#endif // LOCKSCREEN_H
