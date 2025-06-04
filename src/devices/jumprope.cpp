#include "jumprope.h"
#ifdef Q_OS_ANDROID
#include <QAndroidJniObject>
#endif
#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif
#include <QSettings>

jumprope::jumprope() {}

bool jumprope::connected() { return false; }
bluetoothdevice::BLUETOOTH_TYPE jumprope::deviceType() { return bluetoothdevice::JUMPROPE; }


uint16_t jumprope::watts(double weight) {
    return m_watt.value();
}

void jumprope::clearStats() {

    moving.clear(true);
    elapsed.clear(true);
    Speed.clear(false);
    JumpsSequence.clear(false);
    JumpsCount.clear(false);
    KCal.clear(true);
    Distance.clear(true);
    Distance1s.clear(true);
    Heart.clear(false);
    m_jouls.clear(true);
    elevationAcc = 0;
    m_watt.clear(false);
    WeightLoss.clear(false);
    WattKg.clear(false);
    Cadence.clear(false);

    Inclination.clear(false);
    for(int i=0; i<maxHeartZone(); i++) {
        hrZonesSeconds[i].clear(false);
    }
}

void jumprope::setPaused(bool p) {

    paused = p;
    moving.setPaused(p);
    elapsed.setPaused(p);
    Speed.setPaused(p);
    JumpsSequence.setPaused(p);
    JumpsCount.setPaused(p);
    KCal.setPaused(p);
    Distance.setPaused(p);
    Distance1s.setPaused(p);
    Heart.setPaused(p);
    m_jouls.setPaused(p);
    m_watt.setPaused(p);
    Inclination.setPaused(p);
    WeightLoss.setPaused(p);
    WattKg.setPaused(p);
    Cadence.setPaused(p);
    for(int i=0; i<maxHeartZone(); i++) {
        hrZonesSeconds[i].setPaused(p);
    }
}

void jumprope::setLap() {

    moving.setLap(true);
    elapsed.setLap(true);
    Speed.setLap(false);
    JumpsSequence.setLap(false);
    JumpsCount.setLap(false);
    KCal.setLap(true);
    Distance.setLap(true);
    Distance1s.setLap(true);
    Heart.setLap(false);
    m_jouls.setLap(true);
    m_watt.setLap(false);
    WeightLoss.setLap(false);
    WattKg.setLap(false);
    Cadence.setLap(false);

    Inclination.setLap(false);
    for(int i=0; i<maxHeartZone(); i++) {
        hrZonesSeconds[i].setLap(false);
    }
}

