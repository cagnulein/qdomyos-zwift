#include "kettlerhandshakereader.h"
#include <QDebug>

#ifdef Q_OS_ANDROID
#include <QtAndroidExtras/QAndroidJniObject>
#include <QtAndroidExtras/QtAndroid>
#include <QtAndroidExtras/QAndroidJniEnvironment>
#endif

KettlerHandshakeReader* KettlerHandshakeReader::instance = nullptr;

KettlerHandshakeReader::KettlerHandshakeReader(QObject *parent)
    : QObject(parent)
{
    instance = this;
}

KettlerHandshakeReader* KettlerHandshakeReader::getInstance()
{
    return instance;
}

void KettlerHandshakeReader::connectToDevice(const QString& deviceAddress)
{
#ifdef Q_OS_ANDROID
    qDebug() << "KettlerHandshakeReader::connectToDevice called for device:" << deviceAddress;

    try {
        QAndroidJniEnvironment env;

        // Get the current Android activity context
        QAndroidJniObject activity = QtAndroid::androidActivity();
        if (!activity.isValid()) {
            qDebug() << "KettlerHandshakeReader: Failed to get Android activity";
            emit handshakeReadError("Failed to get Android activity");
            return;
        }

        // Convert QString to jstring
        jstring jDeviceAddress = env->NewStringUTF(deviceAddress.toUtf8().constData());
        if (!jDeviceAddress) {
            qDebug() << "KettlerHandshakeReader: Failed to create jstring for device address";
            emit handshakeReadError("Failed to create device address string");
            return;
        }

        // Call the static Java method
        QAndroidJniObject::callStaticMethod<void>(
            "org/cagnulen/qdomyoszwift/KettlerHandshakeReader",
            "connectToDevice",
            "(Ljava/lang/String;Landroid/content/Context;)V",
            jDeviceAddress,
            activity.object<jobject>()
        );

        // Clean up local reference
        env->DeleteLocalRef(jDeviceAddress);

        // Check for Java exceptions
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            qDebug() << "KettlerHandshakeReader: Java exception occurred";
            emit handshakeReadError("Java exception occurred during device connection");
            return;
        }

        qDebug() << "KettlerHandshakeReader: Successfully called Java connectToDevice method";

    } catch (const std::exception& e) {
        qDebug() << "KettlerHandshakeReader: C++ exception:" << e.what();
        emit handshakeReadError(QString("C++ exception: %1").arg(e.what()));
    } catch (...) {
        qDebug() << "KettlerHandshakeReader: Unknown exception occurred";
        emit handshakeReadError("Unknown exception occurred");
    }
#else
    Q_UNUSED(deviceAddress)
    qDebug() << "KettlerHandshakeReader: Not supported on this platform";
    emit handshakeReadError("Not supported on this platform");
#endif
}

void KettlerHandshakeReader::sendHandshakeResponse(const QByteArray& handshakeData)
{
#ifdef Q_OS_ANDROID
    qDebug() << "KettlerHandshakeReader::sendHandshakeResponse called with" << handshakeData.size() << "bytes";

    try {
        QAndroidJniEnvironment env;

        // Convert QByteArray to jbyteArray
        jbyteArray jHandshakeData = env->NewByteArray(handshakeData.size());
        env->SetByteArrayRegion(jHandshakeData, 0, handshakeData.size(), reinterpret_cast<const jbyte*>(handshakeData.data()));

        // Call the static Java method
        QAndroidJniObject::callStaticMethod<void>(
            "org/cagnulen/qdomyoszwift/KettlerHandshakeReader",
            "sendHandshakeResponse",
            "([B)V",
            jHandshakeData
        );

        // Clean up local reference
        env->DeleteLocalRef(jHandshakeData);

        // Check for Java exceptions
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            qDebug() << "KettlerHandshakeReader: Java exception during handshake response";
        }

    } catch (const std::exception& e) {
        qDebug() << "KettlerHandshakeReader: C++ exception in sendHandshakeResponse:" << e.what();
    }
#else
    Q_UNUSED(handshakeData)
    qDebug() << "KettlerHandshakeReader: sendHandshakeResponse not supported on this platform";
#endif
}

void KettlerHandshakeReader::setPower(int power)
{
#ifdef Q_OS_ANDROID
    qDebug() << "KettlerHandshakeReader::setPower called with power:" << power;

    try {
        // Call the static Java method
        QAndroidJniObject::callStaticMethod<void>(
            "org/cagnulen/qdomyoszwift/KettlerHandshakeReader",
            "setPower",
            "(I)V",
            power
        );

        // Check for Java exceptions
        QAndroidJniEnvironment env;
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            qDebug() << "KettlerHandshakeReader: Java exception during setPower";
        }

    } catch (const std::exception& e) {
        qDebug() << "KettlerHandshakeReader: C++ exception in setPower:" << e.what();
    }
#else
    Q_UNUSED(power)
    qDebug() << "KettlerHandshakeReader: setPower not supported on this platform";
#endif
}

void KettlerHandshakeReader::disconnectDevice()
{
#ifdef Q_OS_ANDROID
    qDebug() << "KettlerHandshakeReader::disconnectDevice called";

    try {
        // Call the static Java method
        QAndroidJniObject::callStaticMethod<void>(
            "org/cagnulen/qdomyoszwift/KettlerHandshakeReader",
            "forceCleanup",
            "()V"
        );

        // Check for Java exceptions
        QAndroidJniEnvironment env;
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            qDebug() << "KettlerHandshakeReader: Java exception during disconnect";
        }

    } catch (const std::exception& e) {
        qDebug() << "KettlerHandshakeReader: C++ exception in disconnectDevice:" << e.what();
    }
#else
    qDebug() << "KettlerHandshakeReader: disconnectDevice not supported on this platform";
#endif
}

#ifdef Q_OS_ANDROID
void KettlerHandshakeReader::onDeviceConnected()
{
    qDebug() << "KettlerHandshakeReader::onDeviceConnected called";
    if (instance) {
        emit instance->deviceConnected();
    } else {
        qDebug() << "KettlerHandshakeReader: No instance available to emit deviceConnected signal";
    }
}

void KettlerHandshakeReader::onDeviceDisconnected()
{
    qDebug() << "KettlerHandshakeReader::onDeviceDisconnected called";
    if (instance) {
        emit instance->deviceDisconnected();
    } else {
        qDebug() << "KettlerHandshakeReader: No instance available to emit deviceDisconnected signal";
    }
}

void KettlerHandshakeReader::onHandshakeSeedReceived(const QByteArray& seedData)
{
    qDebug() << "KettlerHandshakeReader::onHandshakeSeedReceived called with" << seedData.size() << "bytes";
    if (instance) {
        emit instance->handshakeSeedReceived(seedData);
    } else {
        qDebug() << "KettlerHandshakeReader: No instance available to emit signal";
    }
}

void KettlerHandshakeReader::onHandshakeReadError(const QString& error)
{
    qDebug() << "KettlerHandshakeReader::onHandshakeReadError called:" << error;
    if (instance) {
        emit instance->handshakeReadError(error);
    } else {
        qDebug() << "KettlerHandshakeReader: No instance available to emit error signal";
    }
}

void KettlerHandshakeReader::onDataReceived(const QString& characteristicUuid, const QByteArray& data)
{
    qDebug() << "KettlerHandshakeReader::onDataReceived called for characteristic:" << characteristicUuid << "with" << data.size() << "bytes";
    if (instance) {
        emit instance->dataReceived(characteristicUuid, data);
    } else {
        qDebug() << "KettlerHandshakeReader: No instance available to emit dataReceived signal";
    }
}

// JNI callback implementations
extern "C" {
    JNIEXPORT void JNICALL Java_org_cagnulen_qdomyoszwift_KettlerHandshakeReader_onDeviceConnected(JNIEnv *env, jclass clazz)
    {
        Q_UNUSED(env)
        Q_UNUSED(clazz)

        qDebug() << "JNI: Device connected callback";
        KettlerHandshakeReader::onDeviceConnected();
    }

    JNIEXPORT void JNICALL Java_org_cagnulen_qdomyoszwift_KettlerHandshakeReader_onDeviceDisconnected(JNIEnv *env, jclass clazz)
    {
        Q_UNUSED(env)
        Q_UNUSED(clazz)

        qDebug() << "JNI: Device disconnected callback";
        KettlerHandshakeReader::onDeviceDisconnected();
    }

    JNIEXPORT void JNICALL Java_org_cagnulen_qdomyoszwift_KettlerHandshakeReader_onHandshakeSeedReceived(JNIEnv *env, jclass clazz, jbyteArray seedData)
    {
        Q_UNUSED(clazz)

        if (!seedData) {
            qDebug() << "JNI: Received null seed data";
            KettlerHandshakeReader::onHandshakeReadError("Received null seed data from Java");
            return;
        }

        // Convert jbyteArray to QByteArray
        jsize length = env->GetArrayLength(seedData);
        jbyte* bytes = env->GetByteArrayElements(seedData, nullptr);

        if (!bytes) {
            qDebug() << "JNI: Failed to get byte array elements";
            KettlerHandshakeReader::onHandshakeReadError("Failed to get byte array elements");
            return;
        }

        QByteArray data(reinterpret_cast<const char*>(bytes), length);

        qDebug() << "JNI: Received handshake seed data:" << data.toHex(' ');

        // Release the byte array
        env->ReleaseByteArrayElements(seedData, bytes, JNI_ABORT);

        // Call the static method to handle the data
        KettlerHandshakeReader::onHandshakeSeedReceived(data);
    }

    JNIEXPORT void JNICALL Java_org_cagnulen_qdomyoszwift_KettlerHandshakeReader_onHandshakeReadError(JNIEnv *env, jclass clazz, jstring error)
    {
        Q_UNUSED(clazz)

        if (!error) {
            KettlerHandshakeReader::onHandshakeReadError("Unknown error from Java (null string)");
            return;
        }

        // Convert jstring to QString
        const char* errorCStr = env->GetStringUTFChars(error, nullptr);
        QString errorString = QString::fromUtf8(errorCStr);
        env->ReleaseStringUTFChars(error, errorCStr);

        qDebug() << "JNI: Received handshake read error:" << errorString;

        // Call the static method to handle the error
        KettlerHandshakeReader::onHandshakeReadError(errorString);
    }

    JNIEXPORT void JNICALL Java_org_cagnulen_qdomyoszwift_KettlerHandshakeReader_onDataReceived(JNIEnv *env, jclass clazz, jstring characteristicUuid, jbyteArray data)
    {
        Q_UNUSED(clazz)

        if (!characteristicUuid || !data) {
            qDebug() << "JNI: Received null data in onDataReceived";
            return;
        }

        // Convert jstring to QString
        const char* uuidCStr = env->GetStringUTFChars(characteristicUuid, nullptr);
        QString uuidString = QString::fromUtf8(uuidCStr);
        env->ReleaseStringUTFChars(characteristicUuid, uuidCStr);

        // Convert jbyteArray to QByteArray
        jsize length = env->GetArrayLength(data);
        jbyte* bytes = env->GetByteArrayElements(data, nullptr);

        if (!bytes) {
            qDebug() << "JNI: Failed to get byte array elements in onDataReceived";
            return;
        }

        QByteArray dataArray(reinterpret_cast<const char*>(bytes), length);

        qDebug() << "JNI: Received data for characteristic" << uuidString << ":" << dataArray.toHex(' ');

        // Release the byte array
        env->ReleaseByteArrayElements(data, bytes, JNI_ABORT);

        // Call the static method to handle the data
        KettlerHandshakeReader::onDataReceived(uuidString, dataArray);
    }
}
#endif