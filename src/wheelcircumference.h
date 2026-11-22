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
        QSettings settings;
        // Rouvy sequential virtual gearing has 24 positions per profile. The user-defined
        // physical gear table below typically has only 12, and would show 0/0 from gear 13.
        // Keep this presentation lookup separate from the wheel-diameter/resistance logic.
        if (gear >= 1 && gear <= 24 &&
            settings.value(QZSettings::rouvy_compatibility, QZSettings::default_rouvy_compatibility).toBool()) {
            struct RouvyGear { int chainring; int cog; };
            // Position order matches the Rouvy ratio tables in
            // CharacteristicWriteProcessor0003::handleZwiftGear().
            static constexpr RouvyGear rouvyGears[4][24] = {
            // Mixed Terrain (55T virtual chainring)
            {
                {55, 73}, {55, 63}, {55, 56}, {55, 50},
                {55, 46}, {55, 42}, {55, 39}, {55, 36},
                {55, 33}, {55, 30}, {55, 27}, {55, 25},
                {55, 23}, {55, 21}, {55, 20}, {55, 18},
                {55, 17}, {55, 16}, {55, 15}, {55, 14},
                {55, 13}, {55, 12}, {55, 11}, {55, 10}
            },
            // Flat (39/53T, 10-28T)
            {
                {39, 28}, {39, 26}, {39, 24}, {53, 28},
                {39, 20}, {53, 26}, {39, 18}, {53, 24},
                {39, 16}, {39, 15}, {53, 20}, {39, 14},
                {53, 18}, {39, 13}, {39, 12}, {53, 16},
                {53, 15}, {39, 11}, {53, 14}, {39, 10},
                {53, 13}, {53, 12}, {53, 11}, {53, 10}
            },
            // All-Arounder (35/48T, 10-33T)
            {
                {35, 33}, {35, 28}, {48, 33}, {35, 24},
                {35, 21}, {48, 28}, {35, 19}, {48, 24},
                {35, 17}, {48, 21}, {35, 15}, {35, 14},
                {48, 19}, {35, 13}, {48, 17}, {35, 12},
                {35, 11}, {48, 15}, {48, 14}, {35, 10},
                {48, 13}, {48, 12}, {48, 11}, {48, 10}
            },
            // Climbing (30/43T, 10-36T)
            {
                {30, 36}, {30, 32}, {30, 28}, {43, 36},
                {30, 24}, {43, 32}, {30, 21}, {43, 28},
                {30, 19}, {30, 17}, {43, 24}, {30, 15},
                {43, 21}, {43, 19}, {30, 13}, {30, 12},
                {43, 17}, {30, 11}, {43, 15}, {30, 10},
                {43, 13}, {43, 12}, {43, 11}, {43, 10}
            }
            };
            const int profile = settings.value(QZSettings::rouvy_virtual_gearing_profile,
                                                QZSettings::default_rouvy_virtual_gearing_profile).toInt();
            if (profile >= 0 && profile < 4) {
                const RouvyGear &g = rouvyGears[profile][gear - 1];
                return QString::number(g.chainring) + "/" + QString::number(g.cog);
            }
        }

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

      private:
        std::vector<GearInfo> gears;
    };
};

#endif // WHEELCIRCUMFERENCE_H
