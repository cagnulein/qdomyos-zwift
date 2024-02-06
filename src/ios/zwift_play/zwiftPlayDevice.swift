import Foundation
import os.log

@available(iOS 14.0, *)
class ZwiftPlayDevice: AbstractZapDevice {
    private var batteryLevel = 0

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
                os_log("controllerNotificationMessageType")
                //processButtonNotification(notification: ControllerNotification(data: message))
            case ZapConstants.clickType:
                return processClickButtonNotification(data: data)
            case ZapConstants.emptyMessageType:
                os_log("emptyMessageType")
            case ZapConstants.batteryLevelType:
                os_log("batteryLevelType")
            default:
                os_log("Unprocessed - Type: %{public}@ Data: %{public}@", log: OSLog.default, type: .error, String(describing: type), data.toHexString())
            }
        } catch {
            os_log("Decrypt failed: %{public}@", log: OSLog.default, type: .error, error.localizedDescription)
        }
        return 0
    }
/*
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
    }*/

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
