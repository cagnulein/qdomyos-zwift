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
    QTime totalElapsedTime();
    QTime currentRowElapsedTime();
    QTime duration();
    double totalDistance();

    QList<trainrow> rows;
    uint32_t elapsed = 0;
    bool enabled = true;

    void restart();
    void scheduler(int tick);

public slots:
    void onTapeStarted();

signals:
    void start();
    void stop();
    void changeSpeed(double speed);
    void changeInclination(double inclination);
    void changeSpeedAndInclination(double speed, double inclination);

private:
    bool started = false;
    uint32_t ticks = 0;
    uint16_t currentStep = 0;
    uint32_t ticksCurrentRow = 0;
    uint32_t elapsedCurrentRow = 0;
};

#endif // TRAINPROGRAM_H
