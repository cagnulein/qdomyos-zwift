#ifndef METRIC_H
#define METRIC_H

#include <math.h>

class metric
{
public:
    typedef enum _metric_type
    {
        METRIC_OTHER = 0,
        METRIC_WATT = 1,
    }_metric_type;

    metric();
    void setType(_metric_type t);
    void setValue(double value);
    double value();
    double average();
    double min();
    double max();
    double lapValue();
    double lapAverage();
    double lapMin();
    double lapMax();
    void clearLap(bool accumulator);
    void clear(bool accumulator);
    void operator = (double);
    void operator += (double);
    void setPaused(bool p);
    void setLap(bool accumulator);

    static double calculateSpeedFromPower(double power);
    static double calculateWeightLoss(double kcal);

private:
    double m_value = 0;
    double m_totValue = 0;
    double m_countValue = 0;
    double m_min = 999999999;
    double m_max = 0;
    double m_offset = 0;

    double m_lapOffset = 0;
    double m_lapTotValue = 0;
    double m_lapCountValue = 0;
    double m_lapMin = 999999999;
    double m_lapMax = 0;

    _metric_type m_type = METRIC_OTHER;

    bool paused = false;
};

#endif // METRIC_H
