#include "custom_erg_table.h"
#include <QDebug>
#include <QtMath>

/*
 * Copyright (C) 2020  Anthony Doud & Joel Baranick
 * All rights reserved
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

custom_erg_table::custom_erg_table(bluetoothdevice* bt) {
    this->bt = bt;
    this->powerTable.bt = bt;
}

void PowerBuffer::set(int i, int watts, int cad, int resistance) {
    this->powerEntry[i].readings       = 1;
    this->powerEntry[i].watts          = watts;
    this->powerEntry[i].cad            = cad;
    this->powerEntry[i].targetResistance = resistance;
}

void PowerBuffer::reset() {
    for (int i = 0; i < POWER_SAMPLES; i++) {
        this->powerEntry[i].readings       = 0;
        this->powerEntry[i].watts          = 0;
        this->powerEntry[i].cad            = 0;
        this->powerEntry[i].targetResistance = 0;
    }
}

void PowerTable::processPowerValue(PowerBuffer& powerBuffer, int cadence, metric watts) {
    if ((cadence >= (NORMAL_CAD - 20)) && (cadence <= (NORMAL_CAD + 20)) && (watts.value() > 10) && (watts.value() < (POWERTABLE_SIZE * POWERTABLE_INCREMENT))) {
        if (powerBuffer.powerEntry[0].readings == 0) {
            // Take Initial reading
            powerBuffer.set(0, 0, 0, 0);
            // Check that reading is within 25w of the initial reading
        } else if (abs(powerBuffer.powerEntry[0].watts - watts.value()) < (POWERTABLE_INCREMENT / 2)) {
            for (int i = 1; i < POWER_SAMPLES; i++) {
                if (powerBuffer.powerEntry[i].readings == 0) {
                    powerBuffer.set(i, bt->wattsMetric().value(), bt->currentCadence().value(), bt->currentResistance().value());  // Add additional readings to the buffer.
                    break;
                }
            }
            if (powerBuffer.powerEntry[POWER_SAMPLES - 1].readings == 1) {  // If buffer is full, create a new table entry and clear the buffer.
                this->newEntry(powerBuffer);
                this->toLog();
                powerBuffer.reset();
            }
        } else {  // Reading was outside the range - clear the buffer and start over.
            powerBuffer.reset();
        }
    }
}

// Accepts new data into the table and averages input by number of readings in the power entry.
void PowerTable::newEntry(PowerBuffer& powerBuffer) {
    float watts            = 0;
    int cad                = 0;
    int32_t targetResistance = 0;

    for (int i = 0; i < POWER_SAMPLES; i++) {
        if (powerBuffer.powerEntry[i].readings == 0) {
            // break if powerEntry is not set. This should never happen.
            break;
        }

               // Adjust input watts to an cadence of NORMAL_CAD
        powerBuffer.powerEntry[i].watts = _adjustWattsForCadence(powerBuffer.powerEntry[i].watts, powerBuffer.powerEntry[i].cad);
        powerBuffer.powerEntry[i].cad   = NORMAL_CAD;

        if (i == 0) {  // first loop -> assign values
            watts          = powerBuffer.powerEntry[i].watts;
            targetResistance = powerBuffer.powerEntry[i].targetResistance;
            cad            = powerBuffer.powerEntry[i].cad;
            continue;
        }
#ifdef DEBUG_POWERTABLE
        SS2K_LOGW(POWERTABLE_LOG_TAG, "Buf[%d](%dw)(%dpos)(%dcad)", i, powerBuffer.powerEntry[i].watts, powerBuffer.powerEntry[i].targetResistance, powerBuffer.powerEntry[i].cad);
#endif
        // calculate average
        watts          = (watts + powerBuffer.powerEntry[i].watts) / 2;
        targetResistance = (targetResistance + powerBuffer.powerEntry[i].targetResistance) / 2;
        cad            = (cad + powerBuffer.powerEntry[i].cad) / 2;
    }
#ifdef DEBUG_POWERTABLE
    qDebug() <<"Avg:(%dw)(%dpos)(%dcad)", (int)watts, targetResistance, cad);
#endif
    // Done with powerBuffer
    // To start working on the PowerTable, we need to calculate position in the table for the new entry
    int i = round(watts / POWERTABLE_INCREMENT);

           // Prohibit entries that are less than the number to the left
    if (i > 0) {
        for (int j = i - 1; j > 0; j--) {
            if ((this->powerEntry[j].targetResistance != 0) && (this->powerEntry[j].targetResistance >= targetResistance)) {
                qDebug() <<"Target Slot (%dw)(%d)(%d) was less than previous (%d)(%d)" << (int)watts << i << targetResistance << j << this->powerEntry[j].targetResistance;
                this->powerEntry[j].readings = 1;  // Make previous slot easier to round/faster to change.
                return;
            }
        }
    }
    // Prohibit entries that are greater than the number to the right
    if (i < POWERTABLE_SIZE) {
        for (int j = i + 1; j < POWERTABLE_SIZE; j++) {
            if ((this->powerEntry[j].targetResistance != 0) && (targetResistance >= this->powerEntry[j].targetResistance)) {
                qDebug() <<"Target Slot (%dw)(%d)(%d) was greater than next (%d)(%d)" << (int)watts << i << targetResistance << j << this->powerEntry[j].targetResistance;
                this->powerEntry[j].readings = 1;  // Make next slot easier to round/faster to change.
                return;
            }
        }
    }

    if (this->powerEntry[i].readings == 0) {  // if first reading in this entry
        this->powerEntry[i].watts          = watts;
        this->powerEntry[i].cad            = cad;
        this->powerEntry[i].targetResistance = targetResistance;
        this->powerEntry[i].readings       = 1;
    } else {  // Average and update the readings.
        this->powerEntry[i].watts          = (watts + (this->powerEntry[i].watts * this->powerEntry[i].readings)) / (this->powerEntry[i].readings + 1.0);
        this->powerEntry[i].cad            = (cad + (this->powerEntry[i].cad * this->powerEntry[i].readings)) / (this->powerEntry[i].readings + 1.0);
        this->powerEntry[i].targetResistance = (targetResistance + (this->powerEntry[i].targetResistance * this->powerEntry[i].readings)) / (this->powerEntry[i].readings + 1.0);
        this->powerEntry[i].readings++;
        if (this->powerEntry[i].readings > 10) {
            this->powerEntry[i].readings = 10;  // keep from diluting recent readings too far.
        }
    }
}

// looks up an incline for the requested power and cadence and interpolates the result.
// Returns -99 if no entry matched.
int32_t PowerTable::lookup(int watts, int cad) {
    struct entry {
        float power;
        int32_t targetResistance;
        float cad;
    };

    watts = _adjustWattsForCadence(watts, cad);
    if (watts <= 0) {
        return -99;
    }
    cad = NORMAL_CAD;

    int i         = round(watts / POWERTABLE_INCREMENT);  // find the closest entry
    float scale   = watts / POWERTABLE_INCREMENT - i;     // Should we look at the next higher or next lower index for comparison?
    int indexPair = -1;  // The next closest index with data for interpolation                                                                           // The next closest index
                        // with data for interpolation
    entry above;
    entry below;
    above.power = 0;
    below.power = 0;

    if (this->powerEntry[i].readings == 0) {  // If matching entry is empty, find the next closest index with data
        for (int x = 1; x < POWERTABLE_SIZE; x++) {
            if (i + x < POWERTABLE_SIZE) {
                if (this->powerEntry[i + x].readings > 0) {
                    i += x;
                    break;
                }
            }
            if (i - x >= 0) {
                if (this->powerEntry[i - x].readings > 0) {
                    i -= x;
                    break;
                }
            }
            if ((i - x <= 0) && (i + x >= POWERTABLE_SIZE)) {
                qDebug() << "No data found in Power Table.";
                return RETURN_ERROR;
            }
        }
    }
    if (scale > 0) {  // select the paired element (preferably) above the entry for interpolation
        for (int x = 1; x < POWERTABLE_SIZE; x++) {
            if (i + x < POWERTABLE_SIZE) {
                if (this->powerEntry[i + x].readings > 0) {
                    indexPair = i + x;
                    break;
                }
            }
            if (i - x >= 0) {
                if (this->powerEntry[i - x].readings > 0) {
                    indexPair = i - x;
                    break;
                }
            }
        }
    } else if (scale <= 0) {  // select the paired element (preferably) below the entry for interpolation
        for (int x = 1; x < POWERTABLE_SIZE; x++) {
            if (i + x < POWERTABLE_SIZE) {
                if (this->powerEntry[i + x].readings > 0) {
                    indexPair = i + x;
                    break;
                }
            }
            if (i - x >= 0) {
                if (this->powerEntry[i - x].readings > 0) {
                    indexPair = i - x;
                    break;
                }
            }
        }
    }

    if (indexPair != -1) {
        if (i > indexPair) {
            below.power          = this->powerEntry[indexPair].watts;
            below.targetResistance = this->powerEntry[indexPair].targetResistance;
            below.cad            = this->powerEntry[indexPair].cad;
            above.power          = this->powerEntry[i].watts;
            above.targetResistance = this->powerEntry[i].targetResistance;
            above.cad            = this->powerEntry[i].cad;
        } else if (i < indexPair) {
            below.power          = this->powerEntry[i].watts;
            below.targetResistance = this->powerEntry[i].targetResistance;
            below.cad            = this->powerEntry[i].cad;
            above.power          = this->powerEntry[indexPair].watts;
            above.targetResistance = this->powerEntry[indexPair].targetResistance;
            above.cad            = this->powerEntry[indexPair].cad;
        }
        if (below.targetResistance >= above.targetResistance) {
            qDebug() << "Reverse/No Delta in Power Table";
            return (RETURN_ERROR);
        }
    } else {  // Not enough data
        qDebug() << "No pair in power table";
        return (RETURN_ERROR);
    }
    qDebug() << "PowerTable pairs [%d][%d]" << i << indexPair;

    if (!below.power || !above.power) {  // We should never get here. This is a failsafe vv
        qDebug() << "One of the pair was zero. Calculation rejected.";
        return (RETURN_ERROR);
    }

           // actual interpolation
    int32_t rtargetResistance = below.targetResistance + ((watts - below.power) / (above.power - below.power)) * (above.targetResistance - below.targetResistance);

    return rtargetResistance;
}

int PowerTable::_adjustWattsForCadence(int watts, float cad) {
    if (cad > 0) {
        watts = (watts * (((NORMAL_CAD / cad) + 1) / 2));
        return watts;
    } else {
        return 0;
    }
}

bool PowerTable::load() {
    // load power table from littleFs
    return false;  // return unsuccessful
}

bool PowerTable::save() {
    // save power table from littleFs
    return false;  // return unsuccessful
}

// Display power table in log
void PowerTable::toLog() {
    int len = 4;
    for (int i = 0; i < POWERTABLE_SIZE; i++) {  // Find the longest integer to dynamically size the power table
        int l = snprintf(nullptr, 0, "%d", this->powerEntry[i].targetResistance);
        if (len < l) {
            len = l;
        }
    }
    char buffer[len + 2];
    QString oString  = "";
    char oFormat[5] = "";
    sprintf(oFormat, "|%%%dd", len);

    for (int i = 0; i < POWERTABLE_SIZE; i++) {
        sprintf(buffer, oFormat, this->powerEntry[i].watts);
        oString += buffer;
    }
    qDebug() << oString;
    oString = "";

           // Currently not using CAD in the Power Table.
           // for (int i = 0; i < POWERTABLE_SIZE; i++) {
           //  sprintf(buffer, oFormat, this->powerEntry[i].cad);
           //  oString += buffer;
           //}
           // qDebug() <<"%s|", oString.c_str());
           // oString = "";

    for (int i = 0; i < POWERTABLE_SIZE; i++) {
        sprintf(buffer, oFormat, this->powerEntry[i].targetResistance);
        oString += buffer;
    }
    qDebug() << oString;
}
