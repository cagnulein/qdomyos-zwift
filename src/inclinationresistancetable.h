#ifndef INCLINATIONRESISTANCETABLE_H
#define INCLINATIONRESISTANCETABLE_H

#include <QList>
#include <QSettings>
#include <QObject>
#include <QDebug>
#include <QDateTime>
#include <QMap>
#include <algorithm>
#include "qzsettings.h"

// Structure to store an inclination/resistance data point
struct inclinationResistanceDataPoint {
    double inclination = 0.0;  // Inclination as double to support decimal values
    uint16_t resistance = 0;   // Resistance level

    inclinationResistanceDataPoint() = default;
    inclinationResistanceDataPoint(double i, uint16_t r) : inclination(i), resistance(r) {}
};

Q_DECLARE_METATYPE(inclinationResistanceDataPoint)

// Class to manage the inclination/resistance data table
class inclinationResistanceTable : public QObject {
    Q_OBJECT

public:
    inclinationResistanceTable(QObject *parent = nullptr) : QObject(parent) {
        loadSettings();
    }

    ~inclinationResistanceTable() {
        saveSettings();
    }

    // Reset all collected data
    void reset() {
        dataPoints.clear();
        lastInclinationValue = -999.0;
        lastResistanceValue = 0xFFFF;
        lastStableTime = QDateTime::currentDateTime();
        inclinationStable = false;
        resistanceStable = false;

        // Remove data from settings
        QSettings settings;
        settings.remove(QZSettings::inclinationResistancePoints);
        settings.sync();
    }

    // Add a new data point if values are stable for 5 seconds
    void collectData(double inclination, uint16_t resistance, double watt) {
        // Skip invalid data points (zero inclination or resistance)
        if (inclination == 0 || resistance == 0 || watt == 0) {
            qDebug() << "Skipping invalid data point:" << "Incl:" << inclination 
                     << "Res:" << resistance << "Watt:" << watt;
            return;
        }

        QDateTime currentTime = QDateTime::currentDateTime();
        
        // Check if values have changed
        bool inclinationChanged = qAbs(inclination - lastInclinationValue) > 0.3;
        bool resistanceChanged = resistance != lastResistanceValue;
        
        if (inclinationChanged) {
            inclinationStable = false;
            lastInclinationValue = inclination;
            lastStableTime = currentTime;
            qDebug() << QStringLiteral("Inclination not stable, resetting...");
        }
        
        if (resistanceChanged) {
            resistanceStable = false;
            lastResistanceValue = resistance;
            lastStableTime = currentTime;
            qDebug() << QStringLiteral("Resistance not stable, resetting...");
        }
        
        // If one of the values is not yet stable, check if 5 seconds have passed
        if (!inclinationStable || !resistanceStable) {
            // Check if 5 seconds have passed since the last change
            if (lastStableTime.msecsTo(currentTime) >= 5000) {
                inclinationStable = true;
                resistanceStable = true;
                
                // Check if a point with similar inclination already exists
                bool foundInclination = false;
                for (int i = 0; i < dataPoints.size(); i++) {
                    if (qAbs(dataPoints[i].inclination - inclination) < 0.1) {
                        // Update existing point
                        dataPoints[i].resistance = resistance;
                        foundInclination = true;
                        qDebug() << "Updated existing inclination point:" << "Incl:" << inclination 
                                 << "Res:" << resistance;
                        break;
                    }
                }
                
                // Check if a point with the same resistance already exists
                bool foundResistance = false;
                if (!foundInclination) {
                    for (int i = 0; i < dataPoints.size(); i++) {
                        if (dataPoints[i].resistance == resistance) {
                            // Update existing point with new inclination
                            qDebug() << "Found duplicate resistance:" << resistance
                                     << "Old Incl:" << dataPoints[i].inclination
                                     << "New Incl:" << inclination;
                            
                            // Only update if the new inclination has larger absolute value
                            if (qAbs(inclination) > qAbs(dataPoints[i].inclination)) {
                                dataPoints[i].inclination = inclination;
                                qDebug() << "Updated existing resistance point with larger inclination";
                            } else {
                                qDebug() << "Kept existing point with larger inclination value";
                            }
                            foundResistance = true;
                            break;
                        }
                    }
                }
                
                // If it doesn't exist, add a new point
                if (!foundInclination && !foundResistance) {
                    dataPoints.append(inclinationResistanceDataPoint(inclination, resistance));
                    qDebug() << "Added new point:" << "Incl:" << inclination 
                             << "Res:" << resistance;
                }
                
                // Save settings
                saveSettings();
            }
        }
    }

    // Estimate inclination based on given resistance
    double estimateInclination(uint16_t givenResistance) {
        if (dataPoints.isEmpty()) {
            qDebug() << "estimateInclination: No data points available, returning 0.0";
            return 0.0;
        }

        // Skip invalid resistance
        if (givenResistance == 0) {
            qDebug() << "estimateInclination: Invalid resistance 0, returning 0.0";
            return 0.0;
        }

        qDebug() << "estimateInclination: Searching for resistance level" << givenResistance 
                 << "in" << dataPoints.size() << "data points";

        // First check for exact match
        for (const auto& point : dataPoints) {
            if (point.resistance == givenResistance) {
                qDebug() << "estimateInclination: Found exact match - Resistance:" << point.resistance 
                         << "Inclination:" << point.inclination;
                return point.inclination;
            }
        }

        // If no exact match, find the closest point or interpolate if possible
        double minResistanceDiff = 9999.0;
        uint16_t lowerResistance = 0, upperResistance = 65535;
        double lowerInclination = 0.0, upperInclination = 0.0;
        double closestInclination = 0.0;

        for (const auto& point : dataPoints) {
            uint16_t resistanceDiff = qAbs(static_cast<int>(point.resistance) - static_cast<int>(givenResistance));
            
            // Find the closest point
            if (resistanceDiff < minResistanceDiff) {
                minResistanceDiff = resistanceDiff;
                closestInclination = point.inclination;
                qDebug() << "estimateInclination: Found closer point - Resistance:" << point.resistance 
                         << "Inclination:" << point.inclination << "Diff:" << resistanceDiff;
            }
            
            // Find points for interpolation
            if (point.resistance < givenResistance && point.resistance > lowerResistance) {
                lowerResistance = point.resistance;
                lowerInclination = point.inclination;
                qDebug() << "estimateInclination: Updated lower bound - Resistance:" << lowerResistance 
                         << "Inclination:" << lowerInclination;
            }
            if (point.resistance > givenResistance && point.resistance < upperResistance) {
                upperResistance = point.resistance;
                upperInclination = point.inclination;
                qDebug() << "estimateInclination: Updated upper bound - Resistance:" << upperResistance 
                         << "Inclination:" << upperInclination;
            }
        }

        // Interpolate if possible
        if (lowerResistance > 0) {
            if (upperResistance < 65535) {
                // Standard interpolation between two points
                double ratio = static_cast<double>(givenResistance - lowerResistance) /
                               static_cast<double>(upperResistance - lowerResistance);
                double result = lowerInclination + ratio * (upperInclination - lowerInclination);

                qDebug() << "estimateInclination: Interpolating between points -"
                         << "Lower Resistance:" << lowerResistance << "Lower Inclination:" << lowerInclination
                         << "Upper Resistance:" << upperResistance << "Upper Inclination:" << upperInclination
                         << "Ratio:" << ratio << "Result:" << result;

                return result;
            } else {
                // Extrapolation based on the last known point
                // We use a linear rate based on the inclination of the last point
                double estimatedIncreaseRate;
                if (dataPoints.size() >= 2) {
                    // Calculate an increase rate based on the two points with highest resistance
                    QList<inclinationResistanceDataPoint> sortedPoints = dataPoints;
                    std::sort(sortedPoints.begin(), sortedPoints.end(),
                              [](const inclinationResistanceDataPoint& a, const inclinationResistanceDataPoint& b) {
                                  return a.resistance > b.resistance;
                              });

                    if (sortedPoints[0].resistance != sortedPoints[1].resistance) {
                        estimatedIncreaseRate = (sortedPoints[0].inclination - sortedPoints[1].inclination) /
                                                (sortedPoints[0].resistance - sortedPoints[1].resistance);
                    } else {
                        estimatedIncreaseRate = 0.1; // default value if we can't calculate it
                    }
                } else {
                    estimatedIncreaseRate = 0.1; // default value
                }

                // Linear extrapolation
                double result = lowerInclination + (givenResistance - lowerResistance) * estimatedIncreaseRate;

                qDebug() << "estimateInclination: No upper bound found, using extrapolation -"
                         << "Lower Resistance:" << lowerResistance << "Lower Inclination:" << lowerInclination
                         << "Increase Rate:" << estimatedIncreaseRate << "Result:" << result;

                return result;
            }
        }
        
        // Otherwise use the closest value
        qDebug() << "estimateInclination: Using closest point, no interpolation possible."
                 << "Returning inclination:" << closestInclination;
        
        return closestInclination;
    }

    // Estimate resistance based on given inclination
    uint16_t estimateResistance(double givenInclination) {
        if (dataPoints.isEmpty()) return 0;

        // Sort points by inclination for easier processing
        QList<inclinationResistanceDataPoint> sortedPoints = dataPoints;
        std::sort(sortedPoints.begin(), sortedPoints.end(),
                  [](const inclinationResistanceDataPoint& a, const inclinationResistanceDataPoint& b) {
                      return a.inclination < b.inclination;
                  });

        // If inclination is very low, return the lowest resistance
        if (qAbs(givenInclination) < 0.1) {
            qDebug() << "estimateResistance: Inclination near zero, returning lowest resistance";
            if (!sortedPoints.isEmpty()) {
                return sortedPoints.first().resistance;
            }
            return 0;
        }

        // If the given inclination is smaller than the smallest one in the table,
        // return the resistance of the smallest inclination point
        if (givenInclination < sortedPoints.first().inclination) {
            qDebug() << "estimateResistance: Inclination is lower than the minimum, returning lowest resistance";
            return sortedPoints.first().resistance;
        }

        // If the given inclination is larger than the largest one in the table,
        // return the resistance of the largest inclination point
        if (givenInclination > sortedPoints.last().inclination) {
            qDebug() << "estimateResistance: Inclination is higher than the maximum, returning highest resistance";
            return sortedPoints.last().resistance;
        }

        // Find points for interpolation
        double lowerInclination = -9999.0, upperInclination = 9999.0;
        uint16_t lowerResistance = 0, upperResistance = 0;

        for (const auto& point : sortedPoints) {
            if (point.inclination <= givenInclination && point.inclination > lowerInclination) {
                lowerInclination = point.inclination;
                lowerResistance = point.resistance;
            }
            if (point.inclination >= givenInclination && point.inclination < upperInclination) {
                upperInclination = point.inclination;
                upperResistance = point.resistance;
            }
        }

        // Interpolate if possible
        if (lowerInclination > -9999.0 && upperInclination < 9999.0 &&
            lowerInclination != upperInclination) {
            double ratio = (givenInclination - lowerInclination) /
                           (upperInclination - lowerInclination);
            return lowerResistance + ratio * (upperResistance - lowerResistance);
        }

        // If interpolation is not possible, find the closest point
        double minInclinationDiff = 9999.0;
        uint16_t closestResistance = 0;
        for (const auto& point : sortedPoints) {
            double inclinationDiff = qAbs(point.inclination - givenInclination);
            if (inclinationDiff < minInclinationDiff) {
                minInclinationDiff = inclinationDiff;
                closestResistance = point.resistance;
            }
        }
        return closestResistance;
    }

    // Get all data points
    QList<inclinationResistanceDataPoint> getDataPoints() const {
        return dataPoints;
    }

private:
    QList<inclinationResistanceDataPoint> dataPoints;
    double lastInclinationValue = -999.0;
    uint16_t lastResistanceValue = 0xFFFF;
    QDateTime lastStableTime = QDateTime::currentDateTime();
    bool inclinationStable = false;
    bool resistanceStable = false;

    // Load data from settings
    void loadSettings() {
        QSettings settings;
        QString data = settings.value(QZSettings::inclinationResistancePoints, 
                                      QString()).toString();
        QStringList dataList = data.split(";", Qt::SkipEmptyParts);

        for (const QString& pair : dataList) {
            QStringList fields = pair.split("|");
            if (fields.size() == 2) {
                double inclination = fields[0].toDouble();
                uint16_t resistance = fields[1].toUInt();
                dataPoints.append(inclinationResistanceDataPoint(inclination, resistance));
            }
        }
    }

    // Save data to settings
    void saveSettings() {
        QSettings settings;
        QStringList dataStrings;

        for (const inclinationResistanceDataPoint& point : dataPoints) {
            dataStrings.append(QString("%1|%2").arg(point.inclination, 0, 'f', 2)
                                  .arg(point.resistance));
        }

        settings.setValue(QZSettings::inclinationResistancePoints, dataStrings.join(";"));
    }
};

#endif // INCLINATIONRESISTANCETABLE_H
