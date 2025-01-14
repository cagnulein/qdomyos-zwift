// tss_calculator.h
#ifndef TSS_CALCULATOR_H
#define TSS_CALCULATOR_H

#include <vector>
#include <cmath>
#include <QSettings>
#include <QObject>
#include <QDateTime>
#include <QDebug>
#include "qzsettings.h"

class TSSCalculator : public QObject {
    Q_OBJECT

  private:
    static const int ROLLING_WINDOW_SECONDS = 30;
    struct PowerSample {
        double watts;
        qint64 timestamp;

        PowerSample(double w = 0, qint64 ts = 0) : watts(w), timestamp(ts) {}
    };

    std::vector<PowerSample> powerHistory;
    double rollingSum;
    double totalPower4;
    int sampleCount;
    qint64 lastTimestamp = 0;

    double getCurrentFTP() const {
        QSettings settings;
        return settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble();
    }

  public:
    explicit TSSCalculator(QObject *parent = nullptr) :
                                                        QObject(parent),
                                                        powerHistory(ROLLING_WINDOW_SECONDS),
                                                        rollingSum(0.0),
                                                        totalPower4(0.0),
                                                        sampleCount(0),
                                                        lastTimestamp(0)
    {
    }

    void reset() {
        powerHistory.clear();
        powerHistory.resize(ROLLING_WINDOW_SECONDS);
        rollingSum = 0.0;
        totalPower4 = 0.0;
        sampleCount = 0;
        lastTimestamp = 0;
    }

    void addPowerData(double watts) {
        qint64 timestamp = QDateTime::currentMSecsSinceEpoch();

        if (lastTimestamp == 0) {
            lastTimestamp = timestamp;
            return;
        }

        double intervalSeconds = (timestamp - lastTimestamp) / 1000.0;

        if (intervalSeconds < 0.8 || intervalSeconds > 1.2) {
            return;
        }

        rollingSum -= powerHistory[sampleCount % ROLLING_WINDOW_SECONDS].watts;
        powerHistory[sampleCount % ROLLING_WINDOW_SECONDS] = PowerSample(watts, timestamp);
        rollingSum += watts;

        if (sampleCount >= ROLLING_WINDOW_SECONDS - 1) {
            double avgPower = rollingSum / ROLLING_WINDOW_SECONDS;
            totalPower4 += pow(avgPower, 4);
        }

        sampleCount++;
        lastTimestamp = timestamp;
    }

    double getNormalizedPower() const {
        if (sampleCount < ROLLING_WINDOW_SECONDS) return 0.0;
        double avgPower4 = totalPower4 / (sampleCount - ROLLING_WINDOW_SECONDS + 1);
        return pow(avgPower4, 0.25);
    }

    double getIntensityFactor() const {
        double np = getNormalizedPower();
        return np / getCurrentFTP();
    }

    double getTSS(int elapsedSeconds) const {
        if (elapsedSeconds == 0 || sampleCount < ROLLING_WINDOW_SECONDS) return 0.0;
        double if_ = getIntensityFactor();
        double hours = elapsedSeconds / 3600.0;
        return 100.0 * hours * if_ * if_;
    }
};

#endif // TSS_CALCULATOR_H
