#include "sessionline.h"

SessionLine::SessionLine(double speed, int8_t inclination, double distance, uint16_t watt, int8_t resistance, uint8_t heart, double pace, uint8_t cadence, double calories, double elevationGain, uint32_t elapsed, QDateTime time)
{
    this->speed = speed;
    this->inclination = inclination;
    this->distance = distance;
    this->watt = watt;
    this->resistance = resistance;
    this->heart = heart;
    this->pace = pace;
    this->time = time;
    this->cadence = cadence;
    this->calories = calories;
    this->elevationGain = elevationGain;
    this->elapsedTime = elapsed;
}

SessionLine::SessionLine() {}
