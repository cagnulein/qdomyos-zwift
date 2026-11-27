#ifndef FITSHOWSTOPCOMMAND_H
#define FITSHOWSTOPCOMMAND_H

#include <cstdint>

constexpr uint8_t FITSHOW_CONTROL_STOP = 3;
constexpr uint8_t FITSHOW_CONTROL_PAUSE = 6;
constexpr uint8_t FITSHOW_STATUS_PAUSED = 10;

// Explicit Stop must not depend on the workout's paused state.
constexpr uint8_t fitshowStopCommand(bool pauseRequested, bool merachT12) {
    return !pauseRequested ? FITSHOW_CONTROL_STOP
                           : (merachT12 ? FITSHOW_STATUS_PAUSED : FITSHOW_CONTROL_PAUSE);
}

// Only the standard Pause command can fall back to Stop, after exhausting
// the existing retries and only while that pause request is still current.
constexpr bool fitshowShouldFallbackToStop(uint8_t command, bool pauseStillRequested) {
    return command == FITSHOW_CONTROL_PAUSE && pauseStillRequested;
}

#endif // FITSHOWSTOPCOMMAND_H
