//
//  SportSelectionView.swift
//  watchkit Extension
//
//  Apple-style sport selection interface
//

import SwiftUI

struct SportSelectionView: View {
    @EnvironmentObject var workoutManager: WorkoutManager
    @EnvironmentObject var watchConnection: WatchConnectionManager
    @State private var showingWorkoutTypeSelection = false
    
    var body: some View {
        ScrollView {
            VStack(spacing: 16) {
                // Header
                VStack(spacing: 4) {
                    Text(watchConnection.userName)
                        .font(.system(size: 18, weight: .semibold))
                        .foregroundColor(.primary)
                        .multilineTextAlignment(.center)
                    
                    Text("Choose a workout")
                        .font(.system(size: 12, weight: .medium))
                        .foregroundColor(.secondary)
                }
                .padding(.top, 8)
                
                // Sport Selection Grid
                LazyVGrid(columns: [
                    GridItem(.flexible(), spacing: 8),
                    GridItem(.flexible(), spacing: 8)
                ], spacing: 12) {
                    ForEach(WorkoutManager.Sport.allCases, id: \.self) { sport in
                        SportSelectionButton(
                            sport: sport,
                            isSelected: workoutManager.selectedSport == sport
                        ) {
                            workoutManager.selectSport(sport)
                        }
                    }
                }
                .padding(.horizontal, 8)
                
                // Current Metrics Display
                if workoutManager.stepCadence > 0 || workoutManager.heartRate > 0 {
                    VStack(spacing: 8) {
                        Divider()
                            .background(Color.gray.opacity(0.3))
                        
                        Text("Current Activity")
                            .font(.system(size: 12, weight: .medium))
                            .foregroundColor(.secondary)
                        
                        HStack(spacing: 20) {
                            if workoutManager.stepCadence > 0 {
                                MetricView(
                                    value: "\(workoutManager.stepCadence)",
                                    unit: "STEP CAD",
                                    color: .blue
                                )
                            }
                            
                            if workoutManager.heartRate > 0 {
                                MetricView(
                                    value: "\(Int(workoutManager.heartRate))",
                                    unit: "BPM",
                                    color: .red
                                )
                            }
                        }
                    }
                    .padding(.horizontal)
                }
                
                // Start Button
                Button(action: {
                    workoutManager.startWorkout()
                }) {
                    HStack {
                        Image(systemName: "play.fill")
                            .font(.system(size: 16, weight: .semibold))
                        Text("Start \(workoutManager.selectedSport.displayName)")
                            .font(.system(size: 16, weight: .semibold))
                    }
                    .foregroundColor(.white)
                    .frame(maxWidth: .infinity)
                    .frame(height: 44)
                    .background(Color.green)
                    .cornerRadius(22)
                }
                .padding(.horizontal)
                .padding(.top, 8)
            }
            .padding(.bottom, 20)
        }
        .navigationBarHidden(true)
    }
}

// MARK: - Sport Selection Button
struct SportSelectionButton: View {
    let sport: WorkoutManager.Sport
    let isSelected: Bool
    let action: () -> Void
    
    var body: some View {
        Button(action: action) {
            VStack(spacing: 8) {
                ZStack {
                    Circle()
                        .fill(isSelected ? Color.green : Color.gray.opacity(0.2))
                        .frame(width: 60, height: 60)
                    
                    Image(systemName: sport.icon)
                        .font(.system(size: 24, weight: .medium))
                        .foregroundColor(isSelected ? .white : .primary)
                }
                
                Text(sport.displayName)
                    .font(.system(size: 12, weight: .medium))
                    .foregroundColor(.primary)
                    .multilineTextAlignment(.center)
                    .lineLimit(2)
            }
        }
        .buttonStyle(PlainButtonStyle())
        .frame(maxWidth: .infinity)
        .padding(.vertical, 8)
        .background(
            RoundedRectangle(cornerRadius: 12)
                .fill(isSelected ? Color.green.opacity(0.1) : Color.clear)
        )
        .overlay(
            RoundedRectangle(cornerRadius: 12)
                .stroke(isSelected ? Color.green : Color.clear, lineWidth: 2)
        )
    }
}

// MARK: - Metric View
struct MetricView: View {
    let value: String
    let unit: String
    let color: Color
    
    var body: some View {
        VStack(spacing: 2) {
            Text(value)
                .font(.system(size: 16, weight: .semibold, design: .rounded))
                .foregroundColor(color)
            
            Text(unit)
                .font(.system(size: 10, weight: .medium))
                .foregroundColor(.secondary)
        }
    }
}

// MARK: - Workout Type Selection (Modal)
struct WorkoutTypeSelectionView: View {
    @Binding var isPresented: Bool
    @EnvironmentObject var workoutManager: WorkoutManager
    
    let workoutTypes = [
        ("Indoor Cycling", "bicycle"),
        ("Outdoor Cycling", "location"),
        ("Virtual Cycling", "tv"),
    ]
    
    var body: some View {
        NavigationView {
            List {
                ForEach(workoutTypes, id: \.0) { type in
                    Button(action: {
                        // Handle workout type selection
                        isPresented = false
                    }) {
                        HStack {
                            Image(systemName: type.1)
                                .font(.system(size: 16))
                                .foregroundColor(.green)
                                .frame(width: 24)
                            
                            Text(type.0)
                                .font(.system(size: 16))
                                .foregroundColor(.primary)
                            
                            Spacer()
                        }
                        .padding(.vertical, 4)
                    }
                    .buttonStyle(PlainButtonStyle())
                }
            }
            .navigationTitle("Cycling")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") {
                        isPresented = false
                    }
                }
            }
        }
    }
}

#Preview {
    SportSelectionView()
        .environmentObject(WorkoutManager())
        .environmentObject(WatchConnectionManager())
}