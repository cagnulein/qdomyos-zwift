//
//  watchAppStart.swift
//  qdomyoszwift
//
//  Created by Roberto Viola on 30/12/2020.
//

import UIKit
import HealthKit
import WatchConnectivity

class watchAppStart: NSObject {
    // MARK: - IB Outlets
  
    @IBOutlet weak var statusLabel: UILabel!

    // MARK: - Properties
    //var workout: Workout?
    var workoutConfiguration: HKWorkoutConfiguration!
    private let healthStore = HKHealthStore()
    private var watchAppLaunchRequested = false
    
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
    public func startWatchApp(deviceType: Int) {
        guard !watchAppLaunchRequested,
              WCSession.isSupported(),
              WCSession.default.activationState == .activated,
              WCSession.default.isPaired,
              WCSession.default.isWatchAppInstalled else { return }

        workoutConfiguration = HKWorkoutConfiguration()
        switch deviceType {
        case 1: // treadmill
            workoutConfiguration.activityType = .running
        case 2: // bike
            workoutConfiguration.activityType = .cycling
        case 3: // rower
            workoutConfiguration.activityType = .rowing
        case 4: // elliptical
            workoutConfiguration.activityType = .elliptical
        default:
            workoutConfiguration.activityType = .other
        }
        workoutConfiguration.locationType = .indoor

        watchAppLaunchRequested = true
        healthStore.startWatchApp(with: workoutConfiguration) { [weak self] (success, error) in
            if !success {
                self?.watchAppLaunchRequested = false
                print("starting watch app failed with error: \(String(describing: error))")
            }
        }
    }

    public func resetWorkout() {
        watchAppLaunchRequested = false
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
    
}
