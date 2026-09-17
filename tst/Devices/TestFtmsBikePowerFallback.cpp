#include <gtest/gtest.h>

#include <QMetaObject>
#include <QBluetoothAddress>
#include <QBluetoothDeviceInfo>
#include <QBluetoothUuid>
#include <QSettings>
#include <QtBluetooth/qlowenergycharacteristicdata.h>

#include "devices/ftmsbike/ftmsbike.h"
#include "qzsettings.h"

namespace {

class TestFtmsBike : public ftmsbike {
  public:
    TestFtmsBike() : ftmsbike(true, true, 0, 1.0) {}

    uint16_t wattValue() const { return m_watt.value(); }
};

void feedPacket(TestFtmsBike &device, const QByteArray &packet) {
    QLowEnergyCharacteristicData characteristicData;
    characteristicData.setUuid(QBluetoothUuid((quint16)0x2AD2));
    QLowEnergyCharacteristic characteristic(characteristicData);
    QMetaObject::invokeMethod(&device, "characteristicChanged", Qt::DirectConnection,
                              Q_ARG(QLowEnergyCharacteristic, characteristic), Q_ARG(QByteArray, packet));
}

} // namespace

TEST(FtmsBikePowerFallbackTest, MerachS26cLogPacketKeepsPowerAtZeroWhenCadenceStops) {
    QSettings settings;
    const bool hadPowerSensorSetting = settings.contains(QZSettings::power_sensor_name);
    const QVariant previousPowerSensor = settings.value(QZSettings::power_sensor_name);
    settings.setValue(QZSettings::power_sensor_name, QStringLiteral("Disabled"));
    settings.sync();

    TestFtmsBike device;
    device.deviceDiscovered(QBluetoothDeviceInfo(QBluetoothAddress(QStringLiteral("00:00:00:00:00:00")),
                                                 QStringLiteral("MRK-S26C-247E"), 0));

    // FTMS packet from a0a94bf8-82e7-4a15-b538-6f1d0d01dba5_debug-Wed_Sep_16_19_47_01_2026.log.gz:
    // instant cadence = 0, average cadence = 36, average power = 26 W.
    feedPacket(device, QByteArray::fromHex("fc 0b 00 00 00 00 48 00 c8 00 00 01 00 20 00 1a 00 05 00 00 00 00 00 42 00"));

    EXPECT_EQ(device.wattValue(), 0);

    if (hadPowerSensorSetting)
        settings.setValue(QZSettings::power_sensor_name, previousPowerSensor);
    else
        settings.remove(QZSettings::power_sensor_name);
}

TEST(FtmsBikePowerFallbackTest, OtherFtmsBikeKeepsAveragePowerFromTheSameLogPacket) {
    QSettings settings;
    const bool hadPowerSensorSetting = settings.contains(QZSettings::power_sensor_name);
    const QVariant previousPowerSensor = settings.value(QZSettings::power_sensor_name);
    settings.setValue(QZSettings::power_sensor_name, QStringLiteral("Disabled"));
    settings.sync();

    TestFtmsBike device;
    device.deviceDiscovered(QBluetoothDeviceInfo(QBluetoothAddress(QStringLiteral("00:00:00:00:00:00")),
                                                 QStringLiteral("OTHER-FTMS-BIKE"), 0));
    feedPacket(device, QByteArray::fromHex("fc 0b 00 00 00 00 48 00 c8 00 00 01 00 20 00 1a 00 05 00 00 00 00 00 42 00"));

    EXPECT_EQ(device.wattValue(), 26);

    if (hadPowerSensorSetting)
        settings.setValue(QZSettings::power_sensor_name, previousPowerSensor);
    else
        settings.remove(QZSettings::power_sensor_name);
}
