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

@available(iOS 16.1, *)
private struct CompactMetricStyle {
    let icon: String
    let color: Color
}

@available(iOS 16.1, *)
private func compactMetricStyle(for metric: String) -> CompactMetricStyle {
    switch metric {
    case "Heart Rate":
        return CompactMetricStyle(icon: "heart.fill", color: .red)
    case "Watt", "AVG Watt", "Target Power", "FTP", "Watt/Kg":
        return CompactMetricStyle(icon: "bolt.fill", color: .yellow)
    case "Cadence", "Target Cadence":
        return CompactMetricStyle(icon: "arrow.clockwise", color: .primary)
    case "Speed", "Pace":
        return CompactMetricStyle(icon: "speedometer", color: .blue)
    case "Calories", "Weight Loss":
        return CompactMetricStyle(icon: "flame.fill", color: .orange)
    case "Odometer", "Elevation":
        return CompactMetricStyle(icon: "map", color: .green)
    case "Resistance", "Peloton Resistance", "Target Resistance", "Target Peloton Resistance", "Peloton Offset":
        return CompactMetricStyle(icon: "dial.low.fill", color: .mint)
    case "Lap Elapsed", "Elapsed", "Moving Time", "Date Time":
        return CompactMetricStyle(icon: "timer", color: .primary)
    case "Fan":
        return CompactMetricStyle(icon: "fan.fill", color: .cyan)
    case "Inclination":
        return CompactMetricStyle(icon: "mountain.2.fill", color: .brown)
    case "Jouls":
        return CompactMetricStyle(icon: "bolt.circle.fill", color: .yellow)
    default:
        return CompactMetricStyle(icon: "gauge.with.dots.needle.33percent", color: .primary)
    }
}

@available(iOS 16.1, *)
private struct CompactMetricView: View {
    let metric: String
    let value: Int

    var body: some View {
        let style = compactMetricStyle(for: metric)

        HStack(spacing: 2) {
            Image(systemName: style.icon)
                .foregroundColor(style.color)
            Text("\(value)")
                .font(.caption2)
        }
    }
}

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
                CompactMetricView(metric: context.state.compactLeadingMetric, value: context.state.compactLeadingValue)
            } compactTrailing: {
                CompactMetricView(metric: context.state.compactTrailingMetric, value: context.state.compactTrailingValue)
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
        useMiles: false,
        compactLeadingMetric: "Heart Rate",
        compactLeadingValue: 145,
        compactTrailingMetric: "Watt",
        compactTrailingValue: 200
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
