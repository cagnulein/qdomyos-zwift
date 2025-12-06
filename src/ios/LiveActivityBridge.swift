//
//  LiveActivityBridge.swift
//  QDomyos-Zwift
//
//  Bridge between C++/Objective-C and Swift Live Activities
//

import Foundation
import ActivityKit
import UIKit

@available(iOS 16.1, *)
@objc public class LiveActivityBridge: NSObject {

    private var currentActivity: Activity<QZWorkoutAttributes>?

    @objc public func startActivity(deviceName: String, useMiles: Bool) {
        // Check if Live Activities are supported and enabled
        guard ActivityAuthorizationInfo().areActivitiesEnabled else {
            print("Live Activities are not enabled")
            return
        }

        // End any existing activity first
        endActivity()

        let attributes = QZWorkoutAttributes(deviceName: deviceName, useMiles: useMiles)
        let initialState = QZWorkoutAttributes.ContentState(
            speed: 0.0,
            cadence: 0.0,
            power: 0.0,
            heartRate: 0,
            distance: 0.0,
            kcal: 0.0,
            useMiles: useMiles
        )

        do {
            let activity = try Activity<QZWorkoutAttributes>.request(
                attributes: attributes,
                contentState: initialState,
                pushType: nil
            )
            currentActivity = activity
            print("✅ Live Activity started successfully (useMiles: \(useMiles))")
        } catch {
            print("❌ Failed to start Live Activity: \(error.localizedDescription)")
        }
    }

    @objc public func updateActivity(speed: Double, cadence: Double, power: Double, heartRate: Int, distance: Double, kcal: Double, useMiles: Bool) {
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
            kcal: kcal,
            useMiles: useMiles
        )

        Task {
            // Set stale date to 60 seconds in the future
            // If app stops updating (because it was force-killed), the Live Activity will auto-dismiss after 60 seconds
            // This is the ONLY reliable way to handle force-kill scenarios, as iOS doesn't send any notifications
            // when an app is terminated from the app switcher
            let staleDate = Date().addingTimeInterval(60)
            await activity.update(
                ActivityContent<QZWorkoutAttributes.ContentState>(
                    state: updatedState,
                    staleDate: staleDate
                )
            )
        }
    }

    @objc public func endActivity() {
        guard let activity = currentActivity else {
            return
        }

        Task {
            // Use .immediate to dismiss from both Dynamic Island and Lock Screen immediately
            await activity.end(using: nil, dismissalPolicy: .immediate)
            currentActivity = nil
            print("Live Activity ended")
        }
    }

    @objc public func isActivityRunning() -> Bool {
        return currentActivity != nil
    }
}
