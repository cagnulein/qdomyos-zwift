#include "safearea.h"
#include <QQmlEngine>
#include <QJSEngine>

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include <QAndroidJniObject>
#include <QAndroidJniEnvironment>
#endif

SafeArea::SafeArea(QObject *parent)
    : QObject(parent)
    , m_top(0)
    , m_right(0)
    , m_bottom(0)
    , m_left(0)
{
#ifdef Q_OS_ANDROID
    // Get initial safe area values from Android
    QAndroidJniObject activity = QtAndroid::androidActivity();
    if (activity.isValid()) {
        QAndroidJniObject safeAreaObject("org/cagnulein/qdomyoszwift/SafeArea", "(Landroid/app/Activity;)V", activity.object<jobject>());
        if (safeAreaObject.isValid()) {
            m_top = safeAreaObject.callMethod<jint>("getTop");
            m_right = safeAreaObject.callMethod<jint>("getRight");
            m_bottom = safeAreaObject.callMethod<jint>("getBottom");
            m_left = safeAreaObject.callMethod<jint>("getLeft");
        }
    }
#endif
}

void SafeArea::registerQmlType()
{
    qmlRegisterSingletonType<SafeArea>("SafeArea", 1, 0, "SafeArea", [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject* {
        Q_UNUSED(engine)
        Q_UNUSED(scriptEngine)
        return new SafeArea();
    });
}

int SafeArea::top() const
{
    return m_top;
}

int SafeArea::right() const
{
    return m_right;
}

int SafeArea::bottom() const
{
    return m_bottom;
}

int SafeArea::left() const
{
    return m_left;
}

void SafeArea::updateSafeArea(int top, int right, int bottom, int left)
{
    if (m_top != top || m_right != right || m_bottom != bottom || m_left != left) {
        m_top = top;
        m_right = right;
        m_bottom = bottom;
        m_left = left;
        emit safeAreaChanged();
    }
}