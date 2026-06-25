#include "androidstatusbar.h"
#include <QQmlEngine>
#include <QDebug>

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include <QAndroidJniEnvironment>
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
    return QAndroidJniObject::callStaticMethod<jint>("org/cagnulen/qdomyoszwift/CustomQtActivity", "getApiLevel", "()I");
#else
    return 0;
#endif
}

void AndroidStatusBar::onInsetsChanged(int top, int bottom, int left, int right, int waterfallTop,
                                        int waterfallBottom, int waterfallLeft, int waterfallRight)
{
    if (m_top != top || m_bottom != bottom || m_left != left || m_right != right ||
        m_waterfallTop != waterfallTop || m_waterfallBottom != waterfallBottom ||
        m_waterfallLeft != waterfallLeft || m_waterfallRight != waterfallRight) {
        m_top = top;
        m_bottom = bottom;
        m_left = left;
        m_right = right;
        m_waterfallTop = waterfallTop;
        m_waterfallBottom = waterfallBottom;
        m_waterfallLeft = waterfallLeft;
        m_waterfallRight = waterfallRight;
        qDebug() << "Insets changed - Top:" << m_top << "Bottom:" << m_bottom << "Left:" << m_left
                 << "Right:" << m_right << "WaterfallTop:" << m_waterfallTop
                 << "WaterfallBottom:" << m_waterfallBottom << "WaterfallLeft:" << m_waterfallLeft
                 << "WaterfallRight:" << m_waterfallRight;
        emit insetsChanged();
    }
}

#ifdef Q_OS_ANDROID
// JNI method with standard naming convention
extern "C" JNIEXPORT void JNICALL
Java_org_cagnulen_qdomyoszwift_CustomQtActivity_onInsetsChanged(JNIEnv *env, jobject thiz, jint top,
                                                                    jint bottom, jint left, jint right,
                                                                    jint waterfallTop, jint waterfallBottom,
                                                                    jint waterfallLeft, jint waterfallRight)
{
    Q_UNUSED(env);
    Q_UNUSED(thiz);
    if (AndroidStatusBar::instance()) {
        AndroidStatusBar::instance()->onInsetsChanged(top, bottom, left, right, waterfallTop, waterfallBottom,
                                                       waterfallLeft, waterfallRight);
    }
}
#endif
