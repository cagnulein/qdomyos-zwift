#ifndef IOS_LIVEACTIVITY_H
#define IOS_LIVEACTIVITY_H

class ios_liveactivity {
public:
    static void startLiveActivity(const char* deviceName, bool useMiles, const char* compactLeadingMetric,
                                  const char* compactTrailingMetric);
    static void updateLiveActivity(double speed, double cadence, double power, int heartRate, double distance,
                                   double kcal, bool useMiles, const char* compactLeadingMetric,
                                   int compactLeadingValue, const char* compactTrailingMetric,
                                   int compactTrailingValue);
    static void endLiveActivity();
    static bool isLiveActivityRunning();
};

#endif // IOS_LIVEACTIVITY_H
