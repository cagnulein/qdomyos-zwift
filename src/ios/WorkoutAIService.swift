import Foundation

#if canImport(FoundationModels)
import FoundationModels
#endif

fileprivate struct CanonicalWorkoutStep: Codable {
    let name: String
    let durationSeconds: Int?
    let repeat: Int?
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
                            let normalized = try self.normalizeCanonicalJson(jsonCandidate, fallbackDevice: device)
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
        encoder.outputFormatting = [.withoutEscapingSlashes]
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
                                        repeat: nil,
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
                                        repeat: nil,
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
                                        repeat: nil,
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
            let resistance: Int
            let cadence: Int
            let power: Int
            switch lowerIntensity {
            case "easy":
                resistance = 14
                cadence = 80
                power = 140
            case "hard":
                resistance = 30
                cadence = 95
                power = 260
            default:
                resistance = 22
                cadence = 88
                power = 200
            }
            return CanonicalWorkoutStep(name: name,
                                        durationSeconds: durationSeconds,
                                        repeat: nil,
                                        steps: nil,
                                        powerWatts: power,
                                        cadenceRpm: cadence,
                                        resistance: resistance,
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

    private func normalizeCanonicalJson(_ rawJson: String, fallbackDevice: String) throws -> String {
        let data = Data(rawJson.utf8)
        let decoder = JSONDecoder()
        let payload = try decoder.decode(CanonicalWorkoutPayload.self, from: data)
        let normalized = CanonicalWorkoutPayload(
            schemaVersion: 1,
            title: payload.title.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty ? "AI Workout" : payload.title,
            device: payload.device.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty ? normalizedDevice(fallbackDevice) : normalizedDevice(payload.device),
            steps: normalizeSteps(payload.steps)
        )
        let encoder = JSONEncoder()
        encoder.outputFormatting = [.withoutEscapingSlashes]
        let normalizedData = try encoder.encode(normalized)
        return String(decoding: normalizedData, as: UTF8.self)
    }

    private func normalizeSteps(_ steps: [CanonicalWorkoutStep]) -> [CanonicalWorkoutStep] {
        steps.compactMap { step in
            if let nested = step.steps, !nested.isEmpty {
                let normalizedNested = normalizeSteps(nested)
                guard !normalizedNested.isEmpty else {
                    return nil
                }
                return CanonicalWorkoutStep(
                    name: step.name,
                    durationSeconds: nil,
                    repeat: max(1, step.repeat ?? 1),
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
                repeat: nil,
                steps: nil,
                powerWatts: step.powerWatts,
                cadenceRpm: step.cadenceRpm,
                resistance: step.resistance,
                inclinePercent: step.inclinePercent,
                speedKph: step.speedKph,
                forceSpeed: step.forceSpeed,
                fanSpeed: step.fanSpeed,
                pelotonResistance: step.pelotonResistance,
                heartRateZone: step.heartRateZone,
                heartRateMin: step.heartRateMin,
                heartRateMax: step.heartRateMax,
                mets: step.mets
            )
        }
    }
}
