#ifndef METRIC_H
#define METRIC_H

#include "qdebugfixup.h"
#include "sessionline.h"
#include <QDateTime>
#include <math.h>

class metric {

  public:
    typedef enum _metric_type {

        METRIC_OTHER = 0,
        METRIC_WATT = 1,
        METRIC_SPEED = 2,
        METRIC_ELAPSED = 3,
    } _metric_type;

    metric();
    void setType(_metric_type t);
    void setValue(double value, bool applyGainAndOffset = true);
    double value();
    QDateTime lastChanged() {return m_lastChanged;}
    double average();
    double average5s();

    // rate of the current metric in a second, useful to know how many Kcal i will burn in a
    // minute if i keep the current pace
    double rate1s() { return m_rateAtSec; }

    double min();
    double max();
    double lapValue();
    double lapAverage();
    double lapMin();
    double lapMax();
    void clearLap(bool accumulator);
    void clear(bool accumulator);
    void operator=(double);
    void operator+=(double);
    void setPaused(bool p);
    void setLap(bool accumulator);
    void setColor(QString color) {m_color = color;}
    QString color() {return m_color;}

    static double calculateMaxSpeedFromPower(double power, double inclination);
    static double calculatePowerFromSpeed(double speed, double inclination);
    static double calculateSpeedFromPower(double power, double inclination, double speed, double deltaTimeSeconds, double speedLimit);
    static double calculateWeightLoss(double kcal);
    static double calculateVO2Max(QList<SessionLine> *session);
    static double calculateKCalfromHR(double HR_AVG, double elapsed);

  private:
    double m_value = 0;
    double m_totValue = 0;
    double m_countValue = 0;
    double m_min = 999999999;
    double m_max = 0;
    double m_offset = 0;
    QList<double> m_last5;

    double m_lapOffset = 0;
    double m_lapTotValue = 0;
    double m_lapCountValue = 0;
    double m_lapMin = 999999999;
    double m_lapMax = 0;

    QDateTime m_lastChanged = QDateTime::currentDateTime();
    double m_rateAtSec = 0;

    _metric_type m_type = METRIC_OTHER;

    bool paused = false;
    QString m_color;
};

#endif // METRIC_H
