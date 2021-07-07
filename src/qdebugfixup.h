#ifndef QDEBUGFIXUP_H
#define QDEBUGFIXUP_H

#include <QDebug>

// DONE: Eureka found a solution for the debug messages only problem is how make them sustain this state
// NOTE: Needs testing
#undef QT_MESSAGELOG_FILE
#undef QT_MESSAGELOG_LINE
#undef QT_MESSAGELOG_FUNC

#define QT_MESSAGELOG_FILE static_cast<const char *>(__FILE__)
#define QT_MESSAGELOG_LINE __LINE__
#define QT_MESSAGELOG_FUNC static_cast<const char *>(Q_FUNC_INFO)

#endif // QDEBUGFIXUP_H
