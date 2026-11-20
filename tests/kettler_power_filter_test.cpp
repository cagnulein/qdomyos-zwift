#include "../src/devices/kettlerusbbike/kettler_power_filter.h"
#include <cassert>
int main() {
    KettlerPowerFilter f;
    assert(f.apply(10, 60, 0) == 10);
    assert(f.apply(140, 64, 100) == 140);
    assert(f.apply(10, 64, 200) == 140);
    assert(f.apply(10, 64, 4199) == 140);
    assert(f.apply(10, 64, 4200) == 10);
    assert(f.apply(10, 64, 4300) == 10);
    assert(f.apply(150, 64, 4400) == 150);
    assert(f.apply(10, 0, 4500) == 10);
    assert(f.apply(80, 60, 5000) == 80);
    assert(f.apply(10, 60, 5100) == 80);
    assert(f.apply(90, 60, 5200) == 90);
    assert(f.apply(10, 60, 5300) == 90);
    f.reset();
    assert(f.apply(10, 60, 5400) == 10);
    assert(f.apply(100, 60, 6000) == 100);
    assert(f.apply(5, 60, 6100) == 100);
    assert(f.apply(10, 60, 8000) == 100);
    assert(f.apply(2, 60, 10099) == 100);
    assert(f.apply(5, 60, 10100) == 5);
    assert(f.apply(100, 60, 11000) == 100);
    assert(f.apply(1, 60, 11100) == 1);
    assert(f.apply(100, 60, 12000) == 100);
    assert(f.apply(0, 60, 12100) == 0);
    assert(f.apply(100, 60, 13000) == 100);
    assert(f.apply(5, 0, 13100) == 5);
    // Replay the 17:25:54-58 sequence, including the 170 W frame that
    // arrived during the CM response rather than the PW response.
    f.reset();
    assert(f.apply(105, 75, 0) == 105);
    assert(f.apply(10, 75, 657) == 105);
    assert(f.apply(10, 72, 2451) == 105);
    assert(f.apply(10, 71, 3114) == 105);
    assert(f.apply(10, 70, 3779) == 105);
    assert(f.apply(170, 69, 3947) == 170);
    assert(f.apply(10, 65, 5592) == 170);
}
