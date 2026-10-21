#include <QDebug>
#include <QSettings>
#include "qzsettings.h"
#ifdef Q_OS_ANDROID
#include <QAndroidJniObject>
#include <jni.h>

// Keep Java polling diagnostics governed by the QZ Debug Log option,
// rather than introducing an independent compile-time logging switch.
extern "C" JNIEXPORT jboolean JNICALL
Java_org_cagnulen_qdomyoszwift_QLog_isDebugLoggingEnabledNative(JNIEnv *, jclass) {
    QSettings settings;
    return settings.value(QZSettings::log_debug, QZSettings::default_log_debug).toBool()
               ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL
Java_org_cagnulen_qdomyoszwift_QLog_sendToQt(JNIEnv *env, jclass clazz,
                                             jint level, jstring tag, jstring message) {
    const char *tagChars = env->GetStringUTFChars(tag, nullptr);
    const char *msgChars = env->GetStringUTFChars(message, nullptr);

    QString tagStr = QString::fromUtf8(tagChars);
    QString msgStr = QString::fromUtf8(msgChars);

    // Converti i livelli di log Android in livelli Qt
    switch (level) {
    case 2: // VERBOSE
        qDebug() << "[VERBOSE:" << tagStr << "]" << msgStr;
        break;
    case 3: // DEBUG
        qDebug() << "[DEBUG:" << tagStr << "]" << msgStr;
        break;
    case 4: // INFO
        qInfo() << "[INFO:" << tagStr << "]" << msgStr;
        break;
    case 5: // WARN
        qWarning() << "[WARN:" << tagStr << "]" << msgStr;
        break;
    case 6: // ERROR
        qCritical() << "[ERROR:" << tagStr << "]" << msgStr;
        break;
    case 7: // ASSERT/WTF
        qCritical() << "[ASSERT:" << tagStr << "]" << msgStr;
        break;
    default:
        qDebug() << "[LOG:" << tagStr << "(" << level << ")]" << msgStr;
    }

    env->ReleaseStringUTFChars(tag, tagChars);
    env->ReleaseStringUTFChars(message, msgChars);
}
#endif
