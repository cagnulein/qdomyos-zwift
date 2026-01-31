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
    static var start: Bool! = false
    let pedometer = CMPedometer()
    var sport: Int = 0
    
    override func awake(withContext context: Any?) {
        super.awake(withContext: context)
        let sports: [WKPickerItem] = [WKPickerItem(),WKPickerItem(),WKPickerItem(),WKPickerItem(),WKPickerItem()]
        sports[0].title = "Bike"
        sports[1].title = "Run"
        sports[2].title = "Walk"
        sports[3].title = "Elliptical"
        sports[4].title = "Rowing"
        cmbSports.setItems(sports)
        sport = UserDefaults.standard.value(forKey: "sport") as? Int ?? 0
        cmbSports.setSelectedItemIndex(sport)
        
        // Configure interface objects here.
        print("AWAKE")
    }
    
    @IBAction func changeSport(_ value: Int) {
        self.sport = value
        UserDefaults.standard.set(value, forKey: "sport")
        UserDefaults.standard.synchronize()
    }
    
    override func willActivate() {
        // This method is called when watch view controller is about to be visible to user
        super.willActivate()
        print("WILL ACTIVE")
        WorkoutTracking.shared.fetchStepCounts()
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
        // Stop pedometer updates when view is not visible to save battery
        pedometer.stopUpdates()
    }
}

extension MainController {
    
    @IBAction func startWorkout() {
        if(!MainController.start){
            MainController.start = true
            startButton.setTitle("Stop")
            WorkoutTracking.authorizeHealthKit()
            WorkoutTracking.shared.setSport(sport)
            WorkoutTracking.shared.startWorkOut()
            WorkoutTracking.shared.delegate = self
            
            WatchKitConnection.shared.delegate = self
            WatchKitConnection.shared.startSession()
        }
        else {
            MainController.start = false
            startButton.setTitle("Start")
            WorkoutTracking.shared.stopWorkOut()
            // Stop pedometer updates when workout ends to save battery
            pedometer.stopUpdates()
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
        WorkoutTracking.steps = WatchKitConnection.steps
                
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
