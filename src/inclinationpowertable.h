// -----------------------------------------------------------------------------
// inclinationpowertable.h
// -----------------------------------------------------------------------------
// What this does:
// - Learns a mapping of (inclination, cadence) -> wattage for bikes that do not
//   support ERG mode but can only receive simulation grade (e.g., D2RIDE).
// - Given a target power and current cadence, we estimate the inclination to send
//   via FTMS SET_INDOOR_BIKE_SIMULATION_PARAMS so the bike holds the requested watt.
//
// How data is collected:
// - collectData(cadence, watt, inclination) is fed during SIM rides.
// - We ignore samples with cadence or watt == 0.
// - We require the inclination to stay stable for at least 1 second (delta <= 0.1).
//   This keeps noisy transitions from polluting the table.
// - Samples are bucketed by (cadence, inclination*10) to allow decimals, and we
//   keep up to 100 samples per bucket. Once 10 samples exist, we store the median
//   wattage for that bucket in consolidatedData.
//
// How estimation works:
// - estimateInclinationForPower(cadence, power):
//   1) Find points with the closest cadence to the current one (smallest cadence diff).
//   2) Within that subset, try to bracket the requested power (lower/upper watt).
//      If both sides exist, interpolate inclination between them.
//   3) If no bracket, return the closest inclination by absolute watt delta.
// - hasData() indicates if any consolidated samples exist.
//
// Persistence:
// - consolidatedData is serialized to QSettings under inclinationPowerDataPoints
//   so learning survives restarts.
//
// Integration reminder:
// - ftmsbike.cpp uses this table for D2RIDE when ergModeSupported=false. If the
//   table has data, power requests are translated into an inclination set command.
//   If empty, we fall back to inclination/resistance or gain/offset baselines.
// -----------------------------------------------------------------------------
#ifndef INCLINATIONPOWERTABLE_H
#define INCLINATIONPOWERTABLE_H

#include <QDateTime>
#include <QDebug>
#include <QList>
#include <QMap>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QtGlobal>
#include <QMetaType>
#include <algorithm>
#include "qzsettings.h"

struct inclinationPowerDataPoint {
    uint16_t cadence = 0;
    double inclination = 0.0;
    uint16_t wattage = 0;

    inclinationPowerDataPoint() = default;
    inclinationPowerDataPoint(uint16_t c, double i, uint16_t w) : cadence(c), inclination(i), wattage(w) {}
};

Q_DECLARE_METATYPE(inclinationPowerDataPoint)

struct CadenceInclinationKey {
    uint16_t cadence;
    int16_t inclinationTimes10;

    bool operator<(const CadenceInclinationKey &other) const {
        if (inclinationTimes10 != other.inclinationTimes10)
            return inclinationTimes10 < other.inclinationTimes10;
        return cadence < other.cadence;
    }
};

class WattageSamples {
  public:
    static const int MAX_SAMPLES = 100;
    static const int MIN_SAMPLES_REQUIRED = 10;

    void addSample(uint16_t wattage) {
        samples.append(wattage);
        if (samples.size() > MAX_SAMPLES) {
            samples.removeFirst();
        }
        medianNeedsUpdate = true;
    }

    uint16_t getMedian() {
        if (!medianNeedsUpdate)
            return cachedMedian;
        if (samples.isEmpty())
            return 0;

        QList<uint16_t> sortedSamples = samples;
        std::sort(sortedSamples.begin(), sortedSamples.end());

        int middle = sortedSamples.size() / 2;
        if (sortedSamples.size() % 2 == 0) {
            cachedMedian = (sortedSamples[middle - 1] + sortedSamples[middle]) / 2;
        } else {
            cachedMedian = sortedSamples[middle];
        }

        medianNeedsUpdate = false;
        return cachedMedian;
    }

    int sampleCount() const { return samples.size(); }

    void clear() {
        samples.clear();
        cachedMedian = 0;
        medianNeedsUpdate = true;
    }

  private:
    QList<uint16_t> samples;
    uint16_t cachedMedian = 0;
    bool medianNeedsUpdate = true;
};

class inclinationPowerTable {
  public:
    inclinationPowerTable() { loadSettings(); }

    ~inclinationPowerTable() { saveSettings(); }

    void reset() {
        wattageData.clear();
        consolidatedData.clear();
        lastInclinationValue = -999.0;
        lastInclinationTime = QDateTime::currentDateTime();

        QSettings settings;
        settings.remove(QZSettings::inclinationPowerDataPoints);
        settings.sync();
    }

    void collectData(uint16_t cadence, uint16_t wattage, double inclination) {
        if (cadence == 0 || wattage == 0)
            return;

        QDateTime now = QDateTime::currentDateTime();
        if (qAbs(inclination - lastInclinationValue) > 0.1) {
            lastInclinationValue = inclination;
            lastInclinationTime = now;
            return;
        }

        if (lastInclinationTime.msecsTo(now) < 1000) {
            return;
        }

        CadenceInclinationKey key{cadence, static_cast<int16_t>(qRound(inclination * 10.0))};
        wattageData[key].addSample(wattage);

        if (wattageData[key].sampleCount() >= WattageSamples::MIN_SAMPLES_REQUIRED) {
            updateDataTable(key);
        }
    }

    double estimateInclinationForPower(uint16_t cadence, uint16_t power) {
        if (consolidatedData.isEmpty())
            return 0.0;

        QList<inclinationPowerDataPoint> closestCadencePoints;
        uint16_t bestCadenceDiff = UINT16_MAX;
        for (const auto &point : consolidatedData) {
            uint16_t cadenceDiff = qAbs(static_cast<int>(point.cadence) - static_cast<int>(cadence));
            if (cadenceDiff < bestCadenceDiff) {
                bestCadenceDiff = cadenceDiff;
                closestCadencePoints.clear();
                closestCadencePoints.append(point);
            } else if (cadenceDiff == bestCadenceDiff) {
                closestCadencePoints.append(point);
            }
        }

        if (closestCadencePoints.isEmpty())
            return 0.0;

        bool hasLower = false;
        bool hasUpper = false;
        uint16_t lowerWatt = 0, upperWatt = 0;
        double lowerInclination = 0.0, upperInclination = 0.0;
        double closestInclination = closestCadencePoints.first().inclination;
        int minDiff = qAbs(static_cast<int>(closestCadencePoints.first().wattage) - static_cast<int>(power));

        for (const auto &point : closestCadencePoints) {
            int diff = qAbs(static_cast<int>(point.wattage) - static_cast<int>(power));
            if (diff < minDiff) {
                minDiff = diff;
                closestInclination = point.inclination;
            }

            if (point.wattage <= power && (!hasLower || point.wattage > lowerWatt)) {
                hasLower = true;
                lowerWatt = point.wattage;
                lowerInclination = point.inclination;
            }
            if (point.wattage >= power && (!hasUpper || point.wattage < upperWatt)) {
                hasUpper = true;
                upperWatt = point.wattage;
                upperInclination = point.inclination;
            }
        }

        if (hasLower && hasUpper && upperWatt != lowerWatt) {
            double ratio = static_cast<double>(power - lowerWatt) / static_cast<double>(upperWatt - lowerWatt);
            return lowerInclination + ratio * (upperInclination - lowerInclination);
        }

        return closestInclination;
    }

    QList<inclinationPowerDataPoint> getConsolidatedData() const { return consolidatedData; }
    bool hasData() const { return !consolidatedData.isEmpty(); }

  private:
    QMap<CadenceInclinationKey, WattageSamples> wattageData;
    QList<inclinationPowerDataPoint> consolidatedData;
    double lastInclinationValue = -999.0;
    QDateTime lastInclinationTime = QDateTime::currentDateTime();

    void updateDataTable(const CadenceInclinationKey &key) {
        uint16_t medianWatt = wattageData[key].getMedian();
        double inclination = static_cast<double>(key.inclinationTimes10) / 10.0;

        for (int i = consolidatedData.size() - 1; i >= 0; --i) {
            if (consolidatedData[i].cadence == key.cadence &&
                qAbs(consolidatedData[i].inclination - inclination) < 0.05) {
                consolidatedData.removeAt(i);
                break;
            }
        }

        consolidatedData.append(inclinationPowerDataPoint(key.cadence, inclination, medianWatt));
        saveSettings();
    }

    void loadSettings() {
        QSettings settings;
        QString data = settings.value(QZSettings::inclinationPowerDataPoints, QString()).toString();
        QStringList dataList = data.split(";", Qt::SkipEmptyParts);

        for (const QString &triple : dataList) {
            QStringList fields = triple.split("|");
            if (fields.size() == 3) {
                double inclination = fields[0].toDouble();
                uint16_t watt = fields[1].toUInt();
                uint16_t cadence = fields[2].toUInt();
                consolidatedData.append(inclinationPowerDataPoint(cadence, inclination, watt));
            }
        }
    }

    void saveSettings() {
        QSettings settings;
        QStringList dataStrings;

        for (const inclinationPowerDataPoint &point : consolidatedData) {
            dataStrings.append(QString("%1|%2|%3")
                                   .arg(point.inclination, 0, 'f', 2)
                                   .arg(point.wattage)
                                   .arg(point.cadence));
        }

        settings.setValue(QZSettings::inclinationPowerDataPoints, dataStrings.join(";"));
    }
};

#endif // INCLINATIONPOWERTABLE_H
