//
//  WatchWorkoutTracking.swift
//  ElecDemo WatchKit Extension
//
//  Created by NhatHM on 8/12/19.
//  Copyright © 2019 GST.PID. All rights reserved.
//

import Foundation
import HealthKit

let SwiftDebug = swiftDebug()

protocol WorkoutTrackingDelegate: class {
    func didReceiveHealthKitHeartRate(_ heartRate: Double)
    func didReceiveHealthKitStepCounts(_ stepCounts: Double)
    func didReceiveHealthKitStepCadence(_ stepCadence: Double)
    func didReceiveHealthKitDistanceCycling(_ distanceCycling: Double)
    func didReceiveHealthKitActiveEnergyBurned(_ activeEnergyBurned: Double)
}

protocol WorkoutTrackingProtocol {
    static func authorizeHealthKit()
    func startWorkOut(deviceType: UInt16)
    func stopWorkOut()
}

@available(iOS 17.0, *)
@objc class WorkoutTracking: NSObject {
    static let shared = WorkoutTracking()
    public static var lastDateMetric = Date()
    public static var distance = Double()
    public static var kcal = Double()
    var sport: Int = 0
    let healthStore = HKHealthStore()
    let configuration = HKWorkoutConfiguration()
    var workoutBuilder: HKWorkoutBuilder!
    var workoutInProgress: Bool = false
    
    weak var delegate: WorkoutTrackingDelegate?
    
    override init() {
        super.init()
    }        
}

@available(iOS 17.0, *)
extension WorkoutTracking {
    func setSport(_ sport: Int) {
        self.sport = sport
    }
    
    private func configWorkout() {
        var activityType = HKWorkoutActivityType.cycling
        if self.sport == 1 {
            activityType = HKWorkoutActivityType.running
        } else if self.sport == 2 {
            activityType = HKWorkoutActivityType.cycling
        } else if self.sport == 3 {
            activityType = HKWorkoutActivityType.rowing
        } else if self.sport == 4 {
            activityType = HKWorkoutActivityType.elliptical
        }
        
        configuration.activityType = activityType
        configuration.locationType = .indoor
        
        do {
            workoutBuilder = try HKWorkoutBuilder(healthStore: healthStore, configuration: configuration, device: .local())
        } catch {
            return
        }
    }
}

@available(iOS 17.0, *)
extension WorkoutTracking: WorkoutTrackingProtocol {
    
    @objc static func requestAuth() {
        authorizeHealthKit()
    }
    
    @objc public static func authorizeHealthKit() {
        if HKHealthStore.isHealthDataAvailable() {
            let infoToRead = Set([
                HKSampleType.quantityType(forIdentifier: .stepCount)!,
                HKSampleType.quantityType(forIdentifier: .heartRate)!,
                /*HKSampleType.quantityType(forIdentifier: .distanceCycling)!,
                HKSampleType.quantityType(forIdentifier: .activeEnergyBurned)!,*/
                HKSampleType.workoutType()
                ])
            
            var infoToShare: Set<HKSampleType> = []
            
            if #available(watchOSApplicationExtension 10.0, *) {
                infoToShare = Set([
                    HKSampleType.quantityType(forIdentifier: .stepCount)!,
                    HKSampleType.quantityType(forIdentifier: .heartRate)!,
                    HKSampleType.quantityType(forIdentifier: .distanceCycling)!,
                    HKSampleType.quantityType(forIdentifier: .distanceWalkingRunning)!,
                    HKSampleType.quantityType(forIdentifier: .activeEnergyBurned)!,
                    HKSampleType.quantityType(forIdentifier: .cyclingPower)!,
                    HKSampleType.quantityType(forIdentifier: .cyclingSpeed)!,
                    HKSampleType.quantityType(forIdentifier: .cyclingCadence)!,
                    HKSampleType.quantityType(forIdentifier: .runningPower)!,
                    HKSampleType.quantityType(forIdentifier: .runningSpeed)!,
                    HKSampleType.quantityType(forIdentifier: .runningStrideLength)!,
                    HKSampleType.quantityType(forIdentifier: .runningVerticalOscillation)!,
                    HKSampleType.quantityType(forIdentifier: .walkingSpeed)!,
                    HKSampleType.quantityType(forIdentifier: .walkingStepLength)!,
                    HKSampleType.workoutType()
                    ])
            } else {
                // Fallback on earlier versions
                infoToShare = Set([
                    HKSampleType.quantityType(forIdentifier: .stepCount)!,
                    HKSampleType.quantityType(forIdentifier: .heartRate)!,
                    HKSampleType.quantityType(forIdentifier: .distanceCycling)!,
                    HKSampleType.quantityType(forIdentifier: .distanceWalkingRunning)!,
                    HKSampleType.quantityType(forIdentifier: .activeEnergyBurned)!,
                    HKSampleType.workoutType()
                    ])
            }
            
            DispatchQueue.main.async {
                HKHealthStore().requestAuthorization(toShare: infoToShare, read: infoToRead) { (success, error) in
                    if success {
                        SwiftDebug.qtDebug("WorkoutTracking: Authorization healthkit success")
                    } else if let error = error {
                        SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                    }
                }
            }
            
        } else {
            SwiftDebug.qtDebug("WorkoutTracking: HealthKit not avaiable")            
        }
    }
    
    @objc func startWorkOut(deviceType: UInt16) {
        if(workoutInProgress) {
            return;
        }
        WorkoutTracking.authorizeHealthKit()
        workoutInProgress = true;
        WorkoutTracking.lastDateMetric = Date()
        SwiftDebug.qtDebug("WorkoutTracking: Start workout")
        setSport(Int(deviceType))
        configWorkout()
        workoutBuilder.beginCollection(withStart: Date()) { (success, error) in
            SwiftDebug.qtDebug(success.description)
            if let error = error {
                SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
            }
        }
    }
    
    @objc func stopWorkOut() {
        SwiftDebug.qtDebug("WorkoutTracking: Stop workout")
        
        guard let quantityType = HKQuantityType.quantityType(
          forIdentifier: .activeEnergyBurned) else {
          return
        }
            
        let unit = HKUnit.kilocalorie()
        let totalEnergyBurned = WorkoutTracking.kcal
        let quantity = HKQuantity(unit: unit,
                                  doubleValue: totalEnergyBurned)
        
        let sample = HKCumulativeQuantitySeriesSample(type: quantityType,
                                                      quantity: quantity,
                                                      start: workoutBuilder.startDate!,
                                                      end: Date())
        
        workoutBuilder.add([sample]) {(success, error) in}
            
        let unitDistance = HKUnit.mile()
        let miles = WorkoutTracking.distance
        let quantityMiles = HKQuantity(unit: unitDistance,
                                  doubleValue: miles)
        
        if(sport == 2) {
            
            guard let quantityTypeDistance = HKQuantityType.quantityType(
                    forIdentifier: .distanceCycling) else {
              return
            }
            
            
            let sampleDistance = HKCumulativeQuantitySeriesSample(type: quantityTypeDistance,
                                                          quantity: quantityMiles,
                                                          start: workoutBuilder.startDate!,
                                                          end: Date())
            
            workoutBuilder.add([sampleDistance]) {(success, error) in
                if let error = error {
                    SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                }
            }
                self.workoutBuilder.endCollection(withEnd: Date()) { (success, error) in
                    if let error = error {
                        SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                    }
                    self.workoutBuilder.finishWorkout{ (workout, error) in
                        if let error = error {
                            SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                        }
                        workout?.setValue(quantityMiles, forKey: "totalDistance")
                    }
                }
        } else {
            
            // Guard to check if steps quantity type is available
            guard let quantityTypeSteps = HKQuantityType.quantityType(
                forIdentifier: .stepCount) else {
                return
            }

            let stepsQuantity = HKQuantity(unit: HKUnit.count(), doubleValue: Double(WorkoutTracking.steps))
            
            // Create a sample for total steps
            let sampleSteps = HKCumulativeQuantitySeriesSample(
                type: quantityTypeSteps,
                quantity: stepsQuantity,  // Use your steps quantity here
                start: workoutSession.startDate!,
                end: Date())

            // Add the steps sample to workout builder
            workoutBuilder.add([sampleSteps]) { (success, error) in
                if let error = error {
                    print(error)
                }
                
                // End the data collection
                self.workoutBuilder.endCollection(withEnd: Date()) { (success, error) in
                    if let error = error {
                        print(error)
                    }
                    
                    // Finish the workout and save total steps
                    self.workoutBuilder.finishWorkout { (workout, error) in
                        if let error = error {
                            print(error)
                        }
                        workout?.setValue(stepsQuantity, forKey: "totalSteps")
                    }
                }
            }

            guard let quantityTypeDistance = HKQuantityType.quantityType(
                forIdentifier: .distanceWalkingRunning) else {
              return
            }
      
            let sampleDistance = HKCumulativeQuantitySeriesSample(type: quantityTypeDistance,
                                                          quantity: quantityMiles,
                                                          start: workoutBuilder.startDate!,
                                                          end: Date())
            
            workoutBuilder.add([sampleDistance]) {(success, error) in
                if let error = error {
                    SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                }
                self.workoutBuilder.endCollection(withEnd: Date()) { (success, error) in
                    if let error = error {
                        SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                    }
                    self.workoutBuilder.finishWorkout{ (workout, error) in
                        if let error = error {
                            SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                        }
                        workout?.setValue(quantityMiles, forKey: "totalDistance")
                    }
                }
            }
        }
        
        workoutInProgress = false;
    }
    
    @objc func addMetrics(power: Double, cadence: Double, speed: Double, kcal: Double, steps: Double, deviceType: UInt8) {
        SwiftDebug.qtDebug("WorkoutTracking: GET DATA: \(Date())")
        
        if(workoutInProgress == false && power > 0) {
            startWorkOut(deviceType: deviceType)
        } else if(workoutInProgress == false && power == 0) {
            return;
        }

        let Speed = speed / 100;
        
        WorkoutTracking.kcal = kcal
        WorkoutTracking.steps = steps

        if(sport == 2) {
            if #available(watchOSApplicationExtension 10.0, *) {
                let wattPerInterval = HKQuantity(unit: HKUnit.watt(),
                                                doubleValue: power)
                
                if(WorkoutTracking.lastDateMetric.distance(to: Date()) < 1) {
                    return
                }
                
                guard let powerType = HKQuantityType.quantityType(
                    forIdentifier: .cyclingPower) else {
                return
                }
                let wattPerIntervalSample = HKQuantitySample(type: powerType,
                                                                quantity: wattPerInterval,
                                                            start: WorkoutTracking.lastDateMetric,
                                                            end: Date())
                workoutBuilder.add([wattPerIntervalSample]) {(success, error) in
                    if let error = error {
                        SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                    }
                }

                let cadencePerInterval = HKQuantity(unit: HKUnit.count().unitDivided(by: HKUnit.second()),
                                                    doubleValue: cadence / 60.0)
                
                guard let cadenceType = HKQuantityType.quantityType(
                    forIdentifier: .cyclingCadence) else {
                return
                }
                let cadencePerIntervalSample = HKQuantitySample(type: cadenceType,
                                                                quantity: cadencePerInterval,
                                                            start: WorkoutTracking.lastDateMetric,
                                                            end: Date())
                workoutBuilder.add([cadencePerIntervalSample]) {(success, error) in
                    if success {
                        SwiftDebug.qtDebug("WorkoutTracking: OK")
                    }
                    if let error = error {
                        SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                    }
                }
                
                let speedPerInterval = HKQuantity(unit: HKUnit.meter().unitDivided(by: HKUnit.second()),
                                                  doubleValue: (Speed / 3.6))
                
                guard let speedType = HKQuantityType.quantityType(
                    forIdentifier: .cyclingSpeed) else {
                return
                }
                let speedPerIntervalSample = HKQuantitySample(type: speedType,
                                                                quantity: speedPerInterval,
                                                            start: WorkoutTracking.lastDateMetric,
                                                            end: Date())
                workoutBuilder.add([speedPerIntervalSample]) {(success, error) in
                    if let error = error {
                        SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                    }
                }

            } else {
                // Fallback on earlier versions
            }
        } else if(sport == 1) {
            if #available(watchOSApplicationExtension 10.0, *) {
                // Guard to check if steps quantity type is available
                guard let quantityTypeSteps = HKQuantityType.quantityType(
                    forIdentifier: .stepCount) else {
                    return
                }

                let stepsQuantity = HKQuantity(unit: HKUnit.count(), doubleValue: Double(WorkoutTracking.steps))
                
                // Create a sample for total steps
                let sampleSteps = HKCumulativeQuantitySeriesSample(
                    type: quantityTypeSteps,
                    quantity: stepsQuantity,  // Use your steps quantity here
                    start: workoutSession.startDate!,
                    end: Date())

                // Add the steps sample to workout builder
                workoutBuilder.add([sampleSteps]) { (success, error) in
                    if let error = error {
                        print(error)
                    }                    
                }

                let wattPerInterval = HKQuantity(unit: HKUnit.watt(),
                                                doubleValue: power)
                
                if(WorkoutTracking.lastDateMetric.distance(to: Date()) < 1) {
                    return
                }
                
                guard let powerType = HKQuantityType.quantityType(
                    forIdentifier: .runningPower) else {
                return
                }
                let wattPerIntervalSample = HKQuantitySample(type: powerType,
                                                                quantity: wattPerInterval,
                                                            start: WorkoutTracking.lastDateMetric,
                                                            end: Date())
                workoutBuilder.add([wattPerIntervalSample]) {(success, error) in
                    if let error = error {
                        SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                    }
                }
            
                let speedPerInterval = HKQuantity(unit: HKUnit.meter().unitDivided(by: HKUnit.second()),
                                                doubleValue: Speed * 0.277778)
                
                guard let speedType = HKQuantityType.quantityType(
                    forIdentifier: .runningSpeed) else {
                return
                }
                let speedPerIntervalSample = HKQuantitySample(type: speedType,
                                                                quantity: speedPerInterval,
                                                            start: WorkoutTracking.lastDateMetric,
                                                            end: Date())
                workoutBuilder.add([speedPerIntervalSample]) {(success, error) in
                    if let error = error {
                        SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                    }
                }

            } else {
                // Fallback on earlier versions
            }
        }/* else if(sport == 2) {
            if #available(watchOSApplicationExtension 10.0, *) {
                let speedPerInterval = HKQuantity(unit: HKUnit.meter().unitDivided(by: HKUnit.second()),
                                                doubleValue: Speed * 0.277778)
                
                guard let speedType = HKQuantityType.quantityType(
                    forIdentifier: .walkingSpeed) else {
                return
                }
                let speedPerIntervalSample = HKQuantitySample(type: speedType,
                                                                quantity: speedPerInterval,
                                                            start: WorkoutTracking.lastDateMetric,
                                                            end: Date())
                workoutBuilder.add([speedPerIntervalSample]) {(success, error) in
                    if let error = error {
                        SwiftDebug.qtDebug("WorkoutTracking: " + error)
                    }
                }

            } else {
                // Fallback on earlier versions
            }
        }*/
        
        // TODO HANDLE WALKING, ROWING AND ELLIPTICAL
        
        WorkoutTracking.lastDateMetric = Date()
    }
}

