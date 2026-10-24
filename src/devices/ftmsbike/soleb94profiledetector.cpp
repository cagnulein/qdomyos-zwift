#include "soleb94profiledetector.h"

namespace {
const QBluetoothUuid kFitnessMachineFeature((quint16)0x2ACC);
const QBluetoothUuid kSupportedResistanceLevelRange((quint16)0x2AD6);
const QBluetoothUuid kTreadmillData((quint16)0x2ACD);
const QBluetoothUuid kCrossTrainerData((quint16)0x2ACE);
}

SoleB94Protocol soleb94profiledetector::detect(const QList<QBluetoothUuid> &characteristicUuids) {
    const bool hasModernFeature = characteristicUuids.contains(kFitnessMachineFeature);
    const bool hasModernResistanceRange = characteristicUuids.contains(kSupportedResistanceLevelRange);
    const bool hasLegacyTreadmillData = characteristicUuids.contains(kTreadmillData);
    const bool hasLegacyCrossTrainerData = characteristicUuids.contains(kCrossTrainerData);

    const bool hasModernMarker = hasModernFeature || hasModernResistanceRange;
    const bool hasLegacyMarker = hasLegacyTreadmillData || hasLegacyCrossTrainerData;

    if (hasModernMarker && hasLegacyMarker) {
        return SoleB94Protocol::Unknown;
    }

    if (hasModernFeature && hasModernResistanceRange) {
        return SoleB94Protocol::TargetResistanceTenths;
    }

    if (hasLegacyTreadmillData && hasLegacyCrossTrainerData) {
        return SoleB94Protocol::LegacySimulationParameters;
    }

    return SoleB94Protocol::Unknown;
}
