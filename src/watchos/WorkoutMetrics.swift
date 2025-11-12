//
//  WorkoutMetrics.swift
//  qdomyos-zwift Watch Extension
//
//  Created by Claude Code for SwiftUI Redesign
//

import Foundation
import Combine

/// Observable class containing all workout metrics
class WorkoutMetrics: ObservableObject {
    // MARK: - Published Properties

    @Published var heartRate: Double = 0.0
    @Published var speed: Double = 0.0
    @Published var cadence: Double = 0.0
    @Published var stepCadence: Int = 0
    @Published var power: Double = 0.0
    @Published var calories: Double = 0.0
    @Published var distance: Double = 0.0
    @Published var pace: String = "--:--"
    @Published var resistance: Int = 0
    @Published var watt: Double = 0.0
    @Published var avgWatt: Double = 0.0
    @Published var elevation: Double = 0.0
    @Published var elapsedTime: TimeInterval = 0
    @Published var inclination: Double = 0.0
    @Published var targetResistance: Int = 0
    @Published var steps: Int = 0

    // MARK: - Sport Type

    @Published var sportType: SportType = .bike

    enum SportType: Int {
        case bike = 0
        case run = 1
        case walk = 2
        case elliptical = 3
        case rowing = 4

        var displayName: String {
            switch self {
            case .bike: return "Bike"
            case .run: return "Run"
            case .walk: return "Walk"
            case .elliptical: return "Elliptical"
            case .rowing: return "Rowing"
            }
        }
    }

    // MARK: - Singleton

    static let shared = WorkoutMetrics()

    private init() {}

    // MARK: - Helper Methods

    /// Update metrics from WatchKitConnection
    func updateFromConnection() {
        self.heartRate = Double(WatchKitConnection.currentHeartRate)
        self.distance = WatchKitConnection.distance
        self.calories = WatchKitConnection.kcal
        self.cadence = WatchKitConnection.cadence
        self.power = WatchKitConnection.power
        self.speed = WatchKitConnection.speed
        self.steps = WatchKitConnection.steps
        self.stepCadence = WatchKitConnection.stepCadence

        // Calculate derived metrics
        updateDerivedMetrics()
    }

    /// Calculate pace from speed
    private func updateDerivedMetrics() {
        if speed > 0 {
            let minutesPerMile = 60.0 / speed
            let minutes = Int(minutesPerMile)
            let seconds = Int((minutesPerMile - Double(minutes)) * 60)
            pace = String(format: "%d:%02d", minutes, seconds)
        } else {
            pace = "--:--"
        }

        watt = power
        avgWatt = power // TODO: Calculate actual average
    }
}
