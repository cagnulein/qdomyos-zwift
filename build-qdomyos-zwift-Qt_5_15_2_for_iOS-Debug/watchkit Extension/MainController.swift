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
    private static weak var activeController: MainController?
    private static var autoSyncActive = false
    private static var syncedWorkoutState = 3
    private var syncTimer: Timer?
    let pedometer = CMPedometer()
    var sport: Int = 0
    
    override func awake(withContext context: Any?) {
        super.awake(withContext: context)
        MainController.activeController = self
        let sports: [WKPickerItem] = [WKPickerItem(),WKPickerItem(),WKPickerItem(),WKPickerItem(),WKPickerItem()]
        sports[0].title = "Bike"
        sports[1].title = "Run"
        sports[2].title = "Walk"
        sports[3].title = "Elliptical"
        sports[4].title = "Rowing"
        cmbSports.setItems(sports)
        sport = UserDefaults.standard.value(forKey: "sport") as? Int ?? 0
        cmbSports.setSelectedItemIndex(sport)

        WatchKitConnection.shared.delegate = self
        WatchKitConnection.shared.startSession()
        syncTimer?.invalidate()
        syncTimer = Timer.scheduledTimer(timeInterval: 1.0, target: self, selector: #selector(syncWithQZ), userInfo: nil, repeats: true)
        
        // Configure interface objects here.
        print("AWAKE")
    }

    @objc private func syncWithQZ() {
        WatchKitConnection.shared.sendMessage(message: ["ping": "0" as AnyObject])
    }
    
    @IBAction func changeSport(_ value: Int) {
        sport = value
        UserDefaults.standard.set(value, forKey: "sport")
        UserDefaults.standard.synchronize()
    }
    
    override func willActivate() {
        // This method is called when watch view controller is about to be visible to user
        super.willActivate()
        MainController.activeController = self
        WatchKitConnection.shared.delegate = self
        WatchKitConnection.shared.startSession()
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
    }

    private func beginWorkout(_ selectedSport: Int) {
        guard !MainController.start else { return }
        sport = selectedSport
        cmbSports.setSelectedItemIndex(selectedSport)
        MainController.start = true
        startButton.setTitle("Stop")
        cmbSports.setEnabled(false)
        cmbSports.setHidden(true)
        WorkoutTracking.authorizeHealthKit()
        WorkoutTracking.shared.setSport(selectedSport)
        WorkoutTracking.shared.delegate = self
        WatchKitConnection.shared.delegate = self
        WatchKitConnection.shared.startSession()
        WorkoutTracking.shared.startWorkOut()
    }

    private func endWorkout() {
        guard MainController.start else { return }
        MainController.start = false
        startButton.setTitle("Start")
        cmbSports.setEnabled(true)
        cmbSports.setHidden(false)
        WorkoutTracking.shared.stopWorkOut()
    }

    private static func watchSport(for deviceType: Int) -> Int {
        switch deviceType {
        case 1: // treadmill -> running
            return 1
        case 2: // bike
            return 0
        case 3: // rower
            return 4
        case 4: // elliptical
            return 3
        default:
            return 0
        }
    }

    static func syncWorkoutState(_ workoutState: Int, deviceType: Int) {
        DispatchQueue.main.async {
            guard let controller = MainController.activeController else { return }

            switch workoutState {
            case 0: // STARTED
                MainController.autoSyncActive = true
                if !MainController.start && WatchKitConnection.speed > 0 {
                    controller.beginWorkout(MainController.watchSport(for: deviceType))
                }
                if MainController.start {
                    MainController.syncedWorkoutState = 0
                }

            case 1: // PAUSED
                MainController.autoSyncActive = true
                if MainController.start && MainController.syncedWorkoutState != 1 {
                    WorkoutTracking.shared.workoutSession?.pause()
                }
                MainController.syncedWorkoutState = 1

            case 2: // RESUMED
                MainController.autoSyncActive = true
                if !MainController.start && WatchKitConnection.speed > 0 {
                    controller.beginWorkout(MainController.watchSport(for: deviceType))
                }
                if MainController.start && MainController.syncedWorkoutState != 2 {
                    WorkoutTracking.shared.workoutSession?.resume()
                }
                MainController.syncedWorkoutState = 2

            case 3: // STOPPED
                if MainController.autoSyncActive {
                    controller.endWorkout()
                    MainController.autoSyncActive = false
                }
                MainController.syncedWorkoutState = 3

            default:
                break
            }
        }
    }
}

extension MainController {
    
    @IBAction func startWorkout() {
        if(!MainController.start){
            let selectedSport = sport
            let workoutName = ["Bike", "Run", "Walk", "Elliptical", "Rowing"][selectedSport]
            let startAction = WKAlertAction(title: "Start", style: .default) { [weak self] in
                guard let self = self else { return }
                self.beginWorkout(selectedSport)
            }
            let cancelAction = WKAlertAction(title: "Cancel", style: .cancel) {}
            presentAlert(withTitle: "Start Workout", message: "Start \(workoutName) workout?", preferredStyle: .alert, actions: [cancelAction, startAction])
        }
        else {
            endWorkout()
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
        WorkoutTracking.totalKcal = WatchKitConnection.totalKcal
        WorkoutTracking.speed = WatchKitConnection.speed
        WorkoutTracking.power = WatchKitConnection.power
        WorkoutTracking.cadence = WatchKitConnection.cadence
        WorkoutTracking.steps = WatchKitConnection.steps
                
        if Locale.current.measurementSystem != "Metric" {
            self.distanceLabel.setText("Distance \(String(format:"%.2f", WorkoutTracking.distance * 0.621371)) mi")
        } else {
            self.distanceLabel.setText("Distance \(String(format:"%.2f", WorkoutTracking.distance)) km")
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
