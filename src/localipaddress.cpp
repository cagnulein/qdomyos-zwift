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
        return NULL;
    }
    jclass jclz = env->GetObjectClass(wifiInfoObj);
    jmethodID mid = env->GetMethodID(jclz, "getIpAddress", "()I");
    return env->CallIntMethod(wifiInfoObj, mid);
}
#endif

namespace {

// Best guess of our own IPv4 when there is no peer address to match against.
// Interfaces are scored by type so that virtual adapters (VirtualBox host-only,
// VPNs, ...) don't win over the real Wi-Fi/Ethernet one.
QHostAddress bestLocalIPv4() {
    QHostAddress best;
    int bestScore = -1;

    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &networkInterface : interfaces) {
        const auto flags = networkInterface.flags();
        if (!flags.testFlag(QNetworkInterface::IsUp) || !flags.testFlag(QNetworkInterface::IsRunning) ||
            flags.testFlag(QNetworkInterface::IsLoopBack)) {
            continue;
        }

        int score;
        switch (networkInterface.type()) {
        case QNetworkInterface::Wifi:
            score = 3;
            break;
        case QNetworkInterface::Ethernet:
            score = 2;
            break;
        case QNetworkInterface::Virtual:
            score = 0;
            break;
        default:
            score = 1;
            break;
        }

        const auto entries = networkInterface.addressEntries();
        for (const QNetworkAddressEntry &entry : entries) {
            const QHostAddress address = entry.ip();
            if (address.protocol() != QAbstractSocket::IPv4Protocol || address.isLoopback() ||
                address.isLinkLocal()) {
                continue;
            }
            if (score > bestScore) {
                bestScore = score;
                best = address;
            }
        }
    }
    return best;
}

} // namespace

QHostAddress localipaddress::getIP(const QHostAddress &srcAddress) {
    // Attempt to find the interface that corresponds with the provided
    // address and determine this device's address from the interface

    if(!srcAddress.isNull()) {
        const auto interfaces = QNetworkInterface::allInterfaces();
        for (const QNetworkInterface &networkInterface : interfaces) {
            // Same gate as bestLocalIPv4(). A disconnected adapter keeps its APIPA
            // entry, and Wi-Fi Direct/VPN adapters are down but still enumerated;
            // answering an mDNS query with one of those addresses hands the client
            // an address it can never reach, which reads as "connecting" forever.
            const auto flags = networkInterface.flags();
            if (!flags.testFlag(QNetworkInterface::IsUp) || !flags.testFlag(QNetworkInterface::IsRunning) ||
                flags.testFlag(QNetworkInterface::IsLoopBack)) {
                continue;
            }
            const auto entries = networkInterface.addressEntries();
            for (const QNetworkAddressEntry &entry : entries) {
                if (srcAddress.isInSubnet(entry.ip(), entry.prefixLength())) {
                    for (const QNetworkAddressEntry &newEntry : entries) {
                        QHostAddress address = newEntry.ip();
                        if (address.protocol() == QAbstractSocket::IPv4Protocol && !address.isLoopback() &&
                            !address.isLinkLocal()) {
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
    jobject wifiManagerObj = getWifiManagerObj(env, QtAndroid::androidContext().object());
    jobject wifiInfoObj = getWifiInfoObj(env, wifiManagerObj);
    int ip = getIpAddress(env, wifiInfoObj);
    QHostAddress qip = QHostAddress(qFromBigEndian<quint32>(ip));
    qDebug() << "getIP from JNI" << qip;
    // WifiInfo.getIpAddress() returns 0 on Android 10+ and on non-wifi connections
    if (!qip.isNull() && qip != QHostAddress(QHostAddress::AnyIPv4))
        return qip;
#endif
    // No peer address to match against (this is how provider.cpp announces the
    // mDNS A record), so fall back to enumerating our own interfaces. Without
    // this the A record is published empty and the service is unreachable.
    const QHostAddress fallback = bestLocalIPv4();
    qDebug() << "getIP fallback" << fallback;
    return fallback;
}
