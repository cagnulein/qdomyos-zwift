#include <QSettings>

#ifdef Q_OS_ANDROID
#include <QDebug>
#include <QAndroidJniObject>
#endif

#include "qzsettings.h"
#include "androidlockscreen.h"

AndroidLockscreen::AndroidLockscreen()
{
#ifndef Q_OS_ANDROID
    throw "Not supported in this operating system.";
#endif
}

bool AndroidLockscreen::getGarminCompanionEnabled() const {
    QSettings settings;
    return settings.value(QZSettings::garmin_companion, QZSettings::default_garmin_companion).toBool();
}

QZLockscreenFunctions::configurationType AndroidLockscreen::get_virtualDeviceType() const { return QZLockscreenFunctions::configurationType::NONE; }


void AndroidLockscreen::setTimerDisabled() {  }

void AndroidLockscreen::request() {}

long AndroidLockscreen::heartRate() {
    // Use the Garmin heart rate
    return this->getHR();
}

long AndroidLockscreen::stepCadence() { return this->getFootCad(); }

void AndroidLockscreen::setKcal(double kcal) {}

void AndroidLockscreen::setDistance(double distance) {}

void AndroidLockscreen::virtualbike_ios() { }

void AndroidLockscreen::virtualbike_setHeartRate(unsigned char heartRate) {}

void AndroidLockscreen::virtualbike_setCadence(unsigned short crankRevolutions, unsigned short lastCrankEventTime) {}

void AndroidLockscreen::virtualbike_zwift_ios(bool disableHeartRate) {}

double AndroidLockscreen::virtualbike_getCurrentSlope() { return 0.0;}

double AndroidLockscreen::virtualbike_getCurrentCRR() {return 0.0;}

double AndroidLockscreen::virtualbike_getCurrentCW() {return 0.0;}

double AndroidLockscreen::virtualbike_getPowerRequested() {return 0.0;}

bool AndroidLockscreen::virtualbike_updateFTMS(unsigned short normalizeSpeed, unsigned char currentResistance, unsigned short currentCadence, unsigned short currentWatt, unsigned short CrankRevolutions, unsigned short LastCrankEventTime) { return false;}

int AndroidLockscreen::virtualbike_getLastFTMSMessage(unsigned char *message) { return 0;}

void AndroidLockscreen::virtualrower_ios() {}

void AndroidLockscreen::virtualrower_setHeartRate(unsigned char heartRate) {}

bool AndroidLockscreen::virtualrower_updateFTMS(unsigned short normalizeSpeed, unsigned char currentResistance, unsigned short currentCadence, unsigned short currentWatt, unsigned short CrankRevolutions, unsigned short LastCrankEventTime, unsigned short StrokesCount, unsigned int Distance, unsigned short KCal, unsigned short Pace) { return false; }

int AndroidLockscreen::virtualrower_getLastFTMSMessage(unsigned char *message) { return 0; }

void AndroidLockscreen::virtualtreadmill_zwift_ios() { }

void AndroidLockscreen::virtualtreadmill_setHeartRate(unsigned char heartRate) {}

double AndroidLockscreen::virtualtreadmill_getCurrentSlope() { return 0.0;}

uint64_t AndroidLockscreen::virtualtreadmill_lastChangeCurrentSlope() { return 0;}

double AndroidLockscreen::virtualtreadmill_getPowerRequested() { return 0.0;}

bool AndroidLockscreen::virtualtreadmill_updateFTMS(unsigned short normalizeSpeed, unsigned char currentResistance, unsigned short currentCadence, unsigned short currentWatt, unsigned short currentInclination) { return false;}

double AndroidLockscreen::getVolume() { return 0.0;}

bool AndroidLockscreen::urlParser(const char *url){ return false;}

void AndroidLockscreen::garminconnect_init(){

#ifdef Q_OS_ANDROID
    if(this->getGarminCompanionEnabled()) {
        QAndroidJniObject::callStaticMethod<void>(
            "org/cagnulen/qdomyoszwift/Garmin", "init", "(Landroid/content/Context;)V", QtAndroid::androidContext().object());
    }
#endif
}

int AndroidLockscreen::getHR() {

#ifdef Q_OS_ANDROID
    if(this->getGarminCompanionEnabled()) {
        return QAndroidJniObject::callStaticMethod<jint>("org/cagnulen/qdomyoszwift/Garmin", "getHR", "()I");
    }
#endif
    return 0;
}

int AndroidLockscreen::getFootCad(){

#ifdef Q_OS_ANDROID
    QSettings settings;
    if(settings.value(QZSettings::garmin_companion, QZSettings::default_garmin_companion).toBool()) {
        Cadence = QAndroidJniObject::callStaticMethod<jint>("org/cagnulen/qdomyoszwift/Garmin", "getFootCad", "()I");
        qDebug() << QStringLiteral("Current Garmin Cadence: ") << QString::number(Cadence.value());
    }
#endif
    return 0;
}
