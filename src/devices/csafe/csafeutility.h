#ifndef CSAFEUTILITY_H
#define CSAFEUTILITY_H

#include <QString>
#include <QMap>
#include <QPair>
#include <cstdint> // For uint8_t

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
