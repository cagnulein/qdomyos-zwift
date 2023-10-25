#ifndef CUSTOM_ERG_TABLE_H
#define CUSTOM_ERG_TABLE_H

#include <QObject>
#include "bluetooth.h"
#include "metric.h"

/* Number of entries in the ERG Power Lookup Table
 This is currently maintained as to keep memory usage lower and reduce the print output of the table.
 It can be depreciated in the future should we decide to remove logging of the power table. Then it should be calculated in ERG_Mode.cpp
 by dividing userConfig.getMaxWatts() by POWERTABLE_INCREMENT.  */
#define POWERTABLE_SIZE 20

// Size of increments (in watts) for the ERG Lookup Table. Needs to be one decimal place for proper calculations i.e. 50.0
#define POWERTABLE_INCREMENT 50.0

// Number of similar power samples to take before writing to the Power Table
#define POWER_SAMPLES 5

// Normal cadence value (used in power table and other areas)
#define NORMAL_CAD           90
#define ERG_MODE_DELAY       700
#define RETURN_ERROR         -99

class PowerEntry {
  public:
    int watts;
    int32_t targetResistance;
    int cad;
    int readings;

    PowerEntry() {
        this->watts                = 0;
        this->targetResistance     = 0;
        this->cad                  = 0;
        this->readings             = 0;
    }
};

class PowerBuffer {
  public:
    PowerEntry powerEntry[POWER_SAMPLES];
    void set(int i, int watts, int cad, int resistance);
    void reset();
};

class PowerTable {
  public:
    PowerEntry powerEntry[POWERTABLE_SIZE];

           // Pick up new power value and put them into the power table
    void processPowerValue(PowerBuffer& powerBuffer, int cadence, metric watts);

           // Sets stepper min/max value from power table
    void setStepperMinMax();

           // Catalogs a new entry into the power table.
    void newEntry(PowerBuffer& powerBuffer);

           // returns incline for wattTarget. Null if not found.
    int32_t lookup(int watts, int cad);

           // load power table from littlefs
    bool load();

           // save powertable from littlefs
    bool save();

           // Display power table in log
    void toLog();

    bluetoothdevice* bt = nullptr;

  private:
    // Adjust Watts For Cadence
    int _adjustWattsForCadence(int watts, float cad);
};

class custom_erg_table
{
  public:
    custom_erg_table(bluetoothdevice* bt);

  private:
    bluetoothdevice* bt = nullptr;
    PowerTable powerTable;

 };

#endif // CUSTOM_ERG_TABLE_H
