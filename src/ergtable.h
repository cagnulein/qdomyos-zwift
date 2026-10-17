#ifndef ERGTABLE_H
#define ERGTABLE_H

#include <QList>
#include <QSettings>
#include <QObject>
#include <QDebug>
#include <QDateTime>
#include <QMap>
#include <algorithm>
#include <cmath>
#include "qzsettings.h"

struct ergDataPoint {
    // wattage keeps the historical table value; trainerWattage is the optional paired trainer value.
    uint16_t cadence = 0;
    uint16_t wattage = 0;
    uint16_t resistance = 0;
    uint16_t trainerWattage = 0;

    ergDataPoint() = default;
    ergDataPoint(uint16_t c, uint16_t w, uint16_t r) : cadence(c), wattage(w), resistance(r) {}
    ergDataPoint(uint16_t c, uint16_t w, uint16_t r, uint16_t trainerW)
        : cadence(c), wattage(w), resistance(r), trainerWattage(trainerW) {}
};

Q_DECLARE_METATYPE(ergDataPoint)

struct CadenceResistancePair {
    uint16_t cadence;
    uint16_t resistance;

    bool operator<(const CadenceResistancePair& other) const {
        if (resistance != other.resistance) return resistance < other.resistance;
        return cadence < other.cadence;
    }
};

class WattageStats {
  public:
    static const int MAX_SAMPLES = 100;
    static const int MIN_SAMPLES_REQUIRED = 10;

    void addSample(uint16_t wattage, uint16_t trainerWattage = 0) {
        samples.append(wattage);
        if (samples.size() > MAX_SAMPLES) {
            samples.removeFirst();
        }
        if (trainerWattage > 0) {
            trainerSamples.append(trainerWattage);
            if (trainerSamples.size() > MAX_SAMPLES) {
                trainerSamples.removeFirst();
            }
        }
        medianNeedsUpdate = true;
        trainerMedianNeedsUpdate = true;
    }

    uint16_t getMedian() {
        if (!medianNeedsUpdate) return cachedMedian;
        if (samples.isEmpty()) return 0;

        QList<uint16_t> sortedSamples = samples;
        std::sort(sortedSamples.begin(), sortedSamples.end());

        int middle = sortedSamples.size() / 2;
        if (sortedSamples.size() % 2 == 0) {
            cachedMedian = (sortedSamples[middle-1] + sortedSamples[middle]) / 2;
        } else {
            cachedMedian = sortedSamples[middle];
        }

        medianNeedsUpdate = false;
        return cachedMedian;
    }

    int sampleCount() const {
        return samples.size();
    }

    int trainerSampleCount() const {
        return trainerSamples.size();
    }

    uint16_t getTrainerMedian() {
        if (!trainerMedianNeedsUpdate) return cachedTrainerMedian;
        if (trainerSamples.isEmpty()) return 0;

        QList<uint16_t> sortedSamples = trainerSamples;
        std::sort(sortedSamples.begin(), sortedSamples.end());

        int middle = sortedSamples.size() / 2;
        if (sortedSamples.size() % 2 == 0) {
            cachedTrainerMedian = (sortedSamples[middle - 1] + sortedSamples[middle]) / 2;
        } else {
            cachedTrainerMedian = sortedSamples[middle];
        }

        trainerMedianNeedsUpdate = false;
        return cachedTrainerMedian;
    }

    void clear() {
        samples.clear();
        trainerSamples.clear();
        cachedMedian = 0;
        cachedTrainerMedian = 0;
        medianNeedsUpdate = true;
        trainerMedianNeedsUpdate = true;
    }

  private:
    QList<uint16_t> samples;
    QList<uint16_t> trainerSamples;
    uint16_t cachedMedian = 0;
    uint16_t cachedTrainerMedian = 0;
    bool medianNeedsUpdate = true;
    bool trainerMedianNeedsUpdate = true;
};

class ergTable : public QObject {
    Q_OBJECT

  public:
    ergTable(QObject *parent = nullptr) : QObject(parent) {
        loadSettings();
    }

    // No save here: every change is already saved when it happens. Every bluetoothdevice owns an
    // ergTable (heart rate belts and sensors too), so saving in the destructor let a copy loaded at
    // startup overwrite the points the bike learned during the session.
    ~ergTable() {}

    void reset() {
        wattageData.clear();
        consolidatedData.clear();
        lastResistanceValue = 0xFFFF;
        lastResistanceTime = QDateTime::currentDateTime();

        // Clear the settings completely
        QSettings settings;
        settings.remove(QZSettings::ergDataPoints);
        settings.sync();
    }

    void collectData(uint16_t cadence, uint16_t wattage, uint16_t resistance, bool ignoreResistanceTiming = false) {
        collectData(cadence, wattage, resistance, 0, ignoreResistanceTiming);
    }

    void collectData(uint16_t cadence, uint16_t wattage, uint16_t resistance, uint16_t trainerWattage,
                     bool ignoreResistanceTiming = false) {
        if (resistance != lastResistanceValue) {
            qDebug() << "resistance changed";
            lastResistanceTime = QDateTime::currentDateTime();
            lastResistanceValue = resistance;
        }

        if (lastResistanceTime.msecsTo(QDateTime::currentDateTime()) < 1000 && !ignoreResistanceTiming) {
            qDebug() << "skipping collecting data due to resistance changing too fast";
            return;
        }

        if (wattage > 0 && cadence > 0) {
            CadenceResistancePair pair{cadence, resistance};
            wattageData[pair].addSample(wattage, trainerWattage);

            if (wattageData[pair].sampleCount() >= WattageStats::MIN_SAMPLES_REQUIRED) {
                updateDataTable(pair);
            }
        }
    }

    double estimateWattage(uint16_t givenCadence, uint16_t givenResistance) {
        if (consolidatedData.isEmpty()) return 0;

               // Get all points with matching resistance
        QList<ergDataPoint> sameResPoints;
        for (const auto& point : consolidatedData) {
            if (point.resistance == givenResistance) {
                sameResPoints.append(point);
            }
        }

               // If no exact resistance match, find closest resistance
        if (sameResPoints.isEmpty()) {
            uint16_t minResDiff = UINT16_MAX;
            uint16_t closestRes = 0;

            for (const auto& point : consolidatedData) {
                uint16_t resDiff = abs(int(point.resistance) - int(givenResistance));
                if (resDiff < minResDiff) {
                    minResDiff = resDiff;
                    closestRes = point.resistance;
                }
            }

            for (const auto& point : consolidatedData) {
                if (point.resistance == closestRes) {
                    sameResPoints.append(point);
                }
            }
        }

               // Find points for interpolation
        double lowerWatts = 0, upperWatts = 0;
        uint16_t lowerCadence = 0, upperCadence = 0;

        for (const auto& point : sameResPoints) {
            if (point.cadence <= givenCadence && point.cadence > lowerCadence) {
                lowerWatts = point.wattage;
                lowerCadence = point.cadence;
            }
            if (point.cadence >= givenCadence && (upperCadence == 0 || point.cadence < upperCadence)) {
                upperWatts = point.wattage;
                upperCadence = point.cadence;
            }
        }

               // Interpolate or use closest value
        if (lowerCadence != 0 && upperCadence != 0 && lowerCadence != upperCadence) {
            double ratio = (givenCadence - lowerCadence) / double(upperCadence - lowerCadence);
            return lowerWatts + ratio * (upperWatts - lowerWatts);
        } else if (lowerCadence != 0) {
            return lowerWatts;
        } else if (upperCadence != 0) {
            return upperWatts;
        }

               // Fallback to closest point
        return sameResPoints.first().wattage;
    }

    void setCadenceResistanceBandStep(uint16_t step) {
        cadenceResistanceBandStep = step;
    }

    uint16_t resistanceFromPowerRequest(uint16_t power, uint16_t cadence, uint16_t maxResistance) {
        if (cadenceResistanceBandStep > 1) {
            cadence = (cadence / cadenceResistanceBandStep) * cadenceResistanceBandStep;
        }

        qDebug() << QStringLiteral("resistanceFromPowerRequest") << cadence;

        if (cadence == 0)
            return 1;

        uint16_t best_resistance_match = 1;
        int min_watt_difference = 1000; 

        for (uint16_t i = 1; i < maxResistance; i++) {
            uint16_t current_watts = estimateWattage(cadence, i);
            uint16_t next_watts = estimateWattage(cadence, i + 1);

            if (current_watts <= power && next_watts >= power) {
                qDebug() << current_watts << next_watts << power;
                return i;
            }

            int diff = abs(current_watts - power);
            if (diff < min_watt_difference) {
                min_watt_difference = diff;
                best_resistance_match = i;
                qDebug() << QStringLiteral("best match") << best_resistance_match << "with watts" << current_watts << "diff" << diff;
            }
        }

        qDebug() << "Bracketing not found, best match:" << best_resistance_match;
        return best_resistance_match;
    }

    int32_t trainerPowerForPedalTarget(uint16_t pedalTarget, uint16_t givenCadence,
                                       uint16_t givenResistance) const {
        QList<ergDataPoint> pairedPoints;
        for (const auto &point : consolidatedData) {
            if (point.trainerWattage > 0) {
                pairedPoints.append(point);
            }
        }
        if (pairedPoints.isEmpty()) {
            return 0;
        }

        uint16_t minResistanceDifference = UINT16_MAX;
        uint16_t closestResistance = 0;
        for (const auto &point : pairedPoints) {
            const uint16_t resistanceDifference =
                static_cast<uint16_t>(abs(static_cast<int>(point.resistance) - static_cast<int>(givenResistance)));
            if (resistanceDifference < minResistanceDifference) {
                minResistanceDifference = resistanceDifference;
                closestResistance = point.resistance;
            }
        }

        double lowerDelta = 0;
        double upperDelta = 0;
        uint16_t lowerCadence = 0;
        uint16_t upperCadence = 0;
        bool hasPoint = false;

        for (const auto &point : pairedPoints) {
            if (point.resistance != closestResistance) {
                continue;
            }

            const double delta = static_cast<double>(point.trainerWattage) - point.wattage;
            hasPoint = true;
            if (point.cadence <= givenCadence && point.cadence > lowerCadence) {
                lowerDelta = delta;
                lowerCadence = point.cadence;
            }
            if (point.cadence >= givenCadence && (upperCadence == 0 || point.cadence < upperCadence)) {
                upperDelta = delta;
                upperCadence = point.cadence;
            }
        }

        if (!hasPoint) {
            return 0;
        }

        double delta = 0;
        if (lowerCadence != 0 && upperCadence != 0 && lowerCadence != upperCadence) {
            const double ratio = (givenCadence - lowerCadence) /
                                 static_cast<double>(upperCadence - lowerCadence);
            delta = lowerDelta + ratio * (upperDelta - lowerDelta);
        } else if (lowerCadence != 0) {
            delta = lowerDelta;
        } else if (upperCadence != 0) {
            delta = upperDelta;
        } else {
            for (const auto &point : pairedPoints) {
                if (point.resistance == closestResistance) {
                    delta = static_cast<double>(point.trainerWattage) - point.wattage;
                    break;
                }
            }
        }

        return std::max<int32_t>(0, static_cast<int32_t>(std::lround(pedalTarget + delta)));
    }

    QList<ergDataPoint> getConsolidatedData() const {
        return consolidatedData;
    }

    QMap<CadenceResistancePair, WattageStats> getWattageData() const {
        return wattageData;
    }

    uint16_t getMaxResistance() const {
        if (consolidatedData.isEmpty()) return 0;

        uint16_t maxRes = 0;
        for (const auto& point : consolidatedData) {
            if (point.resistance > maxRes) {
                maxRes = point.resistance;
            }
        }
        return maxRes;
    }

    /**
     * @brief Load default calibration data for a specific bike model.
     * Only populates if the table is currently empty (won't overwrite user's learned data).
     * Data format: "cadence|wattage|resistance;cadence|wattage|resistance;..."
     */
    void loadDefaultData(const QString& defaultDataString) {
        if (!consolidatedData.isEmpty()) {
            qDebug() << "ergTable: skipping defaults, user data already exists ("
                     << consolidatedData.size() << "points)";
            return;
        }
        QStringList dataList = defaultDataString.split(";", Qt::SkipEmptyParts);
        for (const QString& triple : dataList) {
            QStringList fields = triple.split("|");
            if (fields.size() == 3) {
                uint16_t cadence = fields[0].toUInt();
                uint16_t wattage = fields[1].toUInt();
                uint16_t resistance = fields[2].toUInt();
                consolidatedData.append(ergDataPoint(cadence, wattage, resistance));
            }
        }
        qDebug() << "ergTable: loaded" << consolidatedData.size() << "default data points";
        saveSettings();
    }

  private:
    QMap<CadenceResistancePair, WattageStats> wattageData;
    QList<ergDataPoint> consolidatedData;
    uint16_t lastResistanceValue = 0xFFFF;
    uint16_t cadenceResistanceBandStep = 0;
    QDateTime lastResistanceTime = QDateTime::currentDateTime();

    void updateDataTable(const CadenceResistancePair& pair) {
        WattageStats& stats = wattageData[pair];
        uint16_t medianWattage = stats.getMedian();
        uint16_t medianTrainerWattage =
            stats.trainerSampleCount() >= WattageStats::MIN_SAMPLES_REQUIRED ? stats.getTrainerMedian() : 0;

        // Remove existing point if it exists
        for (int i = consolidatedData.size() - 1; i >= 0; --i) {
            if (consolidatedData[i].cadence == pair.cadence &&
                consolidatedData[i].resistance == pair.resistance) {
                // This runs on every metrics update once a pair has enough samples; when the median
                // did not move there is nothing to update, and rewriting the settings file would
                // only cost I/O on the main thread.
                if (consolidatedData[i].wattage == medianWattage &&
                    consolidatedData[i].trainerWattage == medianTrainerWattage)
                    return;
                consolidatedData.removeAt(i);
                break;
            }
        }

        // Add new point
        consolidatedData.append(ergDataPoint(pair.cadence, medianWattage, pair.resistance, medianTrainerWattage));

        qDebug() << "Added/Updated point:"
                 << "C:" << pair.cadence
                 << "W:" << medianWattage
                 << "TW:" << medianTrainerWattage
                 << "R:" << pair.resistance;
        saveSettings();
    }

    void loadSettings() {
        QSettings settings;
        QString data = settings.value(QZSettings::ergDataPoints,
                                      QZSettings::default_ergDataPoints).toString();
        QStringList dataList = data.split(";", Qt::SkipEmptyParts);

        for (const QString& triple : dataList) {
            QStringList fields = triple.split("|");
            if (fields.size() == 3 || fields.size() == 4) {
                uint16_t cadence = fields[0].toUInt();
                uint16_t wattage = fields[1].toUInt();
                uint16_t resistance = fields[2].toUInt();
                uint16_t trainerWattage = fields.size() == 4 ? fields[3].toUInt() : 0;
                consolidatedData.append(ergDataPoint(cadence, wattage, resistance, trainerWattage));
            }
        }
    }

    void saveSettings() {
        QSettings settings;
        QStringList dataStrings;

        for (const ergDataPoint& point : consolidatedData) {
            dataStrings.append(QString("%1|%2|%3|%4").arg(point.cadence)
                                   .arg(point.wattage)
                                   .arg(point.resistance)
                                   .arg(point.trainerWattage));
        }

        settings.setValue(QZSettings::ergDataPoints, dataStrings.join(";"));
    }
};

#endif // ERGTABLE_H
