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
    public static var totalKcal = Double()
    public static var steps = Double()
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
                    HKSampleType.quantityType(forIdentifier: .basalEnergyBurned)!,
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
                    HKSampleType.quantityType(forIdentifier: .basalEnergyBurned)!,
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
        
        guard let workoutBuilder = self.workoutBuilder,
              let startDate = workoutBuilder.startDate else {
            SwiftDebug.qtDebug("WorkoutTracking: Cannot stop workout - no workout builder or start date available")
            workoutInProgress = false
            return
        }
        
        // Write active calories
        guard let activeQuantityType = HKQuantityType.quantityType(
          forIdentifier: .activeEnergyBurned) else {
          return
        }
            
        let unit = HKUnit.kilocalorie()
        let activeEnergyBurned = WorkoutTracking.kcal
        let activeQuantity = HKQuantity(unit: unit,
                                       doubleValue: activeEnergyBurned)
        
        let activeSample = HKCumulativeQuantitySeriesSample(type: activeQuantityType,
                                                           quantity: activeQuantity,
                                                           start: startDate,
                                                           end: Date())
        
        workoutBuilder.add([activeSample]) {(success, error) in
            if let error = error {
                SwiftDebug.qtDebug("WorkoutTracking active calories: " + error.localizedDescription)
            }
        }
        
        // Write total calories if available (when active calories setting is enabled)
        if WorkoutTracking.totalKcal > 0 {
            guard let basalQuantityType = HKQuantityType.quantityType(
              forIdentifier: .basalEnergyBurned) else {
              return
            }
            
            // Calculate basal calories as difference between total and active
            let basalEnergyBurned = WorkoutTracking.totalKcal - activeEnergyBurned
            let basalQuantity = HKQuantity(unit: unit,
                                          doubleValue: basalEnergyBurned)
            
            let basalSample = HKCumulativeQuantitySeriesSample(type: basalQuantityType,
                                                              quantity: basalQuantity,
                                                              start: startDate,
                                                              end: Date())
            
            workoutBuilder.add([basalSample]) {(success, error) in
                if let error = error {
                    SwiftDebug.qtDebug("WorkoutTracking basal calories: " + error.localizedDescription)
                }
            }
        }
            
        let unitDistance = HKUnit.mile()
        let miles = WorkoutTracking.distance * 0.000621371
        let quantityMiles = HKQuantity(unit: unitDistance,
                                  doubleValue: miles)
        
        if(sport == 2) {
            
            guard let quantityTypeDistance = HKQuantityType.quantityType(
                    forIdentifier: .distanceCycling) else {
              return
            }
            
            
            let sampleDistance = HKCumulativeQuantitySeriesSample(type: quantityTypeDistance,
                                                          quantity: quantityMiles,
                                                          start: startDate,
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
                        // Set total energy burned on the workout
                        let totalEnergy = WorkoutTracking.totalKcal > 0 ? WorkoutTracking.totalKcal : activeEnergyBurned
                        let totalEnergyQuantity = HKQuantity(unit: unit, doubleValue: totalEnergy)
                        workout?.setValue(totalEnergyQuantity, forKey: "totalEnergyBurned")
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
                start: startDate,
                end: Date())

            // Guard to check if distance quantity type is available
            guard let quantityTypeDistance = HKQuantityType.quantityType(
                forIdentifier: .distanceWalkingRunning) else {
                return
            }

            let sampleDistance = HKCumulativeQuantitySeriesSample(
                type: quantityTypeDistance,
                quantity: quantityMiles,
                start: startDate,
                end: Date())

            // Add both samples to the workout builder
            workoutBuilder.add([sampleSteps, sampleDistance]) { (success, error) in
                if let error = error {
                    print(error)
                    SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                    return
                }
                
                // End the data collection - only do this once
                self.workoutBuilder.endCollection(withEnd: Date()) { (success, error) in
                    if let error = error {
                        print(error)
                        SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                        return
                    }
                    
                    // Finish the workout - only do this once
                    self.workoutBuilder.finishWorkout { (workout, error) in
                        if let error = error {
                            print(error)
                            SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                            return
                        }
                        
                        // Set total energy burned on the workout
                        let totalEnergy = WorkoutTracking.totalKcal > 0 ? WorkoutTracking.totalKcal : activeEnergyBurned
                        let totalEnergyQuantity = HKQuantity(unit: unit, doubleValue: totalEnergy)
                        workout?.setValue(totalEnergyQuantity, forKey: "totalEnergyBurned")
                    }
                }
            }
        }
        
        workoutInProgress = false;
    }
    
    @objc func addMetrics(power: Double, cadence: Double, speed: Double, kcal: Double, steps: Double, deviceType: UInt8, distance: Double) {
        SwiftDebug.qtDebug("WorkoutTracking: GET DATA: \(Date())")
        
        if(workoutInProgress == false && power > 0) {
            startWorkOut(deviceType: UInt16(deviceType))
        } else if(workoutInProgress == false && power == 0) {
            return;
        }

        let Speed = speed / 100;
        
        WorkoutTracking.kcal = kcal
        WorkoutTracking.steps = steps
        WorkoutTracking.distance = distance

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

