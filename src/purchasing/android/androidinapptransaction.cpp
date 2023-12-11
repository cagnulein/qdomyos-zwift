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

#include "../inapp/inappproduct.h"
#include "androidinapptransaction.h"
#include "androidinapppurchasebackend.h"

QT_BEGIN_NAMESPACE

AndroidInAppTransaction::AndroidInAppTransaction(const QString &signature,
                                                   const QString &data,
                                                   const QString &purchaseToken,
                                                   const QString &orderId,
                                                   TransactionStatus status,
                                                   InAppProduct *product,
                                                   const QDateTime &timestamp,
                                                   FailureReason failureReason,
                                                   const QString &errorString,
                                                   QObject *parent)
    : InAppTransaction(status, product, parent)
    , m_signature(signature)
    , m_data(data)
    , m_purchaseToken(purchaseToken)
    , m_orderId(orderId)
    , m_timestamp(timestamp)
    , m_errorString(errorString)
    , m_failureReason(failureReason)
{
    Q_ASSERT(qobject_cast<AndroidInAppPurchaseBackend *>(parent) != 0);
}

QString AndroidInAppTransaction::orderId() const
{
    return m_orderId;
}

QDateTime AndroidInAppTransaction::timestamp() const
{
    return m_timestamp;
}

QString AndroidInAppTransaction::errorString() const
{
    return m_errorString;
}

InAppTransaction::FailureReason AndroidInAppTransaction::failureReason() const
{
    return m_failureReason;
}

QString AndroidInAppTransaction::platformProperty(const QString &propertyName) const
{
    if (propertyName.compare(QStringLiteral("AndroidSignature"), Qt::CaseInsensitive) == 0)
        return m_signature;
    else if (propertyName.compare(QStringLiteral("AndroidPurchaseData"), Qt::CaseInsensitive) == 0)
        return m_data;
    else
        return InAppTransaction::platformProperty(propertyName);
}

void AndroidInAppTransaction::finalize()
{
    AndroidInAppPurchaseBackend *backend = qobject_cast<AndroidInAppPurchaseBackend *>(parent());
    if (status() == PurchaseApproved || status() == PurchaseRestored) {
        if (product()->productType() == InAppProduct::Consumable){
            backend->consumeTransaction(m_purchaseToken);}
        else if (product()->productType() == InAppProduct::Unlockable){
            backend->registerFinalizedUnlockable(m_purchaseToken);}
        else {
            qWarning("Product type not implemented.");
        }
    }

    deleteLater();
}
QT_END_NAMESPACE
