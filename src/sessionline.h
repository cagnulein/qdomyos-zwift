#ifndef SESSIONLINE_H
#define SESSIONLINE_H

#include <QDateTime>
#include <QGeoCoordinate>
#include <QTimer>

#include "definitions.h"

class SessionLine {

  public:
    double speed;
    int8_t inclination;
    double distance;
    uint16_t watt;
    resistance_t resistance;
    int8_t peloton_resistance;
    uint8_t heart;
    double pace;
    uint8_t cadence;
    QDateTime time;
    double calories;
    double elevationGain;
    uint32_t elapsedTime;
    bool lapTrigger = false;
    uint32_t totalStrokes;
    double avgStrokesRate;
    double maxStrokesRate;
    double avgStrokesLength;
    QGeoCoordinate coordinate;
    double instantaneousStrideLengthCM;
    double groundContactMS;
    double verticalOscillationMM;
    double stepCount;

    SessionLine();
    SessionLine(double speed, int8_t inclination, double distance, uint16_t watt, resistance_t resistance,
                int8_t peloton_resistance, uint8_t heart, double pace, uint8_t cadence, double calories,
                double elevationGain, uint32_t elapsed, bool lap, uint32_t totalStrokes, double avgStrokesRate,
                double maxStrokesRate, double avgStrokesLength, const QGeoCoordinate coordinate,
                double instantaneousStrideLengthCM, double groundContactMS, double verticalOscillationMM, double stepCount,
                const QDateTime &time = QDateTime::currentDateTime());
};

#endif // SESSIONLINE_H
