/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QJniObject>
#include <QtCore/qdatetime.h>
#include <jni.h>

#include "androidinapppurchasebackend.h"

QT_USE_NAMESPACE

static void purchasedProductsQueried(JNIEnv *, jclass, jlong nativePointer) {
    AndroidInAppPurchaseBackend *backend = reinterpret_cast<AndroidInAppPurchaseBackend *>(nativePointer);
    QMetaObject::invokeMethod(backend, "registerReady", Qt::AutoConnection);
}

static void registerProduct(JNIEnv *, jclass, jlong nativePointer, jstring productId, jstring price, jstring title,
                            jstring description) {
    AndroidInAppPurchaseBackend *backend = reinterpret_cast<AndroidInAppPurchaseBackend *>(nativePointer);
    QMetaObject::invokeMethod(
        backend, "registerProduct", Qt::AutoConnection, Q_ARG(QString, QJniObject(productId).toString()),
        Q_ARG(QString, QJniObject(price).toString()), Q_ARG(QString, QJniObject(title).toString()),
        Q_ARG(QString, QJniObject(description).toString()));
}

static void registerPurchased(JNIEnv *, jclass, jlong nativePointer, jstring identifier, jstring signature,
                              jstring data, jstring purchaseToken, jstring orderId, jlong timestamp) {
    QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(qint64(timestamp));
    dateTime.setTimeSpec(Qt::LocalTime);

    AndroidInAppPurchaseBackend *backend = reinterpret_cast<AndroidInAppPurchaseBackend *>(nativePointer);
    QMetaObject::invokeMethod(
        backend, "registerPurchased", Qt::AutoConnection, Q_ARG(QString, QJniObject(identifier).toString()),
        Q_ARG(QString, QJniObject(signature).toString()), Q_ARG(QString, QJniObject(data).toString()),
        Q_ARG(QString, QJniObject(purchaseToken).toString()),
        Q_ARG(QString, QJniObject(orderId).toString()), Q_ARG(QDateTime, dateTime));
}

static void purchaseSucceeded(JNIEnv *, jclass, jlong nativePointer, jint requestCode, jstring signature, jstring data,
                              jstring purchaseToken, jstring orderId, jlong timestamp) {
    QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(qint64(timestamp));
    dateTime.setTimeSpec(Qt::LocalTime);

    AndroidInAppPurchaseBackend *backend = reinterpret_cast<AndroidInAppPurchaseBackend *>(nativePointer);
    QMetaObject::invokeMethod(backend, "purchaseSucceeded", Qt::AutoConnection, Q_ARG(int, int(requestCode)),
                              Q_ARG(QString, QJniObject(signature).toString()),
                              Q_ARG(QString, QJniObject(data).toString()),
                              Q_ARG(QString, QJniObject(purchaseToken).toString()),
                              Q_ARG(QString, QJniObject(orderId).toString()), Q_ARG(QDateTime, dateTime));
}

static void purchaseFailed(JNIEnv *, jclass, jlong nativePointer, jint requestCode, jint failureReason,
                           jstring errorString) {
    AndroidInAppPurchaseBackend *backend = reinterpret_cast<AndroidInAppPurchaseBackend *>(nativePointer);
    QMetaObject::invokeMethod(backend, "purchaseFailed", Qt::AutoConnection, Q_ARG(int, int(requestCode)),
                              Q_ARG(int, int(failureReason)),
                              Q_ARG(QString, QJniObject(errorString).toString()));
}

static JNINativeMethod methods[] = {
    {"purchasedProductsQueried", "(J)V", (void *)purchasedProductsQueried},
    {"registerProduct", "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
     (void *)registerProduct},
    {"registerPurchased",
     "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;J)V",
     (void *)registerPurchased},
    {"purchaseSucceeded", "(JILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;J)V",
     (void *)purchaseSucceeded},
    {"purchaseFailed", "(JIILjava/lang/String;)V", (void *)purchaseFailed}};

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *) {
    static bool initialized = false;
    if (initialized) {
        return JNI_VERSION_1_6;
    }
    initialized = true;

    JNIEnv *env;
    if (vm->GetEnv((void **)&env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    jclass clazz = env->FindClass("org/qtproject/qt/android/purchasing/InAppPurchase");
    if (!clazz) {
        return JNI_ERR;
    }

    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) < 0) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}
