#include "androidstatusbar.h"
#include <QQmlEngine>
#include <QGuiApplication>
#include <QScreen>

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include <QAndroidJniObject>
#include <QAndroidJniEnvironment>
#endif

AndroidStatusBar::AndroidStatusBar(QObject *parent)
    : QObject(parent)
    , m_cachedHeight(-1)
    , m_cachedNavigationBarHeight(-1)
{
    // Listen for orientation changes
    if (QGuiApplication::primaryScreen()) {
        connect(QGuiApplication::primaryScreen(), &QScreen::orientationChanged,
                this, &AndroidStatusBar::onOrientationChanged);
    }
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

int AndroidStatusBar::navigationBarHeight() const
{
    if (m_cachedNavigationBarHeight == -1) {
        m_cachedNavigationBarHeight = getNavigationBarHeightFromAndroid();
    }
    return m_cachedNavigationBarHeight;
}

int AndroidStatusBar::apiLevel() const
{
    return getApiLevelFromAndroid();
}

void AndroidStatusBar::onOrientationChanged()
{
    invalidateCache();
}

void AndroidStatusBar::invalidateCache()
{
    m_cachedHeight = -1;
    m_cachedNavigationBarHeight = -1;
    emit heightChanged();
    emit navigationBarHeightChanged();
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

int AndroidStatusBar::getNavigationBarHeightFromAndroid() const
{
#ifdef Q_OS_ANDROID
    try {
        // Call the static method that returns int directly
        int height = QAndroidJniObject::callStaticMethod<jint>(
            "org/cagnulen/qdomyoszwift/CustomQtActivity",
            "getNavigationBarHeight",
            "()I"
        );
        
        return height;
    } catch (...) {
        // Fallback: return a reasonable default
        return 48; // ~16dp for typical Android devices
    }
#else
    return 0; // Non-Android platforms don't have navigation bar
#endif
}

int AndroidStatusBar::getApiLevelFromAndroid() const
{
#ifdef Q_OS_ANDROID
    try {
        // Call the static method that returns int directly
        int apiLevel = QAndroidJniObject::callStaticMethod<jint>(
            "org/cagnulen/qdomyoszwift/CustomQtActivity",
            "getApiLevel",
            "()I"
        );
        
        return apiLevel;
    } catch (...) {
        // Fallback: return a reasonable default
        return 30; // Default to API 30 if we can't get it
    }
#else
    return 0; // Non-Android platforms
#endif
}