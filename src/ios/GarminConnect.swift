//
//  GarminConnect.swift
//  qdomyoszwift
//
//  Created by Roberto Viola on 17/03/23.
//

import Foundation
import ConnectIQ

extension ConnectIQ {
    static var shared: ConnectIQ? {
        return sharedInstance()
    }
}

@available(iOS 13.0, *)
@objc public class GarminConnect : NSObject {
    let v = GarminConnectSwift()
    
    @objc public func getHR() -> Int {
        return v.HR;
    }
    
    @objc public func getFootCad() -> Int {
        return v.FootCad;
    }
    
    @objc public func getPower() -> Int {
        return v.Power;
    }
    
    @objc public func getSpeed() -> Double {
        return v.Speed;
    }

    @objc public func urlParser(_ url: URL) {
        v.urlParser(url)
    }
}

@available(iOS 13.0, *)
class GarminConnectSwift: NSObject, IQDeviceEventDelegate, IQAppMessageDelegate {
    // This must match the value in `Info.plist`.
    private static let urlScheme = "org.cagnulein.connectiqcomms-ciq"
    // UUID from `manifest.xml` of the ConnectIQ app.
    private static let watchAppUuid = UUID(uuidString: "feec8674-2795-4e03-a283-0b69a0a291e3")

    // Device UUID mapped to the CommsApp on that device.
    private var apps: [UUID: IQApp] = [:]
    private var devices: [UUID: IQDevice] = [:]

    @Published private var message = ""
    
    public var HR: Int = 0
    public var FootCad: Int = 0
    public var Power: Int = 0
    public var Speed: Double = 0
    
    let SwiftDebug = swiftDebug()

    private let formatter: NumberFormatter = {
        let formatter = NumberFormatter()
        formatter.maximumFractionDigits = 7
        return formatter
    }()
    
    private func broadcastMessage() {
        for app in apps.values {
            // You may send any ObjC type (e.g. NSNumber, NSString, NSArray, NSDictionary).
            // Unless you're experiencing difficulties, there's no need to use the `NS*` types directly,
            // you can use their Swift equivalents.
            ConnectIQ.shared?.sendMessage("General Kenobi.", to: app, progress: nil, completion: { print($0) })
        }
    }

    func urlParser(_ url: URL) -> Bool {
        guard url.scheme == GarminConnectSwift.urlScheme,
                 let devices = ConnectIQ.shared?.parseDeviceSelectionResponse(from: url) as? [IQDevice] else { return false }
        for device in devices {
            self.devices[device.uuid] = device
        }
        registerForDeviceEvents(devices: devices)
        SwiftDebug.qtDebug("GarminConnect: urlParser: return true")
        return true
    }

    private func registerForDeviceEvents(devices: [IQDevice]) {
        for device in devices {
            ConnectIQ.shared?.register(forDeviceEvents: device, delegate: self)
        }
    }

    public func deviceStatusChanged(_ device: IQDevice!, status: IQDeviceStatus) {
        let descriptionStatus: String
        switch status {
        case .connected:
            // The `store` is not necessary for sending messages, I suppose it's for when you want the user to download the app.
            // `IQApp` class needs to be instantiated for every IQDevice, you can't share them, it's the app on the specific device.
            let app = IQApp(uuid: GarminConnectSwift.watchAppUuid, store: nil, device: device)
            apps[device.uuid] = app
            print(device.uuid)
            descriptionStatus = "connected"

            ConnectIQ.shared?.register(forAppMessages: app, delegate: self)

            // IMPORTANT: Apparently sending a message right after connecting sends the messages to the void.
            // I have no idea why it doesn't work, but feel free to shrink the delay. I've found that 100ms works consistently.
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.1) {
                ConnectIQ.shared?.sendMessage("Hello there.", to: app, progress: nil, completion: { print($0) })
            }

        case .bluetoothNotReady, .invalidDevice, .notConnected, .notFound:
            print(device.uuid)
            if let app = apps[device.uuid] {
                ConnectIQ.shared?.unregister(forAppMessages: app, delegate: self)
            }
            descriptionStatus = "invalidDevice"
            apps.removeValue(forKey: device.uuid)

        @unknown default:
            descriptionStatus = "case not handled"
            print("New case, still unhandled. \(status.rawValue)")
        }
        
        SwiftDebug.qtDebug("GarminConnect: deviceStatusChanged: status " + descriptionStatus + " " + device.uuid.uuidString)
    }

    func receivedMessage(_ message: Any!, from app: IQApp!) {
        print("Received message from ConnectIQ: \(message.debugDescription)")
        SwiftDebug.qtDebug("GarminConnect: receivedMessage: " + message.debugDescription)

        guard let array = message as? [Any] else {
            print("Failed to parse message sent from ConnectIQ.")
            return
        }

        for (index, contents) in array.enumerated() {
            guard let dictionary = contents as? [Int: Any] else {
                print("Failed to parse ConnectIQ message contents at index \(index).")
                return
            }
            print(dictionary)
            HR = dictionary[0] as? Int ?? 0
            FootCad = dictionary[1] as? Int ?? 0
            Power = dictionary[2] as? Int ?? 0
            Speed = dictionary[3] as? Double ?? 0
            SwiftDebug.qtDebug("Garmin HR: \(HR)")
            SwiftDebug.qtDebug("Garmin Foot Cadence: \(FootCad)")
            SwiftDebug.qtDebug("Garmin Power: \(Power)")
            SwiftDebug.qtDebug("Garmin Speed: \(Speed)")
        }
    }

    deinit {
        ConnectIQ.shared?.unregister(forAllDeviceEvents: self)
        ConnectIQ.shared?.unregister(forAllAppMessages: self)
    }
}

