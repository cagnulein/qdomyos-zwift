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
    static var distance = 0.0
    static var stepCadence = 0
    static var kcal = 0.0
    static var totalKcal = 0.0
    static var speed = 0.0
    static var power = 0.0
    static var cadence = 0.0
    static var steps = 0
    static var elevationGain = 0.0
    static var workoutState = 3
    static var workoutType = 0
    
    private override init() {
        super.init()
    }
    
    public func heartRate() -> Int
    {
        return WatchKitConnection.currentHeartRate;
    }

    private static func doubleValue(_ value: Any?) -> Double? {
        if let value = value as? Double {
            return value
        }
        if let value = value as? NSNumber {
            return value.doubleValue
        }
        if let value = value as? String {
            return Double(value)
        }
        return nil
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
        if let watchDebug = message["watchDebug"] as? String {
            SwiftDebug.qtDebug("Watch debug: \(watchDebug)")
        }
    }
    
    func session(_ session: WCSession, didReceiveMessage message: [String : Any], replyHandler: @escaping ([String : Any]) -> Void) {
        var replyValues = Dictionary<String, Double>()
        
        print("didReceiveMessage with reply")
        print(message)
        
        SwiftDebug.qtDebug("WatchKitConnection received payload: \(message)")

        if let watchDebug = message["watchDebug"] as? String {
            SwiftDebug.qtDebug("Watch debug: \(watchDebug)")
        }
        
        if let heartRate = Self.doubleValue(message["heartRate"]), heartRate > 0, heartRate <= 250 {
            WatchKitConnection.currentHeartRate = Int(heartRate.rounded())
        } else if let stepCadence = message["stepCadence"] as? String {
            WatchKitConnection.stepCadence = Int(stepCadence) ?? WatchKitConnection.stepCadence
        } else if let watchDebug = message["watchDebug"] as? String {
            SwiftDebug.qtDebug("Watch debug: \(watchDebug)")
        }
        
        replyValues["distance"] = WatchKitConnection.distance
        replyValues["kcal"] = WatchKitConnection.kcal
        replyValues["totalKcal"] = WatchKitConnection.totalKcal
        replyValues["cadence"] = WatchKitConnection.cadence
        replyValues["power"] = WatchKitConnection.power
        replyValues["speed"] = WatchKitConnection.speed
        replyValues["steps"] = Double(WatchKitConnection.steps)
        replyValues["elevationGain"] = WatchKitConnection.elevationGain
        replyValues["workout_state"] = Double(WatchKitConnection.workoutState)
        replyValues["workout_type"] = Double(WatchKitConnection.workoutType)

        SwiftDebug.qtDebug(replyValues.debugDescription)

        replyHandler(replyValues)
    }
}
