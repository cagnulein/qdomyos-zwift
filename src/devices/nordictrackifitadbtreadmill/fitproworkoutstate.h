#ifndef FITPROWORKOUTSTATE_H
#define FITPROWORKOUTSTATE_H

enum class FitProInitialWorkoutState {
    None,
    Paused,
    Stopped,
};

inline FitProInitialWorkoutState fitproInitialWorkoutState(int previousState, int currentState) {
    if (previousState != 0) return FitProInitialWorkoutState::None;
    if (currentState == 4) return FitProInitialWorkoutState::Paused;
    if (currentState == 1 || currentState == 5) return FitProInitialWorkoutState::Stopped;
    return FitProInitialWorkoutState::None;
}

#endif  // FITPROWORKOUTSTATE_H
