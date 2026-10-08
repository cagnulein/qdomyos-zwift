#pragma once

#include <QByteArray>
#include <QList>
#include <QtGlobal>

namespace fitpro {

struct FieldWrite {
    quint8 id;
    QByteArray value;
};

struct Frame {
    quint8 device = 0;
    quint8 command = 0;
    QList<FieldWrite> fields;
};

class Codec {
public:
    static constexpr quint8 MainDevice = 0x02;
    static constexpr quint8 Treadmill = 0x04;
    static constexpr quint8 InclineTrainer = 0x05;
    static constexpr quint8 Elliptical = 0x06;
    static constexpr quint8 FitnessBike = 0x07;
    static constexpr quint8 SpinBike = 0x08;

    static constexpr quint8 WriteReadData = 0x02;
    static constexpr quint8 GetSupportedDevices = 0x80;
    static constexpr quint8 GetInfo = 0x81;
    static constexpr quint8 GetSupportedCommands = 0x88;

    static constexpr quint8 FieldSpeed = 0x00;
    static constexpr quint8 FieldGrade = 0x01;
    static constexpr quint8 FieldResistance = 0x02;
    static constexpr quint8 FieldWorkoutMode = 0x0c;
    static constexpr quint8 FieldRequiredStartRequested = 0x6c;

    // Numeric converters used by the modern iFit FitPro path.
    static QByteArray fixedU16(double value, double scale = 100.0);
    static QByteArray speed(double kph);
    static QByteArray grade(double percentage);
    static QByteArray resistance(double value);
    static QByteArray boolValue(bool value);

    // Returns the complete FitPro command, before the BLE wrapper.
    static QByteArray command(quint8 device, quint8 commandId, const QByteArray &payload = QByteArray());
    static QByteArray writeReadFrame(quint8 device, const QList<FieldWrite> &writes = {},
                                     const QList<quint8> &reads = {});
    static QByteArray discoveryFrame(quint8 commandId, quint8 device = MainDevice);

    // Returns [02 04 02 <core length>] + core.
    static QByteArray wrap(const QByteArray &core);

    // Returns the exact BLE values emitted by the APK packetizer: one FE header
    // followed by 20-byte fragments carrying up to 18 command bytes each.
    static QList<QByteArray> packetize(const QByteArray &wrapped);
    static QList<QByteArray> writeRead(quint8 device, const QList<FieldWrite> &writes = {},
                                       const QList<quint8> &reads = {});
    static QList<QByteArray> discovery(quint8 commandId, quint8 device = MainDevice);
    static QList<QByteArray> speedCommand(double kph, quint8 device = Treadmill);
    static QList<QByteArray> gradeCommand(double percentage, quint8 device = Treadmill);
    static QList<QByteArray> resistanceCommand(double value, quint8 device = FitnessBike);
    static QList<QByteArray> rawResistanceCommand(quint16 rawValue, quint8 device = FitnessBike);
    static QList<QByteArray> requiredStartCommand(bool requested, quint8 device = Treadmill);
    static QList<QByteArray> workoutModeRead(quint8 device = Treadmill);

    // RX helpers for the same FE/fragment/FF transport used by the APK.
    // An empty result means that framing or checksum validation failed.
    static QByteArray reassemble(const QList<QByteArray> &packets);
    static QByteArray unwrap(const QByteArray &wrapped);
    static bool decode(const QByteArray &core, Frame &frame);

    static quint8 checksum(const QByteArray &data);

private:
    static QByteArray writeSection(const QList<FieldWrite> &writes);
    static QByteArray readSection(const QList<quint8> &reads);
};

} // namespace fitpro
