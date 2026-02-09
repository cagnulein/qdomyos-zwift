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
    public static var previousDistance = Double()
    public static var kcal = Double()
    public static var totalKcal = Double()
    public static var steps = Double()
    public static var flightsClimbed = Double()
    static var sport: Int = 0
    static let healthStore = HKHealthStore()
    static let configuration = HKWorkoutConfiguration()
    static var workoutBuilder: HKWorkoutBuilder!
    static var workoutInProgress: Bool = false
    private var heartRateQuery: HKAnchoredObjectQuery?
    private var heartRateQueryAnchor: HKQueryAnchor?
    private let heartRateUnit = HKUnit.count().unitDivided(by: HKUnit.minute())
    private let heartRateType = HKQuantityType.quantityType(forIdentifier: .heartRate)
    private static var isUsingBluetoothHR: Bool = false
    private static var firstWorkout: Bool = true

    weak var delegate: WorkoutTrackingDelegate?

    override init() {
        super.init()
    }
}

@available(iOS 17.0, *)
extension WorkoutTracking {
    func setSport(_ sport: Int) {
        WorkoutTracking.sport = sport
    }
    
    private func configWorkout() {
        var activityType = HKWorkoutActivityType.cycling
        if WorkoutTracking.sport == 0 {
            activityType = HKWorkoutActivityType.walking
        } else if WorkoutTracking.sport == 1 {
            activityType = HKWorkoutActivityType.running
        } else if WorkoutTracking.sport == 2 {
            activityType = HKWorkoutActivityType.cycling
        } else if WorkoutTracking.sport == 3 {
            activityType = HKWorkoutActivityType.rowing
        } else if WorkoutTracking.sport == 4 {
            activityType = HKWorkoutActivityType.elliptical
        }
        
        WorkoutTracking.configuration.activityType = activityType
        WorkoutTracking.configuration.locationType = .indoor
        
        do {
            WorkoutTracking.workoutBuilder = try HKWorkoutBuilder(healthStore: WorkoutTracking.healthStore, configuration: WorkoutTracking.configuration, device: .local())
        } catch {
            return
        }
    }

    private func startHeartRateStreamingQuery() {
        guard heartRateQuery == nil else { return }
        guard HKHealthStore.isHealthDataAvailable() else {
            SwiftDebug.qtDebug("WorkoutTracking: Health data unavailable for heart rate query")
            return
        }
        guard let heartRateType = heartRateType else {
            SwiftDebug.qtDebug("WorkoutTracking: Heart rate type unavailable")
            return
        }

        let predicate = HKQuery.predicateForSamples(withStart: Date(), end: nil, options: .strictStartDate)
        let query = HKAnchoredObjectQuery(type: heartRateType, predicate: predicate, anchor: heartRateQueryAnchor, limit: HKObjectQueryNoLimit) { [weak self] _, samples, _, newAnchor, error in
            self?.handleHeartRateSamples(samples, error: error)
            self?.heartRateQueryAnchor = newAnchor
        }

        query.updateHandler = { [weak self] _, samples, _, newAnchor, error in
            self?.handleHeartRateSamples(samples, error: error)
            self?.heartRateQueryAnchor = newAnchor
        }

        heartRateQuery = query
        WorkoutTracking.healthStore.execute(query)
    }

    private func stopHeartRateStreamingQuery() {
        if let query = heartRateQuery {
            WorkoutTracking.healthStore.stop(query)
        }
        heartRateQuery = nil
        heartRateQueryAnchor = nil
    }

    private func handleHeartRateSamples(_ samples: [HKSample]?, error: Error?) {
        if let error = error {
            SwiftDebug.qtDebug("WorkoutTracking: Heart rate query error " + error.localizedDescription)
            return
        }

        guard
            let quantitySamples = samples as? [HKQuantitySample],
            let latestSample = quantitySamples.last
        else {
            return
        }

        let bpm = latestSample.quantity.doubleValue(for: heartRateUnit)

        DispatchQueue.main.async {
            WatchKitConnection.currentHeartRate = Int(round(bpm))
            self.delegate?.didReceiveHealthKitHeartRate(bpm)
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
                if #available(iOS 18.0, *) {
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
                        HKSampleType.quantityType(forIdentifier: .distanceRowing)!,
                        HKSampleType.quantityType(forIdentifier: .rowingSpeed)!,
                        HKSampleType.quantityType(forIdentifier: .flightsClimbed)!,
                        HKSampleType.workoutType()
                        ])
                } else {
                    // Fallback on earlier versions
                }
            } else {
                // Fallback on earlier versions
                infoToShare = Set([
                    HKSampleType.quantityType(forIdentifier: .stepCount)!,
                    HKSampleType.quantityType(forIdentifier: .heartRate)!,
                    HKSampleType.quantityType(forIdentifier: .distanceCycling)!,
                    HKSampleType.quantityType(forIdentifier: .distanceWalkingRunning)!,
                    HKSampleType.quantityType(forIdentifier: .activeEnergyBurned)!,
                    HKSampleType.quantityType(forIdentifier: .basalEnergyBurned)!,
                    HKSampleType.quantityType(forIdentifier: .runningPower)!,
                    HKSampleType.quantityType(forIdentifier: .runningSpeed)!,
                    HKSampleType.quantityType(forIdentifier: .distanceRowing)!,
                    HKSampleType.quantityType(forIdentifier: .rowingSpeed)!,
                    HKSampleType.quantityType(forIdentifier: .flightsClimbed)!,
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
        if(WorkoutTracking.workoutInProgress) {
            return;
        }
        WorkoutTracking.authorizeHealthKit()
        WorkoutTracking.workoutInProgress = true;
        WorkoutTracking.lastDateMetric = Date()
        // Reset flights climbed and previous distance for new workout
        WorkoutTracking.flightsClimbed = 0
        WorkoutTracking.previousDistance = 0
        SwiftDebug.qtDebug("WorkoutTracking: Start workout")
        setSport(Int(deviceType))
        configWorkout()
        // Always start HealthKit HR query (will be stopped if Bluetooth HR arrives)
        startHeartRateStreamingQuery()
        WorkoutTracking.workoutBuilder.beginCollection(withStart: Date()) { (success, error) in
            SwiftDebug.qtDebug(success.description)
            if let error = error {
                SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
            }
        }
    }

    @objc func stopWorkOut() {
        SwiftDebug.qtDebug("WorkoutTracking: Stop workout")

        stopHeartRateStreamingQuery()

        // Reset Bluetooth HR flag
        WorkoutTracking.isUsingBluetoothHR = false

        guard let workoutBuilder = WorkoutTracking.workoutBuilder,
              let startDate = workoutBuilder.startDate else {
            SwiftDebug.qtDebug("WorkoutTracking: Cannot stop workout - no workout builder or start date available")
            WorkoutTracking.workoutInProgress = false
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
            
        let unitDistance = HKUnit.mile()
        let miles = WorkoutTracking.distance * 0.000621371
        let quantityMiles = HKQuantity(unit: unitDistance,
                                  doubleValue: miles)
        
        if(WorkoutTracking.sport == 2 || WorkoutTracking.sport == 4) {

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
            WorkoutTracking.workoutBuilder.endCollection(withEnd: Date()) { (success, error) in
                    if let error = error {
                        SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                    }
                WorkoutTracking.workoutBuilder.finishWorkout{ (workout, error) in
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

            // Create flights climbed sample
            var samplesToAdd: [HKCumulativeQuantitySeriesSample] = [sampleSteps, sampleDistance]

            if WorkoutTracking.flightsClimbed > 0 {
                if let quantityTypeFlights = HKQuantityType.quantityType(forIdentifier: .flightsClimbed) {
                    let flightsQuantity = HKQuantity(unit: HKUnit.count(), doubleValue: WorkoutTracking.flightsClimbed)
                    let sampleFlights = HKCumulativeQuantitySeriesSample(
                        type: quantityTypeFlights,
                        quantity: flightsQuantity,
                        start: startDate,
                        end: Date())
                    samplesToAdd.append(sampleFlights)
                    SwiftDebug.qtDebug("WorkoutTracking: Adding flights climbed to workout: \(WorkoutTracking.flightsClimbed)")
                }
            }

            // Add all samples to the workout builder
            workoutBuilder.add(samplesToAdd) { (success, error) in
                if let error = error {
                    print(error)
                    SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                    return
                }

                // End the data collection - only do this once
                WorkoutTracking.workoutBuilder.endCollection(withEnd: Date()) { (success, error) in
                    if let error = error {
                        print(error)
                        SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                        return
                    }

                    // Finish the workout - only do this once
                    WorkoutTracking.workoutBuilder.finishWorkout { (workout, error) in
                        if let error = error {
                            print(error)
                            SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                            return
                        }

                        // Set total energy burned on the workout
                        let totalEnergy = WorkoutTracking.totalKcal > 0 ? WorkoutTracking.totalKcal : activeEnergyBurned
                        let totalEnergyQuantity = HKQuantity(unit: unit, doubleValue: totalEnergy)
                        workout?.setValue(totalEnergyQuantity, forKey: "totalEnergyBurned")

                        // Reset flights climbed for next workout
                        WorkoutTracking.flightsClimbed = 0
                    }
                }
            }
        }
        
        WorkoutTracking.workoutInProgress = false;
    }
    
    @objc func addMetrics(power: Double, cadence: Double, speed: Double, kcal: Double, steps: Double, deviceType: UInt8, distance: Double, totalKcal: Double, elevationGain: Double = 0) {
        SwiftDebug.qtDebug("WorkoutTracking: GET DATA: \(Date())")

        if(WorkoutTracking.workoutInProgress == false && power > 0 && WorkoutTracking.firstWorkout) {
            WorkoutTracking.firstWorkout = false
            startWorkOut(deviceType: UInt16(deviceType))
        } else if(WorkoutTracking.workoutInProgress == false && power == 0) {
            return;
        }

        let Speed = speed / 100
        let previousSteps = WorkoutTracking.steps

        WorkoutTracking.kcal = kcal
        WorkoutTracking.totalKcal = totalKcal
        WorkoutTracking.steps = steps
        WorkoutTracking.distance = distance

        // Calculate flights climbed from elevation gain for treadmill (sport == 0 or 1)
        // elevationGain is already calculated by QZ in meters
        if (WorkoutTracking.sport == 0 || WorkoutTracking.sport == 1) && elevationGain > 0 {
            // One flight = 10 feet = 3.048 meters
            WorkoutTracking.flightsClimbed = elevationGain / 3.048
            SwiftDebug.qtDebug("WorkoutTracking: Flights climbed: \(WorkoutTracking.flightsClimbed) from elevation: \(elevationGain)m")
        }

        if(WorkoutTracking.sport == 2) {
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
                WorkoutTracking.workoutBuilder.add([wattPerIntervalSample]) {(success, error) in
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
                WorkoutTracking.workoutBuilder.add([cadencePerIntervalSample]) {(success, error) in
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
                WorkoutTracking.workoutBuilder.add([speedPerIntervalSample]) {(success, error) in
                    if let error = error {
                        SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                    }
                }

            } else {
                // Fallback on earlier versions
            }
        } else if(WorkoutTracking.sport == 1) {
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
                WorkoutTracking.workoutBuilder.add([wattPerIntervalSample]) {(success, error) in
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
                WorkoutTracking.workoutBuilder.add([speedPerIntervalSample]) {(success, error) in
                    if let error = error {
                        SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                    }
                }

            } else {
                // Fallback on earlier versions
            }
        } else if(WorkoutTracking.sport == 0) {
            if #available(watchOSApplicationExtension 10.0, *) {
                if(WorkoutTracking.lastDateMetric.distance(to: Date()) < 1) {
                    return
                }
                
                let speedPerInterval = HKQuantity(unit: HKUnit.meter().unitDivided(by: HKUnit.second()),
                                                  doubleValue: Speed * 0.277778)
                
                if let walkingSpeedType = HKQuantityType.quantityType(forIdentifier: .walkingSpeed) {
                    let speedSample = HKQuantitySample(type: walkingSpeedType,
                                                       quantity: speedPerInterval,
                                                       start: WorkoutTracking.lastDateMetric,
                                                       end: Date())
                    WorkoutTracking.workoutBuilder.add([speedSample]) { (success, error) in
                        if let error = error {
                            SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                        }
                    }
                }
                
                let stepDelta = max(0, steps - previousSteps)
                if stepDelta > 0,
                   let stepType = HKQuantityType.quantityType(forIdentifier: .stepCount) {
                    let stepQuantity = HKQuantity(unit: HKUnit.count(), doubleValue: stepDelta)
                    let stepSample = HKQuantitySample(type: stepType,
                                                      quantity: stepQuantity,
                                                      start: WorkoutTracking.lastDateMetric,
                                                      end: Date())
                    WorkoutTracking.workoutBuilder.add([stepSample]) { (success, error) in
                        if let error = error {
                            SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                        }
                    }
                }
            } else {
                // Fallback on earlier versions
            }
        } else if(WorkoutTracking.sport == 4) {
            if #available(watchOSApplicationExtension 10.0, *) {
                if(WorkoutTracking.lastDateMetric.distance(to: Date()) < 1) {
                    return
                }

                let speedPerInterval = HKQuantity(unit: HKUnit.meter().unitDivided(by: HKUnit.second()),
                                                  doubleValue: (Speed / 3.6))

                if let cyclingSpeedType = HKQuantityType.quantityType(forIdentifier: .cyclingSpeed) {
                    let speedSample = HKQuantitySample(type: cyclingSpeedType,
                                                       quantity: speedPerInterval,
                                                       start: WorkoutTracking.lastDateMetric,
                                                       end: Date())
                    WorkoutTracking.workoutBuilder.add([speedSample]) { (success, error) in
                        if let error = error {
                            SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                        }
                    }
                }

                if power > 0,
                   let runningPowerType = HKQuantityType.quantityType(forIdentifier: .runningPower) {
                    let powerQuantity = HKQuantity(unit: HKUnit.watt(), doubleValue: power)
                    let powerSample = HKQuantitySample(type: runningPowerType,
                                                       quantity: powerQuantity,
                                                       start: WorkoutTracking.lastDateMetric,
                                                       end: Date())
                    WorkoutTracking.workoutBuilder.add([powerSample]) { (success, error) in
                        if let error = error {
                            SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                        }
                    }
                }

                if cadence > 0,
                   let cyclingCadenceType = HKQuantityType.quantityType(forIdentifier: .cyclingCadence) {
                    // For ellipticals, divide cadence by 2 to match QZ display (steps per minute vs revolutions per minute)
                    let cadenceQuantity = HKQuantity(unit: HKUnit.count().unitDivided(by: HKUnit.second()),
                                                     doubleValue: (cadence / 2.0) / 60.0)
                    let cadenceSample = HKQuantitySample(type: cyclingCadenceType,
                                                         quantity: cadenceQuantity,
                                                         start: WorkoutTracking.lastDateMetric,
                                                         end: Date())
                    WorkoutTracking.workoutBuilder.add([cadenceSample]) { (success, error) in
                        if let error = error {
                            SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                        }
                    }
                }

                let distanceDelta = max(0, distance - WorkoutTracking.previousDistance)
                if distanceDelta > 0,
                   let distanceType = HKQuantityType.quantityType(forIdentifier: .distanceCycling) {
                    let distanceQuantity = HKQuantity(unit: HKUnit.meter(), doubleValue: distanceDelta)
                    let distanceSample = HKQuantitySample(type: distanceType,
                                                          quantity: distanceQuantity,
                                                          start: WorkoutTracking.lastDateMetric,
                                                          end: Date())
                    WorkoutTracking.workoutBuilder.add([distanceSample]) { (success, error) in
                        if let error = error {
                            SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                        }
                    }
                }

            } else {
                // Fallback on earlier versions
            }
        } else if(WorkoutTracking.sport == 3) {
            if #available(watchOSApplicationExtension 10.0, *) {
                if(WorkoutTracking.lastDateMetric.distance(to: Date()) < 1) {
                    return
                }
                
                if #available(iOS 18.0, *) {
                    if let rowingSpeedType = HKQuantityType.quantityType(forIdentifier: .rowingSpeed) {
                        let speedQuantity = HKQuantity(unit: HKUnit.meter().unitDivided(by: HKUnit.second()),
                                                       doubleValue: Speed * 0.277778)
                        let speedSample = HKQuantitySample(type: rowingSpeedType,
                                                           quantity: speedQuantity,
                                                           start: WorkoutTracking.lastDateMetric,
                                                           end: Date())
                        WorkoutTracking.workoutBuilder.add([speedSample]) { (success, error) in
                            if let error = error {
                                SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                            }
                        }
                    }
                } else {
                    // Fallback on earlier versions
                }
                
                let distanceDelta = max(0, distance - WorkoutTracking.previousDistance)
                if #available(iOS 18.0, *) {
                    if distanceDelta > 0,
                       let distanceType = HKQuantityType.quantityType(forIdentifier: .distanceRowing) {
                        let distanceQuantity = HKQuantity(unit: HKUnit.meter(), doubleValue: distanceDelta)
                        let distanceSample = HKQuantitySample(type: distanceType,
                                                              quantity: distanceQuantity,
                                                              start: WorkoutTracking.lastDateMetric,
                                                              end: Date())
                        WorkoutTracking.workoutBuilder.add([distanceSample]) { (success, error) in
                            if let error = error {
                                SwiftDebug.qtDebug("WorkoutTracking: " + error.localizedDescription)
                            }
                        }
                    }
                } else {
                    // Fallback on earlier versions
                }
            } else {
                // Fallback on earlier versions
            }
        }

        WorkoutTracking.previousDistance = distance
        WorkoutTracking.lastDateMetric = Date()
    }

    @objc func setBluetoothHeartRate(heartRate: Double) {
        // Verify workout is in progress
        if !WorkoutTracking.workoutInProgress {
            SwiftDebug.qtDebug("WorkoutTracking: Cannot write HR - workout not in progress")
            return
        }

        // Verify we have a valid heart rate value
        if heartRate <= 0 || heartRate > 250 {
            SwiftDebug.qtDebug("WorkoutTracking: Invalid HR value: \(heartRate)")
            return
        }

        // Mark that we're using Bluetooth HR (priority over HealthKit)
        if !WorkoutTracking.isUsingBluetoothHR {
            SwiftDebug.qtDebug("WorkoutTracking: Switching to Bluetooth HR source")
            // Stop HealthKit HR query if it was running
            stopHeartRateStreamingQuery()
            WorkoutTracking.isUsingBluetoothHR = true
        }

        // Get HR quantity type
        guard let heartRateType = HKQuantityType.quantityType(forIdentifier: .heartRate) else {
            SwiftDebug.qtDebug("WorkoutTracking: Heart rate type unavailable")
            return
        }

        // Create heart rate quantity
        let heartRateQuantity = HKQuantity(unit: heartRateUnit, doubleValue: heartRate)

        // Create heart rate sample with current timestamp
        let now = Date()
        let heartRateSample = HKQuantitySample(type: heartRateType,
                                               quantity: heartRateQuantity,
                                               start: now,
                                               end: now)

        // Add sample to workout builder
        WorkoutTracking.workoutBuilder.add([heartRateSample]) { (success, error) in
            if let error = error {
                SwiftDebug.qtDebug("WorkoutTracking HR: " + error.localizedDescription)
            } else {
                SwiftDebug.qtDebug("WorkoutTracking: HR written to HealthKit: \(heartRate) BPM")
            }
        }
    }
}

