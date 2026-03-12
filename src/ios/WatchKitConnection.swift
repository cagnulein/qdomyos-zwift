//
//  WatchKitConnection.swift
//  ElecDemo
//
//  Created by NhatHM on 8/12/19.
//  Copyright © 2019 GST.PID. All rights reserved.
//

import Foundation
import WatchConnectivity

protocol WatchKitConnectionDelegate: class {
    func didFinishedActiveSession()
}

protocol WatchKitConnectionProtocol {
    func startSession()
    func sendMessage(message: [String : AnyObject], replyHandler: (([String : AnyObject]) -> Void)?, errorHandler: ((NSError) -> Void)?)
}

class WatchKitConnection: NSObject {
    let SwiftDebug = swiftDebug()
    static let shared = WatchKitConnection()
    weak var delegate: WatchKitConnectionDelegate?
    static var currentHeartRate = 0
    static var lastHeartRateUpdate = Date.distantPast
    static var distance = 0.0
    static var stepCadence = 0
    static var lastStepCadenceUpdate = Date.distantPast
    static var kcal = 0.0
    static var totalKcal = 0.0
    static var speed = 0.0
    static var power = 0.0
    static var cadence = 0.0
    static var steps = 0
    static var elevationGain = 0.0
    
    private override init() {
        super.init()
    }
    
    public func heartRate() -> Int
    {
        return WatchKitConnection.currentHeartRate;
    }

    public func stepCadence() -> Int
    {
        return WatchKitConnection.stepCadence;
    }
    
    public func steps() -> Int
    {
        return WatchKitConnection.steps;
    }
    
    public func setKCal(Kcal: Double) -> Void
    {
        WatchKitConnection.kcal = Kcal;
    }
    
    public func setTotalKCal(TotalKcal: Double) -> Void
    {
        WatchKitConnection.totalKcal = TotalKcal;
    }
    
    public func setDistance(Distance: Double) -> Void
    {
        WatchKitConnection.distance = Distance;
    }
    
    private let session: WCSession? = WCSession.isSupported() ? WCSession.default : nil
    
    private var validSession: WCSession? {
#if os(iOS)
        if let session = session, session.isPaired, session.isWatchAppInstalled {
            return session
        }
#elseif os(watchOS)
            return session
#endif
        return nil
    }
    
    private var validReachableSession: WCSession? {
        if let session = validSession, session.isReachable {
            return session
        }
        return nil
    }
}

extension WatchKitConnection: WatchKitConnectionProtocol {
    func startSession() {
        session?.delegate = self
        session?.activate()
    }
    
    func sendMessage(message: [String : AnyObject],
                     replyHandler: (([String : AnyObject]) -> Void)? = nil,
                     errorHandler: ((NSError) -> Void)? = nil)
    {
        validReachableSession?.sendMessage(message, replyHandler: { (result) in
            print(result)
        }, errorHandler: { (error) in
            print(error)
        })
    }
}

extension WatchKitConnection: WCSessionDelegate {
    private func intValue(from payload: Any?) -> Int? {
        switch payload {
        case let value as Int:
            return value
        case let value as Double:
            return Int(value)
        case let value as Float:
            return Int(value)
        case let value as NSNumber:
            return value.intValue
        case let value as String:
            return Int(value)
        default:
            return nil
        }
    }

    func session(_ session: WCSession, activationDidCompleteWith activationState: WCSessionActivationState, error: Error?) {
        print("activationDidCompleteWith")
        delegate?.didFinishedActiveSession()
    }
    
    func sessionDidBecomeInactive(_ session: WCSession) {
        print("sessionDidBecomeInactive")
    }
    
    func sessionDidDeactivate(_ session: WCSession) {
        print("sessionDidDeactivate")
    }
    
    func session(_ session: WCSession, didReceiveMessage message: [String : Any]) {
        print("didReceiveMessage")
        print(message)
    }
    
    func session(_ session: WCSession, didReceiveMessage message: [String : Any], replyHandler: @escaping ([String : Any]) -> Void) {
        var replyValues = Dictionary<String, Double>()
        
        print("didReceiveMessage with reply")
        print(message)
        if let heartRate = intValue(from: message["heartRate"]) {
            WatchKitConnection.currentHeartRate = heartRate
            WatchKitConnection.lastHeartRateUpdate = Date()
        }
        if let stepCadence = intValue(from: message["stepCadence"]) {
            WatchKitConnection.stepCadence = stepCadence
            WatchKitConnection.lastStepCadenceUpdate = Date()
        }

        SwiftDebug.qtDebug("WatchKitConnection received payload: \(message)")
        SwiftDebug.qtDebug("WatchKitConnection state HR=\(WatchKitConnection.currentHeartRate) CAD=\(WatchKitConnection.stepCadence)")
        
        replyValues["distance"] = WatchKitConnection.distance
        replyValues["kcal"] = WatchKitConnection.kcal
        replyValues["cadence"] = WatchKitConnection.cadence
        replyValues["power"] = WatchKitConnection.power
        replyValues["speed"] = WatchKitConnection.speed
        replyValues["steps"] = Double(WatchKitConnection.steps)
        replyValues["elevationGain"] = WatchKitConnection.elevationGain

        SwiftDebug.qtDebug(replyValues.debugDescription)

        replyHandler(replyValues)
                
        //LocalNotificationHelper.fireHeartRate(heartReateDouble)
    }
}
