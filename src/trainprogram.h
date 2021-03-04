#ifndef TRAINPROGRAM_H
#define TRAINPROGRAM_H
#include <QTime>
#include <QTimer>
#include <QObject>
#include "bluetooth.h"

class trainrow
{
public:
    QTime duration;
    double speed;
    double fanspeed = -1;
    double inclination;
    int8_t resistance;
    bool forcespeed;
};

class trainprogram: public QObject
{
    Q_OBJECT

public:
    trainprogram(QList<trainrow>, bluetooth* b);
    void save(QString filename);
    static trainprogram* load(QString filename, bluetooth* b);
    QTime totalElapsedTime();
    QTime currentRowElapsedTime();
    QTime duration();
    double totalDistance();

    QList<trainrow> rows;
    QList<trainrow> loadedRows; // rows as loaded
    uint32_t elapsed = 0;
    bool enabled = true;

    void restart();
    void scheduler(int tick);

public slots:
    void onTapeStarted();
    void scheduler();

signals:
    void start();
    void stop();
    void changeSpeed(double speed);
    bool changeFanSpeed(uint8_t speed);
    void changeInclination(double inclination);
    void changeResistance(int8_t resistance);
    void changeSpeedAndInclination(double speed, double inclination);

private:
    bluetooth* bluetoothManager;
    bool started = false;
    uint32_t ticks = 0;
    uint16_t currentStep = 0;
    uint32_t ticksCurrentRow = 0;
    uint32_t elapsedCurrentRow = 0;
    QTimer timer;
};

#endif // TRAINPROGRAM_H
