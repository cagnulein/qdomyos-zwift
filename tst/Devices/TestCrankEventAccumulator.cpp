#include "../../src/devices/nordictrackifitadbbike/crankeventaccumulator.h"
#include <cassert>

int main() {
    CrankEventAccumulator accumulator;
    uint16_t event = 0;
    uint64_t count = 0;
    // 80 RPM for 30 seconds, at 10 Hz: 40 revolutions, not 300.
    for (int t = 0; t <= 30000; t += 100)
        count += accumulator.update(t, 80, event);
    assert(count == 40 && event == 30720);
    // Variable RPM, irregular sample times and multiple events per poll.
    CrankEventAccumulator variable;
    event = 0;
    assert(variable.update(0, 60, event) == 0);
    assert(variable.update(400, 120, event) == 0);
    assert(variable.update(1250, 120, event) == 2);
    assert(event == 1228);
    assert(variable.update(1450, 0, event) == 0 && event == 1228);
    assert(variable.update(2000, 0, event) == 0 && event == 1228);
    assert(variable.update(3000, 60, event) == 0);
    assert(variable.update(4000, 60, event) == 1 && event == 4096);
    assert(variable.update(7001, 60, event) == 0 && event == 4096);
    assert(variable.update(8001, 60, event) == 1);
    // Timestamp wraps after 64 seconds, but the revolution count continues.
    CrankEventAccumulator wrapping;
    count = 0;
    event = 0;
    for (int t = 0; t <= 65000; t += 1000)
        count += wrapping.update(t, 60, event);
    assert(count == 65 && event == 1024);
    assert(wrapping.update(65000, 60, event) == 0 && event == 1024);
    assert(wrapping.update(66000, 0, event) == 0 && event == 1024);
}
