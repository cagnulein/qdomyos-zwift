#include "localipaddress.h"
#include <QNetworkInterface>

#ifdef Q_OS_ANDROID
#include <QAndroidJniEnvironment>
#include <QtAndroid>
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
        return 0;
    }
    jclass jclz = env->GetObjectClass(wifiInfoObj);
    jmethodID mid = env->GetMethodID(jclz, "getIpAddress", "()I");
    return env->CallIntMethod(wifiInfoObj, mid);
}

/*
 * Get the device's own IPv4 address via ConnectivityManager.getLinkProperties()
 * on the active network. Unlike QNetworkInterface (which reads netlink route
 * sockets and is blocked by SELinux for untrusted apps on modern Android) and
 * the deprecated WifiManager.getConnectionInfo().getIpAddress() (which returns
 * 0 on Android 10+ without extra permissions), this goes through the regular
 * ConnectivityManager system service and is not subject to either restriction.
 */
QString getConnectivityManagerIp(JNIEnv *env, jobject jCtxObj) {
    qDebug() << "getConnectivityManagerIp....";

    jclass jCtxClz = env->FindClass("android/content/Context");
    jfieldID fidConnService = env->GetStaticFieldID(jCtxClz, "CONNECTIVITY_SERVICE", "Ljava/lang/String;");
    jstring jstrConnService = (jstring)env->GetStaticObjectField(jCtxClz, fidConnService);
    jmethodID midGetSystemService =
        env->GetMethodID(jCtxClz, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jobject connManager = env->CallObjectMethod(jCtxObj, midGetSystemService, jstrConnService);
    env->DeleteLocalRef(jCtxClz);
    env->DeleteLocalRef(jstrConnService);
    if (connManager == NULL) {
        return QString();
    }

    jclass connManagerClz = env->GetObjectClass(connManager);
    // getActiveNetwork() is only available on API 23+; on older devices this
    // GetMethodID throws NoSuchMethodError and returns NULL.
    jmethodID midGetActiveNetwork = env->GetMethodID(connManagerClz, "getActiveNetwork", "()Landroid/net/Network;");
    if (midGetActiveNetwork == NULL) {
        env->ExceptionClear();
        env->DeleteLocalRef(connManagerClz);
        env->DeleteLocalRef(connManager);
        return QString();
    }
    jobject network = env->CallObjectMethod(connManager, midGetActiveNetwork);
    env->DeleteLocalRef(connManager);
    if (network == NULL) {
        env->DeleteLocalRef(connManagerClz);
        return QString();
    }

    jmethodID midGetLinkProperties = env->GetMethodID(
        connManagerClz, "getLinkProperties", "(Landroid/net/Network;)Landroid/net/LinkProperties;");
    jobject linkProperties = env->CallObjectMethod(connManager, midGetLinkProperties, network);
    env->DeleteLocalRef(connManagerClz);
    env->DeleteLocalRef(network);
    if (linkProperties == NULL) {
        return QString();
    }

    jclass linkPropertiesClz = env->GetObjectClass(linkProperties);
    jmethodID midGetLinkAddresses = env->GetMethodID(linkPropertiesClz, "getLinkAddresses", "()Ljava/util/List;");
    jobject linkAddresses = env->CallObjectMethod(linkProperties, midGetLinkAddresses);
    env->DeleteLocalRef(linkPropertiesClz);
    env->DeleteLocalRef(linkProperties);
    if (linkAddresses == NULL) {
        return QString();
    }

    jclass listClz = env->GetObjectClass(linkAddresses);
    jmethodID midSize = env->GetMethodID(listClz, "size", "()I");
    jmethodID midGet = env->GetMethodID(listClz, "get", "(I)Ljava/lang/Object;");
    const jint size = env->CallIntMethod(linkAddresses, midSize);

    jclass linkAddressClz = env->FindClass("android/net/LinkAddress");
    jmethodID midGetAddress = env->GetMethodID(linkAddressClz, "getAddress", "()Ljava/net/InetAddress;");
    jclass inet4Clz = env->FindClass("java/net/Inet4Address");

    QString result;
    for (jint i = 0; i < size; ++i) {
        jobject linkAddress = env->CallObjectMethod(linkAddresses, midGet, i);
        if (linkAddress == NULL) {
            continue;
        }
        jobject inetAddress = env->CallObjectMethod(linkAddress, midGetAddress);
        env->DeleteLocalRef(linkAddress);
        if (inetAddress == NULL) {
            continue;
        }
        if (!env->IsInstanceOf(inetAddress, inet4Clz)) {
            env->DeleteLocalRef(inetAddress);
            continue;
        }

        jclass inetAddressClz = env->GetObjectClass(inetAddress);
        jmethodID midIsLoopback = env->GetMethodID(inetAddressClz, "isLoopbackAddress", "()Z");
        const bool isLoopback = env->CallBooleanMethod(inetAddress, midIsLoopback);
        if (!isLoopback) {
            jmethodID midGetHostAddress = env->GetMethodID(inetAddressClz, "getHostAddress", "()Ljava/lang/String;");
            jstring jHostAddress = (jstring)env->CallObjectMethod(inetAddress, midGetHostAddress);
            const char *hostAddressChars = env->GetStringUTFChars(jHostAddress, NULL);
            result = QString::fromUtf8(hostAddressChars);
            env->ReleaseStringUTFChars(jHostAddress, hostAddressChars);
            env->DeleteLocalRef(jHostAddress);
        }
        env->DeleteLocalRef(inetAddressClz);
        env->DeleteLocalRef(inetAddress);
        if (!result.isEmpty()) {
            break;
        }
    }

    env->DeleteLocalRef(linkAddressClz);
    env->DeleteLocalRef(inet4Clz);
    env->DeleteLocalRef(listClz);
    env->DeleteLocalRef(linkAddresses);
    return result;
}
#endif

static bool isValidUsableIPv4(const QHostAddress &address) {
    return address.protocol() == QAbstractSocket::IPv4Protocol && !address.isLoopback() &&
           !address.isNull() && address != QHostAddress::AnyIPv4;
}

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
    QAndroidJniEnvironment env;

    const QString connectivityIp = getConnectivityManagerIp(env, QtAndroid::androidContext().object());
    if (env->ExceptionCheck()) {
        // getActiveNetwork()/getLinkProperties() require API 23+; ignore on older devices.
        env->ExceptionClear();
    } else if (!connectivityIp.isEmpty()) {
        QHostAddress connectivityAddress(connectivityIp);
        if (isValidUsableIPv4(connectivityAddress)) {
            qDebug() << "getIP from ConnectivityManager" << connectivityAddress;
            return connectivityAddress;
        }
    }

    jobject wifiManagerObj = getWifiManagerObj(env, QtAndroid::androidContext().object());
    jobject wifiInfoObj = getWifiInfoObj(env, wifiManagerObj);
    int ip = getIpAddress(env, wifiInfoObj);
    if (ip != 0) {
        QHostAddress qip = QHostAddress(qFromBigEndian<quint32>(ip));
        if (isValidUsableIPv4(qip)) {
            qDebug() << "getIP from JNI" << qip;
            return qip;
        }
    }
#endif
    return QHostAddress();
}
