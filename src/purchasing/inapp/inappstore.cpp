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

#include "inappstore.h"
#include "inapppurchasebackend.h"
#include "inapptransaction.h"

#ifdef Q_OS_ANDROID
#include "../android/androidinapppurchasebackend.h"
#endif

#ifdef Q_OS_IOS
#include "../ios/iosinapppurchasebackend.h"
#endif

class IAPRegisterMetaTypes
{
public:
    IAPRegisterMetaTypes()
    {
        qRegisterMetaType<InAppProduct::ProductType>("InAppProduct::ProductType");
    }
} _registerIAPMetaTypes;

InAppStore::InAppStore(QObject *parent)
    : QObject(parent)
{
    d = QSharedPointer<InAppStorePrivate>(new InAppStorePrivate);
    setupBackend();
}

InAppStore::~InAppStore()
{
}

void InAppStore::setupBackend()
{
    #ifdef Q_OS_ANDROID
    d->backend = new AndroidInAppPurchaseBackend;
    #endif
    #ifdef Q_OS_IOS
    d->backend = new IosInAppPurchaseBackend;
    #endif
    d->backend->setStore(this);

    connect(d->backend, &InAppPurchaseBackend::ready,
            this, &InAppStore::registerPendingProducts);
    connect(d->backend, &InAppPurchaseBackend::transactionReady,
            this, &InAppStore::transactionReady);
    connect(d->backend, &InAppPurchaseBackend::productQueryFailed,
            this, &InAppStore::productUnknown);
    connect(d->backend, &InAppPurchaseBackend::productQueryDone,
            this, static_cast<void (InAppStore::*)(InAppProduct *)>(&InAppStore::registerProduct));
}

void InAppStore::registerProduct(InAppProduct *product)
{
    d->registeredProducts[product->identifier()] = product;
    emit productRegistered(product);
}

void InAppStore::registerPendingProducts()
{
    QList<InAppPurchaseBackend::Product> products;
    products.reserve(d->pendingProducts.size());

    QHash<QString, InAppProduct::ProductType>::const_iterator it;
    for (it = d->pendingProducts.constBegin(); it != d->pendingProducts.constEnd(); ++it)
        products.append(InAppPurchaseBackend::Product(it.value(), it.key()));
    d->pendingProducts.clear();

    d->backend->queryProducts(products);
    if (d->pendingRestorePurchases)
        restorePurchases();
}

void InAppStore::restorePurchases()
{
    if (d->backend->isReady()) {
        d->pendingRestorePurchases = false;
        d->backend->restorePurchases();
    } else {
        d->pendingRestorePurchases = true;
    }
}

void InAppStore::setPlatformProperty(const QString &propertyName, const QString &value)
{
    d->backend->setPlatformProperty(propertyName, value);
}

void InAppStore::registerProduct(InAppProduct::ProductType productType, const QString &identifier)
{
    if (!d->backend->isReady()) {
        d->pendingProducts[identifier] = productType;
        if (!d->hasCalledInitialize) {
            d->hasCalledInitialize = true;
            d->backend->initialize();
        }
    } else {
        d->backend->queryProduct(productType, identifier);
    }
}

InAppProduct *InAppStore::registeredProduct(const QString &identifier) const
{
    return d->registeredProducts.value(identifier);
}
