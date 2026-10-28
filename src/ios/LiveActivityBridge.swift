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

    // Preserve the original 10-second inactivity timeout for normal workouts.
    // Only extend it when iOS background keepalive is actually running and the
    // app is not active: Live Activity updates are then throttled to 5 seconds.
    private let inactivityTimeout: TimeInterval = 10.0
    private let backgroundKeepAliveInactivityTimeout: TimeInterval = 30.0

    // MARK: - Private Methods

    private func startInactivityTimer(timeout: TimeInterval) {
        // Cancel existing timer
        inactivityTimer?.invalidate()

        // Use RunLoop.common so the timer continues to run in common run-loop modes.
        inactivityTimer = Timer.scheduledTimer(
            timeInterval: timeout,
            target: self,
            selector: #selector(inactivityTimerFired(_:)),
            userInfo: nil,
            repeats: false
        )
        RunLoop.current.add(inactivityTimer!, forMode: .common)
    }

    @objc private func inactivityTimerFired(_ timer: Timer) {
        print("⚠️ No Live Activity updates received for \(timer.timeInterval) seconds - auto-closing")
        endActivity()
    }

    private func stopInactivityTimer() {
        inactivityTimer?.invalidate()
        inactivityTimer = nil
    }

    // MARK: - Public Methods

    @objc public func startActivity(deviceName: String, useMiles: Bool, compactLeadingMetric: String,
                                    compactTrailingMetric: String) {
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
            useMiles: useMiles,
            compactLeadingMetric: compactLeadingMetric,
            compactLeadingValue: 0,
            compactTrailingMetric: compactTrailingMetric,
            compactTrailingValue: 0
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
            startInactivityTimer(timeout: inactivityTimeout)
        } catch {
            print("❌ Failed to start Live Activity: \(error.localizedDescription)")
        }
    }

    @objc public func updateActivity(speed: Double, cadence: Double, power: Double, heartRate: Int,
                                     distance: Double, kcal: Double, compactLeadingMetric: String,
                                     compactLeadingValue: Int, compactTrailingMetric: String,
                                     compactTrailingValue: Int, useMiles: Bool,
                                     extendedBackgroundTimeout: Bool) {
        guard let activity = currentActivity else {
            print("No active Live Activity to update")
            return
        }

        // Preserve the old 10-second behavior unless the opt-in keepalive is
        // running while QZ is in background (updates are throttled there).
        let timeout = extendedBackgroundTimeout ? backgroundKeepAliveInactivityTimeout : inactivityTimeout
        startInactivityTimer(timeout: timeout)

        let updatedState = QZWorkoutAttributes.ContentState(
            speed: speed,
            cadence: cadence,
            power: power,
            heartRate: heartRate,
            distance: distance,
            kcal: kcal,
            useMiles: useMiles,
            compactLeadingMetric: compactLeadingMetric,
            compactLeadingValue: compactLeadingValue,
            compactTrailingMetric: compactTrailingMetric,
            compactTrailingValue: compactTrailingValue
        )

        Task {
            // Also set stale date as fallback indicator when app is killed
            // This won't close the Live Activity, but at least marks it as outdated visually
            let staleDate = Date().addingTimeInterval(timeout + 5)
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
