#include "androidstatusbar.h"
#include <QQmlEngine>
#include <QDebug>

#ifdef Q_OS_ANDROID
#include <QCoreApplication>
#include <QJniEnvironment>
#include <QJniObject>
#endif

AndroidStatusBar* AndroidStatusBar::m_instance = nullptr;

AndroidStatusBar::AndroidStatusBar(QObject *parent) : QObject(parent)
{
    m_instance = this;
}

AndroidStatusBar* AndroidStatusBar::instance()
{
    return m_instance;
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

int AndroidStatusBar::apiLevel() const
{
#ifdef Q_OS_ANDROID
    return QJniObject::callStaticMethod<jint>("org/cagnulen/qdomyoszwift/CustomQtActivity", "getApiLevel", "()I");
#else
    return 0;
#endif
}

void AndroidStatusBar::onInsetsChanged(int top, int bottom, int left, int right)
{
    if (m_top != top || m_bottom != bottom || m_left != left || m_right != right) {
        m_top = top;
        m_bottom = bottom;
        m_left = left;
        m_right = right;
        qDebug() << "Insets changed - Top:" << m_top << "Bottom:" << m_bottom << "Left:" << m_left << "Right:" << m_right;
        emit insetsChanged();
    }
}

#ifdef Q_OS_ANDROID
// JNI method with standard naming convention
extern "C" JNIEXPORT void JNICALL
Java_org_cagnulen_qdomyoszwift_CustomQtActivity_onInsetsChanged(JNIEnv *env, jobject thiz, jint top, jint bottom, jint left, jint right)
{
    Q_UNUSED(env);
    Q_UNUSED(thiz);
    if (AndroidStatusBar::instance()) {
        AndroidStatusBar::instance()->onInsetsChanged(top, bottom, left, right);
    }
}
#endif
