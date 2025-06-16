#ifndef ERGTABLE_H
#define ERGTABLE_H

#include <QList>
#include <QSettings>
#include <QObject>
#include <QDebug>
#include <QDateTime>
#include <QMap>
#include <algorithm>
#include "qzsettings.h"

struct ergDataPoint {
    uint16_t cadence = 0;
    uint16_t wattage = 0;
    uint16_t resistance = 0;

    ergDataPoint() = default;
    ergDataPoint(uint16_t c, uint16_t w, uint16_t r) : cadence(c), wattage(w), resistance(r) {}
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

    void addSample(uint16_t wattage) {
        samples.append(wattage);
        if (samples.size() > MAX_SAMPLES) {
            samples.removeFirst();
        }
        medianNeedsUpdate = true;
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

class ergTable : public QObject {
    Q_OBJECT

  public:
    ergTable(QObject *parent = nullptr) : QObject(parent) {
        loadSettings();
    }

    ~ergTable() {
        saveSettings();
    }

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
            wattageData[pair].addSample(wattage);

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

    QList<ergDataPoint> getConsolidatedData() const {
        return consolidatedData;
    }

    QMap<CadenceResistancePair, WattageStats> getWattageData() const {
        return wattageData;
    }

  private:
    QMap<CadenceResistancePair, WattageStats> wattageData;
    QList<ergDataPoint> consolidatedData;
    uint16_t lastResistanceValue = 0xFFFF;
    QDateTime lastResistanceTime = QDateTime::currentDateTime();

    void updateDataTable(const CadenceResistancePair& pair) {
        uint16_t medianWattage = wattageData[pair].getMedian();

        // Remove existing point if it exists
        for (int i = consolidatedData.size() - 1; i >= 0; --i) {
            if (consolidatedData[i].cadence == pair.cadence &&
                consolidatedData[i].resistance == pair.resistance) {
                consolidatedData.removeAt(i);
                break;
            }
        }

        // Add new point
        consolidatedData.append(ergDataPoint(pair.cadence, medianWattage, pair.resistance));
        qDebug() << "Added/Updated point:"
                 << "C:" << pair.cadence
                 << "W:" << medianWattage
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
            if (fields.size() == 3) {
                uint16_t cadence = fields[0].toUInt();
                uint16_t wattage = fields[1].toUInt();
                uint16_t resistance = fields[2].toUInt();
                consolidatedData.append(ergDataPoint(cadence, wattage, resistance));
            }
        }
    }

    void saveSettings() {
        QSettings settings;
        QStringList dataStrings;

        for (const ergDataPoint& point : consolidatedData) {
            dataStrings.append(QString("%1|%2|%3").arg(point.cadence)
                                   .arg(point.wattage)
                                   .arg(point.resistance));
        }

        settings.setValue(QZSettings::ergDataPoints, dataStrings.join(";"));
    }
};

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

#endif // ERGTABLE_H
