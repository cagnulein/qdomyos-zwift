#ifndef WHEELCIRCUMFERENCE_H
#define WHEELCIRCUMFERENCE_H

#include <QtCore/qbytearray.h>

#ifndef Q_OS_ANDROID
#include <QtCore/qcoreapplication.h>
#else
#include <QtGui/qguiapplication.h>
#endif
#include <QtCore/qlist.h>
#include <QtCore/qmutex.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qtimer.h>

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QSettings>
#include <QDebug>

#include "qzsettings.h"

class wheelCircumference : public QObject {

    Q_OBJECT

  public:
    static double gearsToWheelDiameter(double gear) {
        QSettings settings;
        GearTable table;
        if(gear < 1) gear = 1;
        else if(gear > table.maxGears) gear = table.maxGears;
        double original_ratio = ((double)settings.value(QZSettings::gear_crankset_size, QZSettings::default_gear_crankset_size).toDouble()) / ((double)settings.value(QZSettings::gear_cog_size, QZSettings::default_gear_cog_size).toDouble());
        GearTable::GearInfo g = table.getGear((int)gear);
        double current_ratio =  ((double)g.crankset / (double)g.rearCog);
        return (((double)settings.value(QZSettings::gear_circumference, QZSettings::default_gear_circumference).toDouble()) / original_ratio) * ((double)current_ratio);
    }

    static QString gearsInfo(int gear) {
        GearTable table;
        GearTable::GearInfo g = table.getGear((int)gear);
        return QString::number(g.crankset, 'f' , 0) + "/" + QString::number(g.rearCog, 'f' , 0);
    }

    class GearTable {
      public:

        int maxGears = 12;

        struct GearInfo {
            int gear;
            int crankset;
            int rearCog;
        };

        void loadGearSettings() {
            QSettings settings;

            QString gearConfig = settings.value("gear_configuration").toString();
            if (gearConfig.isEmpty()) {

                gearConfig = "1|38|44|true\n2|38|38|true\n3|38|32|true\n4|38|28|true\n"
                             "5|38|24|true\n6|38|21|true\n7|38|19|true\n8|38|17|true\n"
                             "9|38|15|true\n10|38|13|true\n11|38|11|true\n12|38|10|true";
            }

            gears.clear();
            maxGears = 0;

                   // Parsa la configurazione
            QStringList rows = gearConfig.split('\n');
            for (const QString& row : rows) {
                QStringList parts = row.split('|');
                if (parts.size() >= 4 && (parts[3] == "true")) {
                    GearInfo config;
                    config.gear = parts[0].toInt();
                    config.crankset = parts[1].toInt();
                    config.rearCog = parts[2].toInt();

                    gears.push_back(config);
                    maxGears = qMax(maxGears, config.gear);
                }
            }
        }

        void addGear(int gear, int crankset, int rearCog) {
            gears.push_back({gear, crankset, rearCog});
        }

        void removeGear(int gear) {
            gears.erase(std::remove_if(gears.begin(), gears.end(),
                                       [gear](const GearInfo& info) { return info.gear == gear; }),
                        gears.end());
        }

        void printTable() const {
            qDebug() << "| Gear | Crankset | Rear Cog |\n";
            qDebug() << "|------|----------|----------|\n";
            for (const auto& gear : gears) {
                qDebug()  << "| " << gear.gear << " | " << gear.crankset
                         << " | " << gear.rearCog << " |\n";
            }
        }

        GearInfo getGear(int gearNumber) const {
            auto it = std::find_if(gears.begin(), gears.end(),
                                   [gearNumber](const GearInfo& info) { return info.gear == gearNumber; });

            if (it != gears.end()) {
                return *it;
            }
            return GearInfo();
        }

        GearTable() {
            loadGearSettings();
        }

        int chainRingUp(int currentGear) {
            GearTable table;
            GearTable::GearInfo currentGearInfo = table.getGear(currentGear);

            // If current gear info is invalid, return current gear
            if (currentGearInfo.gear == 0) return currentGear;

            int highestMatchingGear = currentGear;
            int highestCrankset = currentGearInfo.crankset;

            // Scan through all gears to find one with a higher crankset
            for (int i = 1; i <= table.maxGears; i++) {
                GearTable::GearInfo gear = table.getGear(i);
                if (gear.gear != 0 && gear.crankset > highestCrankset) {
                    highestMatchingGear = gear.gear;
                    highestCrankset = gear.crankset;
                    break;  // Take the first higher crankset we find
                }
            }

            return highestMatchingGear;
        }

        int chainRingDown(int currentGear) {
            GearTable table;
            GearTable::GearInfo currentGearInfo = table.getGear(currentGear);

            // If current gear info is invalid, return current gear
            if (currentGearInfo.gear == 0) return currentGear;

            int lowestMatchingGear = currentGear;
            int lowestCrankset = currentGearInfo.crankset;

            // Scan through all gears to find one with a lower crankset
            for (int i = table.maxGears; i >= 1; i--) {
                GearTable::GearInfo gear = table.getGear(i);
                if (gear.gear != 0 && gear.crankset < lowestCrankset) {
                    lowestMatchingGear = gear.gear;
                    lowestCrankset = gear.crankset;
                    break;  // Take the first lower crankset we find
                }
            }

            return lowestMatchingGear;
        }

        int cassetteUp(int currentGear) {
            GearTable table;
            GearTable::GearInfo currentGearInfo = table.getGear(currentGear);

            // If current gear info is invalid, return current gear
            if (currentGearInfo.gear == 0) return currentGear;

            int nextGear = currentGear;
            int currentRearCog = currentGearInfo.rearCog;

            // Find the next gear with same crankset but higher rear cog
            for (int i = 1; i <= table.maxGears; i++) {
                GearTable::GearInfo gear = table.getGear(i);
                if (gear.gear != 0 &&
                    gear.crankset == currentGearInfo.crankset &&
                    gear.rearCog > currentRearCog) {
                    nextGear = gear.gear;
                    break;  // Take the first higher rear cog we find
                }
            }

            return nextGear;
        }

        int cassetteDown(int currentGear) {
            GearTable table;
            GearTable::GearInfo currentGearInfo = table.getGear(currentGear);

            // If current gear info is invalid, return current gear
            if (currentGearInfo.gear == 0) return currentGear;

            int nextGear = currentGear;
            int currentRearCog = currentGearInfo.rearCog;

            // Find the next gear with same crankset but lower rear cog
            for (int i = table.maxGears; i >= 1; i--) {
                GearTable::GearInfo gear = table.getGear(i);
                if (gear.gear != 0 &&
                    gear.crankset == currentGearInfo.crankset &&
                    gear.rearCog < currentRearCog) {
                    nextGear = gear.gear;
                    break;  // Take the first lower rear cog we find
                }
            }

            return nextGear;
        }

      private:
        std::vector<GearInfo> gears;
    };
};

#endif // WHEELCIRCUMFERENCE_H
