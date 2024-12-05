#ifndef ERGTABLE_TEST_H
#define ERGTABLE_TEST_H

#include "ergtable.h"
#include <QDebug>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QThread>

struct ResistanceStats {
    int samples = 0;
    double totalWatts = 0;
    uint16_t minWatts = UINT16_MAX;
    uint16_t maxWatts = 0;
    uint16_t minCadence = UINT16_MAX;
    uint16_t maxCadence = 0;
};

class ergTableTester {
  public:
    struct TrainingDataPoint {
        QDateTime timestamp;
        uint16_t cadence;
        uint16_t wattage;
        uint16_t resistance;
        bool isResistanceChange;

        TrainingDataPoint(const QDateTime& ts, uint16_t c, uint16_t w, uint16_t r, bool isResChange = false)
            : timestamp(ts), cadence(c), wattage(w), resistance(r), isResistanceChange(isResChange) {}
    };

    static bool runAllTests() {
        ergTableTester tester;
        tester.testTrainingSession();
        return tester.testPowerEstimationTable();
    }

  private:
    std::vector<TrainingDataPoint> loadTrainingData(const QString& filename) {
        std::vector<TrainingDataPoint> data;
        QFile file(filename);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Error opening file:" << filename;
            return data;
        }

        QTextStream in(&file);
        uint16_t lastCadence = 0;
        uint16_t lastWattage = 0;
        uint16_t currentResistance = 18; // Initial resistance
        QDateTime lastTimestamp;

        QRegularExpression timeRegex("(\\d{2}:\\d{2}:\\d{2}) (\\d{4}) (\\d+)");
        QRegularExpression cadenceRegex("Cadence: (\\d+)");
        QRegularExpression wattRegex("watt: (\\d+)");
        QRegularExpression resistanceRegex("resistance: (\\d+)");

        while (!in.atEnd()) {
            QString line = in.readLine();

            // Extract timestamp
            QRegularExpressionMatch timeMatch = timeRegex.match(line);
            if (timeMatch.hasMatch()) {
                QString timeStr = timeMatch.captured(1);
                QString yearStr = timeMatch.captured(2);
                qint64 msecs = timeMatch.captured(3).toLongLong();
                lastTimestamp = QDateTime::fromString(timeStr + " " + yearStr, "hh:mm:ss yyyy");
                lastTimestamp = lastTimestamp.addMSecs(msecs % 1000);
            }

                   // Extract resistance changes
            QRegularExpressionMatch resistanceMatch = resistanceRegex.match(line);
            if (resistanceMatch.hasMatch()) {
                uint16_t newResistance = resistanceMatch.captured(1).toUInt();
                if (newResistance != currentResistance) {
                    // Add resistance change point
                    data.emplace_back(lastTimestamp, lastCadence, lastWattage, newResistance, true);
                    currentResistance = newResistance;
                    qDebug() << "Resistance changed to:" << currentResistance
                             << "at" << lastTimestamp.toString("hh:mm:ss.zzz");
                }
            }

            // Extract cadence
            QRegularExpressionMatch cadenceMatch = cadenceRegex.match(line);
            if (cadenceMatch.hasMatch()) {
                lastCadence = cadenceMatch.captured(1).toUInt();
            }

            // Extract wattage
            QRegularExpressionMatch wattMatch = wattRegex.match(line);
            if (wattMatch.hasMatch()) {
                lastWattage = wattMatch.captured(1).toUInt();
                if (lastTimestamp.isValid()) {
                    data.emplace_back(lastTimestamp, lastCadence, lastWattage, currentResistance, false);
                }
            }
        }

        file.close();

        // Sort by timestamp
        std::sort(data.begin(), data.end(),
                  [](const TrainingDataPoint& a, const TrainingDataPoint& b) {
                      return a.timestamp < b.timestamp;
                  });

        return data;
    }

    bool testPowerEstimationTable() {
        qDebug() << "\nTesting power estimation table...";
        ergTable table;
        table.reset();

               // First load and process the training data to populate the table
        auto trainingData = loadTrainingData("c:/powertraining.txt");
        QDateTime lastResistanceChange;
        QDateTime lastTimestamp;

        // Process all training data to populate the table
        for (const auto& point : trainingData) {
            // Calculate and simulate real time delta
            if (!lastTimestamp.isNull()) {
                qint64 msSinceLastSample = lastTimestamp.msecsTo(point.timestamp);
                QThread::msleep(std::min(static_cast<qint64>(10), msSinceLastSample));
            }
            lastTimestamp = point.timestamp;

            if (point.isResistanceChange) {
                lastResistanceChange = point.timestamp;
                continue;
            }

            // Skip data points too close to resistance changes
            if (!lastResistanceChange.isNull()) {
                qint64 msSinceResistanceChange = lastResistanceChange.msecsTo(point.timestamp);
                if (msSinceResistanceChange < 1000) {
                    continue;
                }
            }

            // Populate the table with training data
            table.collectData(point.cadence, point.wattage, point.resistance);
        }

               // Now create formatted table header
        qDebug() << "\nPower Estimation Table (Watts)";
        qDebug() << "Cadence |  R25  |  R26  |  R27  |  R28  |  R29  |  R30  |  R31  |  R32";
        qDebug() << "--------|-------|-------|-------|-------|-------|-------|-------|-------";

               // Generate table for cadences 50-80
        for (uint16_t cadence = 50; cadence <= 80; cadence += 2) {
            QString line = QString("%1").arg(cadence, 7);
            line += " |";

                   // Test each resistance level
            for (uint16_t resistance = 25; resistance <= 32; resistance++) {
                double watts = table.estimateWattage(cadence, resistance);
                // Format with 1 decimal place and right-aligned in 6 chars + separator
                line += QString(" %1 |").arg(QString::number(watts, 'f', 1), 6);
            }
            qDebug().noquote() << line;
        }

               // Analyze some specific power targets
        qDebug() << "\nPower Target Analysis:";
        QVector<int> powerTargets = {200, 250, 300, 350, 400};

        qDebug() << "Target |  Cadence 60  |  Cadence 70  |  Cadence 80";
        qDebug() << "-------|--------------|--------------|-------------";

        for (int targetPower : powerTargets) {
            QString line = QString("%1W").arg(targetPower, 6);
            line += " |";

            // Find closest resistance for different cadences
            for (uint16_t cadence : {60, 70, 80}) {
                uint16_t bestResistance = 25;
                double closestPower = table.estimateWattage(cadence, bestResistance);
                double minDiff = std::abs(closestPower - targetPower);

                // Try each resistance level
                for (uint16_t r = 26; r <= 32; r++) {
                    double power = table.estimateWattage(cadence, r);
                    double diff = std::abs(power - targetPower);
                    if (diff < minDiff) {
                        minDiff = diff;
                        closestPower = power;
                        bestResistance = r;
                    }
                }

                // Add to output: "R28 (245W)"
                line += QString("  R%1 (%2W) |")
                            .arg(bestResistance)
                            .arg(qRound(closestPower));
            }
            qDebug().noquote() << line;
        }

               // Print some stats about the collected data
        auto consolidatedData = table.getConsolidatedData();
        qDebug() << "\nTable populated with" << consolidatedData.size() << "data points";

        // Print min/max values found in consolidated data
        if (!consolidatedData.isEmpty()) {
            uint16_t minCadence = UINT16_MAX, maxCadence = 0;
            uint16_t minResistance = UINT16_MAX, maxResistance = 0;
            uint16_t minWatts = UINT16_MAX, maxWatts = 0;

            for (const auto& point : consolidatedData) {
                minCadence = std::min(minCadence, point.cadence);
                maxCadence = std::max(maxCadence, point.cadence);
                minResistance = std::min(minResistance, point.resistance);
                maxResistance = std::max(maxResistance, point.resistance);
                minWatts = std::min(minWatts, point.wattage);
                maxWatts = std::max(maxWatts, point.wattage);
            }

            qDebug() << "Data ranges:";
            qDebug() << "  Cadence:" << minCadence << "-" << maxCadence << "RPM";
            qDebug() << "  Resistance:" << minResistance << "-" << maxResistance;
            qDebug() << "  Power:" << minWatts << "-" << maxWatts << "W";
        }

        return true;
    }

    bool testTrainingSession() {
        qDebug() << "Testing with real training session data...";
        ergTable table;
        table.reset();

        auto trainingData = loadTrainingData("c:/powertraining.txt");
        qDebug() << "Loaded" << trainingData.size() << "data points";

        std::map<uint16_t, ResistanceStats> resistanceStats;
        QDateTime lastTimestamp;
        QDateTime lastResistanceChange;

        for (const auto& point : trainingData) {
            // Calculate real time delta and simulate it
            if (!lastTimestamp.isNull()) {
                qint64 msSinceLastSample = lastTimestamp.msecsTo(point.timestamp);
                if (msSinceLastSample > 2000) {
                    qDebug() << "Time gap:" << msSinceLastSample << "ms at"
                             << point.timestamp.toString("hh:mm:ss.zzz");
                }

                // If this is a resistance change, update the timestamp
                if (point.isResistanceChange) {
                    lastResistanceChange = point.timestamp;
                }

                // Check if enough time has passed since last resistance change
                if (!point.isResistanceChange && !lastResistanceChange.isNull()) {
                    qint64 msSinceResistanceChange = lastResistanceChange.msecsTo(point.timestamp);
                    if (msSinceResistanceChange < 1000) {
                        qDebug() << "Skipping data point due to recent resistance change"
                                 << "Time since change:" << msSinceResistanceChange << "ms";
                        continue;
                    }
                }

                       // Simulate the actual delay
                QThread::msleep(std::min(static_cast<qint64>(10), msSinceLastSample));
            }
            lastTimestamp = point.timestamp;

            // Process the data point
            table.collectData(point.cadence, point.wattage, point.resistance);

            // Update statistics
            if (point.wattage > 0 && !point.isResistanceChange) {
                auto& stats = resistanceStats[point.resistance];
                stats.samples++;
                stats.totalWatts += point.wattage;
                stats.minWatts = std::min(stats.minWatts, point.wattage);
                stats.maxWatts = std::max(stats.maxWatts, point.wattage);
                stats.minCadence = std::min(stats.minCadence, point.cadence);
                stats.maxCadence = std::max(stats.maxCadence, point.cadence);

                if (stats.samples > 10) {
                    double estimated = table.estimateWattage(point.cadence, point.resistance);
                    double error = std::abs(estimated - point.wattage);
                    double errorPercent = (error / point.wattage) * 100.0;
                    if (errorPercent > 20) {
                        qDebug() << "High error at" << point.timestamp.toString("hh:mm:ss.zzz")
                                 << "R:" << point.resistance
                                 << "C:" << point.cadence
                                 << "Est:" << estimated
                                 << "Act:" << point.wattage
                                 << "Error:" << QString::number(errorPercent, 'f', 1) << "%";
                    }
                }
            }
        }

               // Print final statistics
        qDebug() << "\nResistance level statistics:";
        for (const auto& [resistance, stats] : resistanceStats) {
            double avgWatts = stats.totalWatts / stats.samples;
            qDebug() << "Resistance" << resistance << ":";
            qDebug() << "  Samples:" << stats.samples;
            qDebug() << "  Power range:" << stats.minWatts << "-" << stats.maxWatts
                     << "W (avg:" << QString::number(avgWatts, 'f', 1) << "W)";
            qDebug() << "  Cadence range:" << stats.minCadence << "-" << stats.maxCadence << "RPM";
        }

        auto finalData = table.getConsolidatedData();
        qDebug() << "\nFinal consolidated data points:" << finalData.size();

        return true;
    }
};

#endif // ERGTABLE_TEST_H
