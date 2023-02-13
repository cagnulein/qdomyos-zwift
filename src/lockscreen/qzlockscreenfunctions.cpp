
#include <qglobal.h>
#include "lockscreen/qzlockscreenfunctions.h"

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
#include "ios/ioslockscreenfunctions.h"
#endif
#endif

bool QZLockscreenFunctions::isPelotonWorkaroundActive() const { return false; }

QZLockscreenFunctions *QZLockscreenFunctions::create(){
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    return new IOSLockscreenFunctions();
#endif
#endif
    return nullptr;
}
