#ifndef ERGTABLE_H
#define ERGTABLE_H

#include <QList>
#include <QSettings>
#include <QObject>
#include <QDebug>
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

    void collectData(uint16_t cadence, uint16_t wattage, uint16_t resistance) {
        if (wattage > 0 && !ergDataPointExists(cadence, wattage, resistance)) {
            qDebug() << "newPointAdded" << "C" << cadence << "W" << wattage << "R" << resistance;
            ergDataPoint point(cadence, wattage, resistance);
            dataTable.append(point);
            saveergDataPoint(point); // Save each new point to QSettings
        } else {
            qDebug() << "discarded" << "C" << cadence << "W" << wattage << "R" << resistance;
        }
    }

    double estimateWattage(uint16_t givenCadence, uint16_t givenResistance) {
        double lowerDiff = std::numeric_limits<double>::max();
        double upperDiff = std::numeric_limits<double>::max();
        ergDataPoint lowerPoint, upperPoint;

        for (const ergDataPoint& point : dataTable) {
            double diff = std::abs(point.cadence - givenCadence) + std::abs(point.resistance - givenResistance);

            if (point.cadence <= givenCadence && diff < lowerDiff) {
                lowerDiff = diff;
                lowerPoint = point;
            } else if (point.cadence > givenCadence && diff < upperDiff) {
                upperDiff = diff;
                upperPoint = point;
            }
        }

        qDebug() << lowerDiff << upperDiff << lowerPoint.cadence << lowerPoint.resistance << lowerPoint.wattage << upperPoint.cadence << upperPoint.resistance << upperPoint.wattage;

        double r;

        // If no exact match found, interpolate between the closest lower and upper points
        if (lowerDiff != std::numeric_limits<double>::max() && upperDiff != std::numeric_limits<double>::max()) {
            double cadenceRange = upperPoint.cadence - lowerPoint.cadence;
            double resistanceRange = upperPoint.resistance - lowerPoint.resistance;
            double cadenceRatio = (givenCadence - lowerPoint.cadence) / cadenceRange;
            double resistanceRatio = (givenResistance - lowerPoint.resistance) / resistanceRange;

            if(qIsNaN(cadenceRatio) || qIsInf(cadenceRatio)) {
                cadenceRatio = 1.0;
            }
            if(qIsNaN(resistanceRatio) || qIsInf(resistanceRatio)) {
                resistanceRatio = 1.0;
            }

            r = (double)lowerPoint.wattage + ((double)(upperPoint.wattage - lowerPoint.wattage)) * (cadenceRatio + resistanceRatio) / 2.0;

            qDebug() << "interpolation" << r;

            return r;
        }

        r = (lowerDiff < upperDiff ? lowerPoint.wattage : upperPoint.wattage);

        qDebug() << "closest point" << r;

        // Fallback if only one closest point is found or none
        return r;
    }

private:
    QList<ergDataPoint> dataTable;

    bool ergDataPointExists(uint16_t cadence, uint16_t wattage, uint16_t resistance) {
        for (const ergDataPoint& point : dataTable) {
            if (point.cadence == cadence && point.wattage == wattage && point.resistance == resistance) {
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
