#pragma once

// The identified S22i uses the controller gear as its resistance level.
namespace S22iControl {
inline double inclineTarget(double grade, double gear, bool separateGear, bool s22i) {
    return grade + ((separateGear || s22i) ? 0.0 : gear);
}
inline bool discardResistanceAfterIncline(bool s22i) { return !s22i; }
inline bool applyGearToIncline(bool s22i) { return !s22i; }
inline double resistanceAfterGearChange(double resistance, double gear, double previousGear) {
    return resistance + gear - previousGear;
}
}
