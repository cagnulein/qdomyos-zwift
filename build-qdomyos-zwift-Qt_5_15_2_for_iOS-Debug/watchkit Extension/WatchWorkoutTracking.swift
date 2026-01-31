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
    func startWorkOut()
    func stopWorkOut()
    func fetchStepCounts()
}

class WorkoutTracking: NSObject {
    static let shared = WorkoutTracking()
    public static var distance = Double()
    public static var kcal = Double()
    public static var totalKcal = Double()
    public static var cadenceTimeStamp = NSDate().timeIntervalSince1970
    public static var cadenceLastSteps = Double()
    public static var cadenceSteps = 0
    public static var speed = Double()
    public static var power = Double()
    public static var steps = Int()
    public static var cadence = Double()
    public static var lastDateMetric = Date()
    public static var flightsClimbed = Double()
    var sport: Int = 0
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
            guard let value = statistics.mostRecentQuantity()?.doubleValue(for: distanceUnit) else {
                return
            }
            let roundedValue = Double( round( 1 * value ) / 1 )
            delegate?.didReceiveHealthKitDistanceCycling(roundedValue)
            
        case HKQuantityType.quantityType(forIdentifier: .activeEnergyBurned):
            let energyUnit = HKUnit.kilocalorie()
            guard let value = statistics.mostRecentQuantity()?.doubleValue(for: energyUnit) else {
                return
            }
            let roundedValue = Double( round( 1 * value ) / 1 )
            delegate?.didReceiveHealthKitActiveEnergyBurned(roundedValue)
        
        case HKQuantityType.quantityType(forIdentifier: .heartRate):
            let heartRateUnit = HKUnit.count().unitDivided(by: HKUnit.minute())
            guard let value = statistics.mostRecentQuantity()?.doubleValue(for: heartRateUnit) else {
                return
            }
            let roundedValue = Double( round( 1 * value ) / 1 )
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
                    let now = NSDate().timeIntervalSince1970
                    let deltaT = now - WorkoutTracking.cadenceTimeStamp
                    let deltaC = resultCount - WorkoutTracking.cadenceLastSteps
                    WorkoutTracking.cadenceLastSteps = resultCount
                    WorkoutTracking.cadenceTimeStamp = now
                    weakSelf.delegate?.didReceiveHealthKitStepCounts(resultCount)
                    weakSelf.delegate?.didReceiveHealthKitStepCadence((deltaC / deltaT) * 60)
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
    
    func setSport(_ sport: Int) {
        self.sport = sport
    }
    
    private func configWorkout() {
        var activityType = HKWorkoutActivityType.cycling
        if self.sport == 1 {
            activityType = HKWorkoutActivityType.running
        } else if self.sport == 2 {
            activityType = HKWorkoutActivityType.walking
        } else if self.sport == 3 {
            activityType = HKWorkoutActivityType.elliptical
        } else if self.sport == 4 {
            activityType = HKWorkoutActivityType.rowing
        }
        
        configuration.activityType = activityType
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
                    HKSampleType.quantityType(forIdentifier: .flightsClimbed)!,
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
                    HKSampleType.quantityType(forIdentifier: .flightsClimbed)!,
                    HKSampleType.workoutType()
                    ])
            }
            
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
        WorkoutTracking.lastDateMetric = Date()
        // Reset flights climbed for new workout
        WorkoutTracking.flightsClimbed = 0
        print("Start workout")
        configWorkout()
        workoutSession.startActivity(with: Date())
        workoutBuilder.beginCollection(withStart: Date()) { (success, error) in
            print(success)
            if let error = error {
                print(error)
            }

            if self.sport > 0 {
                self.workoutBuilder.dataSource?.enableCollection(for: HKQuantityType.quantityType(forIdentifier: .distanceWalkingRunning)!, predicate: nil)
            }
        }
    }
    
    func stopWorkOut() {
        print("Stop workout")

        // Safety check - if workoutSession or workoutBuilder is nil, we can't properly stop
        guard workoutSession != nil, workoutBuilder != nil else {
            print("WatchWorkoutTracking: workoutSession or workoutBuilder is nil, cannot stop workout properly")
            return
        }

        workoutSession.stopActivity(with: Date())
        workoutSession.end()

        let unit = HKUnit.kilocalorie()
        let activeEnergyBurned = WorkoutTracking.kcal
        let startDate = workoutSession.startDate ?? WorkoutTracking.lastDateMetric
        let unitDistance = HKUnit.mile()
        let miles = WorkoutTracking.distance
        let quantityMiles = HKQuantity(unit: unitDistance, doubleValue: miles)

        // Collect all samples to add (optional - some may fail but we still finish the workout)
        var samplesToAdd: [HKSample] = []

        // Steps quantity for Running/Walking/Elliptical (to set totalSteps on workout)
        var stepsQuantityForWorkout: HKQuantity? = nil

        // Add active calories sample if possible
        if let activeQuantityType = HKQuantityType.quantityType(forIdentifier: .activeEnergyBurned) {
            let activeQuantity = HKQuantity(unit: unit, doubleValue: activeEnergyBurned)
            let activeSample = HKCumulativeQuantitySeriesSample(type: activeQuantityType,
                                                               quantity: activeQuantity,
                                                               start: startDate,
                                                               end: Date())
            samplesToAdd.append(activeSample)
        }

        // Add sport-specific samples
        if sport == 0 { // Cycling
            if let quantityTypeDistance = HKQuantityType.quantityType(forIdentifier: .distanceCycling) {
                let sampleDistance = HKCumulativeQuantitySeriesSample(type: quantityTypeDistance,
                                                              quantity: quantityMiles,
                                                              start: startDate,
                                                              end: Date())
                samplesToAdd.append(sampleDistance)
            }
        } else if sport == 4 { // Rowing
            // Add steps sample
            if let quantityTypeSteps = HKQuantityType.quantityType(forIdentifier: .stepCount) {
                let stepsQuantity = HKQuantity(unit: HKUnit.count(), doubleValue: Double(WorkoutTracking.steps))
                let sampleSteps = HKCumulativeQuantitySeriesSample(
                    type: quantityTypeSteps,
                    quantity: stepsQuantity,
                    start: startDate,
                    end: Date())
                samplesToAdd.append(sampleSteps)
            }

            // Add distance sample for rowing
            var quantityTypeDistance: HKQuantityType?
            if #available(watchOSApplicationExtension 10.0, *) {
                quantityTypeDistance = HKQuantityType.quantityType(forIdentifier: .distanceSwimming)
            } else {
                quantityTypeDistance = HKQuantityType.quantityType(forIdentifier: .distanceWalkingRunning)
            }

            if let typeDistance = quantityTypeDistance {
                let sampleDistance = HKCumulativeQuantitySeriesSample(type: typeDistance,
                                                              quantity: quantityMiles,
                                                              start: startDate,
                                                              end: Date())
                samplesToAdd.append(sampleDistance)
            }
        } else { // Running, Walking, Elliptical
            // Add steps sample
            if let quantityTypeSteps = HKQuantityType.quantityType(forIdentifier: .stepCount) {
                let stepsQuantity = HKQuantity(unit: HKUnit.count(), doubleValue: Double(WorkoutTracking.steps))
                stepsQuantityForWorkout = stepsQuantity  // Save for setting on workout
                let sampleSteps = HKCumulativeQuantitySeriesSample(
                    type: quantityTypeSteps,
                    quantity: stepsQuantity,
                    start: startDate,
                    end: Date())
                samplesToAdd.append(sampleSteps)
            }

            // Add distance sample
            if let quantityTypeDistance = HKQuantityType.quantityType(forIdentifier: .distanceWalkingRunning) {
                let sampleDistance = HKCumulativeQuantitySeriesSample(type: quantityTypeDistance,
                                                              quantity: quantityMiles,
                                                              start: startDate,
                                                              end: Date())
                samplesToAdd.append(sampleDistance)
            }

            // Add flights climbed sample if available
            if WorkoutTracking.flightsClimbed > 0 {
                if let quantityTypeFlights = HKQuantityType.quantityType(forIdentifier: .flightsClimbed) {
                    let flightsQuantity = HKQuantity(unit: HKUnit.count(), doubleValue: WorkoutTracking.flightsClimbed)
                    let sampleFlights = HKCumulativeQuantitySeriesSample(
                        type: quantityTypeFlights,
                        quantity: flightsQuantity,
                        start: startDate,
                        end: Date())
                    samplesToAdd.append(sampleFlights)
                    print("WatchWorkoutTracking: Adding flights climbed to workout: \(WorkoutTracking.flightsClimbed)")
                }
            }
        }

        // Helper function to finish the workout - ALWAYS called regardless of sample success
        let finishWorkout = { [weak self] in
            guard let self = self else { return }
            self.workoutBuilder.endCollection(withEnd: Date()) { (success, error) in
                if let error = error {
                    print("WatchWorkoutTracking endCollection error: \(error.localizedDescription)")
                }
                self.workoutBuilder.finishWorkout { (workout, error) in
                    if let error = error {
                        print("WatchWorkoutTracking finishWorkout error: \(error.localizedDescription)")
                    }
                    // Set workout properties
                    if let stepsQuantity = stepsQuantityForWorkout {
                        workout?.setValue(stepsQuantity, forKey: "totalSteps")
                    }
                    workout?.setValue(quantityMiles, forKey: "totalDistance")
                    let totalEnergy = WorkoutTracking.totalKcal > 0 ? WorkoutTracking.totalKcal : activeEnergyBurned
                    let totalEnergyQuantity = HKQuantity(unit: unit, doubleValue: totalEnergy)
                    workout?.setValue(totalEnergyQuantity, forKey: "totalEnergyBurned")

                    // Reset state for next workout
                    WorkoutTracking.flightsClimbed = 0
                    print("WatchWorkoutTracking: Workout finished successfully")
                }
            }
        }

        // Add samples and finish workout - ALWAYS finish even if adding samples fails
        if !samplesToAdd.isEmpty {
            workoutBuilder.add(samplesToAdd) { (success, error) in
                if let error = error {
                    print("WatchWorkoutTracking add samples error: \(error.localizedDescription)")
                }
                finishWorkout()
            }
        } else {
            // No samples to add, just finish the workout
            finishWorkout()
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
        
        if(sport == 0) {
            if #available(watchOSApplicationExtension 10.0, *) {            
                let wattPerInterval = HKQuantity(unit: HKUnit.watt(),
                                                doubleValue: WorkoutTracking.power)
                
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
                        print(error)
                    }
                }

                let cadencePerInterval = HKQuantity(unit: HKUnit.count().unitDivided(by: HKUnit.second()),
                                                    doubleValue: WorkoutTracking.cadence / 60.0)
                
                guard let cadenceType = HKQuantityType.quantityType(
                    forIdentifier: .cyclingCadence) else {
                return
                }
                let cadencePerIntervalSample = HKQuantitySample(type: cadenceType,
                                                                quantity: cadencePerInterval,
                                                            start: WorkoutTracking.lastDateMetric,
                                                            end: Date())
                workoutBuilder.add([cadencePerIntervalSample]) {(success, error) in
                    if let error = error {
                        print(error)
                    }
                }
                
                let speedPerInterval = HKQuantity(unit: HKUnit.meter().unitDivided(by: HKUnit.second()),
                                                doubleValue: WorkoutTracking.speed * 0.277778)
                
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
                        print(error)
                    }
                }

            } else {
                // Fallback on earlier versions
            }
        } else if(sport == 1) {
            if #available(watchOSApplicationExtension 10.0, *) {
                let wattPerInterval = HKQuantity(unit: HKUnit.watt(),
                                                doubleValue: WorkoutTracking.power)
                
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
                        print(error)
                    }
                }
            
                let speedPerInterval = HKQuantity(unit: HKUnit.meter().unitDivided(by: HKUnit.second()),
                                                doubleValue: WorkoutTracking.speed * 0.277778)
                
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
                        print(error)
                    }
                }

            } else {
                // Fallback on earlier versions
            }
        } else if(sport == 2) {
            if #available(watchOSApplicationExtension 10.0, *) {
                let speedPerInterval = HKQuantity(unit: HKUnit.meter().unitDivided(by: HKUnit.second()),
                                                doubleValue: WorkoutTracking.speed * 0.277778)
                
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
                        print(error)
                    }
                }

            } else {
                // Fallback on earlier versions
            }
        }
        
        WorkoutTracking.lastDateMetric = Date()
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
