#include "metric.h"
#include "qdebugfixup.h"
#include <QSettings>

#ifdef TEST
static uint32_t random_value_uint32 = 0;
static uint8_t random_value_uint8 = 0;
#endif

metric::metric() {}

void metric::setType(_metric_type t) { m_type = t; }

void metric::setValue(double v, bool applyGainAndOffset) {
    QSettings settings;
    if (applyGainAndOffset) {
        if (m_type == METRIC_WATT) {
            if (v > 0) {
                if (settings.value(QStringLiteral("watt_gain"), 1.0).toDouble() <= 2.00) {
                    if (settings.value(QStringLiteral("watt_gain"), 1.0).toDouble() != 1.0) {
                        qDebug() << QStringLiteral("watt value was ") << v
                                 << QStringLiteral("but it will be transformed to")
                                 << v * settings.value(QStringLiteral("watt_gain"), 1.0).toDouble();
                    }
                    v *= settings.value(QStringLiteral("watt_gain"), 1.0).toDouble();
                }
                if (settings.value(QStringLiteral("watt_offset"), 0.0).toDouble() < 0) {
                    if (settings.value(QStringLiteral("watt_offset"), 0.0).toDouble() != 0.0) {
                        qDebug() << QStringLiteral("watt value was ") << v
                                 << QStringLiteral("but it will be transformed to")
                                 << v + settings.value(QStringLiteral("watt_offset"), 0.0).toDouble();
                    }
                    v += settings.value(QStringLiteral("watt_offset"), 0.0).toDouble();
                }
            }
        } else if (m_type == METRIC_SPEED) {
            if (v > 0) {
                v *= settings.value(QStringLiteral("speed_gain"), 1.0).toDouble();
                v += settings.value(QStringLiteral("speed_offset"), 0.0).toDouble();
            }
        }
    }

    QDateTime now = QDateTime::currentDateTime();
    if (v != m_value) {
        if (m_last5.count() > 1) {
            double diff = v - m_value;
            double diffFromLastValue = qAbs(now.msecsTo(m_lastChanged));
            if (diffFromLastValue > 0)
                m_rateAtSec = diff * (1000.0 / diffFromLastValue);
            else
                m_rateAtSec = 0;
        }
        m_lastChanged = now;
    }

    m_value = v;

    if (paused) {
        return;
    }

    if (value() != 0) {
        m_countValue++;
        m_lapCountValue++;
        m_totValue += value();
        m_lapTotValue += value();
        m_last5.append(value());

        if (m_last5.count() > 5)
            m_last5.removeAt(0);

        if (value() < m_min) {
            m_min = value();
        }

        if (value() < m_lapMin) {
            m_lapMin = value();
        }
    }

    if (value() > m_max) {
        m_max = value();
    }

    if (value() > m_lapMax) {
        m_lapMax = value();
    }
}

void metric::clear(bool accumulator) {
    if (accumulator) {
        m_offset = m_value;
    }
    m_max = 0;
    m_totValue = 0;
    m_countValue = 0;
    m_min = 999999999;
    m_last5.clear();
    clearLap(accumulator);
#ifdef TEST
    random_value_uint8 = 0;
    random_value_uint32 = 0;
#endif
}

double metric::value() {
#ifdef TEST
    if (m_type != METRIC_ELAPSED) {
        return (double)(rand() % 256);
    } else {
        return (double)(random_value_uint32++);
    }

#endif
    return m_value - m_offset;
}

double metric::lapValue() { return m_value - m_lapOffset; }

double metric::average() {
    if (m_countValue == 0) {
        return 0;
    } else {
        return (m_totValue / m_countValue);
    }
}

double metric::lapAverage() {
    if (m_lapCountValue == 0) {
        return 0;
    } else {
        return (m_lapTotValue / m_lapCountValue);
    }
}

double metric::average5s() {
    if (m_last5.count() == 0)
        return 0;
    else {
        double sum = 0;
        uint8_t c = 0;
        QMutableListIterator<double> i(m_last5);
        while (i.hasNext()) {
            double b = i.next();
            sum += b;
            c++;
        }

        if (c > 0)
            return (sum / c);
        else
            return 0;
    }
}

void metric::operator=(double v) { setValue(v); }

void metric::operator+=(double v) { setValue(m_value + v); }

double metric::min() { return m_min; }

double metric::max() { return m_max; }

double metric::lapMin() { return m_lapMin; }

double metric::lapMax() { return m_lapMax; }

void metric::setPaused(bool p) { paused = p; }

void metric::clearLap(bool accumulator) {
    if (accumulator) {
        m_lapOffset = m_value;
    }
    m_lapMax = 0;
    m_lapTotValue = 0;
    m_lapCountValue = 0;
    m_lapMin = 999999999;
}

void metric::setLap(bool accumulator) { clearLap(accumulator); }

double metric::calculateSpeedFromPower(double power, double inclination) {
    QSettings settings;
    if (inclination < -5)
        inclination = -5;
    double twt = 9.8 * (settings.value(QStringLiteral("weight"), 75.0).toFloat() +
                        settings.value(QStringLiteral("bike_weight"), 0.0).toFloat());
    double aero = 0.22691607640851885;
    double hw = 0; // wind speed
    double tr = twt * ((inclination / 100.0) + 0.005);
    double tran = 0.95;
    double p = power;
    double vel = 20;        // Initial guess
    const uint8_t MAX = 10; // maximum iterations
    double TOL = 0.05;      // tolerance
    for (int i = 1; i < MAX; i++) {
        double tv = vel + hw;
        double aeroEff = (tv > 0.0) ? aero : -aero;           // wind in face, must reverse effect
        double f = vel * (aeroEff * tv * tv + tr) - tran * p; // the function
        double fp = aeroEff * (3.0 * vel + hw) * tv + tr;     // the derivative
        double vNew = vel - f / fp;
        if (qAbs(vNew - vel) < TOL) {
            if (vNew < 0)
                return 0;
            else if (vNew > 19) // 19 m/s == 70 km/h
                return 70;
            return vNew * 3.6;
        } // success
        vel = vNew;
    }
    return 0.0; // failed to converge
}

double metric::calculateWeightLoss(double kcal) {
    return kcal / 7716.1854; // comes from 1 lbs = 3500 kcal. Converted to kg
}

struct IntervalBest {
    double avg = 0;
    int64_t start = 0;
    int64_t stop = 0;
};

struct CompareBests {
    // Sort by decreasing power and increasing start time.
    bool operator()(const IntervalBest &a, const IntervalBest &b) const {
        if (a.avg > b.avg)
            return true;
        if (b.avg > a.avg)
            return false;
        return a.start < b.start;
    }
};

// VO2 (L/min) = 0.0108 x power (W) + 0.007 x body mass (kg)
// power = 5 min peak power for a specific ride
double metric::calculateVO2Max(QList<SessionLine> *session) {
    QList<IntervalBest> bests;
    QList<IntervalBest> _results;

    uint windowSize = 5 * 60; // 5 mins
    double total = 0.0;
    QList<const SessionLine *> window;

    if (session->count() == 0)
        return -1;

    // ride is shorter than the window size!
    if (windowSize > session->last().elapsedTime)
        return -1;

    int i = 0;
    // We're looking for intervals with durations in [windowSizeSecs, windowSizeSecs + secsDelta).
    foreach (SessionLine point, *session) {

        total += point.watt;
        window.append(&session->at(i));
        double duration = window.last()->elapsedTime - window.first()->elapsedTime;

        if (duration >= windowSize) {
            double start = window.first()->elapsedTime;
            double stop = window.last()->elapsedTime;
            double avg = total / duration;
            IntervalBest b;
            b.start = start;
            b.stop = stop;
            b.avg = avg;
            bests.append(b);

            total -= window.first()->watt;
            window.removeFirst();
        }
        i++;
    }

    std::sort(bests.begin(), bests.end(), CompareBests());

    double peak = bests.first().avg;
    QSettings settings;
    return ((0.0108 * peak + 0.007 * settings.value(QStringLiteral("weight"), 75.0).toFloat()) /
            settings.value(QStringLiteral("weight"), 75.0).toFloat()) *
           1000.0;
}

double metric::calculateKCalfromHR(double HR_AVG, double elapsed) {
    /*
         * If VO2 max is unknown, the following formulas would apply:

Women:

    CB = T * (0.4472*H - 0.1263*W + 0.074*A - 20.4022) / 4.184

         Men:

        CB = T * (0.6309*H + 0.1988*W + 0.2017*A - 55.0969) / 4.184

             Where:

        CB is the number of calories burned;
    T is the duration of exercise in minutes;
    H is your average heart rate in beats per minute;
    W is your weight in kilograms; and
        A is your age in years.
            */

    QSettings settings;
    QString sex = settings.value(QStringLiteral("sex"), "Male").toString();
    double weight = settings.value(QStringLiteral("weight"), 75.0).toFloat();
    double age = settings.value(QStringLiteral("age"), 35).toDouble();
    double T = elapsed / 60;
    double H = HR_AVG;
    double W = weight;
    double A = age;
    if (sex.toLower().contains("female")) {
        return (T * ((0.4472 * H) - (0.1263 * W) + (0.074 * A) - 20.4022) / 4.184);
    } else {
        return (T * ((0.6309 * H) + (0.1988 * W) + (0.2017 * A) - 55.0969) / 4.184);
    }
}
