#ifndef ERGTABLE_H
#define ERGTABLE_H

#include <QList>
#include <QSettings>
#include <QObject>
#include <QDebug>
#include <QDateTime>
#include <QMap>
#include <QVector>
#include <algorithm>
#include <cmath>
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

    /**
     * @brief Declare that the resistance the device reports back is the level it was commanded,
     * not the position the magnets have physically reached.
     *
     * The settling window below exists because on most bikes a sample taken right after a change
     * is attributed to a level the magnets have not arrived at yet. Where the console computes
     * the power it publishes from the commanded level, there is nothing to settle - the reported
     * wattage already belongs to the reported level - and holding the window open only throws
     * data away. It throws away exactly the data that matters, too: with the resistance slew
     * limiter ramping a level at a time, every level a ramp passes through is held for less than
     * the window, so the levels that are only ever visited in passing never get a single sample.
     */
    void setResistanceReportsCommand(bool value) { resistanceReportsCommand = value; }

    void collectData(uint16_t cadence, uint16_t wattage, uint16_t resistance, bool ignoreResistanceTiming = false) {
        if (resistance != lastResistanceValue) {
            qDebug() << "resistance changed";
            lastResistanceTime = QDateTime::currentDateTime();
            lastResistanceValue = resistance;
        }

        if (lastResistanceTime.msecsTo(QDateTime::currentDateTime()) < 1000 && !ignoreResistanceTiming &&
            !resistanceReportsCommand) {
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

    /**
     * @brief Estimated wattage for a (cadence, resistance) pair.
     *
     * Both axes are handled the same way: interpolate between the measurements that bracket the
     * request, and follow the local slope out when the request falls outside them.
     *
     * Returning the nearest measurement instead of extrapolating - the behaviour this replaces -
     * skewed ERG in one direction. Below the lowest cadence a level was ever learned at, the
     * table quoted the wattage of a *faster* cadence, so the selector believed the level made
     * more power than it does and chose one too low; the rider then came up short exactly when
     * the cadence dropped. Measured on a YPOO console: 26 W short of a 74 W target at 59 rpm,
     * against 4 W short above 75 rpm.
     */
    double estimateWattage(uint16_t givenCadence, uint16_t givenResistance) {
        if (consolidatedData.isEmpty()) return 0;

        bool found = false;
        const double watts = wattageAtResistance(givenCadence, givenResistance, &found);
        if (found)
            return watts;

        // This level has no samples at all. Interpolate between the nearest level below and the
        // nearest above, rather than adopting the closest level's numbers wholesale: a level
        // that borrowed a neighbour learned only at high cadence used to read far too high at
        // every cadence, and being both untouched and unreachable it stayed that way - a wall
        // the selector's upward scan stopped at.
        bool haveBelow = false, haveAbove = false;
        uint16_t below = 0, above = 0;
        for (const auto& point : consolidatedData) {
            if (point.resistance < givenResistance && (!haveBelow || point.resistance > below)) {
                below = point.resistance;
                haveBelow = true;
            }
            if (point.resistance > givenResistance && (!haveAbove || point.resistance < above)) {
                above = point.resistance;
                haveAbove = true;
            }
        }

        if (haveBelow && haveAbove) {
            const double lowerWatts = wattageAtResistance(givenCadence, below, &found);
            const double upperWatts = wattageAtResistance(givenCadence, above, &found);
            const double ratio = double(givenResistance - below) / double(above - below);
            return lowerWatts + ratio * (upperWatts - lowerWatts);
        }
        if (haveBelow)
            return wattageAtResistance(givenCadence, below, &found);
        if (haveAbove)
            return wattageAtResistance(givenCadence, above, &found);
        return 0;
    }

    /**
     * @brief The resistance level whose estimated wattage is closest to the requested power.
     *
     * The estimates are not guaranteed to rise with the resistance - a level with few samples
     * can read below its neighbour - so the curve is forced non-decreasing before it is
     * searched. Scanning for the first level that brackets the target over a curve that dips
     * and spikes returns whichever bump comes first, which is how a 93 W request landed on
     * level 4 on a table whose level 4 makes 68 W and whose level 16 makes 89 W.
     *
     * Taking the nearest level rather than the lower end of a bracket also removes a
     * systematic undershoot of roughly one level, worth 3-15 W depending on where in the
     * range the bike is.
     */
    uint16_t resistanceFromPowerRequest(uint16_t power, uint16_t cadence, uint16_t maxResistance) {
        qDebug() << QStringLiteral("resistanceFromPowerRequest") << cadence;

        if (cadence == 0)
            return 1;

        if (maxResistance < 2)
            return 1;

        QVector<double> estimates;
        estimates.reserve(maxResistance - 1);
        for (uint16_t i = 1; i < maxResistance; i++)
            estimates.append(estimateWattage(cadence, i));

        const QVector<double> curve = nonDecreasing(estimates);

        uint16_t best_resistance_match = 1;
        double best_difference = -1;
        double best_watts = 0;

        for (int i = 0; i < curve.size(); i++) {
            // Strictly less than, so a plateau keeps the lowest level that reaches the target.
            const double difference = std::fabs(curve.at(i) - double(power));
            if (best_difference < 0 || difference < best_difference) {
                best_difference = difference;
                best_resistance_match = (uint16_t)(i + 1);
                best_watts = curve.at(i);
            }
        }

        qDebug() << QStringLiteral("resistanceFromPowerRequest: target") << power << QStringLiteral("cadence")
                 << cadence << QStringLiteral("-> resistance") << best_resistance_match << QStringLiteral("estimated")
                 << best_watts;
        return best_resistance_match;
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
    QDateTime lastResistanceTime = QDateTime::currentDateTime();
    bool resistanceReportsCommand = false;

    /**
     * @brief Wattage from the samples of a single resistance level.
     * @param found Set to false when that level has no samples at all, so the caller can fall
     *        back to its neighbours instead of mistaking an empty level for 0 W.
     */
    double wattageAtResistance(uint16_t givenCadence, uint16_t resistance, bool* found) {
        QList<ergDataPoint> samples;
        for (const auto& point : consolidatedData) {
            if (point.resistance == resistance) {
                samples.append(point);
            }
        }

        *found = !samples.isEmpty();
        if (samples.isEmpty())
            return 0;

        std::sort(samples.begin(), samples.end(),
                  [](const ergDataPoint& a, const ergDataPoint& b) { return a.cadence < b.cadence; });

        // Outside the measured range in either direction: carry on along the slope of the two
        // nearest samples. Wattage is close enough to linear in cadence at a fixed resistance
        // over the span a rider actually covers for this to beat repeating the endpoint.
        if (givenCadence < samples.first().cadence) {
            if (samples.size() < 2)
                return scaledByCadence(samples.first(), givenCadence);
            return lineThrough(samples.at(0), samples.at(1), givenCadence);
        }
        if (givenCadence > samples.last().cadence) {
            if (samples.size() < 2)
                return scaledByCadence(samples.last(), givenCadence);
            return lineThrough(samples.at(samples.size() - 2), samples.last(), givenCadence);
        }

        for (int i = 1; i < samples.size(); i++) {
            if (samples.at(i).cadence >= givenCadence)
                return lineThrough(samples.at(i - 1), samples.at(i), givenCadence);
        }

        // A single sample, at exactly this cadence.
        return samples.first().wattage;
    }

    /**
     * @brief The closest non-decreasing curve to a set of wattage estimates.
     *
     * Pool adjacent violators: wherever a level reads below the one before it, the two are
     * replaced by their mean, and the merge repeats backwards until the curve rises again.
     *
     * More resistance cannot make less power, so anything that dips is noise - a level with one
     * bad sample, or one learned at a cadence nobody rides. Simply carrying the running maximum
     * forward would fix the dips but smear a *spike* across every level above it, which is the
     * more dangerous of the two: it hands a high target to a low level. Pooling absorbs a spike
     * into its neighbours instead.
     */
    static QVector<double> nonDecreasing(const QVector<double>& values) {
        QVector<double> level;   // the pooled value of each block
        QVector<int> width;      // how many levels that block covers
        level.reserve(values.size());
        width.reserve(values.size());

        for (const double value : values) {
            level.append(value);
            width.append(1);

            while (level.size() > 1 && level.at(level.size() - 2) > level.last()) {
                const int merged = width.at(width.size() - 2) + width.last();
                const double total =
                    level.at(level.size() - 2) * width.at(width.size() - 2) + level.last() * width.last();
                level.removeLast();
                width.removeLast();
                level[level.size() - 1] = total / double(merged);
                width[width.size() - 1] = merged;
            }
        }

        QVector<double> result;
        result.reserve(values.size());
        for (int i = 0; i < level.size(); i++)
            for (int j = 0; j < width.at(i); j++)
                result.append(level.at(i));
        return result;
    }

    /**
     * @brief The wattage the line through two samples gives at a cadence, never below zero.
     */
    static double lineThrough(const ergDataPoint& a, const ergDataPoint& b, uint16_t cadence) {
        if (a.cadence == b.cadence)
            return a.wattage;

        const double slope = (double(b.wattage) - double(a.wattage)) / (double(b.cadence) - double(a.cadence));
        const double watts = double(a.wattage) + slope * (double(cadence) - double(a.cadence));
        return watts > 0 ? watts : 0;
    }

    /**
     * @brief Last resort for a level with one sample: assume wattage scales with cadence.
     *
     * Wrong in detail - the real relationship has an offset - but right in direction, which is
     * all that is needed to stop a single high-cadence sample from claiming the same wattage
     * all the way down.
     */
    static double scaledByCadence(const ergDataPoint& point, uint16_t cadence) {
        if (point.cadence == 0)
            return point.wattage;
        return double(point.wattage) * double(cadence) / double(point.cadence);
    }

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

#endif // ERGTABLE_H
