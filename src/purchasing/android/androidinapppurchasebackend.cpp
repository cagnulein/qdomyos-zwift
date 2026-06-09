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

#include <QJniEnvironment>
#include <QJniObject>
#include <QDebug>
#include <QCoreApplication>
#include <QtCore>

#include "../inapp/inappstore.h"
#include "androidinappproduct.h"
#include "androidinapppurchasebackend.h"
#include "androidinapptransaction.h"

QT_BEGIN_NAMESPACE

AndroidInAppPurchaseBackend::AndroidInAppPurchaseBackend(QObject *parent)
    : InAppPurchaseBackend(parent), m_isReady(false) {
    m_javaObject = QJniObject("org/qtproject/qt/android/purchasing/InAppPurchase");
    QJniObject context = QJniObject::callStaticObjectMethod("org/qtproject/qt/android/QtNative", "getContext", "()Landroid/content/Context;");
    m_javaObject.callMethod<void>("initPointer", "(Landroid/content/Context;J)V", context.object(),
                                  this);

    if (!m_javaObject.isValid()) {
        qWarning("Cannot initialize IAP backend for Android due to missing dependency: InAppPurchase class");
        return;
    }
}

void AndroidInAppPurchaseBackend::initialize() { m_javaObject.callMethod<void>("initializeConnection"); }

bool AndroidInAppPurchaseBackend::isReady() const {
    QMutexLocker locker(&m_mutex);
    return m_isReady;
}

void AndroidInAppPurchaseBackend::restorePurchases() {
    for (const QString &purchasedUnlockeble : purchasedUnlockebles) {
        InAppProduct *product = store()->registeredProduct(purchasedUnlockeble);
        Q_ASSERT(product != 0);

        checkFinalizationStatus(product, InAppTransaction::PurchaseRestored);
    }
}

void AndroidInAppPurchaseBackend::queryProducts(const QList<Product> &products) {
    QMutexLocker locker(&m_mutex);
    QJniEnvironment environment;

    QStringList newProducts;
    for (int i = 0; i < products.size(); ++i) {
        const Product &product = products.at(i);
        if (m_productTypeForPendingId.contains(product.identifier)) {
            qWarning("Product query already pending for %s", qPrintable(product.identifier));
            continue;
        } else {
            m_productTypeForPendingId[product.identifier] = product.productType;
            newProducts.append(product.identifier);
        }
    }

    if (newProducts.isEmpty())
        return;

    jclass cls = environment->FindClass("java/lang/String");
    jobjectArray productIds = environment->NewObjectArray(newProducts.size(), cls, 0);
    environment->DeleteLocalRef(cls);

    for (int i = 0; i < newProducts.size(); ++i) {
        QJniObject identifier = QJniObject::fromString(newProducts.at(i));
        environment->SetObjectArrayElement(productIds, i, identifier.object());
    }

    m_javaObject.callMethod<void>("queryDetails", "([Ljava/lang/String;)V", productIds);
    environment->DeleteLocalRef(productIds);
}

void AndroidInAppPurchaseBackend::queryProduct(InAppProduct::ProductType productType, const QString &identifier) {
    queryProducts(QList<Product>() << Product(productType, identifier));
}

void AndroidInAppPurchaseBackend::setPlatformProperty(const QString &propertyName, const QString &value) {
    QMutexLocker locker(&m_mutex);
    if (propertyName.compare(QStringLiteral("AndroidPublicKey"), Qt::CaseInsensitive) == 0) {
        m_javaObject.callMethod<void>("setPublicKey", "(Ljava/lang/String;)V",
                                      QJniObject::fromString(value).object<jstring>());
    }
}

void AndroidInAppPurchaseBackend::consumeTransaction(const QString &purchaseToken) {
    QMutexLocker locker(&m_mutex);
    m_javaObject.callMethod<void>("consumePurchase", "(Ljava/lang/String;)V",
                                  QJniObject::fromString(purchaseToken).object<jstring>());
}

void AndroidInAppPurchaseBackend::registerFinalizedUnlockable(const QString &purchaseToken) {
    QMutexLocker locker(&m_mutex);
    m_javaObject.callMethod<void>("acknowledgeUnlockablePurchase", "(Ljava/lang/String;)V",
                                  QJniObject::fromString(purchaseToken).object<jstring>());
}

bool AndroidInAppPurchaseBackend::transactionFinalizedForProduct(InAppProduct *product) {
    Q_ASSERT(m_infoForPurchase.contains(product->identifier()));

    if (product->productType() != InAppProduct::Consumable && purchasedUnlockebles.contains(product->identifier())) {
        return true;
    }
    return false;
}

void AndroidInAppPurchaseBackend::checkFinalizationStatus(InAppProduct *product,
                                                          InAppTransaction::TransactionStatus status) {
    // Verifies the finalization status of an item based on the following logic:
    // 1. If the item is not purchased yet, do nothing (it's either never been purchased, or it's a
    //    consumed consumable.
    // 2. If the item is purchased, and it's a consumable, it's unfinalized. Emit a new transaction.
    //    Consumable items are consumed when they are finalized.
    // 3. If the item is purchased, and it's an unlockable, check the local cache for finalized
    //    unlockable purchases. If it's not there, then the transaction is unfinalized. This means
    //    that if the cache gets deleted or corrupted, the worst-case scenario is that the transactions
    //    are republished.
    QHash<QString, PurchaseInfo>::iterator it = m_infoForPurchase.find(product->identifier());

    if (it == m_infoForPurchase.end()) {
        return;
    }

    const PurchaseInfo &info = it.value();
    if (transactionFinalizedForProduct(product)) {
        AndroidInAppTransaction *transaction =
            new AndroidInAppTransaction(info.signature, info.data, info.purchaseToken, info.orderId, status, product,
                                        info.timestamp, InAppTransaction::NoFailure, QString(), this);
        emit transactionReady(transaction);
    }
}

void AndroidInAppPurchaseBackend::registerProduct(const QString &productId, const QString &price, const QString &title,
                                                  const QString &description) {
    QMutexLocker locker(&m_mutex);
    QHash<QString, InAppProduct::ProductType>::iterator it = m_productTypeForPendingId.find(productId);
    Q_ASSERT(it != m_productTypeForPendingId.end());

    AndroidInAppProduct *product = new AndroidInAppProduct(this, price, title, description, it.value(), it.key(), this);

    emit productQueryDone(product);
}

void AndroidInAppPurchaseBackend::registerPurchased(const QString &identifier, const QString &signature,
                                                    const QString &data, const QString &purchaseToken,
                                                    const QString &orderId, const QDateTime &timestamp) {
    QMutexLocker locker(&m_mutex);
    m_infoForPurchase.insert(identifier, PurchaseInfo(signature, data, purchaseToken, orderId, timestamp));

    QHash<QString, InAppProduct::ProductType>::iterator it = m_productTypeForPendingId.find(identifier);
    if (it.value() == InAppProduct::Unlockable && !purchasedUnlockebles.contains(identifier))
        purchasedUnlockebles.append(identifier);
}

void AndroidInAppPurchaseBackend::registerReady() {
    QMutexLocker locker(&m_mutex);
    m_isReady = true;
    emit ready();
}

void AndroidInAppPurchaseBackend::purchaseProduct(AndroidInAppProduct *product) {
    QMutexLocker locker(&m_mutex);
    if (!m_javaObject.isValid()) {
        purchaseFailed(product, InAppTransaction::ErrorOccurred, QStringLiteral("Java backend is not initialized"));
        return;
    }

    int requestCode = 0;
    while (m_activePurchaseRequests.contains(requestCode)) {
        requestCode++;
        if (requestCode == 0) {
            qWarning("No available request code for purchase request.");
            return;
        }
    }

    m_activePurchaseRequests[requestCode] = product;

    m_javaObject.callMethod<void>("launchBillingFlow", "(Ljava/lang/String;I)V",
                                  QJniObject::fromString(product->identifier()).object<jstring>(), requestCode);
}

void AndroidInAppPurchaseBackend::purchaseFailed(int requestCode, int failureReason, const QString &errorString) {
    QMutexLocker locker(&m_mutex);
    InAppProduct *product = m_activePurchaseRequests.take(requestCode);
    if (product == 0) {
        qWarning("No product registered for requestCode %d", requestCode);
        return;
    }

    purchaseFailed(product, failureReason, errorString);
}

void AndroidInAppPurchaseBackend::purchaseFailed(InAppProduct *product, int failureReason, const QString &errorString) {
    InAppTransaction *transaction = new AndroidInAppTransaction(
        QString(), QString(), QString(), QString(), InAppTransaction::PurchaseFailed, product, QDateTime(),
        InAppTransaction::FailureReason(failureReason), errorString, this);
    emit transactionReady(transaction);
}

void AndroidInAppPurchaseBackend::purchaseSucceeded(int requestCode, const QString &signature, const QString &data,
                                                    const QString &purchaseToken, const QString &orderId,
                                                    const QDateTime &timestamp)

{
    QMutexLocker locker(&m_mutex);
    InAppProduct *product = m_activePurchaseRequests.take(requestCode);
    if (product == 0) {
        qWarning("No product registered for requestCode %d", requestCode);
        return;
    }

    m_infoForPurchase.insert(product->identifier(), PurchaseInfo(signature, data, purchaseToken, orderId, timestamp));
    InAppTransaction *transaction =
        new AndroidInAppTransaction(signature, data, purchaseToken, orderId, InAppTransaction::PurchaseApproved,
                                    product, timestamp, InAppTransaction::NoFailure, QString(), this);
    emit transactionReady(transaction);
}
QT_END_NAMESPACE
