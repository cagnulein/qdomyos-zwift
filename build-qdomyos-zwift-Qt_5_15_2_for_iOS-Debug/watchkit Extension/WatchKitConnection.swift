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
    func didReceiveUserName(_ userName: String)
}

protocol WatchKitConnectionProtocol {
    func startSession()
    func sendMessage(message: [String : AnyObject], replyHandler: (([String : AnyObject]) -> Void)?, errorHandler: ((NSError) -> Void)?)
    
}

class WatchKitConnection: NSObject {
    static let shared = WatchKitConnection()
    public static var distance = 0.0
    public static var kcal = 0.0
    public static var totalKcal = 0.0
    public static var stepCadence = 0
    public static var speed = 0.0
    public static var cadence = 0.0
    public static var power = 0.0
    public static var steps = 0
    public static var elevationGain = 0.0
    weak var delegate: WatchKitConnectionDelegate?
    
    private override init() {
        super.init()
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
    
    private let session: WCSession? = WCSession.isSupported() ? WCSession.default : nil
    
    private var validSession: WCSession? {
#if os(iOS)
        if let session = session, session.isPaired, session.isWatchAppInstalled {
            return session
        }
#elseif os(watchOS)
            return session
#endif
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
            if let dDistance = WatchKitConnection.doubleValue(result["distance"]) {
                WatchKitConnection.distance = dDistance
            }
            if let dKcal = WatchKitConnection.doubleValue(result["kcal"]) {
                WatchKitConnection.kcal = dKcal
                WorkoutTracking.kcal = dKcal
            } else {
                WatchKitConnection.shared.sendDebug("reply missing kcal raw=\(result)")
            }
            if let totalKcalDouble = WatchKitConnection.doubleValue(result["totalKcal"]) {
                WatchKitConnection.totalKcal = totalKcalDouble
                WorkoutTracking.totalKcal = totalKcalDouble
            } else {
                WatchKitConnection.shared.sendDebug("reply missing totalKcal raw=\(result) localKcal=\(WatchKitConnection.kcal) localTotalKcal=\(WatchKitConnection.totalKcal)")
            }
            if let dSpeed = WatchKitConnection.doubleValue(result["speed"]) {
                WatchKitConnection.speed = dSpeed
            }
            if let dPower = WatchKitConnection.doubleValue(result["power"]) {
                WatchKitConnection.power = dPower
            }
            if let dCadence = WatchKitConnection.doubleValue(result["cadence"]) {
                WatchKitConnection.cadence = dCadence
            }
            if let stepsDouble = WatchKitConnection.doubleValue(result["steps"]) {
                let iSteps = Int(stepsDouble)
                WatchKitConnection.steps = iSteps
            }
            if let elevationGainDouble = WatchKitConnection.doubleValue(result["elevationGain"]) {
                WatchKitConnection.elevationGain = elevationGainDouble
                // Calculate flights climbed and update WorkoutTracking
                let flightsClimbed = elevationGainDouble / 3.048  // One flight = 10 feet = 3.048 meters
                WorkoutTracking.flightsClimbed = flightsClimbed
                WorkoutTracking.elevationGain = elevationGainDouble
                print("WatchKitConnection: Received elevation gain: \(elevationGainDouble)m, flights: \(flightsClimbed)")
            }
            WatchKitConnection.shared.sendDebug("reply parsed local distance=\(WatchKitConnection.distance) kcal=\(WatchKitConnection.kcal) totalKcal=\(WatchKitConnection.totalKcal) speed=\(WatchKitConnection.speed) power=\(WatchKitConnection.power) cadence=\(WatchKitConnection.cadence) steps=\(WatchKitConnection.steps) elevationGain=\(WatchKitConnection.elevationGain)")
        }, errorHandler: { (error) in
            print(error)
        })
    }

    func sendDebug(_ message: String) {
        validReachableSession?.sendMessage(["watchDebug": message as AnyObject], replyHandler: nil, errorHandler: { (error) in
            print("Watch debug send error: \(error)")
        })
    }
}

extension WatchKitConnection: WCSessionDelegate {
    func session(_ session: WCSession, activationDidCompleteWith activationState: WCSessionActivationState, error: Error?) {
        print("activationDidCompleteWith")
    }
    
    func session(_ session: WCSession, didReceiveMessage message: [String : Any], replyHandler: @escaping ([String : Any]) -> Void) {
        guard let userName = message.values.first as? String else {
            return
        }
        
        delegate?.didReceiveUserName(userName)
    }
}
