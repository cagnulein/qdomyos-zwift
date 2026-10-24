#ifndef SOLEB94PROFILEDETECTOR_H
#define SOLEB94PROFILEDETECTOR_H

#include <QBluetoothUuid>
#include <QByteArray>
#include <QList>
#include <QtGlobal>

#include "devices/bike.h"

enum class SoleB94Protocol {
    Unknown,
    TargetResistanceTenths,
    LegacySimulationParameters
};

namespace soleb94profiledetector {
SoleB94Protocol detect(const QList<QBluetoothUuid> &characteristicUuids);
QByteArray buildResistanceCommand(SoleB94Protocol protocol, resistance_t resistance, double gain, double offset);
resistance_t resistanceFromTelemetry(SoleB94Protocol protocol, bool localTargetActive, resistance_t localTarget,
                                     resistance_t reportedResistance);
}

#endif // SOLEB94PROFILEDETECTOR_H
