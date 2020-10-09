#ifndef TRAINPROGRAM_H
#define TRAINPROGRAM_H
#include <QTime>
#include <QObject>

class trainrow
{
public:
    QTime duration;
    double speed;
    double inclination;
    bool forcespeed;
};

class trainprogram: public QObject
{
    Q_OBJECT

public:
    trainprogram(QList<trainrow>);
    void save(QString filename);
    static trainprogram* load(QString filename);

    QList<trainrow> rows;
    uint32_t elapsed = 0;

    void restart();
    void scheduler(int tick);

signals:
    void start();
    void stop();
    void changeSpeed(double speed);
    void changeInclination(double inclination);

private:
    uint32_t ticks = 0;
    uint16_t currentStep = 0;
    uint32_t ticksCurrentRow = 0;
    uint32_t elapsedCurrentRow = 0;
};

#endif // TRAINPROGRAM_H
