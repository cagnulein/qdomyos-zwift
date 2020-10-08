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
    void save(QString filename);
    static trainprogram* load(QString filename);

    QList<trainrow> rows;
    uint32_t elapsed = 0;

    void restart();
    void scheduler(int tick);

private:
    uint32_t ticks = 0;
    uint16_t currentStep = 0;
    uint32_t ticksCurrentRow = 0;
    uint32_t elapsedCurrentRow = 0;
};

#endif // TRAINPROGRAM_H
