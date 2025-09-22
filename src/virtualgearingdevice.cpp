#include "virtualgearingdevice.h"
#include <QDebug>
#include <QQmlEngine>

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
#endif

VirtualGearingDevice* VirtualGearingDevice::m_instance = nullptr;

VirtualGearingDevice::VirtualGearingDevice(QObject *parent) : QObject(parent)
{
    m_instance = this;
}

VirtualGearingDevice* VirtualGearingDevice::instance()
{
    return m_instance;
}

void VirtualGearingDevice::registerQmlType()
{
    qmlRegisterSingletonType<VirtualGearingDevice>("VirtualGearingDevice", 1, 0, "VirtualGearingDevice",
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject* {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            return instance();
        });
}

bool VirtualGearingDevice::isAccessibilityServiceEnabled()
{
#ifdef Q_OS_ANDROID
    QAndroidJniObject activity = QtAndroid::androidActivity();
    if (activity.isValid()) {
        return QAndroidJniObject::callStaticMethod<jboolean>(
            "org/cagnulen/qdomyoszwift/VirtualGearingBridge",
            "isAccessibilityServiceEnabled",
            "(Landroid/content/Context;)Z",
            activity.object<jobject>());
    }
#endif
    return false;
}

void VirtualGearingDevice::openAccessibilitySettings()
{
#ifdef Q_OS_ANDROID
    QAndroidJniObject activity = QtAndroid::androidActivity();
    if (activity.isValid()) {
        QAndroidJniObject::callStaticMethod<void>(
            "org/cagnulen/qdomyoszwift/VirtualGearingBridge",
            "openAccessibilitySettings",
            "(Landroid/content/Context;)V",
            activity.object<jobject>());
    }
#endif
}

void VirtualGearingDevice::simulateShiftUp()
{
#ifdef Q_OS_ANDROID
    qDebug() << "VirtualGearingDevice: Simulating shift up";
    QAndroidJniObject::callStaticMethod<void>(
        "org/cagnulen/qdomyoszwift/VirtualGearingBridge",
        "simulateShiftUp",
        "()V");
#endif
}

void VirtualGearingDevice::simulateShiftDown()
{
#ifdef Q_OS_ANDROID
    qDebug() << "VirtualGearingDevice: Simulating shift down";
    QAndroidJniObject::callStaticMethod<void>(
        "org/cagnulen/qdomyoszwift/VirtualGearingBridge",
        "simulateShiftDown",
        "()V");
#endif
}

void VirtualGearingDevice::simulateTouch(int x, int y)
{
#ifdef Q_OS_ANDROID
    qDebug() << "VirtualGearingDevice: Simulating touch at (" << x << ", " << y << ")";
    QAndroidJniObject::callStaticMethod<void>(
        "org/cagnulen/qdomyoszwift/VirtualGearingBridge",
        "simulateTouch",
        "(II)V",
        x, y);
#endif
}

bool VirtualGearingDevice::isServiceRunning()
{
#ifdef Q_OS_ANDROID
    return QAndroidJniObject::callStaticMethod<jboolean>(
        "org/cagnulen/qdomyoszwift/VirtualGearingBridge",
        "isServiceRunning",
        "()Z");
#endif
    return false;
}