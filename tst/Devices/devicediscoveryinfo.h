#pragma once

#include <QBluetoothDeviceInfo>
#include <QBluetoothUuid>
#include <QSettings>
#include <QString>
#include <QMap>

/**
 * @brief Settings used during the device discovery process.
 */
class DeviceDiscoveryInfo {
    QMap<QString, QVariant> values;
    QBluetoothDeviceInfo bluetoothDeviceInfo;
public :
    /**
     * @brief Constructor.
     * @param loadDefaults Indicates if the default values should be loaded.
     */
    explicit DeviceDiscoveryInfo(bool loadDefaults=true);

    /**
     * @brief Constructor that configures with a specific bluetooth device info object.
     * @param deviceInfo
     * @param loadDefaults Indicates if the default values should be loaded.
     */
    DeviceDiscoveryInfo(const QBluetoothDeviceInfo& deviceInfo, bool loadDefaults=true);



    /**
     * @brief Constructor that configures with a specific bluetooth device info object, copying other values
     * from a DeviceDiscoveryInfo object.
     * @param other The configuration to copy.
     * @param deviceInfo
     * @param loadDefaults Indicates if the default values should be loaded.
     */
    DeviceDiscoveryInfo(const DeviceDiscoveryInfo& other, const QBluetoothDeviceInfo& deviceInfo);

    /**
     * @brief Gets a pointer to the bluetooth device information object.
     * @return
     */
    QBluetoothDeviceInfo * DeviceInfo();

    /**
     * @brief Constant function to get the bluetooth device name.
     * @return
     */
    const QString DeviceName() const;

    /**
     * @brief Utility to include/exclude a service from the bluetooth device info.
     * @param serviceUuid
     * @param include True includes, False excludes
     */
    void includeBluetoothService(const QBluetoothUuid& serviceUuid, bool include);

    /**
     * @brief Adds a specific service if it's not already present.
     * @param serviceUuid
     */
    void addBluetoothService(const QBluetoothUuid& serviceUuid);

    /**
     * @brief Removes a specific service if it's present.
     * @param serviceUuid
     */
    void removeBluetoothService(const QBluetoothUuid& serviceUuid);

    /**
     * @brief Configures the QSettings object.
     * @param settings
     * @param clear Specifies whether to clear the settings object.
     */
    void setValues(QSettings& settings, bool clear=true) const;

    /**
     * @brief Loads this object with values from the QSettings object.
     */
    void getValues(QSettings& settings);

    /**
     * @brief Temporarily creates an empty QSettings object and loads this object's members from it.
     */
    void loadDefaultValues();

    /**
     * @brief Gets the value for the specified key, or the default value if the key is not present.
     * @param key
     * @param defaultValue
     * @return
     */
    QVariant Value(const QString& key, const QVariant& defaultValue) const;


    /**
     * @brief Gets the value for the specified key.
     * @param key
     * @return
     */
    QVariant Value(const QString& key) const;

    /**
     * @brief Sets the value for the specified key.
     * @param key
     * @param value
     */
    void setValue(const QString& key, const QVariant& value);

};
