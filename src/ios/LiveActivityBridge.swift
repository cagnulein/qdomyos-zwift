//
//  LiveActivityBridge.swift
//  QDomyos-Zwift
//
//  Bridge between C++/Objective-C and Swift Live Activities
//

import Foundation
import ActivityKit
import UIKit

@available(iOS 16.2, *)
@objc public class LiveActivityBridge: NSObject {

    private var currentActivity: Activity<QZWorkoutAttributes>?
    private var inactivityTimer: Timer?

    // Timeout in seconds - if no updates received, auto-close the Live Activity
    private let inactivityTimeout: TimeInterval = 10.0

    // MARK: - Private Methods

    private func startInactivityTimer() {
        // Cancel existing timer
        inactivityTimer?.invalidate()

        // Create new timer that fires after inactivityTimeout seconds
        // Using RunLoop.common mode ensures timer works even when app is in background
        inactivityTimer = Timer.scheduledTimer(
            timeInterval: inactivityTimeout,
            target: self,
            selector: #selector(inactivityTimerFired),
            userInfo: nil,
            repeats: false
        )
        RunLoop.current.add(inactivityTimer!, forMode: .common)
    }

    @objc private func inactivityTimerFired() {
        print("⚠️ No Live Activity updates received for \(inactivityTimeout) seconds - auto-closing")
        endActivity()
    }

    private func stopInactivityTimer() {
        inactivityTimer?.invalidate()
        inactivityTimer = nil
    }

    // MARK: - Public Methods

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

            // Start inactivity timer
            startInactivityTimer()
        } catch {
            print("❌ Failed to start Live Activity: \(error.localizedDescription)")
        }
    }

    @objc public func updateActivity(speed: Double, cadence: Double, power: Double, heartRate: Int, distance: Double, kcal: Double, useMiles: Bool) {
        guard let activity = currentActivity else {
            print("No active Live Activity to update")
            return
        }

        // Reset inactivity timer - we received an update, so activity is still alive
        startInactivityTimer()

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
            // Also set stale date as fallback indicator when app is killed
            // This won't close the Live Activity, but at least marks it as outdated visually
            let staleDate = Date().addingTimeInterval(inactivityTimeout + 5)
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
            // Stop timer even if no activity
            stopInactivityTimer()
            return
        }

        // Stop the inactivity timer
        stopInactivityTimer()

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
