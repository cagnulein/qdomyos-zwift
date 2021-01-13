#include "metric.h"

metric::metric()
{

}

void metric::setValue(double value)
{
    m_value = value;
    if(value != 0)
    {
        m_countValue++;
        m_totValue += value;

        if(m_value < m_min)
            m_min = m_value;
    }

    if(m_value > m_max)
        m_max = m_value;
}

double metric::value()
{
    return m_value;
}

double metric::average()
{
    if(m_countValue == 0)
        return 0;
    else
        return (m_totValue / m_countValue);
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
