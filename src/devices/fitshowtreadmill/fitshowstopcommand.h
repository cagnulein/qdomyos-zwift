#ifndef FITSHOWSTOPCOMMAND_H
#define FITSHOWSTOPCOMMAND_H

#include <cstdint>

constexpr uint8_t FITSHOW_CONTROL_STOP = 3;
constexpr uint8_t FITSHOW_CONTROL_PAUSE = 6;
constexpr uint8_t FITSHOW_STATUS_PAUSED = 10;

// Some controllers do not acknowledge the standard Pause command. For a
// confirmed stop-only controller, use the working Stop command even on Pause.
constexpr uint8_t fitshowStopCommand(bool pauseRequested, bool merachT12, bool stopOnlyController) {
    return !pauseRequested || stopOnlyController ? FITSHOW_CONTROL_STOP
                                                  : (merachT12 ? FITSHOW_STATUS_PAUSED : FITSHOW_CONTROL_PAUSE);
}

#endif // FITSHOWSTOPCOMMAND_H
