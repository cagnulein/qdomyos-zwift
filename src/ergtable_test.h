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
        return tester.testTrainingSession();
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
