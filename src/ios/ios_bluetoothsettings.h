#ifndef IOS_BLUETOOTHSETTINGS_H
#define IOS_BLUETOOTHSETTINGS_H

#include <QString>
#include <QStringList>

/**
 * @brief C++ bridge to AccessorySetupKit (iOS 18+).
 *
 * AccessorySetupKit lets users authorize specific Bluetooth fitness devices
 * through a native iOS picker. Authorized devices:
 *  - Appear in iOS Settings > Privacy & Security > Accessories
 *  - Can be connected via CoreBluetooth without a separate permission prompt
 *  - Are remembered across app launches
 *
 * On iOS < 18 all methods are no-ops / return empty/false.
 *
 * Usage:
 *   // On app start – activate the session so accessories are tracked
 *   ios_bluetoothsettings::activateSession();
 *
 *   // Show the picker (e.g. from a settings button)
 *   ios_bluetoothsettings::showAccessoryPicker();
 *
 *   // Query authorized devices (UUIDs match CBPeripheral identifiers)
 *   QStringList uuids = ios_bluetoothsettings::getAuthorizedBluetoothUUIDs();
 */
class ios_bluetoothsettings {
  public:
    /// Activates the AccessorySetupKit session.
    /// Call once, early in the app lifecycle (e.g. after QApplication starts).
    static void activateSession();

    /// Presents the accessory picker so the user can authorize a nearby
    /// fitness device. Safe to call on iOS < 18 (does nothing).
    static void showAccessoryPicker();

    /// Returns the CoreBluetooth peripheral UUID strings for all accessories
    /// the user has authorized. Empty on iOS < 18 or when no devices are
    /// authorized.
    static QStringList getAuthorizedBluetoothUUIDs();

    /// Returns the display names of all authorized accessories.
    static QStringList getAuthorizedAccessoryNames();

    /// True if at least one accessory has been authorized.
    static bool hasAuthorizedAccessories();

    /// Number of authorized accessories.
    static int authorizedAccessoryCount();

    /// Removes an accessory identified by its CoreBluetooth UUID string.
    static void removeAccessory(const QString &bluetoothUUID);

    /// Removes all authorized accessories.
    static void removeAllAccessories();
};

#endif // IOS_BLUETOOTHSETTINGS_H
