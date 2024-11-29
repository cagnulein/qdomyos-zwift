#ifndef ERGTABLE_H
#define ERGTABLE_H

#include <QList>
#include <QSettings>
#include <QObject>
#include <QDebug>
#include <QDateTime>
#include "qzsettings.h"

struct ergDataPoint {
    uint16_t cadence = 0; // RPM
    uint16_t wattage = 0; // Watts
    uint16_t resistance = 0; // Some unit

    ergDataPoint() = default;

    ergDataPoint(uint16_t c, uint16_t w, uint16_t r) : cadence(c), wattage(w), resistance(r) {}
};

Q_DECLARE_METATYPE(ergDataPoint)

class ergTable : public QObject {
    Q_OBJECT

public:
    ergTable(QObject *parent = nullptr) : QObject(parent) {
        loadSettings();
    }

    ~ergTable() {
        saveSettings();
    }

    void collectData(uint16_t cadence, uint16_t wattage, uint16_t resistance, bool ignoreResistanceTiming = false) {
        if(resistance != lastResistanceValue) {
            qDebug() << "resistance changed";
            lastResistanceTime = QDateTime::currentDateTime();
            lastResistanceValue = resistance;
        }
        if(lastResistanceTime.msecsTo(QDateTime::currentDateTime()) < 1000 && ignoreResistanceTiming == false) {
            qDebug() << "skipping collecting data due to resistance changing too fast";
            return;
        }
        if (wattage > 0 && cadence > 0  && !ergDataPointExists(cadence, wattage, resistance)) {
            qDebug() << "newPointAdded" << "C" << cadence << "W" << wattage << "R" << resistance;
            ergDataPoint point(cadence, wattage, resistance);
            dataTable.append(point);
            saveergDataPoint(point); // Save each new point to QSettings
        } else {
            qDebug() << "discarded" << "C" << cadence << "W" << wattage << "R" << resistance;
        }
    }

    double estimateWattage(uint16_t givenCadence, uint16_t givenResistance) {
        QList<ergDataPoint> filteredByResistance;
        double minResDiff = std::numeric_limits<double>::max();

               // Initial filtering by resistance
        for (const ergDataPoint& point : dataTable) {
            double resDiff = std::abs(point.resistance - givenResistance);
            if (resDiff < minResDiff) {
                filteredByResistance.clear();
                filteredByResistance.append(point);
                minResDiff = resDiff;
            } else if (resDiff == minResDiff) {
                filteredByResistance.append(point);
            }
        }

               // Fallback search if no close resistance match is found
        if (filteredByResistance.isEmpty()) {
            double minSimilarity = std::numeric_limits<double>::max();
            ergDataPoint closestPoint;

            for (const ergDataPoint& point : dataTable) {
                double cadenceDiff = std::abs(point.cadence - givenCadence);
                double resDiff = std::abs(point.resistance - givenResistance);
                // Weighted similarity measure: Giving more weight to resistance
                double similarity = resDiff * 2 + cadenceDiff;

                if (similarity < minSimilarity) {
                    minSimilarity = similarity;
                    closestPoint = point;
                }
            }

            qDebug() << "case1" << closestPoint.wattage;
            // Use the wattage of the closest match based on similarity
            return closestPoint.wattage;
        }

               // Find lower and upper points based on cadence within the filtered list
        double lowerDiff = std::numeric_limits<double>::max();
        double upperDiff = std::numeric_limits<double>::max();
        ergDataPoint lowerPoint, upperPoint;

        for (const ergDataPoint& point : filteredByResistance) {
            double cadenceDiff = std::abs(point.cadence - givenCadence);

            if (point.cadence <= givenCadence && cadenceDiff < lowerDiff) {
                lowerDiff = cadenceDiff;
                lowerPoint = point;
            } else if (point.cadence > givenCadence && cadenceDiff < upperDiff) {
                upperDiff = cadenceDiff;
                upperPoint = point;
            }
        }

        double r;

        // Estimate wattage
        if (lowerDiff != std::numeric_limits<double>::max() && upperDiff != std::numeric_limits<double>::max() && lowerDiff !=0 && upperDiff != 0) {
            // Interpolation between lower and upper points
            double cadenceRatio = 1.0;
            if (upperPoint.cadence != lowerPoint.cadence) { // Avoid division by zero
                cadenceRatio = (givenCadence - lowerPoint.cadence) / (double)(upperPoint.cadence - lowerPoint.cadence);
            }
            r = lowerPoint.wattage + (upperPoint.wattage - lowerPoint.wattage) * cadenceRatio;
            //qDebug() << "case2" << r << lowerPoint.wattage << upperPoint.wattage << lowerPoint.cadence << upperPoint.cadence << cadenceRatio << lowerDiff << upperDiff;
            return r;
        } else if (lowerDiff == 0) {
            //qDebug() << "case3" << lowerPoint.wattage;
            return lowerPoint.wattage;
        } else if (upperDiff == 0) {
            //qDebug() << "case4" << upperPoint.wattage;
            return upperPoint.wattage;            
        } else {
            r = (lowerDiff < upperDiff) ? lowerPoint.wattage : upperPoint.wattage;
            //qDebug() << "case5" << r;
            // Use the closest point if only one match is found
            return r;
        }
    }


private:
    QList<ergDataPoint> dataTable;
    uint16_t lastResistanceValue = 0xFFFF;
    QDateTime lastResistanceTime = QDateTime::currentDateTime();

    bool ergDataPointExists(uint16_t cadence, uint16_t wattage, uint16_t resistance) {
        for (const ergDataPoint& point : dataTable) {
            if (point.cadence == cadence && point.resistance == resistance && cadence != 0 && wattage != 0) {
                return true; // Found duplicate
            }
        }
        return false; // No duplicate
    }

    void loadSettings() {
        QSettings settings;
        QString data = settings.value(QZSettings::ergDataPoints, QZSettings::default_ergDataPoints).toString();
        QStringList dataList = data.split(";");

        for (const QString& triple : dataList) {
            QStringList fields = triple.split("|");
            if (fields.size() == 3) {
                uint16_t cadence = fields[0].toUInt();
                uint16_t wattage = fields[1].toUInt();
                uint16_t resistance = fields[2].toUInt();

                //qDebug() << "inputs.append(ergDataPoint(" << cadence << ", " << wattage << ", "<< resistance << "));";

                dataTable.append(ergDataPoint(cadence, wattage, resistance));
            }
        }
    }

    void saveSettings() {
        QSettings settings;
        QString data;
        for (const ergDataPoint& point : dataTable) {
            QString triple = QString::number(point.cadence) + "|" + QString::number(point.wattage) + "|" + QString::number(point.resistance);
            data += triple + ";";
        }
        settings.setValue(QZSettings::ergDataPoints, data);
    }

    void saveergDataPoint(const ergDataPoint& point) {
        QSettings settings;
        QString data = settings.value(QZSettings::ergDataPoints, QZSettings::default_ergDataPoints).toString();
        data += QString::number(point.cadence) + "|" + QString::number(point.wattage) + "|" + QString::number(point.resistance) + ";";
        settings.setValue(QZSettings::ergDataPoints, data);
    }
};

#endif // ERGTABLE_H
