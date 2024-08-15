#ifndef TREADMILLERGTABLE_H
#define TREADMILLERGTABLE_H

#include <QList>
#include <QSettings>
#include <QObject>
#include <QDebug>
#include <QDateTime>
#include "qzsettings.h"

struct treadmillDataPoint {
    float speed = 0; // Speed in km/h
    uint16_t wattage = 0; // Watts
    float inclination = 0; // Inclination in km/h

    treadmillDataPoint() = default;

    treadmillDataPoint(float s, uint16_t w, float i) : speed(s), wattage(w), inclination(i) {}
};

Q_DECLARE_METATYPE(treadmillDataPoint)

class treadmillErgTable : public QObject {
    Q_OBJECT

  public:
    treadmillErgTable(QObject *parent = nullptr) : QObject(parent) {
        loadSettings();
    }

    ~treadmillErgTable() {
        saveSettings();
    }

    void collectTreadmillData(float speed, uint16_t wattage, float inclination, bool ignoreInclinationTiming = false) {
        if(inclination != lastInclinationValue || speed != lastSpeedValue) {
            qDebug() << "inclination or speed changed";
            lastChangedTime = QDateTime::currentDateTime();
            lastInclinationValue = inclination;
            lastSpeedValue = speed;
        }
        if(lastChangedTime.msecsTo(QDateTime::currentDateTime()) < 5000 && ignoreInclinationTiming == false) {
            qDebug() << "skipping collecting data due to inclination changing too fast";
            return;
        }
        if (wattage > 0 && speed > 0 && !treadmillDataPointExists(speed, wattage, inclination)) {
            qDebug() << "newPointAdded" << "S" << speed << "W" << wattage << "I" << inclination;
            treadmillDataPoint point(speed, wattage, inclination);
            dataTable.append(point);
            saveTreadmillDataPoint(point); // Save each new point to QSettings
        } else {
            qDebug() << "discarded" << "S" << speed << "W" << wattage << "I" << inclination;
        }
    }

    double estimateWattage(float givenSpeed, float givenInclination) {
        QList<treadmillDataPoint> filteredByInclination;
        double minInclinationDiff = std::numeric_limits<double>::max();

               // Initial filtering by inclination
        for (const treadmillDataPoint& point : dataTable) {
            double inclinationDiff = std::abs(point.inclination - givenInclination);
            if (inclinationDiff < minInclinationDiff) {
                filteredByInclination.clear();
                filteredByInclination.append(point);
                minInclinationDiff = inclinationDiff;
            } else if (inclinationDiff == minInclinationDiff) {
                filteredByInclination.append(point);
            }
        }

               // Fallback search if no close inclination match is found
        if (filteredByInclination.isEmpty()) {
            double minSimilarity = std::numeric_limits<double>::max();
            treadmillDataPoint closestPoint;

            for (const treadmillDataPoint& point : dataTable) {
                double speedDiff = std::abs(point.speed - givenSpeed);
                double inclinationDiff = std::abs(point.inclination - givenInclination);
                // Weighted similarity measure: Giving more weight to inclination
                double similarity = inclinationDiff * 2 + speedDiff;

                if (similarity < minSimilarity) {
                    minSimilarity = similarity;
                    closestPoint = point;
                }
            }

            qDebug() << "case1" << closestPoint.wattage;
            // Use the wattage of the closest match based on similarity
            return closestPoint.wattage;
        }

               // Find lower and upper points based on speed within the filtered list
        double lowerDiff = std::numeric_limits<double>::max();
        double upperDiff = std::numeric_limits<double>::max();
        treadmillDataPoint lowerPoint, upperPoint;

        for (const treadmillDataPoint& point : filteredByInclination) {
            double speedDiff = std::abs(point.speed - givenSpeed);

            if (point.speed <= givenSpeed && speedDiff < lowerDiff) {
                lowerDiff = speedDiff;
                lowerPoint = point;
            } else if (point.speed > givenSpeed && speedDiff < upperDiff) {
                upperDiff = speedDiff;
                upperPoint = point;
            }
        }

        double r;

               // Estimate wattage
        if (lowerDiff != std::numeric_limits<double>::max() && upperDiff != std::numeric_limits<double>::max() && lowerDiff != 0 && upperDiff != 0) {
            // Interpolation between lower and upper points
            double speedRatio = 1.0;
            if (upperPoint.speed != lowerPoint.speed) { // Avoid division by zero
                speedRatio = (givenSpeed - lowerPoint.speed) / (double)(upperPoint.speed - lowerPoint.speed);
            }
            r = lowerPoint.wattage + (upperPoint.wattage - lowerPoint.wattage) * speedRatio;
            return r;
        } else if (lowerDiff == 0) {
            return lowerPoint.wattage;
        } else if (upperDiff == 0) {
            return upperPoint.wattage;
        } else {
            r = (lowerDiff < upperDiff) ? lowerPoint.wattage : upperPoint.wattage;
            // Use the closest point if only one match is found
            return r;
        }
    }

  private:
    QList<treadmillDataPoint> dataTable;
    float lastInclinationValue = -9999;
    float lastSpeedValue = -9999;
    QDateTime lastChangedTime = QDateTime::currentDateTime();

    bool treadmillDataPointExists(float speed, uint16_t wattage, float inclination) {
        for (const treadmillDataPoint& point : dataTable) {
            if (point.speed == speed && point.inclination == inclination && speed != 0 && wattage != 0) {
                return true; // Found duplicate
            }
        }
        return false; // No duplicate
    }

    void loadSettings() {
        QSettings settings;
        QString data = settings.value(QZSettings::treadmillDataPoints, QZSettings::default_treadmillDataPoints).toString();
        QStringList dataList = data.split(";");

        for (const QString& triple : dataList) {
            QStringList fields = triple.split("|");
            if (fields.size() == 3) {
                float speed = fields[0].toUInt();
                uint16_t wattage = fields[1].toUInt();
                float inclination = fields[2].toUInt();

                qDebug() << "inputs.append(treadmillDataPoint(" << speed << ", " << wattage << ", " << inclination << "));";

                dataTable.append(treadmillDataPoint(speed, wattage, inclination));
            }
        }
    }

    void saveSettings() {
        QSettings settings;
        QString data;
        for (const treadmillDataPoint& point : dataTable) {
            QString triple = QString::number(point.speed) + "|" + QString::number(point.wattage) + "|" + QString::number(point.inclination);
            data += triple + ";";
        }
        settings.setValue(QZSettings::treadmillDataPoints, data);
    }

    void saveTreadmillDataPoint(const treadmillDataPoint& point) {
        QSettings settings;
        QString data = settings.value(QZSettings::treadmillDataPoints, QZSettings::default_treadmillDataPoints).toString();
        data += QString::number(point.speed) + "|" + QString::number(point.wattage) + "|" + QString::number(point.inclination) + ";";
        settings.setValue(QZSettings::treadmillDataPoints, data);
    }
};

#endif // TREADMILLERGTABLE_H
