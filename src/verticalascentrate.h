#ifndef VERTICALASCENTRATE_H
#define VERTICALASCENTRATE_H

#include <cmath>

// Instantaneous Vertical Ascent in meters/hour. Speed is km/h, grade is percent.
inline double instantaneousVam(double speedKmh, double gradePercent, bool paused) {
    if (paused || !std::isfinite(speedKmh) || !std::isfinite(gradePercent) ||
        speedKmh <= 0 || gradePercent <= 0)
        return 0.0;
    return speedKmh * gradePercent * 10.0;
}

// Session average includes all moving time (flat/downhill too), but not pauses.
inline double averageVam(double positiveAscentMeters, double movingSeconds) {
    if (!std::isfinite(positiveAscentMeters) || !std::isfinite(movingSeconds) ||
        positiveAscentMeters <= 0 || movingSeconds <= 0)
        return 0.0;
    return positiveAscentMeters * 3600.0 / movingSeconds;
}

#endif // VERTICALASCENTRATE_H
