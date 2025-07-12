#include "androidstatusbar.h"
#include <QQmlEngine>

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include <QAndroidJniObject>
#include <QAndroidJniEnvironment>
#endif

AndroidStatusBar::AndroidStatusBar(QObject *parent)
    : QObject(parent)
    , m_cachedHeight(-1)
{
}

void AndroidStatusBar::registerQmlType()
{
    qmlRegisterSingletonType<AndroidStatusBar>("AndroidStatusBar", 1, 0, "AndroidStatusBar", 
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject* {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            return new AndroidStatusBar();
        });
}

int AndroidStatusBar::height() const
{
    if (m_cachedHeight == -1) {
        m_cachedHeight = getStatusBarHeightFromAndroid();
    }
    return m_cachedHeight;
}

int AndroidStatusBar::getStatusBarHeightFromAndroid() const
{
#ifdef Q_OS_ANDROID
    try {
        // Call the static method that returns int directly
        int height = QAndroidJniObject::callStaticMethod<jint>(
            "org/cagnulen/qdomyoszwift/CustomQtActivity",
            "getStatusBarHeight",
            "()I"
        );
        
        return height;
    } catch (...) {
        // Fallback: return a reasonable default
        return 72; // ~24dp for typical Android devices
    }
#else
    return 0; // Non-Android platforms don't have status bar
#endif
}