//
//  TileConfiguration.swift
//  qdomyos-zwift Watch Extension
//
//  Created by Claude Code for SwiftUI Redesign
//

import Foundation
import SwiftUI

/// Enum representing all available tile types matching iOS app settings
enum TileType: String, Codable, CaseIterable {
    case speed
    case inclination
    case cadence
    case elevation
    case calories
    case odometer
    case pace
    case resistance
    case watt
    case avgWatt = "avgwatt"
    case heart = "heart_rate"
    case elapsed
    case targetResistance = "target_resistance"

    /// Display name for the tile
    var displayName: String {
        switch self {
        case .speed: return "Speed"
        case .inclination: return "Incline"
        case .cadence: return "Cadence"
        case .elevation: return "Elevation"
        case .calories: return "Calories"
        case .odometer: return "Distance"
        case .pace: return "Pace"
        case .resistance: return "Resistance"
        case .watt: return "Power"
        case .avgWatt: return "Avg Power"
        case .heart: return "Heart Rate"
        case .elapsed: return "Time"
        case .targetResistance: return "Target"
        }
    }

    /// Unit suffix for the metric
    var unit: String {
        switch self {
        case .speed: return "mph"
        case .inclination: return "%"
        case .cadence: return "rpm"
        case .elevation: return "ft"
        case .calories: return "cal"
        case .odometer: return "mi"
        case .pace: return "/mi"
        case .resistance, .targetResistance: return ""
        case .watt, .avgWatt: return "w"
        case .heart: return "bpm"
        case .elapsed: return ""
        }
    }

    /// SF Symbol name for the tile icon
    var iconName: String {
        switch self {
        case .speed: return "speedometer"
        case .inclination: return "arrow.up.right"
        case .cadence: return "repeat.circle"
        case .elevation: return "mountain.2"
        case .calories: return "flame"
        case .odometer: return "map"
        case .pace: return "timer"
        case .resistance, .targetResistance: return "gauge"
        case .watt, .avgWatt: return "bolt"
        case .heart: return "heart.fill"
        case .elapsed: return "clock"
        }
    }

    /// Get current value from metrics
    func getValue(from metrics: WorkoutMetrics) -> String {
        switch self {
        case .speed:
            return String(format: "%.1f", metrics.speed)
        case .inclination:
            return String(format: "%.1f", metrics.inclination)
        case .cadence:
            return String(format: "%.0f", metrics.cadence)
        case .elevation:
            return String(format: "%.0f", metrics.elevation)
        case .calories:
            return String(format: "%.0f", metrics.calories)
        case .odometer:
            return String(format: "%.2f", metrics.distance)
        case .pace:
            return metrics.pace
        case .resistance:
            return String(format: "%d", metrics.resistance)
        case .watt:
            return String(format: "%.0f", metrics.watt)
        case .avgWatt:
            return String(format: "%.0f", metrics.avgWatt)
        case .heart:
            return String(format: "%.0f", metrics.heartRate)
        case .elapsed:
            let minutes = Int(metrics.elapsedTime) / 60
            let seconds = Int(metrics.elapsedTime) % 60
            return String(format: "%d:%02d", minutes, seconds)
        case .targetResistance:
            return String(format: "%d", metrics.targetResistance)
        }
    }
}

/// Manages tile configuration from iOS app settings
class TileConfiguration: ObservableObject {
    @Published var enabledTiles: [TileType] = []

    static let shared = TileConfiguration()

    private init() {
        loadDefaultTiles()
    }

    /// Load default enabled tiles (matching iOS defaults from settings-tiles.qml)
    private func loadDefaultTiles() {
        // Default enabled tiles based on settings-tiles.qml defaults
        enabledTiles = [
            .heart,
            .speed,
            .cadence,
            .calories,
            .odometer,
            .watt
        ]
    }

    /// Update enabled tiles from settings dictionary received from iOS
    func updateFromSettings(_ settings: [String: Any]) {
        var tiles: [(TileType, Int)] = []

        // Parse all tile_*_enabled and tile_*_order settings
        for (key, value) in settings {
            if key.hasPrefix("tile_") && key.hasSuffix("_enabled") {
                guard let enabled = value as? Bool, enabled else { continue }

                // Extract tile name
                let tileName = key
                    .replacingOccurrences(of: "tile_", with: "")
                    .replacingOccurrences(of: "_enabled", with: "")

                // Get corresponding order
                let orderKey = "tile_\(tileName)_order"
                let order = settings[orderKey] as? Int ?? 0

                // Map to TileType
                if let tileType = mapTileType(from: tileName) {
                    tiles.append((tileType, order))
                }
            }
        }

        // Sort by order and extract tile types
        tiles.sort { $0.1 < $1.1 }
        enabledTiles = tiles.map { $0.0 }

        // Limit to 6 tiles for watch display
        if enabledTiles.count > 6 {
            enabledTiles = Array(enabledTiles.prefix(6))
        }
    }

    /// Map tile name from settings to TileType
    private func mapTileType(from settingName: String) -> TileType? {
        switch settingName {
        case "speed": return .speed
        case "inclination": return .inclination
        case "cadence": return .cadence
        case "elevation": return .elevation
        case "calories": return .calories
        case "odometer": return .odometer
        case "pace": return .pace
        case "resistance": return .resistance
        case "watt": return .watt
        case "avgwatt": return .avgWatt
        case "heart": return .heart
        case "elapsed": return .elapsed
        case "target_resistance": return .targetResistance
        default: return nil
        }
    }
}
