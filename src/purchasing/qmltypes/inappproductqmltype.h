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

#ifndef INAPPPRODUCTQMLTYPE_H
#define INAPPPRODUCTQMLTYPE_H

#include <QtQml/qqmlparserstatus.h>
#include <QObject>
#include <QtQml/qqml.h>

#include "inappstoreqmltype.h"
#include "../inapp/inappproduct.h"
#include "../inapp/inapptransaction.h"

class InAppTransaction;
class InAppStoreQmlType;
class InAppProductQmlType : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString identifier READ identifier WRITE setIdentifier NOTIFY identifierChanged)
    Q_PROPERTY(ProductType type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(QString price READ price NOTIFY priceChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(InAppStoreQmlType *store READ store WRITE setStore NOTIFY storeChanged)
    QML_NAMED_ELEMENT(Product)

public:
    enum Status {
        Uninitialized,
        PendingRegistration,
        Registered,
        Unknown
    };
    Q_ENUM(Status);

    // Must match InAppProduct::ProductType
    enum ProductType {
        Consumable,
        Unlockable
    };
    Q_ENUM(ProductType);

    explicit InAppProductQmlType(QObject *parent = 0);

    Q_INVOKABLE void purchase();
    Q_INVOKABLE void resetStatus();

    void setIdentifier(const QString &identifier);
    QString identifier() const;

    Status status() const;
    QString price() const;
    QString title() const;
    QString description() const;

    void setStore(InAppStoreQmlType *store);
    InAppStoreQmlType *store() const;

    void setType(ProductType type);
    ProductType type() const;

Q_SIGNALS:
    void purchaseSucceeded(InAppTransaction *transaction);
    void purchaseFailed(InAppTransaction *transaction);
    void purchaseRestored(InAppTransaction *transaction);
    void identifierChanged();
    void statusChanged();
    void priceChanged();
    void titleChanged();
    void descriptionChanged();
    void storeChanged();
    void typeChanged();

protected:
    void componentComplete();
    void classBegin() {}

private Q_SLOTS:
    void handleTransaction(InAppTransaction *transaction);
    void handleProductRegistered(InAppProduct *product);
    void handleProductUnknown(InAppProduct::ProductType, const QString &identifier);

private:
    void setProduct(InAppProduct *product);
    void updateProduct();

    QString m_identifier;
    Status m_status;
    InAppProductQmlType::ProductType m_type;
    bool m_componentComplete;

    InAppStoreQmlType *m_store;
    InAppProduct *m_product;
};

#endif // INAPPPRODUCTQMLTYPE_H
