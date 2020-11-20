#ifndef SESSIONLINE_H
#define SESSIONLINE_H
#include <QTimer>
#include <QTime>

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
    QTime time;

    SessionLine();
    SessionLine(double speed, int8_t inclination, double distance, uint8_t watt, int8_t resistance, uint8_t heart, double pace, QTime time = QTime::currentTime());
};

#endif // SESSIONLINE_H
