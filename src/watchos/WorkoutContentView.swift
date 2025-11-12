//
//  WorkoutContentView.swift
//  qdomyos-zwift Watch Extension
//
//  Created by Claude Code for SwiftUI Redesign
//  Main workout view with configurable metric tiles
//

import SwiftUI

struct WorkoutContentView: View {
    @ObservedObject var metrics = WorkoutMetrics.shared
    @ObservedObject var tileConfig = TileConfiguration.shared
    @State private var isWorkoutActive = false
    @State private var selectedSportIndex = 0

    let sportTypes: [WorkoutMetrics.SportType] = [.bike, .run, .walk, .elliptical, .rowing]

    var body: some View {
        ScrollView {
            VStack(spacing: 10) {
                // Sport selector (only when workout is not active)
                if !isWorkoutActive {
                    sportSelectorSection
                }

                // Metric tiles grid
                metricsGridSection

                // Control button
                controlButtonSection
            }
            .padding(.horizontal, 4)
            .padding(.vertical, 8)
        }
        .background(Color.black.edgesIgnoringSafeArea(.all))
        .onAppear {
            // Load saved sport type
            selectedSportIndex = UserDefaults.standard.integer(forKey: "sport")
            metrics.sportType = sportTypes[selectedSportIndex]
        }
    }

    // MARK: - Sport Selector Section

    private var sportSelectorSection: some View {
        VStack(spacing: 4) {
            Text("Sport")
                .font(.system(size: 12, weight: .semibold))
                .foregroundColor(.gray)
                .frame(maxWidth: .infinity, alignment: .leading)

            Picker("Sport", selection: $selectedSportIndex) {
                ForEach(0..<sportTypes.count, id: \.self) { index in
                    Text(sportTypes[index].displayName)
                        .tag(index)
                }
            }
            .pickerStyle(.wheel)
            .frame(height: 80)
            .onChange(of: selectedSportIndex) { newValue in
                metrics.sportType = sportTypes[newValue]
                UserDefaults.standard.set(newValue, forKey: "sport")
                UserDefaults.standard.synchronize()
            }
        }
        .padding(.bottom, 8)
    }

    // MARK: - Metrics Grid Section

    private var metricsGridSection: some View {
        let columns = [
            GridItem(.flexible(), spacing: 8),
            GridItem(.flexible(), spacing: 8)
        ]

        return LazyVGrid(columns: columns, spacing: 8) {
            ForEach(tileConfig.enabledTiles, id: \.self) { tileType in
                MetricTileView(tileType: tileType, metrics: metrics)
                    .frame(height: 70)
            }
        }
    }

    // MARK: - Control Button Section

    private var controlButtonSection: some View {
        Button(action: toggleWorkout) {
            HStack {
                Image(systemName: isWorkoutActive ? "stop.fill" : "play.fill")
                    .font(.system(size: 16, weight: .semibold))
                Text(isWorkoutActive ? "Stop" : "Start")
                    .font(.system(size: 18, weight: .semibold))
            }
            .frame(maxWidth: .infinity)
            .frame(height: 44)
            .background(isWorkoutActive ? Color.red : Color.green)
            .foregroundColor(.white)
            .cornerRadius(22)
        }
        .buttonStyle(PlainButtonStyle())
        .padding(.top, 8)
    }

    // MARK: - Actions

    private func toggleWorkout() {
        isWorkoutActive.toggle()

        if isWorkoutActive {
            startWorkout()
        } else {
            stopWorkout()
        }
    }

    private func startWorkout() {
        // Start workout tracking
        WorkoutTracking.authorizeHealthKit()
        WorkoutTracking.shared.setSport(selectedSportIndex)
        WorkoutTracking.shared.startWorkOut()
        WorkoutTracking.shared.delegate = WorkoutCoordinator.shared

        // Start watch connectivity
        WatchKitConnection.shared.delegate = WorkoutCoordinator.shared
        WatchKitConnection.shared.startSession()

        // Start metrics update timer
        WorkoutCoordinator.shared.startMetricsUpdates()
    }

    private func stopWorkout() {
        // Stop workout tracking
        WorkoutTracking.shared.stopWorkOut()

        // Stop metrics update timer
        WorkoutCoordinator.shared.stopMetricsUpdates()
    }
}

// MARK: - Workout Coordinator

/// Coordinates between WorkoutTracking, WatchKitConnection, and WorkoutMetrics
class WorkoutCoordinator: WorkoutTrackingDelegate, WatchKitConnectionDelegate {
    static let shared = WorkoutCoordinator()

    private var metricsTimer: Timer?

    private init() {}

    // MARK: - Metrics Update

    func startMetricsUpdates() {
        metricsTimer = Timer.scheduledTimer(withTimeInterval: 1.0, repeats: true) { [weak self] _ in
            self?.updateMetrics()
        }
    }

    func stopMetricsUpdates() {
        metricsTimer?.invalidate()
        metricsTimer = nil
    }

    private func updateMetrics() {
        WorkoutMetrics.shared.updateFromConnection()
    }

    // MARK: - WorkoutTrackingDelegate

    func didReceiveHealthKitHeartRate(_ heartRate: Double) {
        WorkoutMetrics.shared.heartRate = heartRate

        // Send heart rate to iOS app
        WatchKitConnection.shared.sendMessage(message: ["heartRate": "\(heartRate)" as AnyObject])

        // Update other metrics from connection
        WorkoutMetrics.shared.distance = WorkoutTracking.distance
        WorkoutMetrics.shared.calories = WorkoutTracking.kcal

        // Update WorkoutTracking with data from iOS
        WorkoutTracking.distance = WatchKitConnection.distance
        WorkoutTracking.kcal = WatchKitConnection.kcal
        WorkoutTracking.speed = WatchKitConnection.speed
        WorkoutTracking.power = WatchKitConnection.power
        WorkoutTracking.cadence = WatchKitConnection.cadence
        WorkoutTracking.steps = WatchKitConnection.steps
    }

    func didReceiveHealthKitStepCounts(_ stepCounts: Double) {
        // Update step counts if needed
    }

    func didReceiveHealthKitStepCadence(_ stepCadence: Double) {
        WorkoutMetrics.shared.stepCadence = Int(stepCadence)

        // Send step cadence to iOS app
        WatchKitConnection.shared.sendMessage(message: ["stepCadence": "\(Int(stepCadence))" as AnyObject])
    }

    func didReceiveHealthKitDistanceCycling(_ distanceCycling: Double) {
        // Distance is updated from iOS app
    }

    func didReceiveHealthKitActiveEnergyBurned(_ activeEnergyBurned: Double) {
        // Calories updated from iOS app
    }

    // MARK: - WatchKitConnectionDelegate

    func didFinishedActiveSession() {
        print("Watch session activated")
    }

    func didReceiveUserName(_ userName: String) {
        print("Received username: \(userName)")
    }
}

// MARK: - Preview

struct WorkoutContentView_Previews: PreviewProvider {
    static var previews: some View {
        WorkoutContentView()
    }
}
