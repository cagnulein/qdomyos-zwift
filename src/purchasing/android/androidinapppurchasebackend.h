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

#ifndef ANDROIDINAPPPURCHASEBACKEND_H
#define ANDROIDINAPPPURCHASEBACKEND_H

#include <QJniObject>
#include <QDateTime>
#include <QMutex>
#include <QSet>
//#include <QJniObject> Qt.6
//#include <QJniEnvironment>

#include "../inapp/inappproduct.h"
#include "../inapp/inapppurchasebackend.h"
#include "../inapp/inapptransaction.h"

QT_BEGIN_NAMESPACE
class AndroidInAppProduct;
class AndroidInAppPurchaseBackend : public InAppPurchaseBackend {
    Q_OBJECT
  public:
    explicit AndroidInAppPurchaseBackend(QObject *parent = 0);

    void initialize();
    bool isReady() const;

    void queryProducts(const QList<Product> &products);
    void queryProduct(InAppProduct::ProductType productType, const QString &identifier);
    void restorePurchases();

    void setPlatformProperty(const QString &propertyName, const QString &value);

    void purchaseProduct(AndroidInAppProduct *product);

    void consumeTransaction(const QString &purchaseToken);
    void registerFinalizedUnlockable(const QString &identifier);

    // Callbacks from Java
    Q_INVOKABLE void registerProduct(const QString &productId, const QString &price, const QString &title,
                                     const QString &description);
    Q_INVOKABLE void registerPurchased(const QString &identifier, const QString &signature, const QString &data,
                                       const QString &purchaseToken, const QString &orderId,
                                       const QDateTime &timestamp);
    Q_INVOKABLE void purchaseSucceeded(int requestCode, const QString &signature, const QString &data,
                                       const QString &purchaseToken, const QString &orderId,
                                       const QDateTime &timestamp);
    Q_INVOKABLE void purchaseFailed(int requestCode, int failureReason, const QString &errorString);
    Q_INVOKABLE void registerReady();

  private:
    void checkFinalizationStatus(InAppProduct *product,
                                 InAppTransaction::TransactionStatus status = InAppTransaction::PurchaseApproved);
    bool transactionFinalizedForProduct(InAppProduct *product);
    void purchaseFailed(InAppProduct *product, int failureReason, const QString &errorString);

    struct PurchaseInfo {
        PurchaseInfo(const QString &signature_, const QString &data_, const QString &purchaseToken_,
                     const QString &orderId_, const QDateTime &timestamp_)
            : signature(signature_), data(data_), purchaseToken(purchaseToken_), orderId(orderId_),
              timestamp(timestamp_) {}

        QString signature;
        QString data;
        QString purchaseToken;
        QString orderId;
        QDateTime timestamp;
    };

    mutable QRecursiveMutex m_mutex;
    bool m_isReady;
    QList<QString> purchasedUnlockebles;
    QJniObject m_javaObject;
    QScopedPointer<AndroidInAppPurchaseBackend> d;
    QHash<QString, InAppProduct::ProductType> m_productTypeForPendingId;
    QHash<QString, PurchaseInfo> m_infoForPurchase;
    QHash<int, InAppProduct *> m_activePurchaseRequests;
};
QT_END_NAMESPACE

#endif // ANDROIDINAPPPURCHASEBACKEND_H
