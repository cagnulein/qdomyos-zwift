//
//  QZWorkoutAttributes.swift
//  QDomyos-Zwift
//
//  Shared attributes for Live Activities
//  MUST be included in both qdomyoszwift and QZWidget targets
//

import Foundation
import ActivityKit

@available(iOS 16.1, *)
public struct QZWorkoutAttributes: ActivityAttributes {
    public struct ContentState: Codable, Hashable {
        public var speed: Double
        public var cadence: Double
        public var power: Double
        public var heartRate: Int
        public var distance: Double
        public var kcal: Double

        public init(speed: Double, cadence: Double, power: Double, heartRate: Int, distance: Double, kcal: Double) {
            self.speed = speed
            self.cadence = cadence
            self.power = power
            self.heartRate = heartRate
            self.distance = distance
            self.kcal = kcal
        }
    }

    public var deviceName: String
    public var useMiles: Bool

    public init(deviceName: String, useMiles: Bool) {
        self.deviceName = deviceName
        self.useMiles = useMiles
    }
}
