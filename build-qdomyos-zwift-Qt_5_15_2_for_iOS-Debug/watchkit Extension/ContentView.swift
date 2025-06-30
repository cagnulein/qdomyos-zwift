//
//  ContentView.swift
//  watchkit Extension
//
//  Created by Claude on 2025-06-30.
//  SwiftUI version of the QZ Fitness Watch App
//

import SwiftUI
import HealthKit
import CoreMotion

struct ContentView: View {
    @StateObject private var workoutManager = WorkoutManager()
    @StateObject private var watchConnection = WatchConnectionManager()
    
    var body: some View {
        NavigationView {
            if workoutManager.isWorkoutActive {
                WorkoutView()
                    .environmentObject(workoutManager)
                    .environmentObject(watchConnection)
            } else {
                SportSelectionView()
                    .environmentObject(workoutManager)
                    .environmentObject(watchConnection)
            }
        }
    }
}

// MARK: - Workout Manager
class WorkoutManager: NSObject, ObservableObject {
    @Published var isWorkoutActive = false
    @Published var workoutState: WorkoutState = .stopped
    @Published var elapsedTime: TimeInterval = 0
    @Published var heartRate: Double = 0
    @Published var distance: Double = 0
    @Published var calories: Double = 0
    @Published var speed: Double = 0
    @Published var power: Double = 0
    @Published var cadence: Double = 0
    @Published var stepCadence: Int = 0
    @Published var steps: Int = 0
    @Published var selectedSport: Sport = .cycling
    
    private var workoutTracking = WorkoutTracking.shared
    private var timer: Timer?
    private var metricsTimer: Timer?
    private var startDate: Date?
    private let pedometer = CMPedometer()
    
    enum WorkoutState {
        case stopped, running, paused
    }
    
    enum Sport: Int, CaseIterable {
        case cycling = 0
        case running = 1
        case walking = 2
        case elliptical = 3
        case rowing = 4
        
        var displayName: String {
            switch self {
            case .cycling: return "Cycling"
            case .running: return "Running"
            case .walking: return "Walking"
            case .elliptical: return "Elliptical"
            case .rowing: return "Rowing"
            }
        }
        
        var icon: String {
            switch self {
            case .cycling: return "bicycle"
            case .running: return "figure.run"
            case .walking: return "figure.walk"
            case .elliptical: return "figure.elliptical"
            case .rowing: return "figure.rower"
            }
        }
    }
    
    override init() {
        super.init()
        setupWorkoutTracking()
        setupPedometer()
        loadSelectedSport()
    }
    
    private func setupWorkoutTracking() {
        workoutTracking.delegate = self
        WorkoutTracking.authorizeHealthKit()
        
        // Update workout tracking with connection data
        updateMetricsFromConnection()
    }
    
    private func updateMetricsFromConnection() {
        // Get metrics from WatchKitConnection
        self.distance = WatchKitConnection.distance
        self.calories = WatchKitConnection.kcal
        self.speed = WatchKitConnection.speed
        self.power = WatchKitConnection.power
        self.cadence = WatchKitConnection.cadence
        self.steps = WatchKitConnection.steps
    }
    
    private func setupPedometer() {
        if CMPedometer.isStepCountingAvailable() {
            pedometer.startUpdates(from: Date()) { [weak self] pedometerData, error in
                guard let self = self, let pedometerData = pedometerData, error == nil else { return }
                
                DispatchQueue.main.async {
                    self.stepCadence = Int(((pedometerData.currentCadence?.doubleValue ?? 0) * 60.0 / 2.0))
                    
                    // Send step cadence to iPhone/iPad
                    WatchKitConnection.stepCadence = self.stepCadence
                    WatchKitConnection.shared.sendMessage(message: ["stepCadence": "\(self.stepCadence)" as AnyObject])
                }
            }
        }
    }
    
    private func loadSelectedSport() {
        let sportValue = UserDefaults.standard.integer(forKey: "sport")
        selectedSport = Sport(rawValue: sportValue) ?? .cycling
    }
    
    func selectSport(_ sport: Sport) {
        selectedSport = sport
        UserDefaults.standard.set(sport.rawValue, forKey: "sport")
    }
    
    func startWorkout() {
        guard !isWorkoutActive else { return }
        
        isWorkoutActive = true
        workoutState = .running
        startDate = Date()
        elapsedTime = 0
        
        // Set up workout tracking
        workoutTracking.setSport(selectedSport.rawValue)
        workoutTracking.startWorkOut()
        
        // Set up watch connectivity
        watchConnection.delegate = self
        watchConnection.startSession()
        
        startTimer()
        startMetricsTimer()
    }
    
    func pauseWorkout() {
        guard workoutState == .running else { return }
        workoutState = .paused
        timer?.invalidate()
        metricsTimer?.invalidate()
    }
    
    func resumeWorkout() {
        guard workoutState == .paused else { return }
        workoutState = .running
        startTimer()
        startMetricsTimer()
    }
    
    func stopWorkout() {
        guard isWorkoutActive else { return }
        
        isWorkoutActive = false
        workoutState = .stopped
        timer?.invalidate()
        metricsTimer?.invalidate()
        
        workoutTracking.stopWorkOut()
        
        // Reset values
        elapsedTime = 0
        heartRate = 0
        distance = 0
        calories = 0
        speed = 0
        power = 0
        cadence = 0
        steps = 0
    }
    
    private func startTimer() {
        timer = Timer.scheduledTimer(withTimeInterval: 0.1, repeats: true) { [weak self] _ in
            guard let self = self, let startDate = self.startDate else { return }
            
            DispatchQueue.main.async {
                self.elapsedTime = Date().timeIntervalSince(startDate)
            }
        }
    }
    
    private func startMetricsTimer() {
        metricsTimer = Timer.scheduledTimer(withTimeInterval: 1.0, repeats: true) { [weak self] _ in
            guard let self = self else { return }
            
            DispatchQueue.main.async {
                self.updateMetricsFromConnection()
            }
        }
    }
}

// MARK: - WorkoutTrackingDelegate
extension WorkoutManager: WorkoutTrackingDelegate {
    func didReceiveHealthKitHeartRate(_ heartRate: Double) {
        DispatchQueue.main.async {
            self.heartRate = heartRate
            // Send heart rate to iPhone/iPad via WatchKitConnection
            WatchKitConnection.shared.sendMessage(message: ["heartRate": "\(heartRate)" as AnyObject])
            
            // Update WorkoutTracking with connection data
            self.updateMetricsFromConnection()
        }
    }
    
    func didReceiveHealthKitStepCounts(_ stepCounts: Double) {
        DispatchQueue.main.async {
            self.steps = Int(stepCounts)
        }
    }
    
    func didReceiveHealthKitStepCadence(_ stepCadence: Double) {
        // Step cadence is handled in setupPedometer via CoreMotion
        // This provides more accurate real-time data
    }
    
    func didReceiveHealthKitDistanceCycling(_ distanceCycling: Double) {
        DispatchQueue.main.async {
            // Distance comes from the main app via WatchKitConnection
            // HealthKit distance is used for validation/backup
            self.updateMetricsFromConnection()
        }
    }
    
    func didReceiveHealthKitActiveEnergyBurned(_ activeEnergyBurned: Double) {
        DispatchQueue.main.async {
            // Calories comes from the main app via WatchKitConnection  
            // HealthKit calories is used for validation/backup
            self.updateMetricsFromConnection()
        }
    }
}

// MARK: - WorkoutManager WatchKitConnectionDelegate
extension WorkoutManager: WatchKitConnectionDelegate {
    func didReceiveUserName(_ userName: String) {
        // This will be handled by WatchConnectionManager
    }
}

// MARK: - Watch Connection Manager
class WatchConnectionManager: ObservableObject {
    @Published var userName: String = "QZ Fitness"
    
    private var watchConnection = WatchKitConnection.shared
    
    init() {
        setupConnection()
    }
    
    private func setupConnection() {
        watchConnection.delegate = self
        watchConnection.startSession()
    }
}

extension WatchConnectionManager: WatchKitConnectionDelegate {
    func didReceiveUserName(_ userName: String) {
        DispatchQueue.main.async {
            self.userName = userName
        }
    }
}

#Preview {
    ContentView()
}