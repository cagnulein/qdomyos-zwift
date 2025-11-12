//
//  MetricTileView.swift
//  qdomyos-zwift Watch Extension
//
//  Created by Claude Code for SwiftUI Redesign
//  Design inspired by Apple Workout app
//

import SwiftUI

/// A metric tile view with Apple Workout-like design
struct MetricTileView: View {
    let tileType: TileType
    @ObservedObject var metrics: WorkoutMetrics

    var body: some View {
        VStack(alignment: .leading, spacing: 2) {
            // Top: Icon and label
            HStack(spacing: 4) {
                Image(systemName: tileType.iconName)
                    .font(.system(size: 12, weight: .medium))
                    .foregroundColor(tileColor)

                Text(tileType.displayName.uppercased())
                    .font(.system(size: 10, weight: .semibold))
                    .foregroundColor(.gray)

                Spacer()
            }

            // Bottom: Value and unit
            HStack(alignment: .firstTextBaseline, spacing: 2) {
                Text(tileType.getValue(from: metrics))
                    .font(.system(size: 24, weight: .semibold, design: .rounded))
                    .foregroundColor(.white)
                    .minimumScaleFactor(0.5)
                    .lineLimit(1)

                if !tileType.unit.isEmpty {
                    Text(tileType.unit)
                        .font(.system(size: 12, weight: .medium))
                        .foregroundColor(.gray)
                }

                Spacer()
            }
        }
        .padding(.horizontal, 8)
        .padding(.vertical, 6)
        .frame(maxWidth: .infinity, alignment: .leading)
        .background(
            RoundedRectangle(cornerRadius: 10)
                .fill(Color(white: 0.15))
        )
    }

    /// Color for the tile icon (mimicking Apple Workout colors)
    private var tileColor: Color {
        switch tileType {
        case .heart:
            return Color(red: 1.0, green: 0.27, blue: 0.23) // Red
        case .speed:
            return Color(red: 0.35, green: 0.93, blue: 0.42) // Green
        case .cadence:
            return Color(red: 0.20, green: 0.68, blue: 0.98) // Blue
        case .calories:
            return Color(red: 1.0, green: 0.62, blue: 0.04) // Orange
        case .watt, .avgWatt:
            return Color(red: 1.0, green: 0.92, blue: 0.23) // Yellow
        case .odometer, .elevation:
            return Color(red: 0.35, green: 0.93, blue: 0.42) // Green
        case .pace, .elapsed:
            return Color(red: 0.64, green: 0.71, blue: 0.78) // Gray-blue
        case .resistance, .targetResistance:
            return Color(red: 0.95, green: 0.53, blue: 0.91) // Pink
        case .inclination:
            return Color(red: 0.50, green: 0.85, blue: 0.95) // Light blue
        }
    }
}

// MARK: - Preview

struct MetricTileView_Previews: PreviewProvider {
    static var previews: some View {
        let metrics = WorkoutMetrics.shared
        metrics.heartRate = 142
        metrics.speed = 18.5
        metrics.cadence = 85
        metrics.calories = 245
        metrics.distance = 5.23
        metrics.watt = 185

        return Group {
            MetricTileView(tileType: .heart, metrics: metrics)
                .previewLayout(.sizeThatFits)
                .padding()
                .background(Color.black)

            MetricTileView(tileType: .speed, metrics: metrics)
                .previewLayout(.sizeThatFits)
                .padding()
                .background(Color.black)

            MetricTileView(tileType: .watt, metrics: metrics)
                .previewLayout(.sizeThatFits)
                .padding()
                .background(Color.black)
        }
    }
}
