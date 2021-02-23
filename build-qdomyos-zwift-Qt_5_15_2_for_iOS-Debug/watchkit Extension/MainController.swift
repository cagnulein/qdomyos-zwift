//
//  MainController.swift
//  ElecDemo WatchKit Extension
//
//  Created by NhatHM on 8/12/19.
//  Copyright © 2019 GST.PID. All rights reserved.
//

import WatchKit
import HealthKit

class MainController: WKInterfaceController {
    @IBOutlet weak var userNameLabel: WKInterfaceLabel!
    @IBOutlet weak var stepCountsLabel: WKInterfaceLabel!
    @IBOutlet weak var heartRateLabel: WKInterfaceLabel!
    @IBOutlet weak var startButton: WKInterfaceButton!
    static var start: Bool! = false
    
    override func awake(withContext context: Any?) {
        super.awake(withContext: context)
        
        // Configure interface objects here.
        print("AWAKE")
    }
    
    override func willActivate() {
        // This method is called when watch view controller is about to be visible to user
        super.willActivate()
        print("WILL ACTIVE")
        WorkoutTracking.shared.fetchStepCounts()
    }
    
    override func didDeactivate() {
        // This method is called when watch view controller is no longer visible
        super.didDeactivate()
        print("DID DEACTIVE")
    }
}

extension MainController {
    
    @IBAction func startWorkout() {
        if(!MainController.start){
            MainController.start = true
            startButton.setTitle("Stop")
            WorkoutTracking.authorizeHealthKit()
            WorkoutTracking.shared.startWorkOut()
            WorkoutTracking.shared.delegate = self
            
            WatchKitConnection.shared.delegate = self
            WatchKitConnection.shared.startSession()
        }
        else {
            MainController.start = false
            startButton.setTitle("Start")
            WorkoutTracking.shared.stopWorkOut()
        }
    }
}

extension MainController: WorkoutTrackingDelegate {
    func didReceiveHealthKitHeartRate(_ heartRate: Double) {
        heartRateLabel.setText("\(heartRate) BPM")
        WatchKitConnection.shared.sendMessage(message: ["heartRate":
            "\(heartRate)" as AnyObject])
    }
    
    func didReceiveHealthKitStepCounts(_ stepCounts: Double) {
        stepCountsLabel.setText("\(stepCounts) STEPS")
    }
}

extension MainController: WatchKitConnectionDelegate {
    func didReceiveUserName(_ userName: String) {
        userNameLabel.setText(userName)
    }
}
