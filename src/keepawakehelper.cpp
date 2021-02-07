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
    ant = 0;
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

    /*antObject(true);
    KeepAwakeHelper::antObject(true)->callMethod<void>("antStart","(Landroid/app/Activity;)V", activity.object<jobject>());*/
}

int KeepAwakeHelper::heart(){
    int heart = KeepAwakeHelper::antObject(true)->callMethod<int>("getHeart","()I");
    qDebug() << "antHeart:" << heart;
    return heart;
}

QAndroidJniObject* KeepAwakeHelper::antObject(bool forceCreate){
    if(!ant && forceCreate)
        ant = new QAndroidJniObject("org/cagnulen/qdomyoszwift/Ant");
    return ant;
}

KeepAwakeHelper::~KeepAwakeHelper()
{
    if ( m_wakeLock.isValid() )
    {
        m_wakeLock.callMethod<void>("release", "()V");

        QSettings settings;
        if((settings.value("ant_cadence", false).toBool() || settings.value("ant_heart", false).toBool()) && KeepAwakeHelper::antObject(false))
            KeepAwakeHelper::antObject(false)->callMethod<void>("doUnbindChannelService", "()V");

        qDebug() << "Unlocked device, can now go to standby";
    }
}

#endif
