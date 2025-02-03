#include "sessionline.h"

SessionLine::SessionLine(double speed, int8_t inclination, double distance, uint16_t watt, resistance_t resistance,
                         int8_t peloton_resistance, uint8_t heart, double pace, uint8_t cadence, double calories,
                         double elevationGain, uint32_t elapsed, bool lap, uint32_t totalStrokes, double avgStrokesRate,
                         double maxStrokesRate, double avgStrokesLength, const QGeoCoordinate coordinate,
                         double instantaneousStrideLengthCM, double groundContactMS, double verticalOscillationMM, double stepCount,
                         const QDateTime &time) {
    this->speed = speed;
    this->inclination = inclination;
    this->distance = distance;
    this->watt = watt;
    this->resistance = resistance;
    this->peloton_resistance = peloton_resistance;
    this->heart = heart;
    this->pace = pace;
    this->time = time;
    this->cadence = cadence;
    this->calories = calories;
    this->elevationGain = elevationGain;
    this->elapsedTime = elapsed;
    this->lapTrigger = lap;
    this->totalStrokes = totalStrokes;
    this->avgStrokesLength = avgStrokesLength;
    this->avgStrokesRate = avgStrokesRate;
    this->maxStrokesRate = maxStrokesRate;
    this->coordinate = coordinate;
    this->instantaneousStrideLengthCM = instantaneousStrideLengthCM;
    this->groundContactMS = groundContactMS;
    this->verticalOscillationMM = verticalOscillationMM;
    this->stepCount = stepCount;
}

SessionLine::SessionLine() {}
