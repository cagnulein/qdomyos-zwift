#ifndef ERGTABLE_H
#define ERGTABLE_H

#include <QList>
#include <QSettings>
#include <QObject>
#include <cmath>

#include <QMetaType>
#include <QDebug>

class DataPoint {
public:
    double cadence = 0; // RPM, default initialized
    double wattage = 0; // Watts, default initialized
    double resistance = 0; // Some unit, default initialized

    DataPoint() = default; // Default constructor

    DataPoint(double c, double w, double r) : cadence(c), wattage(w), resistance(r) {} // Custom constructor
};

Q_DECLARE_METATYPE(DataPoint)

class ergTable : public QObject {
    Q_OBJECT

public:
    ergTable(QObject *parent = nullptr) : QObject(parent) {
        loadSettings();
    }

    ~ergTable() {
        saveSettings();
    }

    void collectData(double cadence, double wattage, double resistance) {
        if (wattage > 0 && !dataPointExists(cadence, wattage, resistance)) {
            qDebug() << "newPointAdded" << "C" << cadence << "W" << wattage << "R" << resistance;
            DataPoint point(cadence, wattage, resistance);
            dataTable.append(point);
            saveDataPoint(point); // Save each new point to QSettings
        } else {
            qDebug() << "discarded" << "C" << cadence << "W" << wattage << "R" << resistance;
        }
    }

    double estimateWattage(double givenCadence, double givenResistance) {
        // Placeholder for closest points. Initialize with max possible difference.
        double lowerDiff = std::numeric_limits<double>::max();
        double upperDiff = std::numeric_limits<double>::max();
        DataPoint lowerPoint, upperPoint;

        for (const DataPoint& point : dataTable) {
            double diff = std::abs(point.cadence - givenCadence) + std::abs(point.resistance - givenResistance);

            // Update lower or upper point based on being lower or higher than the given values
            if (point.cadence <= givenCadence && diff < lowerDiff) {
                lowerDiff = diff;
                lowerPoint = point;
            } else if (point.cadence > givenCadence && diff < upperDiff) {
                upperDiff = diff;
                upperPoint = point;
            }
        }

        qDebug() << lowerDiff << upperDiff << lowerPoint.cadence << lowerPoint.resistance << lowerPoint.wattage << upperPoint.cadence << upperPoint.resistance << upperPoint.wattage;

        // If no exact match found, interpolate between the closest lower and upper points
        if (lowerDiff != std::numeric_limits<double>::max() && upperDiff != std::numeric_limits<double>::max()) {
            double cadenceRange = upperPoint.cadence - lowerPoint.cadence;
            double resistanceRange = upperPoint.resistance - lowerPoint.resistance;
            double cadenceRatio = (givenCadence - lowerPoint.cadence) / cadenceRange;
            double resistanceRatio = (givenResistance - lowerPoint.resistance) / resistanceRange;

            qDebug() << "interpolation" << lowerPoint.wattage + (upperPoint.wattage - lowerPoint.wattage) * (cadenceRatio + resistanceRatio) / 2;

            if(qIsNaN(cadenceRatio) || qIsInf(cadenceRatio)) {
                cadenceRatio = 1.0;
            }
            if(qIsNaN(resistanceRatio) || qIsInf(resistanceRatio)) {
                resistanceRatio = 1.0;
            }

            return lowerPoint.wattage + (upperPoint.wattage - lowerPoint.wattage) * (cadenceRatio + resistanceRatio) / 2;
        }

        qDebug() << "closest point" << (lowerDiff < upperDiff ? lowerPoint.wattage : upperPoint.wattage);

        // Fallback if only one closest point is found or none
        return lowerDiff < upperDiff ? lowerPoint.wattage : upperPoint.wattage;
    }


private:
    QList<DataPoint> dataTable;

    bool dataPointExists(double cadence, double wattage, double resistance) {
        for (const DataPoint& point : dataTable) {
            if (point.cadence == cadence && point.wattage == wattage && point.resistance == resistance) {
                return true; // Found duplicate
            }
        }
        return false; // No duplicate
    }

    void loadSettings() {
        QSettings settings;
        int size = settings.beginReadArray("DataPoints");
        for (int i = 0; i < size; ++i) {
            settings.setArrayIndex(i);
            double cadence = settings.value("cadence").toDouble();
            double wattage = settings.value("wattage").toDouble();
            double resistance = settings.value("resistance").toDouble();
            dataTable.append(DataPoint(cadence, wattage, resistance));
        }
        settings.endArray();
    }

    void saveSettings() {
        QSettings settings;
        settings.beginWriteArray("DataPoints");
        for (int i = 0; i < dataTable.size(); ++i) {
            settings.setArrayIndex(i);
            settings.setValue("cadence", dataTable[i].cadence);
            settings.setValue("wattage", dataTable[i].wattage);
            settings.setValue("resistance", dataTable[i].resistance);
        }
        settings.endArray();
    }

    void saveDataPoint(const DataPoint& point) {
        QSettings settings;
        int size = settings.beginReadArray("DataPoints");
        settings.endArray();

        settings.beginWriteArray("DataPoints", size + 1);
        settings.setArrayIndex(size);
        settings.setValue("cadence", point.cadence);
        settings.setValue("wattage", point.wattage);
        settings.setValue("resistance", point.resistance);
        settings.endArray();
    }
};


#endif // ERGTABLE_H
