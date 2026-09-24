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
    private static var workoutIsActive = false
    private static var workoutIsStarting = false
    private static weak var activeController: MainController?
    private static var autoSyncActive = false
    private static var syncedWorkoutState = 3
    private var syncTimer: Timer?
    let pedometer = CMPedometer()
    
    override func awake(withContext context: Any?) {
        super.awake(withContext: context)
        MainController.activeController = self
        userNameLabel.setText("Waiting for QZ")

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
        guard !MainController.workoutIsActive, !MainController.workoutIsStarting else { return }
        MainController.workoutIsStarting = true
        userNameLabel.setText("Preparing workout")

        WorkoutTracking.authorizeHealthKit { [weak self] authorized in
            DispatchQueue.main.async {
                guard let self = self else { return }
                MainController.workoutIsStarting = false

                guard MainController.autoSyncActive,
                      WatchKitConnection.workoutState == 0 || WatchKitConnection.workoutState == 2 else {
                    return
                }
                guard authorized else {
                    MainController.autoSyncActive = false
                    MainController.syncedWorkoutState = 3
                    self.userNameLabel.setText("Health unavailable")
                    return
                }

                WorkoutTracking.shared.setSport(selectedSport)
                WorkoutTracking.shared.delegate = self
                WatchKitConnection.shared.delegate = self
                WatchKitConnection.shared.startSession()
                guard WorkoutTracking.shared.startWorkOut() else {
                    MainController.autoSyncActive = false
                    MainController.syncedWorkoutState = 3
                    self.userNameLabel.setText("Workout unavailable")
                    return
                }
                MainController.workoutIsActive = true
                self.userNameLabel.setText("Workout active")
            }
        }
    }

    private func endWorkout() {
        guard MainController.workoutIsActive else { return }
        MainController.workoutIsActive = false
        userNameLabel.setText("Saving workout…")
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

    private static func equipmentIsMoving(_ deviceType: Int) -> Bool {
        // A bike in ERG mode can report target power before speed has risen.
        WatchKitConnection.speed > 0 || (deviceType == 2 && WatchKitConnection.power > 0)
    }

    static func syncWorkoutState(_ workoutState: Int, deviceType: Int) {
        DispatchQueue.main.async {
            guard let controller = MainController.activeController else { return }

            switch workoutState {
            case 0: // STARTED
                MainController.autoSyncActive = true
                if !MainController.workoutIsActive && MainController.equipmentIsMoving(deviceType) {
                    controller.beginWorkout(MainController.watchSport(for: deviceType))
                }
                MainController.syncedWorkoutState = 0

            case 1: // PAUSED
                MainController.autoSyncActive = true
                if MainController.workoutIsActive && MainController.syncedWorkoutState != 1 {
                    WorkoutTracking.shared.workoutSession?.pause()
                }
                MainController.syncedWorkoutState = 1

            case 2: // RESUMED
                MainController.autoSyncActive = true
                if !MainController.workoutIsActive && MainController.equipmentIsMoving(deviceType) {
                    controller.beginWorkout(MainController.watchSport(for: deviceType))
                }
                if MainController.workoutIsActive && MainController.syncedWorkoutState != 2 {
                    WorkoutTracking.shared.workoutSession?.resume()
                }
                MainController.syncedWorkoutState = 2

            case 3: // STOPPED
                MainController.workoutIsStarting = false
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

extension MainController: WorkoutTrackingDelegate {
    
    func didReceiveHealthKitDistanceCycling(_ distanceCycling: Double) {
        
    }
    func didReceiveHealthKitActiveEnergyBurned(_ activeEnergyBurned: Double) {
        
    }

    func didSaveHealthKitWorkout(_ success: Bool, error: String?) {
        DispatchQueue.main.async {
            if success {
                self.userNameLabel.setText("Workout saved")
            } else {
                self.userNameLabel.setText("Save failed")
                WatchKitConnection.shared.sendDebug("HealthKit workout save failed: \(error ?? "unknown error")")
            }
        }
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
