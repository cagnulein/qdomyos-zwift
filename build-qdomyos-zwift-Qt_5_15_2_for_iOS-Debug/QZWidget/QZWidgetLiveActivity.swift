//
//  QZWidgetLiveActivity.swift
//  QDomyos-Zwift Live Activity Widget
//
//  Displays workout metrics on Dynamic Island and Lock Screen
//

import ActivityKit
import WidgetKit
import SwiftUI

// QZWorkoutAttributes is defined in QZWorkoutAttributes.swift (shared file)

// MARK: - Live Activity Widget
@available(iOS 16.1, *)
struct QZWidgetLiveActivity: Widget {
    var body: some WidgetConfiguration {
        ActivityConfiguration(for: QZWorkoutAttributes.self) { context in
            // Lock screen/banner UI
            LockScreenLiveActivityView(context: context)
        } dynamicIsland: { context in
            DynamicIsland {
                // Expanded UI
                DynamicIslandExpandedRegion(.leading) {
                    VStack(alignment: .leading, spacing: 4) {
                        let speed = context.attributes.useMiles ? context.state.speed * 0.621371 : context.state.speed
                        let speedUnit = context.attributes.useMiles ? "mph" : "km/h"
                        Label("\(Int(speed)) \(speedUnit)", systemImage: "speedometer")
                            .font(.caption)
                        Label("\(context.state.heartRate) bpm", systemImage: "heart.fill")
                            .font(.caption)
                            .foregroundColor(.red)
                    }
                }

                DynamicIslandExpandedRegion(.trailing) {
                    VStack(alignment: .trailing, spacing: 4) {
                        Label("\(Int(context.state.power)) W", systemImage: "bolt.fill")
                            .font(.caption)
                            .foregroundColor(.yellow)
                        Label("\(Int(context.state.cadence)) rpm", systemImage: "arrow.clockwise")
                            .font(.caption)
                    }
                }

                DynamicIslandExpandedRegion(.center) {
                    // Empty or can add more info
                }

                DynamicIslandExpandedRegion(.bottom) {
                    HStack {
                        let distanceKm = context.state.distance / 1000.0
                        let distance = context.attributes.useMiles ? distanceKm * 0.621371 : distanceKm
                        let distanceUnit = context.attributes.useMiles ? "mi" : "km"
                        Label(String(format: "%.2f \(distanceUnit)", distance), systemImage: "map")
                        Spacer()
                        Label("\(Int(context.state.kcal)) kcal", systemImage: "flame.fill")
                            .foregroundColor(.orange)
                    }
                    .font(.caption)
                    .padding(.horizontal)
                }
            } compactLeading: {
                // Compact leading (left side of Dynamic Island)
                HStack(spacing: 2) {
                    Image(systemName: "heart.fill")
                        .foregroundColor(.red)
                    Text("\(context.state.heartRate)")
                        .font(.caption2)
                }
            } compactTrailing: {
                // Compact trailing (right side of Dynamic Island)
                HStack(spacing: 2) {
                    Image(systemName: "bolt.fill")
                        .foregroundColor(.yellow)
                    Text("\(Int(context.state.power))")
                        .font(.caption2)
                }
            } minimal: {
                // Minimal view (when multiple activities)
                Image(systemName: "figure.run")
            }
        }
    }
}

// MARK: - Lock Screen View
@available(iOS 16.1, *)
struct LockScreenLiveActivityView: View {
    let context: ActivityViewContext<QZWorkoutAttributes>

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            HStack {
                Image(systemName: "figure.indoor.cycle")
                    .foregroundColor(.blue)
                Text(context.attributes.deviceName)
                    .font(.headline)
                Spacer()
            }

            HStack(spacing: 16) {
                let speed = context.attributes.useMiles ? context.state.speed * 0.621371 : context.state.speed
                let speedUnit = context.attributes.useMiles ? "mph" : "km/h"
                MetricView(icon: "speedometer", value: String(format: "%.1f", speed), unit: speedUnit)
                MetricView(icon: "heart.fill", value: "\(context.state.heartRate)", unit: "bpm", color: .red)
                MetricView(icon: "bolt.fill", value: "\(Int(context.state.power))", unit: "W", color: .yellow)
            }

            HStack(spacing: 16) {
                let distanceKm = context.state.distance / 1000.0
                let distance = context.attributes.useMiles ? distanceKm * 0.621371 : distanceKm
                let distanceUnit = context.attributes.useMiles ? "mi" : "km"
                MetricView(icon: "arrow.clockwise", value: "\(Int(context.state.cadence))", unit: "rpm")
                MetricView(icon: "map", value: String(format: "%.2f", distance), unit: distanceUnit)
                MetricView(icon: "flame.fill", value: "\(Int(context.state.kcal))", unit: "kcal", color: .orange)
            }
        }
        .padding()
    }
}

// MARK: - Metric View Component
struct MetricView: View {
    let icon: String
    let value: String
    let unit: String
    var color: Color = .primary

    var body: some View {
        VStack(spacing: 2) {
            Image(systemName: icon)
                .foregroundColor(color)
                .font(.caption)
            Text(value)
                .font(.system(.body, design: .rounded))
                .fontWeight(.semibold)
            Text(unit)
                .font(.caption2)
                .foregroundColor(.secondary)
        }
        .frame(maxWidth: .infinity)
    }
}

// MARK: - Preview
@available(iOS 16.1, *)
struct QZWidgetLiveActivity_Previews: PreviewProvider {
    static let attributes = QZWorkoutAttributes(deviceName: "QZ Bike", useMiles: false)
    static let contentState = QZWorkoutAttributes.ContentState(
        speed: 25.5,
        cadence: 85,
        power: 200,
        heartRate: 145,
        distance: 12500,  // meters (will be displayed as 12.50 km or 7.77 mi)
        kcal: 320,
        useMiles: false
    )

    static var previews: some View {
        attributes
            .previewContext(contentState, viewKind: .dynamicIsland(.compact))
            .previewDisplayName("Island Compact")
        attributes
            .previewContext(contentState, viewKind: .dynamicIsland(.expanded))
            .previewDisplayName("Island Expanded")
        attributes
            .previewContext(contentState, viewKind: .dynamicIsland(.minimal))
            .previewDisplayName("Minimal")
        attributes
            .previewContext(contentState, viewKind: .content)
            .previewDisplayName("Lock Screen")
    }
}
