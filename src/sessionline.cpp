#include "sessionline.h"

SessionLine::SessionLine(double speed, int8_t inclination, double distance, uint8_t watt, int8_t resistance, uint8_t heart, double pace, uint8_t cadence, double calories, QDateTime time)
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
}

SessionLine::SessionLine() {}
