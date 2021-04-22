#ifndef LOCKSCREEN_H
#define LOCKSCREEN_H

class lockscreen
{
public:
    void setTimerDisabled();
    void request();
    long heartRate();
    void virtualbike_ios();
    void virtualbike_setHeartRate(unsigned char heartRate);
    void virtualbike_setCadence(unsigned short crankRevolutions, unsigned short lastCrankEventTime);

    void virtualbike_zwift_ios();
    double virtualbike_getCurrentSlope();
    double virtualbike_getPowerRequested();
    void virtualbike_updateFTMS(unsigned short normalizeSpeed, unsigned char currentResistance, unsigned short currentCadence, unsigned short currentWatt);
};

#endif // LOCKSCREEN_H
