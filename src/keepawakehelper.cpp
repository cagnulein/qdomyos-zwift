#include <QDebug>
#ifdef Q_OS_ANDROID
#include <QSettings>
#include <QAndroidJniObject>
#include <QtAndroidExtras/QtAndroid>
#include <QApplication>
#include "keepawakehelper.h"
#include "jni.h"

KeepAwakeHelper::KeepAwakeHelper()
{
    QSettings settings;
    bool wake = settings.value("android_wakelock", true).toBool();
    if(!wake)
    {
        return;
    }

    activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative", "activity", "()Landroid/app/Activity;");
    if ( activity.isValid() )
    {
        QAndroidJniObject serviceName = QAndroidJniObject::getStaticObjectField<jstring>("android/content/Context","POWER_SERVICE");
        if ( serviceName.isValid() )
        {
            QAndroidJniObject powerMgr = activity.callObjectMethod("getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;",serviceName.object<jobject>());
            if ( powerMgr.isValid() )
            {
                jint levelAndFlags = QAndroidJniObject::getStaticField<jint>("android/os/PowerManager","SCREEN_DIM_WAKE_LOCK");

                QAndroidJniObject tag = QAndroidJniObject::fromString( "My Tag" );

                m_wakeLock = powerMgr.callObjectMethod("newWakeLock", "(ILjava/lang/String;)Landroid/os/PowerManager$WakeLock;", levelAndFlags,tag.object<jstring>());
            }
        }
    }

    if ( m_wakeLock.isValid() )
    {
        m_wakeLock.callMethod<void>("acquire", "()V");
        qDebug() << "Locked device, can't go to standby anymore";
    }
    else
    {
        assert( false );
    }
}

KeepAwakeHelper::~KeepAwakeHelper()
{
    if ( m_wakeLock.isValid() )
    {
        m_wakeLock.callMethod<void>("release", "()V");
        qDebug() << "Unlocked device, can now go to standby";
    }
}

#endif
