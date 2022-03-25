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

#include <QtCore/qcoreevent.h>

#include "inappproductqmltype.h"
#include "inappstoreqmltype.h"
#include "../inapp/inapptransaction.h"
#include "../inapp/inappstore.h"

InAppProductQmlType::InAppProductQmlType(QObject *parent)
    : QObject(parent)
    , m_status(Uninitialized)
    , m_type(InAppProductQmlType::ProductType(-1))
    , m_componentComplete(false)
    , m_store(0)
    , m_product(0)
{
}

void InAppProductQmlType::setStore(InAppStoreQmlType *store)
{
    if (m_store == store)
        return;

    if (m_store != 0)
        m_store->store()->disconnect(this);

    m_store = store;
    connect(m_store->store(), &InAppStore::productRegistered,
            this, &InAppProductQmlType::handleProductRegistered);
    connect(m_store->store(), &InAppStore::productUnknown,
            this, &InAppProductQmlType::handleProductUnknown);
    connect(m_store->store(), &InAppStore::transactionReady,
                     this, &InAppProductQmlType::handleTransaction);

    updateProduct();

    emit storeChanged();
}

InAppStoreQmlType *InAppProductQmlType::store() const
{
    return m_store;
}

void InAppProductQmlType::componentComplete()
{
    if (!m_componentComplete) {
        m_componentComplete = true;
        updateProduct();
    }
}

void InAppProductQmlType::setIdentifier(const QString &identifier)
{
    if (m_identifier == identifier)
        return;

    if (m_status != Uninitialized) {
        qWarning("A product's identifier cannot be changed once the product has been initialized.");
        return;
    }

    m_identifier = identifier;
    if (m_componentComplete)
        updateProduct();
    emit identifierChanged();
}

void InAppProductQmlType::updateProduct()
{
    if (m_store == 0)
        return;

    Status oldStatus = m_status;
    InAppProduct *product = 0;
    if (m_identifier.isEmpty() || m_type == InAppProductQmlType::ProductType(-1)) {
        m_status = Uninitialized;
    } else {
        product = m_store->store()->registeredProduct(m_identifier);
        if (product != 0 && product == m_product)
            return;

        if (product == 0) {
            m_status = PendingRegistration;
            m_store->store()->registerProduct(InAppProduct::ProductType(m_type), m_identifier);
        } else if (product->productType() != InAppProduct::ProductType(m_type)) {
            qWarning("Product registered multiple times with different product types.");
            product = 0;
            m_status = Uninitialized;
        } else {
            m_status = Registered;
        }
    }

    setProduct(product);
    if (oldStatus != m_status)
        emit statusChanged();
}

void InAppProductQmlType::resetStatus()
{
    updateProduct();
}

QString InAppProductQmlType::identifier() const
{
    return m_identifier;
}

void InAppProductQmlType::setType(InAppProductQmlType::ProductType type)
{
    if (m_type == type)
        return;

    if (m_status != Uninitialized) {
        qWarning("A product's type cannot be changed once the product has been initialized.");
        return;
    }

    m_type = type;
    if (m_componentComplete)
        updateProduct();

    emit typeChanged();
}

InAppProductQmlType::ProductType InAppProductQmlType::type() const
{
    return m_type;
}

InAppProductQmlType::Status InAppProductQmlType::status() const
{
    return m_status;
}

QString InAppProductQmlType::price() const
{
    return m_product != 0 ? m_product->price() : QString();
}

QString InAppProductQmlType::title() const
{
    return m_product != 0 ? m_product->title() : QString();
}

QString InAppProductQmlType::description() const
{
    return m_product != 0 ? m_product->description() : QString();
}

void InAppProductQmlType::setProduct(InAppProduct *product)
{
    if (m_product == product)
        return;

    QString oldPrice = price();
    QString oldTitle = title();
    QString oldDescription = description();
    m_product = product;
    if (price() != oldPrice)
        emit priceChanged();
    if (title() != oldTitle)
        emit titleChanged();
    if (description() != oldDescription)
        emit descriptionChanged();
}

void InAppProductQmlType::handleProductRegistered(InAppProduct *product)
{
    if (product->identifier() == m_identifier) {
        Q_ASSERT(product->productType() == InAppProduct::ProductType(m_type));
        setProduct(product);
        if (m_status != Registered) {
            m_status = Registered;
            emit statusChanged();
        }
    }
}

void InAppProductQmlType::handleProductUnknown(InAppProduct::ProductType, const QString &identifier)
{
    if (identifier == m_identifier) {
        setProduct(0);
        if (m_status != Unknown) {
            m_status = Unknown;
            emit statusChanged();
        }
    }
}

void InAppProductQmlType::handleTransaction(InAppTransaction *transaction)
{
    if (transaction->product()->identifier() != m_identifier)
        return;

    if (transaction->status() == InAppTransaction::PurchaseApproved)
        emit purchaseSucceeded(transaction);
    else if (transaction->status() == InAppTransaction::PurchaseRestored)
        emit purchaseRestored(transaction);
    else{
        emit purchaseFailed(transaction);}
}

void InAppProductQmlType::purchase()
{
    if (m_product != 0 && m_status == Registered)
        m_product->purchase();
    else
        qWarning("Attempted to purchase unregistered product");
}
