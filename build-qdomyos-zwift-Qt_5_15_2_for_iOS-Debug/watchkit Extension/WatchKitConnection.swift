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
    public static var stepCadence = 0
    public static var speed = 0.0
    public static var cadence = 0.0
    public static var power = 0.0
    public static var steps = 0
    weak var delegate: WatchKitConnectionDelegate?
    
    private override init() {
        super.init()
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
            let dDistance = Double(result["distance"] as! Double)
            WatchKitConnection.distance = dDistance
            let dKcal = Double(result["kcal"] as! Double)
            WatchKitConnection.kcal = dKcal
            
            let dSpeed = Double(result["speed"] as! Double)
            WatchKitConnection.speed = dSpeed
            let dPower = Double(result["power"] as! Double)
            WatchKitConnection.power = dPower
            let dCadence = Double(result["cadence"] as! Double)
            WatchKitConnection.cadence = dCadence
            if let stepsDouble = result["steps"] as? Double {
                let iSteps = Int(stepsDouble)
                WatchKitConnection.steps = iSteps
            }
        }, errorHandler: { (error) in
            print(error)
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
