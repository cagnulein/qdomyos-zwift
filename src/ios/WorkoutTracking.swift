//
//  WatchWorkoutTracking.swift
//  ElecDemo WatchKit Extension
//
//  Created by NhatHM on 8/12/19.
//  Copyright © 2019 GST.PID. All rights reserved.
//

import Foundation
import HealthKit

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
    var queryRunning = true
    
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
                        WorkoutTracking.shared.queryRunning = false
                        print("Authorization healthkit success")
                    } else if let error = error {
                        print(error)
                    }
                }
            }
            
        } else {
            print("HealthKit not avaiable")
        }
    }
    
    @objc func startWorkOut(deviceType: UInt16) {
        if(workoutInProgress) {
            return;
        }
        workoutInProgress = true;
        WorkoutTracking.lastDateMetric = Date()
        print("Start workout")
        setSport(Int(deviceType))
        configWorkout()
        workoutBuilder.beginCollection(withStart: Date()) { (success, error) in
            print(success)
            if let error = error {
                print(error)
            }
        }
    }
    
    @objc func stopWorkOut() {
        print("Stop workout")
        
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
        
        if(sport == 0) {
            
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
                    print(error)
                }
            }
                self.workoutBuilder.endCollection(withEnd: Date()) { (success, error) in
                    if let error = error {
                        print(error)
                    }
                    self.workoutBuilder.finishWorkout{ (workout, error) in
                        if let error = error {
                            print(error)
                        }
                        workout?.setValue(quantityMiles, forKey: "totalDistance")
                    }
                }
        } else {
            
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
                    print(error)
                }
                self.workoutBuilder.endCollection(withEnd: Date()) { (success, error) in
                    if let error = error {
                        print(error)
                    }
                    self.workoutBuilder.finishWorkout{ (workout, error) in
                        if let error = error {
                            print(error)
                        }
                        workout?.setValue(quantityMiles, forKey: "totalDistance")
                    }
                }
            }
        }
        
        workoutInProgress = false;
    }
}



@available(iOS 17.0, *)
extension WorkoutTracking {
    public func fetchLatestHeartRateSample(completionHandler: @escaping (_ sample: HKQuantitySample?) -> Void) {
        if(self.queryRunning) {
            return
        }
        self.queryRunning = true
        guard let sampleType = HKObjectType.quantityType(forIdentifier: HKQuantityTypeIdentifier.heartRate) else {
            completionHandler(nil)
            return
        }
        
        let tenMinutesAgo = Calendar.current.date(byAdding: .minute, value: -90, to: Date())!
        let predicate = HKQuery.predicateForSamples(withStart: tenMinutesAgo, end: Date(), options: .strictEndDate)
        let sortDescriptor = NSSortDescriptor(key: HKSampleSortIdentifierStartDate, ascending: false)
        let query = HKSampleQuery(sampleType: sampleType,
                                  predicate: predicate,
                                  limit: Int(HKObjectQueryNoLimit),
                                  sortDescriptors: [sortDescriptor]) { (_, results, error) in
                                    self.queryRunning = false
                                    if let error = error {
                                        print("Error: \(error.localizedDescription)")
                                        return
                                    }
                                    if(results?.count ?? 0 > 0) {
                                        completionHandler(results?[0] as? HKQuantitySample)
                                    } else {
                                        completionHandler(nil)
                                    }
        }
        
        healthStore.execute(query)
    }
}
