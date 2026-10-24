#include "soleb94profiledetector.h"

namespace {
const QBluetoothUuid kFitnessMachineFeature((quint16)0x2ACC);
const QBluetoothUuid kSupportedResistanceLevelRange((quint16)0x2AD6);
const QBluetoothUuid kTreadmillData((quint16)0x2ACD);
const QBluetoothUuid kCrossTrainerData((quint16)0x2ACE);
constexpr char kLegacySimulationParametersCommand = 0x11;
constexpr char kTargetResistanceCommand = 0x04;
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

QByteArray soleb94profiledetector::buildResistanceCommand(SoleB94Protocol protocol, resistance_t resistance,
                                                            double gain, double offset) {
    if (protocol == SoleB94Protocol::Unknown) {
        return QByteArray();
    }

    const resistance_t nonNegativeResistance = qMax<resistance_t>(0, resistance);
    if (protocol == SoleB94Protocol::LegacySimulationParameters) {
        const int32_t encodedResistance = qBound<int32_t>(
            0, static_cast<int32_t>((static_cast<double>(nonNegativeResistance) * gain + offset) * 10.0), 0xFFFF);
        QByteArray command;
        command.append(kLegacySimulationParametersCommand);
        command.append(static_cast<char>(0x00));
        command.append(static_cast<char>(0x00));
        command.append(static_cast<char>(encodedResistance & 0xFF));
        command.append(static_cast<char>((encodedResistance >> 8) & 0xFF));
        command.append(static_cast<char>(0x28));
        command.append(static_cast<char>(0x19));
        return command;
    }

    const int32_t encodedResistance = qBound<int32_t>(
        0, static_cast<int32_t>(static_cast<double>(nonNegativeResistance) * 10.0), 0xFF);
    QByteArray command;
    command.append(kTargetResistanceCommand);
    command.append(static_cast<char>(encodedResistance));
    return command;
}

resistance_t soleb94profiledetector::resistanceFromTelemetry(SoleB94Protocol protocol, bool localTargetActive,
                                                              resistance_t localTarget,
                                                              resistance_t reportedResistance) {
    if (protocol == SoleB94Protocol::LegacySimulationParameters && localTargetActive) {
        return localTarget;
    }
    return reportedResistance;
}
