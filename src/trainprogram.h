#ifndef TRAINPROGRAM_H
#define TRAINPROGRAM_H
#include <QTime>

class trainrow
{
public:
    QTime duration;
    double speed;
    double inclination;
    bool forcespeed;
};

class trainprogram
{
public:
    trainprogram(QList<trainrow>);

private:
    QList<trainrow> rows;
};

#endif // TRAINPROGRAM_H
