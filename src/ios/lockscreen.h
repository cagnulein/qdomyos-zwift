#ifndef LOCKSCREEN_H
#define LOCKSCREEN_H

#include <QByteArray>

class lockscreen {
  public:
    void setTimerDisabled();
    void request();
    long heartRate();
    long stepCadence();
    void setKcal(double kcal);
    void setTotalKcal(double totalKcal);
    void setDistance(double distance);
    void setSteps(double steps);
    void setSpeed(double speed);
    void setPower(double power);
    void setCadence(double cadence);
    void setElevationGain(double elevationGain);
    void setHeartRate(unsigned char heartRate);
    void startWorkout(unsigned short deviceType);
    void stopWorkout();
    void workoutTrackingUpdate(double speed, unsigned short cadence, unsigned short watt, unsigned short currentCalories, unsigned long long currentSteps, unsigned char deviceType, double currentDistance, double totalKcal, bool useMiles, unsigned char heartRate);
    bool appleWatchAppInstalled();

    // virtualbike
    void virtualbike_ios();
    void virtualbike_setHeartRate(unsigned char heartRate);
    void virtualbike_setCadence(unsigned short crankRevolutions, unsigned short lastCrankEventTime);

    void virtualbike_zwift_ios(bool disable_hr, bool garmin_bluetooth_compatibility, bool zwift_play_emulator, bool watt_bike_emulator);
    double virtualbike_getCurrentSlope();
    double virtualbike_getCurrentCRR();
    double virtualbike_getCurrentCW();
    double virtualbike_getPowerRequested();
    bool virtualbike_updateFTMS(unsigned short normalizeSpeed, unsigned char currentResistance, unsigned short currentCadence, unsigned short currentWatt, unsigned short CrankRevolutions, unsigned short LastCrankEventTime, signed short Gears, unsigned short currentCalories, unsigned int Distance, unsigned char deviceType);
    int virtualbike_getLastFTMSMessage(unsigned char *message);

    // virtualrower
    void virtualrower_ios();
    void virtualrower_ios_pm5(bool pm5Mode);
    void virtualrower_setHeartRate(unsigned char heartRate);
    void virtualrower_setPM5Mode(bool enabled);
    bool virtualrower_updateFTMS(unsigned short normalizeSpeed, unsigned char currentResistance,
                                 unsigned short currentCadence, unsigned short currentWatt,
                                 unsigned short CrankRevolutions, unsigned short LastCrankEventTime,
                                 unsigned short StrokesCount, unsigned int Distance, unsigned short KCal,
                                 unsigned short Pace, unsigned char deviceType);
    int virtualrower_getLastFTMSMessage(unsigned char *message);

    // virtualtreadmill
    void virtualtreadmill_zwift_ios(bool garmin_bluetooth_compatibility, bool bike_cadence_sensor);
    void virtualtreadmill_setHeartRate(unsigned char heartRate);
    double virtualtreadmill_getCurrentSlope();
    uint64_t virtualtreadmill_lastChangeCurrentSlope();
    double virtualtreadmill_getPowerRequested();
    double virtualtreadmill_getRequestedSpeed();
    bool virtualtreadmill_updateFTMS(unsigned short normalizeSpeed, unsigned char currentResistance,
                                     unsigned short currentCadence, unsigned short currentWatt,
                                     unsigned short currentInclination, unsigned long long currentDistance, double elevationGain,
                                     unsigned short currentCalories, qint32 currentSteps, unsigned short elapsedSeconds,
                                     unsigned char deviceType);

    // volume
    double getVolume();

    // background audio (keep-alive when iOS would kill the app)
    void startBackgroundAudio();
    void stopBackgroundAudio();

    // garmin
    bool urlParser(const char *url);
    void garminconnect_init();
    int getHR();
    int getFootCad();
    int getPower();
    double getSpeed();
    
    // debug
    static void debug(const char* debugstring);
    static void nslog(const char* log);
    
    //adb
    void adb_connect(const char* IP);
    void adb_sendcommand(const char* command);
    
    // Elite Aria Fan
    void eliteAriaFan();
    void eliteAriaFan_fanSpeedRequest(unsigned char speed);

    // Echelon Connect Sport
    void echelonConnectSport(const char*  Name, void* deviceClass);
    void echelonConnectSport_WriteCharacteristic(unsigned char* qdata, unsigned char length);

    // Wahoo KICKR/SNAP Bike
    void wahooKickrSnapBike(const char* Name, void* deviceClass);
    void writeCharacteristic(unsigned char* qdata, unsigned char length);

    // Zwift Click Remote
    void zwiftClickRemote(const char* Name, const char* UUID, void* deviceClass);
    void zwiftClickRemote_WriteCharacteristic(unsigned char* qdata, unsigned char length, void* deviceClass);
    
    // Zwift API
    void zwift_api_decodemessage_player(const char* data, int len);
    float zwift_api_getaltitude();
    int zwift_api_getdistance();
    float zwift_api_getlatitude();
    float zwift_api_getlongitude();
    
    // Zwift Hub Protobuf
    static QByteArray zwift_hub_inclinationCommand(double inclination);
    static QByteArray zwift_hub_setGearsCommand(unsigned int gears);
    static uint32_t zwift_hub_getPowerFromBuffer(const QByteArray& buffer);
    static uint32_t zwift_hub_getCadenceFromBuffer(const QByteArray& buffer);
    
    // quick actions
    static void set_action_profile(const char* profile);
    static const char* get_action_profile();

    // multi-window detection for iPadOS
    static bool isInMultiWindowMode();

    // web view cache
    static void clearWebViewCache();
};

#endif // LOCKSCREEN_H
