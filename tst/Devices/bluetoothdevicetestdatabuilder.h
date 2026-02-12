#pragma once


#include "bluetoothdevice.h"
#include "bluetoothdevicetestdata.h"
#include "devicetestdataindex.h"

/**
 * @brief Extra functionality on a BluetoothDeviceTestData for configuring it.
 */
class BluetoothDeviceTestDataBuilder : public virtual BluetoothDeviceTestData
{

public:
    /**
     * @brief Constructor.
     * @param name A user-friendly name for the device variant this test data is for. Should be unique for this object.
     */
    BluetoothDeviceTestDataBuilder(QString name);

    /**
     * @brief Indicates the type of bluetoothdevice object that should be produced from the conditions defined in this object.
     * @return
     */
    template <class T>
    BluetoothDeviceTestDataBuilder * expectDevice()
    {
        if(this->isExpectedDevice)
            throw std::invalid_argument("Expected device already set.");

        this->isExpectedDevice= [](bluetoothdevice * detectedDevice) -> bool { return dynamic_cast<T*>(detectedDevice)!=nullptr; };
        this->expectedDeviceType = DeviceTestDataIndex::GetTypeId<T>();

        // TODO enhance exclusion test to be able to distinguish instances rather than just type.
        // // Exclude this type, i.e. don't redetect and recreate.
        // this->excluding<T>();

        return this;
    }

    /**
     * @brief Add a list of device names that should be identified as this device.
     * If the comparison specified contains "starts with", then samples that start with the
     * specified device name will be added. If a case sensitive comparison is specified,
     * samples with invalid casing will be added to the invalid names list.
     * @param deviceNames
     */
    BluetoothDeviceTestDataBuilder * acceptDeviceNames(const QStringList& deviceNames, DeviceNameComparison cmp);

    /**
     * @brief Add a device name that should be identified as this device.
     * If the comparison specified contains "starts with", then samples that start with the
     * specified device name will be added. If a case sensitive comparison is specified,
     * samples with invalid casing will be added to the invalid names list.
     * @param deviceName
     * @param length The expected length. Use 0 for unrestricted.
     */
    BluetoothDeviceTestDataBuilder * acceptDeviceName(const QString& deviceName, DeviceNameComparison cmp, uint8_t length=0);

    /**
     * @brief Add a device name as a prefix and suffix that should be identified as this device.
     * If a case sensitive comparison is specified,
     * samples with invalid casing will be added to the invalid names list.
     * @param deviceName
     */
    BluetoothDeviceTestDataBuilder * acceptDeviceName(const QString& deviceNameStartsWith, const QString& deviceNameEndsWith, DeviceNameComparison cmp);

    /**
     * @brief Add a device name that should NOT be identified as this device.
     * @param deviceName
     */
    BluetoothDeviceTestDataBuilder * rejectDeviceName(const QString& deviceName, const DeviceNameComparison cmp);

    /**
     * @brief Sets a generator for multiple enabling or disabling settings configurations.
     * @param applicator
     * @return
     */
    BluetoothDeviceTestDataBuilder * configureSettingsWith(ConfigurationApplicatorMultiple applicator);

    /**
     * @brief Sets a functor to produce the single enabling or disabling settings configuration.
     * @param applicator
     * @return
     */
    BluetoothDeviceTestDataBuilder * configureSettingsWith(ConfigurationApplicatorSingle applicator);

    /**
     * @brief Specifies that a single boolean setting is used to generate an enabling or
     *  disabling configuration.
     * @param qzSettingsKey The settings key to alter.
     * @param enabledValue The value of the setting that indicates an enabling configuration.
     * @return
     */
    BluetoothDeviceTestDataBuilder * configureSettingsWith(const QString& qzSettingsKey, bool enabledValue=true);

    /**
     * @brief Specifies that a single setting is used to generate an enabling or disabling configuration,
     * and additionally specify the enabling value and disabling value, which must be different.
     * @param qzSettingsKey The settings key to alter.
     * @param enablingValue The value of the setting that indicates an enabling configuration.
     * @param disablingValue The value of the setting that indicates a disabling configuration.
     * @return
     */
    BluetoothDeviceTestDataBuilder * configureSettingsWith(const QString &qzSettingsKey, QVariant enablingValue, QVariant disablingValue);

    /**
     * @brief Specifies that a single bluetooth service is used to generate an enabling or disabling configuration.
     * @param uuid The UUID of the bluetooth service.
     * @param addedIsEnabled Indicates what the presence of the service indicates.
     * @return
     */
    BluetoothDeviceTestDataBuilder * configureSettingsWith(const QBluetoothUuid &uuid, bool addedIsEnabled=true);

    /**
     * @brief Indicates that if a device of the types with the specified type ids is already recognised,
     * this one should not be detected even if the other conditions are met.
     * @param exclusions
     * @return
     */
    BluetoothDeviceTestDataBuilder * excluding(std::initializer_list<DeviceTypeId> exclusions);

    /**
     * @brief Indicates that if a device of the specified type is already recognised,
     * this one should not be detected even if the other conditions are met.
     * @return
     */
    template<class T>
    BluetoothDeviceTestDataBuilder * excluding() { return excluding({DeviceTestDataIndex::GetTypeId<T>()}); }

    /**
     * @brief Disable the test data, optionally stating a reason, and excluding it from any test reports.
     * @param reason
     * @return
     */
    BluetoothDeviceTestDataBuilder * disable(const QString& reason=nullptr);

    /**
     * @brief Indicates that the test data should be included in tests, but skipped so it appears in reports as having been skipped,
     * with an optional reason.
     * @param reason
     * @return
     */
    BluetoothDeviceTestDataBuilder * skip(const QString& reason=nullptr);
};
