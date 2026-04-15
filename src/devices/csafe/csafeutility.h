/*
 * Copyright (c) 2024 Marcel Verpaalen (marcel@verpaalen.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * This emulates a serial port over a network connection.
 * e.g. as created by ser2net or hardware serial to ethernet converters
 * 
 */
#ifndef CSAFEUTILITY_H
#define CSAFEUTILITY_H

#include <cstdint> // For uint8_t
#include <QString>
#include <QMap>
#include <QPair>

/** 
* @brief This class contains some utility functions supporting the CSAFE protocol
*/
class CSafeUtility {
public:
    static QString getUnitName(int unitCode);
    static double convertToStandard(int unitCode, double value);
    static QString statusByteToText(int statusByte);

private:
    // Static map to hold unit data
    static const QMap<int, QPair<QString, double>> unitData;
};

#endif // CSAFEUTILITY_H
