//
//  LiveActivityManager.swift
//  qdomyos-zwift
//
//  iOS Live Activity manager for fitness metrics
//

import Foundation
import ActivityKit
import SwiftUI

// Define the attributes for the Live Activity
@available(iOS 16.1, *)
struct FitnessActivityAttributes: ActivityAttributes {
    public struct ContentState: Codable, Hashable {
        var speed: Double
        var cadence: Double
        var power: Double
        var heartRate: Int
        var distance: Double
        var kcal: Double
    }

    var deviceName: String
}

// Live Activity manager class
@available(iOS 16.1, *)
@objc public class LiveActivityManager: NSObject {

    private var currentActivity: Activity<FitnessActivityAttributes>?

    @objc public func startActivity(deviceName: String) {
        // Check if Live Activities are supported
        guard ActivityAuthorizationInfo().areActivitiesEnabled else {
            print("Live Activities are not enabled")
            return
        }

        // End any existing activity first
        endActivity()

        let attributes = FitnessActivityAttributes(deviceName: deviceName)
        let initialState = FitnessActivityAttributes.ContentState(
            speed: 0.0,
            cadence: 0.0,
            power: 0.0,
            heartRate: 0,
            distance: 0.0,
            kcal: 0.0
        )

        do {
            currentActivity = try Activity<FitnessActivityAttributes>.request(
                attributes: attributes,
                contentState: initialState,
                pushType: nil
            )
            print("Live Activity started successfully")
        } catch {
            print("Failed to start Live Activity: \(error.localizedDescription)")
        }
    }

    @objc public func updateActivity(speed: Double, cadence: Double, power: Double, heartRate: Int, distance: Double, kcal: Double) {
        guard let activity = currentActivity else {
            print("No active Live Activity to update")
            return
        }

        let updatedState = FitnessActivityAttributes.ContentState(
            speed: speed,
            cadence: cadence,
            power: power,
            heartRate: heartRate,
            distance: distance,
            kcal: kcal
        )

        Task {
            await activity.update(using: updatedState)
        }
    }

    @objc public func endActivity() {
        guard let activity = currentActivity else {
            return
        }

        Task {
            await activity.end(dismissalPolicy: .immediate)
            currentActivity = nil
            print("Live Activity ended")
        }
    }

    @objc public func isActivityRunning() -> Bool {
        return currentActivity != nil
    }
}
