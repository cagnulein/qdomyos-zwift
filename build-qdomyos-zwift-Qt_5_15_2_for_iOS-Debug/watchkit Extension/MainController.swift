//
//  MainController.swift
//  ElecDemo WatchKit Extension
//
//  Created by NhatHM on 8/12/19.
//  Copyright © 2019 GST.PID. All rights reserved.
//

import WatchKit
import HealthKit
import CoreMotion

class MainController: WKInterfaceController {
    @IBOutlet weak var userNameLabel: WKInterfaceLabel!
    @IBOutlet weak var stepCountsLabel: WKInterfaceLabel!
    @IBOutlet weak var caloriesLabel: WKInterfaceLabel!
    @IBOutlet weak var distanceLabel: WKInterfaceLabel!
    @IBOutlet weak var heartRateLabel: WKInterfaceLabel!
    @IBOutlet weak var startButton: WKInterfaceButton!
    @IBOutlet weak var cmbSports: WKInterfacePicker!
    let pedometer = CMPedometer()

    //enum WORKOUT_EVENT_STATE { STARTED = 0, PAUSED = 1, RESUMED = 2, STOPPED = 3 };
    public static var workout_state = 3;

    override func awake(withContext context: Any?) {
        super.awake(withContext: context)
        
        WatchKitConnection.shared.delegate = self
        WatchKitConnection.shared.startSession()
        
        // Configure interface objects here.
        print("AWAKE")
    }
    
    override func willActivate() {
        // This method is called when watch view controller is about to be visible to user
        super.willActivate()
        print("WILL ACTIVE")
        WorkoutTracking.shared.fetchStepCounts()
        WorkoutTracking.shared.delegate = self

        if CMPedometer.isStepCountingAvailable() {
            pedometer.startUpdates(from: Date()) { pedometerData, error in
                guard let pedometerData = pedometerData, error == nil else { return }
                self.stepCountsLabel.setText("\(Int(((pedometerData.currentCadence?.doubleValue ?? 0) * 60.0 / 2.0))) STEP CAD.")
                WatchKitConnection.stepCadence = Int(((pedometerData.currentCadence?.doubleValue ?? 0) * 60.0 / 2.0))
                WatchKitConnection.shared.sendMessage(message: ["stepCadence":
                    "\(WatchKitConnection.stepCadence)" as AnyObject])
            }
        }
    }
    
    override func didDeactivate() {
        // This method is called when watch view controller is no longer visible
        super.didDeactivate()
        print("DID DEACTIVE")
    }
}

extension MainController {
    public static func syncWorkoutState(state: Int) {
        if(state != workout_state) {
            switch state {
            case 0:
                WorkoutTracking.authorizeHealthKit()
                WorkoutTracking.shared.startWorkOut()

            case 1:
                WorkoutTracking.shared.pauseWorkout()

            case 2:
                WorkoutTracking.shared.resumeWorkout()

            case 3:
                WorkoutTracking.shared.stopWorkOut()
            default:
                print("error!")
            }
            workout_state = state
        }
    }
}

extension MainController: WorkoutTrackingDelegate {
    
    func didReceiveHealthKitDistanceCycling(_ distanceCycling: Double) {
        
    }
    func didReceiveHealthKitActiveEnergyBurned(_ activeEnergyBurned: Double) {
        
    }
    
    func didReceiveHealthKitHeartRate(_ heartRate: Double) {
        heartRateLabel.setText("\(heartRate) BPM")
        WatchKitConnection.shared.sendMessage(message: ["heartRate":
            "\(heartRate)" as AnyObject])
        WorkoutTracking.distance = WatchKitConnection.distance
        WorkoutTracking.kcal = WatchKitConnection.kcal
        WorkoutTracking.speed = WatchKitConnection.speed
        WorkoutTracking.power = WatchKitConnection.power
        WorkoutTracking.cadence = WatchKitConnection.cadence
        WorkoutTracking.workout_state = WatchKitConnection.workout_state
                
		if Locale.current.measurementSystem != "Metric" {
			self.distanceLabel.setText("Distance \(String(format:"%.2f", WorkoutTracking.distance))")
		} else {
			self.distanceLabel.setText("Distance \(String(format:"%.2f", WorkoutTracking.distance * 1.60934))")
		}
        self.caloriesLabel.setText("KCal \(Int(WorkoutTracking.kcal))")
        //WorkoutTracking.cadenceSteps = pedometer.
    }
    
    func didReceiveHealthKitStepCounts(_ stepCounts: Double) {
        //stepCountsLabel.setText("\(stepCounts) STEPS")
    }
    func didReceiveHealthKitStepCadence(_ stepCadence: Double) {
        
    }
}

extension MainController: WatchKitConnectionDelegate {
    func didReceiveUserName(_ userName: String) {
        userNameLabel.setText(userName)
    }
}

extension Locale
{
   var measurementSystem : String?
   {
      return (self as NSLocale).object(forKey: NSLocale.Key.measurementSystem) as? String
   }
}
