import Foundation
import os.log

// Assuming Logger and extensions for Data to convert to hex string and to check prefix are defined elsewhere in your Swift project.
extension Data {
    func toHexString() -> String {
        self.map { String(format: "%02x", $0) }.joined()
    }

    func starts(with prefix: Data) -> Bool {
        self.prefix(prefix.count) == prefix
    }
}

class AbstractZapDevice {
    private static let logRaw = true

    private var devicePublicKeyBytes: Data?
    private let localKeyProvider = LocalKeyProvider()
    open var zapEncryption: ZapCrypto

    init() {
        self.zapEncryption = ZapCrypto(localKeyProvider: localKeyProvider)
    }

    func processCharacteristic(characteristicName: String, bytes: Data?) -> Int {
        guard let bytes = bytes else { return 0 }

        if Self.logRaw {
            os_log("%{public}@ %{public}@", log: OSLog.default, type: .debug, characteristicName, bytes.toHexString())
        }

        if bytes.starts(with: ZapConstants.rideOn + ZapConstants.responseStart) {
            processDevicePublicKeyResponse(bytes: bytes)
        } else if bytes.count > MemoryLayout<Int>.size + EncryptionUtils.macLength {
            return processEncryptedData(bytes: bytes)
        } else {
            // Logger equivalent in Swift
            os_log("Unprocessed - Data Type: %{public}@", log: OSLog.default, type: .error, bytes.toHexString())
        }
        return 0
    }

    func buildHandshakeStart() -> Data {
        return ZapConstants.rideOn + ZapConstants.requestStart + localKeyProvider.getPublicKeyBytes()
    }

    private func processDevicePublicKeyResponse(bytes: Data) {
        let startIndex = ZapConstants.rideOn.count + ZapConstants.responseStart.count
        devicePublicKeyBytes = bytes.subdata(in: startIndex..<bytes.count)
        zapEncryption.initialise(devicePublicKeyBytes: devicePublicKeyBytes!)
        if Self.logRaw {
            // Logger equivalent in Swift
            os_log("Device Public Key - %{public}@", log: OSLog.default, type: .debug, devicePublicKeyBytes!.toHexString())
        }
    }

    // Abstract method placeholder - Swift doesn't support abstract classes/methods directly, so this method should be overridden in subclass.
    func processEncryptedData(bytes: Data) -> Int {
        fatalError("This method should be overridden by subclasses")
    }
}
