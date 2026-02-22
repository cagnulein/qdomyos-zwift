import Foundation
import CoreBluetooth

@objc class BleScanner: NSObject, CBCentralManagerDelegate {
    private var centralManager: CBCentralManager!
    private var deviceServices: [String: [String]] = [:]  // MAC address -> service UUIDs
    private var isScanning = false

    static let shared = BleScanner()

    private override init() {
        super.init()
        centralManager = CBCentralManager(delegate: self, queue: nil)
    }

    // MARK: - Public Methods

    @objc func startScan() {
        guard centralManager.state == .poweredOn else {
            print("BleScanner: Bluetooth not powered on, state: \(centralManager.state.rawValue)")
            return
        }

        if !isScanning {
            // Clear previous scan results
            deviceServices.removeAll()

            // Start scanning for all devices (nil = discover all)
            centralManager.scanForPeripherals(withServices: nil, options: [
                CBCentralManagerScanOptionAllowDuplicatesKey: true  // Allow duplicates to catch progressive service updates
            ])
            isScanning = true
            print("BleScanner: Started scanning for BLE devices")
        }
    }

    @objc func stopScan() {
        if isScanning {
            centralManager.stopScan()
            isScanning = false
            print("BleScanner: Stopped scanning, found \(deviceServices.count) devices")
        }
    }

    @objc func getDeviceServices(macAddress: String) -> String {
        // iOS doesn't expose MAC addresses directly, so we use peripheral identifiers
        // The C++ code will need to use peripheral.identifier.uuidString as the "MAC address"
        guard let services = deviceServices[macAddress] else {
            return ""
        }

        // Return comma-separated list of service UUIDs in full 128-bit format
        return services.joined(separator: ",")
    }

    // MARK: - CBCentralManagerDelegate

    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        switch central.state {
        case .poweredOn:
            print("BleScanner: Bluetooth is powered on")
        case .poweredOff:
            print("BleScanner: Bluetooth is powered off")
        case .resetting:
            print("BleScanner: Bluetooth is resetting")
        case .unauthorized:
            print("BleScanner: Bluetooth is unauthorized")
        case .unsupported:
            print("BleScanner: Bluetooth is unsupported")
        case .unknown:
            print("BleScanner: Bluetooth state is unknown")
        @unknown default:
            print("BleScanner: Unknown Bluetooth state")
        }
    }

    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral,
                       advertisementData: [String: Any], rssi RSSI: NSNumber) {

        // Use peripheral identifier as unique key (iOS doesn't expose MAC addresses)
        let deviceId = peripheral.identifier.uuidString

        // Extract service UUIDs from advertisement data
        var serviceUUIDs: [String] = []

        // Get advertised service UUIDs
        if let services = advertisementData[CBAdvertisementDataServiceUUIDsKey] as? [CBUUID] {
            serviceUUIDs.append(contentsOf: services.map { $0.uuidString.lowercased() })
        }

        // Get overflow service UUIDs (services that couldn't fit in advertisement packet)
        if let overflowServices = advertisementData[CBAdvertisementDataOverflowServiceUUIDsKey] as? [CBUUID] {
            serviceUUIDs.append(contentsOf: overflowServices.map { $0.uuidString.lowercased() })
        }

        // Get solicited service UUIDs
        if let solicitedServices = advertisementData[CBAdvertisementDataSolicitedServiceUUIDsKey] as? [CBUUID] {
            serviceUUIDs.append(contentsOf: solicitedServices.map { $0.uuidString.lowercased() })
        }

        // Merge services with existing ones for this device
        if var existingServices = deviceServices[deviceId] {
            // Add new services that aren't already in the list
            for service in serviceUUIDs {
                if !existingServices.contains(service) {
                    existingServices.append(service)
                }
            }
            deviceServices[deviceId] = existingServices
        } else {
            // First time seeing this device
            deviceServices[deviceId] = serviceUUIDs
        }

        // Debug logging
        let name = peripheral.name ?? "Unknown"
        if !serviceUUIDs.isEmpty {
            print("BleScanner: Device '\(name)' (\(deviceId)) advertises services: \(serviceUUIDs.joined(separator: ", "))")
            if let allServices = deviceServices[deviceId], allServices.count > serviceUUIDs.count {
                print("BleScanner: Device '\(name)' total services collected: \(allServices.joined(separator: ", "))")
            }
        }
    }
}
