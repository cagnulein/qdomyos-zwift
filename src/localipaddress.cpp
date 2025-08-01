#include "localipaddress.h"
#include <QNetworkInterface>

#ifdef Q_OS_ANDROID
#include <QJniEnvironment>
#include <QCoreApplication>
#include <QtEndian>
#endif

#ifdef Q_OS_ANDROID

/*
 * Get WifiManager object
 * Parameters: jCtxObj is Context object
 */
jobject getWifiManagerObj(JNIEnv *env, jobject jCtxObj) {
    qDebug() << "gotWifiMangerObj ";
    // Get the value of Context.WIFI_SERVICE
    // jstring  jstr_wifi_service = env->NewStringUTF("wifi");
    jclass jCtxClz = env->FindClass("android/content/Context");
    jfieldID fid_wifi_service = env->GetStaticFieldID(jCtxClz, "WIFI_SERVICE", "Ljava/lang/String;");
    jstring jstr_wifi_service = (jstring)env->GetStaticObjectField(jCtxClz, fid_wifi_service);

    jclass jclz = env->GetObjectClass(jCtxObj);
    jmethodID mid_getSystemService =
        env->GetMethodID(jclz, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jobject wifiManager = env->CallObjectMethod(jCtxObj, mid_getSystemService, jstr_wifi_service);

    // Because jclass inherits from jobject, it needs to be released;
    // jfieldID and jmethodID are memory addresses, this memory is not allocated in our code, and we don’t need to
    // release it.
    env->DeleteLocalRef(jCtxClz);
    env->DeleteLocalRef(jclz);
    env->DeleteLocalRef(jstr_wifi_service);

    return wifiManager;
}

/*
 * Get the WifiInfo object
 * Parameters: wifiMgrObj is the WifiManager object
 */
jobject getWifiInfoObj(JNIEnv *env, jobject wifiMgrObj) {
    qDebug() << "getWifiInfoObj ";
    if (wifiMgrObj == NULL) {
        return NULL;
    }
    jclass jclz = env->GetObjectClass(wifiMgrObj);
    jmethodID mid = env->GetMethodID(jclz, "getConnectionInfo", "()Landroid/net/wifi/WifiInfo;");
    jobject wifiInfo = env->CallObjectMethod(wifiMgrObj, mid);

    env->DeleteLocalRef(jclz);
    return wifiInfo;
}

/*
 * Get MAC address
 * Parameters: wifiInfoObj, WifiInfo object
 */
char *getMacAddress(JNIEnv *env, jobject wifiInfoObj) {
    qDebug() << "getMacAddress.... ";
    if (wifiInfoObj == NULL) {
        return NULL;
    }
    jclass jclz = env->GetObjectClass(wifiInfoObj);
    jmethodID mid = env->GetMethodID(jclz, "getMacAddress", "()Ljava/lang/String;");
    jstring jstr_mac = (jstring)env->CallObjectMethod(wifiInfoObj, mid);
    if (jstr_mac == NULL) {
        env->DeleteLocalRef(jclz);
        return NULL;
    }

    const char *tmp = env->GetStringUTFChars(jstr_mac, NULL);
    char *mac = (char *)malloc(strlen(tmp) + 1);
    memcpy(mac, tmp, strlen(tmp) + 1);
    env->ReleaseStringUTFChars(jstr_mac, tmp);
    env->DeleteLocalRef(jclz);
    return mac;
}

/*
 * Get MAC address
 * Parameters: wifiInfoObj, WifiInfo object
 */
int getIpAddress(JNIEnv *env, jobject wifiInfoObj) {
    qDebug() << "getIpAddress.... ";
    if (wifiInfoObj == NULL) {
        return NULL;
    }
    jclass jclz = env->GetObjectClass(wifiInfoObj);
    jmethodID mid = env->GetMethodID(jclz, "getIpAddress", "()I");
    return env->CallIntMethod(wifiInfoObj, mid);
}
#endif

QHostAddress localipaddress::getIP(const QHostAddress &srcAddress) {
    // Attempt to find the interface that corresponds with the provided
    // address and determine this device's address from the interface

    if(!srcAddress.isNull()) {
        const auto interfaces = QNetworkInterface::allInterfaces();
        for (const QNetworkInterface &networkInterface : interfaces) {
            const auto entries = networkInterface.addressEntries();
            for (const QNetworkAddressEntry &entry : entries) {
                if (srcAddress.isInSubnet(entry.ip(), entry.prefixLength())) {
                    for (const QNetworkAddressEntry &newEntry : entries) {
                        QHostAddress address = newEntry.ip();
                        if ((address.protocol() == QAbstractSocket::IPv4Protocol && !address.isLoopback())) {
                            qDebug() << "getIP" << address;
                            return address;
                        }
                    }
                }
            }
        }
    }
#ifdef Q_OS_ANDROID
    QJniEnvironment env;
    QJniObject context = QJniObject::callStaticObjectMethod("org/qtproject/qt/android/QtNative", "getContext", "()Landroid/content/Context;");
    jobject wifiManagerObj = getWifiManagerObj(env.jniEnv(), context.object());
    jobject wifiInfoObj = getWifiInfoObj(env.jniEnv(), wifiManagerObj);
    int ip = getIpAddress(env.jniEnv(), wifiInfoObj);
    QHostAddress qip = QHostAddress(qFromBigEndian<quint32>(ip));
    qDebug() << "getIP from JNI" << qip;
    return qip;
#endif
    return QHostAddress();
}
