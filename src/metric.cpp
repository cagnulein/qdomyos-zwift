#include "metric.h"
#include "qdebugfixup.h"
#include "qzsettings.h"
#include <QSettings>

#ifdef TEST
static uint32_t random_value_uint32 = 0;
static uint8_t random_value_uint8 = 0;
#endif

metric::metric() {}

void metric::setType(_metric_type t, BLUETOOTH_TYPE bt) {
    m_type = t;
    m_bluetooth_type = bt;
}

void metric::setValue(double v, bool applyGainAndOffset) {
    QSettings settings;
    // these special cases are also in the valueRaw
    if (applyGainAndOffset) {
        if (m_type == METRIC_WATT) {
            if (v > 0) {
                double maxGain = (m_bluetooth_type == ROWING) ? 5.00 : 2.00;
                if (settings.value(QZSettings::watt_gain, QZSettings::default_watt_gain).toDouble() <= maxGain) {
                    if (settings.value(QZSettings::watt_gain, QZSettings::default_watt_gain).toDouble() != 1.0) {
                        qDebug() << QStringLiteral("watt value was ") << v
                                 << QStringLiteral("but it will be transformed to")
                                 << v * settings.value(QZSettings::watt_gain, QZSettings::default_watt_gain).toDouble();
                    }
                    v *= settings.value(QZSettings::watt_gain, QZSettings::default_watt_gain).toDouble();
                }
                if (settings.value(QZSettings::watt_offset, QZSettings::default_watt_offset).toDouble() != 0.0) {
                    qDebug()
                        << QStringLiteral("watt value was ") << v << QStringLiteral("but it will be transformed to")
                        << v + settings.value(QZSettings::watt_offset, QZSettings::default_watt_offset).toDouble();
                    v += settings.value(QZSettings::watt_offset, QZSettings::default_watt_offset).toDouble();
                }
            }
        } else if (m_type == METRIC_SPEED) {
            if (v > 0) {
                v *= settings.value(QZSettings::speed_gain, QZSettings::default_speed_gain).toDouble();
                v += settings.value(QZSettings::speed_offset, QZSettings::default_speed_offset).toDouble();
            }
        }
    }

    QDateTime now = QDateTime::currentDateTime();
    if (v != m_value && v != INFINITY) {
        m_valueChanged = now;
        if (m_last5.count() > 1) {
            double diff = v - m_value;
            double diffFromLastValue = qAbs(now.msecsTo(m_lastChanged));
            if (diffFromLastValue > 0)
                m_rateAtSec = diff * (1000.0 / diffFromLastValue);
            else
                m_rateAtSec = 0;
        }
    }

    // it has to be here, even if the value is the same, due to https://github.com/cagnulein/qdomyos-zwift/issues/1325
    m_lastChanged = now;

    m_value = v;

    if (paused) {
        return;
    }

    if (value() != 0 && value() != INFINITY) {
        m_countValue++;
        m_lapCountValue++;
        m_totValue += value();
        m_lapTotValue += value();
        m_last5.append(value());
        m_last20.append(value());

        if (m_last5.count() > 5)
            m_last5.removeAt(0);

        if (m_last20.count() > 20)
            m_last20.removeAt(0);

        if (value() < m_min) {
            m_min = value();
        }

        if (value() < m_lapMin) {
            m_lapMin = value();
        }

        if (value() > m_max) {
            m_max = value();
        }

        if (value() > m_lapMax) {
            m_lapMax = value();
        }
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
    m_last20.clear();
    clearLap(accumulator);
#ifdef TEST
    random_value_uint8 = 0;
    random_value_uint32 = 0;
#endif
}

double metric::valueRaw() {
    double v = m_value;
    QSettings settings;

    if (m_type == METRIC_WATT) {
        if (v > 0) {
            double maxGain = (m_bluetooth_type == ROWING) ? 5.00 : 2.00;
            if (settings.value(QZSettings::watt_gain, QZSettings::default_watt_gain).toDouble() <= maxGain) {
                v /= settings.value(QZSettings::watt_gain, QZSettings::default_watt_gain).toDouble();
            }
            if (settings.value(QZSettings::watt_offset, QZSettings::default_watt_offset).toDouble() != 0.0) {
                v -= settings.value(QZSettings::watt_offset, QZSettings::default_watt_offset).toDouble();
            }
        }
    } else if (m_type == METRIC_SPEED) {
        if (v > 0) {
            v /= settings.value(QZSettings::speed_gain, QZSettings::default_speed_gain).toDouble();
            v -= settings.value(QZSettings::speed_offset, QZSettings::default_speed_offset).toDouble();
        }
    }
    return v;
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

double metric::average20s() {
    if (m_last20.count() == 0)
        return 0;
    else {
        double sum = 0;
        uint8_t c = 0;
        QMutableListIterator<double> i(m_last20);
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

double metric::calculateMaxSpeedFromPower(double power, double inclination) {
    QSettings settings;
    double rolling_resistance =
        settings.value(QZSettings::rolling_resistance, QZSettings::default_rolling_resistance).toFloat();
    double twt = 9.8 * (settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() +
                        settings.value(QZSettings::bike_weight, QZSettings::default_bike_weight).toFloat());
    double aero = 0.22691607640851885;
    double hw = 0; // wind speed
    double tr = twt * ((inclination / 100.0) + rolling_resistance);
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

double metric::calculatePowerFromSpeed(double speed, double inclination) {
    QSettings settings;
    double rolling_resistance =
        settings.value(QZSettings::rolling_resistance, QZSettings::default_rolling_resistance).toFloat();
    double v = speed / 3.6; // converted to m/s;
    double tv = v + 0;
    double tran = 0.95;
    const double aero = 0.22691607640851885;
    double A2Eff = (tv > 0.0) ? aero : -aero; // wind in face, must reverse effect
    double twt = 9.8 * (settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() +
                        settings.value(QZSettings::bike_weight, QZSettings::default_bike_weight).toFloat());
    double tr = twt * ((inclination / 100.0) + rolling_resistance);
    return (v * tr + v * tv * tv * A2Eff) / tran;
}

double metric::calculateSpeedFromPower(double power, double inclination, double speed, double deltaTimeSeconds,
                                       double speedLimit) {
    QSettings settings;
    double speed_gain = settings.value(QZSettings::speed_gain, QZSettings::default_speed_gain).toDouble();
    double speed_offset = settings.value(QZSettings::speed_offset, QZSettings::default_speed_offset).toDouble();
    if (inclination < -5)
        inclination = -5;
    if (speed_offset != QZSettings::default_speed_offset)
        speed -= speed_offset;
    if (speed_gain != QZSettings::default_speed_gain)
        speed /= speed_gain;

    double fullWeight = (settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() +
                         settings.value(QZSettings::bike_weight, QZSettings::default_bike_weight).toFloat());
    double maxSpeed = calculateMaxSpeedFromPower(power, inclination);
    double maxPowerFromSpeed = calculatePowerFromSpeed(speed, inclination);
    double acceleration = (power - maxPowerFromSpeed) / fullWeight;
    double newSpeed = speed + (acceleration * 3.6 * deltaTimeSeconds);
    if (speedLimit > 0 && newSpeed > speedLimit)
        newSpeed = speedLimit;
    if (speedLimit > 0 && maxSpeed > speedLimit)
        maxSpeed = speedLimit;
    if (newSpeed < 0)
        newSpeed = 0;
    if (maxSpeed > newSpeed)
        return newSpeed;
    else if (maxSpeed < speed)
        return newSpeed;
    else
        return maxSpeed;
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

double metric::powerPeak(QList<SessionLine> *session, int seconds) {
    QList<IntervalBest> bests;
    QList<IntervalBest> _results;

    uint windowSize = seconds;
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

    if(bests.length() > 0)
        return bests.first().avg;
    else
        return 0;
}

// VO2 (L/min) = 0.0108 x power (W) + 0.007 x body mass (kg)
// power = 5 min peak power for a specific ride
double metric::calculateVO2Max(QList<SessionLine> *session) {       
    double peak = powerPeak(session, 5*60);
    QSettings settings;
    return ((0.0108 * peak + 0.007 * settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()) /
            settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()) *
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
    QString sex = settings.value(QZSettings::sex, QZSettings::default_sex).toString();
    double weight = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();
    double age = settings.value(QZSettings::age, QZSettings::default_age).toDouble();
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

double metric::calculateBMR() {
    // Calculate Basal Metabolic Rate using Mifflin-St Jeor equation
    // BMR (kcal/day) for males: 10 * weight(kg) + 6.25 * height(cm) - 5 * age + 5
    // BMR (kcal/day) for females: 10 * weight(kg) + 6.25 * height(cm) - 5 * age - 161
    
    QSettings settings;
    QString sex = settings.value(QZSettings::sex, QZSettings::default_sex).toString();
    double weight = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();
    double age = settings.value(QZSettings::age, QZSettings::default_age).toDouble();
    double height = settings.value(QZSettings::height, QZSettings::default_height).toDouble();
    
    // Full Mifflin-St Jeor equation with height
    if (sex.toLower().contains("female")) {
        return (10 * weight) + (6.25 * height) - (5 * age) - 161;
    } else {
        return (10 * weight) + (6.25 * height) - (5 * age) + 5;
    }
}

double metric::calculateActiveKCal(double totalKCal, double elapsed) {
    QSettings settings;
    bool activeOnly = settings.value(QZSettings::calories_active_only, QZSettings::default_calories_active_only).toBool();
    
    if (!activeOnly) {
        return totalKCal; // Return total calories if active-only mode is disabled
    }
    
    // Calculate BMR in calories per second
    double bmrPerDay = calculateBMR();
    double bmrPerSecond = bmrPerDay / (24.0 * 60.0 * 60.0); // Convert from calories/day to calories/second
    
    // Calculate BMR calories for the elapsed time
    double bmrForElapsed = bmrPerSecond * elapsed;
    
    // Active calories = Total calories - BMR calories for the elapsed time
    double activeKCal = totalKCal - bmrForElapsed;
    
    // Ensure we don't return negative calories
    return activeKCal > 0 ? activeKCal : 0;
}
