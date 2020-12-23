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
    static let shared = WatchKitConnection()
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
    }
    
    func session(_ session: WCSession, didReceiveMessage message: [String : Any], replyHandler: @escaping ([String : Any]) -> Void) {
        print("didReceiveMessage with reply")
        print(message)
        guard let heartReate = message.values.first as? String else {
            return
        }
        guard let heartReateDouble = Double(heartReate) else {
            return
        }
        LocalNotificationHelper.fireHeartRate(heartReateDouble)
    }
}
