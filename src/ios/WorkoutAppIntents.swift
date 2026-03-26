import Foundation
import AppIntents

@_silgen_name("ios_workout_ai_queue_canonical_workout")
private func ios_workout_ai_queue_canonical_workout(_ canonicalJson: UnsafePointer<CChar>?,
                                                    _ autoStart: Bool,
                                                    _ outError: UnsafeMutablePointer<UnsafeMutablePointer<CChar>?>?) -> Bool

@_silgen_name("ios_workout_ai_free_string")
private func ios_workout_ai_free_string(_ value: UnsafeMutablePointer<CChar>?)

@available(iOS 16.0, *)
private func queueCanonicalWorkoutFromIntent(_ canonicalJson: String,
                                             autoStart: Bool) throws {
    var errorPointer: UnsafeMutablePointer<CChar>?
    let queued = canonicalJson.withCString { jsonPointer in
        ios_workout_ai_queue_canonical_workout(jsonPointer, autoStart, &errorPointer)
    }

    let errorMessage: String? = {
        guard let errorPointer else {
            return nil
        }
        defer { ios_workout_ai_free_string(errorPointer) }
        return String(cString: errorPointer)
    }()

    if !queued {
        throw NSError(domain: "QZWorkoutIntent",
                      code: 1,
                      userInfo: [NSLocalizedDescriptionKey: errorMessage ?? "Unable to queue workout"])
    }
}

@available(iOS 16.0, *)
enum WorkoutShortcutDevice: String, AppEnum {
    case bike
    case treadmill
    case elliptical
    case rower

    static var typeDisplayRepresentation = TypeDisplayRepresentation(name: "Workout Device")
    static var caseDisplayRepresentations: [WorkoutShortcutDevice: DisplayRepresentation] = [
        .bike: DisplayRepresentation(title: "Bike"),
        .treadmill: DisplayRepresentation(title: "Treadmill"),
        .elliptical: DisplayRepresentation(title: "Elliptical"),
        .rower: DisplayRepresentation(title: "Rower")
    ]
}

@available(iOS 16.0, *)
enum WorkoutShortcutIntensity: String, AppEnum {
    case easy
    case moderate
    case hard

    static var typeDisplayRepresentation = TypeDisplayRepresentation(name: "Workout Intensity")
    static var caseDisplayRepresentations: [WorkoutShortcutIntensity: DisplayRepresentation] = [
        .easy: DisplayRepresentation(title: "Easy"),
        .moderate: DisplayRepresentation(title: "Moderate"),
        .hard: DisplayRepresentation(title: "Hard")
    ]
}

@available(iOS 16.0, *)
struct StartQuickWorkoutIntent: AppIntent {
    static var title: LocalizedStringResource = "Start Quick Workout"
    static var description = IntentDescription("Create a structured workout and optionally start it in QZ.")
    static var openAppWhenRun = true

    @Parameter(title: "Duration Minutes", default: 45)
    var durationMinutes: Int

    @Parameter(title: "Device", default: .bike)
    var device: WorkoutShortcutDevice

    @Parameter(title: "Intensity", default: .moderate)
    var intensity: WorkoutShortcutIntensity

    @Parameter(title: "Start Immediately", default: true)
    var startImmediately: Bool

    func perform() async throws -> some IntentResult & ProvidesDialog {
        let service = WorkoutAIService()
        let canonicalJson = service.buildQuickWorkoutCanonicalJson(durationMinutes: durationMinutes,
                                                                  device: device.rawValue,
                                                                  intensity: intensity.rawValue)
        try queueCanonicalWorkoutFromIntent(canonicalJson, autoStart: startImmediately)

        let message: String
        if startImmediately {
            message = "Starting a \(durationMinutes)-minute \(intensity.rawValue) \(device.rawValue) workout in QZ."
        } else {
            message = "Created a \(durationMinutes)-minute \(intensity.rawValue) \(device.rawValue) workout in QZ."
        }
        return .result(dialog: IntentDialog(stringLiteral: message))
    }
}

@available(iOS 16.0, *)
struct GenerateCustomWorkoutIntent: AppIntent {
    static var title: LocalizedStringResource = "Generate Custom Workout"
    static var description = IntentDescription("Use Apple on-device AI to generate a workout and optionally start it in QZ.")
    static var openAppWhenRun = true

    @Parameter(title: "Request")
    var request: String

    @Parameter(title: "Device", default: .bike)
    var device: WorkoutShortcutDevice

    @Parameter(title: "Start Immediately", default: true)
    var startImmediately: Bool

    func perform() async throws -> some IntentResult & ProvidesDialog {
        let service = WorkoutAIService()
        let canonicalJson = try await withCheckedThrowingContinuation { (continuation: CheckedContinuation<String, Error>) in
            service.generateCanonicalWorkout(prompt: request, device: device.rawValue) { json, error in
                if let json {
                    continuation.resume(returning: json as String)
                } else {
                    continuation.resume(throwing: NSError(domain: "QZWorkoutIntent",
                                                          code: 2,
                                                          userInfo: [NSLocalizedDescriptionKey: (error as String?) ?? "AI generation failed"]))
                }
            }
        }

        do {
            try queueCanonicalWorkoutFromIntent(canonicalJson, autoStart: startImmediately)
        } catch {
            throw NSError(domain: "QZWorkoutIntent",
                          code: 3,
                          userInfo: [NSLocalizedDescriptionKey: error.localizedDescription])
        }

        let message = startImmediately ? "Generating and starting your custom workout in QZ."
                                       : "Generating your custom workout in QZ."
        return .result(dialog: IntentDialog(stringLiteral: message))
    }
}

@available(iOS 16.0, *)
struct QZWorkoutShortcuts: AppShortcutsProvider {
    static var appShortcuts: [AppShortcut] {
        AppShortcut(
            intent: StartQuickWorkoutIntent(),
            phrases: [
                "Start a workout in \(.applicationName)",
                "Create a workout in \(.applicationName)"
            ],
            shortTitle: "Quick Workout",
            systemImageName: "figure.indoor.cycle"
        )
        AppShortcut(
            intent: GenerateCustomWorkoutIntent(),
            phrases: [
                "Generate a custom workout in \(.applicationName)",
                "Create a custom workout in \(.applicationName)"
            ],
            shortTitle: "Custom Workout",
            systemImageName: "waveform.and.mic"
        )
    }
}
