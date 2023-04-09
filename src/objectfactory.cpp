#include "objectfactory.h"
#include "lockscreen/lockscreenfunctions.h"

#undef IOS_ENABLED
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
#include "ios/lockscreen.h"
#define IOS_ENABLED 1
#endif
#endif

std::function<QZLockscreen*()> ObjectFactory::lockscreenFactory = nullptr;
std::function<QZLockscreenFunctions*()> ObjectFactory::lockscreenFunctionsFactory = nullptr;

QZLockscreen *ObjectFactory::createLockscreen() {

    if(lockscreenFactory)
        return lockscreenFactory();

#ifdef Q_OS_ANDROID
    return new AndroidLockscreen();
#endif

#ifdef IOS_ENABLED
    return new lockscreen();
#endif

    // if(settings.value(QZSettings::log_debug, QZSettings::default_log_debug).toBool()) {
    //     bool iosPelotonWorkaroundActive = settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
    //     return new qdebuglockscreen(iosPelotonWorkaroundActive);
    // }

    return nullptr;
}

QZLockscreenFunctions *ObjectFactory::createLockscreenFunctions() {

    if(lockscreenFunctionsFactory)
        return lockscreenFunctionsFactory();
    return new LockscreenFunctions();

    // if(settings.value(QZSettings::log_debug, QZSettings::default_log_debug).toBool()) {
    //     bool iosPelotonWorkaroundActive = settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
    //     return new qdebuglockscreenfunctions(iosPelotonWorkaroundActive);
    // }
}
