//
//  watchAppStart.swift
//  qdomyoszwift
//
//  Created by Roberto Viola on 30/12/2020.
//

import UIKit
import HealthKit
import WatchConnectivity

class watchAppStart: NSObject, WCSessionDelegate {
    func sessionDidBecomeInactive(_ session: WCSession) {
        
    }
    
    func sessionDidDeactivate(_ session: WCSession) {
        
    }
    
    
    // MARK: - IB Outlets
  
    @IBOutlet weak var statusLabel: UILabel!

    // MARK: - Properties
    //var workout: Workout?
    var workoutConfiguration: HKWorkoutConfiguration!
    private let healthStore = HKHealthStore()
    private var wcSessionActivationCompletion: ((WCSession) -> Void)?
    private var watchConnectivitySession: WCSession?
    private var stateDate: Date?
    
    
    
    // MARK: - UIViewController
    /*
    override func viewDidLoad() {
        super.viewDidLoad()
        workoutConfiguration = HKWorkoutConfiguration()
        workoutConfiguration.activityType = .traditionalStrengthTraining
        workoutConfiguration.locationType = .indoor
        self.navigationItem.title = workout!.name
        startWatchApp()
    }*/
    
    // MARK: - Convenience
    public func startWatchApp() {
        
        workoutConfiguration = HKWorkoutConfiguration()
        workoutConfiguration.activityType = .cycling
        workoutConfiguration.locationType = .indoor
        
        guard let workoutConfiguration = workoutConfiguration else { return }
        
        getActiveWCSession { wcSession in
            if wcSession.activationState == .activated && wcSession.isWatchAppInstalled {
                self.healthStore.startWatchApp(with: workoutConfiguration) { (success, error) in
                    if !success {
                        print("starting watch app failed with error: \(String(describing: error))")
                    }
                }
            }
        }
    }
    
    private func getActiveWCSession(completion: @escaping (WCSession) -> Void) {
        guard WCSession.isSupported() else {
            // ... Alert the user that their iOS device does not support watch connectivity
            fatalError("watch connectivity session not supported")
        }
        
        let wcSession = WCSession.default
        wcSession.delegate = self
        
        switch wcSession.activationState {
        case .activated:
            completion(wcSession)
        case .inactive, .notActivated:
            wcSession.activate()
            wcSessionActivationCompletion = completion
        }
    }
    /*
    private func updateSessionState(_ state: String) {
        if state == "ended" {
            DispatchQueue.main.async {
                self.dismiss(animated: true)
            }
        } else {
            statusLabel.text = state
        }
    }*/
    
    // MARK: - WCSessionDelegate
    
    func session(_ session: WCSession, activationDidCompleteWith activationState: WCSessionActivationState, error: Error?) {
        if activationState == .activated, let activationCompletion = wcSessionActivationCompletion {
            activationCompletion(session)
            wcSessionActivationCompletion = nil
        }
    }
}

