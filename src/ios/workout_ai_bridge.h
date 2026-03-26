#ifndef WORKOUT_AI_BRIDGE_H
#define WORKOUT_AI_BRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif

bool ios_workout_ai_generate_canonical_json(const char *prompt, const char *device, char **outJson, char **outError);
bool ios_workout_ai_queue_canonical_workout(const char *canonicalJson, bool autoStart, char **outError);
void ios_workout_ai_consume_pending_request_if_needed(void);
void ios_workout_ai_free_string(char *value);

#ifdef __cplusplus
}
#endif

#endif // WORKOUT_AI_BRIDGE_H
