//
//  LiveActivityBridge.swift
//  QDomyos-Zwift
//
//  Bridge between C++/Objective-C and Swift Live Activities
//

import Foundation
import ActivityKit

@available(iOS 16.1, *)
@objc public class LiveActivityBridge: NSObject {

    private var currentActivity: Activity<QZWorkoutAttributes>?

    @objc public func startActivity(deviceName: String) {
        // Check if Live Activities are supported and enabled
        guard ActivityAuthorizationInfo().areActivitiesEnabled else {
            print("Live Activities are not enabled")
            return
        }

        // End any existing activity first
        endActivity()

        let attributes = QZWorkoutAttributes(deviceName: deviceName)
        let initialState = QZWorkoutAttributes.ContentState(
            speed: 0.0,
            cadence: 0.0,
            power: 0.0,
            heartRate: 0,
            distance: 0.0,
            kcal: 0.0
        )

        do {
            let activity = try Activity<QZWorkoutAttributes>.request(
                attributes: attributes,
                contentState: initialState,
                pushType: nil
            )
            currentActivity = activity
            print("✅ Live Activity started successfully")
        } catch {
            print("❌ Failed to start Live Activity: \(error.localizedDescription)")
        }
    }

    @objc public func updateActivity(speed: Double, cadence: Double, power: Double, heartRate: Int, distance: Double, kcal: Double) {
        guard let activity = currentActivity else {
            print("No active Live Activity to update")
            return
        }

        let updatedState = QZWorkoutAttributes.ContentState(
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
            await activity.end(using: nil, dismissalPolicy: .immediate)
            currentActivity = nil
            print("Live Activity ended")
        }
    }

    @objc public func isActivityRunning() -> Bool {
        return currentActivity != nil
    }
}
