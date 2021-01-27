#ifndef METRIC_H
#define METRIC_H

#include <math.h>

class metric
{
public:
    metric();
    void setValue(double value);
    double value();
    double average();
    double min();
    double max();
    void clear(bool accumulator);
    void operator = (double);
    void operator += (double);
    void setPaused(bool p);

private:
    double m_value = 0;
    double m_totValue = 0;
    double m_countValue = 0;
    double m_min = 999999999;
    double m_max = 0;
    double m_offset = 0;

    bool paused = false;
};

#endif // METRIC_H
