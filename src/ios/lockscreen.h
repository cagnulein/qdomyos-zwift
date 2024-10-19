#ifndef LOCKSCREEN_H
#define LOCKSCREEN_H

class lockscreen {
  public:
    void setTimerDisabled();
    void request();
    long heartRate();
    long stepCadence();
    void setKcal(double kcal);
    void setDistance(double distance);
    void setSpeed(double speed);
    void setPower(double power);
    void setCadence(double cadence);

    // virtualbike
    void virtualbike_ios();
    void virtualbike_setHeartRate(unsigned char heartRate);
    void virtualbike_setCadence(unsigned short crankRevolutions, unsigned short lastCrankEventTime);

    void virtualbike_zwift_ios(bool disable_hr, bool garmin_bluetooth_compatibility);
    double virtualbike_getCurrentSlope();
    double virtualbike_getCurrentCRR();
    double virtualbike_getCurrentCW();
    double virtualbike_getPowerRequested();
    bool virtualbike_updateFTMS(unsigned short normalizeSpeed, unsigned char currentResistance,
                                unsigned short currentCadence, unsigned short currentWatt,
                                unsigned short CrankRevolutions, unsigned short LastCrankEventTime);
    int virtualbike_getLastFTMSMessage(unsigned char *message);

    // virtualrower
    void virtualrower_ios();
    void virtualrower_setHeartRate(unsigned char heartRate);
    bool virtualrower_updateFTMS(unsigned short normalizeSpeed, unsigned char currentResistance,
                                 unsigned short currentCadence, unsigned short currentWatt,
                                 unsigned short CrankRevolutions, unsigned short LastCrankEventTime,
                                 unsigned short StrokesCount, unsigned int Distance, unsigned short KCal,
                                 unsigned short Pace);
    int virtualrower_getLastFTMSMessage(unsigned char *message);

    // virtualtreadmill
    void virtualtreadmill_zwift_ios();
    void virtualtreadmill_setHeartRate(unsigned char heartRate);
    double virtualtreadmill_getCurrentSlope();
    uint64_t virtualtreadmill_lastChangeCurrentSlope();
    double virtualtreadmill_getPowerRequested();
    bool virtualtreadmill_updateFTMS(unsigned short normalizeSpeed, unsigned char currentResistance,
                                     unsigned short currentCadence, unsigned short currentWatt,
                                     unsigned short currentInclination, unsigned long long currentDistance);

    // volume
    double getVolume();

    // garmin
    bool urlParser(const char *url);
    void garminconnect_init();
    int getHR();
    int getFootCad();
    
    // debug
    static void debug(const char* debugstring);
    static void nslog(const char* log);
    
    //adb
    void adb_connect(const char* IP);
    void adb_sendcommand(const char* command);
    
    // Elite Aria Fan
    void eliteAriaFan();
    void eliteAriaFan_fanSpeedRequest(unsigned char speed);
    
    // Zwift API
    void zwift_api_decodemessage_player(const char* data, int len);
    float zwift_api_getaltitude();
    int zwift_api_getdistance();
    float zwift_api_getlatitude();
    float zwift_api_getlongitude();
    
    // quick actions    
    static void set_action_profile(const char* profile);
    static const char* get_action_profile();

};

#endif // LOCKSCREEN_H
