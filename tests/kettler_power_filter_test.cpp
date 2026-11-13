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
}
