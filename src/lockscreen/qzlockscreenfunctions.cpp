
#include <qglobal.h>
#include <QSettings>
#include "qzsettings.h"
#include "lockscreen/qzlockscreenfunctions.h"
#include "qdebuglockscreenfunctions.h"

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
    QSettings settings;

    if(settings.value(QZSettings::log_debug, QZSettings::default_log_debug).toBool()) {
        bool iosPelotonWorkaroundActive = settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
        return new qdebuglockscreenfunctions(iosPelotonWorkaroundActive);
    }

    return nullptr;
}
