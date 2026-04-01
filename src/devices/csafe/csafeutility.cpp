#include "csafeutility.h"

// Static data map initialization
const QMap<int, QPair<QString, double>> CSafeUtility::unitData = {{1, {"mile", 1609.34}},
                                                                      {2, {"0.1 mile", 160.934}},
                                                                      {3, {"0.01 mile", 16.0934}},
                                                                      {4, {"0.001 mile", 1.60934}},
                                                                      {5, {"ft", 0.3048}},
                                                                      {6, {"inch", 0.0254}},
                                                                      {7, {"lbs.", 0.453592}},
                                                                      {8, {"0.1 lbs.", 0.0453592}},
                                                                      {10, {"10 ft", 3.048}},
                                                                      {16, {"mile/hour", 0.44704}},
                                                                      {17, {"0.1 mile/hour", 0.044704}},
                                                                      {18, {"0.01 mile/hour", 0.0044704}},
                                                                      {19, {"ft/minute", 0.00508}},
                                                                      {33, {"Km", 1000}},
                                                                      {34, {"0.1 km", 100}},
                                                                      {35, {"0.01 km", 10}},
                                                                      {36, {"Meter", 1}},
                                                                      {37, {"0.1 meter", 0.1}},
                                                                      {38, {"Cm", 0.01}},
                                                                      {39, {"Kg", 1}},
                                                                      {40, {"0.1 kg", 0.1}},
                                                                      {48, {"Km/hour", 0.277778}},
                                                                      {49, {"0.1 Km/hour", 0.0277778}},
                                                                      {50, {"0.01 Km/hour", 0.00277778}},
                                                                      {51, {"Meter/minute", 0.0166667}},
                                                                      {55, {"Minutes/mile", 1}},
                                                                      {56, {"Minutes/km", 1}},
                                                                      {57, {"Seconds/km", 1}},
                                                                      {58, {"Seconds/mile", 1}},
                                                                      {65, {"floors", 1}},
                                                                      {66, {"0.1 floors", 0.1}},
                                                                      {67, {"steps", 1}},
                                                                      {68, {"revolutions", 1}},
                                                                      {69, {"strides", 1}},
                                                                      {70, {"strokes", 1}},
                                                                      {71, {"beats", 1}},
                                                                      {72, {"calories", 1}},
                                                                      {73, {"Kp", 1}},
                                                                      {74, {"% grade", 1}},
                                                                      {75, {"0.01 % grade", 0.01}},
                                                                      {76, {"0.1 % grade", 0.1}},
                                                                      {79, {"0.1 floors/minute", 0.1}},
                                                                      {80, {"floors/minute", 1}},
                                                                      {81, {"steps/minute", 1}},
                                                                      {82, {"revs/minute", 1}},
                                                                      {83, {"strides/minute", 1}},
                                                                      {84, {"stokes/minute", 1}},
                                                                      {85, {"beats/minute", 1}},
                                                                      {86, {"calories/minute", 1}},
                                                                      {87, {"calories/hour", 1}},
                                                                      {88, {"Watts", 1}},
                                                                      {89, {"Kpm", 1}},
                                                                      {90, {"Inch-Lb", 1}},
                                                                      {91, {"Foot-Lb", 1}},
                                                                      {92, {"Newton-Meters", 1}},
                                                                      {97, {"Amperes", 1}},
                                                                      {98, {"0.001 Amperes", 0.001}},
                                                                      {99, {"Volts", 1}},
                                                                      {100, {"0.001 Volts", 0.001}}};

QString CSafeUtility::getUnitName(int unitCode) {
    if (unitData.contains(unitCode)) {
        return unitData[unitCode].first; // Return the description
    }
    return "Unknown unit";
}

double CSafeUtility::convertToStandard(int unitCode, double value) {
    if (unitData.contains(unitCode)) {
        return value * unitData[unitCode].second; // Apply the conversion factor
    }
    return value; // Return the original value if no conversion factor is available
}

QString CSafeUtility::statusByteToText(int status) {
    switch (status) {
    case 0x00:
        return "Error";
    case 0x01:
        return "Ready";
    case 0x02:
        return "Idle";
    case 0x03:
        return "Have ID";
    case 0x05:
        return "In Use";
    case 0x06:
        return "Pause";
    case 0x07:
        return "Finish";
    case 0x08:
        return "Manual";
    case 0x09:
        return "Off line";
    default:
        return "Unknown";
    }
}
