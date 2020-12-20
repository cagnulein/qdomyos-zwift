#ifndef SESSIONLINE_H
#define SESSIONLINE_H
#include <QTimer>
#include <QDateTime>

class SessionLine
{
public:
    double speed;
    int8_t inclination;
    double distance;
    uint8_t watt;
    int8_t resistance;
    uint8_t heart;
    double pace;
    uint8_t cadence;
    QDateTime time;
    double calories;

    SessionLine();
    SessionLine(double speed, int8_t inclination, double distance, uint8_t watt, int8_t resistance, uint8_t heart, double pace, uint8_t cadence, double calories, QDateTime time = QDateTime::currentDateTime());
};

#endif // SESSIONLINE_H
