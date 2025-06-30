//
//  WorkoutView.swift
//  watchkit Extension
//
//  Apple Watch Workout UI with paginated metrics display
//

import SwiftUI

struct WorkoutView: View {
    @EnvironmentObject var workoutManager: WorkoutManager
    @State private var currentPage = 0
    
    var body: some View {
        GeometryReader { geometry in
            VStack(spacing: 0) {
                // Status Bar
                HStack {
                    if workoutManager.workoutState == .paused {
                        Text("Paused")
                            .font(.system(size: 12, weight: .semibold))
                            .foregroundColor(.green)
                    }
                    Spacer()
                    Text(getCurrentTime())
                        .font(.system(size: 12, weight: .medium))
                        .foregroundColor(.primary)
                }
                .padding(.horizontal, 4)
                .padding(.top, 2)
                
                // Main Content with Pagination
                TabView(selection: $currentPage) {
                    // Page 1: Primary Metrics
                    PrimaryMetricsView()
                        .environmentObject(workoutManager)
                        .tag(0)
                    
                    // Page 2: Secondary Metrics  
                    SecondaryMetricsView()
                        .environmentObject(workoutManager)
                        .tag(1)
                    
                    // Page 3: Activity Rings & Additional Metrics
                    ActivityRingsView()
                        .environmentObject(workoutManager)
                        .tag(2)
                }
                .tabViewStyle(PageTabViewStyle(indexDisplayMode: .automatic))
                .frame(maxWidth: .infinity, maxHeight: .infinity)
                
                // Control Buttons
                HStack(spacing: 20) {
                    Button(action: {
                        if workoutManager.workoutState == .running {
                            workoutManager.pauseWorkout()
                        } else if workoutManager.workoutState == .paused {
                            workoutManager.resumeWorkout()
                        }
                    }) {
                        Image(systemName: workoutManager.workoutState == .running ? "pause.fill" : "play.fill")
                            .font(.system(size: 20, weight: .semibold))
                            .foregroundColor(.white)
                    }
                    .frame(width: 44, height: 44)
                    .background(Color.orange)
                    .clipShape(Circle())
                    
                    Button(action: {
                        workoutManager.stopWorkout()
                    }) {
                        Image(systemName: "stop.fill")
                            .font(.system(size: 20, weight: .semibold))
                            .foregroundColor(.white)
                    }
                    .frame(width: 44, height: 44)
                    .background(Color.red)
                    .clipShape(Circle())
                }
                .padding(.bottom, 8)
            }
        }
        .navigationBarHidden(true)
        .edgesIgnoringSafeArea(.all)
    }
    
    private func getCurrentTime() -> String {
        let formatter = DateFormatter()
        formatter.timeStyle = .short
        return formatter.string(from: Date())
    }
}

// MARK: - Primary Metrics View
struct PrimaryMetricsView: View {
    @EnvironmentObject var workoutManager: WorkoutManager
    
    var body: some View {
        VStack(spacing: 4) {
            // Main Timer
            Text(formatElapsedTime(workoutManager.elapsedTime))
                .font(.system(size: 36, weight: .light, design: .rounded))
                .foregroundColor(.yellow)
                .multilineTextAlignment(.center)
                .lineLimit(1)
                .minimumScaleFactor(0.8)
            
            // Split Time (if applicable)
            if workoutManager.selectedSport == .cycling || workoutManager.selectedSport == .running {
                Text(formatSplitTime())
                    .font(.system(size: 14, weight: .medium))
                    .foregroundColor(.pink)
            }
            
            // Speed
            HStack {
                Text(formatSpeed())
                    .font(.system(size: 18, weight: .semibold))
                    .foregroundColor(.white)
                Text("MPH")
                    .font(.system(size: 12, weight: .medium))
                    .foregroundColor(.gray)
            }
            
            // Distance
            HStack {
                Image(systemName: "location")
                    .font(.system(size: 10))
                    .foregroundColor(.green)
                Text(formatDistance())
                    .font(.system(size: 16, weight: .medium))
                    .foregroundColor(.white)
                Text("SPLIT")
                    .font(.system(size: 10, weight: .medium))
                    .foregroundColor(.gray)
            }
            
            // Heart Rate
            HStack {
                Text(formatHeartRate())
                    .font(.system(size: 18, weight: .semibold))
                    .foregroundColor(.white)
                Text("❤️")
                    .font(.system(size: 14))
            }
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .background(Color.black)
    }
    
    private func formatElapsedTime(_ time: TimeInterval) -> String {
        let hours = Int(time) / 3600
        let minutes = Int(time) % 3600 / 60
        let seconds = Int(time) % 60
        let milliseconds = Int((time.truncatingRemainder(dividingBy: 1)) * 100)
        
        if hours > 0 {
            return String(format: "%d:%02d:%02d.%02d", hours, minutes, seconds, milliseconds)
        } else {
            return String(format: "%02d:%02d.%02d", minutes, seconds, milliseconds)
        }
    }
    
    private func formatSplitTime() -> String {
        // Calculate split time based on distance
        let splitDistance = 1.0 // 1 mile split
        if workoutManager.distance > 0 {
            let splitTime = workoutManager.elapsedTime / workoutManager.distance * splitDistance
            let minutes = Int(splitTime) / 60
            let seconds = Int(splitTime) % 60
            return String(format: "%d:%02d SPLIT", minutes, seconds)
        }
        return "0:00 SPLIT"
    }
    
    private func formatSpeed() -> String {
        return String(format: "%.1f", workoutManager.speed)
    }
    
    private func formatDistance() -> String {
        if Locale.current.measurementSystem == "Metric" {
            return String(format: "%.2fKM", workoutManager.distance * 1.60934)
        } else {
            return String(format: "%.2fMI", workoutManager.distance)
        }
    }
    
    private func formatHeartRate() -> String {
        return String(format: "%.0fBPM", workoutManager.heartRate)
    }
}

// MARK: - Secondary Metrics View  
struct SecondaryMetricsView: View {
    @EnvironmentObject var workoutManager: WorkoutManager
    
    var body: some View {
        VStack(spacing: 8) {
            // Sport Icon
            Image(systemName: workoutManager.selectedSport.icon)
                .font(.system(size: 24, weight: .semibold))
                .foregroundColor(.green)
            
            // Main Timer (smaller)
            Text(formatElapsedTime(workoutManager.elapsedTime))
                .font(.system(size: 28, weight: .light, design: .rounded))
                .foregroundColor(.yellow)
                .multilineTextAlignment(.center)
            
            // Elevation Chart Placeholder (if cycling/running)
            if workoutManager.selectedSport == .cycling || workoutManager.selectedSport == .running {
                ElevationChartView()
                    .frame(height: 40)
            }
            
            // Elevation Metrics
            VStack(alignment: .leading, spacing: 2) {
                HStack {
                    Text("69FT")
                        .font(.system(size: 16, weight: .semibold))
                        .foregroundColor(.green)
                    Text("ELEV")
                        .font(.system(size: 10, weight: .medium))
                        .foregroundColor(.gray)
                    Spacer()
                    Text("GAINED")
                        .font(.system(size: 10, weight: .medium))
                        .foregroundColor(.gray)
                }
                
                HStack {
                    Text("816FT")
                        .font(.system(size: 16, weight: .semibold))
                        .foregroundColor(.white)
                    Text("ELEV")
                        .font(.system(size: 10, weight: .medium))
                        .foregroundColor(.gray)
                    Spacer()
                }
            }
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .background(Color.black)
    }
    
    private func formatElapsedTime(_ time: TimeInterval) -> String {
        let hours = Int(time) / 3600
        let minutes = Int(time) % 3600 / 60
        let seconds = Int(time) % 60
        let milliseconds = Int((time.truncatingRemainder(dividingBy: 1)) * 100)
        
        if hours > 0 {
            return String(format: "%d:%02d:%02d.%02d", hours, minutes, seconds, milliseconds)
        } else {
            return String(format: "%02d:%02d.%02d", minutes, seconds, milliseconds)
        }
    }
}

// MARK: - Activity Rings View
struct ActivityRingsView: View {
    @EnvironmentObject var workoutManager: WorkoutManager
    
    var body: some View {
        VStack(spacing: 8) {
            // Main Timer (smaller)
            Text(formatElapsedTime(workoutManager.elapsedTime))
                .font(.system(size: 24, weight: .light, design: .rounded))
                .foregroundColor(.yellow)
                .multilineTextAlignment(.center)
            
            // Activity Rings
            ZStack {
                // Move Ring (Red)
                Circle()
                    .stroke(Color.red.opacity(0.3), lineWidth: 8)
                    .frame(width: 80, height: 80)
                
                Circle()
                    .trim(from: 0, to: min(workoutManager.calories / 500, 1.0))
                    .stroke(Color.red, style: StrokeStyle(lineWidth: 8, lineCap: .round))
                    .frame(width: 80, height: 80)
                    .rotationEffect(.degrees(-90))
                
                // Exercise Ring (Green)  
                Circle()
                    .stroke(Color.green.opacity(0.3), lineWidth: 8)
                    .frame(width: 64, height: 64)
                
                Circle()
                    .trim(from: 0, to: min(workoutManager.elapsedTime / 1800, 1.0)) // 30 min goal
                    .stroke(Color.green, style: StrokeStyle(lineWidth: 8, lineCap: .round))
                    .frame(width: 64, height: 64)
                    .rotationEffect(.degrees(-90))
                
                // Stand Ring (Blue)
                Circle()
                    .stroke(Color.blue.opacity(0.3), lineWidth: 8) 
                    .frame(width: 48, height: 48)
                
                Circle()
                    .trim(from: 0, to: 0.25) // Static for demo
                    .stroke(Color.blue, style: StrokeStyle(lineWidth: 8, lineCap: .round))
                    .frame(width: 48, height: 48)
                    .rotationEffect(.degrees(-90))
            }
            
            // Ring Stats
            VStack(spacing: 4) {
                HStack {
                    Text("MOVE")
                        .font(.system(size: 10, weight: .medium))
                        .foregroundColor(.red)
                    Spacer()
                    Text("\(Int(workoutManager.calories))/500")
                        .font(.system(size: 12, weight: .semibold))
                        .foregroundColor(.red)
                }
                
                HStack {
                    Text("EXERCISE") 
                        .font(.system(size: 10, weight: .medium))
                        .foregroundColor(.green)
                    Spacer()
                    Text("\(Int(workoutManager.elapsedTime/60))/30")
                        .font(.system(size: 12, weight: .semibold))
                        .foregroundColor(.green)
                }
                
                HStack {
                    Text("STAND")
                        .font(.system(size: 10, weight: .medium))
                        .foregroundColor(.blue)
                    Spacer()
                    Text("3/12")
                        .font(.system(size: 12, weight: .semibold))
                        .foregroundColor(.blue)
                }
            }
            .padding(.horizontal, 16)
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .background(Color.black)
    }
    
    private func formatElapsedTime(_ time: TimeInterval) -> String {
        let hours = Int(time) / 3600
        let minutes = Int(time) % 3600 / 60
        let seconds = Int(time) % 60
        let milliseconds = Int((time.truncatingRemainder(dividingBy: 1)) * 100)
        
        if hours > 0 {
            return String(format: "%d:%02d:%02d.%02d", hours, minutes, seconds, milliseconds)
        } else {
            return String(format: "%02d:%02d.%02d", minutes, seconds, milliseconds)
        }
    }
}

// MARK: - Elevation Chart View
struct ElevationChartView: View {
    var body: some View {
        HStack(alignment: .bottom, spacing: 1) {
            ForEach(0..<30, id: \.self) { index in
                Rectangle()
                    .fill(Color.green)
                    .frame(width: 2, height: CGFloat.random(in: 5...25))
            }
        }
        .background(
            HStack {
                Text("30 MIN AGO")
                    .font(.system(size: 8, weight: .medium))
                    .foregroundColor(.gray)
                Spacer()
                Text("NOW")
                    .font(.system(size: 8, weight: .medium))
                    .foregroundColor(.gray)
            }
            .padding(.horizontal, 4),
            alignment: .bottom
        )
    }
}

// MARK: - Extensions
extension Locale {
    var measurementSystem: String? {
        return (self as NSLocale).object(forKey: NSLocale.Key.measurementSystem) as? String
    }
}

#Preview {
    WorkoutView()
        .environmentObject(WorkoutManager())
}