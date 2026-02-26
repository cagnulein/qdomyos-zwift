//
//  AccessorySetupManager.swift
//  QDomyos-Zwift
//
//  Integrates AccessorySetupKit (iOS 18+) to allow users to pre-authorize
//  Bluetooth fitness devices. Authorized devices appear in iOS Settings >
//  Privacy & Security > Accessories and can be managed from there.
//
//  After a device is authorized via the picker, QZ's CoreBluetooth scanning
//  will find it and connect normally.
//

import Foundation
import CoreBluetooth
import UIKit

@available(iOS 18.0, *)
import AccessorySetupKit

@available(iOS 18.0, *)
@objc public class AccessorySetupManager: NSObject {

    @objc public static let shared = AccessorySetupManager()

    private var session = ASAccessorySession()
    private var sessionActivated = false

    private override init() {
        super.init()
    }

    // MARK: - Session Management

    /// Activates the ASAccessorySession. Must be called before using the picker
    /// or querying authorized accessories.
    @objc public func activateSession() {
        guard !sessionActivated else { return }
        session.activate(on: .main) { [weak self] event in
            self?.handleSessionEvent(event)
        }
        sessionActivated = true
    }

    private func handleSessionEvent(_ event: ASAccessoryEvent) {
        switch event.eventType {
        case .activated:
            print("AccessorySetupKit: Session activated, \(session.accessories.count) authorized accessory/ies")
        case .accessoryAdded:
            let name = event.accessory?.displayName ?? "unknown"
            print("AccessorySetupKit: Accessory added – \(name)")
        case .accessoryChanged:
            let name = event.accessory?.displayName ?? "unknown"
            print("AccessorySetupKit: Accessory changed – \(name)")
        case .accessoryRemoved:
            let name = event.accessory?.displayName ?? "unknown"
            print("AccessorySetupKit: Accessory removed – \(name)")
        case .invalidated:
            print("AccessorySetupKit: Session invalidated")
            sessionActivated = false
        @unknown default:
            break
        }
    }

    // MARK: - Picker

    /// Presents the AccessorySetupKit picker so the user can authorize a
    /// nearby fitness device. Once authorized the device appears in iOS
    /// Settings > Privacy & Security > Accessories.
    ///
    /// The picker discovers devices advertising the most common fitness-
    /// equipment BLE service UUIDs:
    ///   0x1826  FTMS  – bikes, treadmills, rowers, ellipticals
    ///   0x180D  HRM   – heart rate monitors
    ///   0x1816  CSC   – cycling speed & cadence sensors
    ///   0x1818  CP    – cycling power meters
    ///   0x1814  RSC   – running speed & cadence sensors
    @objc public func showPicker(completion: @escaping (Bool) -> Void) {
        var items: [ASPickerDisplayItem] = []

        // FTMS – covers most modern bikes, treadmills, rowers, ellipticals
        let ftmsDescriptor = ASDiscoveryDescriptor()
        ftmsDescriptor.bluetoothServiceUUID = CBUUID(string: "1826")
        let ftmsItem = ASPickerDisplayItem(
            name: NSLocalizedString("Fitness Equipment", comment: "FTMS device picker label"),
            productImage: UIImage(systemName: "figure.run") ?? UIImage(),
            descriptor: ftmsDescriptor
        )
        items.append(ftmsItem)

        // Heart Rate Monitor
        let hrmDescriptor = ASDiscoveryDescriptor()
        hrmDescriptor.bluetoothServiceUUID = CBUUID(string: "180D")
        let hrmItem = ASPickerDisplayItem(
            name: NSLocalizedString("Heart Rate Monitor", comment: "HRM device picker label"),
            productImage: UIImage(systemName: "heart.fill") ?? UIImage(),
            descriptor: hrmDescriptor
        )
        items.append(hrmItem)

        // Cycling Speed & Cadence
        let cscDescriptor = ASDiscoveryDescriptor()
        cscDescriptor.bluetoothServiceUUID = CBUUID(string: "1816")
        let cscItem = ASPickerDisplayItem(
            name: NSLocalizedString("Speed & Cadence Sensor", comment: "CSC device picker label"),
            productImage: UIImage(systemName: "speedometer") ?? UIImage(),
            descriptor: cscDescriptor
        )
        items.append(cscItem)

        // Cycling Power Meter
        let powerDescriptor = ASDiscoveryDescriptor()
        powerDescriptor.bluetoothServiceUUID = CBUUID(string: "1818")
        let powerItem = ASPickerDisplayItem(
            name: NSLocalizedString("Power Meter", comment: "CP device picker label"),
            productImage: UIImage(systemName: "bolt.fill") ?? UIImage(),
            descriptor: powerDescriptor
        )
        items.append(powerItem)

        // Running Speed & Cadence
        let rscDescriptor = ASDiscoveryDescriptor()
        rscDescriptor.bluetoothServiceUUID = CBUUID(string: "1814")
        let rscItem = ASPickerDisplayItem(
            name: NSLocalizedString("Running Sensor", comment: "RSC device picker label"),
            productImage: UIImage(systemName: "figure.walk") ?? UIImage(),
            descriptor: rscDescriptor
        )
        items.append(rscItem)

        session.showPicker(for: items) { error in
            if let error = error {
                print("AccessorySetupKit picker error: \(error.localizedDescription)")
                completion(false)
            } else {
                print("AccessorySetupKit picker completed successfully")
                completion(true)
            }
        }
    }

    // MARK: - Accessory queries

    /// Returns the CoreBluetooth peripheral UUIDs for all accessories that
    /// the user has authorized via the picker. Pass these UUIDs to
    /// CBCentralManager.retrievePeripherals(withIdentifiers:) to connect.
    @objc public func getAuthorizedBluetoothUUIDs() -> [String] {
        return session.accessories.compactMap { accessory in
            accessory.bluetoothIdentifier?.uuidString
        }
    }

    /// Returns the display names of all authorized accessories.
    @objc public func getAuthorizedAccessoryNames() -> [String] {
        return session.accessories.map { $0.displayName }
    }

    /// True if at least one accessory has been authorized via the picker.
    @objc public func hasAuthorizedAccessories() -> Bool {
        return !session.accessories.isEmpty
    }

    /// Number of accessories currently authorized.
    @objc public func authorizedAccessoryCount() -> Int {
        return session.accessories.count
    }

    // MARK: - Accessory removal

    /// Removes a single accessory by its CoreBluetooth UUID string.
    /// The removed accessory will no longer appear in iOS Settings.
    @objc public func removeAccessory(bluetoothUUID: String, completion: @escaping (Bool) -> Void) {
        guard let accessory = session.accessories.first(where: {
            $0.bluetoothIdentifier?.uuidString == bluetoothUUID
        }) else {
            completion(false)
            return
        }
        session.removeAccessory(accessory) { error in
            if let error = error {
                print("AccessorySetupKit: Failed to remove \(accessory.displayName): \(error.localizedDescription)")
                completion(false)
            } else {
                print("AccessorySetupKit: Removed \(accessory.displayName)")
                completion(true)
            }
        }
    }

    /// Removes all authorized accessories.
    @objc public func removeAllAccessories() {
        for accessory in session.accessories {
            session.removeAccessory(accessory) { error in
                if let error = error {
                    print("AccessorySetupKit: Failed to remove \(accessory.displayName): \(error.localizedDescription)")
                } else {
                    print("AccessorySetupKit: Removed \(accessory.displayName)")
                }
            }
        }
    }
}
