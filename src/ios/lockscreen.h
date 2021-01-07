#ifndef LOCKSCREEN_H
#define LOCKSCREEN_H

class lockscreen
{
public:
   void setTimerDisabled();
   void request();
   long heartRate();
    void virtualbike_ios();
    void virtualbike_setHeartRate(unsigned short heartRate);
    void virtualbike_setCadence(unsigned short crankRevolutions, unsigned short lastCrankEventTime);

};

#endif // LOCKSCREEN_H
