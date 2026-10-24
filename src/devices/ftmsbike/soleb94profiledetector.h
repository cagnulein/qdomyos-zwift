#ifndef SOLEB94PROFILEDETECTOR_H
#define SOLEB94PROFILEDETECTOR_H

#include <QBluetoothUuid>
#include <QList>

enum class SoleB94Protocol {
    Unknown,
    TargetResistanceTenths,
    LegacySimulationParameters
};

namespace soleb94profiledetector {
SoleB94Protocol detect(const QList<QBluetoothUuid> &characteristicUuids);
}

#endif // SOLEB94PROFILEDETECTOR_H
