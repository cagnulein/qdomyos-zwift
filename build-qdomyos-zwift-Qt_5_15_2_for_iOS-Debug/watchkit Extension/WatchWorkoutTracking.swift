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
    func didReceiveHealthKitDistanceCycling(_ distanceCycling: Double)
    func didReceiveHealthKitActiveEnergyBurned(_ activeEnergyBurned: Double)
}

protocol WorkoutTrackingProtocol {
    static func authorizeHealthKit()
    func startWorkOut()
    func stopWorkOut()
    func fetchStepCounts()
}

class WorkoutTracking: NSObject {
    static let shared = WorkoutTracking()
    public static var distance = Double()
    public static var kcal = Double()
    let healthStore = HKHealthStore()
    let configuration = HKWorkoutConfiguration()
    var workoutSession: HKWorkoutSession!
    var workoutBuilder: HKLiveWorkoutBuilder!
    
    weak var delegate: WorkoutTrackingDelegate?
    
    override init() {
        super.init()
    }        
}

extension WorkoutTracking {
    private func handleSendStatisticsData(_ statistics: HKStatistics) {
        switch statistics.quantityType {
        case HKQuantityType.quantityType(forIdentifier: .distanceCycling):
            let distanceUnit = HKUnit.mile()
            let value = statistics.mostRecentQuantity()?.doubleValue(for: distanceUnit)
            let roundedValue = Double( round( 1 * value! ) / 1 )
            delegate?.didReceiveHealthKitDistanceCycling(roundedValue)
            
        case HKQuantityType.quantityType(forIdentifier: .activeEnergyBurned):
            let energyUnit = HKUnit.kilocalorie()
            let value = statistics.mostRecentQuantity()?.doubleValue(for: energyUnit)
            let roundedValue = Double( round( 1 * value! ) / 1 )
            delegate?.didReceiveHealthKitActiveEnergyBurned(roundedValue)
        
        case HKQuantityType.quantityType(forIdentifier: .heartRate):
            let heartRateUnit = HKUnit.count().unitDivided(by: HKUnit.minute())
            let value = statistics.mostRecentQuantity()?.doubleValue(for: heartRateUnit)
            let roundedValue = Double( round( 1 * value! ) / 1 )
            delegate?.didReceiveHealthKitHeartRate(roundedValue)
            
        case HKQuantityType.quantityType(forIdentifier: .stepCount):
            guard let stepCounts = HKQuantityType.quantityType(forIdentifier: .stepCount) else {
                return
            }
            let startOfDay = Calendar.current.startOfDay(for: Date())
            let predicate = HKQuery.predicateForSamples(withStart: startOfDay, end: Date(), options: .strictStartDate)
            
            let query = HKStatisticsQuery(quantityType: stepCounts, quantitySamplePredicate: predicate, options: .cumulativeSum) { [weak self] (_, result, error) in
                guard let weakSelf = self else {
                    return
                }
                var resultCount = 0.0
                guard let result = result else {
                    print("Failed to fetch steps rate")
                    return
                }
                
                if let sum = result.sumQuantity() {
                    resultCount = sum.doubleValue(for: HKUnit.count())
                    weakSelf.delegate?.didReceiveHealthKitStepCounts(resultCount)
                } else {
                    print("Failed to fetch steps rate 2")
                }
            }
            healthStore.execute(query)
            return
        default:
            return
        }
    }
    
    private func configWorkout() {
        configuration.activityType = .cycling
        configuration.locationType = .indoor
        
        do {
            workoutSession = try HKWorkoutSession(healthStore: healthStore, configuration: configuration)
            workoutBuilder = workoutSession?.associatedWorkoutBuilder()
        } catch {
            return
        }
        
        workoutSession.delegate = self
        workoutBuilder.delegate = self
        
        workoutBuilder.dataSource = HKLiveWorkoutDataSource(healthStore: healthStore, workoutConfiguration: configuration)
    }
}

extension WorkoutTracking: WorkoutTrackingProtocol {
    static func authorizeHealthKit() {
        if HKHealthStore.isHealthDataAvailable() {
            let infoToRead = Set([
                HKSampleType.quantityType(forIdentifier: .stepCount)!,
                HKSampleType.quantityType(forIdentifier: .heartRate)!,
                /*HKSampleType.quantityType(forIdentifier: .distanceCycling)!,
                HKSampleType.quantityType(forIdentifier: .activeEnergyBurned)!,*/
                HKSampleType.workoutType()
                ])
            
            let infoToShare = Set([
                HKSampleType.quantityType(forIdentifier: .stepCount)!,
                HKSampleType.quantityType(forIdentifier: .heartRate)!,
                HKSampleType.quantityType(forIdentifier: .distanceCycling)!,
                HKSampleType.quantityType(forIdentifier: .activeEnergyBurned)!,				
                HKSampleType.workoutType()
                ])
            
            HKHealthStore().requestAuthorization(toShare: infoToShare, read: infoToRead) { (success, error) in
                if success {
                    print("Authorization healthkit success")
                } else if let error = error {
                    print(error)
                }
            }
        } else {
            print("HealthKit not avaiable")
        }
    }
    
    func startWorkOut() {
        print("Start workout")
        configWorkout()
        workoutSession.startActivity(with: Date())
        workoutBuilder.beginCollection(withStart: Date()) { (success, error) in
            print(success)
            if let error = error {
                print(error)
            }
        }
    }
    
    func stopWorkOut() {
        print("Stop workout")
        workoutSession.stopActivity(with: Date())
        workoutSession.end()
        
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
                                                      start: workoutSession.startDate!,
                                                      end: Date())
        
        workoutBuilder.add([sample]) {(success, error) in}
        
        guard let quantityTypeDistance = HKQuantityType.quantityType(
                forIdentifier: .distanceCycling) else {
          return
        }
            
        let unitDistance = HKUnit.mile()
        let miles = WorkoutTracking.distance
        let quantityMiles = HKQuantity(unit: unitDistance,
                                  doubleValue: miles)
        
        let sampleDistance = HKCumulativeQuantitySeriesSample(type: quantityTypeDistance,
                                                      quantity: quantityMiles,
                                                      start: workoutSession.startDate!,
                                                      end: Date())
        
        workoutBuilder.add([sample]) {(success, error) in}
        workoutBuilder.add([sampleDistance]) {(success, error) in}
        
        workoutBuilder.endCollection(withEnd: Date()) { (success, error) in
            
        }
    }
    
    func fetchStepCounts() {
        guard let stepCounts = HKQuantityType.quantityType(forIdentifier: .stepCount) else {
            return
        }
        let startOfDay = Calendar.current.startOfDay(for: Date())
        let predicate = HKQuery.predicateForSamples(withStart: startOfDay, end: Date(), options: .strictStartDate)
        
        let query = HKStatisticsQuery(quantityType: stepCounts, quantitySamplePredicate: predicate, options: .cumulativeSum) { [weak self] (_, result, error) in
            guard let weakSelf = self else {
                return
            }
            var resultCount = 0.0
            guard let result = result else {
                print("Failed to fetch steps rate")
                return
            }
            
            if let sum = result.sumQuantity() {
                resultCount = sum.doubleValue(for: HKUnit.count())
                weakSelf.delegate?.didReceiveHealthKitStepCounts(resultCount)
            } else {
                print("Failed to fetch steps rate 2")
            }
        }
        healthStore.execute(query)
    }
}

extension WorkoutTracking: HKLiveWorkoutBuilderDelegate {
    func workoutBuilder(_ workoutBuilder: HKLiveWorkoutBuilder, didCollectDataOf collectedTypes: Set<HKSampleType>) {
        print("GET DATA: \(Date())")
        for type in collectedTypes {
            guard let quantityType = type as? HKQuantityType else {
                return
            }
            
            if let statistics = workoutBuilder.statistics(for: quantityType) {
                handleSendStatisticsData(statistics)
            }
        }
    }
    
    func workoutBuilderDidCollectEvent(_ workoutBuilder: HKLiveWorkoutBuilder) {
        
    }
}

extension WorkoutTracking: HKWorkoutSessionDelegate {
    func workoutSession(_ workoutSession: HKWorkoutSession, didChangeTo toState: HKWorkoutSessionState, from fromState: HKWorkoutSessionState, date: Date) {
        
    }
    
    func workoutSession(_ workoutSession: HKWorkoutSession, didFailWithError error: Error) {
        
    }
}
