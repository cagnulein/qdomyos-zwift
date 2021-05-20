#include "metric.h"
#include <QSettings>
#include <QDebug>

metric::metric()
{

}

void metric::setType(_metric_type t)
{
    m_type = t;
}

void metric::setValue(double v)
{
    QSettings settings;
    if(m_type == METRIC_WATT)
    {
        if(v > 0)
        {
            if(settings.value("watt_gain", 1.0).toDouble() <= 1.25)
            {
                if(settings.value("watt_gain", 1.0).toDouble() != 1.0)
                {
                    qDebug() << "watt value was " << v << "but it will be transformed to" << v * settings.value("watt_gain", 1.0).toDouble();
                }
                v *= settings.value("watt_gain", 1.0).toDouble();
            }
            if(settings.value("watt_offset", 0.0).toDouble() < 0)
            {
                if(settings.value("watt_offset", 0.0).toDouble() != 0.0)
                {
                    qDebug() << "watt value was " << v << "but it will be transformed to" << v + settings.value("watt_offset", 0.0).toDouble();
                }
                v += settings.value("watt_offset", 0.0).toDouble();
            }
        }
    }
    m_value = v;

    if(paused) return;

    if(value() != 0)
    {
        m_countValue++;
        m_lapCountValue++;
        m_totValue += value();
        m_lapTotValue += value();

        if(value() < m_min)
            m_min = value();

        if(value() < m_lapMin)
            m_lapMin = value();
    }

    if(value() > m_max)
        m_max = value();

    if(value() > m_lapMax)
        m_lapMax = value();
}

void metric::clear(bool accumulator)
{
    if(accumulator)
    {
        m_offset = m_value;
    }
    m_max = 0;
    m_totValue = 0;
    m_countValue = 0;
    m_min = 999999999;
    clearLap(accumulator);
}

double metric::value()
{
    //static uint8_t i = 0; return (double)(i++); // DEBUG REMOVE random value to test
    return m_value - m_offset;
}

double metric::lapValue()
{
    return m_value - m_lapOffset;
}

double metric::average()
{
    if(m_countValue == 0)
        return 0;
    else
        return (m_totValue / m_countValue);
}

double metric::lapAverage()
{
    if(m_lapCountValue == 0)
        return 0;
    else
        return (m_lapTotValue / m_lapCountValue);
}

void metric::operator = (double v) {
    setValue(v);
}

void metric::operator += (double v) {
    setValue(m_value + v);
}

double metric::min()
{
    return m_min;
}

double metric::max()
{
    return m_max;
}

double metric::lapMin()
{
    return m_lapMin;
}

double metric::lapMax()
{
    return m_lapMax;
}

void metric::setPaused(bool p)
{
    paused = p;
}

void metric::clearLap(bool accumulator)
{
    if(accumulator)
    {
        m_lapOffset = m_value;
    }
    m_lapMax = 0;
    m_lapTotValue = 0;
    m_lapCountValue = 0;
    m_lapMin = 999999999;
}

void metric::setLap(bool accumulator)
{
    clearLap(accumulator);
}

double metric::calculateSpeedFromPower(double power)
{
    double aero = 0.22691607640851885;
    double hw = 0; // wind speed
    double tr = 3.8710000000000004;
    double tran = 0.95;
    double p = power;
    double  vel = 20;       // Initial guess
    const uint8_t  MAX = 10;       // maximum iterations
    double  TOL = 0.05;     // tolerance
    for (int i=1; i < MAX; i++) {
        double  tv = vel + hw;
        double  aeroEff = (tv > 0.0) ? aero : -aero; // wind in face, must reverse effect
        double  f = vel * (aeroEff * tv * tv + tr) - tran * p; // the function
        double  fp = aeroEff * (3.0 * vel + hw) * tv + tr;     // the derivative
        double  vNew = vel - f / fp;
        if (qAbs(vNew - vel) < TOL)
            return vNew * 3.6;  // success
        vel = vNew;
    }
    return 0.0;  // failed to converge
}
