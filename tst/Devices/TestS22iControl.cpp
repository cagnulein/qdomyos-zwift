#include "../../src/devices/nordictrackifitadbbike/s22icontrol.h"
#include <cassert>

int main() {
    // Slope and resistance must both survive an FTMS simulation command.
    assert(S22iControl::inclineTarget(3.5, 4, false, true) == 3.5);
    assert(!S22iControl::discardResistanceAfterIncline(true));
    assert(!S22iControl::applyGearToIncline(true));
    assert(S22iControl::resistanceAfterGearChange(8, 5, 4) == 9);
    assert(S22iControl::resistanceAfterGearChange(8, 2, 4) == 6);
    // Existing bikes keep their prior incline and resistance routing.
    assert(S22iControl::inclineTarget(3.5, 4, false, false) == 7.5);
    assert(S22iControl::inclineTarget(3.5, 4, true, false) == 3.5);
    assert(S22iControl::discardResistanceAfterIncline(false));
    assert(S22iControl::applyGearToIncline(false));
}
