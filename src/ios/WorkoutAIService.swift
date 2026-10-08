import Foundation
import Darwin

#if canImport(FoundationModels)
import FoundationModels
#endif

fileprivate struct CanonicalWorkoutStep: Codable {
    let name: String
    let durationSeconds: Int?
    let repeatCount: Int?
    let steps: [CanonicalWorkoutStep]?
    let powerWatts: Int?
    let cadenceRpm: Int?
    let resistance: Int?
    let inclinePercent: Double?
    let speedKph: Double?
    let forceSpeed: Bool?
    let fanSpeed: Int?
    let pelotonResistance: Int?
    let heartRateZone: Int?
    let heartRateMin: Int?
    let heartRateMax: Int?
    let mets: Int?
    
    private enum CodingKeys: String, CodingKey {
        case name
        case durationSeconds
        case repeatCount = "repeat"
        case steps
        case powerWatts
        case cadenceRpm
        case resistance
        case inclinePercent
        case speedKph
        case forceSpeed
        case fanSpeed
        case pelotonResistance
        case heartRateZone
        case heartRateMin
        case heartRateMax
        case mets
    }
}

fileprivate struct CanonicalWorkoutPayload: Codable {
    let schemaVersion: Int
    let title: String
    let device: String
    let steps: [CanonicalWorkoutStep]
}

@objc public final class WorkoutAIService: NSObject {

    @objc public func generateCanonicalWorkout(prompt: String,
                                               device: String,
                                               completion: @escaping (NSString?, NSString?) -> Void) {
#if canImport(FoundationModels)
        if #available(iOS 26.0, *) {
            let model = SystemLanguageModel.default
            switch model.availability {
            case .available:
                let instructions = """
                You generate indoor fitness workouts for QZ.
                Return only valid compact JSON.
                The JSON schema is:
                {
                  "schemaVersion": 1,
                  "title": "Short English title",
                  "device": "bike|treadmill|elliptical|rower",
                  "steps": [
                    {
                      "name": "Short English step name",
                      "durationSeconds": 300,
                      "repeat": 6,
                      "steps": [
                        {
                          "name": "Hard",
                          "durationSeconds": 120,
                          "powerWatts": 250
                        },
                        {
                          "name": "Recovery",
                          "durationSeconds": 60,
                          "powerWatts": 140
                        }
                      ],
                      "powerWatts": 200,
                      "cadenceRpm": 85,
                      "resistance": 20,
                      "inclinePercent": 1.0,
                      "speedKph": 10.0,
                      "forceSpeed": true,
                      "fanSpeed": 0,
                      "pelotonResistance": 35,
                      "heartRateZone": 3,
                      "heartRateMin": 120,
                      "heartRateMax": 150,
                      "mets": 8
                    }
                  ]
                }
                Rules:
                - Use only English field names and English step names.
                - Always output schemaVersion = 1.
                - Device must be "\(device)" unless the prompt explicitly asks another supported device.
                - Every leaf step must include durationSeconds.
                - Use repeat blocks when the workout has repeated intervals.
                - A node can either be a leaf step with durationSeconds, or a repeat block with repeat and steps.
                - For bike workouts, prefer only powerWatts targets. Do not set cadenceRpm, resistance, or pelotonResistance.
                - Unless the user explicitly asks for steady, endurance, zone 2, flat, constant, or recovery riding, avoid a single long flat block.
                - Prefer structured workouts with progressions, over-unders, interval sets, pyramids, surges, and clear warmup/cooldown when appropriate.
                - For bike workouts, vary power across steps so the workout is not monotonous by default.
                - Do not include markdown, comments, or explanations.
                - Prefer 3 to 12 steps.
                """

                let session = LanguageModelSession(model: model, instructions: instructions)
                Task {
                    do {
                        let response = try await session.respond(to: prompt)
                        guard let jsonCandidate = self.extractJSONObject(from: response.content) else {
                            completion(nil, "Model response did not contain a valid JSON object")
                            return
                        }
                        do {
                            let normalized = try self.normalizeCanonicalJson(jsonCandidate,
                                                                             fallbackDevice: device,
                                                                             prompt: prompt)
                            completion(normalized as NSString, nil)
                        } catch {
                            completion(nil, "Canonical workout validation failed: \(error.localizedDescription)" as NSString)
                        }
                    } catch {
                        completion(nil, error.localizedDescription as NSString)
                    }
                }
            case .unavailable(let reason):
                completion(nil, "Foundation Models unavailable: \(reason)" as NSString)
            }
            return
        }
#endif
        completion(nil, "Foundation Models are not available on this iOS configuration")
    }

    @objc public func buildQuickWorkoutCanonicalJson(durationMinutes: Int,
                                                     device: String,
                                                     intensity: String) -> String {
        let normalizedDuration = max(10, durationMinutes)
        let warmupMinutes = max(5, min(10, normalizedDuration / 5))
        let cooldownMinutes = max(5, min(10, normalizedDuration / 6))
        let mainMinutes = max(5, normalizedDuration - warmupMinutes - cooldownMinutes)

        let steps = [
            makeStep(name: "Warmup", minutes: warmupMinutes, device: device, intensity: "easy"),
            makeStep(name: "Main Set", minutes: mainMinutes, device: device, intensity: intensity),
            makeStep(name: "Cooldown", minutes: cooldownMinutes, device: device, intensity: "easy")
        ]

        let payload = CanonicalWorkoutPayload(
            schemaVersion: 1,
            title: "\(capitalized(intensity)) \(capitalized(device)) Workout",
            device: normalizedDevice(device),
            steps: steps
        )

        let encoder = JSONEncoder()
        if #available(iOS 13.0, *) {
            encoder.outputFormatting = [.withoutEscapingSlashes]
        } else {
            // Fallback on earlier versions
        }
        let data = try? encoder.encode(payload)
        return String(data: data ?? Data(), encoding: .utf8) ?? "{}"
    }

    private func capitalized(_ value: String) -> String {
        let normalized = normalizedDevice(value)
        if normalized == value {
            return value.prefix(1).uppercased() + value.dropFirst()
        }
        return normalized.prefix(1).uppercased() + normalized.dropFirst()
    }

    private func normalizedDevice(_ value: String) -> String {
        let lower = value.lowercased()
        if lower.contains("row") {
            return "rower"
        }
        if lower.contains("ell") {
            return "elliptical"
        }
        if lower.contains("trea") || lower.contains("run") {
            return "treadmill"
        }
        return "bike"
    }

    private func makeStep(name: String, minutes: Int, device: String, intensity: String) -> CanonicalWorkoutStep {
        let durationSeconds = max(60, minutes * 60)
        let normalizedDevice = normalizedDevice(device)
        let lowerIntensity = intensity.lowercased()

        switch normalizedDevice {
        case "treadmill":
            let speed: Double
            let incline: Double
            switch lowerIntensity {
            case "easy":
                speed = 6.0
                incline = 0.5
            case "hard":
                speed = 11.5
                incline = 1.5
            default:
                speed = 8.5
                incline = 1.0
            }
            return CanonicalWorkoutStep(name: name,
                                        durationSeconds: durationSeconds,
                                        repeatCount: nil,
                                        steps: nil,
                                        powerWatts: nil,
                                        cadenceRpm: nil,
                                        resistance: nil,
                                        inclinePercent: incline,
                                        speedKph: speed,
                                        forceSpeed: true,
                                        fanSpeed: nil,
                                        pelotonResistance: nil,
                                        heartRateZone: nil,
                                        heartRateMin: nil,
                                        heartRateMax: nil,
                                        mets: nil)
        case "elliptical":
            let resistance: Int
            let cadence: Int
            switch lowerIntensity {
            case "easy":
                resistance = 12
                cadence = 72
            case "hard":
                resistance = 26
                cadence = 90
            default:
                resistance = 18
                cadence = 82
            }
            return CanonicalWorkoutStep(name: name,
                                        durationSeconds: durationSeconds,
                                        repeatCount: nil,
                                        steps: nil,
                                        powerWatts: nil,
                                        cadenceRpm: cadence,
                                        resistance: resistance,
                                        inclinePercent: 8.0,
                                        speedKph: nil,
                                        forceSpeed: nil,
                                        fanSpeed: nil,
                                        pelotonResistance: nil,
                                        heartRateZone: nil,
                                        heartRateMin: nil,
                                        heartRateMax: nil,
                                        mets: nil)
        case "rower":
            let power: Int
            let cadence: Int
            switch lowerIntensity {
            case "easy":
                power = 140
                cadence = 20
            case "hard":
                power = 240
                cadence = 30
            default:
                power = 190
                cadence = 25
            }
            return CanonicalWorkoutStep(name: name,
                                        durationSeconds: durationSeconds,
                                        repeatCount: nil,
                                        steps: nil,
                                        powerWatts: power,
                                        cadenceRpm: cadence,
                                        resistance: nil,
                                        inclinePercent: nil,
                                        speedKph: nil,
                                        forceSpeed: nil,
                                        fanSpeed: nil,
                                        pelotonResistance: nil,
                                        heartRateZone: nil,
                                        heartRateMin: nil,
                                        heartRateMax: nil,
                                        mets: nil)
        default:
            let power: Int
            switch lowerIntensity {
            case "easy":
                power = 140
            case "hard":
                power = 260
            default:
                power = 200
            }
            return CanonicalWorkoutStep(name: name,
                                        durationSeconds: durationSeconds,
                                        repeatCount: nil,
                                        steps: nil,
                                        powerWatts: power,
                                        cadenceRpm: nil,
                                        resistance: nil,
                                        inclinePercent: nil,
                                        speedKph: nil,
                                        forceSpeed: nil,
                                        fanSpeed: nil,
                                        pelotonResistance: nil,
                                        heartRateZone: nil,
                                        heartRateMin: nil,
                                        heartRateMax: nil,
                                        mets: nil)
        }
    }

    private func extractJSONObject(from text: String) -> String? {
        guard let start = text.firstIndex(of: "{"), let end = text.lastIndex(of: "}") else {
            return nil
        }
        return String(text[start...end])
    }

    private func normalizeCanonicalJson(_ rawJson: String, fallbackDevice: String, prompt: String) throws -> String {
        let data = Data(rawJson.utf8)
        let decoder = JSONDecoder()
        let payload = try decoder.decode(CanonicalWorkoutPayload.self, from: data)
        let normalizedDeviceKey = payload.device.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty
            ? normalizedDevice(fallbackDevice)
            : normalizedDevice(payload.device)
        let normalizedSteps = normalizeSteps(payload.steps, device: normalizedDeviceKey)
        let normalized = CanonicalWorkoutPayload(
            schemaVersion: 1,
            title: payload.title.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty ? "AI Workout" : payload.title,
            device: normalizedDeviceKey,
            steps: enrichMonotonyIfNeeded(
                expandStructuredPromptIfNeeded(normalizedSteps, device: normalizedDeviceKey, prompt: prompt),
                device: normalizedDeviceKey,
                prompt: prompt
            )
        )
        let encoder = JSONEncoder()
        if #available(iOS 13.0, *) {
            encoder.outputFormatting = [.withoutEscapingSlashes]
        } else {
            // Fallback on earlier versions
        }
        let normalizedData = try encoder.encode(normalized)
        return String(decoding: normalizedData, as: UTF8.self)
    }

    private func expandStructuredPromptIfNeeded(_ steps: [CanonicalWorkoutStep],
                                                device: String,
                                                prompt: String) -> [CanonicalWorkoutStep] {
        let lowerPrompt = prompt.lowercased()
        let wantsWarmup = lowerPrompt.contains("warmup") || lowerPrompt.contains("warm up")
        let wantsCooldown = lowerPrompt.contains("cooldown") || lowerPrompt.contains("cool down")

        guard wantsWarmup && wantsCooldown, steps.count == 1, let onlyStep = steps.first,
              let totalDuration = onlyStep.durationSeconds, totalDuration >= 15 * 60 else {
            return steps
        }

        let totalMinutes = max(15, totalDuration / 60)
        let warmupMinutes = max(5, min(10, totalMinutes / 6))
        let cooldownMinutes = max(5, min(10, totalMinutes / 6))
        let mainMinutes = max(5, totalMinutes - warmupMinutes - cooldownMinutes)

        return [
            makeStep(name: "Warmup", minutes: warmupMinutes, device: device, intensity: "easy"),
            makeStep(name: onlyStep.name.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty ? "Main Set" : onlyStep.name,
                     minutes: mainMinutes,
                     device: device,
                     intensity: inferredIntensity(from: prompt)),
            makeStep(name: "Cooldown", minutes: cooldownMinutes, device: device, intensity: "easy")
        ]
    }

    private func inferredIntensity(from prompt: String) -> String {
        let lower = prompt.lowercased()
        if lower.contains("hard") || lower.contains("threshold") || lower.contains("vo2") || lower.contains("sprint") {
            return "hard"
        }
        if lower.contains("easy") || lower.contains("recovery") || lower.contains("recover") {
            return "easy"
        }
        return "moderate"
    }

    private func enrichMonotonyIfNeeded(_ steps: [CanonicalWorkoutStep], device: String, prompt: String) -> [CanonicalWorkoutStep] {
        guard device == "bike", !shouldPreserveSteadyWorkout(prompt), isFlatBikeWorkout(steps) else {
            return steps
        }

        let totalMinutes = max(20, totalDurationMinutes(for: steps))
        return dynamicBikeWorkout(totalMinutes: totalMinutes, intensity: inferredIntensity(from: prompt))
    }

    private func shouldPreserveSteadyWorkout(_ prompt: String) -> Bool {
        let lower = prompt.lowercased()
        return lower.contains("steady") ||
            lower.contains("endurance") ||
            lower.contains("zone 2") ||
            lower.contains("z2") ||
            lower.contains("flat") ||
            lower.contains("constant") ||
            lower.contains("recovery") ||
            lower.contains("easy spin")
    }

    private func isFlatBikeWorkout(_ steps: [CanonicalWorkoutStep]) -> Bool {
        let leafSteps = flattenedLeafSteps(steps)
        guard !leafSteps.isEmpty else {
            return true
        }

        let powerValues = Set(leafSteps.compactMap { $0.powerWatts })
        if leafSteps.count <= 2 {
            return true
        }
        return powerValues.count <= 1
    }

    private func flattenedLeafSteps(_ steps: [CanonicalWorkoutStep]) -> [CanonicalWorkoutStep] {
        steps.flatMap { step in
            if let nested = step.steps, !nested.isEmpty {
                let repeated = max(1, step.repeatCount ?? 1)
                return (0..<repeated).flatMap { _ in flattenedLeafSteps(nested) }
            }
            return [step]
        }
    }

    private func totalDurationMinutes(for steps: [CanonicalWorkoutStep]) -> Int {
        let totalSeconds = flattenedLeafSteps(steps).reduce(0) { partial, step in
            partial + max(0, step.durationSeconds ?? 0)
        }
        return max(1, totalSeconds / 60)
    }

    private func dynamicBikeWorkout(totalMinutes: Int, intensity: String) -> [CanonicalWorkoutStep] {
        let normalizedDuration = max(20, totalMinutes)
        var plan: [(String, Int, String)] = []

        switch intensity {
        case "hard":
            plan = [
                ("Warmup", 4, "easy"),
                ("Build", 4, "moderate"),
                ("Primer", 2, "hard"),
                ("Hard Interval 1", 3, "hard"),
                ("Recovery 1", 2, "easy"),
                ("Hard Interval 2", 3, "hard"),
                ("Recovery 2", 2, "easy"),
                ("Hard Interval 3", 3, "hard"),
                ("Recovery 3", 2, "easy"),
                ("Hard Interval 4", 3, "hard"),
                ("Recovery 4", 2, "easy"),
                ("Over", 2, "hard"),
                ("Under", 2, "moderate"),
                ("Over", 2, "hard"),
                ("Under", 2, "moderate"),
                ("Over", 2, "hard"),
                ("Recovery", 2, "easy"),
                ("Cooldown", 6, "easy")
            ]
        case "easy":
            plan = [
                ("Warmup", 5, "easy"),
                ("Build", 5, "moderate"),
                ("Tempo Lift 1", 4, "moderate"),
                ("Easy Spin 1", 2, "easy"),
                ("Tempo Lift 2", 4, "moderate"),
                ("Easy Spin 2", 2, "easy"),
                ("Tempo Lift 3", 4, "moderate"),
                ("Easy Spin 3", 2, "easy"),
                ("Steady Finish", 6, "easy"),
                ("Cooldown", 5, "easy")
            ]
        default:
            plan = [
                ("Warmup", 5, "easy"),
                ("Build", 5, "moderate"),
                ("Tempo 1", 6, "moderate"),
                ("Reset 1", 2, "easy"),
                ("Tempo 2", 6, "moderate"),
                ("Reset 2", 2, "easy"),
                ("Tempo 3", 6, "moderate"),
                ("Surge 1", 1, "hard"),
                ("Settle 1", 2, "moderate"),
                ("Surge 2", 1, "hard"),
                ("Settle 2", 2, "moderate"),
                ("Cooldown", 5, "easy")
            ]
        }

        plan = scaledPlan(plan, totalMinutes: normalizedDuration)
        return plan.map { name, minutes, stepIntensity in
            makeStep(name: name, minutes: minutes, device: "bike", intensity: stepIntensity)
        }
    }

    private func scaledPlan(_ plan: [(String, Int, String)], totalMinutes: Int) -> [(String, Int, String)] {
        let baseTotal = max(1, plan.reduce(0) { $0 + $1.1 })
        var scaled = plan.enumerated().map { index, item -> (String, Int, String) in
            let raw = Double(item.1) * Double(totalMinutes) / Double(baseTotal)
            let rounded = index == plan.count - 1 ? max(1, totalMinutes) : max(1, Int(raw.rounded()))
            return (item.0, rounded, item.2)
        }

        var currentTotal = scaled.reduce(0) { $0 + $1.1 }
        if currentTotal == totalMinutes {
            return scaled
        }

        var adjustableIndex = max(0, min(1, scaled.count - 1))
        if scaled.count > 2 {
            adjustableIndex = scaled.count / 2
        }

        while currentTotal < totalMinutes {
            scaled[adjustableIndex].1 += 1
            currentTotal += 1
        }
        while currentTotal > totalMinutes && scaled[adjustableIndex].1 > 1 {
            scaled[adjustableIndex].1 -= 1
            currentTotal -= 1
        }

        return scaled
    }

    private func normalizeSteps(_ steps: [CanonicalWorkoutStep], device: String) -> [CanonicalWorkoutStep] {
        steps.compactMap { step in
            if let nested = step.steps, !nested.isEmpty {
                let normalizedNested = normalizeSteps(nested, device: device)
                guard !normalizedNested.isEmpty else {
                    return nil
                }
                return CanonicalWorkoutStep(
                    name: step.name,
                    durationSeconds: nil,
                    repeatCount: max(1, step.repeatCount ?? 1),
                    steps: normalizedNested,
                    powerWatts: nil,
                    cadenceRpm: nil,
                    resistance: nil,
                    inclinePercent: nil,
                    speedKph: nil,
                    forceSpeed: nil,
                    fanSpeed: nil,
                    pelotonResistance: nil,
                    heartRateZone: nil,
                    heartRateMin: nil,
                    heartRateMax: nil,
                    mets: nil
                )
            }

            guard let durationSeconds = step.durationSeconds, durationSeconds > 0 else {
                return nil
            }

            return CanonicalWorkoutStep(
                name: step.name.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty ? "Step" : step.name,
                durationSeconds: durationSeconds,
                repeatCount: nil,
                steps: nil,
                powerWatts: step.powerWatts,
                cadenceRpm: device == "bike" ? nil : step.cadenceRpm,
                resistance: device == "bike" ? nil : step.resistance,
                inclinePercent: step.inclinePercent,
                speedKph: step.speedKph,
                forceSpeed: step.forceSpeed,
                fanSpeed: step.fanSpeed,
                pelotonResistance: device == "bike" ? nil : step.pelotonResistance,
                heartRateZone: step.heartRateZone,
                heartRateMin: step.heartRateMin,
                heartRateMax: step.heartRateMax,
                mets: step.mets
            )
        }
    }
}

@_cdecl("ios_workout_ai_generate_canonical_json")
public func ios_workout_ai_generate_canonical_json(_ prompt: UnsafePointer<CChar>?,
                                                   _ device: UnsafePointer<CChar>?,
                                                   _ outJson: UnsafeMutablePointer<UnsafeMutablePointer<CChar>?>?,
                                                   _ outError: UnsafeMutablePointer<UnsafeMutablePointer<CChar>?>?) -> Bool {
    let promptText = prompt.map { String(cString: $0) } ?? ""
    let deviceText = device.map { String(cString: $0) } ?? ""
    let service = WorkoutAIService()

    let semaphore = DispatchSemaphore(value: 0)
    var resultJson: String?
    var resultError: String?

    service.generateCanonicalWorkout(prompt: promptText, device: deviceText) { json, error in
        resultJson = json as String?
        resultError = error as String?
        semaphore.signal()
    }

    let waitResult = semaphore.wait(timeout: DispatchTime.now() + .seconds(20))
    if waitResult == .timedOut && resultJson == nil && resultError == nil {
        resultError = "Timed out while waiting for Apple Foundation Models"
    }

    if let outJson {
        outJson.pointee = resultJson.flatMap { strdup($0) }
    }
    if let outError {
        outError.pointee = resultError.flatMap { strdup($0) }
    }
    return resultJson != nil
}
