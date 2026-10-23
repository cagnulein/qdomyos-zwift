#include "androidstatusbar.h"
#include <QQmlEngine>
#include <QDebug>
#include <QGuiApplication>
#include <QFont>
#include <QMutex>

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include <QAndroidJniEnvironment>
#endif
#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

AndroidStatusBar* AndroidStatusBar::m_instance = nullptr;

// The JNI callbacks below run on the Android UI thread. The object lives on the Qt thread and is
// deleted with the QML engine when main() returns, while the activity can outlive it: if the
// activity is paused at that moment (e.g. the screen is off), Qt finishes it only once it is resumed.
static QBasicMutex instanceMutex;

AndroidStatusBar::AndroidStatusBar(QObject *parent) : QObject(parent)
{
    QMutexLocker locker(&instanceMutex);
    m_instance = this;
}

AndroidStatusBar::~AndroidStatusBar()
{
    QMutexLocker locker(&instanceMutex);
    if (m_instance == this)
        m_instance = nullptr;
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

double AndroidStatusBar::systemFontScale() const
{
#ifdef Q_OS_ANDROID
    QAndroidJniObject context = QtAndroid::androidContext();
    if (!context.isValid())
        return 1.0;
    QAndroidJniObject resources = context.callObjectMethod("getResources", "()Landroid/content/res/Resources;");
    QAndroidJniObject configuration = resources.isValid()
        ? resources.callObjectMethod("getConfiguration", "()Landroid/content/res/Configuration;")
        : QAndroidJniObject();
    QAndroidJniEnvironment env;
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        return 1.0;
    }
    if (!configuration.isValid())
        return 1.0;
    const jfloat scale = configuration.getField<jfloat>("fontScale");
    return scale > 0 ? scale : 1.0;
#elif defined(Q_OS_IOS) && !defined(IO_UNDER_QT)
    return lockscreen::systemFontScale();
#elif defined(Q_OS_IOS)
    // The qmake build has no ios/lockscreen.mm (IO_UNDER_QT). Qt gives the app font the size of the
    // body text, 17 at the default system text size: the same scale, read when QZ starts
    const QFont font = QGuiApplication::font();
    const double size = font.pixelSize() > 0 ? font.pixelSize() : font.pointSizeF();
    return size > 0 ? size / 17.0 : 1.0;
#else
    return 1.0;
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

void AndroidStatusBar::onSystemBarSideInsetsChanged(int left, int right)
{
    if (m_systemBarLeft != left || m_systemBarRight != right) {
        m_systemBarLeft = left;
        m_systemBarRight = right;
        qDebug() << "System bar side insets changed - Left:" << m_systemBarLeft << "Right:" << m_systemBarRight;
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
    // Queued: the slot and its insetsChanged() run on the Qt thread, where the QML bindings live.
    // The lock keeps the object from being deleted before the call is posted; a deleted object
    // drops its pending calls.
    QMutexLocker locker(&instanceMutex);
    if (AndroidStatusBar::instance()) {
        QMetaObject::invokeMethod(AndroidStatusBar::instance(), "onInsetsChanged", Qt::QueuedConnection,
                                  Q_ARG(int, top), Q_ARG(int, bottom), Q_ARG(int, left), Q_ARG(int, right),
                                  Q_ARG(int, waterfallTop), Q_ARG(int, waterfallBottom),
                                  Q_ARG(int, waterfallLeft), Q_ARG(int, waterfallRight));
    }
}

extern "C" JNIEXPORT void JNICALL
Java_org_cagnulen_qdomyoszwift_CustomQtActivity_onSystemBarSideInsetsChanged(JNIEnv *env, jobject thiz, jint left,
                                                                                 jint right)
{
    Q_UNUSED(env);
    Q_UNUSED(thiz);
    QMutexLocker locker(&instanceMutex);
    if (AndroidStatusBar::instance()) {
        QMetaObject::invokeMethod(AndroidStatusBar::instance(), "onSystemBarSideInsetsChanged", Qt::QueuedConnection,
                                  Q_ARG(int, left), Q_ARG(int, right));
    }
}
#endif
