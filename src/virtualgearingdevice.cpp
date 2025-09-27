#include "virtualgearingdevice.h"
#include "qzsettings.h"
#include <QDebug>
#include <QQmlEngine>
#include <QSettings>

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
#include <jni.h>
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

#ifdef Q_OS_ANDROID
// JNI implementations for settings access
extern "C" {

JNIEXPORT jdouble JNICALL
Java_org_cagnulen_qdomyoszwift_VirtualGearingBridge_getVirtualGearingShiftUpX(JNIEnv *env, jclass clazz)
{
    Q_UNUSED(env)
    Q_UNUSED(clazz)
    QSettings settings;
    return settings.value(QZSettings::virtual_gearing_shift_up_x, QZSettings::default_virtual_gearing_shift_up_x).toDouble();
}

JNIEXPORT jdouble JNICALL
Java_org_cagnulen_qdomyoszwift_VirtualGearingBridge_getVirtualGearingShiftUpY(JNIEnv *env, jclass clazz)
{
    Q_UNUSED(env)
    Q_UNUSED(clazz)
    QSettings settings;
    return settings.value(QZSettings::virtual_gearing_shift_up_y, QZSettings::default_virtual_gearing_shift_up_y).toDouble();
}

JNIEXPORT jdouble JNICALL
Java_org_cagnulen_qdomyoszwift_VirtualGearingBridge_getVirtualGearingShiftDownX(JNIEnv *env, jclass clazz)
{
    Q_UNUSED(env)
    Q_UNUSED(clazz)
    QSettings settings;
    return settings.value(QZSettings::virtual_gearing_shift_down_x, QZSettings::default_virtual_gearing_shift_down_x).toDouble();
}

JNIEXPORT jdouble JNICALL
Java_org_cagnulen_qdomyoszwift_VirtualGearingBridge_getVirtualGearingShiftDownY(JNIEnv *env, jclass clazz)
{
    Q_UNUSED(env)
    Q_UNUSED(clazz)
    QSettings settings;
    return settings.value(QZSettings::virtual_gearing_shift_down_y, QZSettings::default_virtual_gearing_shift_down_y).toDouble();
}

JNIEXPORT jint JNICALL
Java_org_cagnulen_qdomyoszwift_VirtualGearingBridge_getVirtualGearingApp(JNIEnv *env, jclass clazz)
{
    Q_UNUSED(env)
    Q_UNUSED(clazz)
    QSettings settings;
    return settings.value(QZSettings::virtual_gearing_app, QZSettings::default_virtual_gearing_app).toInt();
}

} // extern "C"
#endif

QString VirtualGearingDevice::getLastTouchCoordinates()
{
#ifdef Q_OS_ANDROID
    QAndroidJniObject result = QAndroidJniObject::callStaticObjectMethod(
        "org/cagnulen/qdomyoszwift/VirtualGearingBridge",
        "getLastTouchCoordinates",
        "()Ljava/lang/String;");
    if (result.isValid()) {
        return result.toString();
    }
#endif
    return "0,0";
}

QString VirtualGearingDevice::getShiftUpCoordinates()
{
#ifdef Q_OS_ANDROID
    QAndroidJniObject result = QAndroidJniObject::callStaticObjectMethod(
        "org/cagnulen/qdomyoszwift/VirtualGearingBridge",
        "getShiftUpCoordinates",
        "()Ljava/lang/String;");
    if (result.isValid()) {
        return result.toString();
    }
#endif
    return "0,0";
}

QString VirtualGearingDevice::getShiftDownCoordinates()
{
#ifdef Q_OS_ANDROID
    QAndroidJniObject result = QAndroidJniObject::callStaticObjectMethod(
        "org/cagnulen/qdomyoszwift/VirtualGearingBridge",
        "getShiftDownCoordinates",
        "()Ljava/lang/String;");
    if (result.isValid()) {
        return result.toString();
    }
#endif
    return "0,0";
}