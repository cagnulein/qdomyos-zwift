import Foundation
import os.log

class ZwiftPlayDevice: AbstractZapDevice {
    private var batteryLevel = 0
    private var lastButtonState: ControllerNotification?

    override func processEncryptedData(bytes: Data) -> Int {
        do {
            os_log("Decrypted: %{public}@", log: OSLog.default, type: .debug, bytes.toHexString())

            let counter = bytes.prefix(MemoryLayout<Int>.size)
            let payload = bytes.suffix(from: MemoryLayout<Int>.size)

            guard let data = zapEncryption.decrypt(counterArray: counter, payload: payload) else {
                os_log("Decrypt failed", log: OSLog.default, type: .error)
                return 0
            }

            let type = data[0]
            let message = data.suffix(from: 1)

            switch type {
            case ZapConstants.controllerNotificationMessageType:
                processButtonNotification(notification: ControllerNotification(data: message))
            case ZapConstants.clickType:
                return processClickButtonNotification(data: data)
            case ZapConstants.emptyMessageType:
                if AbstractZapDevice.logRaw {
                    os_log("Empty Message", log: OSLog.default, type: .debug)
                }
            case ZapConstants.batteryLevelType:
                let notification = BatteryStatus(data: message)
                if batteryLevel != notification.level {
                    batteryLevel = notification.level
                    os_log("Battery level update: %d", log: OSLog.default, type: .debug, batteryLevel)
                }
            default:
                os_log("Unprocessed - Type: %{public}@ Data: %{public}@", log: OSLog.default, type: .error, String(describing: type), data.toHexString())
            }
        } catch {
            os_log("Decrypt failed: %{public}@", log: OSLog.default, type: .error, error.localizedDescription)
        }
        return 0
    }

    private func processButtonNotification(notification: ControllerNotification) {
        if let lastButtonState = lastButtonState {
            let diff = notification.diff(lastButtonState: lastButtonState)
            if !diff.isEmpty {
                os_log("%{public}@", log: OSLog.default, type: .debug, diff)
            }
        } else {
            os_log("%{public}@", log: OSLog.default, type: .debug, notification.description)
        }
        lastButtonState = notification
    }

    private func processClickButtonNotification(data: Data) -> Int {
        if data.count == 5 {
            if data[4] == 0 {
                os_log("Click '-' Button Press", log: OSLog.default, type: .debug)
                return 2
            } else if data[2] == 0 {
                os_log("Click '+' Button Press", log: OSLog.default, type: .debug)
                return 1
            }
        }
        return 0
    }
}
