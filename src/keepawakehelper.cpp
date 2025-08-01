#include "qdebugfixup.h"
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include "qzsettings.h"

KeepAwakeHelper::KeepAwakeHelper() {
    QSettings settings;
    ant = 0;
    bool wake = settings.value(QZSettings::android_wakelock, QZSettings::default_android_wakelock).toBool();
    if (!wake) {
        return;
    }

    activity = QJniObject::callStaticObjectMethod("org/qtproject/qt/android/QtNative", "activity",
                                                         "()Landroid/app/Activity;");
    if (activity.isValid()) {
        QJniObject serviceName =
            QJniObject::getStaticObjectField<jstring>("android/content/Context", "POWER_SERVICE");
        if (serviceName.isValid()) {
            QJniObject powerMgr = activity.callObjectMethod(
                "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;", serviceName.object<jobject>());
            if (powerMgr.isValid()) {
                jint levelAndFlags =
                    QJniObject::getStaticField<jint>("android/os/PowerManager", "SCREEN_DIM_WAKE_LOCK");

                QJniObject tag = QJniObject::fromString("My Tag");

                m_wakeLock =
                    powerMgr.callObjectMethod("newWakeLock", "(ILjava/lang/String;)Landroid/os/PowerManager$WakeLock;",
                                              levelAndFlags, tag.object<jstring>());
            }
        }
    }

    if (m_wakeLock.isValid()) {
        m_wakeLock.callMethod<void>("acquire", "()V");
        qDebug() << "Locked device, can't go to standby anymore";
    } else {
        assert(false);
    }

    keepScreenOn(true);

    /*antObject(true);
    KeepAwakeHelper::antObject(true)->callMethod<void>("antStart","(Landroid/app/Activity;)V",
    activity.object<jobject>());*/
}

void KeepAwakeHelper::keepScreenOn(bool on) {
    QJniObject activity = QJniObject::callStaticObjectMethod("org/qtproject/qt/android/QtNative", "activity",
                                                               "()Landroid/app/Activity;");
    if (activity.isValid()) {
        QJniObject window = activity.callObjectMethod("getWindow", "()Landroid/view/Window;");

        if (window.isValid()) {
            const int FLAG_KEEP_SCREEN_ON = 128;
            if (on) {
                window.callMethod<void>("addFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
                qDebug() << "Activated : Keep screen ON";
            } else {
                window.callMethod<void>("clearFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
                qDebug() << "Deactivated : Keep screen ON";
            }
        }
    }
    QJniEnvironment env;
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
    }
}

int KeepAwakeHelper::heart() {
    int heart = KeepAwakeHelper::antObject(true)->callMethod<int>("getHeart", "()I");
    qDebug() << "antHeart:" << heart;
    return heart;
}

QJniObject *KeepAwakeHelper::antObject(bool forceCreate) {
    if (!ant && forceCreate)
        ant = new QJniObject("org/cagnulen/qdomyoszwift/Ant");
    return ant;
}

KeepAwakeHelper::~KeepAwakeHelper() {
    if (m_wakeLock.isValid()) {
        m_wakeLock.callMethod<void>("release", "()V");

        QSettings settings;
        if ((settings.value(QZSettings::ant_cadence, QZSettings::default_ant_cadence).toBool() || settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool()) &&
            KeepAwakeHelper::antObject(false))
            KeepAwakeHelper::antObject(false)->callMethod<void>("doUnbindChannelService", "()V");

        qDebug() << "Unlocked device, can now go to standby";

        keepScreenOn(false);
    }
}

#endif
